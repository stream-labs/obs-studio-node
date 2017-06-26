#pragma once

#include <string>
#include <obs-properties.h>

namespace obs {

class properties {
public:
    enum status_type {
        invalid,
        okay
    };

    /* TODO We should be able to pass an id without having to type check */
    enum object_type {
        source,
        encoder,
        output,
        service
    };

private:
    obs_properties_t *m_handle;
    status_type m_status;

public:
    properties(obs_properties_t *properties);
    properties(std::string id, object_type type);
    ~properties();

    /* FIXME Not implemented yet. */
    // properties(obs::service service);
    // properties(obs::output output);

    class property {
        friend properties;

    public:
        enum status_type {
            invalid,
            okay
        };
    
    private:
        obs_property_t *m_handle;
        status_type m_status;

        property(obs_property_t *property);

    public:
        obs_property_t *dangerous();
        status_type status();
        std::string name();
        std::string description();
        std::string long_description();
        obs_property_type type();
        bool enabled();
        bool visible();
        bool next();
    };

    status_type status();
    property first();
    property get(std::string property_name);
    obs_properties_t *dangerous();
};

}