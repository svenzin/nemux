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
        Opcode(InstructionName::Unknown, AddressingType::Unknown, 0, 0)
    );

    // Shift
    m_opcodes[0x0A] = Opcode(InstructionName::ASL, AddressingType::Accumulator, 1, 2);
    m_opcodes[0x06] = Opcode(InstructionName::ASL, AddressingType::ZeroPage,    2, 5);
    m_opcodes[0x16] = Opcode(InstructionName::ASL, AddressingType::ZeroPageX,   2, 6);
    m_opcodes[0x0E] = Opcode(InstructionName::ASL, AddressingType::Absolute,    3, 6);
    m_opcodes[0x1E] = Opcode(InstructionName::ASL, AddressingType::AbsoluteX,   3, 7);

    // Bit mask
    m_opcodes[0x24] = Opcode(InstructionName::BIT, AddressingType::ZeroPage, 2, 3);
    m_opcodes[0x2C] = Opcode(InstructionName::BIT, AddressingType::Absolute, 3, 4);

    // Clear flags
    m_opcodes[0x18] = Opcode(InstructionName::CLC, AddressingType::Implicit, 1, 2);

    m_opcodes[0xD8] = Opcode(InstructionName::CLD, AddressingType::Implicit, 1, 2);

    m_opcodes[0x58] = Opcode(InstructionName::CLI, AddressingType::Implicit, 1, 2);

    m_opcodes[0xB8] = Opcode(InstructionName::CLV, AddressingType::Implicit, 1, 2);

    // Decrement
    m_opcodes[0xC6] = Opcode(InstructionName::DEC, AddressingType::ZeroPage,  2, 5);
    m_opcodes[0xD6] = Opcode(InstructionName::DEC, AddressingType::ZeroPageX, 2, 6);
    m_opcodes[0xCE] = Opcode(InstructionName::DEC, AddressingType::Absolute,  3, 6);
    m_opcodes[0xDE] = Opcode(InstructionName::DEC, AddressingType::AbsoluteX, 3, 7);

    m_opcodes[0xCA] = Opcode(InstructionName::DEX, AddressingType::Implicit, 1, 2);

    m_opcodes[0x88] = Opcode(InstructionName::DEY, AddressingType::Implicit, 1, 2);

    // Nop
    m_opcodes[0xEA] = Opcode(InstructionName::NOP, AddressingType::Implicit, 1, 2);
}

Word Cpu::BuildAddress(const AddressingType & type) const {
    switch (type) {
        case AddressingType::Implicit:  return 0;
        case AddressingType::ZeroPage:  return Memory.GetByteAt(PC + 1);
        case AddressingType::ZeroPageX: return Memory.GetByteAt(PC + 1) + X;
        case AddressingType::Absolute:  return Memory.GetWordAt(PC + 1);
        case AddressingType::AbsoluteX: return Memory.GetWordAt(PC + 1) + X;
        default: return -1;
    }
}

inline void Decrement(Byte & value, Flag & Z, Flag & N) {
    value = (value - 1) & BYTE_MASK;//((value + 0x180 - 1) % 0x100) - 0x80;
    Z = (value == 0) ? 1 : 0;
    N = ((value & BYTE_MASK_SIGN) == 0) ? 0 : 1;
}
void Cpu::Execute(const Opcode &op) {//, const std::vector<Byte> &data) {
    switch(op.Instruction) {
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
        case InstructionName::DEC: {
            const auto addr = BuildAddress(op.Addressing);
            auto M = Memory.GetByteAt(addr);
            Decrement(M, Z, N);
            Memory.SetByteAt(addr, M);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::DEX: {
            Decrement(X, Z, N);
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::DEY: {
            Decrement(Y, Z, N);
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::NOP: { PC += op.Bytes; Ticks += op.Cycles; break; }

        default:
        case InstructionName::Unknown:
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
