// Server program for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "shared.hpp"
#include "obs.h"

shared::LogWarnTimer::~LogWarnTimer() {
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds dur =
		std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	size_t ns = dur.count();
	size_t mus = ns / 1000;
	size_t ms = mus / 1000;
	size_t s = ms / 1000;

	if (dur >= warn_limit) {
		blog(LOG_INFO, "Spent %2llu.%03llu,%03llu,%03llu ns in %*s NOT OKAY (limit was %llu ns).",
			s, ms % 1000, mus % 1000, ns % 1000,
			name.length(), name.c_str(),
			warn_limit.count());
	} else {
		blog(LOG_INFO, "Spent %2llu.%03llu,%03llu,%03llu ns in %*s OKAY (limit was %llu ns).",
			s, ms % 1000, mus % 1000, ns % 1000,
			name.length(), name.c_str(),
			warn_limit.count());
	}
}

shared::LogWarnTimer::LogWarnTimer(std::string name, std::chrono::nanoseconds const warn_limit /*= std::chrono::nanoseconds(2000000ull)*/) {
	this->name = name;
	this->warn_limit = warn_limit;
	this->begin = std::chrono::high_resolution_clock::now();
}
