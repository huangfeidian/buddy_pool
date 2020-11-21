#pragma once
#include <vector>
#include <cstdint>

class BuddyPool
{
private:
	std::vector<std::vector<std::uint8_t>> node_status_;
	std::uint32_t max_size_;
	std::uint32_t max_level_;
	bool MarkParent(std::uint32_t level, std::uint32_t index);
	void InitUsed();
public:
	BuddyPool();
	void Init(std::uint32_t max_size);
	bool MarkUsed(std::uint32_t addr, std::uint32_t sz);
	std::int64_t Allocate(std::uint32_t sz);
	bool DeAllocate(std::uint32_t addr, std::uint32_t sz);
	static bool IsPowerOf2(std::uint32_t addr);
	static std::uint32_t LogOf2(std::uint32_t addr);
	static std::uint32_t NextPowerOf2(std::uint32_t addr);
};