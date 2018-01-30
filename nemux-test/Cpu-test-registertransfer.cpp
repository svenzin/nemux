/*
 * Cpu-test-registertransfer.cpp
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

class CpuTestRegisterTransfer : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestRegisterTransfer() : cpu("6502", &memory) {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
    }

    MemoryBlock<0x400> memory;
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
        return [=] (Byte value) { cpu.WriteByteAt(a, value); };
    }
    function<Byte ()> Getter(Byte & b) {
        return [&] () { return b; };
    }
    function<Byte ()> Getter(Word a) {
        return [=] () { return cpu.ReadByteAt(a); };
    }
};

TEST_F(CpuTestRegisterTransfer, TAX) {
    Test_Transfer(Getter(cpu.X), Setter(cpu.A),
                  Opcode(TAX, Implicit, 1, 2));
}

TEST_F(CpuTestRegisterTransfer, TAY) {
    Test_Transfer(Getter(cpu.Y), Setter(cpu.A),
                  Opcode(TAY, Implicit, 1, 2));
}

TEST_F(CpuTestRegisterTransfer, TXA) {
    Test_Transfer(Getter(cpu.A), Setter(cpu.X),
                  Opcode(TXA, Implicit, 1, 2));
}

TEST_F(CpuTestRegisterTransfer, TYA) {
    Test_Transfer(Getter(cpu.A), Setter(cpu.Y),
                  Opcode(TYA, Implicit, 1, 2));
}
