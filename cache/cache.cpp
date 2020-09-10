#include "cache.h"
#include <sstream>

CacheStat::CacheStat()
{
	write_count = 0;
	write_length = 0;

	write_update_count = 0;
	write_update_length = 0;

	read_count = 0;
	read_length = 0;

	read_hit_count = 0;
	read_hit_length = 0;

	usage = 0;
}

std::string CacheStat::toString()
{
	std::stringstream ss;
	ss << "write_count:" << write_count << "\n";
	ss << "write_length:" << write_length << "\n";
	ss << "write_update_count: " << write_update_count << "\n";
	ss << "write_update_length: " << write_update_length << "\n";
	ss << "* read_count: " << read_count << "\n";
	ss << "read_length: " << read_length << "\n";
	ss << "* read_hit_count: " << read_hit_count << "\n";
	ss << "read_hit_length: " << read_hit_length << "\n";
	ss << "hit rate: " << read_hit_count / (double) read_count << "\n";

	return ss.str();
}

Cache::Cache()
{
}

std::string Cache::statToString()
{
	return m_stat.toString();
}
