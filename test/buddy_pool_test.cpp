#include "buddy_pool.h"
#include "gtest/gtest.h"

TEST(BuddyPool, sz_1024)
{
	
	std::uint32_t capacity = 1024;
	BuddyPool cur_pool;
	cur_pool.Init(capacity);
	for (std::uint32_t i = 0; i < capacity; i++)
	{
		ASSERT_EQ(cur_pool.Allocate(1), i);
	}
	ASSERT_EQ(cur_pool.Allocate(1), -1);
	for (std::uint32_t i = 0; i < capacity; i++)
	{
		ASSERT_TRUE(cur_pool.DeAllocate(i, 1));
	}
	ASSERT_EQ(cur_pool.Allocate(capacity), 0);
	ASSERT_EQ(cur_pool.Allocate(1), -1);
	ASSERT_TRUE(cur_pool.DeAllocate(0, capacity));

}

TEST(BuddyPool, sz_1023)
{

	std::uint32_t capacity = 1023;
	BuddyPool cur_pool;
	cur_pool.Init(capacity);
	for (std::uint32_t i = 0; i < capacity; i++)
	{
		ASSERT_EQ(cur_pool.Allocate(1), i);
	}
	ASSERT_EQ(cur_pool.Allocate(1), -1); 
	for (std::uint32_t i = 0; i < capacity; i++)
	{
		ASSERT_TRUE(cur_pool.DeAllocate(i, 1));
	}
	std::uint32_t block_sz = (capacity + 1) / 2;
	std::uint32_t base_addr = 0;
	while (block_sz > 0)
	{
		ASSERT_EQ(cur_pool.Allocate(block_sz), base_addr);
		base_addr += block_sz;
		block_sz /= 2;
	}
	block_sz = 1;
	while (block_sz < capacity)
	{
		base_addr -= block_sz;

		ASSERT_TRUE(cur_pool.DeAllocate(base_addr, block_sz));
		block_sz *= 2;
	}

}

TEST(BuddyPool, sz_513)
{

	std::uint32_t capacity = 513;
	BuddyPool cur_pool;
	cur_pool.Init(capacity);
	for (std::uint32_t i = 0; i < capacity; i++)
	{
		ASSERT_EQ(cur_pool.Allocate(1), i);
	}
	ASSERT_EQ(cur_pool.Allocate(1), -1);
	for (std::uint32_t i = 0; i < capacity; i++)
	{
		ASSERT_TRUE(cur_pool.DeAllocate(i, 1));
	}
	ASSERT_EQ(cur_pool.Allocate(capacity - 1), 0);
	ASSERT_EQ(cur_pool.Allocate(1), capacity - 1);
	ASSERT_EQ(cur_pool.Allocate(1), -1);
	ASSERT_TRUE(cur_pool.DeAllocate(0, capacity - 1));
	ASSERT_TRUE(cur_pool.DeAllocate(capacity - 1, 1));
}

TEST(BuddyPool, sz_used_1023)
{

	std::uint32_t capacity = 1023;
	BuddyPool cur_pool;
	cur_pool.Init(capacity + 1);
	ASSERT_TRUE(cur_pool.MarkUsed(capacity , 1));
	for (std::uint32_t i = 0; i < capacity; i++)
	{
		ASSERT_EQ(cur_pool.Allocate(1), i);
	}
	ASSERT_EQ(cur_pool.Allocate(1), -1);
	for (std::uint32_t i = 0; i < capacity; i++)
	{
		ASSERT_TRUE(cur_pool.DeAllocate(i, 1));
	}
	std::uint32_t block_sz = (capacity + 1) / 2;
	std::uint32_t base_addr = 0;
	while (block_sz > 0)
	{
		ASSERT_EQ(cur_pool.Allocate(block_sz), base_addr);
		base_addr += block_sz;
		block_sz /= 2;
	}
	block_sz = 1;
	while (block_sz < capacity)
	{
		base_addr -= block_sz;

		ASSERT_TRUE(cur_pool.DeAllocate(base_addr, block_sz));
		block_sz *= 2;
	}

}

TEST(BuddyPool, sz_used_513)
{

	std::uint32_t capacity = 513;
	BuddyPool cur_pool;
	cur_pool.Init((capacity - 1) * 2);
	std::uint32_t cur_block_sz = 256;
	std::uint32_t base_addr = (capacity - 1) * 2;
	while (cur_block_sz > 0)
	{
		base_addr -= cur_block_sz;
		ASSERT_TRUE(cur_pool.MarkUsed(base_addr, cur_block_sz));
		cur_block_sz /= 2;
	}
	for (std::uint32_t i = 0; i < capacity; i++)
	{
		ASSERT_EQ(cur_pool.Allocate(1), i);
	}
	ASSERT_EQ(cur_pool.Allocate(1), -1);
	for (std::uint32_t i = 0; i < capacity; i++)
	{
		ASSERT_TRUE(cur_pool.DeAllocate(i, 1));
	}
	ASSERT_EQ(cur_pool.Allocate(capacity - 1), 0);
	ASSERT_EQ(cur_pool.Allocate(1), capacity - 1);
	ASSERT_EQ(cur_pool.Allocate(1), -1);
	ASSERT_TRUE(cur_pool.DeAllocate(0, capacity - 1));
	ASSERT_TRUE(cur_pool.DeAllocate(capacity - 1, 1));
}

TEST(BuddyPool, sz_used_1024)
{

	std::uint32_t capacity = 1024;
	BuddyPool cur_pool;
	cur_pool.Init(capacity);
	cur_pool.MarkUsed(256 * 2, 256);
	for (std::uint32_t i = 0; i < capacity - 256; i++)
	{
		ASSERT_NE(cur_pool.Allocate(1), -1);
	}
	ASSERT_EQ(cur_pool.Allocate(1), -1);
	for (std::uint32_t i = 0; i < 256 * 2; i++)
	{
		ASSERT_TRUE(cur_pool.DeAllocate(i, 1));
	}
	for (std::uint32_t i = 256 * 3; i < capacity; i++)
	{
		ASSERT_TRUE(cur_pool.DeAllocate(i, 1));
	}
	for (std::uint32_t i = 0; i < 3; i++)
	{
		ASSERT_NE(cur_pool.Allocate(256), -1);

	}

	ASSERT_TRUE(cur_pool.DeAllocate(0, 256));
	ASSERT_TRUE(cur_pool.DeAllocate(256, 256));
	ASSERT_TRUE(cur_pool.DeAllocate(768, 256));



}