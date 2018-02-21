#pragma once

#include <obs.h>
#include "obspp-source.hpp"

namespace obs {

class view {
public:
    enum status_type {
        okay,
        invalid
    };

private:
    obs_view_t *m_handle;
    status_type m_status;

public:

    view();
    ~view();

    void output(int channel, obs::source source);
    obs::source output(int channel);

    void render();
};

}