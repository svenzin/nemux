#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

struct CpuTestArithmetic : public CpuBaseTest {
    void Test_ADC(CpuTestBench::addressing_mode_setup addressingMode, int extra) {
        auto tester = [&] (Byte a, Byte m, Flag c, Byte expA, Flag expC, Flag expZ, Flag expV, Flag expN) {
            bench.start();
            
            bench.set_target(m);
            cpu.A = a;
            cpu.C = c;
            ExecuteOne();
            
            EXPECT_EQ(bench.expected_PC(), cpu.PC);
            EXPECT_EQ(bench.expected_ticks(extra), cpu.Ticks);
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expV, cpu.V);
            EXPECT_EQ(expN, cpu.N);
        };
        
        (bench.*addressingMode)(ADC);
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

    void Test_SBC(CpuTestBench::addressing_mode_setup addressingMode, int extra) {
        auto tester = [&] (Byte a, Byte m, Flag c, Byte expA, Flag expC, Flag expZ, Flag expV, Flag expN) {
            bench.start();

            bench.set_target(m);
            cpu.A = a;
            cpu.C = c;
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu.PC);
            EXPECT_EQ(bench.expected_ticks(extra), cpu.Ticks);
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expV, cpu.V);
            EXPECT_EQ(expN, cpu.N);
        };

        (bench.*addressingMode)(SBC);
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

    template<InstructionSet_6502::OpName OP>
    void Test_Compare(Byte& reg, CpuTestBench::addressing_mode_setup addressingMode, int extra) {
        auto tester = [&] (Byte r, Byte m, Flag expC, Flag expZ, Flag expN) {
            bench.start();

            reg = r;
            bench.set_target(m);
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu.PC);
            EXPECT_EQ(bench.expected_ticks(extra), cpu.Ticks);
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        (bench.*addressingMode)(OP);
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
    Test_Compare<CPX>(cpu.X, &CpuTestBench::immediate, 0);
}

TEST_F(CpuTestArithmetic, CPX_ZeroPage) {
    Test_Compare<CPX>(cpu.X, &CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestArithmetic, CPX_Absolute) {
    Test_Compare<CPX>(cpu.X, &CpuTestBench::absolute, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestArithmetic, CPY_Immediate) {
    Test_Compare<CPY>(cpu.Y, &CpuTestBench::immediate, 0);
}

TEST_F(CpuTestArithmetic, CPY_ZeroPage) {
    Test_Compare<CPY>(cpu.Y, &CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestArithmetic, CPY_Absolute) {
    Test_Compare<CPY>(cpu.Y, &CpuTestBench::absolute, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestArithmetic, CMP_Immediate) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::immediate, 0);
}

TEST_F(CpuTestArithmetic, CMP_ZeroPage) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestArithmetic, CMP_ZeroPageX) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::zeropage_x, 0);
}

TEST_F(CpuTestArithmetic, CMP_ZeroPageX_Wraparound) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::zeropage_x_wraparound, 0);
}

TEST_F(CpuTestArithmetic, CMP_Absolute) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::absolute, 0);
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteX) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::absolute_x, 0);
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteX_CrossingPage) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::absolute_x_crossing_page, 1);
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteY) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::absolute_y, 0);
}

TEST_F(CpuTestArithmetic, CMP_AbsoluteY_CrossingPage) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::absolute_y_crossing_page, 1);
}

TEST_F(CpuTestArithmetic, CMP_IndexedIndirect) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::indirect_x, 0);
}

TEST_F(CpuTestArithmetic, CMP_IndexedIndirect_Wraparound) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::indirect_x_wraparound, 0);
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::indirect_y, 0);
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed_CrossingPage) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::indirect_y_crossing_page, 1);
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed_CrossingWordsize) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::indirect_y_crossing_word_size, 1);
}

TEST_F(CpuTestArithmetic, CMP_IndirectIndexed_BaseFromZeroPage) {
    Test_Compare<CMP>(cpu.A, &CpuTestBench::indirect_y_base_from_zeropage, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestArithmetic, ADC_Immediate) {
    Test_ADC(&CpuTestBench::immediate, 0);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPage) {
    Test_ADC(&CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPageX) {
    Test_ADC(&CpuTestBench::zeropage_x, 0);
}

TEST_F(CpuTestArithmetic, ADC_ZeroPageX_Wraparound) {
    Test_ADC(&CpuTestBench::zeropage_x_wraparound, 0);
}

TEST_F(CpuTestArithmetic, ADC_Absolute) {
    Test_ADC(&CpuTestBench::absolute, 0);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteX) {
    Test_ADC(&CpuTestBench::absolute_x, 0);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteX_CrossingPage) {
    Test_ADC(&CpuTestBench::absolute_x_crossing_page, 1);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteY) {
    Test_ADC(&CpuTestBench::absolute_y, 0);
}

TEST_F(CpuTestArithmetic, ADC_AbsoluteY_CrossingPage) {
    Test_ADC(&CpuTestBench::absolute_y_crossing_page, 1);
}

TEST_F(CpuTestArithmetic, ADC_IndexedIndirect) {
    Test_ADC(&CpuTestBench::indirect_x, 0);
}

TEST_F(CpuTestArithmetic, ADC_IndexedIndirect_Wraparound) {
    Test_ADC(&CpuTestBench::indirect_x_wraparound, 0);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed) {
    Test_ADC(&CpuTestBench::indirect_y, 0);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed_CrossingPage) {
    Test_ADC(&CpuTestBench::indirect_y_crossing_page, 1);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed_CrossingWordsize) {
    Test_ADC(&CpuTestBench::indirect_y_crossing_word_size, 1);
}

TEST_F(CpuTestArithmetic, ADC_IndirectIndexed_BaseFromZeroPage) {
    Test_ADC(&CpuTestBench::indirect_y_base_from_zeropage, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestArithmetic, SBC_Immediate) {
    Test_SBC(&CpuTestBench::immediate,  0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPage) {
    Test_SBC(&CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPageX) {
    Test_SBC(&CpuTestBench::zeropage_x, 0);
}

TEST_F(CpuTestArithmetic, SBC_ZeroPageX_Wraparound) {
    Test_SBC(&CpuTestBench::zeropage_x_wraparound, 0);
}

TEST_F(CpuTestArithmetic, SBC_Absolute) {
    Test_SBC(&CpuTestBench::absolute, 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteX) {
    Test_SBC(&CpuTestBench::absolute_x, 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteX_CrossingPage) {
    Test_SBC(&CpuTestBench::absolute_x_crossing_page, 1);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteY) {
    Test_SBC(&CpuTestBench::absolute_y, 0);
}

TEST_F(CpuTestArithmetic, SBC_AbsoluteY_CrossingPage) {
    Test_SBC(&CpuTestBench::absolute_y_crossing_page, 1);
}

TEST_F(CpuTestArithmetic, SBC_IndexedIndirect) {
    Test_SBC(&CpuTestBench::indirect_x, 0);
}

TEST_F(CpuTestArithmetic, SBC_IndexedIndirect_Wraparound) {
    Test_SBC(&CpuTestBench::indirect_x_wraparound, 0);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed) {
    Test_SBC(&CpuTestBench::indirect_y, 0);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed_CrossingPage) {
    Test_SBC(&CpuTestBench::indirect_y_crossing_page, 1);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed_CrossingWordsize) {
    Test_SBC(&CpuTestBench::indirect_y_crossing_word_size, 1);
}

TEST_F(CpuTestArithmetic, SBC_IndirectIndexed_BaseFromZeroPage) {
    Test_SBC(&CpuTestBench::indirect_y_base_from_zeropage, 0);
}
