#include "obspp-audio.hpp"

namespace obs {

audio::audio(audio_t *ctx) 
 : m_handle(ctx) 
{

}

audio::~audio()
{
}

audio_t *audio::dangerous()
{
    return m_handle;
}

obs::audio audio::global()
{
    return obs_get_audio();
}

audio_encoder::audio_encoder(std::string id, std::string name, obs_data_t *settings, size_t mixer, obs_data_t *hotkeys)
{
    m_handle = obs_audio_encoder_create(id.c_str(), name.c_str(), settings, mixer, hotkeys);
    m_status = encoder::status_type::okay; 

    if (!m_handle) {
        m_status = encoder::status_type::invalid;
    }
}

audio_encoder::audio_encoder(obs_encoder_t *encoder)
{
    m_handle = encoder;
    m_status = encoder::status_type::okay;

    if (!m_handle) {
        m_status = encoder::status_type::invalid;
    }
}

obs_encoder_t *audio_encoder::dangerous()
{
    return m_handle;
}

obs::audio_encoder audio_encoder::from_name(std::string name)
{
    obs_encoder_t *encoder = obs_get_encoder_by_name(name.c_str());
    obs_encoder_release(encoder);
    encoder::check_type(encoder, OBS_ENCODER_AUDIO);
    return encoder;
}

std::vector<std::string> audio_encoder::types()
{
    const char *id = nullptr;
    std::vector<std::string> type_list;

    int i = 0;

    while (obs_enum_encoder_types(i++, &id)) {
        if (obs_get_encoder_type(id) != OBS_ENCODER_AUDIO)
            continue;

        type_list.push_back(std::move(std::string(id)));
    }

    return type_list;
}

uint32_t audio_encoder::sample_rate()
{
    return obs_encoder_get_sample_rate(m_handle);
}

void audio_encoder::audio(obs::audio audio)
{
    obs_encoder_set_audio(m_handle, audio.dangerous());
}

obs::audio audio_encoder::audio()
{
    return obs_encoder_audio(m_handle);
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

int volmeter::nr_channels()
{
    return obs_volmeter_get_nr_channels(m_handle);
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