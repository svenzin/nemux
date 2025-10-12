#pragma once

#include "Types.h"

#include <stdexcept>
#include <tuple>

struct InstructionSet_6502 {
    static constexpr size_t INSTRUCTION_COUNT{ 0x100 };

    using OpCode = Byte;

    enum class OpName {
        LDA, LDX, LDY, STA, STX, STY,           // Load, Store
        TAX, TAY, TXA, TYA,                     // Register Transfer
        TSX, TXS, PHA, PLA, PHP, PLP,           // Stack
        AND, BIT, EOR, ORA,                     // Logical
        ADC, SBC, CMP, CPX, CPY,                // Arithmetic
        DEC, DEX, DEY, INC, INX, INY,           // Increment, Decrement
        ASL, LSR, ROL, ROR,                     // Shift
        JMP, JSR, RTS,                          // Jump, Call
        BCC, BCS, BEQ, BMI, BNE, BPL, BVC, BVS, // Branch
        CLC, CLD, CLI, CLV, SEC, SED, SEI,      // Status Change
        BRK, NOP, RTI,                          // System

        // Unofficial instructions
        uSTP, uSLO, uNOP, uANC, uRLA, uSRE, uALR, uRRA, uARR,
        uSAX, uXAA, uAHX, uTAS, uSHY, uSHX, uLAX, uLAS,
        uDCP, uAXS, uISC, uSBC,

        // Unknown (invalid)
        UNK
    };

    enum class AddressingMode {
        IMP, // implied
        ACC, // accumulator
        IMM, // immediate
        ZPG, // zeropage
        ZPX, // zeropage_x
        ZPY, // zeropage_y
        ABS, // absolute
        ABX, // absolute_x
        ABY, // absolute_y
        IND, // indirect
        IDX, // indexed_indirect
        IDY, // indirect_indexed
        REL, // relative
    };

    struct Instruction {
        OpName Name;
        AddressingMode Mode;
        Byte Bytes;
        Byte Cycles;

        auto operator<=>(const Instruction&) const = default;
    };

    static constexpr OpCode Encode(OpName name, AddressingMode mode) {
        using enum OpName;
        constexpr std::array<OpName, 0x100> OpNames{
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

        using enum AddressingMode;
        constexpr std::array<AddressingMode, 0x100> OpModes{
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

        for (int i{ 0 }; i < 0x100; ++i) {
            if (std::tie(OpNames[i], OpModes[i]) == std::tie(name, mode)) {
                return static_cast<Byte>(i);
            }
        }
        throw std::runtime_error("unexpected instruction name and addressing mode");
    }

    static constexpr Instruction Decode(OpCode opcode) {
        using enum OpName;
        constexpr std::array<OpName, 0x100> OpNames{
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

        using enum AddressingMode;
        constexpr std::array<AddressingMode, 0x100> OpModes{
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

        constexpr std::array<Byte, 0x100> OpSizes{
            /*       x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF */
            /* 0x */  2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
            /* 1x */  2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
            /* 2x */  3, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
            /* 3x */  2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
            /* 4x */  1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
            /* 5x */  2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
            /* 6x */  1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
            /* 7x */  2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
            /* 8x */  2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
            /* 9x */  2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
            /* Ax */  2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
            /* Bx */  2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
            /* Cx */  2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
            /* Dx */  2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
            /* Ex */  2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
            /* Fx */  2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
        };

        constexpr std::array<Byte, 0x100> OpCycles{
            /*       x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF */
            /* 0x */  7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
            /* 1x */  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
            /* 2x */  6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
            /* 3x */  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
            /* 4x */  6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
            /* 5x */  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
            /* 6x */  6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
            /* 7x */  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
            /* 8x */  2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
            /* 9x */  2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
            /* Ax */  2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
            /* Bx */  2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
            /* Cx */  2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
            /* Dx */  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
            /* Ex */  2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
            /* Fx */  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
        };

        return { OpNames[opcode], OpModes[opcode], OpSizes[opcode], OpCycles[opcode] };
    }
};
