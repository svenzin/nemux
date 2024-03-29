/*
 * Cpu.cpp
 *
 *  Created on: 10 Jun 2013
 *      Author: scorder
 */
#include "Cpu.h"

#include "BitUtil.h"
#include "Ppu.h"
#include "Controllers.h"

#include <iomanip>
#include <sstream>
#include <iostream>

using namespace std;
using namespace Instructions;
using namespace Addressing;

static const bool USE_RP2A03 = true;
static int nmic = 0;
static void Write2A03State(Cpu& cpu, Ricoh_RP2A03& rp2a03) {
    rp2a03.Halted = !cpu.IsAlive;
    rp2a03.PC = cpu.PC;
    rp2a03.S = cpu.SP;
    rp2a03.A = cpu.A;
    rp2a03.X = cpu.X;
    rp2a03.Y = cpu.Y;
    rp2a03.SetStatus(cpu.GetStatus());
    rp2a03.Map = cpu.Map;
    rp2a03.Ticks = cpu.Ticks;
}
static void Read2A03State(Ricoh_RP2A03& rp2a03, Cpu& cpu) {
    cpu.Ticks = rp2a03.Ticks;
    if (rp2a03.INSTR) cpu.CurrentTick = rp2a03.Ticks;
    cpu.PC = rp2a03.PC;
    cpu.SP = rp2a03.S;
    cpu.A = rp2a03.A;
    cpu.X = rp2a03.X;
    cpu.Y = rp2a03.Y;
    cpu.SetStatus(rp2a03.GetStatus(0));
    cpu.IsAlive = !rp2a03.Halted;
}

Word Cpu::ReadWordAt(const Word address) const {
    const auto lo = ReadByteAt(address);
    const auto hi = ReadByteAt(address + 1);
    return (hi << BYTE_WIDTH) + lo;
}

void Cpu::WriteWordAt(const Word address, const Word value) {
    WriteByteAt(address, value & BYTE_MASK);
    WriteByteAt(address + 1, (value >> BYTE_WIDTH) & BYTE_MASK);
}

Byte Cpu::ReadByteAt(const Word address) const {
    return Map->GetByteAt(address);
}

void Cpu::WriteByteAt(const Word address, const Byte value) {
    Map->SetByteAt(address, value);
}

/* explicit */ Cpu::Cpu(std::string name, MemoryMap * map)
    : Name{name},
      PC {0}, SP {0}, A {0}, X {0}, Y {0},
      C {0}, Z {0}, I {0}, D {0}, B {0}, V {0}, N {0}, Unused{1},
      Ticks{0}, InterruptCycles{7}, Map{map} {
    m_opcodes.resize(
        OPCODES_COUNT,
        Opcode(UNK, Unknown, 0, 0)
    );

    // Shift
    m_opcodes[0x0A] = Opcode(ASL, Accumulator, 1, 2);
    m_opcodes[0x06] = Opcode(ASL, ZeroPage,    2, 5);
    m_opcodes[0x16] = Opcode(ASL, ZeroPageX,   2, 6);
    m_opcodes[0x0E] = Opcode(ASL, Absolute,    3, 6);
    m_opcodes[0x1E] = Opcode(ASL, AbsoluteX,   3, 7);

    m_opcodes[0x4A] = Opcode(LSR, Accumulator, 1, 2);
    m_opcodes[0x46] = Opcode(LSR, ZeroPage,    2, 5);
    m_opcodes[0x56] = Opcode(LSR, ZeroPageX,   2, 6);
    m_opcodes[0x4E] = Opcode(LSR, Absolute,    3, 6);
    m_opcodes[0x5E] = Opcode(LSR, AbsoluteX,   3, 7);

    m_opcodes[0x2A] = Opcode(ROL, Accumulator, 1, 2);
    m_opcodes[0x26] = Opcode(ROL, ZeroPage,    2, 5);
    m_opcodes[0x36] = Opcode(ROL, ZeroPageX,   2, 6);
    m_opcodes[0x2E] = Opcode(ROL, Absolute,    3, 6);
    m_opcodes[0x3E] = Opcode(ROL, AbsoluteX,   3, 7);

    m_opcodes[0x6A] = Opcode(ROR, Accumulator, 1, 2);
    m_opcodes[0x66] = Opcode(ROR, ZeroPage,    2, 5);
    m_opcodes[0x76] = Opcode(ROR, ZeroPageX,   2, 6);
    m_opcodes[0x6E] = Opcode(ROR, Absolute,    3, 6);
    m_opcodes[0x7E] = Opcode(ROR, AbsoluteX,   3, 7);

    // Bit operations
    m_opcodes[0x29] = Opcode(AND, Immediate, 2, 2);
    m_opcodes[0x25] = Opcode(AND, ZeroPage,  2, 3);
    m_opcodes[0x35] = Opcode(AND, ZeroPageX, 2, 4);
    m_opcodes[0x2D] = Opcode(AND, Absolute,  3, 4);
    m_opcodes[0x3D] = Opcode(AND, AbsoluteX, 3, 4);
    m_opcodes[0x39] = Opcode(AND, AbsoluteY, 3, 4);
    m_opcodes[0x21] = Opcode(AND, IndexedIndirect, 2, 6);
    m_opcodes[0x31] = Opcode(AND, IndirectIndexed, 2, 5);

    m_opcodes[0x24] = Opcode(BIT, ZeroPage, 2, 3);
    m_opcodes[0x2C] = Opcode(BIT, Absolute, 3, 4);

    m_opcodes[0x49] = Opcode(EOR, Immediate, 2, 2);
    m_opcodes[0x45] = Opcode(EOR, ZeroPage,  2, 3);
    m_opcodes[0x55] = Opcode(EOR, ZeroPageX, 2, 4);
    m_opcodes[0x4D] = Opcode(EOR, Absolute,  3, 4);
    m_opcodes[0x5D] = Opcode(EOR, AbsoluteX, 3, 4);
    m_opcodes[0x59] = Opcode(EOR, AbsoluteY, 3, 4);
    m_opcodes[0x41] = Opcode(EOR, IndexedIndirect, 2, 6);
    m_opcodes[0x51] = Opcode(EOR, IndirectIndexed, 2, 5);

    m_opcodes[0x09] = Opcode(ORA, Immediate, 2, 2);
    m_opcodes[0x05] = Opcode(ORA, ZeroPage,  2, 3);
    m_opcodes[0x15] = Opcode(ORA, ZeroPageX, 2, 4);
    m_opcodes[0x0D] = Opcode(ORA, Absolute,  3, 4);
    m_opcodes[0x1D] = Opcode(ORA, AbsoluteX, 3, 4);
    m_opcodes[0x19] = Opcode(ORA, AbsoluteY, 3, 4);
    m_opcodes[0x01] = Opcode(ORA, IndexedIndirect, 2, 6);
    m_opcodes[0x11] = Opcode(ORA, IndirectIndexed, 2, 5);

    // Clear flags
    m_opcodes[0x18] = Opcode(CLC, Implicit, 1, 2);

    m_opcodes[0xD8] = Opcode(CLD, Implicit, 1, 2);

    m_opcodes[0x58] = Opcode(CLI, Implicit, 1, 2);

    m_opcodes[0xB8] = Opcode(CLV, Implicit, 1, 2);

    m_opcodes[0x38] = Opcode(SEC, Implicit, 1, 2);

    m_opcodes[0xF8] = Opcode(SED, Implicit, 1, 2);

    m_opcodes[0x78] = Opcode(SEI, Implicit, 1, 2);

    // Arithmetic
    m_opcodes[0x69] = Opcode(ADC, Immediate, 2, 2);
    m_opcodes[0x65] = Opcode(ADC, ZeroPage,  2, 3);
    m_opcodes[0x75] = Opcode(ADC, ZeroPageX, 2, 4);
    m_opcodes[0x6D] = Opcode(ADC, Absolute,  3, 4);
    m_opcodes[0x7D] = Opcode(ADC, AbsoluteX, 3, 4);
    m_opcodes[0x79] = Opcode(ADC, AbsoluteY, 3, 4);
    m_opcodes[0x61] = Opcode(ADC, IndexedIndirect, 2, 6);
    m_opcodes[0x71] = Opcode(ADC, IndirectIndexed, 2, 5);

    m_opcodes[0xE9] = Opcode(SBC, Immediate, 2, 2);
    m_opcodes[0xE5] = Opcode(SBC, ZeroPage,  2, 3);
    m_opcodes[0xF5] = Opcode(SBC, ZeroPageX, 2, 4);
    m_opcodes[0xED] = Opcode(SBC, Absolute,  3, 4);
    m_opcodes[0xFD] = Opcode(SBC, AbsoluteX, 3, 4);
    m_opcodes[0xF9] = Opcode(SBC, AbsoluteY, 3, 4);
    m_opcodes[0xE1] = Opcode(SBC, IndexedIndirect, 2, 6);
    m_opcodes[0xF1] = Opcode(SBC, IndirectIndexed, 2, 5);

    m_opcodes[0xC6] = Opcode(DEC, ZeroPage,  2, 5);
    m_opcodes[0xD6] = Opcode(DEC, ZeroPageX, 2, 6);
    m_opcodes[0xCE] = Opcode(DEC, Absolute,  3, 6);
    m_opcodes[0xDE] = Opcode(DEC, AbsoluteX, 3, 7);

    m_opcodes[0xCA] = Opcode(DEX, Implicit, 1, 2);

    m_opcodes[0x88] = Opcode(DEY, Implicit, 1, 2);

    m_opcodes[0xE6] = Opcode(INC, ZeroPage,  2, 5);
    m_opcodes[0xF6] = Opcode(INC, ZeroPageX, 2, 6);
    m_opcodes[0xEE] = Opcode(INC, Absolute,  3, 6);
    m_opcodes[0xFE] = Opcode(INC, AbsoluteX, 3, 7);

    m_opcodes[0xE8] = Opcode(INX, Implicit, 1, 2);

    m_opcodes[0xC8] = Opcode(INY, Implicit, 1, 2);

    // Branch
    m_opcodes[0x90] = Opcode(BCC, Relative, 2, 2);
    m_opcodes[0xB0] = Opcode(BCS, Relative, 2, 2);
    m_opcodes[0xF0] = Opcode(BEQ, Relative, 2, 2);
    m_opcodes[0x30] = Opcode(BMI, Relative, 2, 2);
    m_opcodes[0xD0] = Opcode(BNE, Relative, 2, 2);
    m_opcodes[0x10] = Opcode(BPL, Relative, 2, 2);
    m_opcodes[0x50] = Opcode(BVC, Relative, 2, 2);
    m_opcodes[0x70] = Opcode(BVS, Relative, 2, 2);

    // Comparisons
    m_opcodes[0xC9] = Opcode(CMP, Immediate, 2, 2);
    m_opcodes[0xC5] = Opcode(CMP, ZeroPage,  2, 3);
    m_opcodes[0xD5] = Opcode(CMP, ZeroPageX, 2, 4);
    m_opcodes[0xCD] = Opcode(CMP, Absolute,  3, 4);
    m_opcodes[0xDD] = Opcode(CMP, AbsoluteX, 3, 4);
    m_opcodes[0xD9] = Opcode(CMP, AbsoluteY, 3, 4);
    m_opcodes[0xC1] = Opcode(CMP, IndexedIndirect, 2, 6);
    m_opcodes[0xD1] = Opcode(CMP, IndirectIndexed, 2, 5);

    m_opcodes[0xE0] = Opcode(CPX, Immediate, 2, 2);
    m_opcodes[0xE4] = Opcode(CPX, ZeroPage,  2, 3);
    m_opcodes[0xEC] = Opcode(CPX, Absolute,  3, 4);

    m_opcodes[0xC0] = Opcode(CPY, Immediate, 2, 2);
    m_opcodes[0xC4] = Opcode(CPY, ZeroPage,  2, 3);
    m_opcodes[0xCC] = Opcode(CPY, Absolute,  3, 4);

    // Stack
    m_opcodes[0x48] = Opcode(PHA, Implicit, 1, 3);

    m_opcodes[0x68] = Opcode(PLA, Implicit, 1, 4);

    m_opcodes[0x08] = Opcode(PHP, Implicit, 1, 3);

    m_opcodes[0x28] = Opcode(PLP, Implicit, 1, 4);

    // Memory
    m_opcodes[0xA2] = Opcode(LDX, Immediate, 2, 2);
    m_opcodes[0xA6] = Opcode(LDX, ZeroPage,  2, 3);
    m_opcodes[0xB6] = Opcode(LDX, ZeroPageY, 2, 4);
    m_opcodes[0xAE] = Opcode(LDX, Absolute,  3, 4);
    m_opcodes[0xBE] = Opcode(LDX, AbsoluteY, 3, 4);

    m_opcodes[0xA0] = Opcode(LDY, Immediate, 2, 2);
    m_opcodes[0xA4] = Opcode(LDY, ZeroPage,  2, 3);
    m_opcodes[0xB4] = Opcode(LDY, ZeroPageX, 2, 4);
    m_opcodes[0xAC] = Opcode(LDY, Absolute,  3, 4);
    m_opcodes[0xBC] = Opcode(LDY, AbsoluteX, 3, 4);

    m_opcodes[0xA9] = Opcode(LDA, Immediate, 2, 2);
    m_opcodes[0xA5] = Opcode(LDA, ZeroPage,  2, 3);
    m_opcodes[0xB5] = Opcode(LDA, ZeroPageX, 2, 4);
    m_opcodes[0xAD] = Opcode(LDA, Absolute,  3, 4);
    m_opcodes[0xBD] = Opcode(LDA, AbsoluteX, 3, 4);
    m_opcodes[0xB9] = Opcode(LDA, AbsoluteY, 3, 4);
    m_opcodes[0xA1] = Opcode(LDA, IndexedIndirect, 2, 6);
    m_opcodes[0xB1] = Opcode(LDA, IndirectIndexed, 2, 5);

    m_opcodes[0x85] = Opcode(STA, ZeroPage,  2, 3);
    m_opcodes[0x95] = Opcode(STA, ZeroPageX, 2, 4);
    m_opcodes[0x8D] = Opcode(STA, Absolute,  3, 4);
    m_opcodes[0x9D] = Opcode(STA, AbsoluteX, 3, 5);
    m_opcodes[0x99] = Opcode(STA, AbsoluteY, 3, 5);
    m_opcodes[0x81] = Opcode(STA, IndexedIndirect, 2, 6);
    m_opcodes[0x91] = Opcode(STA, IndirectIndexed, 2, 6);

    m_opcodes[0x86] = Opcode(STX, ZeroPage,  2, 3);
    m_opcodes[0x96] = Opcode(STX, ZeroPageY, 2, 4);
    m_opcodes[0x8E] = Opcode(STX, Absolute,  3, 4);

    m_opcodes[0x84] = Opcode(STY, ZeroPage,  2, 3);
    m_opcodes[0x94] = Opcode(STY, ZeroPageX, 2, 4);
    m_opcodes[0x8C] = Opcode(STY, Absolute,  3, 4);

    m_opcodes[0xAA] = Opcode(TAX, Implicit, 1, 2);

    m_opcodes[0xA8] = Opcode(TAY, Implicit, 1, 2);

    m_opcodes[0xBA] = Opcode(TSX, Implicit, 1, 2);

    m_opcodes[0x8A] = Opcode(TXA, Implicit, 1, 2);

    m_opcodes[0x9A] = Opcode(TXS, Implicit, 1, 2);

    m_opcodes[0x98] = Opcode(TYA, Implicit, 1, 2);

    // Nop
    // BRK ticks set to 0 because the Interrupt()
    // method already handles the tick count
    m_opcodes[0x00] = Opcode(BRK, Implicit, 2, 0);

    m_opcodes[0xEA] = Opcode(NOP, Implicit, 1, 2);

    m_opcodes[0x40] = Opcode(RTI, Implicit, 1, 6);

    // Jump, Call
    m_opcodes[0x4C] = Opcode(JMP, Absolute, 3, 3);
    m_opcodes[0x6C] = Opcode(JMP, Indirect, 3, 5);

    m_opcodes[0x20] = Opcode(JSR, Absolute, 3, 6);

    m_opcodes[0x60] = Opcode(RTS, Implicit, 1, 6);

    // Unofficial opcodes
    m_opcodes[0x04] = Opcode(uNOP, ZeroPage,  2, 3);
    m_opcodes[0x0C] = Opcode(uNOP, Absolute,  3, 4);
    m_opcodes[0x14] = Opcode(uNOP, ZeroPageX, 2, 4);
    m_opcodes[0x1A] = Opcode(uNOP, Implicit,  1, 2);
    m_opcodes[0x1C] = Opcode(uNOP, AbsoluteX, 3, 4);    //
    m_opcodes[0x34] = Opcode(uNOP, ZeroPageX, 2, 4);
    m_opcodes[0x3A] = Opcode(uNOP, Implicit,  1, 2);
    m_opcodes[0x3C] = Opcode(uNOP, AbsoluteX, 3, 4);    //
    m_opcodes[0x44] = Opcode(uNOP, ZeroPage,  2, 3);
    m_opcodes[0x54] = Opcode(uNOP, ZeroPageX, 2, 4);
    m_opcodes[0x5A] = Opcode(uNOP, Implicit,  1, 2);
    m_opcodes[0x5C] = Opcode(uNOP, AbsoluteX, 3, 4);    //
    m_opcodes[0x64] = Opcode(uNOP, ZeroPage,  2, 3);
    m_opcodes[0x74] = Opcode(uNOP, ZeroPageX, 2, 4);
    m_opcodes[0x7A] = Opcode(uNOP, Implicit,  1, 2);
    m_opcodes[0x7C] = Opcode(uNOP, AbsoluteX, 3, 4);    //
    m_opcodes[0x80] = Opcode(uNOP, Immediate, 2, 2);
    m_opcodes[0x82] = Opcode(uNOP, Immediate, 2, 2);
    m_opcodes[0x89] = Opcode(uNOP, Immediate, 2, 2);
    m_opcodes[0xC2] = Opcode(uNOP, Immediate, 2, 2);
    m_opcodes[0xD4] = Opcode(uNOP, ZeroPageX, 2, 4);
    m_opcodes[0xDA] = Opcode(uNOP, Implicit,  1, 2);
    m_opcodes[0xDC] = Opcode(uNOP, AbsoluteX, 3, 4);    //
    m_opcodes[0xE2] = Opcode(uNOP, Immediate, 2, 2);
    m_opcodes[0xF4] = Opcode(uNOP, ZeroPageX, 2, 4);
    m_opcodes[0xFA] = Opcode(uNOP, Implicit,  1, 2);
    m_opcodes[0xFC] = Opcode(uNOP, AbsoluteX, 3, 4);    //

    m_opcodes[0x02] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0x12] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0x22] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0x32] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0x42] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0x52] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0x62] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0x72] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0x92] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0xB2] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0xD2] = Opcode(uSTP, Implicit, 1, 2);
    m_opcodes[0xF2] = Opcode(uSTP, Implicit, 1, 2);

    m_opcodes[0x03] = Opcode(uSLO, IndexedIndirect, 2, 8);
    m_opcodes[0x07] = Opcode(uSLO, ZeroPage,  2, 5);
    m_opcodes[0x0F] = Opcode(uSLO, Absolute,  3, 6);
    m_opcodes[0x13] = Opcode(uSLO, IndirectIndexed, 2, 8);
    m_opcodes[0x17] = Opcode(uSLO, ZeroPageX, 2, 6);
    m_opcodes[0x1B] = Opcode(uSLO, AbsoluteY, 3, 7);
    m_opcodes[0x1F] = Opcode(uSLO, AbsoluteX, 3, 7);

    m_opcodes[0x0B] = Opcode(uANC, Immediate, 2, 2);
    m_opcodes[0x2B] = Opcode(uANC, Immediate, 2, 2);

    m_opcodes[0x23] = Opcode(uRLA, IndexedIndirect, 2, 8);
    m_opcodes[0x27] = Opcode(uRLA, ZeroPage,  2, 5);
    m_opcodes[0x2F] = Opcode(uRLA, Absolute,  3, 6);
    m_opcodes[0x33] = Opcode(uRLA, IndirectIndexed, 2, 8);
    m_opcodes[0x37] = Opcode(uRLA, ZeroPageX, 2, 6);
    m_opcodes[0x3B] = Opcode(uRLA, AbsoluteY, 3, 7);
    m_opcodes[0x3F] = Opcode(uRLA, AbsoluteX, 3, 7);

    m_opcodes[0x43] = Opcode(uSRE, IndexedIndirect, 2, 8);
    m_opcodes[0x47] = Opcode(uSRE, ZeroPage,  2, 5);
    m_opcodes[0x4F] = Opcode(uSRE, Absolute,  3, 6);
    m_opcodes[0x53] = Opcode(uSRE, IndirectIndexed, 2, 8);
    m_opcodes[0x57] = Opcode(uSRE, ZeroPageX, 2, 6);
    m_opcodes[0x5B] = Opcode(uSRE, AbsoluteY, 3, 7);
    m_opcodes[0x5F] = Opcode(uSRE, AbsoluteX, 3, 7);

    m_opcodes[0x4B] = Opcode(uALR, Immediate, 2, 2);

    m_opcodes[0x63] = Opcode(uRRA, IndexedIndirect, 2, 8);
    m_opcodes[0x67] = Opcode(uRRA, ZeroPage,  2, 5);
    m_opcodes[0x6F] = Opcode(uRRA, Absolute,  3, 6);
    m_opcodes[0x73] = Opcode(uRRA, IndirectIndexed, 2, 8);
    m_opcodes[0x77] = Opcode(uRRA, ZeroPageX, 2, 6);
    m_opcodes[0x7B] = Opcode(uRRA, AbsoluteY, 3, 7);
    m_opcodes[0x7F] = Opcode(uRRA, AbsoluteX, 3, 7);

    m_opcodes[0x6B] = Opcode(uARR, Immediate, 2, 2);

    m_opcodes[0x83] = Opcode(uSAX, IndexedIndirect, 2, 6);
    m_opcodes[0x87] = Opcode(uSAX, ZeroPage, 2, 3);
    m_opcodes[0x8F] = Opcode(uSAX, Absolute, 3, 4);
    m_opcodes[0x97] = Opcode(uSAX, ZeroPageY, 2, 4);

    m_opcodes[0x8B] = Opcode(uXAA, Immediate, 2, 2);

    m_opcodes[0x93] = Opcode(uAHX, IndirectIndexed, 2, 6);
    m_opcodes[0x9F] = Opcode(uAHX, AbsoluteY, 3, 5);

    m_opcodes[0x9B] = Opcode(uTAS, AbsoluteY, 3, 5);

    m_opcodes[0x9C] = Opcode(uSHY, AbsoluteX, 3, 5);

    m_opcodes[0x9E] = Opcode(uSHX, AbsoluteY, 3, 5);

    m_opcodes[0xA3] = Opcode(uLAX, IndexedIndirect, 2, 6);
    m_opcodes[0xA7] = Opcode(uLAX, ZeroPage,  2, 3);
    m_opcodes[0xAB] = Opcode(uLAX, Immediate, 2, 2);
    m_opcodes[0xAF] = Opcode(uLAX, Absolute,  3, 4);
    m_opcodes[0xB3] = Opcode(uLAX, IndirectIndexed, 2, 5);
    m_opcodes[0xB7] = Opcode(uLAX, ZeroPageY, 2, 4);
    m_opcodes[0xBF] = Opcode(uLAX, AbsoluteY, 3, 4);

    m_opcodes[0xBB] = Opcode(uLAS, AbsoluteY, 3, 4);

    m_opcodes[0xC3] = Opcode(uDCP, IndexedIndirect, 2, 8);
    m_opcodes[0xC7] = Opcode(uDCP, ZeroPage,  2, 5);
    m_opcodes[0xCF] = Opcode(uDCP, Absolute,  3, 6);
    m_opcodes[0xD3] = Opcode(uDCP, IndirectIndexed, 2, 8);
    m_opcodes[0xD7] = Opcode(uDCP, ZeroPageX, 2, 6);
    m_opcodes[0xDB] = Opcode(uDCP, AbsoluteY, 3, 7);
    m_opcodes[0xDF] = Opcode(uDCP, AbsoluteX, 3, 7);

    m_opcodes[0xCB] = Opcode(uAXS, Immediate, 2, 2);

    m_opcodes[0xE3] = Opcode(uISC, IndexedIndirect, 2, 8);
    m_opcodes[0xE7] = Opcode(uISC, ZeroPage,  2, 5);
    m_opcodes[0xEF] = Opcode(uISC, Absolute,  3, 6);
    m_opcodes[0xF3] = Opcode(uISC, IndirectIndexed, 2, 8);
    m_opcodes[0xF7] = Opcode(uISC, ZeroPageX, 2, 6);
    m_opcodes[0xFB] = Opcode(uISC, AbsoluteY, 3, 7);
    m_opcodes[0xFF] = Opcode(uISC, AbsoluteX, 3, 7);

    m_opcodes[0xEB] = Opcode(uSBC, Immediate, 2, 2);

    // Power up state
    IsAlive = true;
    SP = 0xFD;
    SetStatus(0x34);
    CurrentTick = 0;
    PendingInterrupt = InterruptType::None;
}

void Cpu::Tick() {
    if (!USE_RP2A03) {
        ++CurrentTick;
        static auto m = dynamic_cast<CpuMemoryMap<Cpu, Ppu, Controllers, Apu<Cpu>> *>(Map);
        static bool nmi = false;
        static bool nmiDelayed1 = false;
        static bool nmiDelayed2 = false;
        static bool nmiDelayed3 = false;
        static bool nmiDelayed4 = false;
        if (m != nullptr) {
            if (!nmiDelayed4 && nmiDelayed3) {
                TriggerNMI();
            }
            nmiDelayed4 = nmiDelayed3;
            nmiDelayed3 = nmiDelayed2;
            nmiDelayed2 = nmiDelayed1;
            nmiDelayed1 = nmi;
            nmi = m->PPU->NMIActive;

            if (I == 0 && (
                m->APU->Frame.Interrupt ||
                m->APU->DMC1.Output.DMA.Interrupt)) {
                TriggerIRQ();
            }

        }
        if (CurrentTick > Ticks) {
            if (PendingInterrupt == InterruptType::Rst) {
                Reset();
            }
            else if (PendingInterrupt == InterruptType::Nmi) {
                NMI();
            }
            else if (PendingInterrupt == InterruptType::Irq) {
                IRQ();
            }
            else {
                const auto instruction = ReadByteAt(PC);
                const auto opcode = Decode(instruction);
                Execute(opcode);
            }
        }
    }
    else {
        Write2A03State(*this, rp2a03);
        rp2a03.Phi1();
        static auto m = dynamic_cast<CpuMemoryMap<Cpu, Ppu, Controllers, Apu<Cpu>> *>(Map);
        if (m != nullptr) {
            rp2a03.NMI = m->PPU->NMIActive;
            rp2a03.IRQ = (I == 0)
                && (m->APU->Frame.Interrupt || m->APU->DMC1.Output.DMA.Interrupt);
        }
        rp2a03.Phi2();
        Read2A03State(rp2a03, *this);
    }
}

Opcode Cpu::Decode(const Byte &byte) const {
    if (0 <= byte && byte < OPCODES_COUNT)
        return m_opcodes[byte];
    return Opcode(UNK, Unknown, 0, 0);
}

address_t Cpu::BuildAddress(const Addressing::Type & type) const {
    const Word PC_1 = PC + 1;
    switch (type) {
        case Immediate: {
            return { PC_1, false };
        }
        case ZeroPage: {
            return { ReadByteAt(PC_1), false };
        }
        case ZeroPageX: {
            const auto address = (ReadByteAt(PC_1) + X) & WORD_LO_MASK;
            return { static_cast<Word>(address), false };
        }
        case ZeroPageY: {
            const auto address = (ReadByteAt(PC_1) + Y) & WORD_LO_MASK;
            return { static_cast<Word>(address), false };
        }
        case Absolute: {
            return { ReadWordAt(PC_1), false };
        }
        case AbsoluteX: {
            const Word address = ReadWordAt(PC_1) + X;
            const bool crossed = (X > (address & BYTE_MASK));
            if (crossed) ReadByteAt(address - 0x0100); // Dummy read
            return { address, crossed };
        }
        case AbsoluteY: {
            const Word address = ReadWordAt(PC_1) + Y;
            const bool crossed = (Y > (address & BYTE_MASK));
            if (crossed) ReadByteAt(address - 0x0100); // Dummy read
            return{ address, crossed };
        }
        case IndexedIndirect: {
            const Word base = ReadWordAt(PC_1) + X;
            const Word lo = base & WORD_LO_MASK;
            const Word hi = (base + 1) & WORD_LO_MASK;
            const Word addr = ReadByteAt(hi) << BYTE_WIDTH | ReadByteAt(lo);
            return { addr, false };
        }
        case IndirectIndexed: {
            const Word base = ReadByteAt(PC_1);
            const Word lo = ReadByteAt(base);
            const Word hi = ReadByteAt((base + 1) & WORD_LO_MASK);
            const Word addr = (hi << BYTE_WIDTH) + lo + Y;
            const bool crossed = (Y > (addr & BYTE_MASK));
            if (crossed) ReadByteAt(addr - 0x0100); // Dummy read
            return { addr, crossed };
        }
        case Indirect: {
            const Word base = ReadWordAt(PC_1);
            const Word lo = base;
            const Word hi = (base & WORD_HI_MASK) | ((base + 1) & WORD_LO_MASK);
            const Word addr = ReadByteAt(hi) << BYTE_WIDTH | ReadByteAt(lo);
            return { addr, false };
        }
        case Implicit:
        case Accumulator:
            // Dummy fetch of the next opcode
            ReadByteAt(PC_1);
        default: return { Word(-1), false };
    }
}

void Cpu::Decrement(Byte & value) {
    Transfer(value - 1, value);
}
void Cpu::Increment(Byte & value) {
    Transfer(value + 1, value);
}
void Cpu::Transfer(const Byte & from, Byte & to) {
    to = from;
    Z = (to == 0) ? 1 : 0;
    N = Bit<Neg>(to);
}
void Cpu::Compare(const Byte lhs, const Byte rhs) {
    const auto r = lhs - rhs;
    C = (r >= 0) ? 1 : 0;
    Z = (r == 0) ? 1 : 0;
    N = (IsBitSet<BYTE_SIGN_BIT>(r)) ? 1 : 0;
}
void Cpu::BranchIf(const bool condition, const Opcode & op) {
    const auto basePC = PC;
    const auto M = ReadByteAt(BuildAddress(Immediate).Address - op.Bytes);
    if (condition) {
        Word offset = Bit<Neg>(M) * WORD_HI_MASK | M;
        PC = (PC + offset) & WORD_MASK;
        Ticks += 1;
        if ((PC & WORD_HI_MASK) != (basePC & WORD_HI_MASK)) {
            Ticks += 1;
        }
    }
}
void Cpu::AddWithCarry(const Byte value) {
    Word a = A + value + C;
    C = (a > BYTE_MASK) ? 1 : 0;
    V = ~Bit<Neg>(A ^ value) & Bit<Neg>(A ^ a);
    Transfer(a & BYTE_MASK, A);
}
void Cpu::SubstractWithCarry(const Byte value) {
    Word a = A - value - (1 - C);
    C = (a > BYTE_MASK) ? 0 : 1;
    V = Bit<Neg>(A ^ value) & Bit<Neg>(A ^ a);
    Transfer(a & BYTE_MASK, A);
}
void Cpu::Jump(const Word address) {
    PC = address;
}
void Cpu::Push(const Byte & value) {
    WriteByteAt(StackPage + SP, value);
    --SP;
}
Byte Cpu::Pull() {
    ++SP;
    return ReadByteAt(StackPage + SP);
}
void Cpu::PushWord(const Word & value) {
    Push((value >> BYTE_WIDTH) & BYTE_MASK);
    Push(value & BYTE_MASK);
}
Word Cpu::PullWord() {
    return Pull() | (Pull() << BYTE_WIDTH);
}
void Cpu::SetStatus(const Byte & status) {
    N = Bit<Neg>(status);
    V = Bit<Ovf>(status);
    B = Bit<Brk>(status);
    D = Bit<Dec>(status);
    I = Bit<Int>(status);
    Z = Bit<Zer>(status);
    C = Bit<Car>(status);
}
Byte Cpu::GetStatus() const {
    return Mask<Neg>(N)      | Mask<Ovf>(V) |
           Mask<Unu>(Unused) | Mask<Brk>(B) |
           Mask<Dec>(D)      | Mask<Int>(I) |
           Mask<Zer>(Z)      | Mask<Car>(C);
}
void Cpu::Interrupt(const Flag & isBRK,
                    const Word & vector,
                    const bool readOnly /*= false*/) {
    B = isBRK;
    if (readOnly) {
        SP -= 3;
    } else {
        PushWord(PC);
        Push(GetStatus());
    }
    I = 1;
    PC = ReadWordAt(vector);
    Ticks += InterruptCycles;
}
void Cpu::PowerUp() {
    PC = ReadWordAt(VectorRST);
}
void Cpu::Reset() {
    if (USE_RP2A03) {
        rp2a03.Reset();
    }
    else {
        std::cout << "RST" << std::endl;
        PendingInterrupt = InterruptType::None;
        Interrupt(0, VectorRST, true);
    }
}
void Cpu::NMI() {
    std::cout << "NMI " << nmic++ << std::endl;
    PendingInterrupt = InterruptType::None;
    Interrupt(0, VectorNMI);
}
void Cpu::IRQ() {
    std::cout << "IRQ" << std::endl;
    PendingInterrupt = InterruptType::None;
    Interrupt(0, VectorIRQ);
}
void Cpu::TriggerReset() {
    PendingInterrupt = InterruptType::Rst;
}
void Cpu::TriggerNMI() {
    if (PendingInterrupt != InterruptType::Rst) {
        PendingInterrupt = InterruptType::Nmi;
    }
}
void Cpu::TriggerIRQ() {
    if (PendingInterrupt == InterruptType::None) {
        PendingInterrupt = InterruptType::Irq;
    }
}
void Cpu::DMA(const Byte page,
    std::array<Byte, 0x0100> & target,
    const Byte offset) {
    if (USE_RP2A03) {
        Write2A03State(*this, rp2a03);
        rp2a03.DMA(page, target.data(), offset);
        Read2A03State(rp2a03, *this);
    }
    else {
        const Word base = page << BYTE_WIDTH;
        for (Word i = 0; i < 0x0100; ++i) {
            target[(i + offset) & WORD_LO_MASK] = ReadByteAt(base + i);
        }
        Ticks += 513;
        if (CurrentTick % 2 == 1) {
            ++Ticks;
        }
    }
}
void Cpu::Execute(const Opcode &op) {
    if (USE_RP2A03) {
        Write2A03State(*this, rp2a03);

        int hexa;
        for (hexa = 0; hexa < 0x100; ++hexa) {
            const auto ophexa = Decode(hexa);
            if ((ophexa.Addressing == op.Addressing)
                && (ophexa.Bytes == op.Bytes)
                && (ophexa.Cycles == op.Cycles)
                && (ophexa.Instruction == op.Instruction)) break;
        }
        WriteByteAt(PC, hexa);
        do {
            rp2a03.Phi1();
            rp2a03.Phi2();
        } while (!rp2a03.INSTR);
        
        Read2A03State(rp2a03, *this);
        return;
    }
    if (!IsAlive) return;

    const auto a = BuildAddress(op.Addressing);
    Ticks += op.Cycles;
    PC += op.Bytes;

    switch (op.Instruction) {
    case BRK: Interrupt(1, VectorIRQ); break;
    case JMP: Jump(a.Address); break;
    case JSR: {
        PushWord(PC - 1);
        Jump(a.Address);
        break;
    }
    case RTS: Jump(PullWord() + 1); break;
    case RTI: {
        SetStatus(Pull());
        Jump(PullWord());
        break;
    }
    case PLP: SetStatus(Pull()); break;
    case PHP: {
        B = 1;
        Push(GetStatus());
        break;
    }
    case PHA: Push(A); break;
    case PLA: Transfer(Pull(), A); break;
    case LDA: {
        Transfer(ReadByteAt(a.Address), A);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case LDX: {
        Transfer(ReadByteAt(a.Address), X);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case LDY: {
        Transfer(ReadByteAt(a.Address), Y);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case EOR: {
        Transfer(A ^ ReadByteAt(a.Address), A);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case ORA: {
        const auto m = ReadByteAt(a.Address);
        Transfer(A | m, A);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case CMP: {
        Compare(A, ReadByteAt(a.Address));
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case CPX: Compare(X, ReadByteAt(a.Address)); break;
    case CPY: Compare(Y, ReadByteAt(a.Address)); break;
    case TAX: Transfer( A, X); break;
    case TAY: Transfer( A, Y); break;
    case TSX: Transfer(SP, X); break;
    case TXA: Transfer( X, A); break;
    case TXS: SP = X;          break; // TXS does not change the flags
    case TYA: Transfer( Y, A); break;
    case STA: WriteByteAt(a.Address, A); break;
    case STX: WriteByteAt(a.Address, X); break;
    case STY: WriteByteAt(a.Address, Y); break;
    case BCC: BranchIf(C == 0, op); break;
    case BCS: BranchIf(C == 1, op); break;
    case BEQ: BranchIf(Z == 1, op); break;
    case BMI: BranchIf(N == 1, op); break;
    case BNE: BranchIf(Z == 0, op); break;
    case BPL: BranchIf(N == 0, op); break;
    case BVC: BranchIf(V == 0, op); break;
    case BVS: BranchIf(V == 1, op); break;
    case ADC: {
        const auto M = ReadByteAt(a.Address);
        AddWithCarry(M);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case uSBC:
    case SBC: {
        const auto M = ReadByteAt(a.Address);
        SubstractWithCarry(M);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case ASL: {
        const auto address = a.Address;
        if (op.Addressing == Accumulator) {
            C = Bit<Left>(A);
            Transfer(A << 1, A);
        }
        else {
            auto M = ReadByteAt(address);
            C = Bit<Left>(M);
            Transfer(M << 1, M);
            WriteByteAt(address, M);
        }
        break;
    }
    case LSR: {
        const auto address = a.Address;
        if (op.Addressing == Accumulator) {
            C = Bit<Right>(A);
            Transfer(A >> 1, A);
        }
        else {
            auto M = ReadByteAt(address);
            C = Bit<Right>(M);
            Transfer(M >> 1, M);
            WriteByteAt(address, M);
        }
        break;
    }
    case ROL: {
        const auto address = a.Address;
        if (op.Addressing == Accumulator) {
            const auto c = Bit<Left>(A);
            Transfer((A << 1) | Mask<Right>(C), A);
            C = c;
        }
        else {
            auto M = ReadByteAt(address);
            const auto c = Bit<Left>(M);
            Transfer((M << 1) | Mask<Right>(C), M);
            C = c;
            WriteByteAt(address, M);
        }
        break;
    }
    case ROR: {
        const auto address = a.Address;
        if (op.Addressing == Accumulator) {
            const auto c = Bit<Right>(A);
            Transfer((A >> 1) | Mask<Left>(C), A);
            C = c;
        }
        else {
            auto M = ReadByteAt(address);
            const auto c = Bit<Right>(M);
            Transfer((M >> 1) | Mask<Left>(C), M);
            C = c;
            WriteByteAt(address, M);
        }
        break;
    }
    case AND: {
        const auto M = ReadByteAt(a.Address);
        Transfer(A & M, A);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case BIT: {
        const auto mask = ReadByteAt(a.Address);
        Z = (mask & A) == 0 ? 1 : 0;
        V = Bit<Ovf>(mask);
        N = Bit<Neg>(mask);
        break;
    }
    case CLC: C = 0; break;
    case CLD: D = 0; break;
    case CLI: I = 0; break;
    case CLV: V = 0; break;
    case SEC: C = 1; break;
    case SED: D = 1; break;
    case SEI: I = 1; break;
    case DEC: {
        auto M = ReadByteAt(a.Address);
        Decrement(M);
        WriteByteAt(a.Address, M);
        break;
    }
    case DEX: Decrement(X); break;
    case DEY: Decrement(Y); break;
    case INC: {
        auto M = ReadByteAt(a.Address);
        Increment(M);
        WriteByteAt(a.Address, M);
        break;
    }
    case INX: Increment(X); break;
    case INY: Increment(Y); break;
    case uSTP: IsAlive = false; break;
    case uSLO: {
        const auto address = a.Address;
        auto M = ReadByteAt(address);
        C = Bit<Left>(M);
        Transfer(M << 1, M);
        WriteByteAt(address, M);
        Transfer(A | M, A);
        break;
    }
    case uANC: {
        const auto M = ReadByteAt(a.Address);
        Transfer(A & M, A);
        C = Bit<Left>(A);
        break;
    }
    case uRLA: {
        const auto address = a.Address;
        auto M = ReadByteAt(address);
        const auto c = Bit<Left>(M);
        Transfer((M << 1) | Mask<Right>(C), M);
        C = c;
        WriteByteAt(address, M);
        Transfer(A & M, A);
        break;
    }
    case uSRE: {
        const auto address = a.Address;
        auto M = ReadByteAt(address);
        C = Bit<Right>(M);
        Transfer(M >> 1, M);
        WriteByteAt(address, M);
        Transfer(A ^ M, A);
        break;
    }
    case uALR: {
        const auto M = ReadByteAt(a.Address);
        Transfer(A & M, A);
        C = Bit<Right>(A);
        Transfer(A >> 1, A);
        break;
    }
    case uRRA: {
        const auto address = a.Address;
        auto M = ReadByteAt(address);
        const auto c = Bit<Right>(M);
        Transfer((M >> 1) | Mask<Left>(C), M);
        C = c;
        WriteByteAt(address, M);
        AddWithCarry(M);
        break;
    }
    case uARR: {
        const auto address = a.Address;
        const auto M = ReadByteAt(address);
        Transfer(A & M, A);
        Transfer((A >> 1) | Mask<Left>(C), A);
        switch ((A >> 5) & 0x03) {
        case 0: { C = 0; V = 0; break; }
        case 1: { C = 0; V = 1; break; }
        case 2: { C = 1; V = 1; break; }
        case 3: { C = 1; V = 0; break; }
        }
        break;
    }
    case uSAX: WriteByteAt(a.Address, A & X); break;
    case uAHX: {
        const auto address = a.Address;
        const auto H = ((address & WORD_HI_MASK) >> BYTE_WIDTH);
        WriteByteAt(address, A & X & H);
        break;
    }
    case uTAS: {
        const auto address = a.Address;
        const auto H = ((address & WORD_HI_MASK) >> BYTE_WIDTH);
        // Don't transfer because flags are not updated
        SP = (A & X);
        WriteByteAt(address, A & X & H);
        break;
    }
    case uSHY: {
        const auto H = ((a.Address & WORD_HI_MASK) >> BYTE_WIDTH);
        const auto M = (Y & (H + 1));
        if (a.HasCrossedPage) {
            // In case the resulting addres crosses a page
            // The bahviour is corrupted
            // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
            const auto address = (M << BYTE_WIDTH) | (a.Address & WORD_LO_MASK);
            WriteByteAt(address, M);
        }
        else WriteByteAt(a.Address, M);
        break;
    }
    case uSHX: {
        const auto H = ((a.Address & WORD_HI_MASK) >> BYTE_WIDTH);
        const auto M = X & (H + 1);
        if (a.HasCrossedPage) {
            // In case the resulting addres crosses a page
            // The bahviour is corrupted
            // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
            const auto address = (M << BYTE_WIDTH) | (a.Address & WORD_LO_MASK);
            WriteByteAt(address, M);
        }
        else WriteByteAt(a.Address, M);
        break;
    }
    case uLAX: {
        const auto M = ReadByteAt(a.Address);
        Transfer(M, A);
        Transfer(M, X);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case uLAS: {
        const auto M = ReadByteAt(a.Address);
        Transfer(M & SP, SP);
        Transfer(SP, A);
        Transfer(SP, X);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case uDCP: {
        auto M = ReadByteAt(a.Address);
        Decrement(M);
        WriteByteAt(a.Address, M);
        Compare(A, M);
        break;
    }
    case uAXS: {
        auto M = ReadByteAt(a.Address);
        X = (A & X);
        Compare(X, M); // Flags are set like CMP
        X = X - M;
        break;
    }
    case uISC: {
        const auto address = a.Address;
        auto M = ReadByteAt(address);
        Increment(M);
        WriteByteAt(address, M);
        SubstractWithCarry(M);
        break;
    }

    case NOP:
    case uNOP:
    case uXAA:
    case UNK:
    default:
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
}

string Cpu::ToString() const {
    ostringstream value;
    value << "Cpu " << Name << endl
          << "- Registers PC 0x" << hex << setfill('0') << setw(4) << PC << "(" << dec << PC << ")" << endl
          << "            SP 0x" << hex << setfill('0') << setw(2) << SP << "(" << dec << SP << ")" << endl
          << "             A 0x" << hex << setfill('0') << setw(2) <<  A << "(" << dec <<  A << ")" << endl
          << "             X 0x" << hex << setfill('0') << setw(2) <<  X << "(" << dec <<  X << ")" << endl
          << "             Y 0x" << hex << setfill('0') << setw(2) <<  Y << "(" << dec <<  Y << ")" << endl
          << "- Flags C " << setw(5) << boolalpha << (C != 0) << endl
          << "        Z " << setw(5) << boolalpha << (Z != 0) << endl
          << "        I " << setw(5) << boolalpha << (I != 0) << endl
          << "        D " << setw(5) << boolalpha << (D != 0) << endl
          << "        B " << setw(5) << boolalpha << (B != 0) << endl
          << "        V " << setw(5) << boolalpha << (V != 0) << endl
          << "        N " << setw(5) << boolalpha << (N != 0) << endl;
    return value.str();
}

std::string Cpu::ToMiniString() const {
    ostringstream value;
    const auto P = GetStatus();
    value << "Cpu " << Name
          << " " << CurrentTick << "@" << Ticks
          << " PC=$" << hex << setfill('0') << setw(4) << PC
          << " S=$" << hex << setfill('0') << setw(2) << Word{SP}
          << " A=$" << hex << setfill('0') << setw(2) << Word{A}
          << " X=$" << hex << setfill('0') << setw(2) << Word{X}
          << " Y=$" << hex << setfill('0') << setw(2) << Word{Y}
          << " P=$" << hex << setfill('0') << setw(2) << Word{P}
          << " "
          << (C == 0 ? 'c' : 'C')
          << (Z == 0 ? 'z' : 'Z')
          << (I == 0 ? 'i' : 'I')
          << (D == 0 ? 'd' : 'D')
          << (B == 0 ? 'b' : 'B')
          << (V == 0 ? 'v' : 'V')
          << (N == 0 ? 'n' : 'N');
    return value.str();
}
