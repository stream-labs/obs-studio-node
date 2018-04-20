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

#include "utility.hpp"

utility::unique_id::unique_id() {}

utility::unique_id::~unique_id() {}

utility::unique_id::id_t utility::unique_id::allocate() {
	if (allocated.size() > 0) {
		for (auto& v : allocated) {
			if (v.first > 0) {
				utility::unique_id::id_t v2 = v.first - 1;
				mark_used(v2);
				return v2;
			} else if (v.second < std::numeric_limits<utility::unique_id::id_t>::max()) {
				utility::unique_id::id_t v2 = v.second + 1;
				mark_used(v2);
				return v2;
			}
		}
	} else {
		mark_used(0);
		return 0;
	}
	
	// No more free indexes. However that has happened.
	return std::numeric_limits<utility::unique_id::id_t>::max();
}

void utility::unique_id::free(utility::unique_id::id_t v) {
	mark_free(v);
}

bool utility::unique_id::is_allocated(utility::unique_id::id_t v) {
	for (auto& v2 : allocated) {
		if ((v >= v2.first) && (v <= v2.second))
			return true;
	}
	return false;
}

utility::unique_id::id_t utility::unique_id::count(bool count_free) {
	id_t count = 0;
	for (auto& v : allocated) {
		count += (v.second - v.first);
	}
	return count_free ? (std::numeric_limits<id_t>::max() - count) : count;
}

bool utility::unique_id::mark_used(utility::unique_id::id_t v) {
	// If no elements have been assigned, simply insert v as used.
	if (allocated.size() == 0) {
		range_t r;
		r.first = r.second = v;
		allocated.push_back(r);
		return true;
	}

	// Otherwise, attempt to find the best fitting element.
	bool lastWasSmaller = false;
	for (auto iter = allocated.begin(); iter != allocated.end(); iter++) {
		auto iter2 = iter;		
		if ((iter->first > 0) && (v == (iter->first - 1))) {
			// If the minimum of the selected element is > 0 and v is equal to
			//  (minimum - 1), decrease the minimum.
			iter->first--;

			// Then test if the previous elements maximum is equal to (v - 1),
			//  if so merge the two since they are now continuous.
			iter2--;
			if ((iter2 != allocated.end()) && (iter2->second == (v - 1))) {
				iter2->second = iter->second;
				allocated.erase(iter);
			}

			return true;
		} else if ((iter->second < std::numeric_limits<utility::unique_id::id_t>::max()) && (v == (iter->second + 1))) {
			// If the maximum of the selected element is < UINT_MAX and v is 
			//  equal to (maximum + 1), increase the maximum.
			iter->second++;

			// Then test if the next elements minimum is equal to (v + 1),
			//  if so merge the two since they are now continuous.
			iter2++;
			if ((iter2 != allocated.end()) && (iter2->first == (v + 1))) {
				iter->second = iter2->second;
				allocated.erase(iter2);
			}

			return true;
		} else if (lastWasSmaller && (v < iter->first)) {
			// If we are between two ranges that are smaller and larger than v
			//  insert a new element before the larger range containing only v.
			allocated.insert(iter, { v, v });
			return true;
		} else if ((iter2++) == allocated.end()) {
			// Otherwise if we reached the end of the list, append v.
			allocated.insert(iter2, { v, v });
			return true;
		}
		lastWasSmaller = (v > iter->second);
	}
	return false;
}

void utility::unique_id::mark_used_range(utility::unique_id::id_t min, utility::unique_id::id_t max) {
	for (utility::unique_id::id_t v = min; v < max; v++) {
		mark_used(v);
	}
}

bool utility::unique_id::mark_free(utility::unique_id::id_t v) {
	for (auto iter = allocated.begin(); iter != allocated.end(); iter++) {
		// Is v inside this range?
		if ((v >= iter->first) && (v <= iter->second)) {
			if (v == iter->first) {
				// If v is simply the beginning of the range, increase the 
				//  minimum and test if the range is now no longer valid.
				iter->first++;
				if (iter->first > iter->second) {
					// If the range is no longer valid, just erase it.
					allocated.erase(iter);
				}
				return true;
			} else if (v == iter->second) {
				// If v is simply the end of the range, decrease the maximum
				//  and test if the range is now no longer valid.
				iter->second--;
				if (iter->second < iter->first) {
					// If the range is no longer valid, just erase it.
					allocated.erase(iter);
				}
				return true;
			} else {
				// Otherwise, since v is inside the range, split the range at
				// v and insert a new element.
				range_t x;
				x.first = iter->first;
				x.second = v - 1;
				iter->first = v + 1;
				allocated.insert(iter, x);
				return true;
			}
		}
	}
	return false;
}

void utility::unique_id::mark_free_range(utility::unique_id::id_t min, utility::unique_id::id_t max) {
	for (utility::unique_id::id_t v = min; v < max; v++) {
		mark_free(v);
	}
}
