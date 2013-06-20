/*
 * Cpu.cpp
 *
 *  Created on: 10 Jun 2013
 *      Author: scorder
 */
#include "Cpu.h"

#include <iomanip>
#include <sstream>

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

    // Bit mask
    m_opcodes[0x24] = Opcode(InstructionName::BIT, AddressingType::ZeroPage, 2, 3);
    m_opcodes[0x2C] = Opcode(InstructionName::BIT, AddressingType::Absolute, 3, 4);

    // Clear flags
    m_opcodes[0x18] = Opcode(InstructionName::CLC, AddressingType::Implicit, 1, 2);

    m_opcodes[0xD8] = Opcode(InstructionName::CLD, AddressingType::Implicit, 1, 2);

    m_opcodes[0x58] = Opcode(InstructionName::CLI, AddressingType::Implicit, 1, 2);

    m_opcodes[0xB8] = Opcode(InstructionName::CLV, AddressingType::Implicit, 1, 2);

    // Decrement
    m_opcodes[0xCA] = Opcode(InstructionName::DEC, AddressingType::ZeroPage,  2, 5);
    m_opcodes[0xCA] = Opcode(InstructionName::DEC, AddressingType::ZeroPageX, 2, 6);
    m_opcodes[0xCA] = Opcode(InstructionName::DEC, AddressingType::Absolute,  3, 6);
    m_opcodes[0xCA] = Opcode(InstructionName::DEC, AddressingType::AbsoluteX, 3, 7);

    m_opcodes[0xCA] = Opcode(InstructionName::DEX, AddressingType::Implicit, 1, 2);

    m_opcodes[0x88] = Opcode(InstructionName::DEY, AddressingType::Implicit, 1, 2);

    // Nop
    m_opcodes[0xEA] = Opcode(InstructionName::NOP, AddressingType::Implicit, 1, 2);
}

Address Cpu::BuildAddress(const AddressingType & type) const {
    switch (type) {
        case AddressingType::Implicit: return Address{0};
        case AddressingType::ZeroPage: return Address{Memory.GetByteAt(PC + 1)};
        case AddressingType::Absolute: return Address{Memory.GetWordAt(PC + 1)};
        default: return Address{-1};
    }
}

void Cpu::Execute(const Opcode &op) {//, const std::vector<Byte> &data) {
    switch(op.Instruction) {
        case InstructionName::BIT: {
            auto mask = Memory.GetByteAt(BuildAddress(op.Addressing));
            Z = (mask & A) == 0 ? 1 : 0;
            V = (mask & 0x40) == 0 ? 0 : 1;
            N = (mask & 0x80) == 0 ? 0 : 1;
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::CLC: { C = 0; PC += op.Bytes; Ticks += op.Cycles; break; }
        case InstructionName::CLD: { D = 0; PC += op.Bytes; Ticks += op.Cycles; break; }
        case InstructionName::CLI: { I = 0; PC += op.Bytes; Ticks += op.Cycles; break; }
        case InstructionName::CLV: { V = 0; PC += op.Bytes; Ticks += op.Cycles; break; }
        case InstructionName::DEC: {
            //auto address = BuildAddress(op.)
            X = ((X + 0x180 - 1) % 0x100) - 0x80; Z = (X == 0) ? 1 : 0; N = (X < 0) ? 1 : 0; break; }
        case InstructionName::DEX: {
            X = ((X + 0x180 - 1) % 0x100) - 0x80;
            Z = (X == 0) ? 1 : 0;
            N = (X < 0) ? 1 : 0;
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::DEY: {
            Y = ((Y + 0x180 - 1) % 0x100) - 0x80;
            Z = (Y == 0) ? 1 : 0;
            N = (Y < 0) ? 1 : 0;
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
