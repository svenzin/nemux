/*
 * Cpu.h
 *
 *  Created on: 10 Jun 2013
 *      Author: scorder
 */

#ifndef CPU_H_
#define CPU_H_

#include "Mapper.h"

#include <string>
#include <vector>

static const auto OPCODES_COUNT = 0x0100;

typedef int Register_8;
typedef int Register_16;
typedef int Flag;
enum class InstructionName {
    BIT,
    CLC,
    CLD,
    CLI,
    CLV,
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
                    const Opsize &bytes, const Optime &cycles)
        : Instruction(name), Addressing(addr), Bytes(bytes), Cycles(cycles) {
    }

    InstructionName Instruction;
    AddressingType Addressing;
    Opsize Bytes;
    Optime Cycles;
};

class Cpu {
public:
    explicit Cpu(std::string name);

    std::string Name;

    Register_16 PC; // Program Counter
    Register_8 SP; // Stack Pointer
    Register_8 A;  // Accumulator
    Register_8 X;  // Index Register X
    Register_8 Y;  // Index Register Y

    Flag C; // Carry Flag
    Flag Z; // Zero Flag
    Flag I; // Interrupt Disable
    Flag D; // Decimal Mode
    Flag B; // Break Command
    Flag V; // Overflow Flag
    Flag N; // Negative Flag

    int Ticks;

    Address BuildAddress(const AddressingType & type) const;
    void Execute(const Opcode &op);//, const std::vector<Byte> &data);

    Mapper Memory;

    std::string ToString() const;

private:
//    std::vector<Instruction> m_opcodes;
//    std::vector<Opsize> m_opsize;
//    std::vector<Optime> m_optime;
//    std::vector<Addressing> m_opaddr;
    std::vector<Opcode> m_opcodes;
};

#endif /* CPU_H_ */
