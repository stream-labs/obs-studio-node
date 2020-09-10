/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include "shared.hpp"

#ifdef WIN32
    std::queue<std::function<void(Napi::Object)>>* initializerFunctions =
        new std::queue<std::function<void(Napi::Object)>>;
#endif
#ifdef __APPLE__
    std::queue<std::function<void(Napi::Object)>>* initializerFunctions = nullptr;
    UtilInt* g_util_osx;
#endif

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
};

#ifdef WIN32
HANDLE create_semaphore(const char* name = nullptr) {
	return CreateSemaphore(NULL, 1, 2, NULL);
}

void remove_semaphore(HANDLE sem, const char* name = nullptr) {
	if (sem) {
		CloseHandle(sem);
	}
}

void wait_semaphore(HANDLE sem) {
	if (sem) {
		WaitForSingleObject(sem, INFINITE);
	}
}

void release_semaphore(HANDLE sem) {
	if (sem) {
		ReleaseSemaphore(sem, 1, NULL);
	}
}
#else
sem_t* create_semaphore(const char* name) {
	sem_unlink(name);
	remove(name);
	return sem_open(name, O_CREAT | O_EXCL, 0644, 1);
}

void remove_semaphore(sem_t *sem, const char* name) {
	sem_close(sem);
	remove(name);
}

void wait_semaphore(sem_t *sem) {
	sem_wait(sem);
}

void release_semaphore(sem_t *sem) {
	sem_post(sem);
}
#endif