#include "obspp-output.hpp"
#include <obs.h>

namespace obs {

std::vector<std::string> output::types()
{
    const char *id = nullptr;
    std::vector<std::string> type_list;

    for (int i = 0; obs_enum_output_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return type_list;
}

int output::frames_dropped()
{
    obs_output_get_frames_dropped(m_handle);
}

int output::total_frames()
{
    obs_output_get_total_frames(m_handle);
}

}
