/*
 * Cpu-test-incrementdecrement.cpp
 *
 *  Created on: 19 Jun 2013
 *      Author: scorder
 */

#include "gtest/gtest.h"

#include "Cpu.h"

#include <vector>
#include <map>
#include <array>
#include <functional>

using namespace std;

class CpuTestIncrementDecrement : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestIncrementDecrement() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

    template<typename Getter, typename Setter>
    void Test_DEC(Getter get, Setter set, Opcode op) {
        auto tester = [&] (Byte m, Byte expM, Flag expZ, Flag expN) {
            set(m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, get());
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x08, 0x07, 0, 0);
        tester(0x01, 0x00, 1, 0);
        tester(0x00, 0xFF, 0, 1);
        tester(0x80, 0x7F, 0, 0);
    }

    template<typename Getter, typename Setter>
    void Test_INC(Getter get, Setter set, Opcode op) {
        auto tester = [&] (Byte m, Byte expM, Flag expZ, Flag expN) {
            set(m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, get());
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x10, 0x11, 0, 0);
        tester(0xFF, 0x00, 1, 0);
        tester(0x7F, 0x80, 0, 1);
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
//}

TEST_F(CpuTestIncrementDecrement, DEC_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_DEC(Getter(0x0020), Setter(0x0020),
             Opcode(InstructionName::DEC, AddressingType::ZeroPage, 2, 5));
}

TEST_F(CpuTestIncrementDecrement, DEC_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_DEC(Getter(0x0028), Setter(0x0028),
             Opcode(InstructionName::DEC, AddressingType::ZeroPageX, 2, 6));
}

TEST_F(CpuTestIncrementDecrement, DEC_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_DEC(Getter(0x0120), Setter(0x0120),
             Opcode(InstructionName::DEC, AddressingType::Absolute, 2, 6));
}

TEST_F(CpuTestIncrementDecrement, DEC_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_DEC(Getter(0x0128), Setter(0x0128),
             Opcode(InstructionName::DEC, AddressingType::AbsoluteX, 2, 7));
}

TEST_F(CpuTestIncrementDecrement, DEX) {
    Test_DEC(Getter(cpu.X), Setter(cpu.X),
             Opcode(InstructionName::DEX, AddressingType::Implicit, 1, 2));
}

TEST_F(CpuTestIncrementDecrement, DEY) {
    Test_DEC(Getter(cpu.Y), Setter(cpu.Y),
             Opcode(InstructionName::DEY, AddressingType::Implicit, 1, 2));
}


TEST_F(CpuTestIncrementDecrement, INC_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_INC(Getter(0x0020), Setter(0x0020),
             Opcode(InstructionName::INC, AddressingType::ZeroPage, 2, 5));
}

TEST_F(CpuTestIncrementDecrement, INC_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_INC(Getter(0x0028), Setter(0x0028),
             Opcode(InstructionName::INC, AddressingType::ZeroPageX, 2, 6));
}

TEST_F(CpuTestIncrementDecrement, INC_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_INC(Getter(0x0120), Setter(0x0120),
             Opcode(InstructionName::INC, AddressingType::Absolute, 2, 6));
}

TEST_F(CpuTestIncrementDecrement, INC_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_INC(Getter(0x0128), Setter(0x0128),
             Opcode(InstructionName::INC, AddressingType::AbsoluteX, 2, 7));
}

TEST_F(CpuTestIncrementDecrement, INX) {
    Test_INC(Getter(cpu.X), Setter(cpu.X),
             Opcode(InstructionName::INX, AddressingType::Implicit, 1, 2));
}

TEST_F(CpuTestIncrementDecrement, INY) {
    Test_INC(Getter(cpu.Y), Setter(cpu.Y),
             Opcode(InstructionName::INY, AddressingType::Implicit, 1, 2));
}
