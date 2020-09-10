#pragma once

#include "cache.h"
#include <map>
#include <vector>
#include <string>
#include <sstream>

struct IntervalInfo {
	uint64_t offset;
	uint64_t length;
	uint64_t cache_offset;

	std::string toString() {
		std::stringstream ss;
		ss << offset << "\t" << length << "\t" << cache_offset;
		return ss.str();
	}
};

class IntervalFileCache : public Cache {
public:
	void setIntervals(const std::vector<std::pair<uint64_t, uint64_t>> &intervals);

	int write(uint64_t offset, uint64_t length);
	int read(uint64_t offset, uint64_t length);

	std::string toString();

private:
	std::map<uint64_t, IntervalInfo> m_intervals;
	uint64_t m_data_length;
};
