#pragma once

namespace obs {

class service {

public:
    static std::list<std::string>> types();
};

std::list<std::string>> service::types()
{
    const char *id = nullptr;
    std::list<std::string> type_list;

    for (int i = 0; obs_enum_service_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return type_list;
}

}