/*
 * Cpu.cpp
 *
 *  Created on: 10 Jun 2013
 *      Author: scorder
 */
#include "Cpu.h"

#include <iomanip>
#include <sstream>
#include <iostream>

using namespace std;

/* explicit */ Cpu::Cpu(std::string name)
    : Name{name},
      PC {0}, SP {0}, A {0}, X {0}, Y {0},
      C {0}, Z {0}, I {0}, D {0}, B {0}, V {0}, N {0},
      Ticks{0}, Memory{"", 0} {
    m_opcodes.resize(
        OPCODES_COUNT,
        Opcode(InstructionName::UNK, AddressingType::Unknown, 0, 0)
    );

    // Shift
    m_opcodes[0x0A] = Opcode(InstructionName::ASL, AddressingType::Accumulator, 1, 2);
    m_opcodes[0x06] = Opcode(InstructionName::ASL, AddressingType::ZeroPage,    2, 5);
    m_opcodes[0x16] = Opcode(InstructionName::ASL, AddressingType::ZeroPageX,   2, 6);
    m_opcodes[0x0E] = Opcode(InstructionName::ASL, AddressingType::Absolute,    3, 6);
    m_opcodes[0x1E] = Opcode(InstructionName::ASL, AddressingType::AbsoluteX,   3, 7);

    // Bit operations
    m_opcodes[0x29] = Opcode(InstructionName::AND, AddressingType::Immediate, 2, 2);
    m_opcodes[0x25] = Opcode(InstructionName::AND, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0x35] = Opcode(InstructionName::AND, AddressingType::ZeroPageX, 2, 4);
    m_opcodes[0x2D] = Opcode(InstructionName::AND, AddressingType::Absolute,  3, 4);
    m_opcodes[0x3D] = Opcode(InstructionName::AND, AddressingType::AbsoluteX, 3, 4, true);
    m_opcodes[0x39] = Opcode(InstructionName::AND, AddressingType::AbsoluteY, 3, 4, true);
    m_opcodes[0x21] = Opcode(InstructionName::AND, AddressingType::IndexedIndirect, 2, 6);
    m_opcodes[0x31] = Opcode(InstructionName::AND, AddressingType::IndirectIndexed, 2, 5, true);

    m_opcodes[0x24] = Opcode(InstructionName::BIT, AddressingType::ZeroPage, 2, 3);
    m_opcodes[0x2C] = Opcode(InstructionName::BIT, AddressingType::Absolute, 3, 4);

    // Clear flags
    m_opcodes[0x18] = Opcode(InstructionName::CLC, AddressingType::Implicit, 1, 2);

    m_opcodes[0xD8] = Opcode(InstructionName::CLD, AddressingType::Implicit, 1, 2);

    m_opcodes[0x58] = Opcode(InstructionName::CLI, AddressingType::Implicit, 1, 2);

    m_opcodes[0xB8] = Opcode(InstructionName::CLV, AddressingType::Implicit, 1, 2);

    m_opcodes[0x38] = Opcode(InstructionName::SEC, AddressingType::Implicit, 1, 2);

    m_opcodes[0xF8] = Opcode(InstructionName::SED, AddressingType::Implicit, 1, 2);

    m_opcodes[0x78] = Opcode(InstructionName::SEI, AddressingType::Implicit, 1, 2);

    // Arithmetic
    m_opcodes[0x69] = Opcode(InstructionName::ADC, AddressingType::Immediate, 2, 2);
    m_opcodes[0x65] = Opcode(InstructionName::ADC, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0x75] = Opcode(InstructionName::ADC, AddressingType::ZeroPageX, 2, 4);
    m_opcodes[0x6D] = Opcode(InstructionName::ADC, AddressingType::Absolute,  3, 4);
    m_opcodes[0x7D] = Opcode(InstructionName::ADC, AddressingType::AbsoluteX, 3, 4, true);
    m_opcodes[0x79] = Opcode(InstructionName::ADC, AddressingType::AbsoluteY, 3, 4, true);
    m_opcodes[0x61] = Opcode(InstructionName::ADC, AddressingType::IndexedIndirect, 2, 6);
    m_opcodes[0x71] = Opcode(InstructionName::ADC, AddressingType::IndirectIndexed, 2, 5, true);

    m_opcodes[0xC6] = Opcode(InstructionName::DEC, AddressingType::ZeroPage,  2, 5);
    m_opcodes[0xD6] = Opcode(InstructionName::DEC, AddressingType::ZeroPageX, 2, 6);
    m_opcodes[0xCE] = Opcode(InstructionName::DEC, AddressingType::Absolute,  3, 6);
    m_opcodes[0xDE] = Opcode(InstructionName::DEC, AddressingType::AbsoluteX, 3, 7);

    m_opcodes[0xCA] = Opcode(InstructionName::DEX, AddressingType::Implicit, 1, 2);

    m_opcodes[0x88] = Opcode(InstructionName::DEY, AddressingType::Implicit, 1, 2);

    m_opcodes[0xE6] = Opcode(InstructionName::INC, AddressingType::ZeroPage,  2, 5);
    m_opcodes[0xF6] = Opcode(InstructionName::INC, AddressingType::ZeroPageX, 2, 6);
    m_opcodes[0xEE] = Opcode(InstructionName::INC, AddressingType::Absolute,  3, 6);
    m_opcodes[0xFE] = Opcode(InstructionName::INC, AddressingType::AbsoluteX, 3, 7);

    m_opcodes[0xE8] = Opcode(InstructionName::INX, AddressingType::Implicit, 1, 2);

    m_opcodes[0xC8] = Opcode(InstructionName::INY, AddressingType::Implicit, 1, 2);

    // Branch
    m_opcodes[0x90] = Opcode(InstructionName::BCC, AddressingType::Relative, 2, 2);
    m_opcodes[0xB0] = Opcode(InstructionName::BCS, AddressingType::Relative, 2, 2);
    m_opcodes[0xF0] = Opcode(InstructionName::BEQ, AddressingType::Relative, 2, 2);
    m_opcodes[0x30] = Opcode(InstructionName::BMI, AddressingType::Relative, 2, 2);
    m_opcodes[0xD0] = Opcode(InstructionName::BNE, AddressingType::Relative, 2, 2);
    m_opcodes[0x10] = Opcode(InstructionName::BPL, AddressingType::Relative, 2, 2);
    m_opcodes[0x50] = Opcode(InstructionName::BVC, AddressingType::Relative, 2, 2);
    m_opcodes[0x70] = Opcode(InstructionName::BVS, AddressingType::Relative, 2, 2);

    // Stack
    m_opcodes[0x00] = Opcode(InstructionName::BRK, AddressingType::Implicit, 1, 7);

    // Memory
    m_opcodes[0x85] = Opcode(InstructionName::STA, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0x95] = Opcode(InstructionName::STA, AddressingType::ZeroPageX, 2, 4);
    m_opcodes[0x8D] = Opcode(InstructionName::STA, AddressingType::Absolute,  3, 4);
    m_opcodes[0x9D] = Opcode(InstructionName::STA, AddressingType::AbsoluteX, 3, 5);
    m_opcodes[0x99] = Opcode(InstructionName::STA, AddressingType::AbsoluteY, 3, 5);
    m_opcodes[0x81] = Opcode(InstructionName::STA, AddressingType::IndexedIndirect, 2, 6);
    m_opcodes[0x91] = Opcode(InstructionName::STA, AddressingType::IndirectIndexed, 2, 6);

    m_opcodes[0x86] = Opcode(InstructionName::STX, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0x96] = Opcode(InstructionName::STX, AddressingType::ZeroPageY, 2, 4);
    m_opcodes[0x8E] = Opcode(InstructionName::STX, AddressingType::Absolute,  3, 4);

    m_opcodes[0x84] = Opcode(InstructionName::STY, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0x94] = Opcode(InstructionName::STY, AddressingType::ZeroPageX, 2, 4);
    m_opcodes[0x8C] = Opcode(InstructionName::STY, AddressingType::Absolute,  3, 4);

    m_opcodes[0xAA] = Opcode(InstructionName::TAX, AddressingType::Implicit, 1, 2);

    m_opcodes[0xA8] = Opcode(InstructionName::TAY, AddressingType::Implicit, 1, 2);

    m_opcodes[0xBA] = Opcode(InstructionName::TSX, AddressingType::Implicit, 1, 2);

    m_opcodes[0x8A] = Opcode(InstructionName::TXA, AddressingType::Implicit, 1, 2);

    m_opcodes[0x9A] = Opcode(InstructionName::TXS, AddressingType::Implicit, 1, 2);

    m_opcodes[0x98] = Opcode(InstructionName::TYA, AddressingType::Implicit, 1, 2);

    // Nop
    m_opcodes[0xEA] = Opcode(InstructionName::NOP, AddressingType::Implicit, 1, 2);
}

Word Cpu::BuildAddress(const AddressingType & type) const {
    switch (type) {
        case AddressingType::Immediate: return PC + 1;
        case AddressingType::ZeroPage:  return Memory.GetByteAt(PC + 1);
        case AddressingType::ZeroPageX: return Memory.GetByteAt(PC + 1) + X;
        case AddressingType::ZeroPageY: return Memory.GetByteAt(PC + 1) + Y;
        case AddressingType::Absolute:  return Memory.GetWordAt(PC + 1);
        case AddressingType::AbsoluteX: return Memory.GetWordAt(PC + 1) + X;
        case AddressingType::AbsoluteY: return Memory.GetWordAt(PC + 1) + Y;
        case AddressingType::IndexedIndirect: return Memory.GetWordAt(Memory.GetByteAt(PC + 1) + X);
        case AddressingType::IndirectIndexed: return Memory.GetWordAt(Memory.GetWordAt(Memory.GetByteAt(PC + 1)) + Y);
        default: return -1;
    }
}

void Cpu::Decrement(Byte & value) {
    value = (value - 1) & BYTE_MASK;//((value + 0x180 - 1) % 0x100) - 0x80;
    Z = (value == 0) ? 1 : 0;
    N = ((value & BYTE_MASK_SIGN) == 0) ? 0 : 1;
}
void Cpu::Increment(Byte & value) {
    value = (value + 1) & BYTE_MASK;//((value + 0x180 - 1) % 0x100) - 0x80;
    Z = (value == 0) ? 1 : 0;
    N = ((value & BYTE_MASK_SIGN) == 0) ? 0 : 1;
}
void Cpu::Transfer(Byte & from, Byte & to) {
    to = from;
    Z = (to == 0) ? 1 : 0;
    N = ((to & BYTE_MASK_SIGN) == 0) ? 0 : 1;
}
void Cpu::BranchIf(const bool condition, const Opcode & op) {
    const auto basePC = PC;
    const auto M = Memory.GetByteAt(BuildAddress(AddressingType::Immediate));
    if (condition) {
        Word offset = ((M & BYTE_MASK_SIGN) >> 7) * 0xFF00 + M;
        PC = (PC + offset) & 0xFFFF;
        Ticks += op.Cycles + 1;
        if ((PC & 0xFF00) != (basePC & 0xFF00)) {
            Ticks += 2;
        }
    } else {
        PC += op.Bytes; Ticks += op.Cycles;
    }
}
void Cpu::Execute(const Opcode &op) {//, const std::vector<Byte> &data) {
    switch(op.Instruction) {
        case InstructionName::TAX: {
            Transfer(A, X);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::TAY: {
            Transfer(A, Y);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::TSX: {
            Transfer(SP, X);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::TXA: {
            Transfer(X, A);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::TXS: {
            Transfer(X, SP);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::TYA: {
            Transfer(Y, A);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::STA: {
            Memory.SetByteAt(BuildAddress(op.Addressing), A);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::STX: {
            Memory.SetByteAt(BuildAddress(op.Addressing), X);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::STY: {
            Memory.SetByteAt(BuildAddress(op.Addressing), Y);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::BCC: {
            BranchIf(C == 0, op);
            break;
        }
        case InstructionName::BCS: {
            BranchIf(C == 1, op);
            break;
        }
        case InstructionName::BEQ: {
            BranchIf(Z == 1, op);
            break;
        }
        case InstructionName::BMI: {
            BranchIf(N == 1, op);
            break;
        }
        case InstructionName::BNE: {
            BranchIf(Z == 0, op);
            break;
        }
        case InstructionName::BPL: {
            BranchIf(N == 0, op);
            break;
        }
        case InstructionName::BVC: {
            BranchIf(V == 0, op);
            break;
        }
        case InstructionName::BVS: {
            BranchIf(V == 1, op);
            break;
        }
        case InstructionName::ADC: {
            const auto addr = BuildAddress(op.Addressing);
            const auto M = Memory.GetByteAt(addr);
            Word a = A + M + C;
            C = (a > BYTE_MASK) ? 1 : 0;
//            V = (a >= BYTE_MASK_SIGN) ? 1 : 0;
            V = ((~((A ^ M) & BYTE_MASK_SIGN) & ((A ^ a) & BYTE_MASK_SIGN)) == 0) ? 0 : 1;
            Z = ((a & BYTE_MASK) == 0) ? 1 : 0;
            N = ((a & BYTE_MASK_SIGN) == 0) ? 0 : 1;
            A = a & BYTE_MASK;
            switch (op.Addressing) {
                case AddressingType::AbsoluteX: {
                    if (X > (addr & BYTE_MASK)) ++Ticks;
                    break;
                }
                case AddressingType::AbsoluteY: {
                    if (Y > (addr & BYTE_MASK)) ++Ticks;
                    break;
                }
                case AddressingType::IndirectIndexed: {
                    const auto base = Memory.GetWordAt(Memory.GetByteAt(PC + 1)) + Y;
                    if (Y > (base & BYTE_MASK)) ++Ticks;
                }
                default: break;
            }
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::ASL: {
            if (op.Addressing == AddressingType::Accumulator) {
                C = ((A & BYTE_MASK_SIGN) == 0) ? 0 : 1;
                A <<= 1;
                Z = (A == 0) ? 1 : 0;
                N = ((A & BYTE_MASK_SIGN) == 0) ? 0 : 1;
            } else {
                const auto addr = BuildAddress(op.Addressing);
                auto M = Memory.GetByteAt(addr);
                C = ((M & BYTE_MASK_SIGN) == 0) ? 0 : 1;
                M <<= 1;
                Z = (M == 0) ? 1 : 0;
                N = ((M & BYTE_MASK_SIGN) == 0) ? 0 : 1;
                Memory.SetByteAt(addr, M);
            }
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::AND: {
            const auto addr = BuildAddress(op.Addressing);
            const auto M = Memory.GetByteAt(addr);
            A = A & M;
            Z = A == 0 ? 1 : 0;
            N = (A & 0x80) == 0 ? 0 : 1;
            switch (op.Addressing) {
                case AddressingType::AbsoluteX: {
                    if (X > (addr & BYTE_MASK)) ++Ticks;
                    break;
                }
                case AddressingType::AbsoluteY: {
                    if (Y > (addr & BYTE_MASK)) ++Ticks;
                    break;
                }
                case AddressingType::IndirectIndexed: {
                    const auto base = Memory.GetWordAt(Memory.GetByteAt(PC + 1)) + Y;
                    if (Y > (base & BYTE_MASK)) ++Ticks;
                }
                default: break;
            }
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::BIT: {
            const auto mask = Memory.GetByteAt(BuildAddress(op.Addressing));
            Z = (mask & A) == 0 ? 1 : 0;
            V = (mask & 0x40) == 0 ? 0 : 1;
            N = (mask & 0x80) == 0 ? 0 : 1;
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::CLC: {
            C = 0;
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::CLD: {
            D = 0;
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::CLI: {
            I = 0;
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::CLV: {
            V = 0;
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::SEC: {
            C = 1;
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::SED: {
            D = 1;
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::SEI: {
            I = 1;
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::DEC: {
            const auto addr = BuildAddress(op.Addressing);
            auto M = Memory.GetByteAt(addr);
            Decrement(M);
            Memory.SetByteAt(addr, M);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::DEX: {
            Decrement(X);
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::DEY: {
            Decrement(Y);
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::INC: {
            const auto addr = BuildAddress(op.Addressing);
            auto M = Memory.GetByteAt(addr);
            Increment(M);
            Memory.SetByteAt(addr, M);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::INX: {
            Increment(X);
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::INY: {
            Increment(Y);
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::NOP: { PC += op.Bytes; Ticks += op.Cycles; break; }

        default:
        case InstructionName::UNK:
            break;
    }
}

string Cpu::ToString() const {
    ostringstream value;
    value << "Cpu " << Name << endl
          << "- Registers PC " << setw(10) << PC << endl
          << "            SP " << setw(10) << SP << endl
          << "             A " << setw(10) <<  A << endl
          << "             X " << setw(10) <<  X << endl
          << "             Y " << setw(10) <<  Y << endl
          << "- Flags C " << setw(5) << boolalpha << (C != 0) << endl
          << "        Z " << setw(5) << boolalpha << (Z != 0) << endl
          << "        I " << setw(5) << boolalpha << (I != 0) << endl
          << "        D " << setw(5) << boolalpha << (D != 0) << endl
          << "        B " << setw(5) << boolalpha << (B != 0) << endl
          << "        V " << setw(5) << boolalpha << (V != 0) << endl
          << "        N " << setw(5) << boolalpha << (N != 0) << endl;
    return value.str();
}
