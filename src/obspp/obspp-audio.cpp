#include "obspp-audio.hpp"

namespace obs {

audio::audio(audio_t *ctx) 
 : handle(ctx) 
{

}

audio::audio(audio_output_info *info)
{
    /* TODO */
}

audio::~audio()
{

}

audio_encoder::audio_encoder(std::string id, std::string name)
 : handle(obs_audio_encoder_create(id.c_str(), name.c_str(), NULL, 0, NULL))
{

}

fader::fader(obs_fader_type type)
 : m_handle(obs_fader_create(type))
{
}

fader::~fader() 
{
    obs_fader_destroy(m_handle);
}

void fader::db(const float ms)
{
    obs_fader_set_db(m_handle, ms);
}

float fader::db()
{
    return obs_fader_get_db(m_handle);
}

void fader::deflection(const float def)
{
    obs_fader_set_deflection(m_handle, def);
}

float fader::deflection()
{
    return obs_fader_get_deflection(m_handle);
}

void fader::mul(const float mul)
{
    obs_fader_set_mul(m_handle, mul);
}

float fader::mul()
{
    return obs_fader_get_mul(m_handle);
}

void fader::attach(obs::source &source)
{
    obs_fader_attach_source(m_handle, source.dangerous());
}

void fader::detach()
{
    obs_fader_detach_source(m_handle);
}

void fader::add_callback(obs_fader_changed_t cb, void *data)
{
    obs_fader_add_callback(m_handle, cb, data);
}

void fader::remove_callback(obs_fader_changed_t cb, void *data)
{
    obs_fader_remove_callback(m_handle, cb, data);
}


volmeter::volmeter(obs_fader_type type)
 : m_handle(obs_volmeter_create(type))
{

}

volmeter::~volmeter()
{
    obs_volmeter_destroy(m_handle);
}

void volmeter::attach(obs::source &source)
{
    obs_volmeter_attach_source(m_handle, source.dangerous());
}

void volmeter::detach()
{
    obs_volmeter_detach_source(m_handle);
}

void volmeter::interval(int ms)
{
    obs_volmeter_set_update_interval(m_handle, ms);
}

int volmeter::interval()
{
    return obs_volmeter_get_update_interval(m_handle);
}

void volmeter::peak_hold(int hold_time)
{
    obs_volmeter_set_peak_hold(m_handle, hold_time);
}

int volmeter::peak_hold()
{
    return obs_volmeter_get_peak_hold(m_handle);
}

void volmeter::add_callback(obs_volmeter_updated_t cb, void *data)
{
    obs_volmeter_add_callback(m_handle, cb, data);
}

void volmeter::remove_callback(obs_volmeter_updated_t cb, void *data)
{
    obs_volmeter_remove_callback(m_handle, cb, data);
}


}