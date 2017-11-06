#pragma once

#include <obs.h>

#include <string>
#include <exception>

#include "obspp-source.hpp"

namespace obs {

bool startup(std::string locale, std::string path);
bool startup(std::string locale);
void shutdown();
bool initialized();
uint32_t version();
std::string locale();
void locale(std::string locale);
void log_handler(log_handler_t handler);
log_handler_t log_handler();
void output(int channel, source source);
source output(int channel);
uint32_t output_flags_from_id(std::string id);
uint32_t total_frames();
uint32_t lagged_frames();

}