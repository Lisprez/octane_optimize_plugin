#include <cstdlib>
#include <cstring>

#include "common_types.h"

void common_types::init_http_response_buf(common_types::HttpResonseBuff* buff)
{
    buff->len = 0;
    buff->ptr = (char*)malloc(buff->len + 1);
    if (buff->ptr == nullptr)
    {
        exit(EXIT_FAILURE);
    }
    buff->ptr[0] = '\0';
}

size_t common_types::write_http_response_buf(void* ptr, size_t size, size_t nmemb, HttpResonseBuff* buff)
{
    size_t new_len = buff->len + size*nmemb;
    buff->ptr = (char*)realloc(buff->ptr, new_len + 1);
    if (buff->ptr == nullptr)
    {
        exit(EXIT_FAILURE);
    }
    memcpy(buff->ptr + buff->len, ptr, size*nmemb);
    buff->ptr[new_len] = '\0';
    buff->len = new_len;
    return size*nmemb;
}