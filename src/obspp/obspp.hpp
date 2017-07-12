#pragma once

#include <obs.h>

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
status_type status();
uint32_t version();
std::string locale();
void locale(std::string locale);
void log_handler(log_handler_t handler);
log_handler_t log_handler();

}