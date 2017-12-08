#pragma once

#include "nodeobs_obspp_index.hpp"

/* Since the manager does type checking, 
 * we need type information.  */
#include <obs-audio-controls.h>

/* Standard facilities */
#include <map>

namespace obs {

static const char *object_type_map[] = {
	"invalid",
	"callback",
	"fader",
	"volmeter"
};

struct object {
    enum objectType {
        invalid,
        callback,
        fader,
        volmeter
    };

    obs::object::objectType type;
    void *handle;

    void checkType(objectType type);
    bool testType(objectType type);
};

class objectManager {
    obs::indexManager m_index;
    std::map<uint32_t, obs::object> m_objects;

public:
    uint32_t map(obs::object object);
    void unmap(uint32_t id);

    obs::object getObject(uint32_t id);
	bool isUsed(uint32_t id, bool used);
};

};

extern obs::objectManager g_objectManager;
