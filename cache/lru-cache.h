#pragma once

#include "cache.h"
#include <map>
#include <list>

class LRUCache : public Cache {
public:
	LRUCache();

	void setMaxUsage(uint64_t max_usage);
	void setPrefetchLength(uint64_t prefetch_length);

	int write(uint64_t offset, uint64_t length);
	int read(uint64_t offset, uint64_t length);

	void rotate(uint64_t next_length);
	void update(uint64_t offset, uint64_t length);

	std::string toString();

	std::string dump();

private:
	struct Interval {
		uint64_t offset;
		uint64_t length;
	};

	uint64_t m_max_usage;
	uint64_t m_prefetch_length;
	std::map<uint64_t, std::list<Interval>::iterator> m_map;
	std::list<Interval> m_intervals;
};
