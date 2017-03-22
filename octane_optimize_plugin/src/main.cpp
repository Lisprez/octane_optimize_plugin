// 日志配置
#define ELPP_NO_DEFAULT_LOG_FILE
#define ELPP_THREAD_SAFE
#define SOL_CHECK_ARGUMENTS

#include <stdio.h>

#include "sol.hpp"
#include "utils.h"
#include "octane_lua_api.h"
#include "xml_rw.h"
#include "scene_data.h"
#include "config_file.h"
#include "download_uploader.h"

#include "easylogging++.h"
#include "main_window.h"

INITIALIZE_EASYLOGGINGPP

constexpr char* config_file_name = "config_test.cfg";

void Easylogging_Setup()
{
    //加载easylogging配置文件
    el::Configurations conf("easylogging++_conf.txt");
    el::Loggers::reconfigureAllLoggers(conf);
    el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);
    el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
}

static sol::state_view* octane_lua_container{ nullptr };

int test_function()
{
    LOG(INFO) << "fuck the refactor of octane plugin!" << std::endl;
    return 0;
}

extern "C" __declspec(dllexport) int luaopen_octane_optimize_plugin(lua_State* L)
{
    using octane_plug_utils::CheckType;
    using octane_plug_utils::TraverseTable;
    using octane_plug_utils::pair_print;

    // 创建用于保存日志的目录
    octane_plug_utils::CreateFolder("log");

    // 初始化日志
    Easylogging_Setup();

    // 这个octane_lua_container就是对octane lua环境的引用
    octane_lua_container = new sol::state_view(L);
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    octane_lua_api_instance.Setup(octane_lua_container);

    xml_rw::XMLHandler& xml_handler_instance = xml_rw::XMLHandler::Get();
    xml_handler_instance.Init();
    //获取场景结点数据, 并生成和插入到xml中
    scene_data::get_scene_node_data();

    //对根结点进行特殊的处理, 因为根结点要保存特定的信息
    scene_data::get_root_node_data();

    // 保存最终的信息到文件中
    xml_rw::XMLHandler::Get().SaveFile("fuck_test_save_file.ocs");
    config_file::ConfigFile& config_file_instance = config_file::ConfigFile::Get();
    config_file_instance.Setup(config_file_name);
    auto download_uploader_instance = new download_upload::DownloadUploader();

    

#if 0
    if (download_uploader_instance->DownloadFileFromOCS("846675380d4911e7b9fa00163e1284d1") == common_types::LoadResult::download_failed)
    {
    //报告下载错误
    //download_uploader_instance->error_message_
        return 0;
    }

    auto extract_dir = octane_plug_utils::extract_zip_file(download_uploader_instance->LastStoredFileFullPathName);
    if (extract_dir.empty())
    {
    //解压失败
    }
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
    }
#endif
#if 0
    std::string extract_dir1 = std::get<0>(config_file_instance.Read("LastExtractFolderPath"));
    //这个函数他妈的根本就是设计的失败,服务端的失败
    octane_plug_utils::CreateAndMove(extract_dir1, "octane");
    std::string zip_file_path = octane_plug_utils::compress_folder(extract_dir1, "item.zip");
    if (zip_file_path.empty())
    {
    //报告错误
    }
    download_uploader_instance->UploadFileToOCS("846675380d4911e7b9fa00163e1284d1", "846675380d4911e7b9fa00163e1284d1/octane.zip", zip_file_path);
#endif
    gui::MainWindow* main_window = new gui::MainWindow();
    main_window->ShowWindow();

    return 0;
}