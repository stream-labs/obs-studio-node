#pragma once

#include <vector>

namespace obs {

class output {
public:
    static std::vector<std::string> types();
    int frames_dropped();
    int total_frames();
};

}