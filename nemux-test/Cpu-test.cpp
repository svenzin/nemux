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
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTest() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

    function<void (Byte)> Setter(Byte & a) {
        return [&] (Byte value) { a = value; };
    }
    function<void (Byte)> Setter(Word a) {
        return [=] (Byte value) { cpu.Memory.SetByteAt(a, value); };
    }
    function<Byte ()> Getter(Byte & b) {
        return [&] () { return b; };
    }
    function<Byte ()> Getter(Word a) {
        return [=] () { return cpu.Memory.GetByteAt(a); };
    }
};


TEST_F(CpuTest, Opcode_Instruction) {
	using namespace Instructions;
    std::array<Instructions::Name, 0x100> opcodes {
/*           x0   x1   x2   x3   x4   x5   x6   x7   x8   x9   xA   xB   xC   xD   xE   xF */
/* 0x */    BRK, ORA, UNK, UNK, UNK, ORA, ASL, UNK, PHP, ORA, ASL, UNK, UNK, ORA, ASL, UNK,
/* 1x */    BPL, ORA, UNK, UNK, UNK, ORA, ASL, UNK, CLC, ORA, UNK, UNK, UNK, ORA, ASL, UNK,
/* 2x */    JSR, AND, UNK, UNK, BIT, AND, ROL, UNK, PLP, AND, ROL, UNK, BIT, AND, ROL, UNK,
/* 3x */    BMI, AND, UNK, UNK, UNK, AND, ROL, UNK, SEC, AND, UNK, UNK, UNK, AND, ROL, UNK,
/* 4x */    RTI, EOR, UNK, UNK, UNK, EOR, LSR, UNK, PHA, EOR, LSR, UNK, JMP, EOR, LSR, UNK,
/* 5x */    BVC, EOR, UNK, UNK, UNK, EOR, LSR, UNK, CLI, EOR, UNK, UNK, UNK, EOR, LSR, UNK,
/* 6x */    RTS, ADC, UNK, UNK, UNK, ADC, ROR, UNK, PLA, ADC, ROR, UNK, JMP, ADC, ROR, UNK,
/* 7x */    BVS, ADC, UNK, UNK, UNK, ADC, ROR, UNK, SEI, ADC, UNK, UNK, UNK, ADC, ROR, UNK,
/* 8x */    UNK, STA, UNK, UNK, STY, STA, STX, UNK, DEY, UNK, TXA, UNK, STY, STA, STX, UNK,
/* 9x */    BCC, STA, UNK, UNK, STY, STA, STX, UNK, TYA, STA, TXS, UNK, UNK, STA, UNK, UNK,
/* Ax */    LDY, LDA, LDX, UNK, LDY, LDA, LDX, UNK, TAY, LDA, TAX, UNK, LDY, LDA, LDX, UNK,
/* Bx */    BCS, LDA, UNK, UNK, LDY, LDA, LDX, UNK, CLV, LDA, TSX, UNK, LDY, LDA, LDX, UNK,
/* Cx */    CPY, CMP, UNK, UNK, CPY, CMP, DEC, UNK, INY, CMP, DEX, UNK, CPY, CMP, DEC, UNK,
/* Dx */    BNE, CMP, UNK, UNK, UNK, CMP, DEC, UNK, CLD, CMP, UNK, UNK, UNK, CMP, DEC, UNK,
/* Ex */    CPX, SBC, UNK, UNK, CPX, SBC, INC, UNK, INX, SBC, NOP, UNK, CPX, SBC, INC, UNK,
/* Fx */    BEQ, SBC, UNK, UNK, UNK, SBC, INC, UNK, SED, SBC, UNK, UNK, UNK, SBC, INC, UNK,
    };

	const Cpu cpu("6502");
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
		/* 0x */    IMP, IDX, UNK, UNK, UNK, ZPG, ZPG, UNK, IMP, IMM, ACC, UNK, UNK, ABS, ABS, UNK,
		/* 1x */    REL, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
		/* 2x */    ABS, IDX, UNK, UNK, ZPG, ZPG, ZPG, UNK, IMP, IMM, ACC, UNK, ABS, ABS, ABS, UNK,
		/* 3x */    REL, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
		/* 4x */    IMP, IDX, UNK, UNK, UNK, ZPG, ZPG, UNK, IMP, IMM, ACC, UNK, ABS, ABS, ABS, UNK,
		/* 5x */    REL, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
		/* 6x */    IMP, IDX, UNK, UNK, UNK, ZPG, ZPG, UNK, IMP, IMM, ACC, UNK, IND, ABS, ABS, UNK,
		/* 7x */    REL, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
		/* 8x */    UNK, IDX, UNK, UNK, ZPG, ZPG, ZPG, UNK, IMP, UNK, IMP, UNK, ABS, ABS, ABS, UNK,
		/* 9x */    REL, IDY, UNK, UNK, ZPX, ZPX, ZPY, UNK, IMP, ABY, IMP, UNK, UNK, ABX, UNK, UNK,
		/* Ax */    IMM, IDX, IMM, UNK, ZPG, ZPG, ZPG, UNK, IMP, IMM, IMP, UNK, ABS, ABS, ABS, UNK,
		/* Bx */    REL, IDY, UNK, UNK, ZPX, ZPX, ZPY, UNK, IMP, ABY, IMP, UNK, ABX, ABX, ABY, UNK,
		/* Cx */    IMM, IDX, UNK, UNK, ZPG, ZPG, ZPG, UNK, IMP, IMM, IMP, UNK, ABS, ABS, ABS, UNK,
		/* Dx */    REL, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
		/* Ex */    IMM, IDX, UNK, UNK, ZPG, ZPG, ZPG, UNK, IMP, IMM, IMP, UNK, ABS, ABS, ABS, UNK,
		/* Fx */    REL, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
    };

	const Cpu cpu("6502");
	for (int i = 0; i < opcodes.size(); ++i) {
		const auto op = cpu.Decode(i);
		EXPECT_EQ(opcodes[i], op.Addressing) << "Instruction 0x" << std::hex << i;
	}
}
