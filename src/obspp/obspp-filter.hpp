#pragma once

#include "obspp-source.hpp"

#include <vector>

namespace obs {

class filter : public source {
public:
    /* Filters are always marked private */
    filter(std::string &id, std::string &name, obs_data_t *settings = nullptr);
    filter(filter &copy);
    filter(obs_source_t *source);

    static std::vector<std::string> types();
};

}