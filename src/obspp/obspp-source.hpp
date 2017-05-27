#pragma once

/* 
    Source derivative classes fulfill the source "concept". 

    Functions:
    constructor(std::string id, std::string name, obs::settings settings)
    *** Constructs the actual source. 
    *** The exception here are scenes. They don't use ids or settings.
    *** See obspp-scene.hpp for more information.

    constructor(obs_source_t *source)
    *** Constructs an object from a native handle. 

    constructor(<source_type> &source)
    *** Constructs an object from a wrapper object.
    *** The underlying handle must stay open!

    destructor()
    *** Destroys all references possible to the source.

    static const std::list<std::string> types()
    *** Lists the different available sources of that category.

    const std::string name()
    *** Lists the visual name of the source. 

    const std::string id()
    *** Lists the id representing the source. 

    const std::string status()
    *** Returns a status. 0 is success, other is class defined.

    Types:
    enum status_type {
        okay,
        <implementation defined>
    };
    *** An enumeration where the first member is always okay
    *** Representative of the status of the source.
 */

/* Please note that this is mostly a parent class. 
   Most other classes rely on this one. Anything associated
   with an input in obs is treated like a source so a source
   is more like a conceptual interface in obs. The types of
   sources are listed in the enumeration obs_source_type listed
   below for convenience: 

    OBS_SOURCE_TYPE_INPUT
    OBS_SOURCE_TYPE_FILTER
    OBS_SOURCE_TYPE_TRANSITION
    OBS_SOURCE_TYPE_SCENE

   Since we're using a type-safe language that allows overloading, it's
   easier for this as a wrapper to treat each one as an invidual type and
   have a single interface for them all while each type has it's own 
   special functionality in its own type instead of one confusing one. 
 */

/*
   For consistency purposes, we use an std::shared_ptr to handle
   reference counting similar to how obs does. The difference is you 
   don't need to really worry about releasing it explicitly yourself nor
   do you need to worry about when something increases reference count. 
 */

#include <obs.h>
#include <string>
#include <exception>

namespace obs {

class source {
public:
    enum status_type {
        okay,
        invalid
    };

protected:
    obs_source_t *m_handle;
    status_type m_status;

    source();
    source(std::string &id, std::string &name, obs_data_t *settings = nullptr);
    source(obs_source_t *source);
    source(source &&source);

    static void check_type(obs_source_t * source, obs_source_type type);

public:
    void release(); /* Two-step destruction for managed languages. */
    ~source();
    bool operator!() const;
    obs_source_t *dangerous();
    status_type status();
    const std::string name();
    void name(std::string name);
    const std::string id();
    bool configurable();
    uint32_t width();
    uint32_t height();
};

source::source()
 : m_handle(nullptr), m_status(status_type::invalid)
{
}

source::source(std::string &id, std::string &name, obs_data_t *settings)
    : m_handle(obs_source_create(id.c_str(), name.c_str(), settings, nullptr)),
    m_status(status_type::okay)
{
    if (!m_handle)
        m_status = status_type::invalid;
}


source::source(obs_source_t *source)
    : m_handle(source),
    m_status(status_type::okay)
{
    if (!m_handle)
        m_status = status_type::invalid;
}

source::source(source &&source)
    : m_handle(source.m_handle),
    m_status(status_type::okay)
{
    obs_source_addref(m_handle);

    if (!m_handle)
        m_status = status_type::invalid;
}

void source::release() {
    obs_source_release(m_handle);
    m_handle = nullptr;
}

source::~source() {
    obs_source_release(m_handle);
}

void source::check_type(obs_source_t * source, obs_source_type type)
{
    obs_source_type source_type = obs_source_get_type(source);

    if (source_type == type)
        return;

    throw std::runtime_error("Incorrect source type used");
}

bool source::operator!() const
{
    return m_status != status_type::okay;
}

obs_source_t *source::dangerous()
{
    return m_handle;
}

source::status_type source::status()
{
    return m_status;
}

const std::string source::name()
{
    return obs_source_get_name(m_handle);
}

const std::string source::id()
{
    return obs_source_get_id(m_handle);
}

bool source::configurable()
{
    return obs_source_configurable(m_handle);
}

uint32_t source::height()
{
    return obs_source_get_height(m_handle);
}

uint32_t source::width()
{
    return obs_source_get_width(m_handle);
}

}