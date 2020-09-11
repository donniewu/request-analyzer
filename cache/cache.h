#pragma once
#include <string>

struct CacheStat {
	CacheStat();

	int write_count;
	uint64_t write_length;

	int write_update_count;
	uint64_t write_update_length;

	int read_count;
	uint64_t read_length;

	int read_hit_count;
	uint64_t read_hit_length;

	int64_t usage;

	std::string toString();
};

class Cache {
public:
	Cache();

	virtual int write(uint64_t offset, uint64_t length) = 0;
	virtual int read(uint64_t offset, uint64_t length) = 0;

	virtual std::string toString() = 0;
	std::string statToString();

protected:
	CacheStat m_stat;
};
