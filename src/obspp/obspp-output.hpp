#pragma once

#include <vector>

namespace obs {

class output {
public:
    static std::vector<std::string> types();

};

std::vector<std::string> output::types()
{
    const char *id = nullptr;
    std::vector<std::string> type_list;

    for (int i = 0; obs_enum_output_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return type_list;
}

}