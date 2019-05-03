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
using namespace Instructions;
using namespace Addressing;

class CpuTestBranch : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestBranch() : cpu("6502", &memory) {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
    }

    MemoryBlock<0x400> memory;
    Cpu cpu;

    template<typename Setter>
    void Test_Branch(Setter set, Flag success, Flag failure, Opcode op) {
        auto tester = [&] (Word pc, Byte offset, Flag c, Word expPC, int extra) {
            cpu.WriteByteAt(pc, 0xFF);
            cpu.WriteByteAt(pc + 1, offset);
            set(c);

            cpu.PC = pc;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(expPC, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
        };

        tester(0x0080, 0x20, failure, 0x0080 + 2, 0); // Fail
        tester(0x0080, 0x20, success, 0x00A0 + 2, 1); // Success, positive offset
        tester(0x0080, 0xE0, success, 0x0060 + 2, 1); // Success, negative offset
        tester(0x00F0, 0x20, success, 0x0110 + 2, 2); // Success, positive offset, crossing page
        tester(0x00F0, 0x0F, success, 0x00FF + 2, 2); // Success, positive offset, crossing page on PC+2
        tester(0x0110, 0xE0, success, 0x00F0 + 2, 2); // Success, negative offset, crossing page
        tester(0x0110, 0xEF, success, 0x00FF + 2, 1); // Success, negative offset, not crossing page on PC+2
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

TEST_F(CpuTestBranch, BCC) {
    Test_Branch(Setter(cpu.C), 0, 1,
        Opcode(BCC, Relative, 2, 2));
}

TEST_F(CpuTestBranch, BCS) {
    Test_Branch(Setter(cpu.C), 1, 0,
        Opcode(BCS, Relative, 2, 2));
}

TEST_F(CpuTestBranch, BEQ) {
    Test_Branch(Setter(cpu.Z), 1, 0,
        Opcode(BEQ, Relative, 2, 2));
}

TEST_F(CpuTestBranch, BMI) {
    Test_Branch(Setter(cpu.N), 1, 0,
        Opcode(BMI, Relative, 2, 2));
}

TEST_F(CpuTestBranch, BNE) {
    Test_Branch(Setter(cpu.Z), 0, 1,
        Opcode(BNE, Relative, 2, 2));
}

TEST_F(CpuTestBranch, BPL) {
    Test_Branch(Setter(cpu.N), 0, 1,
        Opcode(BPL, Relative, 2, 2));
}

TEST_F(CpuTestBranch, BVC) {
    Test_Branch(Setter(cpu.V), 0, 1,
        Opcode(BVC, Relative, 2, 2));
}

TEST_F(CpuTestBranch, BVS) {
    Test_Branch(Setter(cpu.V), 1, 0,
        Opcode(BVS, Relative, 2, 2));
}
