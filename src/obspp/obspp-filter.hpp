#pragma once

#include "obspp-source.hpp"

#include <list>

namespace obs {

class filter : public source {
public:
    filter(std::string &id, std::string &name, obs_data_t *settings = nullptr);
    filter(filter &&copy);
    filter(obs_source_t *source);

    static std::list<std::string> types();
};

}