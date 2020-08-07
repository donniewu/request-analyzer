#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <map>
#include <sstream>
#include "bitmap-context.h"
#include "parser.h"
#include "request.h"

#define KERNEL true
#define FIRST_BACKUP "first"
#define SECOND_BACKUP "second"

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
	std::vector<char> cache;

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

	onCache(reqs12);
	analyze1(reqs12);

    return 0;
}
