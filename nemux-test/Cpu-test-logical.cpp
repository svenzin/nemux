/*
 * Cpu-test-logical.cpp
 *
 *  Created on: 09 Jul 2013
 *      Author: scorder
 */

#include "gtest/gtest.h"

#include "Cpu.h"

#include <vector>
#include <map>
#include <array>
#include <functional>

using namespace std;

class CpuTestLogical : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestLogical() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

    template<typename Setter>
    void Test_AND(Setter set, Opcode op, bool extra) {
        const auto expectedCycles = extra ? op.Cycles + 1 : op.Cycles;
        auto tester = [&] (Byte a, Byte m, Byte expA, Flag expZ, Flag expN) {
            set(m);
            cpu.A = a;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + expectedCycles, cpu.Ticks);
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x66, 0x00, 0x00, 1, 0); // Zero
        tester(0xFF, 0x80, 0x80, 0, 1); // Negative
        tester(0xAA, 0x24, 0x20, 0, 0); // Normal
    }

    template<typename Setter>
    void Test_EOR(Setter set, Opcode op, int extra) {
        auto tester = [&] (Byte a, Byte m, Byte expA, Flag expZ, Flag expN) {
            set(m);
            cpu.A = a;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x0C, 0x0A, 0x06, 0, 0); // 1100b XOR 1010b = 0110b
        tester(0x0C, 0x0C, 0x00, 1, 0); // 1100b XOR 1100b = 0000b
        tester(0x8C, 0x0C, 0x80, 0, 1); // 10001100b XOR 00001100b = 10000000b
    }

    template<typename Setter>
    void Test_ORA(Setter set, Opcode op, int extra) {
        auto tester = [&] (Byte a, Byte m, Byte expA, Flag expZ, Flag expN) {
            set(m);
            cpu.A = a;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x0C, 0x0A, 0x0E, 0, 0); // 1100b OR 1010b = 1110b
        tester(0x00, 0x00, 0x00, 1, 0); // 0000b OR 0000b = 0000b
        tester(0x8C, 0x03, 0x8F, 0, 1); // 10001100b OR 00000011b = 10001111b
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

TEST_F(CpuTestLogical, EOR_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    Test_EOR(Setter(BASE_PC + 1), Opcode(InstructionName::EOR, AddressingType::Immediate, 2, 2), 0);
}

TEST_F(CpuTestLogical, EOR_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_EOR(Setter(0x0020), Opcode(InstructionName::EOR, AddressingType::ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestLogical, EOR_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_EOR(Setter(0x0028), Opcode(InstructionName::EOR, AddressingType::ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestLogical, EOR_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    Test_EOR(Setter(0x0120), Opcode(InstructionName::EOR, AddressingType::Absolute, 3, 4), 0);
}

TEST_F(CpuTestLogical, EOR_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_EOR(Setter(0x0128), Opcode(InstructionName::EOR, AddressingType::AbsoluteX, 3, 4), 0);
}

TEST_F(CpuTestLogical, EOR_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_EOR(Setter(0x0210), Opcode(InstructionName::EOR, AddressingType::AbsoluteX, 3, 4), 1);
}

TEST_F(CpuTestLogical, EOR_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_EOR(Setter(0x0128), Opcode(InstructionName::EOR, AddressingType::AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTestLogical, EOR_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_EOR(Setter(0x0210), Opcode(InstructionName::EOR, AddressingType::AbsoluteY, 3, 4), 1);
}

TEST_F(CpuTestLogical, EOR_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.Memory.SetWordAt(0x28, 0x0120);
    Test_EOR(Setter(0x0120), Opcode(InstructionName::EOR, AddressingType::IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTestLogical, EOR_IndirectIndexed) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    cpu.Memory.SetWordAt(0x0128, 0x0200);
    Test_EOR(Setter(0x0200), Opcode(InstructionName::EOR, AddressingType::IndirectIndexed, 2, 5), 0);
}

TEST_F(CpuTestLogical, EOR_IndirectIndexed_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    cpu.Memory.SetWordAt(0x0210, 0x0200);
    Test_EOR(Setter(0x0200), Opcode(InstructionName::EOR, AddressingType::IndirectIndexed, 2, 5), 1);
}

TEST_F(CpuTestLogical, ORA_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    Test_ORA(Setter(BASE_PC + 1), Opcode(InstructionName::ORA, AddressingType::Immediate, 2, 2), 0);
}

TEST_F(CpuTestLogical, ORA_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_ORA(Setter(0x0020), Opcode(InstructionName::ORA, AddressingType::ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestLogical, ORA_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_ORA(Setter(0x0028), Opcode(InstructionName::ORA, AddressingType::ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestLogical, ORA_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    Test_ORA(Setter(0x0120), Opcode(InstructionName::ORA, AddressingType::Absolute, 3, 4), 0);
}

TEST_F(CpuTestLogical, ORA_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_ORA(Setter(0x0128), Opcode(InstructionName::ORA, AddressingType::AbsoluteX, 3, 4), 0);
}

TEST_F(CpuTestLogical, ORA_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_ORA(Setter(0x0210), Opcode(InstructionName::ORA, AddressingType::AbsoluteX, 3, 4), 1);
}

TEST_F(CpuTestLogical, ORA_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_ORA(Setter(0x0128), Opcode(InstructionName::ORA, AddressingType::AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTestLogical, ORA_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_ORA(Setter(0x0210), Opcode(InstructionName::ORA, AddressingType::AbsoluteY, 3, 4), 1);
}

TEST_F(CpuTestLogical, ORA_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.Memory.SetWordAt(0x28, 0x0120);
    Test_ORA(Setter(0x0120), Opcode(InstructionName::ORA, AddressingType::IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTestLogical, ORA_IndirectIndexed) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    cpu.Memory.SetWordAt(0x0128, 0x0200);
    Test_ORA(Setter(0x0200), Opcode(InstructionName::ORA, AddressingType::IndirectIndexed, 2, 5), 0);
}

TEST_F(CpuTestLogical, ORA_IndirectIndexed_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    cpu.Memory.SetWordAt(0x0210, 0x0200);
    Test_ORA(Setter(0x0200), Opcode(InstructionName::ORA, AddressingType::IndirectIndexed, 2, 5), 1);
}


TEST_F(CpuTestLogical, AND_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_AND(Setter(BASE_PC + 1),
             Opcode(InstructionName::AND, AddressingType::Immediate, 2, 2),
             false);
}

TEST_F(CpuTestLogical, AND_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_AND(Setter(0x0020),
             Opcode(InstructionName::AND, AddressingType::ZeroPage, 2, 3),
             false);
}

TEST_F(CpuTestLogical, AND_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_AND(Setter(0x0028),
             Opcode(InstructionName::AND, AddressingType::ZeroPageX, 2, 4),
             false);
}

TEST_F(CpuTestLogical, AND_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_AND(Setter(0x0120),
             Opcode(InstructionName::AND, AddressingType::Absolute, 3, 4),
             false);
}

TEST_F(CpuTestLogical, AND_AbsoluteX) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_AND(Setter(0x0128),
             Opcode(InstructionName::AND, AddressingType::AbsoluteX, 3, 4),
             false);
}

TEST_F(CpuTestLogical, AND_AbsoluteX_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_AND(Setter(0x0210),
             Opcode(InstructionName::AND, AddressingType::AbsoluteX, 3, 4),
             true);
}

TEST_F(CpuTestLogical, AND_AbsoluteY) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_AND(Setter(0x0128),
             Opcode(InstructionName::AND, AddressingType::AbsoluteY, 3, 4),
             false);
}

TEST_F(CpuTestLogical, AND_AbsoluteY_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_AND(Setter(0x0210),
             Opcode(InstructionName::AND, AddressingType::AbsoluteY, 3, 4),
             true);
}

TEST_F(CpuTestLogical, AND_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.Memory.SetWordAt(0x28, 0x0120);

    Test_AND(Setter(0x0120),
             Opcode(InstructionName::AND, AddressingType::IndexedIndirect, 2, 6),
             false);
}

TEST_F(CpuTestLogical, AND_IndirectIndexed) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x120);
    cpu.Y = 0x08;
    cpu.Memory.SetWordAt(0x0128, 0x0200);

    Test_AND(Setter(0x0200),
             Opcode(InstructionName::AND, AddressingType::IndirectIndexed, 2, 5),
             false);
}

TEST_F(CpuTestLogical, AND_IndirectIndexed_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x120);
    cpu.Y = 0xF0;
    cpu.Memory.SetWordAt(0x0210, 0x0200);

    Test_AND(Setter(0x0200),
             Opcode(InstructionName::AND, AddressingType::IndirectIndexed, 2, 5),
             true);
}

TEST_F(CpuTestLogical, BIT_ZeroPage) {
    for (auto i = 0; i < 0x100; ++i) {
        cpu.Memory.SetByteAt(i, i);
        cpu.Memory.SetByteAt(0x100 + i, i);
    }

    for (auto a = 0; a < 0x100; ++a) {
        for (auto m = 0; m < 0x100; ++m) {
            // ZeroPage
            cpu.PC = 0x200;
            cpu.Memory.SetByteAt(0x200, 0xFF);
            cpu.Memory.SetByteAt(0x201, m);

            cpu.A = a;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(Opcode(InstructionName::BIT, AddressingType::ZeroPage, 2, 3));

            EXPECT_EQ(0x200 + 2, cpu.PC);
            EXPECT_EQ(BASE_TICKS + 3, cpu.Ticks);
            EXPECT_EQ((m & a   ) == 0 ? 1 : 0, cpu.Z);
            EXPECT_EQ((m & 0x40) == 0 ? 0 : 1, cpu.V);
            EXPECT_EQ((m & 0x80) == 0 ? 0 : 1, cpu.N);
        }
    }
}

TEST_F(CpuTestLogical, BIT_Absolute) {
    for (auto i = 0; i < 0x100; ++i) {
        cpu.Memory.SetByteAt(i, i);
        cpu.Memory.SetByteAt(0x100 + i, i);
    }

    for (auto a = 0; a < 0x100; ++a) {
        for (auto m = 0; m < 0x100; ++m) {
            cpu.PC = 0x200;
            cpu.Memory.SetByteAt(0x200, 0xFF);
            cpu.Memory.SetWordAt(0x201, 0x100 + m);

            cpu.A = a;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(Opcode(InstructionName::BIT, AddressingType::Absolute, 3, 4));

            EXPECT_EQ(0x200 + 3, cpu.PC);
            EXPECT_EQ(BASE_TICKS + 4, cpu.Ticks);
            EXPECT_EQ((m & a ) == 0 ? 1 : 0, cpu.Z);
            EXPECT_EQ((m & 0x40) == 0 ? 0 : 1, cpu.V);
            EXPECT_EQ((m & 0x80) == 0 ? 0 : 1, cpu.N);
        }
    }
}