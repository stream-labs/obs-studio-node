#pragma once

#include <obs.h>
#include "obspp-input.hpp"
#include "obspp-module.hpp"
#include "obspp-video.hpp"

#include <string>
#include <exception>

namespace obs {

enum status_type {
    okay,
    invalid
};

bool startup(std::string locale, std::string path);
bool startup(std::string locale);
void shutdown();
unsigned char status();
uint32_t version();
std::string locale();
void locale(std::string locale);
void log_handler(log_handler_t handler);
log_handler_t log_handler();
void output_source(int channel, obs_source_t *source);

template <typename T>
void output_source(int channel, T &source)
{
    obs_set_output_source(channel, source.dangerous());
}

template <typename T>
T&& output_source(int channel)
{
    return std::move(obs_get_output_source(channel));
}

}