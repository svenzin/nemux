#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

struct CpuTestShift : public CpuBaseTest {
    void Test_ASL(CpuTestBench::addressing_mode_setup addressingMode) {
        auto tester = [&] (Byte m, Byte expM, Flag expC, Flag expZ, Flag expN) {
            bench.start();
            
            bench.set_target(m);
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ(expM, bench.get_target());
            EXPECT_EQ(expC, cpu->C);
            EXPECT_EQ(expZ, cpu->Z);
            EXPECT_EQ(expN, cpu->N);
        };

        (bench.*addressingMode)(ASL);
        tester(0x24, 0x48, 0, 0, 0); // Shift
        tester(0x00, 0x00, 0, 1, 0); // Zero flag
        tester(0x80, 0x00, 1, 1, 0); // Zero flag
        tester(0x01, 0x02, 0, 0, 0); // Carry flag
        tester(0x81, 0x02, 1, 0, 0); // Carry flag
        tester(0xA0, 0x40, 1, 0, 0); // Negative flag
        tester(0xF0, 0xE0, 1, 0, 1); // Negative flag
    }

    void Test_LSR(CpuTestBench::addressing_mode_setup addressingMode) {
        auto tester = [&] (Byte m, Byte expM, Flag expC, Flag expZ) {
            bench.start();
            
            bench.set_target(m);
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ(expM, bench.get_target());
            EXPECT_EQ(expC, cpu->C);
            EXPECT_EQ(expZ, cpu->Z);
        };

        (bench.*addressingMode)(LSR);
        tester(0x24, 0x12, 0, 0); // Shift
        tester(0x00, 0x00, 0, 1); // Zero flag
        tester(0x01, 0x00, 1, 1); // Carry flag
    }

    void TesterRotate(Byte m, Byte c, Byte expM, Flag expC, Flag expZ, Flag expN) {
        bench.start();
        
        bench.set_target(m);
        cpu->C = c;
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(expM, bench.get_target());
        EXPECT_EQ(expC, cpu->C);
        EXPECT_EQ(expZ, cpu->Z);
        EXPECT_EQ(expN, cpu->N);
    };

    void Test_ROL(CpuTestBench::addressing_mode_setup addressingMode) {
        (bench.*addressingMode)(ROL);
        TesterRotate(0x21, 0, 0x42, 0, 0, 0); // Shift
        TesterRotate(0x21, 1, 0x43, 0, 0, 0); // Shift w/ Carry
        TesterRotate(0x00, 0, 0x00, 0, 1, 0); // Zero
        TesterRotate(0x80, 0, 0x00, 1, 1, 0); // Carry
        TesterRotate(0x88, 1, 0x11, 1, 0, 0); // Carry
        TesterRotate(0x40, 0, 0x80, 0, 0, 1); // Negative
    }

    void Test_ROR(CpuTestBench::addressing_mode_setup addressingMode) {
        (bench.*addressingMode)(ROR);
        TesterRotate(0x42, 0, 0x21, 0, 0, 0); // Shift
        TesterRotate(0x02, 1, 0x81, 0, 0, 1); // Shift w/ Carry
        TesterRotate(0x00, 0, 0x00, 0, 1, 0); // Zero
        TesterRotate(0x01, 0, 0x00, 1, 1, 0); // Carry
        TesterRotate(0x11, 1, 0x88, 1, 0, 1); // Carry
        TesterRotate(0x00, 1, 0x80, 0, 0, 1); // Negative
    }
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestShift, ASL_Accumulator) {
    Test_ASL(&CpuTestBench::accumulator);
}

TEST_F(CpuTestShift, ASL_ZeroPage) {
    Test_ASL(&CpuTestBench::zeropage);
}

TEST_F(CpuTestShift, ASL_ZeroPageX) {
    Test_ASL(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestShift, ASL_ZeroPageX_Wraparound) {
    Test_ASL(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestShift, ASL_Absolute) {
    Test_ASL(&CpuTestBench::absolute);
}

TEST_F(CpuTestShift, ASL_AbsoluteX) {
    Test_ASL(&CpuTestBench::absolute_x);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestShift, LSR_Accumulator) {
    Test_LSR(&CpuTestBench::accumulator);
}

TEST_F(CpuTestShift, LSR_ZeroPage) {
    Test_LSR(&CpuTestBench::zeropage);
}

TEST_F(CpuTestShift, LSR_ZeroPageX) {
    Test_LSR(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestShift, LSR_ZeroPageX_Wraparound) {
    Test_LSR(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestShift, LSR_Absolute) {
    Test_LSR(&CpuTestBench::absolute);
}

TEST_F(CpuTestShift, LSR_AbsoluteX) {
    Test_LSR(&CpuTestBench::absolute_x);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestShift, ROL_Accumulator) {
    Test_ROL(&CpuTestBench::accumulator);
}

TEST_F(CpuTestShift, ROL_ZeroPage) {
    Test_ROL(&CpuTestBench::zeropage);
}

TEST_F(CpuTestShift, ROL_ZeroPageX) {
    Test_ROL(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestShift, ROL_ZeroPageX_Wraparound) {
    Test_ROL(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestShift, ROL_Absolute) {
    Test_ROL(&CpuTestBench::absolute);
}

TEST_F(CpuTestShift, ROL_AbsoluteX) {
    Test_ROL(&CpuTestBench::absolute_x);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestShift, ROR_Accumulator) {
    Test_ROR(&CpuTestBench::accumulator);
}

TEST_F(CpuTestShift, ROR_ZeroPage) {
    Test_ROR(&CpuTestBench::zeropage);
}

TEST_F(CpuTestShift, ROR_ZeroPageX) {
    Test_ROR(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestShift, ROR_ZeroPageX_Wraparound) {
    Test_ROR(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestShift, ROR_Absolute) {
    Test_ROR(&CpuTestBench::absolute);
}

TEST_F(CpuTestShift, ROR_AbsoluteX) {
    Test_ROR(&CpuTestBench::absolute_x);
}
