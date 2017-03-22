#include "main_window.h"
#include "sol.hpp"
#include "octane_lua_api.h"
#include "utils.h"
#include "config_file.h"

//void gui::MainWindow::ShowWindow()
gui::MainWindow::MainWindow()
    : download_uploader_(new download_upload::DownloadUploader())
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    auto self = octane_lua_api_instance.Self();

    self.create_named_table("file_editor", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["TEXT_EDITOR"],
        "text", "input the model id here",
        "x", 20,
        "y", 20,
        "width", 200,
        "height", 20,
        "enable", true));

    //sol::table file_editor_instance = self["octane"]["gui"]["create"](self["file_editor"]["attr"]);
    file_editor_instance_ = self["octane"]["gui"]["create"](self["file_editor"]["attr"]);

    self.create_named_table("download_button", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["BUTTON"],
        "text", "download",
        "width", 80,
        "height", 20,
        "enable", true,
        "centre", true));
    download_button_instance_ = self["octane"]["gui"]["create"](self["download_button"]["attr"]);

    self.create_named_table("upload_button", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["BUTTON"],
        "text", "upload",
        "width", 80,
        "height", 20,
        "enable", true,
        "centre", true));
    upload_button_instance_ = self["octane"]["gui"]["create"](self["upload_button"]["attr"]);

    self.create_named_table("progressbar", "attr", self.create_table_with(
        //"type", self["octane"]["gui"]["componentType"]["PROGRESSBAR"],
        "enable", true,
        "width", 200,
        "height", 20,
        "name", "progressbar_component",
        "progress", 0));
    //sol::object progressbar_instance = self["octane"]["gui"]["createProgressBar"](self["progressbar"]["attr"]);
    progressbar_instance_ = self["octane"]["gui"]["createProgressBar"](self["progressbar"]["attr"]);

    self.create_named_table("text_color", "attr", self.create_table_with(
        1, 255,
        2, 0,
        3, 0,
        4, 255));
    self.create_named_table("status_label", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["LABEL"],
        "text", "",
        "width", 30,
        "height", 30,
        "textColour", self["text_color"]["attr"]));
    //sol::object status_label_instance = self["octane"]["gui"]["create"](self["status_label"]["attr"]);
    status_label_instance_ = self["octane"]["gui"]["create"](self["status_label"]["attr"]);

    self.create_named_table("button_group_final", "attr", self.create_table_with(
        1, download_button_instance_,
        2, upload_button_instance_));

    self.create_named_table("padding", "attr", self.create_table_with(
        1, 10));
    self.create_named_table("button_group", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["GROUP"],
        "name", "button group",
        "children", self["button_group_final"]["attr"],
        "border", true,
        "rows", 1,
        "cols", 2,
        "width", 100,
        "height", 20,
        "text", "",
        "padding", self["padding"]["attr"],
        "centre", true));
    sol::object button_group_instance = self["octane"]["gui"]["create"](self["button_group"]["attr"]);

    self.create_named_table("widgets_group_final", "attr", self.create_table_with(
        1, file_editor_instance_,
        2, progressbar_instance_,
        3, status_label_instance_,
        4, button_group_instance));

    self.create_named_table("padding1", "attr", self.create_table_with(
        1, 5));
    self.create_named_table("widgets_group", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["GROUP"],
        "name", "widgets group",
        "children", self["widgets_group_final"]["attr"],
        "border", true,
        "rows", 4,
        "cols", 1,
        "width", 160,
        "height", 120,
        "text", "",
        "padding", self["padding1"]["attr"],
        "centre", true,
        "x", 40,
        "y", 30));
    sol::object widgets_group_instance = self["octane"]["gui"]["create"](self["widgets_group"]["attr"]);

    self.create_named_table("final", "attr", self.create_table_with(1, widgets_group_instance));
    self.create_named_table("main_window", "attr", self.create_table_with(
        "text", "octane_optimize_plugin",
        "children", self["final"]["attr"],
        "width", 320,
        "height", 240,
        "centre", true,
        "x", 20,
        "y", 20,
        "enable", true));


    self.set_function("download_button_callback", [this](sol::object component, sol::object event) {
        download_button_callback(component, event);
    });
    self.set_function("upload_button_callback", [this](sol::object component, sol::object event) {
        upload_button_callback(component, event);
    });
    self.create_named_table("download_button_callback", "table", self.create_table_with(
        "callback", self["download_button_callback"]));
    self.create_named_table("upload_button_callback", "table", self.create_table_with(
        "callback", self["upload_button_callback"]));
    self["octane"]["gui"]["updateProperties"](download_button_instance_, self["download_button_callback"]["table"]);
    self["octane"]["gui"]["updateProperties"](upload_button_instance_, self["upload_button_callback"]["table"]);

    main_window_instance_ = self["octane"]["gui"]["createWindow"](self["main_window"]["attr"]);
}

gui::MainWindow::~MainWindow()
{
    delete download_uploader_;
}


void gui::MainWindow::ShowWindow()
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    octane_lua_api_instance["octane"]["gui"]["showWindow"](main_window_instance_);
}

void gui::MainWindow::CloseWindow()
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    octane_lua_api_instance["octane"]["gui"]["closeWindow"](main_window_instance_);
}

void gui::MainWindow::download_button_callback(sol::object component, sol::object event)
{
    config_file::ConfigFile& config_file_instance = config_file::ConfigFile::Get();
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    auto self = octane_lua_api_instance.Self();
    self.create_named_table("download_button", "status", self.create_table_with(
        "enable", false));
    octane_lua_api_instance["octane"]["gui"]["updateProperties"](download_button_instance_ ,self["download_button"]["status"]);
    sol::table properties = octane_lua_api_instance["octane"]["gui"]["getProperties"](file_editor_instance_);
    std::string model_no = properties["text"].get<std::string>();
    octane_plug_utils::trim(model_no);
    if (download_uploader_->DownloadFileFromOCS(model_no) == common_types::LoadResult::download_failed)
    {
        octane_lua_api_instance["octane"]["gui"]["showError"](download_uploader_->GetError(), "Download File Error");
        self.create_named_table("download_button", "status", self.create_table_with(
            "enable", true));
        //octane_lua_api_instance["octane"]["gui"]["updateProperties"](download_button_instance_ ,self["download_button"]["status"]);
        CloseWindow();
    }

    auto extract_dir = octane_plug_utils::extract_zip_file(download_uploader_->LastStoredFileFullPathName);
    if (extract_dir.empty())
    {
        octane_lua_api_instance["octane"]["gui"]["showError"]("Extract file error!", "Extract Error");
    }
    if (octane_plug_utils::IsDirExist(extract_dir + "\\octane"))
    {
        config_file_instance.Write("LastExtractFolderPath", extract_dir);
        octane_lua_api_instance["octane"]["project"]["load"](extract_dir + "\\octane\\item.ocs");
        CloseWindow();
    }
    else if (octane_plug_utils::IsDirExist(extract_dir + "\\item.ocs"))
    {
        config_file_instance.Write("LastExtractFolderPath", extract_dir);
        octane_lua_api_instance["octane"]["project"]["load"](extract_dir + "\\item.ocs");
        CloseWindow();
    }
    else
    {
        // 弹出错误, 报告目录格式问题
        octane_lua_api_instance["octane"]["gui"]["showError"]("Unknow reason error!", "Error");
    }
}

void gui::MainWindow::upload_button_callback(sol::object component, sol::object event)
{

}
