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
#include <inttypes.h>
#include <xmmintrin.h>
#include "gs-limits.h"
extern "C" {
#pragma warning(push)
#pragma warning(disable : 4201)
#include <graphics/vec3.h>
#pragma warning(pop)
}

namespace GS
{
	struct Vertex
	{
		vec3*     position;
		vec3*     normal;
		vec3*     tangent;
		uint32_t* color;
		vec4*     uv[MAXIMUM_UVW_LAYERS];

		Vertex();
		Vertex(vec3* p, vec3* n, vec3* t, uint32_t* col, vec4* uv[MAXIMUM_UVW_LAYERS]);
		~Vertex();

		private:
		bool  hasStore;
		void* store;
	};
} // namespace GS
