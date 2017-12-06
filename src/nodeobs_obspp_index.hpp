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

#pragma once 

#include <vector>
#include <cstdint>

/* A simple resuable ID generator heavily based on the IndexerV2 from Xaymar
 * https://github.com/Xaymar/BlitzUtility/blob/master/Utility/IndexerV2.cpp 
 * Slightly modified to match naming consistencies. */

namespace obs {

class indexManager {
private:
    struct range {
        uint32_t min, max;

        range(uint32_t min, uint32_t max);
    };

    std::vector<indexManager::range> m_range;

public:
    indexManager();

    void markUsed(uint32_t index, bool used);
    bool isUsed(uint32_t index, bool used);
    uint32_t generateNewIndex();
    uint32_t countIndexes(bool used);
};

}