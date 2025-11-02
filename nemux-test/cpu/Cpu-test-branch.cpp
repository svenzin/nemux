#include "CpuBaseTest.h"

using OpName = InstructionSet_6502::OpName;
using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

static constexpr size_t OFFSET_FROM_PREFETCH_NEXT{ 0 };

struct CpuTestBranch : public CpuBaseTest {
    void Test_Branch(Byte& flag, Flag success, Flag failure, OpName opname) {
        auto tester = [&] (Word pc, Byte offset, Flag c, Word expPC, int extra) {
            bench.origin(pc)
                .encode(opname, REL)
                .db(offset)
                .start();
            
            flag = c;
            ExecuteOne();

            EXPECT_EQ(expPC, cpu->PC);
            EXPECT_EQ(
                bench.expected_ticks(extra + OFFSET_FROM_PREFETCH_NEXT),
                cpu->GetTicks()
            );
        };

        tester(0x0080, 0x20, failure, 0x0080 + 2, 0); // Fail
        tester(0x0080, 0x20, success, 0x00A0 + 2, 1); // Success, positive offset
        tester(0x0080, 0xE0, success, 0x0060 + 2, 1); // Success, negative offset
        tester(0x00F0, 0x20, success, 0x0110 + 2, 2); // Success, positive offset, crossing page
        tester(0x00F0, 0x0F, success, 0x00FF + 2, 2); // Success, positive offset, crossing page on PC+2
        tester(0x0110, 0xE0, success, 0x00F0 + 2, 2); // Success, negative offset, crossing page
        tester(0x0110, 0xEF, success, 0x00FF + 2, 1); // Success, negative offset, not crossing page on PC+2
    }
};

TEST_F(CpuTestBranch, BCC) {
    Test_Branch(cpu->C, 0, 1, BCC);
}

TEST_F(CpuTestBranch, BCS) {
    Test_Branch(cpu->C, 1, 0, BCS);
}

TEST_F(CpuTestBranch, BEQ) {
    Test_Branch(cpu->Z, 1, 0, BEQ);
}

TEST_F(CpuTestBranch, BMI) {
    Test_Branch(cpu->N, 1, 0, BMI);
}

TEST_F(CpuTestBranch, BNE) {
    Test_Branch(cpu->Z, 0, 1, BNE);
}

TEST_F(CpuTestBranch, BPL) {
    Test_Branch(cpu->N, 0, 1, BPL);
}

TEST_F(CpuTestBranch, BVC) {
    Test_Branch(cpu->V, 0, 1, BVC);
}

TEST_F(CpuTestBranch, BVS) {
    Test_Branch(cpu->V, 1, 0, BVS);
}
