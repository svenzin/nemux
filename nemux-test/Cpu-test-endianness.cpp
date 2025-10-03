#include "CpuBaseTest.h"

struct CpuTestEndianness : public CpuBaseTest {
};

TEST_F(CpuTestEndianness, SetGetWord) {
    Word w;
    EXPECT_NO_THROW(w = cpu.ReadWordAt(3) + 1);
    EXPECT_NO_THROW(cpu.WriteWordAt(3, w));
    EXPECT_EQ(w, cpu.ReadWordAt(3));
}

TEST_F(CpuTestEndianness, LittleEndianWord) {
    cpu.WriteWordAt(3, Word{ 0xBEEF });
    EXPECT_EQ(Byte{ 0xEF }, cpu.ReadByte(3));
    EXPECT_EQ(Byte{ 0xBE }, cpu.ReadByte(4));
}
