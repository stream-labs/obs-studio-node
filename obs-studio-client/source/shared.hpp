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

#pragma once
#include <functional>
#include <napi.h>
#include <queue>
#include <string>
#include "util-osx.hpp"

#ifdef WIN32
#include <windows.h>
#else
#include <semaphore.h>
#endif

#ifndef __FUNCTION_NAME__
#if defined(_WIN32) || defined(_WIN64) //WINDOWS
#define __FUNCTION_NAME__ __FUNCTION__
#else //*NIX
#define __FUNCTION_NAME__ __func__
#endif
#endif

extern std::queue<std::function<void(Napi::Object)>> *initializerFunctions;
extern std::wstring utfWorkingDir;

#ifdef __APPLE__
extern UtilInt *g_util_osx;
#endif

void replaceAll(std::string &str, const std::string &from, const std::string &to);

#ifdef WIN32
extern HANDLE create_semaphore(const char *name);
extern void remove_semaphore(HANDLE sem, const char *name);
extern void wait_semaphore(HANDLE sem);
extern void release_semaphore(HANDLE sem);
#else
extern sem_t *create_semaphore(const char *name);
extern void remove_semaphore(sem_t *sem, const char *name);
extern void wait_semaphore(sem_t *sem);
extern void release_semaphore(sem_t *sem);
#endif
