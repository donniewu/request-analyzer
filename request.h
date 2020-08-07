#pragma once

#include <sstream>

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
		//ss << seq << " " << action << " " << offset << " " << length << " " << cached;
		ss << seq << " " << action << " " << offset << " " << length;
		return ss.str();
	}
};
