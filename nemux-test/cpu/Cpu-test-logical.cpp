#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

struct CpuTestLogical : public CpuBaseTest {
    void Tester(Byte a, Byte m, Byte expA, Flag expZ, Flag expN, int extra) {
        bench.start();

        bench.set_target(m);
        cpu->A = a;
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(extra), cpu->GetTicks());
        EXPECT_EQ(expA, cpu->A);
        EXPECT_EQ(expZ, cpu->Z);
        EXPECT_EQ(expN, cpu->N);
    }

    void Test_AND(CpuTestBench::addressing_mode_setup addressingMode, int extra) {
        (bench.*addressingMode)(AND);
        Tester(0x66, 0x00, 0x00, 1, 0, extra); // Zero
        Tester(0xFF, 0x80, 0x80, 0, 1, extra); // Negative
        Tester(0xAA, 0x24, 0x20, 0, 0, extra); // Normal
    }

    void Test_EOR(CpuTestBench::addressing_mode_setup addressingMode, int extra) {
        (bench.*addressingMode)(EOR);
        Tester(0x0C, 0x0A, 0x06, 0, 0, extra); // 1100b XOR 1010b = 0110b
        Tester(0x0C, 0x0C, 0x00, 1, 0, extra); // 1100b XOR 1100b = 0000b
        Tester(0x8C, 0x0C, 0x80, 0, 1, extra); // 10001100b XOR 00001100b = 10000000b
    }

    void Test_ORA(CpuTestBench::addressing_mode_setup addressingMode, int extra) {
        (bench.*addressingMode)(ORA);
        Tester(0x0C, 0x0A, 0x0E, 0, 0, extra); // 1100b OR 1010b = 1110b
        Tester(0x00, 0x00, 0x00, 1, 0, extra); // 0000b OR 0000b = 0000b
        Tester(0x8C, 0x03, 0x8F, 0, 1, extra); // 10001100b OR 00000011b = 10001111b
    }
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestLogical, EOR_Immediate) {
    Test_EOR(&CpuTestBench::immediate, 0);
}

TEST_F(CpuTestLogical, EOR_ZeroPage) {
    Test_EOR(&CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestLogical, EOR_ZeroPageX) {
    Test_EOR(&CpuTestBench::zeropage_x, 0);
}

TEST_F(CpuTestLogical, EOR_ZeroPageX_Wraparound) {
    Test_EOR(&CpuTestBench::zeropage_x_wraparound, 0);
}

TEST_F(CpuTestLogical, EOR_Absolute) {
    Test_EOR(&CpuTestBench::absolute, 0);
}

TEST_F(CpuTestLogical, EOR_AbsoluteX) {
    Test_EOR(&CpuTestBench::absolute_x, 0);
}

TEST_F(CpuTestLogical, EOR_AbsoluteX_CrossingPage) {
    Test_EOR(&CpuTestBench::absolute_x_crossing_page, 1);
}

TEST_F(CpuTestLogical, EOR_AbsoluteY) {
    Test_EOR(&CpuTestBench::absolute_y, 0);
}

TEST_F(CpuTestLogical, EOR_AbsoluteY_CrossingPage) {
    Test_EOR(&CpuTestBench::absolute_y_crossing_page, 1);
}

TEST_F(CpuTestLogical, EOR_IndexedIndirect) {
    Test_EOR(&CpuTestBench::indirect_x, 0);
}

TEST_F(CpuTestLogical, EOR_IndexedIndirect_Wraparound) {
    Test_EOR(&CpuTestBench::indirect_x_wraparound, 0);
}

TEST_F(CpuTestLogical, EOR_IndirectIndexed) {
    Test_EOR(&CpuTestBench::indirect_y, 0);
}

TEST_F(CpuTestLogical, EOR_IndirectIndexed_CrossingPage) {
    Test_EOR(&CpuTestBench::indirect_y_crossing_page, 1);
}

TEST_F(CpuTestLogical, EOR_IndirectIndexed_CrossingWordsize) {
    Test_EOR(&CpuTestBench::indirect_y_crossing_word_size, 1);
}

TEST_F(CpuTestLogical, EOR_IndirectIndexed_BaseFromZeroPage) {
    Test_EOR(&CpuTestBench::indirect_y_base_from_zeropage, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestLogical, ORA_Immediate) {
    Test_ORA(&CpuTestBench::immediate, 0);
}

TEST_F(CpuTestLogical, ORA_ZeroPage) {
    Test_ORA(&CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestLogical, ORA_ZeroPageX) {
    Test_ORA(&CpuTestBench::zeropage_x, 0);
}

TEST_F(CpuTestLogical, ORA_ZeroPageX_Wraparound) {
    Test_ORA(&CpuTestBench::zeropage_x_wraparound, 0);
}

TEST_F(CpuTestLogical, ORA_Absolute) {
    Test_ORA(&CpuTestBench::absolute, 0);
}

TEST_F(CpuTestLogical, ORA_AbsoluteX) {
    Test_ORA(&CpuTestBench::absolute_x, 0);
}

TEST_F(CpuTestLogical, ORA_AbsoluteX_CrossingPage) {
    Test_ORA(&CpuTestBench::absolute_x_crossing_page, 1);
}

TEST_F(CpuTestLogical, ORA_AbsoluteY) {
    Test_ORA(&CpuTestBench::absolute_y, 0);
}

TEST_F(CpuTestLogical, ORA_AbsoluteY_CrossingPage) {
    Test_ORA(&CpuTestBench::absolute_y_crossing_page, 1);
}

TEST_F(CpuTestLogical, ORA_IndexedIndirect) {
    Test_ORA(&CpuTestBench::indirect_x, 0);
}

TEST_F(CpuTestLogical, ORA_IndexedIndirect_Wraparound) {
    Test_ORA(&CpuTestBench::indirect_x_wraparound, 0);
}

TEST_F(CpuTestLogical, ORA_IndirectIndexed) {
    Test_ORA(&CpuTestBench::indirect_y, 0);
}

TEST_F(CpuTestLogical, ORA_IndirectIndexed_CrossingPage) {
    Test_ORA(&CpuTestBench::indirect_y_crossing_page, 1);
}

TEST_F(CpuTestLogical, ORA_IndirectIndexed_CrossingWordsize) {
    Test_ORA(&CpuTestBench::indirect_y_crossing_word_size, 1);
}

TEST_F(CpuTestLogical, ORA_IndirectIndexed_BaseFromZeroPage) {
    Test_ORA(&CpuTestBench::indirect_y_base_from_zeropage, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestLogical, AND_Immediate) {
    Test_AND(&CpuTestBench::immediate, 0);
}

TEST_F(CpuTestLogical, AND_ZeroPage) {
    Test_AND(&CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestLogical, AND_ZeroPageX) {
    Test_AND(&CpuTestBench::zeropage_x, 0);
}

TEST_F(CpuTestLogical, AND_ZeroPageX_Wraparound) {
    Test_AND(&CpuTestBench::zeropage_x_wraparound, 0);
}

TEST_F(CpuTestLogical, AND_Absolute) {
    Test_AND(&CpuTestBench::absolute, 0);
}

TEST_F(CpuTestLogical, AND_AbsoluteX) {
    Test_AND(&CpuTestBench::absolute_x, 0);
}

TEST_F(CpuTestLogical, AND_AbsoluteX_CrossingPage) {
    Test_AND(&CpuTestBench::absolute_x_crossing_page, 1);
}

TEST_F(CpuTestLogical, AND_AbsoluteY) {
    Test_AND(&CpuTestBench::absolute_y, 0);
}

TEST_F(CpuTestLogical, AND_AbsoluteY_CrossingPage) {
    Test_AND(&CpuTestBench::absolute_y_crossing_page, 1);
}

TEST_F(CpuTestLogical, AND_IndexedIndirect) {
    Test_AND(&CpuTestBench::indirect_x, 0);
}

TEST_F(CpuTestLogical, AND_IndexedIndirect_Wraparound) {
    Test_AND(&CpuTestBench::indirect_x_wraparound, 0);
}

TEST_F(CpuTestLogical, AND_IndirectIndexed) {
    Test_AND(&CpuTestBench::indirect_y, 0);
}

TEST_F(CpuTestLogical, AND_IndirectIndexed_CrossingPage) {
    Test_AND(&CpuTestBench::indirect_y_crossing_page, 1);
}

TEST_F(CpuTestLogical, AND_IndirectIndexed_CrossingWordsize) {
    Test_AND(&CpuTestBench::indirect_y_crossing_word_size, 1);
}

TEST_F(CpuTestLogical, AND_IndirectIndexed_BaseFromZeroPage) {
    Test_AND(&CpuTestBench::indirect_y_base_from_zeropage, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestLogical, BIT_ZeroPage) {
    FAIL();
    for (auto a = 0; a < 0x100; ++a) {
        for (auto m = 0; m < 0x100; ++m) {
            bench.origin(0x0200)
                 .encode(BIT, ZPG)
                 .db(m)
                 .at(m).db(m)
                 .start();
            
            cpu->A = a;
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ((m & a   ) == 0 ? 1 : 0, cpu->Z);
            EXPECT_EQ((m & 0x40) == 0 ? 0 : 1, cpu->V);
            EXPECT_EQ((m & 0x80) == 0 ? 0 : 1, cpu->N);
        }
    }
}

TEST_F(CpuTestLogical, BIT_Absolute) {
    FAIL();
    for (auto a = 0; a < 0x100; ++a) {
        for (auto m = 0; m < 0x100; ++m) {
            bench.origin(0x0200)
                 .encode(BIT, ABS)
                 .dw(0x0100 + m)
                 .at(0x0100 + m).db(m)
                 .start();
            
            cpu->A = a;
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ((m & a   ) == 0 ? 1 : 0, cpu->Z);
            EXPECT_EQ((m & 0x40) == 0 ? 0 : 1, cpu->V);
            EXPECT_EQ((m & 0x80) == 0 ? 0 : 1, cpu->N);
        }
    }
}
