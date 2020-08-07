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
#include "bitmap-context.h"
#include "parser.h"
#include "request.h"

#define KERNEL true
#define FIRST_BACKUP "first"
#define SECOND_BACKUP "second"

#define BLOCK_DEVICE_SIZE (64 * 1024 * 1024 * 1024ULL)
#define CACHE_SLOT_SIZE (4 * 1024)
#define CACHE_THRESHOLD (64 * 1024)
#define PREFETCH_SIZE (4 * 1024 * 1024)

#define SLOT_SIZE (8 * 1024 * 1024)

void requestStat(const std::vector<Request>& requests)
{
	uint64_t cached_size = 0;

	for (auto &req : requests) {
		if (req.cached == true) {
			cached_size += req.length;
		}
	}

	printf("cached size is %llu\n", cached_size);
}

void analyze(const std::vector<Request>& requests)
{
	std::map<int, std::vector<Request>> stat;

	for (int i = 0; i < requests.size(); i++) {
		auto &req = requests[i];

		if (req.action == 1) {
			continue;
		}

		stat[req.offset / SLOT_SIZE].push_back(req);
	}

	for (auto it = stat.begin(); it != stat.end(); it++) {
		printf("%d %d\n", it->first, it->second.size());
	}
}

void analyze1(const std::vector<Request>& requests)
{
	for (int i = 0; i < requests.size(); i++) {
		auto &req = requests[i];

		if (req.action == 1) {
			continue;
		}

		bool found = false;
		Request last_req;

		for (int j = req.seq - 1; j >= 0; j--) {
			auto &lr = requests[j];

			if (lr.action == 0) {
				continue;
			}

			if (req.offset >= lr.offset && req.offset + req.length <= lr.offset + lr.length) {
				found = true;
				last_req = lr;
				break;
			}
		}

		if (found) {
			if (last_req.cached == false) {
				printf("read %s, last write %s\n", req.toString().c_str(), last_req.toString().c_str());
			}
		} else {
			printf("read %s, not found\n", req.toString().c_str());
		}
	}
}

void analyze2(const std::vector<Request>& requests)
{
	BitmapContext cache;

	uint64_t cache_length = (BLOCK_DEVICE_SIZE / CACHE_SLOT_SIZE) / 8;

	if (((BLOCK_DEVICE_SIZE / CACHE_SLOT_SIZE) % 8) != 0) {
		cache_length += 1;
	}

	std::vector<char> bitmap;
	bitmap.resize(cache_length, 0);

	if (cache.initialize(std::move(bitmap), CACHE_SLOT_SIZE) < 0) {
		printf("Failed to initialize cache\n");
		return;
	}

	int read_count = 0;
	int read_1m_count = 0;
	uint64_t read_1m_size = 0;

	std::set<uint64_t> request_M;

	for (int i = 0; i < requests.size(); i++) {
		auto &req = requests[i];

		read_count++;

		if (req.action == 0) {
			if (cache.getIntervalType(req.offset, req.length) == BitmapContext::ALL_ONE) {
				//printf("read %s\n", req.toString().c_str());
				//hit
			} else {
				printf("read %s not hit\n", req.toString().c_str());
				request_M.insert(req.offset / PREFETCH_SIZE);
			}
		}

		if (req.action == 1) {
			if (req.length <= CACHE_THRESHOLD) {
				read_1m_count++;
				read_1m_size += req.length;

				cache.setInterval(req.offset, req.length, false);
			} else if (req.length >= (32 * 1024 * 1024)) {
				cache.setInterval(req.offset, req.length, false);
			} else {
				cache.setInterval(req.offset, req.length, true);
			}
		}
	}

	printf("%f\n", cache.getMeaningfulPercentage());
	printf("%d %d %llu\n", read_count, read_1m_count, read_1m_size);
	printf("set size is %d\n", request_M.size());
}

void onCache(std::vector<Request>& requests)
{
	for (auto &req : requests) {
		if (req.length <= (1 * 1024 * 1024)) {
			req.cached = true;
		}
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

	int seq_offset = reqs1.back().seq;

	for (auto &r : reqs2) {
		r.seq += seq_offset;
	}

	std::vector<Request> reqs12;

	reqs12 = reqs1;
	reqs12.insert(reqs12.end(), reqs2.begin(), reqs2.end());

	analyze2(reqs1);

    return 0;
}
