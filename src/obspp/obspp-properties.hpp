#pragma once

#include <string>
#include <obs-properties.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace obs {

class properties;
class list_property;
class float_property;
class text_property;
class path_property;

class property {
    friend properties;
public:
    enum status_type {
        invalid,
        okay
    };

protected:
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
    list_property list_property();
    float_property float_property();
    text_property text_property();
    path_property path_property();
    
    bool enabled();
    bool visible();
    bool next();
};

class list_property : public property 
{
    friend property;
    friend class editable_list_property;
    list_property(obs_property_t *);

public:
    obs_combo_type type();
    size_t count();
    const char* get_name(size_t idx);
    const char* get_string(size_t idx);
    long long get_integer(size_t idx);
    double get_float(size_t idx);
};

class editable_list_property : public list_property{
    friend property;
    editable_list_property(obs_property_t *);

public:
    obs_editable_list_type type();
    const char *filter();
    const char *default_path();
};

class float_property : public property {
    friend property;
    float_property(obs_property_t *);

public:
    obs_number_type type();
    double min();
    double max();
    double step();
};

class integer_property : public property {
    friend property;
    integer_property(obs_property_t *);

public:
    obs_number_type type();
    int min();
    int max();
    int step();
};

class text_property : public property {
    friend property;
    text_property(obs_property_t *);

public:
    obs_text_type type();
};

class path_property : public property {
    friend property;
    path_property(obs_property_t *);

public:
    obs_path_type type();
    const char *filter();
    const char *default_path();
};

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
    properties(obs::properties &properties) = delete;
    properties(obs::properties &&properties);
    properties(obs_properties_t *properties);
    properties(std::string id, object_type type);
    ~properties();

    status_type status();
    property first();
    property get(std::string property_name);
    obs_properties_t *dangerous();
};

}