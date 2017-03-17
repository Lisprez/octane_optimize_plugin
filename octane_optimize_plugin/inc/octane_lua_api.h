#pragma once

#include <string>
#include "sol.hpp"

namespace octane_lua_api {
	// OCtaneµÄAPI¶ÔÏó
	class OCtaneLuaAPI {
	public:
		OCtaneLuaAPI(const OCtaneLuaAPI &) = delete;
		OCtaneLuaAPI& operator=(const OCtaneLuaAPI &) = delete;
		OCtaneLuaAPI(OCtaneLuaAPI &&) = delete;
		OCtaneLuaAPI& operator=(OCtaneLuaAPI &&) = delete;

		static OCtaneLuaAPI& Get();
		~OCtaneLuaAPI();
		void Setup(sol::state_view* s);
		sol::table operator [](const std::string& table_name) const;

	private:
		OCtaneLuaAPI();
		sol::state_view* octane_lua_container_ = nullptr;

	
	};

}