#pragma once

#include "obspp-source.hpp"
#include "obspp-filter.hpp"
#include <vector>
#include <string>

namespace obs {

class input : public source {
public:
    input(std::string id, std::string name, obs_data_t *settings = nullptr, obs_data_t *hotkey = nullptr);
    input(std::string id, std::string name, obs_data_t *settings, bool is_private = false);
    input(input &copy);
    input(obs_source_t *source);

    input duplicate(std::string name, bool is_private);

    static input from_name(std::string name);

    void volume(float volume);
    float volume();

    int64_t sync_offset();
    void sync_offset(int64_t offset);

    bool showing();

    void audio_mixers(uint32_t mixer);
    uint32_t audio_mixers();

    void add_filter(obs::filter filter);
    void remove_filter(obs::filter filter);

    obs::filter find_filter(std::string name);
    std::vector<filter> filters();

    static std::vector<std::string> types();
    static std::vector<obs::input> public_sources();
};
}