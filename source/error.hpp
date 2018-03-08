// OBS Studio Node Shared Code
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

#pragma once

enum class ErrorCode : long long {
	// Everything is okay.
	Ok,

	// A generic error (no specific error code) happened.
	Error,

	// A critical generic error happened.
	CriticalError,

	// The reference specified in the arguments is not valid.
	InvalidReference,

	// There are no further Ids free to use.
	OutOfIndexes,

	// Something could not be found.
	NotFound,
};
