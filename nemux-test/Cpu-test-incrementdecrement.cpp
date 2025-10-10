#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

struct CpuTestIncrementDecrement : public CpuBaseTest {
    void Tester(Byte m, Byte expM, Flag expZ, Flag expN) {
        bench.start();

        bench.set_target(m);
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu.PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu.Ticks);
        EXPECT_EQ(expM, bench.get_target());
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expN, cpu.N);
    };

    template <InstructionSet_6502::OpName OP>
    void Test_DEC(CpuTestBench::addressing_mode_setup addressingMode) {
        (bench.*addressingMode)(OP);
        Tester(0x08, 0x07, 0, 0);
        Tester(0x01, 0x00, 1, 0);
        Tester(0x00, 0xFF, 0, 1);
        Tester(0x80, 0x7F, 0, 0);
    }

    template <InstructionSet_6502::OpName OP>
    void Test_INC(CpuTestBench::addressing_mode_setup addressingMode) {
        (bench.*addressingMode)(OP);
        Tester(0x10, 0x11, 0, 0);
        Tester(0xFF, 0x00, 1, 0);
        Tester(0x7F, 0x80, 0, 1);
    }
};

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestIncrementDecrement, DEC_ZeroPage) {
    Test_DEC<DEC>(&CpuTestBench::zeropage);
}

TEST_F(CpuTestIncrementDecrement, DEC_ZeroPageX) {
    Test_DEC<DEC>(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestIncrementDecrement, DEC_ZeroPageX_Wraparound) {
    Test_DEC<DEC>(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestIncrementDecrement, DEC_Absolute) {
    Test_DEC<DEC>(&CpuTestBench::absolute);
}

TEST_F(CpuTestIncrementDecrement, DEC_AbsoluteX) {
    Test_DEC<DEC>(&CpuTestBench::absolute_x);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestIncrementDecrement, DEX) {
    Test_DEC<DEX>(bench.implicit_target(cpu.X));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestIncrementDecrement, DEY) {
    Test_DEC<DEY>(bench.implicit_target(cpu.Y));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestIncrementDecrement, INC_ZeroPage) {
    Test_INC<INC>(&CpuTestBench::zeropage);
}

TEST_F(CpuTestIncrementDecrement, INC_ZeroPageX) {
    Test_INC<INC>(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestIncrementDecrement, INC_ZeroPageX_Wraparound) {
    Test_INC<INC>(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestIncrementDecrement, INC_Absolute) {
    Test_INC<INC>(&CpuTestBench::absolute);
}

TEST_F(CpuTestIncrementDecrement, INC_AbsoluteX) {
    Test_INC<INC>(&CpuTestBench::absolute_x);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestIncrementDecrement, INX) {
    Test_INC<INX>(bench.implicit_target(cpu.X));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestIncrementDecrement, INY) {
    Test_INC<INY>(bench.implicit_target(cpu.Y));
}
