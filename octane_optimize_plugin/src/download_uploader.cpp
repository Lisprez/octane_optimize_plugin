#pragma once

#include <chrono>
#include <thread>

#include "curl.h"
#include "download_uploader.h"
#include "Crypto.h"
#include "utils.h"
#include "json.hpp"
#include "config_file.h"
#include "common_types.h"
#include "hmac.h"
#include "sha1.h"
#include "base64.h"

constexpr char* GET_ADDRESS = "http://model.fuwo.com/model/octane/get_address/";

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

    common_types::HttpResonseBuff http_response;
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
            curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "no", CURLFORM_COPYCONTENTS, params["no"].c_str(), CURLFORM_END);
            curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "timestamp", CURLFORM_COPYCONTENTS, params["timestamp"].c_str(), CURLFORM_END);
            curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "sign", CURLFORM_COPYCONTENTS, secret.c_str(), CURLFORM_END);

            curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_response);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, common_types::write_http_response_buf);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK)
            {
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
            }
        }
        curl_global_cleanup();
    }
    catch(std::exception& ex)
    {
    }

    free(http_response.ptr);
    return result;
}

bool download_upload::DownloadUploader::get_octane_zip_file(const std::string& url)
{
    std::string file_save_path = octane_plug_utils::find_first_available_driver() + "\\octane_plugin_download_folder\\" + model_no_;
    octane_plug_utils::CreateFolder(file_save_path);
    auto& config_file_instance = config_file::ConfigFile::Get();
    config_file_instance.Write("last_zip_save_folder", file_save_path);
    std::string zip_file_name = file_save_path + "\\" + model_no_ + ".zip";
    if (download_file_from_static_url(url, zip_file_name))
    {
        LastStoredFileFullPathName = zip_file_name;
        return true;
    }
    return false;
}

common_types::LoadResult download_upload::DownloadUploader::DownloadFileFromOCS(const std::string& model_no)
{
    if (model_no.empty() || model_no.length() != 32)
    {
        error_message_ = "model_no empty or format error!";
        return common_types::LoadResult::download_failed;
    }

    LastStoredFileFullPathName.clear();
    model_no_ = model_no;

    std::string url = get_octane_zip_file_url(model_no);
    if (url == "no_specified_file")
    {
        error_message_ = "no target file specified by the model_no.";
        return common_types::LoadResult::download_failed;
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
            error_message_ = "wait for application server to give file url timeout!";
            return common_types::LoadResult::download_failed;
        }
    }
    bool status = get_octane_zip_file(url);

    if (!status)
    {
        return common_types::LoadResult::download_failed;
    }
    return common_types::LoadResult::download_success;
}

unsigned char* hexstr_to_char_array(const char* hexstr)
{
    std::string hex_string(hexstr);
    std::string temp{};
    int len = strlen(hexstr);
    unsigned int n = 0;
    int count = len / 2;

    if (len % 2 != 0)
    {
        return nullptr;
    }

    unsigned char* bin = new unsigned char[1024];

    int it = 0;
    for (it = 0; it < count; it++)
    {
        std::stringstream ss;
        ss << hex_string.substr(it * 2, 2);
        ss >> std::hex >> n;
        std::cout << std::hex << n << std::endl;
        bin[it] = n;
    }
    bin[it] = '\0';

    return bin;
}

std::string compute_signature(const std::map<std::string, std::string>& http_info,
    const std::string& key,
    const std::string& security_token,
    std::string& back_time)
{
    std::string canonical_string{};
    canonical_string.reserve(1024);

    std::string gmt_time = octane_plug_utils::get_gmt_time(time(nullptr));
    back_time = gmt_time;

    canonical_string.append(http_info.at("method")).append("\n")
        .append("\n")
        .append(http_info.at("content-type")).append("\n")
        .append(gmt_time).append("\n")
        .append("x-oss-security-token:").append(security_token).append("\n")
        .append(http_info.at("resource"));

    std::string sha1_hash = hmac<SHA1>(canonical_string, key);

    unsigned char* bin = hexstr_to_char_array(sha1_hash.c_str());

    std::string signature = base64_encode(bin, strlen((char*)bin));

    return signature;
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, FILE* stream)
{
    size_t retcode;
    curl_off_t nread;

    /* in real-world cases, this would probably get this data differently
    as this fread() stuff is exactly what the library already would do
    by default internally */
    retcode = fread(ptr, size, nmemb, stream);

    nread = (curl_off_t)retcode;

    //fprintf(stderr, "*** We read %llu" CURL_FORMAT_CURL_OFF_T
    //    " bytes from file\n", nread);

    return retcode;
}

common_types::LoadResult download_upload::DownloadUploader::UploadFileToOCS(const std::string& model_no,
    const std::string& oss_path,
    const std::string& fullPathFilename)
{
    error_message_.clear();

    int result = 0;
    std::string endpoint = "oss-cn-hangzhou.aliyuncs.com";
    common_types::TokenType token_info = messanger_->GetToken(model_no);

    std::string id = token_info.access_id;
    std::string key = token_info.access_key;
    std::string bucket_name = token_info.bucket;
    std::string security_token = token_info.security_token;

    if (id.empty() || key.empty() || bucket_name.empty())
    {
        error_message_ = messanger_->GetError();
        return common_types::LoadResult::upload_failed;
    }

    std::map<std::string, std::string> http_info{};
    http_info["content-type"] = "application/octet-stream";
    http_info["url"] = "http://" + bucket_name + "." + endpoint + "/" + oss_path;
    http_info["resource"] = "/" + bucket_name + "/" + oss_path;
    http_info["method"] = "PUT";

    std::string gmt_time{};
    std::string signature = compute_signature(http_info, key, security_token, gmt_time);

    std::string authorization{};
    authorization.reserve(1024);
    authorization.append("OSS ");
    authorization.append(id);
    authorization.append(":");
    authorization.append(signature);
    CURL* curl;
    CURLcode res;
    FILE* hd_src;
    struct stat file_info;
    stat(fullPathFilename.c_str(), &file_info);
    fopen_s(&hd_src, fullPathFilename.c_str(), "rb");
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        std::string tmp{};
        struct curl_slist* chunk = nullptr;
        chunk = curl_slist_append(chunk, (tmp + "Content-Type: " + http_info.at("content-type")).c_str());
        chunk = curl_slist_append(chunk, (tmp + "Date: " + gmt_time).c_str());
        chunk = curl_slist_append(chunk, (tmp + "Authorization: " + authorization).c_str());
        chunk = curl_slist_append(chunk, (tmp + "x-oss-security-token: " + security_token).c_str());
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, http_info.at("url").c_str());
        curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
            (curl_off_t)file_info.st_size);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            //LOG(INFO) << "curl put file error!";
            //fprintf(stderr, "curl_easy_perform() failed: %s\n",
                //curl_easy_strerror(res));
            error_message_ = curl_easy_strerror(res);
            return common_types::LoadResult::upload_failed;
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        error_message_ = "init libcurl for upload error!";
        return common_types::LoadResult::upload_failed;
    }

    fclose(hd_src);
    curl_global_cleanup();

    return common_types::LoadResult::upload_success;
}


download_upload::DownloadUploader::DownloadUploader()
    : messanger_(new messanger::Messanger())
{
    
}

download_upload::DownloadUploader::~DownloadUploader()
{
    delete messanger_;
}

std::string download_upload::DownloadUploader::GetError() const
{
    return error_message_;
}

std::string download_upload::DownloadUploader::GetTestImage(const std::string& url)
{
    config_file::ConfigFile& config_file_instance = config_file::ConfigFile::Get();
    std::string local_model_folder = std::get<0>(config_file_instance.Read("last_zip_save_folder"));
    std::string test_image_local_save_path = local_model_folder + "\\test.jpg";
    if (download_file_from_static_url(url, test_image_local_save_path))
    {
        return test_image_local_save_path;
    }
    else
    {
        return "";
    }
}

bool download_upload::DownloadUploader::download_file_from_static_url(const std::string& url, const std::string& localSavePath)
{
    FILE* my_zip_file = fopen(localSavePath.c_str(), "ab+");
    if (!my_zip_file)
    {
        error_message_ = "create local file error!";
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
        catch (std::exception& ex)
        {
            error_message_ = "network connection error!";
            fclose(my_zip_file);
            return false;
        }
        fclose(my_zip_file);
        return true;
    }
    error_message_ = "init the libcurl error!";
    fclose(my_zip_file);
    return false;
}
