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

typedef Byte Flag;
enum class InstructionName {
    ASL,                // Shift
    AND, BIT,           // Bit operations
    CLC, CLD, CLI, CLV, SEC, SED, SEI, // Flags
    ADC, DEC, DEX, DEY, INC, INX, INY, // Arithmetic
    BCC, BCS, BEQ, BMI, BNE, BPL, BVC, BVS, // Branch
    BRK, // Stack
    STX, STY, // Memory
    NOP,
    Unknown,
};
typedef int Opsize;
typedef int Optime;

enum class AddressingType {
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

class Opcode {
public:
    explicit Opcode(const InstructionName &name, const AddressingType &addr,
                    const Opsize &bytes, const Optime &cycles, const bool extra = false)
        : Instruction(name), Addressing(addr), Bytes(bytes),
          Cycles(cycles), ExtraOnPageCrossing(extra) {
    }

    InstructionName Instruction;
    AddressingType Addressing;
    Opsize Bytes;
    Optime Cycles;
    bool ExtraOnPageCrossing;
};

class Cpu {
public:
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

    int Ticks;

    Word BuildAddress(const AddressingType & type) const;
    void Execute(const Opcode &op);//, const std::vector<Byte> &data);

    Mapper Memory;

    std::string ToString() const;

    void Decrement(Byte & value);
    void Increment(Byte & value);
    void BranchIf(const bool condition, const Opcode & op);

private:
//    std::vector<Instruction> m_opcodes;
//    std::vector<Opsize> m_opsize;
//    std::vector<Optime> m_optime;
//    std::vector<Addressing> m_opaddr;
    std::vector<Opcode> m_opcodes;
};

#endif /* CPU_H_ */
