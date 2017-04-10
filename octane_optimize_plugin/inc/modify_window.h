#pragma once
#include "sol.hpp"

namespace modify_window {

    class MaterialModifyWindow {
    public:
        MaterialModifyWindow(const std::vector<std::string>& materialNames);
        ~MaterialModifyWindow();
        MaterialModifyWindow(const MaterialModifyWindow&) = delete;
        MaterialModifyWindow& operator=(const MaterialModifyWindow&) = delete;

        void ShowWindow();
        void CloseWindow();

        void ok_button_callback(sol::object component, sol::object event);
        void cancel_button_callback(sol::object component, sol::object event);

    private:
        sol::table window_instance_;
        sol::table ok_button_instance_;
        sol::table cancel_button_instance_;
    };
}
