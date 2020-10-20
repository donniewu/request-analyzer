#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <map>
#include <set>
#include <sstream>
#include <memory>
#include "parser.h"
#include "request.h"
#include "cache/interval-cache.h"
#include "cache/size-cache.h"
#include "cache/lru-cache.h"

#define YELLOW "\033[1;33m"
#define NONE "\033[m"

#define KERNEL true
#define FIRST_BACKUP "first"
#define SECOND_BACKUP "second"

#define BLOCK_DEVICE_SIZE ((uint64_t)((double) 150 * 1024 * 1024 * 1024ULL))

#define STAT_SIZE (100 * 1024 * 1024)

void analyze3(std::vector<Request> &reqs)
{
	std::map<uint64_t, int> stat;

	for (uint64_t i = 0; i < BLOCK_DEVICE_SIZE / STAT_SIZE; i++) {
		stat[i] = 0;
	}

	for (auto &req : reqs) {
		if (req.action == 0) {
			int part = req.offset / STAT_SIZE;
			stat[part] += 1;
		}
	}

	for (uint64_t i = 0; i < BLOCK_DEVICE_SIZE / STAT_SIZE; i++) {
		printf("%d\n", stat[i]);
	}
}

void analyze4(std::vector<Request> &reqs)
{
	int o_0_4_count = 0;
	int o4_64_count = 0;
	int o64_x_count = 0;

	int normal_count = 0;
	int strange_count = 0;

	for (auto &req : reqs) {
		if (req.action == 0) {
			continue;
		}

		if (req.length < 4 * 1024) {
			o_0_4_count += 1;
		} else if (req.length <= 64 * 1024) {
			o4_64_count += 1;
		} else {
			o64_x_count += 1;
		}

		if (req.offset % 4096 == 0 && req.length % 4096 == 0) {
			normal_count += 1;
		} else {
			strange_count += 1;
		}
	}

	printf("%d %d %d, %d %d\n", o_0_4_count, o4_64_count, o64_x_count, normal_count, strange_count);
}

// cache analyze
void analyze5(std::vector<Request> &reqs)
{
	int container_size_GB = 1024;
	uint64_t offset_1 = container_size_GB * 4 + 1736;

	std::vector<std::pair<uint64_t, uint64_t>> intervals;
	intervals.push_back(std::make_pair(0, 3));
	intervals.push_back(std::make_pair(200, 200));
	intervals.push_back(std::make_pair(container_size_GB * 4 + 1736, 300));
	intervals.push_back(std::make_pair(container_size_GB * 8 + 3143, 100));

	for (auto &i : intervals) {
		i.first *= (1024 * 1024);
		i.second *= (1024 * 1024);
	}

	IntervalFileCache* interval_cache = new IntervalFileCache;
	interval_cache->setIntervals(intervals);

	SizeCache* size_cache = new SizeCache;
	size_cache->setThreshold(64 * 1024);

	LRUCache* lru_cache = new LRUCache;
	lru_cache->setMaxUsage(1 * 1024 * 1024 * 1024ULL);
	lru_cache->setPrefetchLength(4 * 1024 * 1024UL);

	std::vector<Cache*> caches;
	caches.push_back(interval_cache);
	caches.push_back(size_cache);
	caches.push_back(lru_cache);

	printf(YELLOW "Apply caches:\n" NONE);

	for (auto &c : caches) {
		printf("%s\n", c->toString().c_str());
	}

	CacheStat cs;

	printf(YELLOW "Start to process all requests...\n" NONE);

	for (auto &req : reqs) {
		if (req.action == 0) {
			cs.read_count += 1;
			cs.read_length += req.length;

			bool hit = false;

			for (auto &c : caches) {
				if (c->read(req.offset, req.length) == 1) {
					hit = true;
					break;
				}
			}

			if (hit) {
				cs.read_hit_count += 1;
				cs.read_hit_length += req.length;
			} else {
				printf("%lu %lu not hit\n", req.offset, req.length);
			}
		} else {
			cs.write_count += 1;
			cs.write_length += req.length;

			for (auto &c : caches) {
				c->write(req.offset, req.length);
			}
		}
	}

	printf(YELLOW "Stat:\n" NONE);

	for (auto &c : caches) {
		printf("*******************************\n");
		printf("%s\n%s\n", c->toString().c_str(), c->statToString().c_str());
	}

	printf(YELLOW "Send to server stat:\n" NONE);
	printf("%s\n", cs.toString().c_str());

END:
	if (interval_cache != nullptr) {
		delete interval_cache;
	}

	if (size_cache != nullptr) {
		delete size_cache;
	}

	if (lru_cache != nullptr) {
		delete lru_cache;
	}
}

int main()
{
    std::vector<Request> reqs1;

    if (parseFile(KERNEL, FIRST_BACKUP, reqs1) < 0) {
        printf("parsefile failed\n");
        return -1;
    }

	std::vector<Request> reqs2;

    if (parseFile(KERNEL, SECOND_BACKUP, reqs2) < 0) {
		printf("parsefile failed\n");
        return -1;
    }

	int seq_offset;

	if (reqs1.size() == 0) {
		seq_offset = 0;
	} else {
		seq_offset = reqs1.back().seq;
	}

	for (auto &r : reqs2) {
		r.seq += seq_offset;
	}

	std::vector<Request> reqs12;

	reqs12 = reqs1;
	reqs12.insert(reqs12.end(), reqs2.begin(), reqs2.end());

	//analyze2(reqs1);
	//analyze2(reqs12);
	analyze3(reqs12);

    return 0;
}
