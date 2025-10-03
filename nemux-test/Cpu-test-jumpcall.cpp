#include "CpuBaseTest.h"

#include <vector>
#include <map>
#include <array>
#include <functional>

using namespace std;
using namespace Instructions;
using namespace Addressing;

struct CpuTestJumpCall : public CpuBaseTest {
};

TEST_F(CpuTestJumpCall, JMP_Absolute) {
    auto op = Opcode(JMP, Absolute, 3, 3);

    cpu.WriteByte(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(0x0120, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
}

TEST_F(CpuTestJumpCall, JMP_Indirect) {
    auto op = Opcode(JMP, Indirect, 3, 5);

    cpu.WriteByte(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.WriteWordAt(0x0120, 0x0200);

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(0x200, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
}

TEST_F(CpuTestJumpCall, JMP_Indirect_Bug) {
    auto op = Opcode(JMP, Indirect, 3, 5);

    cpu.WriteByte(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x01FF);
    cpu.WriteByte(0x01FF, 0xF0);
    cpu.WriteByte(0x0200, 0x02);
    cpu.WriteByte(0x0100, 0x01);

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(0x01F0, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
}

TEST_F(CpuTestJumpCall, JSR) {
    auto op = Opcode(JSR, Absolute, 3, 6);

    cpu.WriteByte(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.StackPage = 0x100;
    cpu.S = 0xF0;

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(0x0120, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0xEE, cpu.S);
    EXPECT_EQ(BASE_PC + 2, cpu.PullWord());
}

TEST_F(CpuTestJumpCall, RTS) {
    auto op = Opcode(RTS, Implicit, 1, 6);

    cpu.WriteByte(BASE_PC, 0xFF);
    cpu.StackPage = 0x100;
    cpu.S = 0xF0;
    cpu.PushWord(0x0120);

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(0x0121, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0xF0, cpu.S);
}
