#pragma once
#include "sol.hpp"
#include "download_uploader.h"

namespace gui {

    class MainWindow {
    public:
        MainWindow();
        ~MainWindow();
        MainWindow(const MainWindow&) = delete;
        MainWindow& operator =(const MainWindow&) = delete;

        void ShowWindow();
        void CloseWindow();

        // 按键的回调函数
        void download_button_callback(sol::object component, sol::object event);
        void upload_button_callback(sol::object component, sol::object event);

    private:
        sol::table download_button_instance_;
        sol::table upload_button_instance_;
        sol::table main_window_instance_;
        sol::table file_editor_instance_;
        sol::table progressbar_instance_;
        sol::table status_label_instance_;
        download_upload::DownloadUploader* download_uploader_;
    };
}
