#pragma once
#include <string>

namespace download_upload {

    class DownloadUploader {
    public:
        void DownloadFileFromOCS(const std::string& model_no);
        void UploadFileToOCS();
        
    private:
        std::string get_key_from_application_server();
        std::string url;
        std::string key;
    };

}
