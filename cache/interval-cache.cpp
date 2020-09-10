#include "interval-cache.h"
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define ERR_MSG printf

void IntervalFileCache::setIntervals(const std::vector<std::pair<uint64_t, uint64_t>> &intervals)
{
	m_data_length = 0;

	for (auto &interval : intervals) {
		IntervalInfo info;
		info.offset = interval.first;
		info.length = interval.second;
		info.cache_offset = m_data_length;

		m_intervals[info.offset] = info;

		m_data_length += info.length;
	}

	m_stat.usage = m_data_length;
}

int IntervalFileCache::write(uint64_t offset, uint64_t length)
{
	m_stat.write_count += 1;
	m_stat.write_length += length;

	for (auto &i : m_intervals) {
		auto &interval = i.second;

		uint64_t start = std::max(interval.offset, offset);
		uint64_t end = std::min(interval.offset + interval.length, offset + length);

		if (start >= end) {
			continue;
		}

		m_stat.write_update_count += 1;
		m_stat.write_update_length += end - start;
	}

	return 0;
}

int IntervalFileCache::read(uint64_t offset, uint64_t length)
{
	m_stat.read_count += 1;
	m_stat.read_length += length;

	for (auto &i : m_intervals) {
		auto &interval = i.second;

		if (interval.offset > offset || interval.offset + interval.length < offset + length) {
			continue;
		}

		m_stat.read_hit_count += 1;
		m_stat.read_hit_length += length;
		return 1;
	}

	return 0;
}

std::string IntervalFileCache::toString()
{
	std::stringstream ss;

	ss << "IntervalFileCache" << "\n";

	for (auto &interval : m_intervals) {
		ss << interval.second.toString() << "\n";
	}

	return ss.str();
}
