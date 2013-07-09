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

class CpuTestLoadStore : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestLoadStore() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

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
        return [=] (Byte value) { cpu.Memory.SetByteAt(a, value); };
    }
    function<Byte ()> Getter(Byte & b) {
        return [&] () { return b; };
    }
    function<Byte ()> Getter(Word a) {
        return [=] () { return cpu.Memory.GetByteAt(a); };
    }
};

TEST_F(CpuTestLoadStore, LDX_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    Test_Load(Getter(cpu.X), Setter(BASE_PC + 1),
              Opcode(InstructionName::LDX, AddressingType::Immediate, 2, 2), 0);
}

TEST_F(CpuTestLoadStore, LDX_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_Load(Getter(cpu.X), Setter(0x0020),
              Opcode(InstructionName::LDX, AddressingType::ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestLoadStore, LDX_ZeroPageY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Y = 0x08;
    Test_Load(Getter(cpu.X), Setter(0x0028),
              Opcode(InstructionName::LDX, AddressingType::ZeroPageY, 2, 4), 0);
}

TEST_F(CpuTestLoadStore, LDX_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    Test_Load(Getter(cpu.X), Setter(0x0120),
              Opcode(InstructionName::LDX, AddressingType::Absolute, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDX_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_Load(Getter(cpu.X), Setter(0x0128),
              Opcode(InstructionName::LDX, AddressingType::AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDX_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_Load(Getter(cpu.X), Setter(0x0210),
              Opcode(InstructionName::LDX, AddressingType::AbsoluteY, 3, 4), 1);
}


TEST_F(CpuTestLoadStore, LDY_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    Test_Load(Getter(cpu.Y), Setter(BASE_PC + 1),
              Opcode(InstructionName::LDY, AddressingType::Immediate, 2, 2), 0);
}

TEST_F(CpuTestLoadStore, LDY_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_Load(Getter(cpu.Y), Setter(0x0020),
              Opcode(InstructionName::LDY, AddressingType::ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestLoadStore, LDY_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_Load(Getter(cpu.Y), Setter(0x0028),
              Opcode(InstructionName::LDY, AddressingType::ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestLoadStore, LDY_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    Test_Load(Getter(cpu.Y), Setter(0x0120),
              Opcode(InstructionName::LDY, AddressingType::Absolute, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDY_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_Load(Getter(cpu.Y), Setter(0x0128),
              Opcode(InstructionName::LDY, AddressingType::AbsoluteX, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDY_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_Load(Getter(cpu.Y), Setter(0x0210),
              Opcode(InstructionName::LDY, AddressingType::AbsoluteX, 3, 4), 1);
}

TEST_F(CpuTestLoadStore, LDA_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    Test_Load(Getter(cpu.A), Setter(BASE_PC + 1),
              Opcode(InstructionName::LDA, AddressingType::Immediate, 2, 2), 0);
}

TEST_F(CpuTestLoadStore, LDA_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_Load(Getter(cpu.A), Setter(0x0020),
              Opcode(InstructionName::LDA, AddressingType::ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestLoadStore, LDA_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_Load(Getter(cpu.A), Setter(0x0028),
              Opcode(InstructionName::LDA, AddressingType::ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestLoadStore, LDA_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    Test_Load(Getter(cpu.A), Setter(0x0120),
              Opcode(InstructionName::LDA, AddressingType::Absolute, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_Load(Getter(cpu.A), Setter(0x0128),
              Opcode(InstructionName::LDA, AddressingType::AbsoluteX, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_Load(Getter(cpu.A), Setter(0x0210),
              Opcode(InstructionName::LDA, AddressingType::AbsoluteX, 3, 4), 1);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_Load(Getter(cpu.A), Setter(0x0128),
              Opcode(InstructionName::LDA, AddressingType::AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_Load(Getter(cpu.A), Setter(0x0210),
              Opcode(InstructionName::LDA, AddressingType::AbsoluteY, 3, 4), 1);
}

TEST_F(CpuTestLoadStore, LDA_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.Memory.SetWordAt(0x28, 0x0120);
    Test_Load(Getter(cpu.A), Setter(0x0120),
              Opcode(InstructionName::LDA, AddressingType::IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTestLoadStore, LDA_IndirectIndexed) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    cpu.Memory.SetWordAt(0x0128, 0x0200);
    Test_Load(Getter(cpu.A), Setter(0x0200),
              Opcode(InstructionName::LDA, AddressingType::IndirectIndexed, 2, 5), 0);
}

TEST_F(CpuTestLoadStore, LDA_IndirectIndexed_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    cpu.Memory.SetWordAt(0x0210, 0x0200);
    Test_Load(Getter(cpu.A), Setter(0x0200),
              Opcode(InstructionName::LDA, AddressingType::IndirectIndexed, 2, 5), 1);
}

TEST_F(CpuTestLoadStore, STX_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Set(Getter(0x0020), Setter(cpu.X),
             Opcode(InstructionName::STX, AddressingType::ZeroPage, 2, 3));
}

TEST_F(CpuTestLoadStore, STX_ZeroPageY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Y = 0x08;

    Test_Set(Getter(0x0028), Setter(cpu.X),
             Opcode(InstructionName::STX, AddressingType::ZeroPageY, 2, 4));
}

TEST_F(CpuTestLoadStore, STX_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_Set(Getter(0x0120), Setter(cpu.X),
             Opcode(InstructionName::STX, AddressingType::Absolute, 3, 4));
}

TEST_F(CpuTestLoadStore, STY_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Set(Getter(0x0020), Setter(cpu.Y),
             Opcode(InstructionName::STY, AddressingType::ZeroPage, 2, 3));
}

TEST_F(CpuTestLoadStore, STY_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_Set(Getter(0x0028), Setter(cpu.Y),
             Opcode(InstructionName::STY, AddressingType::ZeroPageX, 2, 4));
}

TEST_F(CpuTestLoadStore, STY_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_Set(Getter(0x0120), Setter(cpu.Y),
             Opcode(InstructionName::STY, AddressingType::Absolute, 3, 4));
}

TEST_F(CpuTestLoadStore, STA_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Set(Getter(0x0020), Setter(cpu.A),
             Opcode(InstructionName::STA, AddressingType::ZeroPage, 2, 3));
}

TEST_F(CpuTestLoadStore, STA_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_Set(Getter(0x0028), Setter(cpu.A),
             Opcode(InstructionName::STA, AddressingType::ZeroPageX, 2, 4));
}

TEST_F(CpuTestLoadStore, STA_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_Set(Getter(0x0120), Setter(cpu.A),
             Opcode(InstructionName::STA, AddressingType::Absolute, 3, 4));
}

TEST_F(CpuTestLoadStore, STA_AbsoluteX) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_Set(Getter(0x0128), Setter(cpu.A),
             Opcode(InstructionName::STA, AddressingType::AbsoluteX, 3, 4));
}

TEST_F(CpuTestLoadStore, STA_AbsoluteY) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_Set(Getter(0x0128), Setter(cpu.A),
             Opcode(InstructionName::STA, AddressingType::AbsoluteY, 3, 4));
}

TEST_F(CpuTestLoadStore, STA_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.Memory.SetWordAt(0x28, 0x0120);

    Test_Set(Getter(0x0120), Setter(cpu.A),
             Opcode(InstructionName::STA, AddressingType::IndexedIndirect, 2, 6));
}

TEST_F(CpuTestLoadStore, STA_IndirectIndexed) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x120);
    cpu.Y = 0x08;
    cpu.Memory.SetWordAt(0x0128, 0x0200);

    Test_Set(Getter(0x0200), Setter(cpu.A),
             Opcode(InstructionName::STA, AddressingType::IndirectIndexed, 2, 6));
}
