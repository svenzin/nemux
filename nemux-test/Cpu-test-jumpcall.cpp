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

    EXPECT_EQ(0x0120, cpu.PC);
    EXPECT_EQ(bench.Ticks + bench.op.Cycles, cpu.GetTicks());
}

TEST_F(CpuTestJumpCall, JMP_Indirect) {
    bench.encode(JMP, IND)
        .dw(0x0120)
        .at(0x0120).dw(0x0200)
        .start();
    ExecuteOne();

    EXPECT_EQ(0x0200, cpu.PC);
    EXPECT_EQ(bench.Ticks + bench.op.Cycles, cpu.GetTicks());
}

TEST_F(CpuTestJumpCall, JMP_Indirect_Bug) {
    bench.encode(JMP, IND)
        .dw(0x01FF)
        .at(0x01FF).db(0xF0)
        .at(0x0100).db(0x01)
        .start();
    ExecuteOne();

    EXPECT_EQ(0x01F0, cpu.PC);
    EXPECT_EQ(bench.Ticks + bench.op.Cycles, cpu.GetTicks());
}

TEST_F(CpuTestJumpCall, JSR) {
    cpu.S = 0xF0;
    bench.encode(JSR, ABS)
        .dw(0x0120)
        .start();
    ExecuteOne();

    EXPECT_EQ(0x0120, cpu.PC);
    EXPECT_EQ(bench.Ticks + bench.op.Cycles, cpu.GetTicks());
    EXPECT_EQ(bench.S - 2, cpu.S);
    EXPECT_EQ(LO(bench.PC + 2), memory.GetByteAt(cpu.StackPage + bench.S - 1));
    EXPECT_EQ(HI(bench.PC + 2), memory.GetByteAt(cpu.StackPage + bench.S));
}

TEST_F(CpuTestJumpCall, RTS) {
    cpu.S = 0xF0;
    bench.encode(RTS, IMP)
        .at(cpu.StackPage + cpu.S + 1).db(0x20)
        .at(cpu.StackPage + cpu.S + 2).db(0x01)
        .start();
    ExecuteOne();

    EXPECT_EQ(0x0121, cpu.PC);
    EXPECT_EQ(bench.Ticks + bench.op.Cycles, cpu.GetTicks());
    EXPECT_EQ(bench.S + 2, cpu.S);
}
