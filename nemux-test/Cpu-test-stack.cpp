/*
 * Cpu-test-stack.cpp
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
using namespace Instructions;
using namespace Addressing;

class CpuTestStack : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestStack() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

    template<typename Getter, typename Setter>
    void Test_Transfer(Getter get, Setter set, Opcode op) {
        auto tester = [&] (Byte value, Flag expZ, Flag expN) {
            set(value);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(value, get());
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x20, 0, 0);
        tester(0xA0, 0, 1);
        tester(0x00, 1, 0);
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

TEST_F(CpuTestStack, TSX) {
    Test_Transfer(
        [&]              { return cpu.X; },
        [&] (Byte value) { cpu.SP = value; },
        Opcode(TSX, Implicit, 1, 2)
    );
}

TEST_F(CpuTestStack, TXS) {
    // TXS does not change the flags
    auto op = Opcode(TXS, Implicit, 1, 2);
    cpu.X = 0x20;

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0x20, cpu.SP);
}

TEST_F(CpuTestStack, PHP_FlagB) {
    // PHP is software instruction pushing the Status -> B is set
    const auto op = Opcode(PHP, Implicit, 1, 3);
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    cpu.B = 0;
    cpu.StackPage = 0x0100;
    cpu.SP = 0xF0;

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0x10, cpu.Memory.GetByteAt(0x01F0) & 0x10);
    EXPECT_EQ(0xEF, cpu.SP);
}

//TEST_F(CpuTestStack, PLP_FlagB) {
//    const auto op = Opcode(PLP, Implicit, 1, 4);
//    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
//
//    cpu.B = 1;
//    cpu.StackPage = 0x0100;
//    cpu.SP = 0xEF;
//    cpu.Memory.SetByteAt(0x01F0, 0xFF);
//
//    cpu.PC = BASE_PC;
//    cpu.Ticks = BASE_TICKS;
//    cpu.Execute(op);
//
//    EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
//    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
//    EXPECT_EQ(0xF0, cpu.SP);
//    EXPECT_EQ(0, cpu.B);
//}

TEST_F(CpuTestStack, PHA) {
    const auto op = Opcode(PHA, Implicit, 1, 3);

    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    cpu.A = 0x20;
    cpu.StackPage = 0x0100;
    cpu.SP = 0xF0;

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0x20, cpu.Memory.GetByteAt(0x01F0));
    EXPECT_EQ(0xEF, cpu.SP);
}

TEST_F(CpuTestStack, PLA) {
    const auto op = Opcode(PLA, Implicit, 1, 4);
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    auto tester = [&] (Byte m, Flag expZ, Flag expN) {
        cpu.StackPage = 0x0100;
        cpu.SP = 0xEF;
        cpu.Memory.SetByteAt(0x01F0, m);

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(m, cpu.A);
        EXPECT_EQ(0xF0, cpu.SP);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expN, cpu.N);
    };

    tester(0x20, 0, 0);
    tester(0x00, 1, 0);
    tester(0x80, 0, 1);
}

TEST_F(CpuTestStack, PLP) {
    const auto op = Opcode(PLP, Implicit, 1, 4);
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    auto tester = [&] (Byte m, Flag expN, Flag expV, Flag expD, Flag expI, Flag expZ, Flag expC) {
        cpu.StackPage = 0x0100;
        cpu.SP = 0xEF;
        cpu.Memory.SetByteAt(0x01F0, m);

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(0xF0, cpu.SP);
        EXPECT_EQ(expN, cpu.N);
        EXPECT_EQ(expV, cpu.V);
        EXPECT_EQ(expD, cpu.D);
        EXPECT_EQ(expI, cpu.I);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expC, cpu.C);
    };

    tester(0xB5, 1, 0, 0, 1, 0, 1); // Status 10xx0101b
    tester(0x6A, 0, 1, 1, 0, 1, 0); // Status 01xx1010b
}

TEST_F(CpuTestStack, PHP) {
    const auto op = Opcode(PHP, Implicit, 1, 3);
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    auto tester = [&] (Flag n, Flag v, Flag d, Flag i, Flag z, Flag c) {
        cpu.N = n;
        cpu.V = v;
        cpu.D = d;
        cpu.I = i;
        cpu.Z = z;
        cpu.C = c;
        cpu.StackPage = 0x0100;
        cpu.SP = 0xF0;

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        const auto expected = n * 0x80 + v * 0x40 + d * 0x08 +
                              i * 0x04 + z * 0x02 + c * 0x01;
        const auto flagsMask = 0xCF;
        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expected & flagsMask, cpu.Memory.GetByteAt(0x01F0) & flagsMask);
        EXPECT_EQ(0xEF, cpu.SP);
    };

    tester(0, 1, 1, 0, 1, 0);
    tester(1, 0, 0, 1, 0, 1);
}
