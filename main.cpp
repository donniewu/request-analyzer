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

#define KERNEL true
#define FIRST_BACKUP "first"
#define SECOND_BACKUP "second"

struct Request {
	int seq;
	int action;
    unsigned long long offset;
    unsigned long long length;

	bool cached;

	Request() {
		action = -1;
		seq = -1;
		cached = false;
	}

	std::string toString() const {
		std::stringstream ss;
		ss << seq << " " << action << " " << offset << " " << length << " " << cached;
		return ss.str();
	}
};

int parseKernelFile(const std::string &file_name, std::vector<Request>& requests)
{
    FILE* fp = NULL;
    char line[300];
    int ret = -1;
	int seq = 0;

    fp = fopen(file_name.c_str(), "r");

    if (fp == NULL) {
        printf("cannot open file %d\n", errno);
        goto end;
    }

    memset(line, 0, 300);

    while (fgets(line, sizeof(line), fp)) {
		seq += 1;

        char action[6] = {0};
        Request request;

		request.seq = seq;

        int g = 0;
        if ((g = sscanf(line, "%*s %*s %*s %*s %*s %*s %*s %*s req %*d %s %llu %llu", action, &request.offset, &request.length)) != 3) {
            printf("ERROR occur while parse string '%s', '%d'\n", line, g);
            printf("action '%s'\n", action);
            goto end;
        }

        if (strncmp("read", action, 4) == 0) {
			request.action = 0;
        } else if (strncmp("write", action, 5) == 0) {
			request.action = 1;
        } else {
            printf("?????\n");
            goto end;
        }

        memset(line, 0, 300);

		requests.push_back(request);
    }

    ret = 0;

end:
    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }

    return ret;
}

int parseUserFile(const std::string &file_name, std::vector<Request>& requests)
{
    FILE* fp = NULL;
    char line[300];
    int ret = -1;
	int seq = 0;

    fp = fopen(file_name.c_str(), "r");

    if (fp == NULL) {
        printf("cannot open file %d\n", errno);
        goto end;
    }

    memset(line, 0, 300);

    while (fgets(line, sizeof(line), fp)) {
		//printf("%s %d\n", line, seq);
		seq += 1;

        Request request;

        int g = 0;

        if ((g = sscanf(line, "%*s %*s %*s %*s %*s %*s =-=seq: %*d, send action: %d, offset:%llu, length:%llu", &request.action, &request.offset, &request.length)) != 3) {
            printf("ERROR occur while parse string '%s', '%d'\n", line, g);
            goto end;
        }

		request.seq = seq;

        requests.push_back(request);
        memset(line, 0, 300);
    }

    ret = 0;

end:
    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }

    return ret;
}

int parseFile(bool kernel, const std::string &file_name, std::vector<Request>& requests)
{
	if (kernel) {
		return parseKernelFile(file_name, requests);
	} else {
		return parseUserFile(file_name, requests);
	}
}

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
