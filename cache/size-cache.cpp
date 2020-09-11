#include "size-cache.h"
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define ERR_MSG printf

void SizeCache::setThreshold(uint64_t threshold)
{
	m_threshold = threshold;
}

int SizeCache::write(uint64_t offset, uint64_t length)
{
	//ERR_MSG("write %lu %lu\n", offset, length);

	m_stat.write_count += 1;
	m_stat.write_length += length;

	if (length > m_threshold) {
		return 0;
	}

	m_stat.write_update_count += 1;
	m_stat.write_update_length += length;

	if (m_intervals.size() == 0) {
		m_stat.usage += length;
		m_intervals[offset] = length;
		return 0;
	}

	uint64_t cached_start = 0;
	uint64_t cached_end = 0;

	// find cached_start
	{
		auto it = m_intervals.lower_bound(offset);

		if (it == m_intervals.begin()) {
			cached_start = offset;
		} else {
			it = std::prev(it);

			if (it->first + it->second >= offset) {
				cached_start = it->first;
			} else {
				cached_start = offset;
			}
		}
	}

	// find cached_end
	{
		auto it = m_intervals.lower_bound(offset + length + 1);

		if (it == m_intervals.begin()) {
			cached_end = offset + length;
		} else {
			it = std::prev(it);
			cached_end = std::max(it->first + it->second, offset + length);
		}
	}

	// verify cached_start & cached_end
	{
		uint64_t verify_cached_start = offset;
		uint64_t verify_cached_end = offset + length;

		for (auto &i : m_intervals) {
			uint64_t left = std::min(verify_cached_start, i.first);
			uint64_t right = std::max(verify_cached_end, i.first + i.second);

			if (i.second + verify_cached_end - verify_cached_start >= right - left) {
				verify_cached_start = left;
				verify_cached_end = right;
			}
		}

		if (cached_start != verify_cached_start || cached_end != verify_cached_end) {
			printf("\tcc: (%lu %lu) (%lu %lu)\n", cached_start, cached_end, verify_cached_start, verify_cached_end);
			printf("\tcache_start or cached_end wrong\n");
			return -1;
		}
	}

	//printf("\tcc: %lu %lu\n", cached_start, cached_end);

	for (auto it = m_intervals.begin(); it != m_intervals.end();) {
		if (it->first >= cached_start && it->first + it->second <= cached_end) {
			m_stat.usage -= it->second;
			it = m_intervals.erase(it);
		} else {
			it++;
		}
	}

	m_stat.usage += cached_end - cached_start;
	m_intervals[cached_start] = cached_end - cached_start;

	return 0;
}

int SizeCache::read(uint64_t offset, uint64_t length)
{
	m_stat.read_count += 1;
	m_stat.read_length += length;

	for (auto &i : m_intervals) {
		if (i.first > offset || i.first + i.second < offset + length) {
			continue;
		}

		m_stat.read_hit_count += 1;
		m_stat.read_hit_length += length;
		return 1;
	}

	return 0;
}

std::string SizeCache::toString()
{
	std::stringstream ss;

	ss << "SizeCache" << "\n";
	ss << "m_threshold: " << m_threshold << "\n";

	return ss.str();
}

std::string SizeCache::dump()
{
	std::stringstream ss;

	for (auto &i : m_intervals) {
		ss << i.first << " " << i.first + i.second << "\n";
	}

	return ss.str();
}
