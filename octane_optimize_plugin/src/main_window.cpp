#include "main_window.h"
#include "sol.hpp"
#include "octane_lua_api.h"
#include "utils.h"
#include "config_file.h"
#include "xml_rw.h"
#include "scene_data.h"
#include "modify_window.h"
#include "test_image_window.h"
#include "defer.h"

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
        "width", 200,
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
        "height", 270,
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

    download_uploader_->SetProgressBar(progressbar_instance_);

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
    
    //这里尝试通过Label来通知用户当前程序的状态
    self.create_named_table("program_current_job", "attr", self.create_table_with(
        "text", "start downloading ..."));
    self["octane"]["gui"]["updateProperties"](status_label_instance_, self["program_current_job"]["attr"]);
    self["octane"]["gui"]["dispatchGuiEvents"](500);

    self.create_named_table("download_button", "status", self.create_table_with(
        "enable", false));
    octane_lua_api_instance["octane"]["gui"]["updateProperties"](download_button_instance_, self["download_button"]["status"]);
    sol::table properties = octane_lua_api_instance["octane"]["gui"]["getProperties"](file_editor_instance_);
    std::string model_no = properties["text"].get<std::string>();
    octane_plug_utils::trim(model_no);
    if (download_uploader_->DownloadFileFromOCS(model_no) == common_types::LoadResult::download_failed)
    {
        octane_lua_api_instance["octane"]["gui"]["showError"](download_uploader_->GetError(), "Download File Error");
        self.create_named_table("download_button", "status", self.create_table_with(
            "enable", true));
        octane_lua_api_instance["octane"]["gui"]["updateProperties"](download_button_instance_, self["download_button"]["status"]);
        return;
    }

    self.create_named_table("program_current_job", "attr", self.create_table_with(
        "text", "start extracting ..."));
    self["octane"]["gui"]["updateProperties"](status_label_instance_, self["program_current_job"]["attr"]);
    self["octane"]["gui"]["dispatchGuiEvents"](500);
    auto extract_dir = octane_plug_utils::extract_zip_file(download_uploader_->LastStoredFileFullPathName);
    if (extract_dir.empty())
    {
        octane_lua_api_instance["octane"]["gui"]["showError"]("Extract file error!", "Extract Error");
        CloseWindow();
        return;
    }

    self.create_named_table("program_current_job", "attr", self.create_table_with(
        "text", "octane loading ..."));
    self["octane"]["gui"]["updateProperties"](status_label_instance_, self["program_current_job"]["attr"]);
    self["octane"]["gui"]["dispatchGuiEvents"](1000);

    if (octane_plug_utils::IsDirExist(extract_dir + "\\octane"))
    {
        config_file_instance.Write("LastExtractFolderPath", extract_dir);
        octane_lua_api_instance["octane"]["project"]["load"](extract_dir + "\\octane\\item.ocs");
    }
    else if (octane_plug_utils::IsDirExist(extract_dir + "\\item.ocs"))
    {
        config_file_instance.Write("LastExtractFolderPath", extract_dir);
        octane_lua_api_instance["octane"]["project"]["load"](extract_dir + "\\item.ocs");
    }
    else
    {
        // 弹出错误, 报告目录格式问题
        octane_lua_api_instance["octane"]["gui"]["showError"]("Unknow reason error!", "Error");
    }
    CloseWindow();

    return;
}

void gui::MainWindow::upload_button_callback(sol::object component, sol::object event)
{
    config_file::ConfigFile& config_file_instance = config_file::ConfigFile::Get();
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    auto self = octane_lua_api_instance.Self();

    self.create_named_table("program_current_job", "attr", self.create_table_with(
        "text", "start saving works ..."));
    self["octane"]["gui"]["updateProperties"](status_label_instance_, self["program_current_job"]["attr"]);
    self["octane"]["gui"]["dispatchGuiEvents"](500);

    self.create_named_table("upload_button", "status", self.create_table_with(
        "enable", false));
    octane_lua_api_instance["octane"]["gui"]["updateProperties"](upload_button_instance_, self["upload_button"]["status"]);
    std::string extract_dir = std::get<0>(config_file_instance.Read("LastExtractFolderPath"));


    xml_rw::XMLHandler& xml_handler_instance = xml_rw::XMLHandler::Get();
    xml_handler_instance.Init();
    //获取场景结点数据, 并生成和插入到xml中
    scene_data::get_scene_node_data();

    //对根结点进行特殊的处理, 因为根结点要保存特定的信息
    scene_data::get_root_node_data();


    std::string LastExtractFolder = std::get<0>(config_file_instance.Read("LastExtractFolderPath"));
    std::string octane_full_path = LastExtractFolder;
    if (octane_plug_utils::IsDirContained(LastExtractFolder, "octane"))
    {
        octane_full_path.append("\\octane");
    }
    // 保存最终的信息到文件中
    if (!xml_rw::XMLHandler::Get().SaveFile((octane_full_path + "\\new_item.ocs").c_str()))
    {
        octane_lua_api_instance["octane"]["gui"]["showError"]("save ocs file error!", "Save Error!");
        self.create_named_table("upload_button", "status", self.create_table_with(
            "enable", true));
        octane_lua_api_instance["octane"]["gui"]["updateProperties"](upload_button_instance_, self["upload_button"]["status"]);
        return;
    }

    self.create_named_table("program_current_job", "attr", self.create_table_with(
        "text", "backup old file ..."));
    self["octane"]["gui"]["updateProperties"](status_label_instance_, self["program_current_job"]["attr"]);
    self["octane"]["gui"]["dispatchGuiEvents"](500);

    //备份旧的item.ocs文件
    if (octane_plug_utils::rename_file((octane_full_path + "\\item.ocs").c_str(), (octane_full_path + "\\item_backup.ocs").c_str()) == 0)
    {
        octane_lua_api_instance["octane"]["gui"]["showError"]("backup old item.ocs file error!", "Backup Error!");
        self.create_named_table("upload_button", "status", self.create_table_with(
            "enable", true));
        octane_lua_api_instance["octane"]["gui"]["updateProperties"](upload_button_instance_, self["upload_button"]["status"]);
        return;
    }

    //生成新的item.ocs文件
    if (octane_plug_utils::rename_file((octane_full_path + "\\new_item.ocs").c_str(), (octane_full_path + "\\item.ocs").c_str()) == 0)
    {
        octane_lua_api_instance["octane"]["gui"]["showError"]("rename new_item.ocs to item.ocs file error!", "Backup Error!");
        self.create_named_table("upload_button", "status", self.create_table_with(
            "enable", true));
        octane_lua_api_instance["octane"]["gui"]["updateProperties"](upload_button_instance_, self["upload_button"]["status"]);
        return;
    }

    //修改材质名称
    modify_window::MaterialModifyWindow* modify_window = new modify_window::MaterialModifyWindow();
    modify_window->ShowWindow();
    delete modify_window;
    //这个函数他妈的根本就是设计的失败,服务端的失败
    octane_plug_utils::CreateAndMove(extract_dir, "octane");

    self.create_named_table("program_current_job", "attr", self.create_table_with(
        "text", "compress file ..."));
    self["octane"]["gui"]["updateProperties"](status_label_instance_, self["program_current_job"]["attr"]);
    self["octane"]["gui"]["dispatchGuiEvents"](500);

    std::string zip_file_path = octane_plug_utils::compress_folder(extract_dir, "item.zip");
    if (zip_file_path.empty())
    {
        octane_lua_api_instance["octane"]["gui"]["showError"]("compress files error", "compress error");
        self.create_named_table("upload_button", "status", self.create_table_with(
            "enable", true));
        octane_lua_api_instance["octane"]["gui"]["updateProperties"](upload_button_instance_, self["download_button"]["status"]);
        return;
    }
    auto model_no_wraper = config_file_instance.Read("model_no");
    if (!std::get<1>(model_no_wraper))
    {
        octane_lua_api_instance["octane"]["gui"]["showError"]("get model_no from config file error!", "compress error");
        self.create_named_table("upload_button", "status", self.create_table_with(
            "enable", true));
        octane_lua_api_instance["octane"]["gui"]["updateProperties"](upload_button_instance_, self["download_button"]["status"]);
        return;
    }

    self.create_named_table("program_current_job", "attr", self.create_table_with(
        "text", "file oss uploading ..."));
    self["octane"]["gui"]["updateProperties"](status_label_instance_, self["program_current_job"]["attr"]);
    self["octane"]["gui"]["dispatchGuiEvents"](500);

    std::string model_no = std::get<0>(model_no_wraper);
    std::string current_object_path = model_no + "/octane.zip";
    if (download_uploader_->UploadFileToOCS(model_no, current_object_path, zip_file_path) == common_types::LoadResult::upload_success)
    {
        //表示上传成功, 下面要提醒更新模型
        if (!download_uploader_->InformUpdateModel(model_no))
        {
            octane_lua_api_instance["octane"]["gui"]["showError"]("Inform update model error!", "Inform Update Error");
            self.create_named_table("upload_button", "status", self.create_table_with(
                "enable", true));
            octane_lua_api_instance["octane"]["gui"]["updateProperties"](upload_button_instance_, self["download_button"]["status"]);
            return;
        }

        bool read_status;
        std::string test_image_remote_static_url;
        std::tie(test_image_remote_static_url, read_status) = config_file_instance.Read("test_image_url");

        if (!read_status)
        {
            octane_lua_api_instance["octane"]["gui"]["showError"]("Read test url from config file error!", "Read Test URL Error");
            self.create_named_table("upload_button", "status", self.create_table_with(
                "enable", true));
            octane_lua_api_instance["octane"]["gui"]["updateProperties"](upload_button_instance_, self["download_button"]["status"]);
            return;
        }

        int sleep_count = 0;
        self.create_named_table("program_current_job", "attr", self.create_table_with(
            "text", "waiting test image ..."));
        self["octane"]["gui"]["updateProperties"](status_label_instance_, self["program_current_job"]["attr"]);
        octane_lua_api_instance["octane"]["gui"]["dispatchGuiEvents"](3000);

        std::string test_image_path;
        while ((test_image_path = download_uploader_->GetTestImage(test_image_remote_static_url)).empty())
        {
            sleep_count += 1;
            octane_lua_api_instance["octane"]["gui"]["dispatchGuiEvents"](3000);
            if (sleep_count >= 100)
            {
                break;
            }
        }

        if (test_image_path.empty())
        {
            octane_lua_api_instance["octane"]["gui"]["showError"]("Download test image error!", "Get Test Image Error");
            self.create_named_table("upload_button", "status", self.create_table_with(
                "enable", true));
            octane_lua_api_instance["octane"]["gui"]["updateProperties"](upload_button_instance_, self["download_button"]["status"]);
            self.create_named_table("program_current_job", "attr", self.create_table_with(
                "text", "test image timeout ..."));
            self["octane"]["gui"]["updateProperties"](status_label_instance_, self["program_current_job"]["attr"]);
            self["octane"]["gui"]["dispatchGuiEvents"](500);
            return;
        }

        test_image_window::TestImageWindow* test_image_window = new test_image_window::TestImageWindow(test_image_path, model_no);
        test_image_window->ShowWindow();
        delete test_image_window;
        self.create_named_table("upload_button", "status", self.create_table_with(
            "enable", true));
        octane_lua_api_instance["octane"]["gui"]["updateProperties"](upload_button_instance_, self["download_button"]["status"]);

        self.create_named_table("program_current_job", "attr", self.create_table_with(
            "text", "work finished ..."));
        self["octane"]["gui"]["updateProperties"](status_label_instance_, self["program_current_job"]["attr"]);
        self["octane"]["gui"]["dispatchGuiEvents"](500);

        return;
    }
}
