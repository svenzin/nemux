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
      C {0}, Z {0}, I {0}, D {0}, B {0}, V {0}, N {0}, Unused{1},
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
    m_opcodes[0x3D] = Opcode(InstructionName::AND, AddressingType::AbsoluteX, 3, 4);
    m_opcodes[0x39] = Opcode(InstructionName::AND, AddressingType::AbsoluteY, 3, 4);
    m_opcodes[0x21] = Opcode(InstructionName::AND, AddressingType::IndexedIndirect, 2, 6);
    m_opcodes[0x31] = Opcode(InstructionName::AND, AddressingType::IndirectIndexed, 2, 5);

    m_opcodes[0x24] = Opcode(InstructionName::BIT, AddressingType::ZeroPage, 2, 3);
    m_opcodes[0x2C] = Opcode(InstructionName::BIT, AddressingType::Absolute, 3, 4);

    m_opcodes[0x49] = Opcode(InstructionName::EOR, AddressingType::Immediate, 2, 2);
    m_opcodes[0x45] = Opcode(InstructionName::EOR, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0x55] = Opcode(InstructionName::EOR, AddressingType::ZeroPageX, 2, 4);
    m_opcodes[0x40] = Opcode(InstructionName::EOR, AddressingType::Absolute,  3, 4);
    m_opcodes[0x50] = Opcode(InstructionName::EOR, AddressingType::AbsoluteX, 3, 4);
    m_opcodes[0x59] = Opcode(InstructionName::EOR, AddressingType::AbsoluteY, 3, 4);
    m_opcodes[0x41] = Opcode(InstructionName::EOR, AddressingType::IndexedIndirect, 2, 6);
    m_opcodes[0x51] = Opcode(InstructionName::EOR, AddressingType::IndirectIndexed, 2, 5);

    m_opcodes[0x09] = Opcode(InstructionName::ORA, AddressingType::Immediate, 2, 2);
    m_opcodes[0x05] = Opcode(InstructionName::ORA, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0x15] = Opcode(InstructionName::ORA, AddressingType::ZeroPageX, 2, 4);
    m_opcodes[0x0D] = Opcode(InstructionName::ORA, AddressingType::Absolute,  3, 4);
    m_opcodes[0x1D] = Opcode(InstructionName::ORA, AddressingType::AbsoluteX, 3, 4);
    m_opcodes[0x19] = Opcode(InstructionName::ORA, AddressingType::AbsoluteY, 3, 4);
    m_opcodes[0x01] = Opcode(InstructionName::ORA, AddressingType::IndexedIndirect, 2, 6);
    m_opcodes[0x11] = Opcode(InstructionName::ORA, AddressingType::IndirectIndexed, 2, 5);

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
    m_opcodes[0x7D] = Opcode(InstructionName::ADC, AddressingType::AbsoluteX, 3, 4);
    m_opcodes[0x79] = Opcode(InstructionName::ADC, AddressingType::AbsoluteY, 3, 4);
    m_opcodes[0x61] = Opcode(InstructionName::ADC, AddressingType::IndexedIndirect, 2, 6);
    m_opcodes[0x71] = Opcode(InstructionName::ADC, AddressingType::IndirectIndexed, 2, 5);

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

    // Comparisons
    m_opcodes[0xC9] = Opcode(InstructionName::CMP, AddressingType::Immediate, 2, 2);
    m_opcodes[0xC5] = Opcode(InstructionName::CMP, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0xD5] = Opcode(InstructionName::CMP, AddressingType::ZeroPageX, 2, 4);
    m_opcodes[0xCD] = Opcode(InstructionName::CMP, AddressingType::Absolute,  3, 4);
    m_opcodes[0xDD] = Opcode(InstructionName::CMP, AddressingType::AbsoluteX, 3, 4);
    m_opcodes[0xD9] = Opcode(InstructionName::CMP, AddressingType::AbsoluteY, 3, 4);
    m_opcodes[0xC1] = Opcode(InstructionName::CMP, AddressingType::IndexedIndirect, 2, 6);
    m_opcodes[0xD1] = Opcode(InstructionName::CMP, AddressingType::IndirectIndexed, 2, 5);

    m_opcodes[0xE0] = Opcode(InstructionName::CPX, AddressingType::Immediate, 2, 2);
    m_opcodes[0xE4] = Opcode(InstructionName::CPX, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0xEC] = Opcode(InstructionName::CPX, AddressingType::Absolute,  3, 4);

    m_opcodes[0xC0] = Opcode(InstructionName::CPY, AddressingType::Immediate, 2, 2);
    m_opcodes[0xC4] = Opcode(InstructionName::CPY, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0xCC] = Opcode(InstructionName::CPY, AddressingType::Absolute,  3, 4);

    // Stack
//    m_opcodes[0x00] = Opcode(InstructionName::BRK, AddressingType::Implicit, 1, 7);
    m_opcodes[0x48] = Opcode(InstructionName::PHA, AddressingType::Implicit, 1, 3);

    m_opcodes[0x68] = Opcode(InstructionName::PLA, AddressingType::Implicit, 1, 4);

    m_opcodes[0x08] = Opcode(InstructionName::PHP, AddressingType::Implicit, 1, 3);

    m_opcodes[0x28] = Opcode(InstructionName::PLP, AddressingType::Implicit, 1, 4);

    // Memory
    m_opcodes[0xA2] = Opcode(InstructionName::LDX, AddressingType::Immediate, 2, 2);
    m_opcodes[0xA6] = Opcode(InstructionName::LDX, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0xB6] = Opcode(InstructionName::LDX, AddressingType::ZeroPageY, 2, 4);
    m_opcodes[0xAE] = Opcode(InstructionName::LDX, AddressingType::Absolute,  3, 4);
    m_opcodes[0xBE] = Opcode(InstructionName::LDX, AddressingType::AbsoluteY, 3, 4);

    m_opcodes[0xA0] = Opcode(InstructionName::LDY, AddressingType::Immediate, 2, 2);
    m_opcodes[0xA4] = Opcode(InstructionName::LDY, AddressingType::ZeroPage,  2, 3);
    m_opcodes[0xB4] = Opcode(InstructionName::LDY, AddressingType::ZeroPageY, 2, 4);
    m_opcodes[0xAC] = Opcode(InstructionName::LDY, AddressingType::Absolute,  3, 4);
    m_opcodes[0xBC] = Opcode(InstructionName::LDY, AddressingType::AbsoluteY, 3, 4);

    m_opcodes[0xA9] = Opcode(InstructionName::LDA, AddressingType::Immediate, 2, 2);
    m_opcodes[0xA5] = Opcode(InstructionName::LDA, AddressingType::ZeroPage, 2, 3);
    m_opcodes[0xB5] = Opcode(InstructionName::LDA, AddressingType::ZeroPageX, 2, 4);
    m_opcodes[0xAD] = Opcode(InstructionName::LDA, AddressingType::Absolute, 3, 4);
    m_opcodes[0xBD] = Opcode(InstructionName::LDA, AddressingType::AbsoluteX, 3, 4);
    m_opcodes[0xD9] = Opcode(InstructionName::LDA, AddressingType::AbsoluteY, 3, 4);
    m_opcodes[0xA1] = Opcode(InstructionName::LDA, AddressingType::IndexedIndirect, 2, 6);
    m_opcodes[0xB1] = Opcode(InstructionName::LDA, AddressingType::IndirectIndexed, 2, 5);

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

address_t Cpu::BuildAddress(const AddressingType & type) const {
    switch (type) {
        case AddressingType::Immediate: {
            return { PC + 1, false };
        }
        case AddressingType::ZeroPage: {
            return { Memory.GetByteAt(PC + 1), false };
        }
        case AddressingType::ZeroPageX: {
            return { Memory.GetByteAt(PC + 1) + X, false };
        }
        case AddressingType::ZeroPageY: {
            return { Memory.GetByteAt(PC + 1) + Y, false };
        }
        case AddressingType::Absolute: {
            return { Memory.GetWordAt(PC + 1), false };
        }
        case AddressingType::AbsoluteX: {
            const auto address = Memory.GetWordAt(PC + 1) + X;
            return { address, (X > (address & BYTE_MASK)) };
        }
        case AddressingType::AbsoluteY: {
            const auto address = Memory.GetWordAt(PC + 1) + Y;
            return { address, (Y > (address & BYTE_MASK)) };
        }
        case AddressingType::IndexedIndirect: {
            return { Memory.GetWordAt(Memory.GetByteAt(PC + 1) + X), false };
        }
        case AddressingType::IndirectIndexed: {
            const auto base = Memory.GetWordAt(Memory.GetByteAt(PC + 1)) + Y;
            return { Memory.GetWordAt(base), (Y > (base & BYTE_MASK)) };
        }
        default: return { -1, false };
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
    N = ((to & BYTE_MASK_SIGN) == 0) ? 0 : 1;
}
void Cpu::Compare(const Byte lhs, const Byte rhs) {
    const auto r = lhs - rhs;
    C = (r >= 0) ? 1 : 0;
    Z = (r == 0) ? 1 : 0;
    N = (r  < 0) ? 1 : 0;
}
void Cpu::BranchIf(const bool condition, const Opcode & op) {
    const auto basePC = PC;
    const auto M = Memory.GetByteAt(BuildAddress(AddressingType::Immediate).Address);
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
        case InstructionName::PLP: {
            ++SP;
            const auto status = Memory.GetByteAt(StackPage + SP);
            N = (status >> 7) & 0x01;
            V = (status >> 6) & 0x01;
            B = (status >> 4) & 0x01;
            D = (status >> 3) & 0x01;
            I = (status >> 2) & 0x01;
            Z = (status >> 1) & 0x01;
            C = (status >> 0) & 0x01;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::PHP: {
            const auto status = N * 0x80 + V * 0x40 + Unused * 0x20 + B * 0x10 +
                                D * 0x08 + I * 0x04 + Z * 0x02 + C * 0x01;
            Memory.SetByteAt(StackPage + SP, status);
            --SP;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::PHA: {
            Memory.SetByteAt(StackPage + SP, A);
            --SP;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::PLA: {
            ++SP;
            Transfer(Memory.GetByteAt(StackPage + SP), A);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::LDA: {
            const auto a = BuildAddress(op.Addressing);
            Transfer(Memory.GetByteAt(a.Address), A);
            if (a.HasCrossedPage) ++Ticks;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::LDX: {
            const auto a = BuildAddress(op.Addressing);
            Transfer(Memory.GetByteAt(a.Address), X);
            if (a.HasCrossedPage) ++Ticks;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::LDY: {
            const auto a = BuildAddress(op.Addressing);
            Transfer(Memory.GetByteAt(a.Address), Y);
            if (a.HasCrossedPage) ++Ticks;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::EOR: {
            const auto a = BuildAddress(op.Addressing);
            Transfer(A ^ Memory.GetByteAt(a.Address), A);
            if (a.HasCrossedPage) ++Ticks;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::ORA: {
            const auto a = BuildAddress(op.Addressing);
            Transfer(A | Memory.GetByteAt(a.Address), A);
            if (a.HasCrossedPage) ++Ticks;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::CMP: {
            const auto a = BuildAddress(op.Addressing);
            Compare(A, Memory.GetByteAt(a.Address));
            if (a.HasCrossedPage) ++Ticks;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::CPX: {
            Compare(X, Memory.GetByteAt(BuildAddress(op.Addressing).Address));
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::CPY: {
            Compare(Y, Memory.GetByteAt(BuildAddress(op.Addressing).Address));
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
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
            // TXS does not change the flags
            SP = X;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::TYA: {
            Transfer(Y, A);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::STA: {
            Memory.SetByteAt(BuildAddress(op.Addressing).Address, A);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::STX: {
            Memory.SetByteAt(BuildAddress(op.Addressing).Address, X);
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::STY: {
            Memory.SetByteAt(BuildAddress(op.Addressing).Address, Y);
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
            const auto aa = BuildAddress(op.Addressing);
            const auto M = Memory.GetByteAt(aa.Address);
            Word a = A + M + C;
            C = (a > BYTE_MASK) ? 1 : 0;
//            V = (a >= BYTE_MASK_SIGN) ? 1 : 0;
            V = ((~((A ^ M) & BYTE_MASK_SIGN) & ((A ^ a) & BYTE_MASK_SIGN)) == 0) ? 0 : 1;
            Transfer(a & BYTE_MASK, A);
            if (aa.HasCrossedPage) ++Ticks;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::ASL: {
            if (op.Addressing == AddressingType::Accumulator) {
                C = ((A & BYTE_MASK_SIGN) == 0) ? 0 : 1;
                Transfer(A << 1, A);
            } else {
                const auto address = BuildAddress(op.Addressing).Address;
                auto M = Memory.GetByteAt(address);
                C = ((M & BYTE_MASK_SIGN) == 0) ? 0 : 1;
                Transfer(M << 1, M);
                Memory.SetByteAt(address, M);
            }
            PC += op.Bytes; Ticks += op.Cycles; break;
        }
        case InstructionName::AND: {
            const auto a = BuildAddress(op.Addressing);
            const auto M = Memory.GetByteAt(a.Address);
            A = A & M;
            Z = A == 0 ? 1 : 0;
            N = (A & 0x80) == 0 ? 0 : 1;
            if (a.HasCrossedPage) ++Ticks;
            PC += op.Bytes; Ticks += op.Cycles;
            break;
        }
        case InstructionName::BIT: {
            const auto mask = Memory.GetByteAt(BuildAddress(op.Addressing).Address);
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
            const auto address = BuildAddress(op.Addressing).Address;
            auto M = Memory.GetByteAt(address);
            Decrement(M);
            Memory.SetByteAt(address, M);
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
            const auto address = BuildAddress(op.Addressing).Address;
            auto M = Memory.GetByteAt(address);
            Increment(M);
            Memory.SetByteAt(address, M);
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
