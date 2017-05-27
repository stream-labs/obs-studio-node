#pragma once

namespace obs {

class encoder {

public:
    static std::list<std::string>> types();
};

std::list<std::string>> encoder::types()
{
    const char *id = nullptr;
    std::list<std::string> type_list;

    for (int i = 0; obs_enum_encoder_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return type_list;
}

}