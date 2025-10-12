#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

struct CpuTestStatusChange : public CpuBaseTest {
    template <InstructionSet_6502::OpName OP>
    void TesterFlag(Flag &f, Flag initial, Flag expected) {
        f = initial;
        bench.encode(OP, IMP).start();
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(expected, f);
    }
};

TEST_F(CpuTestStatusChange, CLC) {
    TesterFlag<CLC>(cpu->C, 1, 0);
}

TEST_F(CpuTestStatusChange, CLD) {
    TesterFlag<CLD>(cpu->D, 1, 0);
}

TEST_F(CpuTestStatusChange, CLI) {
    TesterFlag<CLI>(cpu->I, 1, 0);
}

TEST_F(CpuTestStatusChange, CLV) {
    TesterFlag<CLV>(cpu->V, 1, 0);
}

TEST_F(CpuTestStatusChange, SEC) {
    TesterFlag<SEC>(cpu->C, 0, 1);
}

TEST_F(CpuTestStatusChange, SED) {
    TesterFlag<SED>(cpu->D, 0, 1);
}

TEST_F(CpuTestStatusChange, SEI) {
    TesterFlag<SEI>(cpu->I, 0, 1);
}
