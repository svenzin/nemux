#include "gtest/gtest.h"

#include "MemoryMap.h"

struct MemoryBlockTest : public ::testing::Test {
    MemoryBlock<0x0100> block;
};

TEST_F(MemoryBlockTest, SetGet) {
    block.SetByteAt(0x0080, 0xBE);
    block.SetByteAt(0x0081, 0xEF);
    EXPECT_EQ(0xBE, block.GetByteAt(0x0080));
    EXPECT_EQ(0xEF, block.GetByteAt(0x0081));
}
