#include "test_image_window.h"
#include "octane_lua_api.h"

test_image_window::TestImageWindow::TestImageWindow(const std::string& testImagePath, const std::string& model_no)
  : download_uploader_(new download_upload::DownloadUploader()),
    model_no_(model_no)   
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    auto self = octane_lua_api_instance.Self();
    sol::table image_object;
    try
    {
        image_object = self["octane"]["image"]["load"](testImagePath, 0);
    }
    catch(std::exception& ex)
    {
        
    }

    self.create_named_table("group_children", "attr", self.create_table_with());

    self.create_named_table("image", "attr", self.create_table_with(
        "width", 512,
        "height", 512,
        "type", 1,
        "image", image_object));

    sol::table image_instance = self["octane"]["gui"]["create"](self["image"]["attr"]);

    self.create_named_table("give_up_button", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["BUTTON"],
        "text", "Give up",
        "width", 50,
        "height", 20,
        "enable", true,
        "centre", true));
    give_up_button_instance_ = self["octane"]["gui"]["create"](self["give_up_button"]["attr"]);

    self.create_named_table("confirm_button", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["BUTTON"],
        "text", "Confirm",
        "width", 50,
        "height", 20,
        "enable", true,
        "centre", true));
    confirm_button_instance_ = self["octane"]["gui"]["create"](self["confirm_button"]["attr"]);

    self.create_named_table("test_button_group", "attr", self.create_table_with(
        1, give_up_button_instance_,
        2, confirm_button_instance_));

    self.create_named_table("button_group_padding", "attr", self.create_table_with(
        1, 10));

    self.create_named_table("test_window_button_group", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["GROUP"],
        "name", "button_group",
        "children", self["test_button_group"]["attr"],
        "border", true,
        "rows", 1,
        "cols", 2,
        "width", 100,
        "height", 20,
        "text", "",
        "padding", self["button_group_padding"]["attr"],
        "centre", true));
    sol::table test_window_button_group_instance = self["octane"]["gui"]["create"](self["test_window_button_group"]["attr"]);
    
    self["group_children"]["attr"][1] = image_instance;
    self["group_children"]["attr"][2] = test_window_button_group_instance;

    self.create_named_table("widgets_group_padding", "attr", self.create_table_with(
        1, 5));
    self.create_named_table("widgets_group", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["GROUP"],
        "name", "widgets group",
        "children", self["group_children"]["attr"],
        "border", true,
        "rows", 2,
        "cols", 1,
        "width", 160,
        "height", 400,
        "text", "",
        "padding", self["widgets_group_padding"]["attr"],
        "centre", true,
        "x", 40,
        "y", 30));

    sol::table widgets_group_instance = self["octane"]["gui"]["create"](self["widgets_group"]["attr"]);
    self.create_named_table("widgets_group_instance_group", "attr", self.create_table_with(
        1, widgets_group_instance));

    self.create_named_table("test_image_window", "attr", self.create_table_with(
        "type", self["octane"]["gui"]["componentType"]["WINDOW"],
        "text", "test image window",
        "children", self["widgets_group_instance_group"]["attr"],
        "width", 1024,
        "height", 768,
        "centre", true,
        "x", 20,
        "y", 20,
        "enable", true));

    self.set_function("confirm_button_callback", [this](sol::object component, sol::object event) {
        confirm_button_callback(component, event);
    });
    self.create_named_table("confirm_button_callback", "table", self.create_table_with(
        "callback", self["confirm_button_callback"]));

    self.set_function("give_up_button_callback", [this](sol::object component, sol::object event) {
        give_up_button_callback(component, event);
    });

    self["octane"]["gui"]["updateProperties"](confirm_button_instance_, self["confirm_button_callback"]["table"]);
    self["octane"]["gui"]["updateProperties"](give_up_button_instance_, self["give_up_button_callback"]["table"]);
    window_instance_ = self["octane"]["gui"]["create"](self["test_image_window"]["attr"]);
}

test_image_window::TestImageWindow::~TestImageWindow()
{
    delete download_uploader_;
}

void test_image_window::TestImageWindow::ShowWindow()
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    octane_lua_api_instance["octane"]["gui"]["showWindow"](window_instance_);
}

void test_image_window::TestImageWindow::CloseWindow()
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    octane_lua_api_instance["octane"]["gui"]["closeWindow"](window_instance_);
}

void test_image_window::TestImageWindow::confirm_button_callback(sol::object component, sol::object event)
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    if (!download_uploader_->InformCreateTask(model_no_))
    {
        octane_lua_api_instance["octane"]["gui"]["showError"]("Inform create task error!", "Inform Create Error");
        return;
    }
    return;
}

void test_image_window::TestImageWindow::give_up_button_callback(sol::object component, sol::object event)
{
    octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
    octane_lua_api_instance["octane"]["gui"]["showError"]("Give up your midification!", "Give Up");
    CloseWindow();
}
