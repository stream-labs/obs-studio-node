#pragma once

namespace obs {

#if 0
template <
    typename T,
    typename void Addref(T),
    typename void Release(T)>
struct raw_strong_base {
    T strong_ref;

    raw_strong_base(T source)
        : strong_ref(source) { }

    ~raw_strong_base() { release(); }
    operator T() { return strong_ref; }

    void addref(){ Addref(strong_ref); }
    void release() { Release(strong_ref); }
};

template <typename T> 
struct raw_strong { };

template <>
struct raw_strong<obs_source_t*> 
    : raw_strong_base<
        obs_source_t*,
        obs_source_addref,
        obs_source_release>
{ 
    raw_strong(obs_source_t *ref) 
        : raw_strong_base(ref) { }
};

#endif 

template <class T>
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

    obs::strong<T> get() { return T(static_cast<decltype(T::m_handle)>(weak_ref.get())); }
    bool test(T handle) { weak_ref.test(handle.dangerous()); }
};

}