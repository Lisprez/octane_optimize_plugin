#include "octane_lua_api.h"
#include "easylogging++.h"

octane_lua_api::OCtaneLuaAPI::OCtaneLuaAPI()
{
	LOG(DEBUG) << "construct OCtaneLuaAPI" << std::endl;
}

octane_lua_api::OCtaneLuaAPI::~OCtaneLuaAPI()
{
	LOG(DEBUG) << "deconstruct OCtaneLuaAPI" << std::endl;
	delete octane_lua_container_;
}

void octane_lua_api::OCtaneLuaAPI::Setup(sol::state_view* s)
{
	LOG(DEBUG) << "config octane_lua_container_" << std::endl;
	octane_lua_container_ = s;
}

octane_lua_api::OCtaneLuaAPI& octane_lua_api::OCtaneLuaAPI::Get()
{
	static OCtaneLuaAPI octane_lua_api{};
	return octane_lua_api;
}

sol::table octane_lua_api::OCtaneLuaAPI::operator [](const std::string& table_name) const
{
	return (*octane_lua_container_)[table_name];
}