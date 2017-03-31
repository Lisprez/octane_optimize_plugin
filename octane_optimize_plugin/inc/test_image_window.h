#pragma once
#include "sol.hpp"
#include "download_uploader.h"

namespace test_image_window {

    class TestImageWindow {
    public:
        TestImageWindow(const std::string& testImagePath, const std::string& model_no);
        ~TestImageWindow();
        TestImageWindow(const TestImageWindow&) = delete;
        TestImageWindow& operator=(const TestImageWindow&) = delete;

        void ShowWindow();
        void CloseWindow();

        void confirm_button_callback(sol::object component, sol::object event);
        void give_up_button_callback(sol::object component, sol::object event);

    private:
        sol::table window_instance_;
        sol::table confirm_button_instance_;
        sol::table give_up_button_instance_;
        download_upload::DownloadUploader* download_uploader_;
        std::string model_no_;
    };
}