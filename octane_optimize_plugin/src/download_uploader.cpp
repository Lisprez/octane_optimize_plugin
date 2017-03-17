#pragma once

#include "curl.h"
#include "download_uploader.h"
#include "Crypto.h"
#include "utils.h"
#include "json.hpp"
#include "config_file.h"
#include "easylogging++.h"
#include <chrono>
#include <thread>

constexpr char* GET_ADDRESS = "http://model.fuwo.com/model/octane/get_address/";

void download_upload::init_http_response_buf(download_upload::HttpResonseBuff* buff)
{
    buff->len = 0;
    buff->ptr = (char*)malloc(buff->len + 1);
    if (buff->ptr == nullptr)
    {
        LOG(ERROR) << "Allocate memory for http response error." << std::endl;
        exit(EXIT_FAILURE);
    }
    buff->ptr[0] = '\0';
}

size_t download_upload::write_http_resonse_buf(void* ptr, size_t size, size_t nmemb, HttpResonseBuff* buff)
{
    size_t new_len = buff->len + size*nmemb;
    buff->ptr = (char*)realloc(buff->ptr, new_len + 1);
    if (buff->ptr == nullptr)
    {
        LOG(ERROR) << "Rellocate memory for http response error." << std::endl;
        exit(EXIT_FAILURE);
    }
    memcpy(buff->ptr + buff->len, ptr, size*nmemb);
    buff->ptr[new_len] = '\0';
    buff->len = new_len;
    return size*nmemb;
}

size_t download_upload::write_file(void* ptr, size_t size, size_t nmemb, FILE* buff)
{
    if (buff == nullptr)
    {
        return 0;
    }

    fwrite(ptr, size, nmemb, buff);
    return size * nmemb;
}

std::string download_upload::DownloadUploader::get_octane_zip_file_url(const std::string& model_no)
{
    std::string result{};
    shadow::Crypto::Init();
    std::map<std::string, std::string> params{};
    params["no"] = model_no;
    params["timestamp"] = octane_plug_utils::GetUTC();
    std::string secret{};
    shadow::Crypto::SignDict("3XngWHHT12Rr0ecSKULWJnTcvrOGfPoO", params, secret);

    HttpResonseBuff http_response;
    init_http_response_buf(&http_response);
    
    try
    {
        CURL* curl = nullptr;
        CURLcode res;
        struct curl_httppost* formpost = nullptr;
        struct curl_httppost* lastptr = nullptr;
        // 在Windows里这一步是初始化winsock
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        if (curl != nullptr)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
            curl_easy_setopt(curl, CURLOPT_URL, GET_ADDRESS);
            curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, params["no"].c_str(), CURLFORM_END);
            curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, params["timestamp"].c_str(), CURLFORM_END);
            curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "sign", secret.c_str(), CURLFORM_END);

            curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_response);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_http_resonse_buf);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK)
            {
                LOG(ERROR) << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
            curl_easy_cleanup(curl);
            curl_formfree(formpost);
            using json = nlohmann::json;
            auto json_structure = json::parse(http_response.ptr);
            std::string http_response_code = json_structure["code"];
            if (http_response_code == "10000")
            {
                std::string decrypted_data{};
                auto json_data = json_structure["data"].get<std::string>();
                shadow::Crypto::Decrypto(json_data, decrypted_data);
                auto path_data = json::parse(decrypted_data);
                result = path_data["octane_zip"].get<std::string>();
                std::string test_image_url = path_data["test_url"].get<std::string>();
                config_file::ConfigFile& config_file_instance = config_file::ConfigFile::Get();
                config_file_instance.Write("test_image_url", test_image_url);
            }
            else if (http_response_code == "10001")
            {
                result = "no_specified_file";
            }
            else
            {
                LOG(ERROR) << "Http response with code: " << http_response_code << std::endl;
            }
        }
        curl_global_cleanup();
    }
    catch(std::exception& ex)
    {
        LOG(ERROR) << "Curl exception: " << ex.what();
    }

    free(http_response.ptr);
    return result;
}

bool download_upload::DownloadUploader::get_octane_zip_file(const std::string& url)
{
    std::string file_save_path = octane_plug_utils::find_first_available_driver() + "\\" + model_no_;
    auto& config_file_instance = config_file::ConfigFile::Get();
    config_file_instance.Write("last_zip_save_folder", file_save_path);
    std::string zip_file_name = file_save_path + "\\" + model_no_ + ".zip";
    FILE* my_zip_file = fopen(zip_file_name.c_str(), "ab+");
    if (!my_zip_file)
    {
        return false;
    }

    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl != nullptr)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3600);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, my_zip_file);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        try 
        {
            res = curl_easy_perform(curl);
        }
        catch(std::exception& ex)
        {
            LOG(ERROR) << "Http get file error: " << ex.what() << std::endl;
            fclose(my_zip_file);
            return false;
        }
        fclose(my_zip_file);
        return true;
    }

    fclose(my_zip_file);
    return false;
}

download_upload::LoadResult download_upload::DownloadUploader::DownloadFileFromOCS(const std::string& model_no)
{
    if (model_no.empty() || model_no.length() != 32)
    {
        LOG(ERROR) << "Model no error!" << std::endl;
        return download_upload::LoadResult::download_failed;
    }
    
    model_no_ = model_no;

    std::string url = get_octane_zip_file_url(model_no);
    if (url == "no_specified_file")
    {
        return LoadResult::download_failed;
    }

    int require_url_count = 0;
    using namespace std::chrono_literals;
    while (url.empty())
    {
        std::this_thread::sleep_for(5s);
        url = get_octane_zip_file_url(model_no);
        require_url_count += 1;
        if (require_url_count == 60)
        {
            LOG(INFO) << "Waiting for octane zip file static url timeout!!!" << std::endl;
            return LoadResult::download_failed;
        }
    }
    bool status = get_octane_zip_file(url);

    if (!status)
    {
        return LoadResult::download_failed;
    }
}