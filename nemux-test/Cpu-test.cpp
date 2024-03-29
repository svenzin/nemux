/*
 * Cpu-test.cpp
 *
 *  Created on: 19 Jun 2013
 *      Author: scorder
 */

#include "gtest/gtest.h"

#include "Cpu.h"

#include <vector>
#include <map>
#include <array>
#include <functional>

using namespace std;

class CpuTest : public ::testing::Test {
public:
    CpuTest() : cpu("6502") {
    }

    Cpu cpu;
};


TEST_F(CpuTest, Opcode_Instruction) {
    using namespace Instructions;
    std::array<Instructions::Name, 0x100> opcodes{
        /*           x0   x1   x2   x3   x4   x5   x6   x7   x8   x9   xA   xB   xC   xD   xE   xF */
        /* 0x */    BRK, ORA,uSTP,uSLO,uNOP, ORA, ASL,uSLO, PHP, ORA, ASL,uANC,uNOP, ORA, ASL,uSLO,
        /* 1x */    BPL, ORA,uSTP,uSLO,uNOP, ORA, ASL,uSLO, CLC, ORA,uNOP,uSLO,uNOP, ORA, ASL,uSLO,
        /* 2x */    JSR, AND,uSTP,uRLA, BIT, AND, ROL,uRLA, PLP, AND, ROL,uANC, BIT, AND, ROL,uRLA,
        /* 3x */    BMI, AND,uSTP,uRLA,uNOP, AND, ROL,uRLA, SEC, AND,uNOP,uRLA,uNOP, AND, ROL,uRLA,
        /* 4x */    RTI, EOR,uSTP,uSRE,uNOP, EOR, LSR,uSRE, PHA, EOR, LSR,uALR, JMP, EOR, LSR,uSRE,
        /* 5x */    BVC, EOR,uSTP,uSRE,uNOP, EOR, LSR,uSRE, CLI, EOR,uNOP,uSRE,uNOP, EOR, LSR,uSRE,
        /* 6x */    RTS, ADC,uSTP,uRRA,uNOP, ADC, ROR,uRRA, PLA, ADC, ROR,uARR, JMP, ADC, ROR,uRRA,
        /* 7x */    BVS, ADC,uSTP,uRRA,uNOP, ADC, ROR,uRRA, SEI, ADC,uNOP,uRRA,uNOP, ADC, ROR,uRRA,
        /* 8x */   uNOP, STA,uNOP,uSAX, STY, STA, STX,uSAX, DEY,uNOP, TXA,uXAA, STY, STA, STX,uSAX,
        /* 9x */    BCC, STA,uSTP,uAHX, STY, STA, STX,uSAX, TYA, STA, TXS,uTAS,uSHY, STA,uSHX,uAHX,
        /* Ax */    LDY, LDA, LDX,uLAX, LDY, LDA, LDX,uLAX, TAY, LDA, TAX,uLAX, LDY, LDA, LDX,uLAX,
        /* Bx */    BCS, LDA,uSTP,uLAX, LDY, LDA, LDX,uLAX, CLV, LDA, TSX,uLAS, LDY, LDA, LDX,uLAX,
        /* Cx */    CPY, CMP,uNOP,uDCP, CPY, CMP, DEC,uDCP, INY, CMP, DEX,uAXS, CPY, CMP, DEC,uDCP,
        /* Dx */    BNE, CMP,uSTP,uDCP,uNOP, CMP, DEC,uDCP, CLD, CMP,uNOP,uDCP,uNOP, CMP, DEC,uDCP,
        /* Ex */    CPX, SBC,uNOP,uISC, CPX, SBC, INC,uISC, INX, SBC, NOP,uSBC, CPX, SBC, INC,uISC,
        /* Fx */    BEQ, SBC,uSTP,uISC,uNOP, SBC, INC,uISC, SED, SBC,uNOP,uISC,uNOP, SBC, INC,uISC,
    };

    for (int i = 0; i < opcodes.size(); ++i) {
        const auto op = cpu.Decode(i);
        EXPECT_EQ(opcodes[i], op.Instruction) << "Instruction 0x" << std::hex << i;
    }
}

TEST_F(CpuTest, Opcode_Addressing) {
    const auto IMP = Addressing::Implicit;
    const auto ACC = Addressing::Accumulator;
    const auto IMM = Addressing::Immediate;
    const auto ZPG = Addressing::ZeroPage;
    const auto ZPX = Addressing::ZeroPageX;
    const auto ZPY = Addressing::ZeroPageY;
    const auto REL = Addressing::Relative;
    const auto ABS = Addressing::Absolute;
    const auto ABX = Addressing::AbsoluteX;
    const auto ABY = Addressing::AbsoluteY;
    const auto IND = Addressing::Indirect;
    const auto IDX = Addressing::IndexedIndirect;
    const auto IDY = Addressing::IndirectIndexed;
    const auto UNK = Addressing::Unknown;

    std::array<Addressing::Type, 0x100> opcodes {
		/*           x0   x1   x2   x3   x4   x5   x6   x7   x8   x9   xA   xB   xC   xD   xE   xF */
		/* 0x */    IMP, IDX, IMP, IDX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMM, ABS, ABS, ABS, ABS,
		/* 1x */    REL, IDY, IMP, IDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
		/* 2x */    ABS, IDX, IMP, IDX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMM, ABS, ABS, ABS, ABS,
		/* 3x */    REL, IDY, IMP, IDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
		/* 4x */    IMP, IDX, IMP, IDX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMM, ABS, ABS, ABS, ABS,
		/* 5x */    REL, IDY, IMP, IDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
		/* 6x */    IMP, IDX, IMP, IDX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMM, IND, ABS, ABS, ABS,
		/* 7x */    REL, IDY, IMP, IDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
		/* 8x */    IMM, IDX, IMM, IDX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
		/* 9x */    REL, IDY, IMP, IDY, ZPX, ZPX, ZPY, ZPY, IMP, ABY, IMP, ABY, ABX, ABX, ABY, ABY,
		/* Ax */    IMM, IDX, IMM, IDX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
		/* Bx */    REL, IDY, IMP, IDY, ZPX, ZPX, ZPY, ZPY, IMP, ABY, IMP, ABY, ABX, ABX, ABY, ABY,
		/* Cx */    IMM, IDX, IMM, IDX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
		/* Dx */    REL, IDY, IMP, IDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
		/* Ex */    IMM, IDX, IMM, IDX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMM, ABS, ABS, ABS, ABS,
		/* Fx */    REL, IDY, IMP, IDY, ZPX, ZPX, ZPX, ZPX, IMP, ABY, IMP, ABY, ABX, ABX, ABX, ABX,
    };

	for (int i = 0; i < opcodes.size(); ++i) {
		const auto op = cpu.Decode(i);
		EXPECT_EQ(opcodes[i], op.Addressing) << "Instruction 0x" << std::hex << i;
	}
}

TEST_F(CpuTest, PowerUpState) {
    EXPECT_TRUE(cpu.IsAlive);

    EXPECT_EQ(0, cpu.A);
    EXPECT_EQ(0, cpu.X);
    EXPECT_EQ(0, cpu.Y);
    EXPECT_EQ(0xFD, cpu.SP);
    EXPECT_EQ(0x34, cpu.GetStatus());

    EXPECT_EQ(0, cpu.Ticks);
    EXPECT_EQ(InterruptType::None, cpu.PendingInterrupt);
}

TEST_F(CpuTest, InterruptPriority) {
    cpu.PendingInterrupt = InterruptType::None;
    cpu.TriggerIRQ();
    EXPECT_EQ(InterruptType::Irq, cpu.PendingInterrupt);
    cpu.TriggerNMI();
    EXPECT_EQ(InterruptType::Nmi, cpu.PendingInterrupt);
    cpu.TriggerIRQ();
    EXPECT_EQ(InterruptType::Nmi, cpu.PendingInterrupt);
    cpu.TriggerReset();
    EXPECT_EQ(InterruptType::Rst, cpu.PendingInterrupt);
    cpu.TriggerNMI();
    EXPECT_EQ(InterruptType::Rst, cpu.PendingInterrupt);
}

TEST_F(CpuTest, Ticking) {
    EXPECT_EQ(0, cpu.Ticks);

    MemoryBlock<0x0400> mem;
    mem.SetByteAt(0x0200, 0xA9); // LDA Immediate
    mem.SetByteAt(0x0201, 0x01); // LDA 1 Tick=2 A=1
    mem.SetByteAt(0x0000, 0x02);
    mem.SetByteAt(0x0202, 0x65); // ADC ZeroPage
    mem.SetByteAt(0x0203, 0x00); // ADC $00 ($0000=2) Tick=5 A=3
    mem.SetByteAt(0x0204, 0xE6); // INC ZeroPage
    mem.SetByteAt(0x0205, 0x00); // INC $00 Tick=10 $0000=3

    cpu.Map = &mem;
    cpu.PC = 0x0200;

    EXPECT_EQ(0, cpu.CurrentTick);
    EXPECT_EQ(0, cpu.Ticks);
    EXPECT_EQ(0, cpu.A);

    // LDA #1
    cpu.Tick();
    EXPECT_EQ(1, cpu.CurrentTick);
    EXPECT_EQ(2, cpu.Ticks);

    cpu.Tick();
    EXPECT_EQ(2, cpu.CurrentTick);
    EXPECT_EQ(2, cpu.Ticks);
    EXPECT_EQ(1, cpu.A);

    // ADC $00
    cpu.Tick();
    EXPECT_EQ(3, cpu.CurrentTick);
    EXPECT_EQ(5, cpu.Ticks);

    cpu.Tick();
    EXPECT_EQ(4, cpu.CurrentTick);
    EXPECT_EQ(5, cpu.Ticks);

    cpu.Tick();
    EXPECT_EQ(5, cpu.CurrentTick);
    EXPECT_EQ(5, cpu.Ticks);
    EXPECT_EQ(3, cpu.A);
    EXPECT_EQ(2, mem.GetByteAt(0x0000));

    // INC $00
    cpu.Tick();
    EXPECT_EQ(6, cpu.CurrentTick);
    EXPECT_EQ(10, cpu.Ticks);

    cpu.Tick();
    cpu.Tick();
    cpu.Tick();
    cpu.Tick();
    EXPECT_EQ(10, cpu.CurrentTick);
    EXPECT_EQ(10, cpu.Ticks);
    EXPECT_EQ(3, mem.GetByteAt(0x0000));
}

TEST_F(CpuTest, TickingWithInterrupt) {
    EXPECT_EQ(0, cpu.Ticks);

    MemoryBlock<0x0400> mem;
    mem.SetByteAt(0x0200, 0x58); // CLI Tick=2 I=0
    mem.SetByteAt(0x0201, 0xA9); // LDA Immediate
    mem.SetByteAt(0x0202, 0x01); // LDA 1 Tick=4 A=1
    mem.SetByteAt(0x0000, 0x02);
    mem.SetByteAt(0x0203, 0x65); // ADC ZeroPage
    mem.SetByteAt(0x0204, 0x00); // ADC $00 ($0000=2) Tick=7 A=3
    mem.SetByteAt(0x0205, 0xE6); // INC ZeroPage
    mem.SetByteAt(0x0206, 0x00); // INC $00 Tick=12 $0000=3

    cpu.Map = &mem;
    cpu.PC = 0x0200;
    cpu.VectorIRQ = 0x03FE;
    cpu.WriteWordAt(0x03FE, 0x0080);
    
    EXPECT_EQ(0, cpu.CurrentTick);
    EXPECT_EQ(0, cpu.Ticks);
    EXPECT_EQ(0, cpu.A);

    // CLI
    cpu.Tick();
    EXPECT_EQ(1, cpu.CurrentTick);
    EXPECT_EQ(2, cpu.Ticks);
    
    cpu.Tick();
    EXPECT_EQ(2, cpu.CurrentTick);
    EXPECT_EQ(2, cpu.Ticks);
    EXPECT_EQ(0, cpu.I);

    // LDA #1
    cpu.Tick();
    EXPECT_EQ(3, cpu.CurrentTick);
    EXPECT_EQ(4, cpu.Ticks);

    cpu.TriggerIRQ();

    cpu.Tick();
    EXPECT_EQ(4, cpu.CurrentTick);
    EXPECT_EQ(4, cpu.Ticks);
    EXPECT_EQ(1, cpu.A);

    // Interrupt
    for (int i = 0; i < cpu.InterruptCycles; ++i) cpu.Tick();
    EXPECT_EQ(4 + cpu.InterruptCycles, cpu.CurrentTick);
    EXPECT_EQ(4 + cpu.InterruptCycles, cpu.Ticks);
    EXPECT_EQ(1, cpu.A);
    EXPECT_EQ(0x0080, cpu.PC);
}

TEST_F(CpuTest, TickingWithInhibitedInterrupt) {
    EXPECT_EQ(0, cpu.Ticks);

    MemoryBlock<0x0400> mem;
    mem.SetByteAt(0x0200, 0x78); // SEI Tick=2 I=1
    mem.SetByteAt(0x0201, 0xA9); // LDA Immediate
    mem.SetByteAt(0x0202, 0x01); // LDA #1 Tick=4 A=1

    cpu.Map = &mem;
    cpu.PC = 0x0200;
    cpu.VectorIRQ = 0x03FE;
    cpu.WriteWordAt(0x03FE, 0x0080);
    
    // SEI
    cpu.Tick();
    cpu.TriggerIRQ();
    cpu.Tick();

    // LDA #1
    cpu.Tick();
    EXPECT_EQ(3, cpu.CurrentTick);
    EXPECT_EQ(4, cpu.Ticks);
    EXPECT_EQ(1, cpu.A);
    EXPECT_EQ(0x0203, cpu.PC);
}

TEST_F(CpuTest, OAMDMA) {
    MemoryBlock<0x0400> mem;
    for (Word i = 0; i < 0x0400; i += 4) {
        mem.SetByteAt(i + 0, 0xDE);
        mem.SetByteAt(i + 1, 0xAD);
        mem.SetByteAt(i + 2, 0xBE);
        mem.SetByteAt(i + 3, 0xEF);
    }
    for (Word i = 0; i < 0x0100; ++i) {
        mem.SetByteAt(0x0100 + i, i & WORD_LO_MASK);
    }

    std::array<Byte, 0x0100> page;
    cpu.Map = &mem;
    cpu.DMA(1, page, 0x00);

    EXPECT_EQ(513, cpu.Ticks);
    for (Word i = 0; i < 0x0100; ++i) {
        EXPECT_EQ(i, page[i]);
    }
}

TEST_F(CpuTest, OAMDMA_OnOddCycle) {
    MemoryBlock<0x0400> mem;
    for (Word i = 0; i < 0x0400; i += 4) {
        mem.SetByteAt(i + 0, 0xDE);
        mem.SetByteAt(i + 1, 0xAD);
        mem.SetByteAt(i + 2, 0xBE);
        mem.SetByteAt(i + 3, 0xEF);
    }
    for (Word i = 0; i < 0x0100; ++i) {
        mem.SetByteAt(0x0100 + i, i & WORD_LO_MASK);
    }

    std::array<Byte, 0x0100> page;
    cpu.Map = &mem;
    cpu.Ticks = cpu.CurrentTick = 1;
    cpu.DMA(1, page, 0x00);

    EXPECT_EQ(1 + 514, cpu.Ticks);
    for (Word i = 0; i < 0x0100; ++i) {
        EXPECT_EQ(i, page[i]);
    }
}

TEST_F(CpuTest, OAMDMA_NonZeroOffset) {
    MemoryBlock<0x0400> mem;
    for (Word i = 0; i < 0x0400; i += 4) {
        mem.SetByteAt(i + 0, 0xDE);
        mem.SetByteAt(i + 1, 0xAD);
        mem.SetByteAt(i + 2, 0xBE);
        mem.SetByteAt(i + 3, 0xEF);
    }
    for (Word i = 0; i < 0x0100; ++i) {
        mem.SetByteAt(0x0100 + i, i & WORD_LO_MASK);
    }

    std::array<Byte, 0x0100> page;
    cpu.Map = &mem;
    cpu.DMA(1, page, 0x20);

    EXPECT_EQ(513, cpu.Ticks);
    for (Word i = 0; i < 0xE0; ++i) {
        EXPECT_EQ(i, page[i + 0x20]);
    }
    for (Word i = 0xE0; i < 0x0100; ++i) {
        EXPECT_EQ(i, page[i - 0xE0]);
    }
}

