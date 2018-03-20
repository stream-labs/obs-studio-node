#include "obspp-output.hpp"
#include <obs.h>

namespace obs {

output::output()
 : m_handle(nullptr), m_status(status_type::invalid)
{

}

output::output(std::string &id, std::string &name, obs_data_t *settings, obs_data_t *hotkey)
 : m_handle(obs_output_create(id.c_str(), name.c_str(), settings, hotkey)),
   m_status(status_type::okay)
{
    if (!m_handle)
        m_status = status_type::invalid;
}

output::output(obs_output_t *output)
 : m_handle(output),
   m_status(status_type::okay)
{
    if (!m_handle)
        m_status = status_type::invalid;
}

output::output(output &copy)
    : m_handle(copy.m_handle),
    m_status(copy.m_status)
{
}

void output::release()
{
    obs_output_release(m_handle);
}

output::status_type output::status()
{
    return m_status;
}

obs_output_t *output::dangerous()
{
    return m_handle;
}

std::string output::name()
{
    return obs_output_get_name(m_handle);
}

std::string output::id()
{
    return obs_output_get_id(m_handle);
}

std::vector<std::string> output::types()
{
    const char *id = nullptr;
    std::vector<std::string> type_list;

    for (int i = 0; obs_enum_output_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return type_list;
}

int output::frames_dropped()
{
    return obs_output_get_frames_dropped(m_handle);
}

int output::total_frames()
{
    return obs_output_get_total_frames(m_handle);
}

int output::total_bytes()
{
    return obs_output_get_total_bytes(m_handle);
}

obs_data_t *output::defaults(std::string id)
{
    return nullptr;
    //return obs_get_output_defaults(id.c_str());
}

output output::from_name(std::string name)
{
    obs_output_t *output = obs_get_output_by_name(name.c_str());
    obs_output_release(output);
    return output;
}

void output::media(obs::video video, obs::audio audio)
{
    obs_output_set_media(m_handle, video.dangerous(), audio.dangerous());
}

obs::video output::video()
{
    return obs_output_video(m_handle);
}

obs::audio output::audio()
{
    return obs_output_audio(m_handle);
}

void output::mixer(size_t idx)
{
    obs_output_set_mixer(m_handle, idx);
}

size_t output::mixer()
{
    return obs_output_get_mixer(m_handle);
}

void output::video_encoder(obs::video_encoder encoder)
{
    obs_output_set_video_encoder(m_handle, encoder.dangerous());
}

void output::audio_encoder(obs::audio_encoder encoder, size_t idx)
{
    obs_output_set_audio_encoder(m_handle, encoder.dangerous(), idx);
}

obs::video_encoder output::video_encoder()
{
    return obs_output_get_video_encoder(m_handle);
}

obs::audio_encoder output::audio_encoder(size_t idx)
{
    return obs_output_get_audio_encoder(m_handle, idx);
}

void output::service(obs::service service)
{
    obs_output_set_service(m_handle, service.dangerous());
}

obs::service output::service()
{
    return obs_output_get_service(m_handle);
}

void output::reconnect_options(int retry_count, int retry_sec)
{
    obs_output_set_reconnect_settings(m_handle, retry_count, retry_sec);
}

void output::preferred_size(uint32_t width, uint32_t height)
{
    obs_output_set_preferred_size(m_handle, width, height);
}

uint32_t output::width()
{
    return obs_output_get_width(m_handle);
}

uint32_t output::height()
{
    return obs_output_get_height(m_handle);
}

float output::congestion()
{
    return obs_output_get_congestion(m_handle);
}

int output::connect_time()
{
    return obs_output_get_connect_time_ms(m_handle);
}

bool output::reconnecting()
{
    return obs_output_reconnecting(m_handle);
}

std::string output::supported_video_codecs()
{
    return std::string(obs_output_get_supported_video_codecs(m_handle));
}

std::string output::supported_audio_codecs()
{
    return std::string(obs_output_get_supported_audio_codecs(m_handle));
}

bool output::start()
{
    return obs_output_start(m_handle);
}

void output::stop()
{
    obs_output_stop(m_handle);
}

void output::delay(uint32_t ms, uint32_t flags)
{
    obs_output_set_delay(m_handle, ms, flags);
}

uint32_t output::delay()
{
    return obs_output_get_delay(m_handle);
}

uint32_t output::active_delay()
{
    return obs_output_get_active_delay(m_handle);
}

std::string output::last_error()
{
    const char * error = obs_output_get_last_error(m_handle);
    return error ? error : "";
}

bool output::configurable()
{
    /* Since apprently only source can be tested. */
    return true;
}

obs::properties output::properties()
{
    return obs_output_properties(m_handle);
}

obs_data_t *output::settings()
{
    return obs_output_get_settings(m_handle);
}

void output::update(obs_data_t *data)
{
    obs_output_update(m_handle, data);
}


}
