#include "gtest/gtest.h"

#include "Cpu.h"

struct CpuTestEndianness : public ::testing::Test {
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestEndianness() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;
};

TEST_F(CpuTestEndianness, SetGetWord) {
    Word w;
    EXPECT_NO_THROW(w = cpu.ReadWordAt(3) + 1);
    EXPECT_NO_THROW(cpu.WriteWordAt(3, w));
    EXPECT_EQ(w, cpu.ReadWordAt(3));
}

TEST_F(CpuTestEndianness, LittleEndianWord) {
    cpu.WriteWordAt(3, Word{ 0xBEEF });
    EXPECT_EQ(Byte{ 0xEF }, cpu.Memory.GetByteAt(3));
    EXPECT_EQ(Byte{ 0xBE }, cpu.Memory.GetByteAt(4));
}
