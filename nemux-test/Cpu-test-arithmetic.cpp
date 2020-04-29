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

    CpuTestArithmetic() : cpu("6502", &memory) {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
    }

    MemoryBlock<0x10000> memory;
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

        // 0x40 + 0xDF + 1 = 0x120
        tester(0x40, 0x20, 1, 0x20, 1, 0, 0, 0); // pos pos C > pos
        // 0x40 + 0xDF + 0 = 0x11F
        tester(0x40, 0x20, 0, 0x1F, 1, 0, 0, 0); // pos pos !C > pos
        // 0x00 + 0xFF + 1 = 0x100
        tester(0x00, 0x00, 1, 0x00, 1, 1, 0, 0); // Zero
        // 0x80 + 0x80 + 0 = 0x100
        tester(0x80, 0x7F, 0, 0x00, 1, 1, 1, 0); // Zero by carry
        // 0x00 + 0x00 + 0 = 0x000
        tester(0x00, 0xFF, 0, 0x00, 0, 1, 0, 0); // Zero by overflow
        // 0x20 + 0xBF + 1 = 0x0E0
        tester(0x20, 0x40, 1, 0xE0, 0, 0, 0, 1); // Carry w/o overflow
        // 0x20 + 0x5F + 1 = 0x080
        tester(0x20, 0xA0, 1, 0x80, 0, 0, 1, 1); // Carry w/ overflow
        // 0x00 + 0xFF + 0 = 0x0FF
        tester(0x00, 0x00, 0, 0xFF, 0, 0, 0, 1); // Carry by borrow
        // 0x20 + 0x5F + 1 = 0x080
        tester(0x20, 0xA0, 1, 0x80, 0, 0, 1, 1); // Overflow pos > neg
        // 0x80 + 0xDF + 1 = 0x160
        tester(0x80, 0x20, 1, 0x60, 1, 0, 1, 0); // Overflow neg > pos
        // 0x80 + 0xFF + 1 = 0x180
        tester(0x80, 0x00, 1, 0x80, 1, 0, 0, 1); // Negative

        // 0x40 + 0xBE + 1 = 0x0FF
        tester(0x40, 0x41, 1, 0xFF, 0, 0, 0, 1); // Negative
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
        tester(0x20, 0x40, 0, 0, 1); // MSB set

        tester(0xF0, 0xE0, 1, 0, 0); // Greater
        tester(0xF0, 0xF0, 1, 1, 0); // Equal
        tester(0xF0, 0xF8, 0, 0, 1); // MSB set

        tester(0x80, 0x00, 1, 0, 1); // MSB set
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

TEST_F(CpuTestArithmetic, CPX_Immediate) {
    cpu.WriteByteAt(BASE_PC, 0xFF);

    Test_Compare(Setter(cpu.X), Setter(BASE_PC + 1),
        Opcode(CPX, Immediate, 2, 2), 0);
}

TEST_F(CpuTestArithmetic, CPX_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);

    Test_Compare(Setter(cpu.X), Setter(0x0020),
        Opcode(CPX, ZeroPage, 2, 3), 0
    );
}

TEST_F(CpuTestArithmetic, CPX_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_Compare(Setter(cpu.X), Setter(0x0120),
        Opcode(CPX, Absolute, 3, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CPY_Immediate) {
    cpu.WriteByteAt(BASE_PC, 0xFF);

    Test_Compare(Setter(cpu.Y), Setter(BASE_PC + 1),
        Opcode(CPY, Immediate, 2, 2), 0
    );
}

TEST_F(CpuTestArithmetic, CPY_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);

    Test_Compare(Setter(cpu.Y), Setter(0x0020),
        Opcode(CPY, ZeroPage, 2, 3), 0
    );
}

TEST_F(CpuTestArithmetic, CPY_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_Compare(Setter(cpu.Y), Setter(0x0120),
        Opcode(CPY, Absolute, 3, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_Immediate) {
    cpu.WriteByteAt(BASE_PC, 0xFF);

    Test_Compare(Setter(cpu.A), Setter(BASE_PC + 1),
        Opcode(CMP, Immediate, 2, 2), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x20);

    Test_Compare(Setter(cpu.A), Setter(0x0020),
        Opcode(CMP, ZeroPage, 2, 3), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_Compare(Setter(cpu.A), Setter(0x0028),
        Opcode(CMP, ZeroPageX, 2, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;

    Test_Compare(Setter(cpu.A), Setter(0x0000),
        Opcode(CMP, ZeroPageX, 2, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_Compare(Setter(cpu.A), Setter(0x0120),
        Opcode(CMP, Absolute, 3, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_Compare(Setter(cpu.A), Setter(0x0128),
        Opcode(CMP, AbsoluteX, 3, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_Compare(Setter(cpu.A), Setter(0x0210),
        Opcode(CMP, AbsoluteX, 3, 4), 1
    );
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_Compare(Setter(cpu.A), Setter(0x0128),
        Opcode(CMP, AbsoluteY, 3, 4), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_Compare(Setter(cpu.A), Setter(0x0210),
        Opcode(CMP, AbsoluteY, 3, 4), 1
    );
}

TEST_F(CpuTestArithmetic, CMP_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);

    Test_Compare(Setter(cpu.A), Setter(0x0120),
        Opcode(CMP, IndexedIndirect, 2, 6), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);

    Test_Compare(Setter(cpu.A), Setter(0x0120),
        Opcode(CMP, IndexedIndirect, 2, 6), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0x08;

    Test_Compare(Setter(cpu.A), Setter(0x0128),
        Opcode(CMP, IndirectIndexed, 2, 5), 0
    );
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0xF0;

    Test_Compare(Setter(cpu.A), Setter(0x0210),
        Opcode(CMP, IndirectIndexed, 2, 5), 1
    );
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;

    Test_Compare(Setter(cpu.A), Setter(0x000F),
        Opcode(CMP, IndirectIndexed, 2, 5), 1
    );
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;

    Test_Compare(Setter(cpu.A), Setter(0x0130),
        Opcode(CMP, IndirectIndexed, 2, 5), 0
    );
}

TEST_F(CpuTestArithmetic, ADC_Immediate) {
    cpu.WriteByteAt(BASE_PC, 0xFF);

    Test_ADC(Setter(BASE_PC + 1),
             Opcode(ADC, Immediate, 2, 2),
             false);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);

    Test_ADC(Setter(0x0020),
             Opcode(ADC, ZeroPage, 2, 3),
             false);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_ADC(Setter(0x0028),
             Opcode(ADC, ZeroPageX, 2, 4),
             false);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;

    Test_ADC(Setter(0x0000),
             Opcode(ADC, ZeroPageX, 2, 4),
             false);
}

TEST_F(CpuTestArithmetic, ADC_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_ADC(Setter(0x0120),
             Opcode(ADC, Absolute, 3, 4),
             false);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_ADC(Setter(0x0128),
             Opcode(ADC, AbsoluteX, 3, 4),
             false);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_ADC(Setter(0x0210),
             Opcode(ADC, AbsoluteX, 3, 4),
             true);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_ADC(Setter(0x0128),
             Opcode(ADC, AbsoluteY, 3, 4),
             false);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_ADC(Setter(0x0210),
             Opcode(ADC, AbsoluteY, 3, 4),
             true);
}

TEST_F(CpuTestArithmetic, ADC_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);

    Test_ADC(Setter(0x0120),
             Opcode(ADC, IndexedIndirect, 2, 6),
             false);
}

TEST_F(CpuTestArithmetic, ADC_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0x00FF, 0x20);
    cpu.WriteByteAt(0x0000, 0x01); 

    Test_ADC(Setter(0x0120),
             Opcode(ADC, IndexedIndirect, 2, 6),
             false);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed) {
    // Same page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0x08;

    Test_ADC(Setter(0x0128),
             Opcode(ADC, IndirectIndexed, 2, 5),
             false);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed_CrossingPage) {
    // Crossing page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0xF0;

    Test_ADC(Setter(0x0210),
        Opcode(ADC, IndirectIndexed, 2, 5),
        true);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed_CrossingWordsize) {
    // Crossing page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;

    Test_ADC(Setter(0x000F),
        Opcode(ADC, IndirectIndexed, 2, 5),
        true);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed_BaseFromZeroPage) {
    // Crossing page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;

    Test_ADC(Setter(0x0130),
        Opcode(ADC, IndirectIndexed, 2, 5),
        false);
}

TEST_F(CpuTestArithmetic, SBC_Immediate) {
    cpu.WriteByteAt(BASE_PC, 0xFF);

    Test_SBC(Setter(BASE_PC + 1),
             Opcode(SBC, Immediate, 2, 2), 0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);

    Test_SBC(Setter(0x0020),
             Opcode(SBC, ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_SBC(Setter(0x0028),
             Opcode(SBC, ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;

    Test_SBC(Setter(0x0000),
             Opcode(SBC, ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTestArithmetic, SBC_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    Test_SBC(Setter(0x0120),
             Opcode(SBC, Absolute, 3, 4), 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

 Test_SBC(Setter(0x0128),
             Opcode(SBC, AbsoluteX, 3, 4), 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_SBC(Setter(0x0210),
             Opcode(SBC, AbsoluteX, 3, 4), 1);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_SBC(Setter(0x0128),
             Opcode(SBC, AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_SBC(Setter(0x0210),
             Opcode(SBC, AbsoluteY, 3, 4), 1);
}

TEST_F(CpuTestArithmetic, SBC_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);

    Test_SBC(Setter(0x0120),
             Opcode(SBC, IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTestArithmetic, SBC_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);

    Test_SBC(Setter(0x0120),
             Opcode(SBC, IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed) {
    // Same page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0x08;

    Test_SBC(Setter(0x0128),
             Opcode(SBC, IndirectIndexed, 2, 5), 0);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed_CrossingPage) {
    // Crossing page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x120);
    cpu.Y = 0xF0;

    Test_SBC(Setter(0x0210),
             Opcode(SBC, IndirectIndexed, 2, 5), 1);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed_CrossingWordsize) {
    // Crossing page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;

    Test_SBC(Setter(0x000F),
        Opcode(SBC, IndirectIndexed, 2, 5), 1);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed_BaseFromZeroPage) {
    // Crossing page
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;

    Test_SBC(Setter(0x0130),
        Opcode(SBC, IndirectIndexed, 2, 5), 0);
}
