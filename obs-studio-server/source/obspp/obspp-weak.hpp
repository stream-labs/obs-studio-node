#pragma once

namespace obs {

/* FIXME: Since any type is accepted here, 
   it's actually a bug since a source can 
   now be generically created by a strong
   container that contains virtually anything.
   
   We should have some metaprogramming that 
   tests for only obs types, otherwise error. */
template <typename T>
class strong {
public:
    T strong_ref;

    strong(T &handle)
     : strong_ref(handle) { }

    T *operator->() { return &strong_ref; }
    T &operator*() { return strong_ref; }
    const T *operator->() const { return &strong_ref; }
    const T &operator*() const { return strong_ref; }

    ~strong() { strong_ref.release(); }

    T &get() { return strong_ref; };
};

template <
    typename T,
    typename Strong,
    typename T Construct(Strong),
    typename Strong Convert(T),
    typename bool Test(T, Strong),
    typename void Addref(T),
    typename void Release(T)>
struct raw_weak_base
{
    T weak_ref;

    raw_weak_base(Strong ref) 
        : weak_ref(Construct(ref)) 
    { }
    
    operator T() { return weak_ref; }

    Strong get()
    {
        return Convert(weak_ref);
    }

    bool test(Strong ref)
    {
        Test(weak_ref, ref);
    }

    void addref()
    {
        Addref(weak_ref);
    }

    void release()
    {
        Release(weak_ref);
    }
};

template <typename T>
struct raw_weak { };

template <>
struct raw_weak <obs_source_t*>
    : raw_weak_base< 
        obs_weak_source_t*,
        obs_source_t*,
        obs_source_get_weak_source,
        obs_weak_source_get_source,
        obs_weak_source_references_source,
        obs_weak_source_addref,
        obs_weak_source_release>
{ 
    raw_weak(obs_source_t* ref) 
        : raw_weak_base(ref) { }
};

template <>
struct raw_weak <obs_service_t*>
    : raw_weak_base< 
        obs_weak_service_t*,
        obs_service_t*,
        obs_service_get_weak_service,
        obs_weak_service_get_service,
        obs_weak_service_references_service,
        obs_weak_service_addref,
        obs_weak_service_release>
{ 
    raw_weak(obs_service_t* ref) 
        : raw_weak_base(ref) { }
};

template <>
struct raw_weak <obs_encoder_t*>
    : raw_weak_base< 
        obs_weak_encoder_t*,
        obs_encoder_t*,
        obs_encoder_get_weak_encoder,
        obs_weak_encoder_get_encoder,
        obs_weak_encoder_references_encoder,
        obs_weak_encoder_addref,
        obs_weak_encoder_release>
{ 
    raw_weak(obs_encoder_t* ref) 
        : raw_weak_base(ref) { }
};

template <>
struct raw_weak <obs_output_t*>
    : raw_weak_base< 
        obs_weak_output_t*,
        obs_output_t*,
        obs_output_get_weak_output,
        obs_weak_output_get_output,
        obs_weak_output_references_output,
        obs_weak_output_addref,
        obs_weak_output_release>
{ 
    raw_weak(obs_output_t* ref) 
        : raw_weak_base(ref) { }
};

template <class T>
class weak {
public:
    raw_weak<decltype(T::m_handle)> weak_ref;

    weak(T &handle)
     : weak_ref(handle.dangerous())
    {
        weak_ref.addref();
    }

    ~weak() { weak_ref.release(); }

    obs::strong<T> get() { return T(weak_ref.get()); }
    bool test(T handle) { weak_ref.test(handle.dangerous()); }
};

}