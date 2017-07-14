#pragma once

#include <obs.h>

#include <string>
#include <exception>

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

}