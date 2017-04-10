#include <map>
#include <regex>
#include <fstream>

#include "modify_window.h"
#include "octane_lua_api.h"
#include "utils.h"
#include "config_file.h"

extern bool cancel_current_modify;

modify_window::MaterialModifyWindow::MaterialModifyWindow(const std::vector<std::string>& materialNames)
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    auto self = octane_lua_api_instance.Self();
    int cols = 2;
    int rows = 0;
    self.create_named_table("labels", "attr", self.create_table_with());
    self.create_named_table("editors", "attr", self.create_table_with());
    self.create_named_table("group_children", "attr", self.create_table_with());

    //获取材质名称列表
    std::vector<std::string> material_names(materialNames);

    /*sol::object scene_graph = octane_lua_api_instance["octane"]["project"]["getSceneGraph"]();
    sol::table all_single_nodes = octane_lua_api_instance["octane"]["nodegraph"]["getOwnedItems"](scene_graph);
    if (all_single_nodes.empty())
    {
        return;
    }

    for (const auto& node: all_single_nodes)
    {
        sol::object node_value = node.second;
        int single_node_pin_num = octane_lua_api_instance["octane"]["node"]["getPinCount"](node_value);
        for (auto i = 0; i < single_node_pin_num; i++)
        {
            sol::table single_node_pin_info = octane_lua_api_instance["octane"]["node"]["getPinInfoIx"](node_value, i+1);
            std::string single_node_pin_name = single_node_pin_info["name"].get<std::string>();
            material_names.push_back(single_node_pin_name);
        }
    }*/

    for (auto i = 0; i < material_names.size(); i++)
    {
        self.create_named_table("label", "attr", self.create_table_with(
            "type", self["octane"]["gui"]["componentType"]["LABEL"],
            "text", material_names[i]));
        sol::table label_instance = self["octane"]["gui"]["create"](self["label"]["attr"]);
        self["labels"]["attr"][i + 1] = label_instance;

        self.create_named_table("editor", "attr", self.create_table_with(
            "type", self["octane"]["gui"]["componentType"]["TEXT_EDITOR"],
            "text", "",
            "x", 20,
            "y", 20,
            "width", 200,
            "height", 20,
            "enable", true));
        sol::table editor_instance = self["octane"]["gui"]["create"](self["editor"]["attr"]);
        self["editors"]["attr"][i + 1] = editor_instance;
        ++rows;
    }

    for (auto i = 0; i < material_names.size(); i++)
    {
        self["group_children"]["attr"][2*i + 1] = self["labels"]["attr"][i + 1];
        self["group_children"]["attr"][2*i + 2] = self["editors"]["attr"][i + 1];
    }

    self.create_named_table("cancel_button", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["BUTTON"],
        "text", "cancel",
        "width", 50,
        "height", 20,
        "enable", true,
        "centre", true));
    cancel_button_instance_ = self["octane"]["gui"]["create"](self["cancel_button"]["attr"]);

    self.create_named_table("ok_button", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["BUTTON"],
        "text", "ok",
        "width", 50,
        "height", 20,
        "enable", true,
        "centre", true));
    ok_button_instance_ = self["octane"]["gui"]["create"](self["ok_button"]["attr"]);

    self["group_children"]["attr"][2*rows + 1] = cancel_button_instance_;
    self["group_children"]["attr"][2*rows + 2] = ok_button_instance_;

    self.create_named_table("modify_widgets_padding", "attr", self.create_table_with(
        1, 5));

    self.create_named_table("widgets_group", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["GROUP"],
        "name", "widget group",
        "children", self["group_children"]["attr"],
        "border", true,
        "rows", rows+1,
        "cols", 2,
        "width", 160,
        "height", 400,
        "text", "",
        "padding", self["modify_widgets_padding"]["attr"],
        "centre", true,
        "x", 40,
        "y", 30));

    self.create_named_table("open_or_close", "attr", self.create_table_with(
        1, true));
    self.create_named_table("captions", "attr", self.create_table_with(
        1, "group"));

    sol::table widgets_group_instance1 = self["octane"]["gui"]["create"](self["widgets_group"]["attr"]);

    self.create_named_table("widgets_group_instance_table", "attr", self.create_table_with(
        1, widgets_group_instance1));

    self.create_named_table("pane_stack", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["PANEL_STACK"],
        "children", self["widgets_group_instance_table"]["attr"],
        "open", self["open_or_close"]["attr"],
        "captions", self["captions"]["attr"],
        "width", 1024,
        "height", 768));

    sol::table panel_stack_instance = self["octane"]["gui"]["create"](self["pane_stack"]["attr"]);

    self.create_named_table("window_panel_table", "attr", self.create_table_with(
        1, panel_stack_instance));

    self.create_named_table("modify_window", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["WINDOW"],
        "children", self["window_panel_table"]["attr"],
        "text", "material modifier",
        "width", 1024,
        "height", 768,
        "centre", true,
        "x", 20,
        "y", 20,
        "enable", true));

    self.set_function("ok_button_callback", [this](sol::object component, sol::object event) {
        ok_button_callback(component, event);
    });

    self.create_named_table("ok_button_callback", "table", self.create_table_with(
        "callback", self["ok_button_callback"]));
    self["octane"]["gui"]["updateProperties"](ok_button_instance_, self["ok_button_callback"]["table"]);

    self.set_function("cancel_button_callback", [this](sol::object component, sol::object event) {
        cancel_button_callback(component, event);
    });

    self.create_named_table("cancel_button_callback", "table", self.create_table_with(
        "callback", self["cancel_button_callback"]));
    self["octane"]["gui"]["updateProperties"](cancel_button_instance_, self["cancel_button_callback"]["table"]);
    

    window_instance_ = self["octane"]["gui"]["create"](self["modify_window"]["attr"]);
}

modify_window::MaterialModifyWindow::~MaterialModifyWindow()
{
    
}

void modify_window::MaterialModifyWindow::ShowWindow()
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    if (!window_instance_.valid())
    {
        return;
    }
    octane_lua_api_instance["octane"]["gui"]["showWindow"](window_instance_);
}

void modify_window::MaterialModifyWindow::CloseWindow()
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    octane_lua_api_instance["octane"]["gui"]["closeWindow"](window_instance_);
}

static bool IsEndWith(const std::string& src, const std::string& test)
{
    size_t src_len = src.length();
    size_t test_len = test.length();
    if (src_len < test_len)
    {
        return false;
    }
    std::string end_str = src.substr(src_len - test_len, src_len - 1);
    if (test == end_str)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void modify_content(const std::string& filePath, std::map<std::string, std::string>& modifyContent)
{
    // 对于备份的旧文件不要对其进行材质名称进行修改了
    std::string backup_file_name = octane_plug_utils::get_file_name_from_fullpath(filePath);
    if (backup_file_name.compare("item_backup.ocs") == 0)
    {
        return;
    }

    if (IsEndWith(filePath, ".ocs") || IsEndWith(filePath, ".obj") || IsEndWith(filePath, ".mtl"))
    {
        // 先将文件全部读入一个vector中
        auto content = std::make_shared<std::vector<std::string>>();
        std::ifstream file_before_modify(filePath);
        std::string line{};

        while (std::getline(file_before_modify, line))
        {
            if (line.empty())
            {
                continue;
            }
            content->push_back(line);
        }
        file_before_modify.close();

        // 依次处理map的每一项目
        for (auto& x : modifyContent)
        {
            std::regex reg(R"(\b)" + x.first + R"(\b)");
            std::string replacement(x.second);
            for (auto& line_ : *content)
            {
                std::string result = std::regex_replace(line_, reg, replacement);
                line_ = result;
            }
        }

        // 将map再写回到文件
        std::ofstream file_after_modify(filePath);
        for (auto& line_ : *content)
        {
            file_after_modify << line_ << "\n";
        }
        file_after_modify.close();
    }
}

static bool is_value_in_map(const std::map<std::string, std::string>& map, const std::string& val)
{
    for (const auto& e: map)
    {
        if (e.second == val)
        {
            return true;
        }
    }
    return false;
}

void modify_window::MaterialModifyWindow::ok_button_callback(sol::object component, sol::object event)
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    config_file::ConfigFile& config_file_instance = config_file::ConfigFile::Get();

    auto self = octane_lua_api_instance.Self();
    sol::table all_editors = self["editors"]["attr"];
    sol::table all_labels = self["labels"]["attr"];

    std::map<std::string, std::string> old_new_pairs{};

    for (const auto& editor: all_editors)
    {
        int index = editor.first.as<int>();
        sol::object editor_value = editor.second;
        sol::table editor_properties = octane_lua_api_instance["octane"]["gui"]["getProperties"](editor_value);
        std::string editor_text = editor_properties["text"].get<std::string>();
        if (editor_text == "")
        {
            continue;
        }
        else if (is_value_in_map(old_new_pairs, editor_text))
        {
            octane_lua_api_instance["octane"]["gui"]["showError"]("Repeat material names!", "Repeat Names");
            return;
        }

        std::string lable_text = all_labels[index]["text"].get<std::string>();
        old_new_pairs.insert_or_assign(lable_text, editor_text);
    }

    if (old_new_pairs.empty())
    {
        //如果没有内容修改直接退出
        CloseWindow();
        return;
    }

    bool read_status;
    std::string last_extract_folder;
    std::tie(last_extract_folder, read_status) = config_file_instance.Read("LastExtractFolderPath");
    if (!read_status)
    {
        octane_lua_api_instance["octane"]["gui"]["showError"]("get last extract folder path error!", "Read Config File Error");
        return;
    }
    octane_plug_utils::ApplyFunctionToFile(last_extract_folder, old_new_pairs, modify_content);
    CloseWindow();

    return;
}

void modify_window::MaterialModifyWindow::cancel_button_callback(sol::object component, sol::object event)
{
    cancel_current_modify = true;
    CloseWindow();
}

