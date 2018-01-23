/*
 * Cpu.h
 *
 *  Created on: 10 Jun 2013
 *      Author: scorder
 */

#ifndef CPU_H_
#define CPU_H_

#include "Types.h"
#include "Mapper.h"

#include <string>
#include <vector>

static const auto OPCODES_COUNT = 0x0100;

namespace Instructions {
enum Name {
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

    UNK,
};
}

typedef int Opsize;
typedef int Optime;

namespace Addressing {
enum Type {
    Implicit,
    Accumulator,
    Immediate,
    ZeroPage,
    ZeroPageX,
    ZeroPageY,
    Relative,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    Indirect,
    IndexedIndirect,
    IndirectIndexed,

    Unknown,
};
}

enum Bits : size_t {
    Car, Zer, Int, Dec, Brk, Unu, Ovf, Neg,
    Left = 7, Right = 0,
};

enum class Interrupt {
    Brk, Irq, Nmi, Rst,
};

class Opcode {
public:
    explicit Opcode(const Instructions::Name &name, const Addressing::Type &addr,
                    const Opsize &bytes, const Optime &cycles)
        : Instruction(name), Addressing(addr), Bytes(bytes),
          Cycles(cycles) {
    }

    Instructions::Name Instruction;
    Addressing::Type Addressing;
    Opsize Bytes;
    Optime Cycles;
};

struct address_t {
    Word Address;
    bool HasCrossedPage;
};

class Cpu {
public:
    Word ReadWordAt(const Word address) const;
    void WriteWordAt(const Word address, const Word value);

    explicit Cpu(std::string name);

    std::string Name;

    Word PC; // Program Counter
    Byte SP; // Stack Pointer
    Byte A;  // Accumulator
    Byte X;  // Index Register X
    Byte Y;  // Index Register Y

    Flag C; // Carry Flag
    Flag Z; // Zero Flag
    Flag I; // Interrupt Disable
    Flag D; // Decimal Mode
    Flag B; // Break Command
    Flag V; // Overflow Flag
    Flag N; // Negative Flag
    const Flag Unused;

    Word StackPage = 0x0100;
    Word VectorRST;
    Word VectorNMI;
    Word VectorIRQ;

    int Ticks;
    int InterruptCycles;

    Opcode Decode(const Byte &byte) const;
    address_t BuildAddress(const Addressing::Type & type) const;
    void Execute(const Opcode &op);//, const std::vector<Byte> &data);

    Mapper Memory;

    std::string ToString() const;

    void Decrement(Byte & value);
    void Increment(Byte & value);
    void Compare(const Byte lhs, const Byte rhs);
    void Transfer(const Byte & from, Byte & to);
    void BranchIf(const bool condition, const Opcode & op);

    void Push(const Byte & value);
    Byte Pull();

    void PushWord(const Word & value);
    Word PullWord();

    void SetStatus(const Byte & status);
    Byte GetStatus();

    void Interrupt(const Flag & isSoft, const Word & returnAddress, const Word & vector);

private:
//    std::vector<Instruction> m_opcodes;
//    std::vector<Opsize> m_opsize;
//    std::vector<Optime> m_optime;
//    std::vector<Addressing> m_opaddr;
    std::vector<Opcode> m_opcodes;
};

#endif /* CPU_H_ */
