/*
 * Cpu-test-arithmetic.cpp
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

class CpuTestArithmetic : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestArithmetic() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

    template<typename Setter>
    void Test_ADC(Setter set, Opcode op, bool extra) {
        const auto expectedCycles = extra ? op.Cycles + 1 : op.Cycles;
        auto tester = [&] (Byte a, Byte m, Flag c, Byte expA, Flag expC, Flag expZ, Flag expV, Flag expN) {
            set(m);
            cpu.A = a;
            cpu.C = c;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + expectedCycles, cpu.Ticks);
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expV, cpu.V);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x10, 0x20, 0, 0x30, 0, 0, 0, 0); // pos pos > pos
        tester(0x10, 0x20, 1, 0x31, 0, 0, 0, 0); // pos pos C > pos
        tester(0x00, 0x00, 0, 0x00, 0, 1, 0, 0); // Zero
        tester(0x7F, 0x80, 1, 0x00, 1, 1, 0, 0); // Zero by carry
        tester(0x80, 0x80, 0, 0x00, 1, 1, 1, 0); // Zero by overflow
        tester(0x20, 0xF0, 0, 0x10, 1, 0, 0, 0); // Carry w/o overflow w/o sign change
        tester(0xF0, 0x20, 0, 0x10, 1, 0, 0, 0); // Carry w/o overflow w/  sign change
        tester(0xA0, 0xA0, 0, 0x40, 1, 0, 1, 0); // Carry with overflow
        tester(0x70, 0x10, 0, 0x80, 0, 0, 1, 1); // Overflow pos > neg
        tester(0xB0, 0xB0, 0, 0x60, 1, 0, 1, 0); // Overflow neg > pos
        tester(0x00, 0xF0, 0, 0xF0, 0, 0, 0, 1); // Negative
    }

    template<typename Setter>
    void Test_SBC(Setter set, Opcode op, int extra) {
        auto tester = [&] (Byte a, Byte m, Flag c, Byte expA, Flag expC, Flag expZ, Flag expV, Flag expN) {
            set(m);
            cpu.A = a;
            cpu.C = c;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expV, cpu.V);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x40, 0x20, 1, 0x20, 0, 0, 0, 0); // pos pos > pos
        tester(0x40, 0x1F, 0, 0x20, 0, 0, 0, 0); // pos pos C > pos
        tester(0x00, 0x00, 1, 0x00, 0, 1, 0, 0); // Zero
        tester(0x80, 0x7F, 0, 0x00, 0, 1, 1, 0); // Zero by overflow
        tester(0x00, 0xFF, 0, 0x00, 1, 1, 0, 0); // Zero by carry
        tester(0x20, 0x40, 1, 0xE0, 1, 0, 0, 1); // Carry w/o overflow
        tester(0x20, 0xA0, 1, 0x80, 1, 0, 1, 1); // Carry w/ overflow
        tester(0x00, 0x00, 0, 0xFF, 1, 0, 0, 1); // Carry by borrow
        tester(0x20, 0xA0, 1, 0x80, 1, 0, 1, 1); // Overflow pos > neg
        tester(0x80, 0x20, 1, 0x60, 0, 0, 1, 0); // Overflow neg > pos
        tester(0x80, 0x00, 1, 0x80, 0, 0, 0, 1); // Negative
    }

    template<typename SetterReg, typename SetterMem>
    void Test_Compare(SetterReg setR, SetterMem setM, Opcode op, int extra) {
        auto tester = [&] (Byte r, Byte m, Flag expC, Flag expZ, Flag expN) {
            setR(r);
            setM(m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x20, 0x10, 1, 0, 0); // Greater
        tester(0x20, 0x20, 1, 1, 0); // Equal
        tester(0x20, 0x40, 0, 0, 1); // Lower

        tester(0xF0, 0xE0, 1, 0, 0); // Greater
        tester(0xF0, 0xF0, 1, 1, 0); // Equal
        tester(0xF0, 0xF8, 0, 0, 1); // Lower
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

TEST_F(CpuTestArithmetic, CPX_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_Compare(Setter(cpu.X), Setter(BASE_PC + 1),
        Opcode(CPX, Immediate, 2, 2), 0);
}

TEST_F(CpuTestArithmetic, CPX_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Compare(Setter(cpu.X), Setter(0x0020),
        Opcode(CPX, ZeroPage, 2, 3), 0
    );
}

TEST_F(CpuTestArithmetic, CPX_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_Compare(Setter(cpu.X), Setter(0x0120),
        Opcode(CPX, Absolute, 3, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CPY_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_Compare(Setter(cpu.Y), Setter(BASE_PC + 1),
        Opcode(CPY, Immediate, 2, 2), 0
    );
}

TEST_F(CpuTestArithmetic, CPY_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Compare(Setter(cpu.Y), Setter(0x0020),
        Opcode(CPY, ZeroPage, 2, 3), 0
    );
}

TEST_F(CpuTestArithmetic, CPY_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_Compare(Setter(cpu.Y), Setter(0x0120),
        Opcode(CPY, Absolute, 3, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_Compare(Setter(cpu.A), Setter(BASE_PC + 1),
        Opcode(CMP, Immediate, 2, 2), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x20);

    Test_Compare(Setter(cpu.A), Setter(0x0020),
        Opcode(CMP, ZeroPage, 2, 3), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_Compare(Setter(cpu.A), Setter(0x0028),
        Opcode(CMP, ZeroPageX, 2, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_Compare(Setter(cpu.A), Setter(0x0120),
        Opcode(CMP, Absolute, 3, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_Compare(Setter(cpu.A), Setter(0x0128),
        Opcode(CMP, AbsoluteX, 3, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_Compare(Setter(cpu.A), Setter(0x0210),
        Opcode(CMP, AbsoluteX, 3, 4), 1
    );
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_Compare(Setter(cpu.A), Setter(0x0128),
        Opcode(CMP, AbsoluteY, 3, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_Compare(Setter(cpu.A), Setter(0x0210),
        Opcode(CMP, AbsoluteY, 3, 4), 1
    );
}

TEST_F(CpuTestArithmetic, CMP_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);

    Test_Compare(Setter(cpu.A), Setter(0x0120),
        Opcode(CMP, IndexedIndirect, 2, 6), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0x08;
    cpu.WriteWordAt(0x0128, 0x0200);

    Test_Compare(Setter(cpu.A), Setter(0x0200),
        Opcode(CMP, IndirectIndexed, 2, 5), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0xF0;
    cpu.WriteWordAt(0x0210, 0x0200);

    Test_Compare(Setter(cpu.A), Setter(0x0200),
        Opcode(CMP, IndirectIndexed, 2, 5), 1
    );
}

TEST_F(CpuTestArithmetic, ADC_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_ADC(Setter(BASE_PC + 1),
             Opcode(ADC, Immediate, 2, 2),
             false);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_ADC(Setter(0x0020),
             Opcode(ADC, ZeroPage, 2, 3),
             false);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_ADC(Setter(0x0028),
             Opcode(ADC, ZeroPageX, 2, 4),
             false);
}

TEST_F(CpuTestArithmetic, ADC_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_ADC(Setter(0x0120),
             Opcode(ADC, Absolute, 3, 4),
             false);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_ADC(Setter(0x0128),
             Opcode(ADC, AbsoluteX, 3, 4),
             false);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_ADC(Setter(0x0210),
             Opcode(ADC, AbsoluteX, 3, 4),
             true);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_ADC(Setter(0x0128),
             Opcode(ADC, AbsoluteY, 3, 4),
             false);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_ADC(Setter(0x0210),
             Opcode(ADC, AbsoluteY, 3, 4),
             true);
}

TEST_F(CpuTestArithmetic, ADC_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);

    Test_ADC(Setter(0x0120),
             Opcode(ADC, IndexedIndirect, 2, 6),
             false);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0x08;
    cpu.WriteWordAt(0x0128, 0x0200);

    Test_ADC(Setter(0x0200),
             Opcode(ADC, IndirectIndexed, 2, 5),
             false);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0xF0;
    cpu.WriteWordAt(0x0210, 0x0200);

    Test_ADC(Setter(0x0200),
             Opcode(ADC, IndirectIndexed, 2, 5),
             true);
}


TEST_F(CpuTestArithmetic, SBC_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_SBC(Setter(BASE_PC + 1),
             Opcode(SBC, Immediate, 2, 2), 0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_SBC(Setter(0x0020),
             Opcode(SBC, ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_SBC(Setter(0x0028),
             Opcode(SBC, ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestArithmetic, SBC_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_SBC(Setter(0x0120),
             Opcode(SBC, Absolute, 3, 4), 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

 Test_SBC(Setter(0x0128),
             Opcode(SBC, AbsoluteX, 3, 4), 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_SBC(Setter(0x0210),
             Opcode(SBC, AbsoluteX, 3, 4), 1);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_SBC(Setter(0x0128),
             Opcode(SBC, AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_SBC(Setter(0x0210),
             Opcode(SBC, AbsoluteY, 3, 4), 1);
}

TEST_F(CpuTestArithmetic, SBC_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);

    Test_SBC(Setter(0x0120),
             Opcode(SBC, IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0x08;
    cpu.WriteWordAt(0x0128, 0x0200);

    Test_SBC(Setter(0x0200),
             Opcode(SBC, IndirectIndexed, 2, 5), 0);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0xF0;
    cpu.WriteWordAt(0x0210, 0x0200);

    Test_SBC(Setter(0x0200),
             Opcode(SBC, IndirectIndexed, 2, 5), 1);
}
