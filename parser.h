#pragma once

#include "extents.h"

struct Request;

int parseKernelFile(const std::string &file_name, std::vector<Request>& requests);
int parseUserFile(const std::string &file_name, std::vector<Request>& requests);

int parseFile(bool kernel, const std::string &file_name, std::vector<Request>& requests);

int parseLayout(const std::string &file_name, Extents &extents);

