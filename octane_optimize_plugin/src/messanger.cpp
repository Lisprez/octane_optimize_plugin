#include "WinSock2.h"
#include "messanger.h"
#include "Crypto.h"
#include "utils.h"
#include "json.hpp"
#include "common_types.h"
#include "curl.h"
constexpr char* GET_TOKEN = "http://model.fuwo.com/model/octane/get_token/";
constexpr char* UPDATE_MODEL = "http://model.fuwo.com/model/octane/update_model/";
constexpr char* CREATE_TASK = "http://model.fuwo.com/model/octane/create_task/";

common_types::TokenType messanger::Messanger::GetToken(const std::string& model_no)
{
    error_message_.clear();
    shadow::Crypto::Init();
    common_types::TokenType token_info{};
    std::map<std::string, std::string> params{};
    params["no"] = model_no;
    params["timestamp"] = octane_plug_utils::GetUTC();
    std::cout << "Current UTC time: " << params["timestamp"] << std::endl;
    params["platform"] = "10";

    std::string secret;
    shadow::Crypto::SignDict("3XngWHHT12Rr0ecSKULWJnTcvrOGfPoO", params, secret);

    common_types::HttpResonseBuff http_response;
    common_types::init_http_response_buf(&http_response);
    try
    {
        CURL *pCurl = nullptr;
        CURLcode res;

        struct curl_httppost *formpost = NULL;
        struct curl_httppost *lastptr = NULL;

        curl_global_init(CURL_GLOBAL_ALL);

        pCurl = curl_easy_init();
        if (nullptr != pCurl)
        {

            // 设置超时时间为1秒  
            curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 5);

            // First set the URL that is about to receive our POST.   
            // This URL can just as well be a   
            // https:// URL if that is what should receive the data.  
            curl_easy_setopt(pCurl, CURLOPT_URL, GET_TOKEN);

            // 设置http发送的内容类型为JSON  
            //curl_slist *plist = curl_slist_append(NULL,
            //	"Content-Type:application/json;charset=UTF-8");
            //curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
            curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, "no",
                CURLFORM_COPYCONTENTS, params["no"].c_str(),
                CURLFORM_END);

            curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, "timestamp",
                CURLFORM_COPYCONTENTS, params["timestamp"].c_str(),
                CURLFORM_END);

            curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, "platform",
                CURLFORM_COPYCONTENTS, "10",
                CURLFORM_END);

            curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, "sign",
                CURLFORM_COPYCONTENTS, secret.c_str(),
                CURLFORM_END);


            // 设置要POST的JSON数据  
            //curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, json_str_buf);
            curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, formpost);
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &http_response);
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, common_types::write_http_response_buf);

            // Perform the request, res will get the return code   
            res = curl_easy_perform(pCurl);
            // Check for errors  
            if (res != CURLE_OK)
            {
                //printf("curl_easy_perform() failed:%s\n", curl_easy_strerror(res));
                error_message_ = curl_easy_strerror(res);
            }
            // always cleanup  
            curl_easy_cleanup(pCurl);
            curl_formfree(formpost);
            std::cout << http_response.ptr << std::endl;
            using json = nlohmann::json;
            auto jsonStructure = json::parse(http_response.ptr);
            if (jsonStructure["code"].get<std::string>() == "10000")
            {
                std::string decrypted_data{};
                auto data = jsonStructure["data"].get<std::string>();
                shadow::Crypto::Decrypto(data, decrypted_data);

                auto token = json::parse(decrypted_data);
                std::string access_id = token["token"]["access_key_id"].get<std::string>();
                std::string access_key = token["token"]["access_key_secret"].get<std::string>();
                std::string security_token = token["token"]["security_token"].get<std::string>();
                std::string bucket = token["bucket_name"].get<std::string>();
                std::string end_point = token["end_point"].get<std::string>();

                token_info.access_id = access_id;
                token_info.access_key = access_key;
                token_info.bucket = bucket;
                token_info.end_point = end_point;
                token_info.security_token = security_token;
            }
            else
            {
                //std::cout << "fuck error" << std::endl;
                error_message_ = "get token error with code != 10000";
            }
        }
        curl_global_cleanup();
    }
    catch (std::exception &ex)
    {
        //LOG(INFO) << "curl exception." << ex.what() << std::endl;
        error_message_ = ex.what();
    }
    free(http_response.ptr);
    return token_info;
}


std::string messanger::Messanger::GetError() const
{
    return error_message_;
}

messanger::Messanger::Messanger()
{

}

messanger::Messanger::~Messanger()
{

}

bool messanger::Messanger::send_message(const std::string& model_no,
    const std::string& api_path,
    std::function<bool(const char*)> cb)
{
    bool result = false;
    error_message_.clear();

    std::map<std::string, std::string> params{};
    params["no"] = model_no;
    params["timestamp"] = octane_plug_utils::GetUTC();

    std::string secret;
    shadow::Crypto::SignDict("3XngWHHT12Rr0ecSKULWJnTcvrOGfPoO", params, secret);

    common_types::HttpResonseBuff http_response;
    common_types::init_http_response_buf(&http_response);
    try
    {
        CURL* pCurl = nullptr;
        CURLcode res;

        struct curl_httppost* formpost = NULL;
        struct curl_httppost* lastptr = NULL;

        // In windows, this will init the winsock stuff  
        curl_global_init(CURL_GLOBAL_ALL);

        // get a curl handle  
        pCurl = curl_easy_init();
        if (NULL != pCurl)
        {

            // 设置超时时间为1秒  
            curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 3000);

            // First set the URL that is about to receive our POST.   
            // This URL can just as well be a   
            // https:// URL if that is what should receive the data.  
            //curl_easy_setopt(pCurl, CURLOPT_URL, "http://model.fuwo.com/model/octane/update_model/");
            curl_easy_setopt(pCurl, CURLOPT_URL, api_path.c_str());

            // 设置http发送的内容类型为JSON  
            //curl_slist *plist = curl_slist_append(NULL,
            //	"Content-Type:application/json;charset=UTF-8");
            //curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
            curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, "no",
                CURLFORM_COPYCONTENTS, params["no"].c_str(),
                CURLFORM_END);

            curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, "timestamp",
                CURLFORM_COPYCONTENTS, params["timestamp"].c_str(),
                CURLFORM_END);

            curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, "sign",
                CURLFORM_COPYCONTENTS, secret.c_str(),
                CURLFORM_END);


            // 设置要POST的JSON数据  
            curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, formpost);
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &http_response);
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, common_types::write_http_response_buf);

            // Perform the request, res will get the return code   
            res = curl_easy_perform(pCurl);
            // Check for errors  
            if (res != CURLE_OK)
            {
                //printf("curl_easy_perform() failed:%s\n", curl_easy_strerror(res));
                error_message_ = curl_easy_strerror(res);
                return false;
            }
            // always cleanup  
            curl_easy_cleanup(pCurl);
            curl_formfree(formpost);
            /*
            using json = nlohmann::json;
            auto jsonStructure = json::parse(http_response.ptr);
            std::cout << http_response.ptr << std::endl;
            if (jsonStructure["code"].get<std::string>() == "10000")
            {
                //LOG(INFO) << "Inform update_model succeed.";
                return true;
            }
            else if (jsonStructure["code"].get<std::string>() == "20005")
            {
                //LOG(INFO) << "Inform update_model error: " << jsonStructure["msg"].get<std::string>() << std::endl;
                error_message_ = jsonStructure["msg"].get<std::string>();
                return false;
            }
            else
            {
                //LOG(INFO) << "Inform update_model error: unknow reason!" << std::endl;
                error_message_ = "inform error with unknow reason";
                return false;
            }*/
            result = cb(http_response.ptr);
        }
        curl_global_cleanup();
    }
    catch (std::exception& ex)
    {
        //LOG(INFO) << "curl exception " << ex.what() << std::endl;
        error_message_ = ex.what();
        return false;
    }

    free(http_response.ptr);

    return result;
}

bool messanger::Messanger::InformUpdateModel(const std::string& model_no)
{
    return send_message(model_no, UPDATE_MODEL, [this](const char* http_response_buf) {
        using json = nlohmann::json;
        auto jsonStructure = json::parse(http_response_buf);
        std::cout << http_response_buf << std::endl;
        if (jsonStructure["code"].get<std::string>() == "10000")
        {
            //LOG(INFO) << "Inform update_model succeed.";
            return true;
        }
        else if (jsonStructure["code"].get<std::string>() == "20005")
        {
            //LOG(INFO) << "Inform update_model error: " << jsonStructure["msg"].get<std::string>() << std::endl;
            error_message_ = jsonStructure["msg"].get<std::string>();
            return false;
        }
        else
        {
            //LOG(INFO) << "Inform update_model error: unknow reason!" << std::endl;
            error_message_ = "inform error with unknow reason";
            return false;
        }
    });
}

bool messanger::Messanger::InformCreateTask(const std::string& model_no)
{
    return send_message(model_no, CREATE_TASK, [this](const char* http_response_buff) {
        using json = nlohmann::json;
        auto jsonStructure = json::parse(http_response_buff);
        if (jsonStructure["code"].get<std::string>() == "10000")
        {
            //LOG(INFO) << "Inform succeed.";
            return true;
        }
        else
        {
            //std::cout << "fuck error" << std::endl;
            return false;
        }
    });
}
