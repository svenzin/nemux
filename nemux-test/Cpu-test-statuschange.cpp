#include "CpuBaseTest.h"

#include <vector>
#include <map>
#include <array>
#include <functional>

using namespace std;
using namespace Instructions;
using namespace Addressing;

struct CpuTestStatusChange : public CpuBaseTest {
    void Test_ClearFlag(Instructions::Name inst, Flag &f) {
        f = 1;
        cpu.Execute(Opcode(inst, Implicit, 1, 2));//, {});

        EXPECT_EQ(BASE_PC + 1, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
        EXPECT_EQ(Flag{0}, f);
    }

    void Test_SetFlag(Instructions::Name inst, Flag &f) {
        f = 0;
        cpu.Execute(Opcode(inst, Implicit, 1, 2));//, {});

        EXPECT_EQ(BASE_PC + 1, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
        EXPECT_EQ(Flag{1}, f);
    }
};

TEST_F(CpuTestStatusChange, CLC) {
    Test_ClearFlag(CLC, cpu.C);
}

TEST_F(CpuTestStatusChange, CLD) {
    Test_ClearFlag(CLD, cpu.D);
}

TEST_F(CpuTestStatusChange, CLI) {
    Test_ClearFlag(CLI, cpu.I);
}

TEST_F(CpuTestStatusChange, CLV) {
    Test_ClearFlag(CLV, cpu.V);
}

TEST_F(CpuTestStatusChange, SEC) {
    Test_SetFlag(SEC, cpu.C);
}

TEST_F(CpuTestStatusChange, SED) {
    Test_SetFlag(SED, cpu.D);
}

TEST_F(CpuTestStatusChange, SEI) {
    Test_SetFlag(SEI, cpu.I);
}
