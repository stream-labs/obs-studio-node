/*
* Modern effects for a modern Streamer
* Copyright (C) 2017 Michael Fabian Dirks
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "util-memory.h"

void* util::malloc_aligned(size_t align, size_t size)
{
#if defined(_MSC_VER)
#ifdef DEBUG
	return _aligned_malloc_dbg(size, align);
#else
	return _aligned_malloc(size, align);
#endif
#else
	return aligned_malloc(align, size);
#endif
}

void util::free_aligned(void* mem)
{
#if defined(_MSC_VER)
#ifdef DEBUG
	_aligned_free_dbg(mem);
#else
	_aligned_free(mem);
#endif
#else
	free(mem);
#endif
}
