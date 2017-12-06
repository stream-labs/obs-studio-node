//  Copyright (C) 2015 Xaymar (Michael Fabian Dirks)
//  Copyright (C) 2017 Zachary Lund <zachary.lund@streamlabs.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the 
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "obspp-index.hpp"

namespace obs {

indexManager::indexManager() {
	/* A good reservation that we won't ever 
	   go above in normal circumstances.*/
	m_range.reserve(64);
}

void indexManager::markUsed(uint32_t index, bool used) {
	if (m_range.size() == 0) {
		if (used == true)
			m_range.push_back(indexManager::range(index, index));

		return;
	}

	for (auto iter = m_range.begin(); iter != m_range.end(); iter++) {
		if (used) {
			if ((index + 1) == iter->min) {
				iter->min--;
				break;
			} else if ((index - 1) == iter->max) {
				iter->max++;
				if ((iter + 1) != m_range.end() && iter->max == (iter + 1)->min) {
					(iter + 1)->min = iter->min;
					m_range.erase(iter);
				}
				break;
			} else if (index < iter->min) {
				m_range.insert(iter, indexManager::range(index, index));
				break;
			}
		}
		else {
			/* If the index we're unmarking is within a range, 
			 * create a new range and split the two. */
			if (index > iter->min && index < iter->max) {
				indexManager::range &range = *iter;
				m_range.insert(iter + 1, indexManager::range(index + 1, iter->max));
				range.min = iter->min;
				range.max = index - 1;
				break;
			}
			/* If index is equal to max, just decrement max. */
			else if (index == iter->max) {
				iter->max--;
				if (iter->min > iter->max)
					m_range.erase(iter);

				break;
			}
			/* If index is equal to min, just increment min. */
			else if (index == iter->min) {
				iter->min++;
				if (iter->min > iter->max)
					m_range.erase(iter);
				break;
			}
		}
	}
}

bool indexManager::isUsed(uint32_t index, bool used) {
	bool isUsed = false;
	for (auto iter = m_range.begin(); iter != m_range.end(); iter++) {
		if (index >= iter->min && index <= iter->max) {
			isUsed = true;
			break;
		}
	}
	return (isUsed == used);
}

uint32_t indexManager::generateNewIndex() {
	if (m_range.size() == 0) {
		m_range.push_back(indexManager::range(1, 1));
		return 1;
	}

	// We only need to check the first element to get a new free index.
	std::vector<indexManager::range>::iterator iter = m_range.begin();
	if (iter->min > 1) {
		return --iter->min;
	} else {
		if (iter->max == UINT32_MAX)
			return UINT32_MAX;

		uint32_t index = ++iter->max;

		// Check if we can combine this element with the next one.
		std::vector<indexManager::range>::iterator iterN = m_range.begin() + 1;
		if ((iterN != m_range.end()) && iterN->min == iter->max + 1) {
			iter->max = iterN->max;
			m_range.erase(iterN);
		}

		return index;
	}

	return UINT32_MAX;
}

uint32_t indexManager::countIndexes(bool used) {
	uint32_t amount = 0;
	for (auto iter = m_range.begin(); iter != m_range.end(); iter++) {
		amount += iter->max - iter->min;
	}
	return (used ? UINT32_MAX - amount : amount);
}

indexManager::range::range(uint32_t min, uint32_t max) {
	this->min = min;
	this->max = max;
}

}