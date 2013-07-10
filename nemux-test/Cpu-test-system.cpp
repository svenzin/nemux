/*
 * Cpu-test-system.cpp
 *
 *  Created on: 09 Jul 2013
 *      Author: scorder
 */

#include "gtest/gtest.h"

#include "Cpu.h"

//#include <sstream>
#include <vector>
#include <map>
#include <array>
#include <functional>

using namespace std;

class CpuTestSystem : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestSystem() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

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

TEST_F(CpuTestSystem, BRK) {
    auto op = Opcode(InstructionName::BRK, AddressingType::Implicit, 2, 7);

    cpu.VectorIRQ = 0x0380;
    cpu.Memory.SetWordAt(0x0380, 0x0120);
    cpu.StackPage = 0x0100;
    cpu.SP = 0xF0;
    cpu.SetStatus(0x00);

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(0x0120, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0xED, cpu.SP);
    EXPECT_EQ(0x30, cpu.Pull()); // BRK pushes B flag set, Unused is always 1
    EXPECT_EQ(1, cpu.I); // BRK sets the I flag
    EXPECT_EQ(BASE_PC + op.Bytes, cpu.PullWord());
}

TEST_F(CpuTestSystem, BRK_FlagI) {
    auto op = Opcode(InstructionName::BRK, AddressingType::Implicit, 2, 7);

    cpu.VectorIRQ = 0x0380;
    cpu.Memory.SetWordAt(0x0380, 0x0120);
    cpu.StackPage = 0x0100;
    cpu.SP = 0xF0;
    cpu.SetStatus(0x00);
    cpu.I = 1;

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(BASE_PC + 2, cpu.PC);
    EXPECT_EQ(BASE_TICKS + 0, cpu.Ticks);
}

TEST_F(CpuTestSystem, NOP) {
    cpu.Execute(Opcode(InstructionName::NOP, AddressingType::Implicit, 1, 2));

    EXPECT_EQ(BASE_PC + 1, cpu.PC);
    EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
}

TEST_F(CpuTestSystem, RTI) {
    auto op = Opcode(InstructionName::RTI, AddressingType::Implicit, 1, 6);

    auto tester = [&] (Byte status, Flag expN, Flag expV, Flag expB, Flag expD, Flag expI, Flag expZ, Flag expC) {
        cpu.Memory.SetByteAt(BASE_PC, 0xFF);
        cpu.StackPage = 0x100;
        cpu.SP = 0xF0;
        cpu.PushWord(0x0120);
        cpu.Push(status); // All status set

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(0x0120, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(0xF0, cpu.SP);
        EXPECT_EQ(expN, cpu.N);
        EXPECT_EQ(expV, cpu.V);
        EXPECT_EQ(expB, cpu.B);
        EXPECT_EQ(expD, cpu.D);
        EXPECT_EQ(expI, cpu.I);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expC, cpu.C);
    };

    tester(0xFF, 1, 1, 1, 1, 1, 1, 1);
    tester(0x00, 0, 0, 0, 0, 0, 0, 0);
}