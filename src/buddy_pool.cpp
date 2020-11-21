#include "buddy_pool.h"
#include <array>
#include <cassert>
#define NODE_UNUSED  0
#define NODE_USED  1
#define NODE_SPLIT  2
#define NODE_FULL  3

BuddyPool::BuddyPool()
{
	max_level_ = 0;
	max_size_ = 0;
}

bool BuddyPool::IsPowerOf2(std::uint32_t addr)
{
	if (addr == 0)
	{
		return false;
	}
	return (addr & (addr - 1)) == 0;
}
std::uint32_t BuddyPool::LogOf2(std::uint32_t addr)
{
	static std::array<std::uint32_t, 32> exp_2_markers =
	{
		1 << 0,
		1 << 1,
		1 << 2,
		1 << 3,
		1 << 4,
		1 << 5,
		1 << 6,
		1 << 7,
		1 << (0 + 8),
		1 << (1 + 8),
		1 << (2 + 8),
		1 << (3 + 8),
		1 << (4 + 8),
		1 << (5 + 8),
		1 << (6 + 8),
		1 << (7 + 8),
		1 << (0 + 16),
		1 << (1 + 16),
		1 << (2 + 16),
		1 << (3 + 16),
		1 << (4 + 16),
		1 << (5 + 16),
		1 << (6 + 16),
		1 << (7 + 16),
		1 << (0 + 24),
		1 << (1 + 24),
		1 << (2 + 24),
		1 << (3 + 24),
		1 << (4 + 24),
		1 << (5 + 24),
		1 << (6 + 24),
		1 << (7 + 24),

	};
	for (auto i = 0; i < 32; i++)
	{
		if (exp_2_markers[i] == addr)
		{
			return i;
		}
	}
	assert(false);
	return 0;
}
std::uint32_t BuddyPool::NextPowerOf2(std::uint32_t x)
{
	if (IsPowerOf2(x))
		return x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}
void BuddyPool::Init(std::uint32_t max_size)
{
	max_size_ = max_size;
	max_level_ = LogOf2(NextPowerOf2(max_size)) + 1;
	node_status_.clear();
	node_status_.reserve(max_level_);

	for (std::uint32_t i = 0; i < max_level_; i++)
	{
		node_status_.push_back(std::vector<std::uint8_t>());
	}
	std::uint32_t cur_level = 0;
	for (std::uint32_t i = 0; i < max_level_; i++)
	{
		node_status_[i].swap(std::vector<std::uint8_t>(static_cast<std::size_t>(1u << (max_level_ - i - 1u)), 0));
	}
	InitUsed();
}
void BuddyPool::InitUsed()
{
	if (IsPowerOf2(max_size_))
	{
		return;
	}
	// 需要标记右边的一部分为已经占用
	std::uint32_t cur_addr = max_size_;
	std::uint32_t cur_level = 0;
	// 找到以这个节点为最左子节点的最高父节点
	while (cur_addr == ((cur_addr) / 2) * 2)
	{
		cur_level++;
		cur_addr = cur_addr / 2;
	}
	// 标记这个节点为被使用
	node_status_[cur_level][cur_addr] = NODE_USED;
	cur_level++;
	cur_addr = cur_addr / 2;
	// 然后递归标记为split
	while (cur_level < max_level_)
	{
		node_status_[cur_level][cur_addr] = NODE_SPLIT;
		if ((cur_addr % 2 != 1) && (cur_level != max_level_ - 1))
		{
			node_status_[cur_level][cur_addr + 1] = NODE_USED;

		}
		cur_level++;
		cur_addr = cur_addr / 2;
	}
}
bool BuddyPool::MarkUsed(std::uint32_t addr, std::uint32_t sz)
{
	if (!IsPowerOf2(sz))
	{
		return false;
	}
	if (addr + sz > max_size_)
	{
		return false;
	}
	if (addr % sz != 0)
	{
		return false;
	}
	auto cur_idx = addr / sz;
	auto cur_level = LogOf2(sz);
	if (node_status_[cur_level][cur_idx] != NODE_UNUSED)
	{
		return false;
		
	}
	node_status_[cur_level][cur_idx] = NODE_USED;
	return MarkParent(cur_level + 1, cur_idx / 2);

}
bool BuddyPool::MarkParent(std::uint32_t level, std::uint32_t index)
{
	while (level < max_level_)
	{
		std::uint8_t left_flag = node_status_[level - 1][index * 2];
		std::uint8_t right_flag = node_status_[level - 1][index * 2 + 1];
		auto pre_flag = node_status_[level][index];
		if (pre_flag == NODE_FULL || pre_flag == NODE_USED)
		{
			return false;
		}
		if (pre_flag == NODE_UNUSED)
		{
			node_status_[level][index] = NODE_SPLIT;
			level++;
			index /= 2;
			continue;
		}
		if ((left_flag == NODE_FULL || left_flag == NODE_USED) && (right_flag == NODE_FULL || right_flag == NODE_USED))
		{
			node_status_[level][index] = NODE_FULL;
			level++;
			index /= 2;
			continue;
		}
		if (pre_flag == NODE_SPLIT)
		{
			return true;
		}
	}
	return true;
	
}

std::int64_t BuddyPool::Allocate(std::uint32_t sz)
{
	if (!IsPowerOf2(sz))
	{
		return -1;
	}
	std::uint32_t index = 0;
	std::uint32_t level = max_level_ - 1;
	std::uint32_t length = 1 << level;
	if (sz > length)
	{
		return -1;
	}
	while (true)
	{
		if (sz == length)
		{
			if (node_status_[level][index] == NODE_UNUSED)
			{
				// 找到了一个可用块 标记为已经使用
				node_status_[level][index] = NODE_USED;
				auto result = index * length;
				// 通知父节点检查状态更新
				while (level < max_level_ - 1)
				{
					auto buddy = 4 * (index / 2) + 1 - index;
					auto buddy_flag = node_status_[level][buddy];
					// 如果兄弟节点也被完全使用了 通知父节点改为full 然后递归修改
					if (buddy_flag == NODE_USED || buddy_flag == NODE_FULL)
					{
						node_status_[level + 1][index / 2] = NODE_FULL;
					}
					else
					{
						break;
					}
					level++;
					index = index / 2;
				}
				return result;
			}
		}
		else
		{
			// 这个分支 length 一定是大于sz的
			switch (node_status_[level][index])
			{
			case NODE_FULL:
			case NODE_USED:
				// 当前节点已经不可用
				break;
			case NODE_UNUSED:
				// 先分裂 然后再走default逻辑
				// 所以这里不能break
				node_status_[level][index] = NODE_SPLIT;
				node_status_[level - 1][index * 2] = NODE_UNUSED;
				node_status_[level - 1][index * 2 + 1] = NODE_UNUSED;
			default:
				// 下沉到下一级的左子节点
				length = length / 2;
				index = index * 2;
				level--;
				continue;
			}
		}
		if (index % 2 == 0)
		{
			if (level == max_level_ - 1)
			{
				return -1;
			}
			// 如果当前节点是父节点的左子节点 则尝试去寻找父节点的右子节点
			index++;
			
			continue;
		}
		// 下面这个就是回溯过程，找到他的父节点
		// 如果这个父节点是祖父节点的右子节点 则继续回溯
		// 直到父节点是祖父节点的左子节点 此时index++ 去查询右子节点是否可以分配
		// 如果回溯到了最顶层 则失败
		for (;;)
		{
			level++;
			length *= 2;
			if (level >= max_level_ - 1)
			{
				// 到达最顶层 返回false
				return -1;
			}
			// 获取父节点
			index = index / 2;
			if (index % 2 == 0)
			{
				// 如果父节点是祖父的左子节点 则尝试祖父的右子节点
				index++;
				break;
			}
			// 否则继续回溯到祖父节点
		}
	}
	return -1;
}
bool BuddyPool::DeAllocate(std::uint32_t addr, std::uint32_t sz)
{
	if (!IsPowerOf2(sz))
	{
		return false;
	}
	if (addr % sz != 0)
	{
		return false;
	}
	auto cur_level = LogOf2(sz);
	auto cur_index = addr / sz;
	if (node_status_[cur_level][cur_index] != NODE_USED)
	{
		return false;
	}
	node_status_[cur_level][cur_index] = NODE_UNUSED;
	
	while (cur_level < max_level_ - 1)
	{
		cur_level++;
		cur_index /= 2;
		auto pre_flag = node_status_[cur_level][cur_index];
		if (pre_flag == NODE_USED || pre_flag == NODE_UNUSED)
		{
			return false;
		}
		if (pre_flag == NODE_FULL)
		{
			node_status_[cur_level][cur_index] = NODE_SPLIT;
			continue;
		}
		// 在split 状态
		auto left_flag = node_status_[cur_level - 1][cur_index * 2];
		auto right_flag = node_status_[cur_level - 1][cur_index * 2 + 1];
		if (left_flag == NODE_UNUSED && right_flag == NODE_UNUSED)
		{
			node_status_[cur_level][cur_index] = NODE_UNUSED;
			continue;
		}
		else
		{
			// 维持split状态
			return true;
		}
	}
	return true;
}


