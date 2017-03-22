#pragma once
#include <functional>
#include "common_types.h"

namespace messanger {
    
    class Messanger {
    public:
        Messanger();
        ~Messanger();

        common_types::TokenType GetToken(const std::string& model_no);
        std::string GetError() const;
        bool InformUpdateModel(const std::string& model_no);
        bool InformCreateTask(const std::string& model_no);
    private:
        bool send_message(const std::string& model_no, const std::string& api_path, std::function<bool(const char*)> cb);
        std::string error_message_;
    };
}
