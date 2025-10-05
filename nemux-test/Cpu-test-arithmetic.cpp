#include "CpuBaseTest.h"

#include <vector>
#include <map>
#include <array>
#include <functional>

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

struct CpuTestArithmetic : public CpuBaseTest {
    template<typename Setter>
    void Test_ADC(Setter set, int extra) {
        auto tester = [&] (Byte a, Byte m, Flag c, Byte expA, Flag expC, Flag expZ, Flag expV, Flag expN) {
            bench.start();

            set(m);
            cpu.A = a;
            cpu.C = c;
            ExecuteOne();

            EXPECT_EQ(bench.PC + bench.op.Bytes, cpu.PC);
            EXPECT_EQ(bench.Ticks + bench.op.Cycles + extra, cpu.GetTicks());
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
    void Test_SBC(Setter set, int extra) {
        auto tester = [&] (Byte a, Byte m, Flag c, Byte expA, Flag expC, Flag expZ, Flag expV, Flag expN) {
            bench.start();

            set(m);
            cpu.A = a;
            cpu.C = c;
            ExecuteOne();

            EXPECT_EQ(bench.PC + bench.op.Bytes, cpu.PC);
            EXPECT_EQ(bench.Ticks + bench.op.Cycles + extra, cpu.GetTicks());
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
    void Test_Compare(SetterReg setR, SetterMem setM, int extra) {
        auto tester = [&] (Byte r, Byte m, Flag expC, Flag expZ, Flag expN) {
            bench.start();

            setR(r);
            setM(m);
            ExecuteOne();

            EXPECT_EQ(bench.PC + bench.op.Bytes, cpu.PC);
            EXPECT_EQ(bench.Ticks + bench.op.Cycles + extra, cpu.GetTicks());
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
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestArithmetic, CPX_Immediate) {
    bench.encode(CPX, IMM);
    Test_Compare(Setter(cpu.X), Setter(BASE_PC + 1), 0);
}

TEST_F(CpuTestArithmetic, CPX_ZeroPage) {
    bench.encode(CPX, ZPG)
         .db(0x20);
    Test_Compare(Setter(cpu.X), Setter(0x0020), 0);
}

TEST_F(CpuTestArithmetic, CPX_Absolute) {
    bench.encode(CPX, ABS)
         .dw(0x0120);
    Test_Compare(Setter(cpu.X), Setter(0x0120), 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestArithmetic, CPY_Immediate) {
    bench.encode(CPY, IMM);
    Test_Compare(Setter(cpu.Y), Setter(BASE_PC + 1), 0);
}

TEST_F(CpuTestArithmetic, CPY_ZeroPage) {
    bench.encode(CPY, ZPG)
         .db(0x20);
    Test_Compare(Setter(cpu.Y), Setter(0x0020), 0);
}

TEST_F(CpuTestArithmetic, CPY_Absolute) {
    bench.encode(CPY, ABS)
         .dw(0x0120);
    Test_Compare(Setter(cpu.Y), Setter(0x0120), 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestArithmetic, CMP_Immediate) {
    bench.encode(CMP, IMM);
    Test_Compare(Setter(cpu.A), Setter(BASE_PC + 1), 0);
}

TEST_F(CpuTestArithmetic, CMP_ZeroPage) {
    bench.encode(CMP, ZPG)
         .db(0x20);
    Test_Compare(Setter(cpu.A), Setter(0x0020), 0);
}

TEST_F(CpuTestArithmetic, CMP_ZeroPageX) {
    cpu.X = 0x08;
    bench.encode(CMP, ZPX)
         .db(0x20);
    Test_Compare(Setter(cpu.A), Setter(0x0028), 0);
}

TEST_F(CpuTestArithmetic, CMP_ZeroPageX_Wraparound) {
    cpu.X = 0x10;
    bench.encode(CMP, ZPX)
         .db(0xF0);
    Test_Compare(Setter(cpu.A), Setter(0x0000), 0);
}

TEST_F(CpuTestArithmetic, CMP_Absolute) {
    bench.encode(CMP, ABS)
         .dw(0x0120);
    Test_Compare(Setter(cpu.A), Setter(0x0120), 0);
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteX) {
    cpu.X = 0x08;
    bench.encode(CMP, ABX)
         .dw(0x0120);
    Test_Compare(Setter(cpu.A), Setter(0x0128), 0);
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteX_CrossingPage) {
    cpu.X = 0xF0;
    bench.encode(CMP, ABX)
         .dw(0x0120);
    Test_Compare(Setter(cpu.A), Setter(0x0210), 1);
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteY) {
    cpu.Y = 0x08;
    bench.encode(CMP, ABY)
         .dw(0x0120);
    Test_Compare(Setter(cpu.A), Setter(0x0128), 0);
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteY_CrossingPage) {
    cpu.Y = 0xF0;
    bench.encode(CMP, ABY)
         .dw(0x0120);
    Test_Compare(Setter(cpu.A), Setter(0x0210), 1);
}

TEST_F(CpuTestArithmetic, CMP_IndexedIndirect) {
    cpu.X = 0x08;
    bench.encode(CMP, IDX)
         .db(0x20)
         .at(0x0028).dw(0x0120);
    Test_Compare(Setter(cpu.A), Setter(0x0120), 0);
}

TEST_F(CpuTestArithmetic, CMP_IndexedIndirect_Wraparound) {
    cpu.X = 0x0F;
    bench.encode(CMP, IDX)
         .db(0xF0)
         .at(0x00FF).db(0x20)
         .at(0x0000).db(0x01);
    Test_Compare(Setter(cpu.A), Setter(0x0120), 0);
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed) {
    // Same page
    cpu.Y = 0x08;
    bench.encode(CMP, IDY)
         .db(0x20)
         .at(0x0020).dw(0x0120);
    Test_Compare(Setter(cpu.A), Setter(0x0128), 0);
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed_CrossingPage) {
    // Crossing page
    cpu.Y = 0xF0;
    bench.encode(CMP, IDY)
         .db(0x20)
         .at(0x0020).dw(0x0120);
    Test_Compare(Setter(cpu.A), Setter(0x0210), 1);
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed_CrossingWordsize) {
    // Crossing page
    cpu.Y = 0x10;
    bench.encode(CMP, IDY)
         .db(0x20)
         .at(0x0020).dw(0xFFFF);
    Test_Compare(Setter(cpu.A), Setter(0x000F), 1);
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed_BaseFromZeroPage) {
    // Crossing page
    cpu.Y = 0x10;
    bench.encode(CMP, IDY)
         .db(0xFF)
         .at(0x00FF).db(0x20)
         .at(0x0000).db(0x01);
    Test_Compare(Setter(cpu.A), Setter(0x0130), 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestArithmetic, ADC_Immediate) {
    bench.encode(ADC, IMM);
    Test_ADC(Setter(BASE_PC + 1), 0);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPage) {
    bench.encode(ADC, ZPG)
         .db(0x20);
    Test_ADC(Setter(0x0020), 0);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPageX) {
    cpu.X = 0x08;
    bench.encode(ADC, ZPX)
         .db(0x20);
    Test_ADC(Setter(0x0028), 0);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPageX_Wraparound) {
    cpu.X = 0x10;
    bench.encode(ADC, ZPX)
         .db(0xF0);
    Test_ADC(Setter(0x0000), 0);
}

TEST_F(CpuTestArithmetic, ADC_Absolute) {
    bench.encode(ADC, ABS)
         .dw(0x0120);
    Test_ADC(Setter(0x0120), 0);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteX) {
    cpu.X = 0x08;
    bench.encode(ADC, ABX)
         .dw(0x0120);
    Test_ADC(Setter(0x0128), 0);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteX_CrossingPage) {
    cpu.X = 0xF0;
    bench.encode(ADC, ABX)
         .dw(0x0120);
    Test_ADC(Setter(0x0210), 1);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteY) {
    cpu.Y = 0x08;
    bench.encode(ADC, ABY)
         .dw(0x0120);
    Test_ADC(Setter(0x0128), 0);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteY_CrossingPage) {
    cpu.Y = 0xF0;
    bench.encode(ADC, ABY)
         .dw(0x0120);
    Test_ADC(Setter(0x0210), 1);
}

TEST_F(CpuTestArithmetic, ADC_IndexedIndirect) {
    cpu.X = 0x08;
    bench.encode(ADC, IDX)
         .db(0x20)
         .at(0x0028).dw(0x0120);
    Test_ADC(Setter(0x0120), 0);
}

TEST_F(CpuTestArithmetic, ADC_IndexedIndirect_Wraparound) {
    cpu.X = 0x0F;
    bench.encode(ADC, IDX)
         .db(0xF0)
         .at(0x00FF).db(0x20)
         .at(0x0000).db(0x01);
    Test_ADC(Setter(0x0120), 0);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed) {
    // Same page
    cpu.Y = 0x08;
    bench.encode(ADC, IDY)
         .db(0x20)
         .at(0x0020).dw(0x0120);
    Test_ADC(Setter(0x0128), 0);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed_CrossingPage) {
    // Crossing page
    cpu.Y = 0xF0;
    bench.encode(ADC, IDY)
         .db(0x20)
         .at(0x0020).dw(0x0120);
    Test_ADC(Setter(0x0210), 1);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed_CrossingWordsize) {
    // Crossing page
    cpu.Y = 0x10;
    bench.encode(ADC, IDY)
         .db(0x20)
         .at(0x0020).dw(0xFFFF);
    Test_ADC(Setter(0x000F), 1);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed_BaseFromZeroPage) {
    // Crossing page
    cpu.Y = 0x10;
    bench.encode(ADC, IDY)
         .db(0xFF)
         .at(0x00FF).db(0x20)
         .at(0x0000).db(0x01);
    Test_ADC(Setter(0x0130), 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestArithmetic, SBC_Immediate) {
    bench.encode(SBC, IMM);
    Test_SBC(Setter(BASE_PC + 1), 0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPage) {
    bench.encode(SBC, ZPG)
         .db(0x20);
    Test_SBC(Setter(0x0020), 0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPageX) {
    cpu.X = 0x08;
    bench.encode(SBC, ZPX)
         .db(0x20);
    Test_SBC(Setter(0x0028), 0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPageX_Wraparound) {
    cpu.X = 0x10;
    bench.encode(SBC, ZPX)
         .db(0xF0);
    Test_SBC(Setter(0x0000), 0);
}

TEST_F(CpuTestArithmetic, SBC_Absolute) {
    bench.encode(SBC, ABS)
         .dw(0x0120);
    Test_SBC(Setter(0x0120), 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteX) {
    cpu.X = 0x08;
    bench.encode(SBC, ABX)
         .dw(0x0120);
    Test_SBC(Setter(0x0128), 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteX_CrossingPage) {
    cpu.X = 0xF0;
    bench.encode(SBC, ABX)
         .dw(0x0120);
    Test_SBC(Setter(0x0210), 1);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteY) {
    cpu.Y = 0x08;
    bench.encode(SBC, ABY)
         .dw(0x0120);
    Test_SBC(Setter(0x0128), 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteY_CrossingPage) {
    cpu.Y = 0xF0;
    bench.encode(SBC, ABY)
         .dw(0x0120);
    Test_SBC(Setter(0x0210), 1);
}

TEST_F(CpuTestArithmetic, SBC_IndexedIndirect) {
    cpu.X = 0x08;
    bench.encode(SBC, IDX)
         .db(0x20)
         .at(0x0028).dw(0x0120);
    Test_SBC(Setter(0x0120), 0);
}

TEST_F(CpuTestArithmetic, SBC_IndexedIndirect_Wraparound) {
    cpu.X = 0x0F;
    bench.encode(SBC, IDX)
         .db(0xF0)
         .at(0x00FF).db(0x20)
         .at(0x0000).db(0x01);
    Test_SBC(Setter(0x0120), 0);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed) {
    // Same page
    cpu.Y = 0x08;
    bench.encode(SBC, IDY)
         .db(0x20)
         .at(0x0020).dw(0x0120);
    Test_SBC(Setter(0x0128), 0);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed_CrossingPage) {
    // Crossing page
    cpu.Y = 0xF0;
    bench.encode(SBC, IDY)
         .db(0x20)
         .at(0x0020).dw(0x0120);
    Test_SBC(Setter(0x0210), 1);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed_CrossingWordsize) {
    // Crossing page
    cpu.Y = 0x10;
    bench.encode(SBC, IDY)
         .db(0x20)
         .at(0x0020).dw(0xFFFF);
    Test_SBC(Setter(0x000F), 1);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed_BaseFromZeroPage) {
    // Crossing page
    cpu.Y = 0x10;
    bench.encode(SBC, IDY)
         .db(0xFF)
         .at(0x00FF).db(0x20)
         .at(0x0000).db(0x01);
    Test_SBC(Setter(0x0130), 0);
}
