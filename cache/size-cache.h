#pragma once

#include "cache.h"
#include <map>
#include <vector>
#include <string>
#include <sstream>

class SizeCache : public Cache {
public:
	void setThreshold(uint64_t threshold);

	int write(uint64_t offset, uint64_t length);
	int read(uint64_t offset, uint64_t length);

	std::string toString();

	std::string dump();

private:
	uint64_t m_threshold;
	std::map<uint64_t, uint64_t> m_intervals;
};
