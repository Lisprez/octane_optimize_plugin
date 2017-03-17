#pragma once

#include <functional>
#include <string>
#include <ostream>
#include <sstream>
#include <ctime>
#include <chrono>

#include "sol.hpp"
#include "pugixml.hpp"
#include "pugiconfig.hpp"
#include "octane_lua_api.h"
#include "easylogging++.h"

constexpr int BUFSIZE = 1024;

namespace octane_plug_utils {

	inline void build_string(std::ostream& o) {}

	template<class First, class... Rest>
	inline void build_string(std::ostream& o,
		const First& value,
		const Rest&... rest)
	{
		o << value;
		build_string(o, rest...);
	}

	template<class... T>
	std::string concat_string(const T&... value)
	{
		std::ostringstream o;
		build_string(o, value...);
		return o.str();
	}

	// 声明以便定义其后的函数调用
	static std::string pair_print(sol::object key, sol::object value);

	static bool CheckType(sol::object v, sol::type target_type)
	{
		if (v.get_type() == target_type)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	static std::string GetTypeAsString(sol::object v)
	{
		switch (v.get_type())
		{
		case sol::type::table:
			return "table";
		case sol::type::function:
			return "function";
		case sol::type::userdata:
			return "userdata";
		case sol::type::boolean:
			return "boolean";
		case sol::type::lightuserdata:
			return "lightuserdata";
		case sol::type::nil:
			return "nil";
		case sol::type::none:
			return "none";
		case sol::type::string:
			return "string";
		case sol::type::number:
			return "number";
		case sol::type::thread:
			return "thread";
		default:
			return "unknow";
		}
	}

	static std::string TraverseTable(sol::table t, std::function<std::string(sol::object, sol::object)> pair_processor)
	{
		if (!t.valid() || t.empty())
		{
			return "";
		}
		
		std::string all_in_string{};

		for (auto& kv: t)
		{
			sol::object key = kv.first;
			sol::object value = kv.second;
			all_in_string += pair_processor(key, value);
			all_in_string += "\n";
		}

		return all_in_string;
	}

	static std::string flat_print(sol::object v)
	{
		std::string format_result{};

		sol::type v_type = v.get_type();
		switch (v_type)
		{
		case sol::type::boolean:
			format_result += concat_string<bool>(v.as<bool>());
			return format_result;
		case sol::type::number:
			format_result += concat_string<double>(v.as<double>());
			return format_result;
		case sol::type::string:
			format_result += concat_string<std::string>(v.as<std::string>());
			return format_result;
		case sol::type::function:
			format_result += "function";
			return format_result;
		case sol::type::table:
			format_result += "\n\t";
			format_result += TraverseTable(v, pair_print);
			return format_result;
		case sol::type::userdata:
			format_result += "userdata";
			return format_result;
		case sol::type::nil:
			format_result += "nil";
			return format_result;
		case sol::type::thread:
			format_result += "thread";
			return format_result;
		default:
			format_result += "unknow_value";
			return format_result;
		}
	}

	static std::string pair_print(sol::object key, sol::object value)
	{
		std::string key_value_string{};
		key_value_string += "[";
		key_value_string += flat_print(key);
		key_value_string += " => ";
		key_value_string += flat_print(value);
		key_value_string += "]";
		return key_value_string;
	}

	static std::string get_time_stamp(const std::string& timeFormatString)
	{
		std::array<char, 40> buf{};
		time_t raw_time;
		struct tm* time_info = nullptr;
		time(&raw_time);
		time_info = localtime(&raw_time);
		size_t len = strftime(buf.data(), buf.size(), timeFormatString.c_str(), time_info);
		if (len == 0)
		{
			return "";
		}
		else
		{
			return std::string(std::begin(buf), std::end(buf));
		}
	}

    static bool IsDirExist(const std::string& directoryName)
    {
        DWORD file_type = GetFileAttributesA(directoryName.c_str());
        if (file_type == INVALID_FILE_ATTRIBUTES)
        {
            return false;
        }

        if (file_type & FILE_ATTRIBUTE_DIRECTORY)
        {
            return true;
        }

        return false;
    }

    static std::string preprocess(const std::string& foldPath)
    {
        size_t len = foldPath.length();
        if (foldPath[len - 1] == '\\')
        {
            return foldPath.substr(0, len - 1);
        }
        else
        {
            return foldPath;
        }
    }

    static bool CreateFolder(const std::string& foldPath)
    {
        if (foldPath.empty())
        {
            return false;
        }

        std::string inner_folder_path = preprocess(foldPath);
        size_t last_backslash_index = inner_folder_path.find_last_of("\\");

        if (last_backslash_index == std::string::npos)
        {
            if (!IsDirExist(inner_folder_path))
            {
                CreateDirectory(inner_folder_path.c_str(), nullptr);
            }

        }
        else
        {
            std::string parent_dir = inner_folder_path.substr(0, last_backslash_index);
            std::cout << parent_dir << std::endl;
            if (IsDirExist(parent_dir))
            {
                CreateDirectory(inner_folder_path.c_str(), nullptr);
            }
            else
            {
                CreateFolder(parent_dir);
                CreateDirectory(foldPath.c_str(), nullptr);
            }
        }

        return true;
    }


    static BOOL GetDriverInfo(LPSTR szDrive, std::vector<std::string>& driverRootPaths)
    {
        UINT uDriverType;

        uDriverType = GetDriveType(szDrive);

        switch (uDriverType)
        {
        case DRIVE_UNKNOWN:
            break;
        case DRIVE_NO_ROOT_DIR:
            break;
        case DRIVE_REMOVABLE:
            break;
        case DRIVE_FIXED:
            driverRootPaths.push_back(std::string(szDrive));
            break;
        case DRIVE_REMOTE:
            break;
        case DRIVE_CDROM:
            break;
        case DRIVE_RAMDISK:
            break;
        default:
            break;
        }

        return TRUE;

    }

    static std::string find_first_available_driver()
    {
        CHAR szLogicDriveStrings[BUFSIZE];
        PCHAR szDrive;

        ZeroMemory(szLogicDriveStrings, BUFSIZE);

        GetLogicalDriveStrings(BUFSIZE - 1, szLogicDriveStrings);
        szDrive = (PCHAR)szLogicDriveStrings;
        std::vector<std::string> diskDrivePaths{};
        do
        {
            if (!GetDriverInfo(szDrive, diskDrivePaths))
            {
                printf("\nGet Volume Information Error:%d", GetLastError());
            }
            szDrive += (lstrlen(szDrive) + 1);
        } while (*szDrive != '\x00');

        if (!diskDrivePaths.empty())
        {
            return diskDrivePaths[0].c_str();
        }

        return "";
    }

    static std::string GetUTC()
	{
        auto timestamp = std::chrono::seconds(std::time(nullptr));
        int seconds = std::chrono::seconds(timestamp).count();
        return std::to_string(seconds);
	}
}