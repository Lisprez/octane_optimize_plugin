#pragma once
#include <string>
#include "common_types.h"
#include "messanger.h"
#include "octane_lua_api.h"

namespace download_upload {
    size_t write_file(void *ptr, size_t size, size_t nmemb, FILE* buff);

    class DownloadUploader {
    public:
        DownloadUploader();
        ~DownloadUploader();

        void SetProgressBar(sol::table progressBar);

        common_types::LoadResult DownloadFileFromOCS(const std::string& model_no);
        common_types::LoadResult UploadFileToOCS(const std::string& model_no, const std::string& oss_path, const std::string& fullPathFileName);
        bool InformUpdateModel(const std::string& model_no) const;
        bool InformCreateTask(const std::string& model_no) const;

        std::string GetTestImage(const std::string& url);

        std::string GetError() const;

        std::string LastStoredFileFullPathName;
    private:
        std::string get_octane_zip_file_url(const std::string& model_no);
        bool get_octane_zip_file(const std::string& url);
        bool download_file_from_static_url(const std::string& url, const std::string& localSavePath);
        std::string url;
        std::string key;
        std::string model_no_;
        messanger::Messanger* messanger_;
        std::string error_message_;
        sol::table progress_bar_;
    };

}
