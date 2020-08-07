#pragma once

#include <stdint.h>
#include <string>
#include <vector>

class BitmapContext
{
public:
	BitmapContext();

	int initialize(std::vector<char> &&m_bitmap, uint64_t block_size);

	uint64_t getBlockSize() const;
	const char* getBitmap() const;
	size_t getBitmapLength() const;

	double getMeaningfulPercentage() const;
	bool isIntervalMeaningful(uint64_t offset, uint64_t length) const;
	bool setIntervalMeaningful(uint64_t offset, uint64_t length);

	int dumpToFile(const std::string &file_path);

	void resizeBlockSize(uint64_t new_block_size);

	void andBitmap(const BitmapContext &context);
	void orBitmap(const BitmapContext &context);

protected:
	std::vector<char> m_bitmap;
	uint64_t m_block_size;
};
