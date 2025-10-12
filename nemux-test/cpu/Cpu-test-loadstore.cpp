#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

struct CpuTestLoadStore : public CpuBaseTest {
    template<InstructionSet_6502::OpName OP>
    void Test_Set(Byte& reg, CpuTestBench::addressing_mode_setup addressingMode) {
        (bench.*addressingMode)(OP);
        bench.start();
        
        reg = 0x80;
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(0x80, bench.get_target());
    }

    template<InstructionSet_6502::OpName OP>
    void Test_Load(Byte& reg, CpuTestBench::addressing_mode_setup addressingMode, int extra) {
        auto tester = [&] (Byte m, Flag expZ, Flag expN) {
            bench.start();

            bench.set_target(m);
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(extra), cpu->GetTicks());
            EXPECT_EQ(m, reg);
            EXPECT_EQ(expZ, cpu->Z);
            EXPECT_EQ(expN, cpu->N);
        };

        (bench.*addressingMode)(OP);
        tester(0x20, 0, 0);
        tester(0x00, 1, 0);
        tester(0x80, 0, 1);
    }
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestLoadStore, LDX_Immediate) {
    Test_Load<LDX>(cpu->X, &CpuTestBench::immediate, 0);
}

TEST_F(CpuTestLoadStore, LDX_ZeroPage) {
    Test_Load<LDX>(cpu->X, &CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestLoadStore, LDX_ZeroPageY) {
    Test_Load<LDX>(cpu->X, &CpuTestBench::zeropage_y, 0);
}

TEST_F(CpuTestLoadStore, LDX_ZeroPageY_Wraparound) {
    Test_Load<LDX>(cpu->X, &CpuTestBench::zeropage_y_wraparound, 0);
}

TEST_F(CpuTestLoadStore, LDX_Absolute) {
    Test_Load<LDX>(cpu->X, &CpuTestBench::absolute, 0);
}

TEST_F(CpuTestLoadStore, LDX_AbsoluteY) {
    Test_Load<LDX>(cpu->X, &CpuTestBench::absolute_y, 0);
}

TEST_F(CpuTestLoadStore, LDX_AbsoluteY_CrossingPage) {
    Test_Load<LDX>(cpu->X, &CpuTestBench::absolute_y_crossing_page, 1);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestLoadStore, LDY_Immediate) {
    Test_Load<LDY>(cpu->Y, &CpuTestBench::immediate, 0);
}

TEST_F(CpuTestLoadStore, LDY_ZeroPage) {
    Test_Load<LDY>(cpu->Y, &CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestLoadStore, LDY_ZeroPageX) {
    Test_Load<LDY>(cpu->Y, &CpuTestBench::zeropage_x, 0);
}

TEST_F(CpuTestLoadStore, LDY_ZeroPageX_Wraparound) {
    Test_Load<LDY>(cpu->Y, &CpuTestBench::zeropage_x_wraparound, 0);
}

TEST_F(CpuTestLoadStore, LDY_Absolute) {
    Test_Load<LDY>(cpu->Y, &CpuTestBench::absolute, 0);
}

TEST_F(CpuTestLoadStore, LDY_AbsoluteX) {
    Test_Load<LDY>(cpu->Y, &CpuTestBench::absolute_x, 0);
}

TEST_F(CpuTestLoadStore, LDY_AbsoluteX_CrossingPage) {
    Test_Load<LDY>(cpu->Y, &CpuTestBench::absolute_x_crossing_page, 1);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestLoadStore, LDA_Immediate) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::immediate, 0);
}

TEST_F(CpuTestLoadStore, LDA_ZeroPage) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestLoadStore, LDA_ZeroPageX) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::zeropage_x, 0);
}

TEST_F(CpuTestLoadStore, LDA_ZeroPageX_Wraparound) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::zeropage_x_wraparound, 0);
}

TEST_F(CpuTestLoadStore, LDA_Absolute) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::absolute, 0);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteX) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::absolute_x, 0);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteX_CrossingPage) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::absolute_x_crossing_page, 1);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteY) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::absolute_y, 0);
}

TEST_F(CpuTestLoadStore, LDA_AbsoluteY_CrossingPage) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::absolute_y_crossing_page, 1);
}

TEST_F(CpuTestLoadStore, LDA_IndexedIndirect) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::indirect_x, 0);
}

TEST_F(CpuTestLoadStore, LDA_IndexedIndirect_Wraparound) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::indirect_x_wraparound, 0);
}

TEST_F(CpuTestLoadStore, LDA_IndirectIndexed) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::indirect_y, 0);
}

TEST_F(CpuTestLoadStore, LDA_IndirectIndexed_CrossingPage) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::indirect_y_crossing_page, 1);
}

TEST_F(CpuTestLoadStore, LDA_IndirectIndexed_CrossingWordsize) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::indirect_y_crossing_word_size, 1);
}

TEST_F(CpuTestLoadStore, LDA_IndirectIndexed_BaseFromZeroPage) {
    Test_Load<LDA>(cpu->A, &CpuTestBench::indirect_y_base_from_zeropage, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestLoadStore, STX_ZeroPage) {
    Test_Set<STX>(cpu->X, &CpuTestBench::zeropage);
}

TEST_F(CpuTestLoadStore, STX_ZeroPageY) {
    Test_Set<STX>(cpu->X, &CpuTestBench::zeropage_y);
}

TEST_F(CpuTestLoadStore, STX_ZeroPageY_Wraparound) {
    Test_Set<STX>(cpu->X, &CpuTestBench::zeropage_y_wraparound);
}

TEST_F(CpuTestLoadStore, STX_Absolute) {
    Test_Set<STX>(cpu->X, &CpuTestBench::absolute);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestLoadStore, STY_ZeroPage) {
    Test_Set<STY>(cpu->Y, &CpuTestBench::zeropage);
}

TEST_F(CpuTestLoadStore, STY_ZeroPageX) {
    Test_Set<STY>(cpu->Y, &CpuTestBench::zeropage_x);
}

TEST_F(CpuTestLoadStore, STY_ZeroPageX_Wraparound) {
    Test_Set<STY>(cpu->Y, &CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestLoadStore, STY_Absolute) {
    Test_Set<STY>(cpu->Y, &CpuTestBench::absolute);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestLoadStore, STA_ZeroPage) {
    Test_Set<STA>(cpu->A, &CpuTestBench::zeropage);
}

TEST_F(CpuTestLoadStore, STA_ZeroPageX) {
    Test_Set<STA>(cpu->A, &CpuTestBench::zeropage_x);
}

TEST_F(CpuTestLoadStore, STA_ZeroPageX_Wraparound) {
    Test_Set<STA>(cpu->A, &CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestLoadStore, STA_Absolute) {
    Test_Set<STA>(cpu->A, &CpuTestBench::absolute);
}

TEST_F(CpuTestLoadStore, STA_AbsoluteX) {
    Test_Set<STA>(cpu->A, &CpuTestBench::absolute_x);
}

TEST_F(CpuTestLoadStore, STA_AbsoluteY) {
    Test_Set<STA>(cpu->A, &CpuTestBench::absolute_y);
}

TEST_F(CpuTestLoadStore, STA_IndexedIndirect) {
    Test_Set<STA>(cpu->A, &CpuTestBench::indirect_x);
}

TEST_F(CpuTestLoadStore, STA_IndexedIndirect_Wraparound) {
    Test_Set<STA>(cpu->A, &CpuTestBench::indirect_x_wraparound);
}

TEST_F(CpuTestLoadStore, STA_IndirectIndexed) {
    Test_Set<STA>(cpu->A, &CpuTestBench::indirect_y);
}

TEST_F(CpuTestLoadStore, STA_IndirectIndexed_CrossingWordsize) {
    Test_Set<STA>(cpu->A, &CpuTestBench::indirect_y_crossing_word_size);
}

TEST_F(CpuTestLoadStore, STA_IndirectIndexed_BaseFromZeroPage) {
    Test_Set<STA>(cpu->A, &CpuTestBench::indirect_y_base_from_zeropage);
}
