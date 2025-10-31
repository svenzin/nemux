#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;
using enum BaseCpu::Bits;

struct CpuTestInterrupt : public CpuBaseTest {
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
// Tests mostly centered around interrupt behavior
//
// For a step-by-step description
//   https://www.nesdev.org/6502_cpu.txt
//   https://www.pagetable.com/?p=410
//   https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Hijacking
//   https://www.nesdev.org/wiki/Visual6502wiki/6502_Timing_of_Interrupt_Handling
//

TEST_F(CpuTestInterrupt, NMI) {
    Setup(NOP);
    cpu->LineNMI = 1;
    ExecuteOne();
    ExpectInterrupt(TargetNMI, 0, bench.PC);
}

TEST_F(CpuTestInterrupt, NMI_FlagI) {
    // NMI is non-maskable so it's a repeat of the NMI test
    Setup(NOP);
    cpu->I = 1;
    cpu->LineNMI = 1;
    ExecuteOne();
    ExpectInterrupt(TargetNMI, 0, bench.PC);
}

TEST_F(CpuTestInterrupt, IRQ) {
    Setup(NOP);
    cpu->LineIRQ = 1;
    ExecuteOne();
    ExpectInterrupt(TargetIRQ, 0, bench.PC);
}

TEST_F(CpuTestInterrupt, IRQ_FlagI) {
    Setup(NOP);
    cpu->I = 1;
    cpu->LineIRQ = 1;
    ExecuteOne();

    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
    EXPECT_EQ(0xF0, cpu->S);
}

TEST_F(CpuTestInterrupt, BRK) {
    // BRK will sequentially push HI(PC), LO(PC) and Status
    // Stack pointer starts at 0xF0, ends at 0xED
    //  ED      EE      EF      F0      F1
    //  x       Status  LO(PC)  HI(PC)  x
    Setup(BRK);
    ExecuteOne();
    ExpectInterrupt(TargetIRQ, 1, bench.expected_PC());
}

TEST_F(CpuTestInterrupt, BRK_FlagI) {
    // BRK seems to be non-maskable
    // see https://www.nesdev.org/wiki/Status_flags#I
    //     https://www.nesdev.org/wiki/Instruction_reference#BRK
    Setup(BRK);
    cpu->I = 1;
    ExecuteOne();
    ExpectInterrupt(TargetIRQ, 1, bench.expected_PC());
}

TEST_F(CpuTestInterrupt, IRQ_NMI_Priority) {
    Setup(NOP);
    cpu->LineIRQ = 1;
    cpu->LineNMI = 1;
    ExecuteOne();
    ExpectInterrupt(TargetNMI, 0, bench.PC);
}

TEST_F(CpuTestInterrupt, NMI_RES_Priority) { FAIL(); }
TEST_F(CpuTestInterrupt, IRQ_Hijacking_BRK) { FAIL(); }
TEST_F(CpuTestInterrupt, NMI_Hijacking_BRK) { FAIL(); }
TEST_F(CpuTestInterrupt, NMI_Hijacking_IRQ) { FAIL(); }
TEST_F(CpuTestInterrupt, RES_Hijacking_BRK) { FAIL(); }
TEST_F(CpuTestInterrupt, RES_Hijacking_IRQ) { FAIL(); }
TEST_F(CpuTestInterrupt, RES_Hijacking_NMI) { FAIL(); }
TEST_F(CpuTestInterrupt, LostBRK) { FAIL(); }
TEST_F(CpuTestInterrupt, LostNMI) { FAIL(); }

//
// extra test scenarios
//
// NMI has priority over IRQ
// RES has priority over NMI
// IRQ steals BRK
// NMI steals BRK
// NMI steals IRQ
// Lost NMI
// Lost BRK when hijacked by an IRQ
//
// Timing limits for triggering the interrupt on the next instruction
// Timing limits for triggering the interrupt stealing
// One instruction from the interrupt handler will get executed due to them not polling interrupt lines
// Interrupt can trigger immediately after RTI due to RTI clearing the I flag before polling
// Interrupt does not trigger immediately after CLI, SEI, PLP because the I flag is modifed after polling
// Branching is complicated
// RES hijacking
// Weird behaviors when asserting RES for very short periods of time
//
// see https://www.nesdev.org/wiki/CPU_interrupts
//     https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Hijacking
//     https://www.nesdev.org/wiki/Visual6502wiki/6502_Timing_States#BRK_Instruction_Timing_States
//     https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Recognition_Stages_and_Tolerances
//     https://www.nesdev.org/wiki/Visual6502wiki/6502_Timing_of_Interrupt_Handling
//
