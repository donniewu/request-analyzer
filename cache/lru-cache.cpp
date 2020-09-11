#include "lru-cache.h"
#include <algorithm>
#include <sstream>

#define ERR_MSG printf

LRUCache::LRUCache()
{
	m_max_usage = 1024 * 1024UL;
	m_prefetch_length = 8 * 1024 * 1024UL;
}

void LRUCache::setMaxUsage(uint64_t max_usage)
{
	m_max_usage = max_usage;
}

void LRUCache::setPrefetchLength(uint64_t prefetch_length)
{
	m_prefetch_length = prefetch_length;
}

int LRUCache::write(uint64_t offset, uint64_t length)
{
	m_stat.write_count += 1;
	m_stat.write_length += length;

	return 0;
}

int LRUCache::read(uint64_t offset, uint64_t length)
{
	m_stat.read_count += 1;
	m_stat.read_length += length;

	if (m_intervals.size() == 0) {
		ERR_MSG("\tnot found\n");
		ERR_MSG("\t%12lu %12lu x\n", offset, length);
		update(offset, length);
		return 0;
	}

	auto it = m_map.lower_bound(offset);

	if (it != m_map.begin()) {
		it = std::prev(it);
	}

	Interval &interval = *(it->second);

	if (interval.offset > offset || interval.offset + interval.length < offset + length) {
		ERR_MSG("\tfound but exceed\n");
		ERR_MSG("\t%12lu %12lu x\n", offset, length);
		update(offset, length);
		return 0;
	}

	ERR_MSG("\t%12lu %12lu\n", offset, length);
	m_stat.read_hit_count += 1;
	m_stat.read_hit_length += length;

	return 1;
}

void LRUCache::rotate(uint64_t next_length)
{
	while (m_stat.usage + next_length > m_max_usage && m_intervals.size() > 0) {
		auto &last_interval = m_intervals.back();
		m_stat.usage -= last_interval.length;
		ERR_MSG("rotate %lu %lu\n", last_interval.offset, last_interval.length);

		m_map.erase(last_interval.offset);
		m_intervals.pop_back();
	}
}

void LRUCache::update(uint64_t offset, uint64_t length)
{
	//ERR_MSG("read %lu %lu\n", offset, length);
	length = std::max(length, m_prefetch_length);
	m_stat.write_update_count += 1;
	m_stat.write_update_length += length;

	auto map_it = m_map.find(offset);

	Interval new_interval;

	if (map_it == m_map.end()) {
		new_interval.offset = offset;
		new_interval.length = length;
	} else {
		Interval old_interval = *(map_it->second);

		m_intervals.erase(map_it->second);
		m_map.erase(map_it);

		m_stat.usage -= old_interval.length;

		new_interval.offset = offset;
		new_interval.length = std::max(old_interval.length, length);
	}

	rotate(new_interval.length);
	m_intervals.push_front(new_interval);
	m_map[offset] = m_intervals.begin();

	m_stat.usage += new_interval.length;
}

std::string LRUCache::toString()
{
	std::stringstream ss;
	ss << "LRUCache\n";
	ss << "m_max_usage: " << m_max_usage << "\n";

	return ss.str();
}

std::string LRUCache::dump()
{
	return "";
}

