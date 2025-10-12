#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

struct CpuTestJumpCall : public CpuBaseTest {
};

TEST_F(CpuTestJumpCall, JMP_Absolute) {
    bench.encode(JMP, ABS)
        .dw(0x0120)
        .start();
    ExecuteOne();

    EXPECT_EQ(0x0120, cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
}

TEST_F(CpuTestJumpCall, JMP_Indirect) {
    bench.encode(JMP, IND)
        .dw(0x0120)
        .at(0x0120).dw(0x0200)
        .start();
    ExecuteOne();

    EXPECT_EQ(0x0200, cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
}

TEST_F(CpuTestJumpCall, JMP_Indirect_Bug) {
    bench.encode(JMP, IND)
        .dw(0x01FF)
        .at(0x01FF).db(0xF0)
        .at(0x0100).db(0x01)
        .start();
    ExecuteOne();

    EXPECT_EQ(0x01F0, cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
}

TEST_F(CpuTestJumpCall, JSR) {
    cpu->S = 0xF0;
    bench.encode(JSR, ABS)
        .dw(0x0120)
        .start();
    ExecuteOne();

    EXPECT_EQ(0x0120, cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
    EXPECT_EQ(0xEE, cpu->S);
    EXPECT_EQ(LO(bench.PC + 2), memory->GetByteAt(0x01EF));
    EXPECT_EQ(HI(bench.PC + 2), memory->GetByteAt(0x01F0));
}

TEST_F(CpuTestJumpCall, RTS) {
    cpu->S = 0xF0;
    bench.encode(RTS, IMP)
        .at(0x01F1).db(0x20)
        .at(0x01F2).db(0x01)
        .start();
    ExecuteOne();

    EXPECT_EQ(0x0121, cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
    EXPECT_EQ(0xF2, cpu->S);
}
