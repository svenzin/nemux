/*
 * Cpu-test-shift.cpp
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

class CpuTestShift : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestShift() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

    template<typename Getter, typename Setter>
    void Test_ASL(Getter get, Setter set, Opcode op) {
        auto Tester = [&] (Byte m, Byte expM, Flag expC, Flag expZ, Flag expN) {
            set(m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, get());
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        Tester(0x24, 0x48, 0, 0, 0); // Shift

        Tester(0x00, 0x00, 0, 1, 0); // Zero flag
        Tester(0x80, 0x00, 1, 1, 0); // Zero flag

        Tester(0x01, 0x02, 0, 0, 0); // Carry flag
        Tester(0x81, 0x02, 1, 0, 0); // Carry flag

        Tester(0xA0, 0x40, 1, 0, 0); // Negative flag
        Tester(0xF0, 0xE0, 1, 0, 1); // Negative flag
    }

    template<typename Getter, typename Setter>
    void Test_LSR(Getter get, Setter set, Opcode op) {
        auto tester = [&] (Byte m, Byte expM, Flag expC, Flag expZ) {
            set(m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, get());
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
        };

        tester(0x24, 0x12, 0, 0); // Shift
        tester(0x00, 0x00, 0, 1); // Zero flag
        tester(0x01, 0x00, 1, 1); // Carry flag
    }

    template<typename Getter, typename Setter>
    void Test_ROL(Getter get, Setter set, Opcode op) {
        auto tester = [&] (Byte m, Byte c, Byte expM, Flag expC, Flag expZ, Flag expN) {
            set(m);
            cpu.C = c;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, get());
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x21, 0, 0x42, 0, 0, 0); // Shift
        tester(0x21, 1, 0x43, 0, 0, 0); // Shift w/ Carry
        tester(0x00, 0, 0x00, 0, 1, 0); // Zero
        tester(0x80, 0, 0x00, 1, 1, 0); // Carry
        tester(0x88, 1, 0x11, 1, 0, 0); // Carry
        tester(0x40, 0, 0x80, 0, 0, 1); // Negative
    }

    template<typename Getter, typename Setter>
    void Test_ROR(Getter get, Setter set, Opcode op) {
        auto tester = [&] (Byte m, Byte c, Byte expM, Flag expC, Flag expZ, Flag expN) {
            set(m);
            cpu.C = c;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, get());
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x42, 0, 0x21, 0, 0, 0); // Shift
        tester(0x02, 1, 0x81, 0, 0, 1); // Shift w/ Carry
        tester(0x00, 0, 0x00, 0, 1, 0); // Zero
        tester(0x01, 0, 0x00, 1, 1, 0); // Carry
        tester(0x11, 1, 0x88, 1, 0, 1); // Carry
        tester(0x00, 1, 0x80, 0, 0, 1); // Negative
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

TEST_F(CpuTestShift, ASL_Accumulator) {
    Test_ASL(
        [&]              { return cpu.A; },
        [&] (Byte value) { cpu.A = value; },
        Opcode(ASL, Accumulator, 1, 2));
}

TEST_F(CpuTestShift, ASL_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_ASL(
        [&]              { return cpu.Memory.GetByteAt(0x20); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x20, value); },
        Opcode(ASL, ZeroPage, 2, 5));
}

TEST_F(CpuTestShift, ASL_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_ASL(
        [&]              { return cpu.Memory.GetByteAt(0x28); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x28, value); },
        Opcode(ASL, ZeroPageX, 2, 6));
}

TEST_F(CpuTestShift, ASL_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_ASL(
        [&]              { return cpu.Memory.GetByteAt(0x0120); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0120, value); },
        Opcode(ASL, Absolute, 3, 6));
}

TEST_F(CpuTestShift, ASL_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_ASL(
        [&]              { return cpu.Memory.GetByteAt(0x0128); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0128, value); },
        Opcode(ASL, AbsoluteX, 3, 7));
}

TEST_F(CpuTestShift, LSR_Accumulator) {
    Test_LSR(Getter(cpu.A), Setter(cpu.A),
             Opcode(LSR, Accumulator, 1, 2));
}

TEST_F(CpuTestShift, LSR_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_LSR(Getter(0x0020), Setter(0x0020),
             Opcode(LSR, ZeroPage, 2, 5));
}

TEST_F(CpuTestShift, LSR_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_LSR(Getter(0x0028), Setter(0x0028),
             Opcode(LSR, ZeroPageX, 2, 6));
}

TEST_F(CpuTestShift, LSR_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_LSR(Getter(0x0120), Setter(0x0120),
             Opcode(LSR, Absolute, 3, 6));
}

TEST_F(CpuTestShift, LSR_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_LSR(Getter(0x0128), Setter(0x0128),
             Opcode(LSR, AbsoluteX, 3, 7));
}

TEST_F(CpuTestShift, ROL_Accumulator) {
    Test_ROL(Getter(cpu.A), Setter(cpu.A),
             Opcode(ROL, Accumulator, 1, 2));
}

TEST_F(CpuTestShift, ROL_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_ROL(Getter(0x0020), Setter(0x0020),
             Opcode(ROL, ZeroPage, 2, 5));
}

TEST_F(CpuTestShift, ROL_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_ROL(Getter(0x0028), Setter(0x0028),
             Opcode(ROL, ZeroPageX, 2, 6));
}

TEST_F(CpuTestShift, ROL_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_ROL(Getter(0x0120), Setter(0x0120),
             Opcode(ROL, Absolute, 3, 6));
}

TEST_F(CpuTestShift, ROL_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_ROL(Getter(0x0128), Setter(0x0128),
             Opcode(ROL, AbsoluteX, 3, 7));
}

TEST_F(CpuTestShift, ROR_Accumulator) {
    Test_ROR(Getter(cpu.A), Setter(cpu.A),
             Opcode(ROR, Accumulator, 1, 2));
}

TEST_F(CpuTestShift, ROR_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_ROR(Getter(0x0020), Setter(0x0020),
             Opcode(ROR, ZeroPage, 2, 5));
}

TEST_F(CpuTestShift, ROR_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_ROR(Getter(0x0028), Setter(0x0028),
             Opcode(ROR, ZeroPageX, 2, 6));
}

TEST_F(CpuTestShift, ROR_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_ROR(Getter(0x0120), Setter(0x0120),
             Opcode(ROR, Absolute, 3, 6));
}

TEST_F(CpuTestShift, ROR_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_ROR(Getter(0x0128), Setter(0x0128),
             Opcode(ROR, AbsoluteX, 3, 7));
}
