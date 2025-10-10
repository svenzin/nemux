#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

struct CpuTestRegisterTransfer : public CpuBaseTest {
    template<InstructionSet_6502::OpName OP>
    void Test_Transfer(Byte& from, Byte& to, CpuTestBench::addressing_mode_setup addressingMode) {
        auto tester = [&] (Byte value, Flag expZ, Flag expN) {
            bench.start();

            from = value;
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu.PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu.Ticks);
            EXPECT_EQ(value, to);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        (bench.*addressingMode)(OP);
        tester(0x20, 0, 0);
        tester(0xA0, 0, 1);
        tester(0x00, 1, 0);
    }
};

TEST_F(CpuTestRegisterTransfer, TAX) {
    Test_Transfer<TAX>(cpu.A, cpu.X, &CpuTestBench::implicit);
}

TEST_F(CpuTestRegisterTransfer, TAY) {
    Test_Transfer<TAY>(cpu.A, cpu.Y, &CpuTestBench::implicit);
}

TEST_F(CpuTestRegisterTransfer, TXA) {
    Test_Transfer<TXA>(cpu.X, cpu.A, &CpuTestBench::implicit);
}

TEST_F(CpuTestRegisterTransfer, TYA) {
    Test_Transfer<TYA>(cpu.Y, cpu.A, &CpuTestBench::implicit);
}
