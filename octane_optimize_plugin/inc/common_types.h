#pragma once
#include <string>

namespace common_types {

    enum class LoadResult : int {
        download_success = 0,
        download_failed = 1,
        upload_success = 2,
        upload_failed = 3
    };

    typedef struct {
        std::string access_id;
        std::string access_key;
        std::string bucket;
        std::string end_point;
        std::string security_token;
    } TokenType;

    typedef struct {
        char* ptr;
        size_t len;
    } HttpResonseBuff;

    void init_http_response_buf(HttpResonseBuff* buff);
    size_t write_http_response_buf(void *ptr, size_t size, size_t nmemb, HttpResonseBuff* buff);
}
