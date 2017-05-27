#pragma once

#include <obs.h>
#include <list>
#include <functional>

namespace obs {

class module {
public:
    enum status_type {
        okay,
        generic_error,
        file_not_found,
        missing_exports,
        incompatible_version
    };
    
private:
    obs_module_t *m_handle;
    status_type m_status;

public:
    struct paths {
        paths(std::string bin, std::string data) 
            : bin_path(bin), data_path(data) {}
        std::string bin_path;
        std::string data_path;
    };

    module(const std::string path, const std::string data_path);
    module(obs_module_t *module);

    status_type status();
    bool initialize();

    std::string file_name();
    std::string name();
    std::string author();
    std::string description();
    std::string binary_path();
    std::string data_path();

    std::string data_filepath(std::string file);
    std::string config_filepath(std::string file);

    static void add_path(const std::string bin_path, const std::string data_path);
    static void load_all();
    static std::vector<module::paths> find();
    static std::vector<module> enumerate();
    static void log_loaded();

};

module::module(const std::string binary_path, const std::string data_path)
{
    int module_status = 
        obs_open_module(&m_handle, binary_path.c_str(), data_path.c_str());

    m_status = static_cast<module::status_type>(-module_status);
}

module::module(obs_module_t *handle)
 : m_handle(handle)
{
}

module::status_type module::status()
{
    return m_status;
}

bool module::initialize()
{
    return obs_init_module(m_handle);
}

std::string module::file_name()
{
    return obs_get_module_file_name(m_handle);
}

std::string module::name()
{
    return obs_get_module_name(m_handle);
}

std::string module::author()
{
    return obs_get_module_author(m_handle);
}

std::string module::description()
{
    return obs_get_module_description(m_handle);
}

std::string module::binary_path()
{
    return obs_get_module_binary_path(m_handle);
}

std::string module::data_path()
{
    return obs_get_module_data_path(m_handle);
}


std::string module::data_filepath(std::string file)
{
    char *buffer = obs_find_module_file(m_handle, file.c_str());

    if (!buffer)
        return "";

    std::string path(buffer);
    bfree(buffer);

    return path;
}

std::string module::config_filepath(std::string file)
{
    char *buffer = obs_module_get_config_path(m_handle, file.c_str());

    if (!buffer)
        return "";

    std::string path(buffer);
    bfree(buffer);

    return path;
}

/********************
 * Global Functions
 ********************/

void module::add_path(const std::string bin_path, const std::string data_path)
{
    obs_add_module_path(bin_path.c_str(), data_path.c_str());
}

void module::load_all()
{
    /* I really hate this convenience function.
     * It should pass back module handles so we 
     * can track them. I might modify this in
     * upstream so I don't have to work around it. */
    obs_load_all_modules();
}

std::vector<module::paths> module::find()
{
    std::vector<module::paths> path_list;

    auto cb_wrapper = 
    [](void *param, const struct obs_module_info *info)
    {
        std::vector<module::paths> *path_list = 
            reinterpret_cast<std::vector<module::paths>*>(param);

        path_list->push_back(module::paths(info->bin_path, info->data_path));
    };

    obs_find_modules(cb_wrapper, &path_list);
    return path_list;
}

std::vector<module> module::enumerate()
{
    std::vector<module> module_list;

    auto cb_wrapper =
    [](void *param, obs_module_t *module_)
    {
        std::vector<module> *module_list =
            reinterpret_cast<std::vector<module>*>(param);

        module_list->push_back(module(module_));
    };

    obs_enum_modules(cb_wrapper, &module_list);
    return module_list;
}

void module::log_loaded()
{
    obs_log_loaded_modules();
}

}