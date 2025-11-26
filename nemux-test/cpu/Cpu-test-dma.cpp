#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;
using enum BaseCpu::Bits;

struct CpuTestDMA : public CpuBaseTest {
    static constexpr Word TargetIRQ{ 0x0120 };
    static constexpr Word TargetNMI{ 0x0140 };
    static constexpr Word TargetRST{ 0x0160 };

    void Setup(auto op) {
        cpu->S = 0xF0;
        cpu->I = 0;
        bench.implicit(op)
             .at(cpu->VectorIRQ).dw(TargetIRQ)
             .at(cpu->VectorNMI).dw(TargetNMI)
             .at(cpu->VectorRST).dw(TargetRST)
             .start();
    }

    void ExpectInterrupt(auto target, Flag flagB, Word returnAddress) {
        EXPECT_EQ(target, cpu->PC);
        EXPECT_EQ(bench.Ticks + 7, cpu->GetTicks());
        EXPECT_EQ(0xED, cpu->S);
        EXPECT_EQ(1, cpu->I);
        Byte status{ memory->GetByteAt(0x01EE) };
        EXPECT_EQ(flagB, Bit<Brk>(status)); // Hardware interrupts push B flag clear
        EXPECT_EQ(1, Bit<Unu>(status)); // Unused is always 1
        EXPECT_EQ(LO(returnAddress), memory->GetByteAt(0x01EF));
        EXPECT_EQ(HI(returnAddress), memory->GetByteAt(0x01F0));
    }
};

//
// For a detailed description of the expected behavior, see:
//   https://www.nesdev.org/wiki/DMA
//

TEST_F(CpuTestDMA, NMI) {
    Setup(NOP);
    cpu->LineNMI = 1;
    ExecuteOne();
    ExpectInterrupt(TargetNMI, 0, bench.PC);
}
