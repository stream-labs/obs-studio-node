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

}