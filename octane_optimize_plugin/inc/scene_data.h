#pragma once

#include <vector>
#include <tuple>

#include "octane_lua_api.h"
#include "xml_rw.h"
#include "utils.h"
#include "config_file.h"

std::vector<std::string> material_names{};

namespace scene_data {
	static int NODE_ID = 200;
	static long FAKE_PIN_ID = 10000;

	static std::vector<std::tuple<std::map<std::string, std::string>, std::string>>
		get_root_node_attributes_data(sol::object node_lua, std::vector<std::string>& attribute_names);

	static std::tuple<std::map<std::string, std::string>, sol::object, bool, int> get_pin_node_data(sol::object father_node, int i);

	static void insert_pin(const std::string& path, sol::object father_node, std::pair<std::string, std::string>* diff_pair, int pin_index);

	static std::vector<std::tuple<std::map<std::string, std::string>, std::string>> get_attribute_node_data(sol::object father_node, const std::string& node_type);

	static void get_scene_node_data()
	{
		octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
		xml_rw::XMLHandler& xml_handler = xml_rw::XMLHandler::Get();

		sol::object sceneGraph = octane_lua_api_instance["octane"]["project"]["getSceneGraph"]();
		sol::table sceneGraphProperties = octane_lua_api_instance["octane"]["nodegraph"]["getProperties"](sceneGraph);
		std::map<std::string, std::string> scene_node_attributes{};
		scene_node_attributes.insert({ "id", std::to_string(NODE_ID++) });
		scene_node_attributes.insert({ "type", std::to_string(sceneGraphProperties["type"].get<int>()) });
		scene_node_attributes.insert({ "name", sceneGraphProperties["name"] });

		std::string time_stamp_format("%Y-%m-%d %H:%M:%S");
		scene_node_attributes["timestamp"] = octane_plug_utils::get_time_stamp(time_stamp_format);
		xml_handler.InsertRootNode("OCS2", "graph", nullptr, scene_node_attributes);
	}

	static void get_root_node_data()
	{
		auto& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
		auto& xml_handler_instance = xml_rw::XMLHandler::Get();
        config_file::ConfigFile& config_file_instance = config_file::ConfigFile::Get();

		sol::object sceneGraph = octane_lua_api_instance["octane"]["project"]["getSceneGraph"]();
		if (octane_plug_utils::CheckType(sceneGraph, sol::type::userdata))
		{
            bool read_status;
            std::string model_no;
            std::tie(model_no, read_status) = config_file_instance.Read("model_no");
            if (!read_status)
            {
                return;
            }
			sol::table rootNodes = octane_lua_api_instance["octane"]["nodegraph"]["findItemsByName"](sceneGraph, model_no, false);
			if (!octane_plug_utils::CheckType(rootNodes, sol::type::table))
			{
				LOG(INFO) << "Error with findItemsByName";
				return;
			}

			if (rootNodes.empty())
			{
				LOG(INFO) << "findItemsByName return empty table";
				return;
			}
			sol::table root_node_attributes_lua = octane_lua_api_instance["octane"]["node"]["getProperties"](rootNodes[1]);
			std::map<std::string, std::string> root_node_attributes_xml{};
			root_node_attributes_xml.insert({ "id", std::to_string(NODE_ID++) });
			root_node_attributes_xml.insert({ "type", std::to_string(root_node_attributes_lua["type"].get<int>()) });
			root_node_attributes_xml.insert({ "name", root_node_attributes_lua["name"] });
			std::string position{};
			position += std::to_string(root_node_attributes_lua["position"][1].get<int>());
			position += " ";
			position += std::to_string(root_node_attributes_lua["position"][2].get<int>());
			root_node_attributes_xml.insert({ "position", position });

			std::string root_node = xml_handler_instance.InsertRootNode("OCS2/graph", "node", nullptr, root_node_attributes_xml);
			// 获取root_node下面的所有属性结点并插入
			std::vector<std::string> all_attribute_names{"objectLayerImport", "scaleUnitType",
				"useSuppliedVertexNormals", "maxSmoothAngle", "defaultHairThickness",
				"subdLevel", "subdLevel", "subdBoundInterp", "subdFaceVaryingBoundInterp",
				"subdFaceVaryingPropagateCorners", "subdScheme", "windingOrder", "useSuppliedSmoothGroupsAsBoundaries",
				"importMtlMaterials", "diffuse", "glossy", "specular", "importImageTextures", "textureDiffuse",
				"textureDiffuseAsFloatImage", "textureSpecular", "textureSpecularAsFloatImage", "textureRoughness",
				"textureRoughnessAsFloatImage", "textureBump", "textureBumpAsFloatImage", "textureBumpUseScale",
				"textureOpacity", "textureOpacityAsFloatImage", "textureAutoAlphaImage", "glossySpecularScale",
				"textureOpacityValueInvert", "textureOpacityInvert", "rgbColorSpaceId"
			};
			auto all_root_attribute_units = get_root_node_attributes_data(rootNodes[1], all_attribute_names);
			for (const auto& root_attribute_unit : all_root_attribute_units)
			{
				auto attribute_head = std::get<0>(root_attribute_unit);
				auto pcdata = std::get<1>(root_attribute_unit);
				xml_handler_instance.InsertAttributeNode(root_node, nullptr, attribute_head, pcdata);
			}
            std::map<std::string, std::string> special_obj_attribute_node{{"name", "filename"}, {"type", "11"}};
            xml_handler_instance.InsertAttributeNode(root_node, nullptr, special_obj_attribute_node, "item.obj");

			//获取当前node的pin的数量
			//遍历处理每一个pin, get_pin_node_data(pin)
			int pin_num = octane_lua_api_instance["octane"]["node"]["getPinCount"](rootNodes[1]);
			for (int i = 0; i < pin_num; i++)
			{
				insert_pin("OCS2/graph/node", rootNodes[1], nullptr, i+1);
			}
		}
	}

	/**
	 * 将一个lua值转换成对应于cpp这边的字符串表示, 比如lua里面的12到c++里的"12"
	 * 本函数只处理基本类型, 对于如userdata, table, function之类的复杂类型都返回一个空字符串
	 */
	static std::string lua_privite_data_to_c_string(sol::object v)
	{
		if (octane_plug_utils::CheckType(v, sol::type::boolean))
		{
			return std::to_string(v.as<bool>());
		}
		else if (octane_plug_utils::CheckType(v, sol::type::number))
		{
			return std::to_string(v.as<double>());
		}
		else if (octane_plug_utils::CheckType(v, sol::type::string))
		{
			return v.as<std::string>();
		}
		else if (octane_plug_utils::CheckType(v, sol::type::nil))
		{
			return "";
		}
		else
		{
			//当是复杂类型的时候, 直接返回""
			return "";
		}
	}

	/**
	 * 将lua值平整到C++中的相应的表示
     *
	 * @param attribute_type 属性值的类型, 比如type="4"就要进行特殊处理
	 * @param value 要平整的lua值
	 * 
	 * @return std::string 返回对就lua值的在C++中的字符串表示
	 */
	static std::string flat_value(const std::string& attribute_type, const sol::object& value)
	{
		std::string result{};
		if (octane_plug_utils::CheckType(value, sol::type::table))
		{
			sol::table inner_data = value;
			for (const auto& pair : inner_data)
			{
				result += lua_privite_data_to_c_string(pair.second);
				result += " ";
			}
		}
		else
		{
			result = lua_privite_data_to_c_string(value);
			if (attribute_type == "4")
			{
				result += " 0 0";
			}
		}

		return result;
	}

	/**
	 * 根据给定的属性名列表获取根结点中对应的属性值的列表, 比如:scaleUnitType对应获取({"scaleUnitType", "2"}, 2)
	 */
	static std::vector<std::tuple<std::map<std::string, std::string>, std::string>>
		get_root_node_attributes_data(sol::object node_lua, std::vector<std::string>& attribute_names)
	{
		xml_rw::XMLHandler& xml_handler_instance = xml_rw::XMLHandler::Get();
		octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();

		static std::vector<std::tuple<std::map<std::string, std::string>, std::string>> all_root_attribute_unit{};
		for (const auto& attribute_name : attribute_names)
		{
			sol::table attribute_info = octane_lua_api_instance["octane"]["node"]["getAttributeInfo"](node_lua, attribute_name);
			if (attribute_info.empty())
			{
				LOG(ERROR) << "getAttributeInfo of " << attribute_name;

				continue;
			}
			std::string attribute_type = std::to_string(attribute_info["type"].get<int>());
			sol::object attribute_value_lua = octane_lua_api_instance["octane"]["node"]["getAttribute"](node_lua, attribute_name);
			std::string attribute_value = flat_value(attribute_type, attribute_value_lua);
			std::map<std::string, std::string> attribute_head{ {"name", attribute_name}, {"type", attribute_type} };
			std::tuple<std::map<std::string, std::string>, std::string> attribute_unit{ attribute_head, attribute_value };
			all_root_attribute_unit.push_back(attribute_unit);
		}
		return all_root_attribute_unit;
	}

	/**
	 * 获取非根结点的一般node结点的属性
	 * 
	 * @param common_node node在lua中的结点实体表示
	 * @param node_id 通过全局变量指定的node的id
	 * @param node_is_pin_owned 指示这个node是独立结点, 还是被pin持有的结点
	 * 
	 * @return std::map<std::string, std::string> 返回非根结点的结点的属性
	 */
	static std::map<std::string, std::string> get_common_node_attribute_data(sol::object common_node, const std::string& node_id, bool node_is_pin_owned)
	{
		octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
		sol::table common_node_attributes_lua = octane_lua_api_instance["octane"]["node"]["getProperties"](common_node);
		std::map<std::string, std::string> common_node_attributes_xml{};
		common_node_attributes_xml.insert({ "id", std::to_string(NODE_ID++) });
		common_node_attributes_xml.insert({ "type", std::to_string(common_node_attributes_lua["type"].get<int>()) });
		common_node_attributes_xml.insert({ "name", common_node_attributes_lua["name"] });
		std::string position{};
		double position_x = common_node_attributes_lua["position"][1].get<double>();
		double position_y = common_node_attributes_lua["position"][2].get<double>();
		if (!node_is_pin_owned)
		{
			position += std::to_string(position_x);
			position += " ";
			position += std::to_string(position_y);
			common_node_attributes_xml.insert({ "position", position });
		}
		return common_node_attributes_xml;
	}

	/**
	 * 获取pin结点的属性值
	 * 
	 * @param father_node 这个pin所在的父结点在Lua的实体表示
	 * @param pin_index 这个pin的索引, 比如2表示这个pin在父结点上的编号是2, 编号是从1开始的
	 * 
	 * @return 返回({"属性名", "属性值"}, 父结点, 是否是dynamic, pin连接的node的id)
	 */
	static std::tuple<std::map<std::string, std::string>, sol::object, bool, int> get_pin_node_data(sol::object father_node, int pin_index)
	{
		octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
		sol::table pin_info = octane_lua_api_instance["octane"]["node"]["getPinInfoIx"](father_node, pin_index);
		std::map<std::string, std::string> pin_attributes{};
		std::string pin_name = pin_info["name"].get<std::string>();

		//获取pin的类型用于收集是否是一个材质结点
		int pin_type = pin_info["type"].get<int>();
		if (pin_type == 7) // 表示连接的是材质结点
		{
			material_names.push_back(pin_name);
		}
		
		pin_attributes.insert({ "name", pin_name });
		if (pin_info["isDynamic"].get<bool>())
		{
			pin_attributes.insert({ "dynamicType", std::to_string(pin_info["type"].get<int>()) });
		}
		int pin_id = pin_info["id"].get<int>();
		sol::object pin_connected_node = octane_lua_api_instance["octane"]["node"]["getInputNodeIx"](father_node, pin_index);
		if (!pin_connected_node.valid())
		{
			std::tuple<std::map<std::string, std::string>, sol::object, bool, int> result{};
			return result;
		}
		sol::table pin_connected_node_properties = octane_lua_api_instance["octane"]["node"]["getProperties"](pin_connected_node);
		int pin_connected_node_id = 0;
		bool node_is_owned = false;
		if (!pin_connected_node_properties["pinOwned"].get<bool>())
		{
			pin_connected_node_id = ++NODE_ID;
			pin_attributes.insert({ "connect", std::to_string(pin_connected_node_id) });
		}
		else
		{
			pin_connected_node_id = ++NODE_ID;
			node_is_owned = true;
		}
		std::tuple<std::map<std::string, std::string>, sol::object, bool, int> result{ pin_attributes, pin_connected_node, node_is_owned, pin_connected_node_id };
		return result;
	}

	/**
	 * 插入一个普通结点到xml中去
	 * 
	 * @param path 被插入位置的父结点的路径
	 * @param diff_pair 唯一确定父结点的键值对
	 * @param common_node_data 普通结点的属性值
	 * 
	 * @return std::string 返回新插入结点的路径
	 */
	static std::string insert_common_node(const std::string& path, const std::pair<std::string, std::string>* diff_pair, const std::map<std::string, std::string>& common_node_data)
	{
		xml_rw::XMLHandler& xml_handler_instance = xml_rw::XMLHandler::Get();
		return xml_handler_instance.InsertRootNode(path, "node", diff_pair, common_node_data);
	}

	/**
	 * 将一个pin插入到xml中
	 * 
	 * @param path 这个pin插入的路径
	 * @param father_node 这个pin的父结点
	 * @param pin_index 这个pin在父结点中的编号
	 * 
	 * @return 函数无返回值
	 */
	static void insert_pin(const std::string& path, sol::object father_node, std::pair<std::string, std::string>* diff_pair, int pin_index)
	{
		xml_rw::XMLHandler& xml_handler_instance = xml_rw::XMLHandler::Get();
		octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();

		auto pin_node_data = get_pin_node_data(father_node, pin_index);
		// 表示pin内嵌结点
		if (std::get<0>(pin_node_data).empty())
		{
			return;
		}
		auto pin_attribute_data = std::get<0>(pin_node_data);
		pin_attribute_data.insert({ "fake_pin_id", std::to_string(++FAKE_PIN_ID) });
		auto pin_connected_node = std::get<1>(pin_node_data);
		bool pin_connected_node_is_owned = std::get<2>(pin_node_data);
		auto pin_connected_node_id = std::to_string(std::get<3>(pin_node_data));

		auto pin_node_path = xml_handler_instance.InsertPin(path, diff_pair, pin_attribute_data);
		std::pair<std::string, std::string> pin_diff_pair{ "fake_pin_id", pin_attribute_data["fake_pin_id"] };
		auto pin_connected_node_data = get_common_node_attribute_data(pin_connected_node, pin_connected_node_id, pin_connected_node_is_owned);

		std::string common_node_path{};

		if (!pin_connected_node_is_owned)
		{
			common_node_path = insert_common_node("OCS2/graph", nullptr, pin_connected_node_data);
		}
		else 
		{
			common_node_path = insert_common_node(pin_node_path, &pin_diff_pair, pin_connected_node_data);
		}
		std::pair<std::string, std::string> node_diff_pair("id", pin_connected_node_id);
		auto attribute_node_datas = get_attribute_node_data(pin_connected_node, pin_connected_node_data["type"]);
		for (const auto& attribute_unit : attribute_node_datas)
		{
			auto attribute_node_attribute = std::get<0>(attribute_unit);
			auto attribute_node_pcdata = std::get<1>(attribute_unit);

			xml_handler_instance.InsertAttributeNode(common_node_path, &node_diff_pair, attribute_node_attribute, attribute_node_pcdata);
		}

		int pin_num = octane_lua_api_instance["octane"]["node"]["getPinCount"](pin_connected_node);
		for (int i = 0; i < pin_num; i++)
		{
			insert_pin(common_node_path, pin_connected_node, &node_diff_pair, i+1);
		}
	}

    static bool is_file_belong_to(const std::string& fullPathFileName, const std::string& fullPathDirName)
	{
        std::string parent_path = octane_plug_utils::get_parent_path(fullPathFileName);
        if (parent_path.compare(fullPathDirName) == 0) //表示文件的父目录和期望目录相同, 也就是文件在所指目录中
        {
            return true;
        }
        return false;
	}

    /**
     * 这个函数的作用是从maps目录中找到图片序号最大的数值
     * 
     * @param maps_folder maps目录的绝对路径
     * 
     * @return int
     */
    static int get_image_index_max(const std::string& maps_folder)
	{
        std::vector<std::string> file_name_lists{};
        octane_plug_utils::GetAllFilesPath(maps_folder, file_name_lists);
        int max_order = 0;
        for (const auto& full_path_file_name: file_name_lists)
        {
            std::string file_name = octane_plug_utils::get_file_name_from_fullpath(full_path_file_name);
            std::string ext_name = octane_plug_utils::get_file_ext_with_dot(file_name);

            size_t index = file_name.find_last_of('_');
            if (index == std::string::npos)
            {
                continue;
            }
            int order = std::stoi(file_name.substr(index + 1, file_name.length()-ext_name.length()));
            if (order > max_order)
            {
                max_order = order;
            }
        }

        return max_order;
	}

    /**
     * 修改贴图的路径
     * 
     * @param imageFullPathName 贴图的绝对路径
     * 
     * @return std::string 返回贴图的相对路径用于填写到ocs文件中
     */
    static std::string change_image_file_path(const std::string& imageFullPathName)
	{
        octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
        config_file::ConfigFile& config_file_instance = config_file::ConfigFile::Get();
        bool read_status;
        std::string last_extract_folder;
        std::tie(last_extract_folder, read_status) = config_file_instance.Read("LastExtractFolderPath");
        if (!read_status)
        {
            octane_lua_api_instance["octane"]["gui"]["showError"]("Read last extract folder error!", "Read Config Error");
        }
        std::string map_folder1 = last_extract_folder + "\\maps"; //之所以存在这个是因为服务器失败的设计
        std::string map_folder2 = last_extract_folder + "\\octane\\maps";
        std::string map_folder;
        if (octane_plug_utils::IsDirExist(map_folder2))
        {
            map_folder = map_folder2;
        }
        else if (octane_plug_utils::IsDirExist(map_folder1))
        {
            map_folder = map_folder1;
        }
        else
        {
            map_folder = map_folder2;
            octane_plug_utils::CreateFolder(map_folder2);
        }

        if (is_file_belong_to(imageFullPathName, map_folder))
        {
            std::string temp("maps\\");
            return temp + octane_plug_utils::get_file_name_from_fullpath(imageFullPathName);
        }
        int order = get_image_index_max(map_folder) + 1;
        std::string ext_name = octane_plug_utils::get_file_ext_with_dot(imageFullPathName);
        std::string file_name;
        file_name.append("image_").append(std::to_string(order)).append(ext_name);
        std::string new_image_fullname = map_folder + "\\" + file_name;
        if (octane_plug_utils::convert_image(imageFullPathName, new_image_fullname) == 0)
        {
            std::string temp("maps\\");
            temp.append(file_name);
            return temp;
        }
        return "";
	}

	/**
	 * 获取一个普通结点的所有属性结点的数据
	 * 
	 * @param father_node 表示一个普通结点
	 * @param node_type 表示这个普通结点的类型
	 * 
	 * @return 返回{ ({"属性名", "属性值"}, pcdata) }
	 */
	static std::vector<std::tuple<std::map<std::string, std::string>, std::string>> get_attribute_node_data(sol::object father_node, const std::string& node_type)
	{
		std::vector<std::tuple<std::map<std::string, std::string>, std::string>> result;

		octane_lua_api::OCtaneLuaAPI& octane_lua_api_instance = octane_lua_api::OCtaneLuaAPI::Get();
		int attribte_num = octane_lua_api_instance["octane"]["node"]["getAttributeCount"](father_node);
		std::map<std::string, std::string> attribute_node_attribute{};

		// 表示结点是一个图片结点, 当是图片结点时只读取一个属性:filename
		if (node_type == "34" || node_type == "35" || node_type == "36")
		{
			std::string filename = octane_lua_api_instance["octane"]["node"]["getAttribute"](father_node, "filename");
			attribute_node_attribute.insert({ "name", "filename" });
			attribute_node_attribute.insert({ "type", "11" });

            //修改图片文件路径
            filename = change_image_file_path(filename);
			std::tuple<std::map<std::string, std::string>, std::string> attribute_unit(attribute_node_attribute, filename);
			result.push_back(attribute_unit);
		}
		else 
		{
			for (int i = 0; i < attribte_num; i++)
			{
				sol::table attribute_node_attribute_info = octane_lua_api_instance["octane"]["node"]["getAttributeInfoIx"](father_node, i + 1);
				attribute_node_attribute["type"] = std::to_string(attribute_node_attribute_info["type"].get<int>());
				int attribute_node_attribute_id = attribute_node_attribute_info["id"].get<int>();
				std::string attribute_node_attribute_name = octane_lua_api_instance["octane"]["apiinfo"]["getAttributeName"](attribute_node_attribute_id);
				attribute_node_attribute.insert({ "name", attribute_node_attribute_name });
				sol::object attribute_node_attribute_value_lua = octane_lua_api_instance["octane"]["node"]["getAttributeIx"](father_node, i + 1);
				std::string attribute_node_attribute_value_xml = flat_value(attribute_node_attribute["type"], attribute_node_attribute_value_lua);
				std::tuple<std::map<std::string, std::string>, std::string> attribute_unit(attribute_node_attribute, attribute_node_attribute_value_xml);
				result.push_back(attribute_unit);
				attribute_node_attribute.clear();
			}
		}

		return result;
	}
}