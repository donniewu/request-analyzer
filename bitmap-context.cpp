#include "bitmap-context.h"
#include <errno.h>
#include <algorithm>

#define ERR_MSG printf
#define INFO_MSG printf

BitmapContext::BitmapContext()
{
}

int BitmapContext::initialize(std::vector<char> &&bitmap, uint64_t block_size)
{
	m_bitmap = bitmap;
	m_block_size = block_size;
	return 0;
}

int BitmapContext::dumpToFile(const std::string &file_path)
{
	FILE *file = NULL;

#ifdef _WIN32
	utf_converter converter;
	file = _wfopen(converter.from_bytes(file_path).c_str(), L"wb");
#else
	file = fopen(file_path.c_str(), "wb");
#endif

	if (file == NULL) {
		ERR_MSG("Failed to open file '%s', %d\n", file_path.c_str(), errno);
		return -1;
	}

	fwrite(m_bitmap.data(), 1, m_bitmap.size(), file);

	fclose(file);

	return 0;
}

void BitmapContext::andBitmap(const BitmapContext &context)
{
	uint64_t new_length = std::max(m_bitmap.size(), context.getBitmapLength());
	std::vector<char> merged_bitmap(new_length, 0);

	for (uint64_t i = 0; i < new_length; i++) {
		char first = (i < m_bitmap.size()) ? m_bitmap[i] : ~0;
		char second = (i < context.getBitmapLength()) ? context.getBitmap()[i] : ~0;

		merged_bitmap[i] = (first & second);
	}

	m_bitmap.swap(merged_bitmap);
}

void BitmapContext::orBitmap(const BitmapContext &context)
{
	uint64_t new_length = std::max(m_bitmap.size(), context.getBitmapLength());
	std::vector<char> merged_bitmap(new_length, 0);

	for (uint64_t i = 0; i < new_length; i++) {
		char first = (i < m_bitmap.size()) ? m_bitmap[i] : 0;
		char second = (i < context.getBitmapLength()) ? context.getBitmap()[i] : 0;

		merged_bitmap[i] = (first | second);
	}

	m_bitmap.swap(merged_bitmap);
}

void BitmapContext::resizeBlockSize(uint64_t new_block_size)
{
	uint64_t new_length;
	uint64_t offset = 0;

	new_length = m_bitmap.size() * m_block_size / new_block_size;

	if (m_bitmap.size() * m_block_size % new_block_size != 0) {
		new_length++;
	}

	std::vector<char> new_bitmap(new_length, 0);

	for (size_t i = 0; i < new_length; i++) {
		for (size_t j = 0; j < 8; j++) {
			if (isIntervalMeaningful(offset, new_block_size)) {
				new_bitmap[i] |= (1 << j);
			}

			offset += new_block_size;
		}
	}

	m_bitmap.swap(new_bitmap);

	m_block_size = new_block_size;
}

uint64_t BitmapContext::getBlockSize() const
{
	return m_block_size;
}

const char* BitmapContext::getBitmap() const
{
	return m_bitmap.data();
}

size_t BitmapContext::getBitmapLength() const
{
	return m_bitmap.size();
}

double BitmapContext::getMeaningfulPercentage() const
{
	if (m_bitmap.size() == 0) {
		return 1;
	}

	int count = 0;

	for (size_t i = 0; i < m_bitmap.size(); i++) {
		for (int j = 0; j < 8; j++) {
			if (((m_bitmap[i] >> j) & 1) == 1) {
				count++;
			}
		}
	}

	return (count / ((double) m_bitmap.size() * 8));
}

bool BitmapContext::isIntervalMeaningful(uint64_t offset, uint64_t length) const
{
	uint64_t begin = offset;
	uint64_t end = offset + length;

	uint64_t start_block = begin / m_block_size;
	uint64_t end_block = end / m_block_size;

	if (end % m_block_size != 0) {
		end_block += 1;
	}

	if (end_block / 8 >= m_bitmap.size()) {
		INFO_MSG("request block %llu %llu is greater than that in bitmap (length %d)\n", offset, length, m_bitmap.size());
		return true;
	}

	for (uint64_t i = start_block; i < end_block; i++) {
		char a = m_bitmap[i / 8];

		if (((a >> (i % 8)) & 1) == 1) {
			return true;
		}
	}

	return false;
}
