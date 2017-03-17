#pragma once

#include <functional>
#include <string>
#include <ostream>
#include <sstream>
#include <ctime>

#include "sol.hpp"
#include "pugixml.hpp"
#include "pugiconfig.hpp"
#include "octane_lua_api.h"
#include "easylogging++.h"

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
}