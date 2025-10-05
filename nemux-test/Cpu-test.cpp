#include "CpuBaseTest.h"

// #include <vector>
// #include <map>
// #include <array>
// #include <functional>

// using namespace std;

struct CpuTest : public CpuBaseTest {
};

TEST_F(CpuTest, OpcodeDecoding) {
    using is6502 = InstructionSet_6502;
    using enum is6502::OpName;
    using enum is6502::AddressingMode;

    size_t counter{ 0 };
    auto Check = [&counter](Byte opcode, is6502::Instruction instruction) {
        const auto decoded{ is6502::Decode(opcode) };
        EXPECT_EQ(instruction, decoded);
        ++counter;
    };
    
    Check(0x00, { BRK, IMP, 1, 7 }); // 1 byte is read, but return from interrupt skips the next byte, so maybe 2 bytes long
    Check(0x01, { ORA, IDX, 2, 6 });
    Check(0x02, { uSTP, IMP, 1, 2 }); // unsure
    Check(0x03, { uSLO, IDX, 2, 8 });
    Check(0x04, { uNOP, ZPG, 2, 3 });
    Check(0x05, { ORA, ZPG, 2, 3 });
    Check(0x06, { ASL, ZPG, 2, 5 });
    Check(0x07, { uSLO, ZPG, 2, 5 });
    Check(0x08, { PHP, IMP, 1, 3 });
    Check(0x09, { ORA, IMM, 2, 2 });
    Check(0x0A, { ASL, ACC, 1, 2 });
    Check(0x0B, { uANC, IMM, 2, 2 });
    Check(0x0C, { uNOP, ABS, 3, 4 });
    Check(0x0D, { ORA, ABS, 3, 4 });
    Check(0x0E, { ASL, ABS, 3, 6 });
    Check(0x0F, { uSLO, ABS, 3, 6 });
    
    Check(0x10, { BPL, REL, 2, 2 });
    Check(0x11, { ORA, IDY, 2, 5 });
    Check(0x12, { uSTP, IMP, 1, 2 }); // unsure
    Check(0x13, { uSLO, IDY, 2, 8 });
    Check(0x14, { uNOP, ZPX, 2, 4 });
    Check(0x15, { ORA, ZPX, 2, 4 });
    Check(0x16, { ASL, ZPX, 2, 6 });
    Check(0x17, { uSLO, ZPX, 2, 6 });
    Check(0x18, { CLC, IMP, 1, 2 });
    Check(0x19, { ORA, ABY, 3, 4 });
    Check(0x1A, { uNOP, IMP, 1, 2 });
    Check(0x1B, { uSLO, ABY, 3, 7 });
    Check(0x1C, { uNOP, ABX, 3, 4 });
    Check(0x1D, { ORA, ABX, 3, 4 });
    Check(0x1E, { ASL, ABX, 3, 7 });
    Check(0x1F, { uSLO, ABX, 3, 7 });
    
    Check(0x20, { JSR, ABS, 3, 6 });
    Check(0x21, { AND, IDX, 2, 6 });
    Check(0x22, { uSTP, IMP, 1, 2 }); // unsure
    Check(0x23, { uRLA, IDX, 2, 8 });
    Check(0x24, { BIT, ZPG, 2, 3 });
    Check(0x25, { AND, ZPG, 2, 3 });
    Check(0x26, { ROL, ZPG, 2, 5 });
    Check(0x27, { uRLA, ZPG, 2, 5 });
    Check(0x28, { PLP, IMP, 1, 4 });
    Check(0x29, { AND, IMM, 2, 2 });
    Check(0x2A, { ROL, ACC, 1, 2 });
    Check(0x2B, { uANC, IMM, 2, 2 });
    Check(0x2C, { BIT, ABS, 3, 4 });
    Check(0x2D, { AND, ABS, 3, 4 });
    Check(0x2E, { ROL, ABS, 3, 6 });
    Check(0x2F, { uRLA, ABS, 3, 6 });

    Check(0x30, { BMI, REL, 2, 2 });
    Check(0x31, { AND, IDY, 2, 5 });
    Check(0x32, { uSTP, IMP, 1, 2 }); // unsure
    Check(0x33, { uRLA, IDY, 2, 8 });
    Check(0x34, { uNOP, ZPX, 2, 4 });
    Check(0x35, { AND, ZPX, 2, 4 });
    Check(0x36, { ROL, ZPX, 2, 6 });
    Check(0x37, { uRLA, ZPX, 2, 6 });
    Check(0x38, { SEC, IMP, 1, 2 });
    Check(0x39, { AND, ABY, 3, 4 });
    Check(0x3A, { uNOP, IMP, 1, 2 });
    Check(0x3B, { uRLA, ABY, 3, 7 });
    Check(0x3C, { uNOP, ABX, 3, 4 });
    Check(0x3D, { AND, ABX, 3, 4 });
    Check(0x3E, { ROL, ABX, 3, 7 });
    Check(0x3F, { uRLA, ABX, 3, 7 });
    
    Check(0x40, { RTI, IMP, 1, 6 });
    Check(0x41, { EOR, IDX, 2, 6 });
    Check(0x42, { uSTP, IMP, 1, 2 }); // unsure
    Check(0x43, { uSRE, IDX, 2, 8 });
    Check(0x44, { uNOP, ZPG, 2, 3 });
    Check(0x45, { EOR, ZPG, 2, 3 });
    Check(0x46, { LSR, ZPG, 2, 5 });
    Check(0x47, { uSRE, ZPG, 2, 5 });
    Check(0x48, { PHA, IMP, 1, 3 });
    Check(0x49, { EOR, IMM, 2, 2 });
    Check(0x4A, { LSR, ACC, 1, 2 });
    Check(0x4B, { uALR, IMM, 2, 2 });
    Check(0x4C, { JMP, ABS, 3, 3 });
    Check(0x4D, { EOR, ABS, 3, 4 });
    Check(0x4E, { LSR, ABS, 3, 6 });
    Check(0x4F, { uSRE, ABS, 3, 6 });
    
    Check(0x50, { BVC, REL, 2, 2 });
    Check(0x51, { EOR, IDY, 2, 5 });
    Check(0x52, { uSTP, IMP, 1, 2 }); // unsure
    Check(0x53, { uSRE, IDY, 2, 8 });
    Check(0x54, { uNOP, ZPX, 2, 4 });
    Check(0x55, { EOR, ZPX, 2, 4 });
    Check(0x56, { LSR, ZPX, 2, 6 });
    Check(0x57, { uSRE, ZPX, 2, 6 });
    Check(0x58, { CLI, IMP, 1, 2 });
    Check(0x59, { EOR, ABY, 3, 4 });
    Check(0x5A, { uNOP, IMP, 1, 2 });
    Check(0x5B, { uSRE, ABY, 3, 7 });
    Check(0x5C, { uNOP, ABX, 3, 4 });
    Check(0x5D, { EOR, ABX, 3, 4 });
    Check(0x5E, { LSR, ABX, 3, 7 });
    Check(0x5F, { uSRE, ABX, 3, 7 });

    Check(0x60, { RTS, IMP, 1, 6 });
    Check(0x61, { ADC, IDX, 2, 6 });
    Check(0x62, { uSTP, IMP, 1, 2 }); // unsure
    Check(0x63, { uRRA, IDX, 2, 8 });
    Check(0x64, { uNOP, ZPG, 2, 3 });
    Check(0x65, { ADC, ZPG, 2, 3 });
    Check(0x66, { ROR, ZPG, 2, 5 });
    Check(0x67, { uRRA, ZPG, 2, 5 });
    Check(0x68, { PLA, IMP, 1, 4 });
    Check(0x69, { ADC, IMM, 2, 2 });
    Check(0x6A, { ROR, ACC, 1, 2 });
    Check(0x6B, { uARR, IMM, 2, 2 });
    Check(0x6C, { JMP, IND, 3, 5 });
    Check(0x6D, { ADC, ABS, 3, 4 });
    Check(0x6E, { ROR, ABS, 3, 6 });
    Check(0x6F, { uRRA, ABS, 3, 6 });

    Check(0x70, { BVS, REL, 2, 2 });
    Check(0x71, { ADC, IDY, 2, 5 });
    Check(0x72, { uSTP, IMP, 1, 2 }); // unsure
    Check(0x73, { uRRA, IDY, 2, 8 });
    Check(0x74, { uNOP, ZPX, 2, 4 });
    Check(0x75, { ADC, ZPX, 2, 4 });
    Check(0x76, { ROR, ZPX, 2, 6 });
    Check(0x77, { uRRA, ZPX, 2, 6 });
    Check(0x78, { SEI, IMP, 1, 2 });
    Check(0x79, { ADC, ABY, 3, 4 });
    Check(0x7A, { uNOP, IMP, 1, 2 });
    Check(0x7B, { uRRA, ABY, 3, 7 });
    Check(0x7C, { uNOP, ABX, 3, 4 });
    Check(0x7D, { ADC, ABX, 3, 4 });
    Check(0x7E, { ROR, ABX, 3, 7 });
    Check(0x7F, { uRRA, ABX, 3, 7 });

    Check(0x80, { uNOP, IMM, 2, 2 });
    Check(0x81, { STA, IDX, 2, 6 });
    Check(0x82, { uNOP, IMM, 2, 2 });
    Check(0x83, { uSAX, IDX, 2, 6 });
    Check(0x84, { STY, ZPG, 2, 3 });
    Check(0x85, { STA, ZPG, 2, 3 });
    Check(0x86, { STX, ZPG, 2, 3 });
    Check(0x87, { uSAX, ZPG, 2, 3 });
    Check(0x88, { DEY, IMP, 1, 2 });
    Check(0x89, { uNOP, IMM, 2, 2 });
    Check(0x8A, { TXA, IMP, 1, 2 });
    Check(0x8B, { uXAA, IMM, 2, 2 }); // unsure
    Check(0x8C, { STY, ABS, 3, 4 });
    Check(0x8D, { STA, ABS, 3, 4 });
    Check(0x8E, { STX, ABS, 3, 4 });
    Check(0x8F, { uSAX, ABS, 3, 4 });

    Check(0x90, { BCC, REL, 2, 2 });
    Check(0x91, { STA, IDY, 2, 6 });
    Check(0x92, { uSTP, IMP, 1, 2 }); // unsure
    Check(0x93, { uAHX, IDY, 2, 6 }); // unsure
    Check(0x94, { STY, ZPX, 2, 4 });
    Check(0x95, { STA, ZPX, 2, 4 });
    Check(0x96, { STX, ZPY, 2, 4 });
    Check(0x97, { uSAX, ZPY, 2, 4 });
    Check(0x98, { TYA, IMP, 1, 2 });
    Check(0x99, { STA, ABY, 3, 5 });
    Check(0x9A, { TXS, IMP, 1, 2 });
    Check(0x9B, { uTAS, ABY, 3, 5 }); // unsure
    Check(0x9C, { uSHY, ABX, 3, 5 }); // unsure
    Check(0x9D, { STA, ABX, 3, 5 });
    Check(0x9E, { uSHX, ABY, 3, 5 }); // unsure
    Check(0x9F, { uAHX, ABY, 3, 5 }); // unsure

    Check(0xA0, { LDY, IMM, 2, 2 });
    Check(0xA1, { LDA, IDX, 2, 6 });
    Check(0xA2, { LDX, IMM, 2, 2 });
    Check(0xA3, { uLAX, IDX, 2, 6 });
    Check(0xA4, { LDY, ZPG, 2, 3 });
    Check(0xA5, { LDA, ZPG, 2, 3 });
    Check(0xA6, { LDX, ZPG, 2, 3 });
    Check(0xA7, { uLAX, ZPG, 2, 3 });
    Check(0xA8, { TAY, IMP, 1, 2 });
    Check(0xA9, { LDA, IMM, 2, 2 });
    Check(0xAA, { TAX, IMP, 1, 2 });
    Check(0xAB, { uLAX, IMM, 2, 2 }); // unsure
    Check(0xAC, { LDY, ABS, 3, 4 });
    Check(0xAD, { LDA, ABS, 3, 4 });
    Check(0xAE, { LDX, ABS, 3, 4 });
    Check(0xAF, { uLAX, ABS, 3, 4 });

    Check(0xB0, { BCS, REL, 2, 2 });
    Check(0xB1, { LDA, IDY, 2, 5 });
    Check(0xB2, { uSTP, IMP, 1, 2 }); // unsure
    Check(0xB3, { uLAX, IDY, 2, 5 });
    Check(0xB4, { LDY, ZPX, 2, 4 });
    Check(0xB5, { LDA, ZPX, 2, 4 });
    Check(0xB6, { LDX, ZPY, 2, 4 });
    Check(0xB7, { uLAX, ZPY, 2, 4 });
    Check(0xB8, { CLV, IMP, 1, 2 });
    Check(0xB9, { LDA, ABY, 3, 4 });
    Check(0xBA, { TSX, IMP, 1, 2 });
    Check(0xBB, { uLAS, ABY, 3, 4 }); // unsure
    Check(0xBC, { LDY, ABX, 3, 4 });
    Check(0xBD, { LDA, ABX, 3, 4 });
    Check(0xBE, { LDX, ABY, 3, 4 });
    Check(0xBF, { uLAX, ABY, 3, 4 });

    Check(0xC0, { CPY, IMM, 2, 2 });
    Check(0xC1, { CMP, IDX, 2, 6 });
    Check(0xC2, { uNOP, IMM, 2, 2 });
    Check(0xC3, { uDCP, IDX, 2, 8 });
    Check(0xC4, { CPY, ZPG, 2, 3 });
    Check(0xC5, { CMP, ZPG, 2, 3 });
    Check(0xC6, { DEC, ZPG, 2, 5 });
    Check(0xC7, { uDCP, ZPG, 2, 5 });
    Check(0xC8, { INY, IMP, 1, 2 });
    Check(0xC9, { CMP, IMM, 2, 2 });
    Check(0xCA, { DEX, IMP, 1, 2 });
    Check(0xCB, { uAXS, IMM, 2, 2 });
    Check(0xCC, { CPY, ABS, 3, 4 });
    Check(0xCD, { CMP, ABS, 3, 4 });
    Check(0xCE, { DEC, ABS, 3, 6 });
    Check(0xCF, { uDCP, ABS, 3, 6 });

    Check(0xD0, { BNE, REL, 2, 2 });
    Check(0xD1, { CMP, IDY, 2, 5 });
    Check(0xD2, { uSTP, IMP, 1, 2 }); // unsure
    Check(0xD3, { uDCP, IDY, 2, 8 });
    Check(0xD4, { uNOP, ZPX, 2, 4 });
    Check(0xD5, { CMP, ZPX, 2, 4 });
    Check(0xD6, { DEC, ZPX, 2, 6 });
    Check(0xD7, { uDCP, ZPX, 2, 6 });
    Check(0xD8, { CLD, IMP, 1, 2 });
    Check(0xD9, { CMP, ABY, 3, 4 });
    Check(0xDA, { uNOP, IMP, 1, 2 });
    Check(0xDB, { uDCP, ABY, 3, 7 });
    Check(0xDC, { uNOP, ABX, 3, 4 });
    Check(0xDD, { CMP, ABX, 3, 4 });
    Check(0xDE, { DEC, ABX, 3, 7 });
    Check(0xDF, { uDCP, ABX, 3, 7 });

    Check(0xE0, { CPX, IMM, 2, 2 });
    Check(0xE1, { SBC, IDX, 2, 6 });
    Check(0xE2, { uNOP, IMM, 2, 2 });
    Check(0xE3, { uISC, IDX, 2, 8 });
    Check(0xE4, { CPX, ZPG, 2, 3 });
    Check(0xE5, { SBC, ZPG, 2, 3 });
    Check(0xE6, { INC, ZPG, 2, 5 });
    Check(0xE7, { uISC, ZPG, 2, 5 });
    Check(0xE8, { INX, IMP, 1, 2 });
    Check(0xE9, { SBC, IMM, 2, 2 });
    Check(0xEA, { NOP, IMP, 1, 2 });
    Check(0xEB, { uSBC, IMM, 2, 2 });
    Check(0xEC, { CPX, ABS, 3, 4 });
    Check(0xED, { SBC, ABS, 3, 4 });
    Check(0xEE, { INC, ABS, 3, 6 });
    Check(0xEF, { uISC, ABS, 3, 6 });

    Check(0xF0, { BEQ, REL, 2, 2 });
    Check(0xF1, { SBC, IDY, 2, 5 });
    Check(0xF2, { uSTP, IMP, 1, 2 }); // unsure
    Check(0xF3, { uISC, IDY, 2, 8 });
    Check(0xF4, { uNOP, ZPX, 2, 4 });
    Check(0xF5, { SBC, ZPX, 2, 4 });
    Check(0xF6, { INC, ZPX, 2, 6 });
    Check(0xF7, { uISC, ZPX, 2, 6 });
    Check(0xF8, { SED, IMP, 1, 2 });
    Check(0xF9, { SBC, ABY, 3, 4 });
    Check(0xFA, { uNOP, IMP, 1, 2 });
    Check(0xFB, { uISC, ABY, 3, 7 });
    Check(0xFC, { uNOP, ABX, 3, 4 });
    Check(0xFD, { SBC, ABX, 3, 4 });
    Check(0xFE, { INC, ABX, 3, 7 });
    Check(0xFF, { uISC, ABX, 3, 7 });

    EXPECT_EQ(0x100, counter);
}

TEST_F(CpuTest, OpcodeEncoding) {
    using is6502 = InstructionSet_6502;
    using enum is6502::OpName;
    using enum is6502::AddressingMode;

    // Relying on the previous test
    for (int i{ 0 }; i < 0x100; ++i) {
        const auto instruction{ is6502::Decode(i) };
        const auto opcode{ is6502::Encode(instruction.Name, instruction.Mode) };
        // EXPECT_EQ(i, opcode); // not ok because some instructions are duplicated
        const auto decoded_instruction{ is6502::Decode(opcode) };
        EXPECT_EQ(instruction, decoded_instruction);
    }
}

TEST_F(CpuTest, Opcode_Instruction) {
    // TODO check if it's actually useless now
    FAIL();

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
    // TODO check if it's actually useless now
    FAIL();
    
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
    EXPECT_EQ(0xFD, cpu.S);
    // On power up or reset, the CPU replaces stack writes by reads
    // => there is no correct value for the "B flag"
    // see https://www.pagetable.com/?p=410
    EXPECT_EQ(0x24, cpu.GetStatusByte(0));

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
    (void)cpu.Tick();
    EXPECT_EQ(1, cpu.CurrentTick);
    EXPECT_EQ(2, cpu.Ticks);

    (void)cpu.Tick();
    EXPECT_EQ(2, cpu.CurrentTick);
    EXPECT_EQ(2, cpu.Ticks);
    EXPECT_EQ(1, cpu.A);

    // ADC $00
    (void)cpu.Tick();
    EXPECT_EQ(3, cpu.CurrentTick);
    EXPECT_EQ(5, cpu.Ticks);

    (void)cpu.Tick();
    EXPECT_EQ(4, cpu.CurrentTick);
    EXPECT_EQ(5, cpu.Ticks);

    (void)cpu.Tick();
    EXPECT_EQ(5, cpu.CurrentTick);
    EXPECT_EQ(5, cpu.Ticks);
    EXPECT_EQ(3, cpu.A);
    EXPECT_EQ(2, mem.GetByteAt(0x0000));

    // INC $00
    (void)cpu.Tick();
    EXPECT_EQ(6, cpu.CurrentTick);
    EXPECT_EQ(10, cpu.Ticks);

    (void)cpu.Tick();
    (void)cpu.Tick();
    (void)cpu.Tick();
    (void)cpu.Tick();
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
    (void)cpu.Tick();
    EXPECT_EQ(1, cpu.CurrentTick);
    EXPECT_EQ(2, cpu.Ticks);
    
    (void)cpu.Tick();
    EXPECT_EQ(2, cpu.CurrentTick);
    EXPECT_EQ(2, cpu.Ticks);
    EXPECT_EQ(0, cpu.I);

    // LDA #1
    (void)cpu.Tick();
    EXPECT_EQ(3, cpu.CurrentTick);
    EXPECT_EQ(4, cpu.Ticks);

    cpu.TriggerIRQ();

    (void)cpu.Tick();
    EXPECT_EQ(4, cpu.CurrentTick);
    EXPECT_EQ(4, cpu.Ticks);
    EXPECT_EQ(1, cpu.A);

    // Interrupt
    for (int i = 0; i < cpu.InterruptCycles; ++i) (void)cpu.Tick();
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
    (void)cpu.Tick();
    cpu.TriggerIRQ();
    (void)cpu.Tick();

    // LDA #1
    (void)cpu.Tick();
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

