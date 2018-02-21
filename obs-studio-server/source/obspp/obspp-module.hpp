#pragma once

#include <obs.h>
#include <vector>
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
    module(obs_module_t *copy);
    module(module &) = default;
    module(module &&) = default;

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

}