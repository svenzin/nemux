#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;
using enum BaseCpu::Bits;

struct CpuTestSystem : public CpuBaseTest {
    static const Word VECTOR_RST = 0xFFFC;
    static const Word VECTOR_IRQ = 0xFFFE;
    static const Word VECTOR_NMI = 0xFFFA;
};

TEST_F(CpuTestSystem, BRK) {
    cpu->SetStatusByte(0x00);
    cpu->S = 0xF0;

    bench.implicit(BRK)
         .at(cpu->VectorIRQ).dw(0x0120)
         .start();

    // BRK will sequentially push HI(PC), LO(PC) and Status
    // Stack pointer starts at 0xF0, ends at 0xED
    //  ED      EE      EF      F0      F1
    //  x       Status  LO(PC)  HI(PC)  x
    ExecuteOne();

    EXPECT_EQ(0x0120, cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks()); // Interrupt => 7 cycles

    Byte status{ memory->GetByteAt(0x01EE) };
    Word expPC{ bench.expected_PC() };
    EXPECT_EQ(1, cpu->I); // BRK sets the I flag
    EXPECT_EQ(0xED, cpu->S);
    EXPECT_TRUE(IsBitSet<Brk>(status)); // BRK pushes B flag set, Unused is always 1
    EXPECT_TRUE(IsBitSet<Unu>(status)); // Unused is always 1
    EXPECT_EQ(LO(expPC), memory->GetByteAt(0x01EF));
    EXPECT_EQ(HI(expPC), memory->GetByteAt(0x01F0));
}

TEST_F(CpuTestSystem, BRK_FlagI) {
    cpu->SetStatusByte(0x00);
    cpu->S = 0xF0;
    cpu->I = 1;

    bench.implicit(BRK)
         .at(cpu->VectorIRQ).dw(0x0120)
         .start();

    ExecuteOne();

    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
}

TEST_F(CpuTestSystem, NOP) {
    bench.implicit(NOP)
         .start();
    ExecuteOne();
    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
}

TEST_F(CpuTestSystem, RTI) {
    auto tester = [&] (Byte status, Flag expN, Flag expV, Flag expD, Flag expI, Flag expZ, Flag expC) {
        cpu->S = 0xF0;
        bench.implicit(RTI)
            .at(0x01F1).db(status)
            .at(0x01F2).db(0x20)
            .at(0x01F3).db(0x01)
            .start();

        // RTI will sequentially pop Status, LO(PC) and HI(PC)
        // Stack pointer starts at 0xF0, ends at 0xF3
        //  F0      F1      F2      F3      F4
        //  x       Status  LO(PC)  HI(PC)  x
        ExecuteOne();

        EXPECT_EQ(0x0120, cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(0xF3, cpu->S);
        EXPECT_EQ(expN, cpu->N);
        EXPECT_EQ(expV, cpu->V);
        EXPECT_EQ(expD, cpu->D);
        EXPECT_EQ(expI, cpu->I);
        EXPECT_EQ(expZ, cpu->Z);
        EXPECT_EQ(expC, cpu->C);
    };

    tester(0xFF, 1, 1, 1, 1, 1, 1);
    tester(0x00, 0, 0, 0, 0, 0, 0);
}

TEST_F(CpuTestSystem, Reset) {
    cpu->S = 0xF0;
    cpu->SetStatusByte(0xFF);
    bench.implicit(NOP)
        .at(cpu->VectorRST).dw(0x0120)
        .start();
    
    cpu->Reset();
    ExecuteOne();

    EXPECT_EQ(0x0120, cpu->PC);
    EXPECT_EQ(bench.Ticks + 7, cpu->GetTicks());
    EXPECT_EQ(0xED, cpu->S);
    EXPECT_EQ(1, cpu->I);
}

TEST_F(CpuTestSystem, NMI) {
    FAIL();
    // cpu->S = 0xF0;
    // cpu->SetStatusByte(0xFF);
    // bench.implicit(NOP)
    //     .at(cpu->VectorNMI).dw(0x0120)
    //     .start();
    
    // cpu->TriggerNMI();
    // ExecuteOne();

    // EXPECT_EQ(0x0120, cpu->PC);
    // EXPECT_EQ(bench.Ticks + 7, cpu->GetTicks());
    // EXPECT_EQ(0xED, cpu->S);
    // EXPECT_EQ(1, cpu->I);
    // Byte status{ memory->GetByteAt(0x01EE) };
    // EXPECT_EQ(0, Bit<Brk>(status)); // Hardware interrupts push B flag clear
    // EXPECT_EQ(1, Bit<Unu>(status)); // Unused is always 1
    // EXPECT_EQ(LO(bench.PC), memory->GetByteAt(0x01EF));
    // EXPECT_EQ(HI(bench.PC), memory->GetByteAt(0x01F0));
}

TEST_F(CpuTestSystem, IRQ) {
    FAIL();
    // cpu->S = 0xF0;
    // cpu->SetStatusByte(0xFF);
    // bench.implicit(NOP)
    //     .at(cpu->VectorIRQ).dw(0x0120)
    //     .start();
    
    // cpu->TriggerIRQ();
    // ExecuteOne();

    // EXPECT_EQ(0x0120, cpu->PC);
    // EXPECT_EQ(bench.Ticks + 7, cpu->GetTicks());
    // EXPECT_EQ(0xED, cpu->S);
    // EXPECT_EQ(1, cpu->I);
    // Byte status{ memory->GetByteAt(0x01EE) };
    // EXPECT_EQ(0, Bit<Brk>(status)); // Hardware interrupts push B flag clear
    // EXPECT_EQ(1, Bit<Unu>(status)); // Unused is always 1
    // EXPECT_EQ(LO(bench.PC), memory->GetByteAt(0x01EF));
    // EXPECT_EQ(HI(bench.PC), memory->GetByteAt(0x01F0));
}
