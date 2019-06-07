#include "gtest/gtest.h"

#include "Cpu.h"
#include "BitUtil.h"

//#include <sstream>
//#include <vector>
//#include <map>
//#include <array>
//#include <functional>

using namespace std;
using namespace Instructions;
using namespace Addressing;

class CpuTestUnofficial : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestUnofficial() : memory(), cpu("6502", &memory) {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
    }

    MemoryBlock<0x1000> memory;
    Cpu cpu;

    void Test_SLO(Word address, Opcode op) {
        auto tester = [&](Byte a, Byte m, Byte expA, Byte expM, Flag expN, Flag expZ, Flag expC) {
            cpu.WriteByteAt(address, m);
            cpu.A = a;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(address));
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expN, cpu.N);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expC, cpu.C);
        };

        tester(0x00, 0x00, 0x00, 0x00, 0, 1, 0);
        tester(0x80, 0x00, 0x80, 0x00, 1, 0, 0);
        tester(0x00, 0x40, 0x80, 0x80, 1, 0, 0);
        tester(0x10, 0x80, 0x10, 0x00, 0, 0, 1);
    }

    void Test_RLA(Word address, Opcode op) {
        auto tester = [&](Byte a, Byte m, Byte c, Byte expA, Byte expM, Flag expN, Flag expZ, Flag expC) {
            cpu.WriteByteAt(address, m);
            cpu.A = a;
            cpu.C = c;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(address));
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expN, cpu.N);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expC, cpu.C);
        };

        tester(0x00, 0x10, 0, 0x00, 0x20, 0, 1, 0); // Shift
        tester(0x00, 0x10, 1, 0x00, 0x21, 0, 1, 0); // Shift w/ Carry
        tester(0x00, 0x80, 0, 0x00, 0x00, 0, 1, 1); // Shift w/ Carry out
        tester(0x08, 0x0F, 0, 0x08, 0x1E, 0, 0, 0); // Positive
        tester(0xFF, 0x40, 0, 0x80, 0x80, 1, 0, 0); // Negative
        tester(0xFF, 0x00, 0, 0x00, 0x00, 0, 1, 0); // Zero
    }
};
const Word CpuTestUnofficial::BASE_PC;
const int CpuTestUnofficial::BASE_TICKS;

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uNOP) {
    cpu.Execute(Opcode(uNOP, Implicit, 1, 2));

    EXPECT_EQ(BASE_PC + 1, cpu.PC);
    EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSTP) {
    cpu.Execute(Opcode(uSTP, Implicit, 1, 2));

    EXPECT_FALSE(cpu.IsAlive);

    // Check that the CPU is dead
    cpu.Execute(Opcode(uNOP, Implicit, 1, 2));
    EXPECT_EQ(BASE_PC, cpu.PC);
    EXPECT_EQ(BASE_TICKS, cpu.Ticks);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSLO_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_SLO(0x0020, Opcode(uSLO, ZeroPage, 2, 5));
}

TEST_F(CpuTestUnofficial, uSLO_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_SLO(0x0028, Opcode(uSLO, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uSLO_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;
    Test_SLO(0x0000, Opcode(uSLO, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uSLO_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_SLO(0x0120, Opcode(uSLO, Absolute, 3, 6));
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_SLO(0x0128, Opcode(uSLO, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_SLO(0x0210, Opcode(uSLO, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_SLO(0x0128, Opcode(uSLO, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_SLO(0x0210, Opcode(uSLO, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uSLO_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_SLO(0x0120, Opcode(uSLO, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uSLO_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_SLO(0x0120, Opcode(uSLO, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_SLO(0x0128, Opcode(uSLO, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_SLO(0x0210, Opcode(uSLO, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_SLO(0x000F, Opcode(uSLO, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;
    Test_SLO(0x0130, Opcode(uSLO, IndirectIndexed, 2, 8));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, ANC_Immediate) {
    Opcode op = Opcode(uANC, Immediate, 2, 2);
    
    auto tester = [&](Byte a, Byte m, Byte expA, Byte expZ, Byte expN, Byte expC) {
        cpu.WriteByteAt(BASE_PC, 0xFF);
        cpu.WriteByteAt(BASE_PC + 1, m);
        cpu.A = a;

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expA, cpu.A);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expN, cpu.N);
        EXPECT_EQ(expC, cpu.C);
    };

    tester(0x0F, 0x00, 0x00, 1, 0, 0);
    tester(0x80, 0xFF, 0x80, 0, 1, 1);
    tester(0x44, 0x24, 0x04, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uRLA_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_RLA(0x0020, Opcode(uRLA, ZeroPage, 2, 5));
}

TEST_F(CpuTestUnofficial, uRLA_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_RLA(0x0028, Opcode(uRLA, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uRLA_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;
    Test_RLA(0x0000, Opcode(uRLA, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uRLA_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_RLA(0x0120, Opcode(uRLA, Absolute, 3, 6));
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_RLA(0x0128, Opcode(uRLA, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_RLA(0x0210, Opcode(uRLA, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_RLA(0x0128, Opcode(uRLA, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_RLA(0x0210, Opcode(uRLA, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uRLA_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_RLA(0x0120, Opcode(uRLA, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uRLA_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_RLA(0x0120, Opcode(uRLA, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_RLA(0x0128, Opcode(uRLA, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_RLA(0x0210, Opcode(uRLA, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_RLA(0x000F, Opcode(uRLA, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;
    Test_RLA(0x0130, Opcode(uRLA, IndirectIndexed, 2, 8));
}
