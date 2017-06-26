#pragma once

#include "obspp-source.hpp"
#include "obspp-filter.hpp"
#include <vector>
#include <string>

namespace obs {

class input : public source {
public:
    input(std::string id, std::string name, obs_data_t *hotkey, obs_data_t *settings);
    input(std::string id, std::string name, obs_data_t *settings, bool is_private);
    input(input &copy);
    input(obs_source_t *source);

    void volume(float volume);
    float volume();

    int64_t sync_offset();
    void sync_offset(int64_t offset);

    bool showing();

    uint32_t flags();
    void flags(uint32_t flag);

    void audio_mixers(uint32_t mixer);
    uint32_t audio_mixers();

    void add_filter(obs::filter filter);
    void remove_filter(obs::filter filter);

    std::vector<filter> filters();

    static std::vector<std::string> types();
};
}