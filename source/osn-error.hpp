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

enum class ErrorCode : long long {
	// Everything is okay.
	Ok,

	// A generic error (no specific error code) happened.
	Error,

	// A critical generic error happened.
	CriticalError,

	// The reference specified in the arguments is not valid.
	InvalidReference,

	// Something could not be found.
	NotFound,

	// Attempted to access something out of bounds.
	OutOfBounds,

	// Add new items at the end, not in between.
};
