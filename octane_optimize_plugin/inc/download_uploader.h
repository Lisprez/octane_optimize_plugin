#pragma once
#include <string>

namespace download_upload {
    
    enum class LoadResult : int {
        download_success = 0,
        download_failed = 1,
        upload_success = 2,
        upload_failed = 3
    };

    typedef struct {
        char* ptr;
        size_t len;
    } HttpResonseBuff;

    void init_http_response_buf(HttpResonseBuff* buff);
    size_t write_http_resonse_buf(void *ptr, size_t size, size_t nmemb, HttpResonseBuff* buff);
    size_t write_file(void *ptr, size_t size, size_t nmemb, FILE* buff);

    class DownloadUploader {
    public:
        LoadResult DownloadFileFromOCS(const std::string& model_no);
        LoadResult UploadFileToOCS();
        
    private:
        std::string get_key_from_application_server();
        std::string get_octane_zip_file_url(const std::string& model_no);
        bool get_octane_zip_file(const std::string& url);
        std::string url;
        std::string key;
        std::string model_no_;
    };

}
