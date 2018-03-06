/*
 * Cpu-test-loadstore.cpp
 *
 *  Created on: 07 Jul 2013
 *      Author: scorder
 */

#include "gtest/gtest.h"

#include "Cpu.h"

#include <vector>
#include <map>
#include <array>
#include <functional>

using namespace std;
using namespace Instructions;
using namespace Addressing;

class CpuTestLoadStore : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestLoadStore() : cpu("6502", &memory) {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
    }

    MemoryBlock<0x400> memory;
    Cpu cpu;

    template<typename Getter, typename Setter>
    void Test_Set(Getter get, Setter set, Opcode op) {
        set(0x80);
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(0x80, get());
    }

    template<typename Getter, typename Setter>
    void Test_Load(Getter get, Setter set, Opcode op, int extra) {
        auto tester = [&] (Byte m, Flag expZ, Flag expN) {
            set(m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            EXPECT_EQ(m, get());
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x20, 0, 0);
        tester(0x00, 1, 0);
        tester(0x80, 0, 1);
    }

    function<void (Byte)> Setter(Byte & a) {
        return [&] (Byte value) { a = value; };
    }
    function<void (Byte)> Setter(Word a) {
        return [=] (Byte value) { cpu.WriteByteAt(a, value); };
    }
    function<Byte ()> Getter(Byte & b) {
        return [&] () { return b; };
    }
    function<Byte ()> Getter(Word a) {
        return [=] () { return cpu.ReadByteAt(a); };
    }
};

TEST_F(CpuTestLoadStore, LDX_Immediate) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    Test_Load(Getter(cpu.X), Setter(BASE_PC + 1),
              Opcode(LDX, Immediate, 2, 2), 0);
}

TEST_F(CpuTestLoadStore, LDX_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_Load(Getter(cpu.X), Setter(0x0020),
              Opcode(LDX, ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestLoadStore, LDX_ZeroPageY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.Y = 0x08;
    Test_Load(Getter(cpu.X), Setter(0x0028),
              Opcode(LDX, ZeroPageY, 2, 4), 0);
}

TEST_F(CpuTestLoadStore, LDX_ZeroPageY_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.Y = 0x10;
    Test_Load(Getter(cpu.X), Setter(0x0000),
              Opcode(LDX, ZeroPageY, 2, 4), 0);
}

TEST_F(CpuTestLoadStore, LDX_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_Load(Getter(cpu.X), Setter(0x0120),
              Opcode(LDX, Absolute, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDX_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_Load(Getter(cpu.X), Setter(0x0128),
              Opcode(LDX, AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDX_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_Load(Getter(cpu.X), Setter(0x0210),
              Opcode(LDX, AbsoluteY, 3, 4), 1);
}


TEST_F(CpuTestLoadStore, LDY_Immediate) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    Test_Load(Getter(cpu.Y), Setter(BASE_PC + 1),
              Opcode(LDY, Immediate, 2, 2), 0);
}

TEST_F(CpuTestLoadStore, LDY_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_Load(Getter(cpu.Y), Setter(0x0020),
              Opcode(LDY, ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestLoadStore, LDY_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_Load(Getter(cpu.Y), Setter(0x0028),
              Opcode(LDY, ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestLoadStore, LDY_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;
    Test_Load(Getter(cpu.Y), Setter(0x0000),
              Opcode(LDY, ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestLoadStore, LDY_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_Load(Getter(cpu.Y), Setter(0x0120),
              Opcode(LDY, Absolute, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDY_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_Load(Getter(cpu.Y), Setter(0x0128),
              Opcode(LDY, AbsoluteX, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDY_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_Load(Getter(cpu.Y), Setter(0x0210),
              Opcode(LDY, AbsoluteX, 3, 4), 1);
}

TEST_F(CpuTestLoadStore, LDA_Immediate) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    Test_Load(Getter(cpu.A), Setter(BASE_PC + 1),
              Opcode(LDA, Immediate, 2, 2), 0);
}

TEST_F(CpuTestLoadStore, LDA_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_Load(Getter(cpu.A), Setter(0x0020),
              Opcode(LDA, ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestLoadStore, LDA_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_Load(Getter(cpu.A), Setter(0x0028),
              Opcode(LDA, ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestLoadStore, LDA_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;
    Test_Load(Getter(cpu.A), Setter(0x0000),
              Opcode(LDA, ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestLoadStore, LDA_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_Load(Getter(cpu.A), Setter(0x0120),
              Opcode(LDA, Absolute, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_Load(Getter(cpu.A), Setter(0x0128),
              Opcode(LDA, AbsoluteX, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_Load(Getter(cpu.A), Setter(0x0210),
              Opcode(LDA, AbsoluteX, 3, 4), 1);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_Load(Getter(cpu.A), Setter(0x0128),
              Opcode(LDA, AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_Load(Getter(cpu.A), Setter(0x0210),
              Opcode(LDA, AbsoluteY, 3, 4), 1);
}

TEST_F(CpuTestLoadStore, LDA_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_Load(Getter(cpu.A), Setter(0x0120),
              Opcode(LDA, IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTestLoadStore, LDA_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_Load(Getter(cpu.A), Setter(0x0120),
              Opcode(LDA, IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTestLoadStore, LDA_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_Load(Getter(cpu.A), Setter(0x0128),
              Opcode(LDA, IndirectIndexed, 2, 5), 0);
}

TEST_F(CpuTestLoadStore, LDA_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_Load(Getter(cpu.A), Setter(0x0210),
        Opcode(LDA, IndirectIndexed, 2, 5), 1);
}

TEST_F(CpuTestLoadStore, LDA_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_Load(Getter(cpu.A), Setter(0x000F),
        Opcode(LDA, IndirectIndexed, 2, 5), 1);
}

TEST_F(CpuTestLoadStore, STX_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);

    Test_Set(Getter(0x0020), Setter(cpu.X),
             Opcode(STX, ZeroPage, 2, 3));
}

TEST_F(CpuTestLoadStore, STX_ZeroPageY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.Y = 0x08;

    Test_Set(Getter(0x0028), Setter(cpu.X),
             Opcode(STX, ZeroPageY, 2, 4));
}

TEST_F(CpuTestLoadStore, STX_ZeroPageY_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.Y = 0x10;

    Test_Set(Getter(0x0000), Setter(cpu.X),
             Opcode(STX, ZeroPageY, 2, 4));
}

TEST_F(CpuTestLoadStore, STX_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_Set(Getter(0x0120), Setter(cpu.X),
             Opcode(STX, Absolute, 3, 4));
}

TEST_F(CpuTestLoadStore, STY_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);

    Test_Set(Getter(0x0020), Setter(cpu.Y),
             Opcode(STY, ZeroPage, 2, 3));
}

TEST_F(CpuTestLoadStore, STY_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_Set(Getter(0x0028), Setter(cpu.Y),
             Opcode(STY, ZeroPageX, 2, 4));
}

TEST_F(CpuTestLoadStore, STY_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;

    Test_Set(Getter(0x0000), Setter(cpu.Y),
             Opcode(STY, ZeroPageX, 2, 4));
}

TEST_F(CpuTestLoadStore, STY_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_Set(Getter(0x0120), Setter(cpu.Y),
             Opcode(STY, Absolute, 3, 4));
}

TEST_F(CpuTestLoadStore, STA_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);

    Test_Set(Getter(0x0020), Setter(cpu.A),
             Opcode(STA, ZeroPage, 2, 3));
}

TEST_F(CpuTestLoadStore, STA_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_Set(Getter(0x0028), Setter(cpu.A),
             Opcode(STA, ZeroPageX, 2, 4));
}

TEST_F(CpuTestLoadStore, STA_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;

    Test_Set(Getter(0x0000), Setter(cpu.A),
             Opcode(STA, ZeroPageX, 2, 4));
}

TEST_F(CpuTestLoadStore, STA_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_Set(Getter(0x0120), Setter(cpu.A),
             Opcode(STA, Absolute, 3, 4));
}

TEST_F(CpuTestLoadStore, STA_AbsoluteX) {
    // Same page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_Set(Getter(0x0128), Setter(cpu.A),
             Opcode(STA, AbsoluteX, 3, 4));
}

TEST_F(CpuTestLoadStore, STA_AbsoluteY) {
    // Same page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_Set(Getter(0x0128), Setter(cpu.A),
             Opcode(STA, AbsoluteY, 3, 4));
}

TEST_F(CpuTestLoadStore, STA_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);

    Test_Set(Getter(0x0120), Setter(cpu.A),
             Opcode(STA, IndexedIndirect, 2, 6));
}

TEST_F(CpuTestLoadStore, STA_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.WriteByteAt(0x120, 0x00);

    Test_Set(Getter(0x0120), Setter(cpu.A),
             Opcode(STA, IndexedIndirect, 2, 6));
}

TEST_F(CpuTestLoadStore, STA_IndirectIndexed) {
    // Same page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0x08;

    Test_Set(Getter(0x0128), Setter(cpu.A),
             Opcode(STA, IndirectIndexed, 2, 6));
}

TEST_F(CpuTestLoadStore, STA_IndirectIndexed_CrossingWordsize) {
    // Same page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;

    Test_Set(Getter(0x000F), Setter(cpu.A),
        Opcode(STA, IndirectIndexed, 2, 6));
}
