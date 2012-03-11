#include "plugin_impl/plugin_lua.h"
#include "log_module.h"

plugin_lua_t::plugin_lua_t(const string& name_):
    m_ls(NULL)
{
    string luapath = "./";
    int pos = name_.find_last_of('/');
    if (-1 == pos)
    {
        m_lua_name = name_;
    }
    else
    {
        m_lua_name = name_.substr(pos+1);
        luapath = name_.substr(0, pos+1);
    }
    pos = m_lua_name.find_first_of('.');
    m_lua_name = m_lua_name.substr(0, pos);
    
    m_ls = lua_open();
    string lua_str = "package.path = package.path .. \"" + luapath + "?.lua\"";
    luaL_openlibs(m_ls);
    
     printf("lua path<%s>\n", lua_str.c_str());
    if (luaL_dostring(m_ls, lua_str.c_str()))
    {
        lua_pop(m_ls, 1);
    }
    m_lua_name = name_;
}

plugin_lua_t::~plugin_lua_t()
{
}

int plugin_lua_t::start()
{
    if (load_lua_mod())
    {
        logerror((PLUGIN_IMPL, "can't find %s.lua\n", m_lua_name.c_str()));
        return -1;
    }
    return 0;
}

int plugin_lua_t::stop()
{
    return 0;
}

int plugin_lua_t::handle_broken(channel_ptr_t channel_)
{
    m_channel_mgr.erase(long(channel_));
    delete channel_;
    return call_lua_handle_broken(long(channel_));
}

int plugin_lua_t::handle_msg(const message_t& msg_, channel_ptr_t channel_)
{
    m_channel_mgr.insert(make_pair((long)channel_, channel_));
    return call_lua_handle_msg((long)channel_, msg_.get_body());
}

int plugin_lua_t::load_lua_mod()
{
    if (luaL_dofile(m_ls, m_lua_name.c_str()))
    {
        lua_pop(m_ls, 1);
        return -1;
    }
    return 0;
}

int plugin_lua_t::call_lua_handle_msg(long val, const string& msg)
{
    lua_checkstack(m_ls, 20);
    lua_getglobal(m_ls, "handle_msg");
    lua_pushnumber(m_ls, val);
    lua_pushlstring(m_ls, msg.c_str(), msg.size());
    if (lua_pcall(m_ls, 2, 0, 0) != 0)
    {
        lua_pop(m_ls, 1);
        return -1;
    }
    return 0;
}

int plugin_lua_t::call_lua_handle_broken(long val)
{
    lua_checkstack(m_ls, 20);
    lua_getglobal(m_ls, "handle_broken");
    lua_pushnumber(m_ls, val);
    if (lua_pcall(m_ls, 1, 0, 0) != 0)
    {
        lua_pop(m_ls, 1);
        return -1;
    }
    return 0;
}

