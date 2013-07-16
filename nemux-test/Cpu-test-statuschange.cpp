/*
 * Cpu-test-statuschange.cpp
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

class CpuTestStatusChange : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestStatusChange() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

    void Test_ClearFlag(Instructions::Name inst, Flag &f) {
        f = 1;
        cpu.Execute(Opcode(inst, Implicit, 1, 2));//, {});

        EXPECT_EQ(BASE_PC + 1, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
        EXPECT_EQ(Flag{0}, f);
    }

    void Test_SetFlag(Instructions::Name inst, Flag &f) {
        f = 0;
        cpu.Execute(Opcode(inst, Implicit, 1, 2));//, {});

        EXPECT_EQ(BASE_PC + 1, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
        EXPECT_EQ(Flag{1}, f);
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

TEST_F(CpuTestStatusChange, CLC) {
    Test_ClearFlag(CLC, cpu.C);
}

TEST_F(CpuTestStatusChange, CLD) {
    Test_ClearFlag(CLD, cpu.D);
}

TEST_F(CpuTestStatusChange, CLI) {
    Test_ClearFlag(CLI, cpu.I);
}

TEST_F(CpuTestStatusChange, CLV) {
    Test_ClearFlag(CLV, cpu.V);
}

TEST_F(CpuTestStatusChange, SEC) {
    Test_SetFlag(SEC, cpu.C);
}

TEST_F(CpuTestStatusChange, SED) {
    Test_SetFlag(SED, cpu.D);
}

TEST_F(CpuTestStatusChange, SEI) {
    Test_SetFlag(SEI, cpu.I);
}
