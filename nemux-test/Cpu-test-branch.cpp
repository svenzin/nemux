/*
 * Cpu-test-branch.cpp
 *
 *  Created on: 10 Jul 2013
 *      Author: scorder
 */

#include "gtest/gtest.h"

#include "Cpu.h"

#include <vector>
#include <map>
#include <array>
#include <functional>

using namespace std;

class CpuTestBranch : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestBranch() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

    template<typename Setter>
    void Test_Branch(Setter set, Flag success, Flag failure, Opcode op) {
        auto tester = [&] (Word pc, Byte offset, Flag c, Word expPC, int extra) {
            cpu.Memory.SetByteAt(pc, 0xFF);
            cpu.Memory.SetByteAt(pc + 1, offset);
            set(c);

            cpu.PC = pc;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(expPC, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
        };

        tester(0x0080, 0x20, failure, 0x0082, 0); // Fail
        tester(0x0080, 0x20, success, 0x00A0, 1); // Success, positive offset
        tester(0x0080, 0xE0, success, 0x0060, 1); // Success, negative offset
        tester(0x00F0, 0x20, success, 0x0110, 3); // Success, positive offset, crossing page
        tester(0x0110, 0xE0, success, 0x00F0, 3); // Success, negative offset, crossing page
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

TEST_F(CpuTestBranch, BCC) {
    Test_Branch(Setter(cpu.C), 0, 1,
        Opcode(InstructionName::BCC, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTestBranch, BCS) {
    Test_Branch(Setter(cpu.C), 1, 0,
        Opcode(InstructionName::BCS, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTestBranch, BEQ) {
    Test_Branch(Setter(cpu.Z), 1, 0,
        Opcode(InstructionName::BEQ, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTestBranch, BMI) {
    Test_Branch(Setter(cpu.N), 1, 0,
        Opcode(InstructionName::BMI, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTestBranch, BNE) {
    Test_Branch(Setter(cpu.Z), 0, 1,
        Opcode(InstructionName::BNE, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTestBranch, BPL) {
    Test_Branch(Setter(cpu.N), 0, 1,
        Opcode(InstructionName::BPL, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTestBranch, BVC) {
    Test_Branch(Setter(cpu.V), 0, 1,
        Opcode(InstructionName::BVC, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTestBranch, BVS) {
    Test_Branch(Setter(cpu.V), 1, 0,
        Opcode(InstructionName::BVS, AddressingType::Relative, 2, 2));
}
