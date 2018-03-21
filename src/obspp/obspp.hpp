#pragma once

#include <obs.h>

#include <string>
#include <exception>
#include <vector>

#include "obspp-source.hpp"

namespace obs {

typedef std::vector<std::pair<std::string, std::string>>
    monitoring_devices_type;

bool startup(std::string locale, std::string path);
bool startup(std::string locale);
void shutdown();
bool initialized();
uint32_t version();
std::string locale();
void locale(std::string locale);
void log_handler(log_handler_t handler);
log_handler_t log_handler();
void output_source(int channel, source source);
source output_source(int channel);
uint32_t output_flags_from_id(std::string id);
uint32_t total_frames();
uint32_t lagged_frames();
monitoring_devices_type monitoring_devices();

}