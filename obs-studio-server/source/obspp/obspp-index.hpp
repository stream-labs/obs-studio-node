#pragma once 

/* A simple resuable ID generator heavily based on the IndexerV2 from Xaymar
 * https://github.com/Xaymar/BlitzUtility/blob/master/Utility/IndexerV2.cpp 
 * Slightly modified to match naming consistencies. */

namespace obs {

class index {
public:
    struct range {
        uint32_t min, max;

        range(uint32_t min, uint32_t max);
    };

private:
    std::vector<index::range> m_range;

public:
    index();

    void mark(uint32_t index, bool used);
    bool is(uint32_t index, bool used);
    uint32_t get();
    uint32_t count(bool used);
};

index::index() {
    /* A good reservation that we won't ever 
       go above in normal circumstances.*/
	m_range.reserve(256);
}

void index::mark(uint32_t index, bool used) {
	if (m_range.size() == 0) {
		if (used == true)
			m_range.push_back(index::range(index, index));
	} else {
		auto iter = m_range.begin();
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
					m_range.insert(iter, index::range(index, index));
					break;
				} 
			} else {
				if (index >= iter->min && index <= iter->max) {
					m_range.insert(iter - 1, index::range(iter->min, index - 1));
					m_range.insert(iter + 1, index::range(index + 1, iter->max));
					m_range.erase(iter); break;
				} else if (index == iter->min) {
					iter->min++;
					if (iter->min == iter->max)
						m_range.erase(iter);
					break;
				} else if (index == iter->max) {
					iter->max--;
					if (iter->min == iter->max)
						m_range.erase(iter);
					
					break;
				}
			}
		}
	}
}

bool index::is(uint32_t index, bool used) {
	bool isUsed = false;
	for (auto iter = m_range.begin(); iter != m_range.end(); iter++) {
		if (index >= iter->min && index <= iter->max) {
			isUsed = true;
			break;
		}
	}
	return (isUsed == used);
}

uint32_t index::get() {
	if (m_range.size() == 0) {
		m_range.push_back(index::range(0, 0));
		return 0;
	}

	// We only need to check the first element to get a new free index.
	std::vector<index::range>::iterator iter = m_range.begin();
	if (iter->min > 0) {
		return --iter->min;
	} else {
		if (iter->max == UINT32_MAX)
			return UINT32_MAX;

		uint32_t index = ++iter->max;

		// Check if we can combine this element with the next one.
		std::vector<index::range>::iterator iterN = m_range.begin() + 1;
		if ((iterN != m_range.end()) && iterN->min == iter->max) {
			iter->max = iterN->max;
			m_range.erase(iterN);
		}

		return index;
	}

	return UINT32_MAX;
}

uint32_t index::count(bool used) {
	uint32_t amount = 0;
	for (auto iter = m_range.begin(); iter != m_range.end(); iter++) {
		amount += iter->max - iter->min;
	}
	return (used ? UINT32_MAX - amount : amount);
}

index::range::range(uint32_t min, uint32_t max) {
	this->min = min;
	this->max = max;
}

}