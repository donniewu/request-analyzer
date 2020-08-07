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
#include "request.h"

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

