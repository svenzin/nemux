/*
 * Cpu-test.cpp
 *
 *  Created on: 19 Jun 2013
 *      Author: scorder
 */

#include "gtest/gtest.h"

#include "Cpu.h"

#include <vector>
#include <map>
#include <array>
#include <functional>

using namespace std;

class CpuTest : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTest() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

    function<void (Byte)> Setter(Byte & a) {
        return [&] (Byte value) { a = value; };
    }
    function<void (Byte)> Setter(Word a) {
        return [=] (Byte value) { cpu.Memory.SetByteAt(a, value); };
    }
    function<Byte ()> Getter(Byte & b) {
        return [&] () { return b; };
    }
    function<Byte ()> Getter(Word a) {
        return [=] () { return cpu.Memory.GetByteAt(a); };
    }
};

//
//TEST_F(CpuTest, Opcode_Instruction) {
//    typedef InstructionName i;
//    std::array<InstructionName, 0x100> opcodes {
///*             x0      x1      x2      x3      x4      x5      x6      x7      x8      x9      xA      xB      xC      xD      xE      xF */
///* 0x */    i::BRK, i::ORA, i::UNK, i::UNK, i::UNK, i::ORA, i::ASL, i::UNK, i::PHP, i::ORA, i::ASL, i::UNK, i::UNK, i::ORA, i::ASL, i::UNK,
///* 1x */    i::BPL, i::ORA, i::UNK, i::UNK, i::UNK, i::ORA, i::ASL, i::UNK, i::CLC, i::ORA, i::UNK, i::UNK, i::UNK, i::ORA, i::ASL, i::UNK,
///* 2x */    i::JSR, i::AND, i::UNK, i::UNK, i::BIT, i::AND, i::ROL, i::UNK, i::PLP, i::AND, i::ROL, i::UNK, i::BIT, i::AND, i::ROL, i::UNK,
///* 3x */    i::BMI, i::AND, i::UNK, i::UNK, i::UNK, i::AND, i::ROL, i::UNK, i::SEC, i::AND, i::UNK, i::UNK, i::UNK, i::AND, i::ROL, i::UNK,
///* 4x */    i::RTI, i::EOR, i::UNK, i::UNK, i::UNK, i::EOR, i::LSR, i::UNK, i::PHA, i::EOR, i::LSR, i::UNK, i::JMP, i::EOR, i::LSR, i::UNK,
///* 5x */    i::BVC, i::EOR, i::UNK, i::UNK, i::UNK, i::EOR, i::LSR, i::UNK, i::CLI, i::EOR, i::UNK, i::UNK, i::UNK, i::EOR, i::LSR, i::UNK,
///* 6x */    i::RTS, i::ADC, i::UNK, i::UNK, i::UNK, i::ADC, i::ROR, i::UNK, i::PLA, i::ADC, i::ROR, i::UNK, i::JMP, i::ADC, i::ROR, i::UNK,
///* 7x */    i::BVS, i::ADC, i::UNK, i::UNK, i::UNK, i::ADC, i::ROR, i::UNK, i::SEI, i::ADC, i::UNK, i::UNK, i::UNK, i::ADC, i::ROR, i::UNK,
///* 8x */    i::UNK, i::STA, i::UNK, i::UNK, i::STY, i::STA, i::STX, i::UNK, i::DEY, i::UNK, i::TXA, i::UNK, i::STY, i::STA, i::STX, i::UNK,
///* 9x */    i::BCC, i::STA, i::UNK, i::UNK, i::STY, i::STA, i::STX, i::UNK, i::TYA, i::STA, i::TXS, i::UNK, i::UNK, i::STA, i::UNK, i::UNK,
///* Ax */    i::LDY, i::LDA, i::LDX, i::UNK, i::LDY, i::LDA, i::LDX, i::UNK, i::TAY, i::LDA, i::TAX, i::UNK, i::LDY, i::LDA, i::LDX, i::UNK,
///* Bx */    i::BCS, i::LDA, i::UNK, i::UNK, i::LDY, i::LDA, i::LDX, i::UNK, i::CLV, i::LDA, i::TSX, i::UNK, i::LDY, i::LDA, i::LDX, i::UNK,
///* Cx */    i::CPY, i::CMP, i::UNK, i::UNK, i::CPY, i::CMP, i::DEC, i::UNK, i::INY, i::CMP, i::DEX, i::UNK, i::CPY, i::CMP, i::DEC, i::UNK,
///* Dx */    i::BNE, i::CMP, i::UNK, i::UNK, i::UNK, i::CMP, i::DEC, i::UNK, i::CLD, i::CMP, i::UNK, i::UNK, i::UNK, i::CMP, i::DEC, i::UNK,
///* Ex */    i::CPX, i::SBC, i::UNK, i::UNK, i::CPX, i::SBC, i::INC, i::UNK, i::INX, i::SBC, i::NOP, i::UNK, i::CPX, i::SBC, i::INC, i::UNK,
///* Fx */    i::BEQ, i::SBC, i::UNK, i::UNK, i::UNK, i::SBC, i::INC, i::UNK, i::SED, i::SBC, i::UNK, i::UNK, i::UNK, i::SBC, i::INC, i::UNK,
//    };
//}
//TEST_F(CpuTest, Opcode_Addressing) {
//    const auto IMP = AddressingType::Implicit;
//    const auto ACC = AddressingType::Accumulator;
//    const auto IMM = AddressingType::Immediate;
//    const auto ZPG = AddressingType::ZeroPage;
//    const auto ZPX = AddressingType::ZeroPageX;
//    const auto ZPY = AddressingType::ZeroPageY;
//    const auto REL = AddressingType::Relative;
//    const auto ABS = AddressingType::Absolute;
//    const auto ABX = AddressingType::AbsoluteX;
//    const auto ABY = AddressingType::AbsoluteY;
//    const auto IND = AddressingType::Indirect;
//    const auto IDX = AddressingType::IndexedIndirect;
//    const auto IDY = AddressingType::IndirectIndexed;
//    const auto UNK = AddressingType::Unknown;
//
//    std::array<AddressingType, 0x100> opcodes {
///*           x0   x1   x2   x3   x4   x5   x6   x7   x8   x9   xA   xB   xC   xD   xE   xF */
///* 0x */    IMP, IDX, UNK, UNK, UNK, ZPG, ZPG, UNK, IMP, IMM, ACC, UNK, UNK, ABS, ABS, UNK,
///* 1x */    IMP, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
///* 2x */    IMP, IDX, UNK, UNK, ZPG, ZPG, ZPG, UNK, IMP, IMM, ACC, UNK, ABS, ABS, ABS, UNK,
///* 3x */    IMP, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
///* 4x */    IMP, IDX, UNK, UNK, UNK, ZPG, ZPG, UNK, IMP, IMM, ACC, UNK, ABS, ABS, ABS, UNK,
///* 5x */    IMP, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
///* 6x */    IMP, IDX, UNK, UNK, UNK, ZPG, ZPG, UNK, IMP, IMM, ACC, UNK, IND, ABS, ABS, UNK,
///* 7x */    IMP, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
///* 8x */    UNK, IDX, UNK, UNK, ZPG, ZPG, ZPG, UNK, IMP, UNK, IMP, UNK, ABS, ABS, ABS, UNK,
///* 9x */    IMP, IDY, UNK, UNK, ZPX, ZPX, ZPY, UNK, IMP, ABY, IMP, UNK, UNK, ABX, UNK, UNK,
///* Ax */    IMM, IDX, IMM, UNK, ZPG, ZPG, ZPG, UNK, IMP, IMM, IMP, UNK, ABS, ABS, ABS, UNK,
///* Bx */    IMP, IDY, UNK, UNK, ZPX, ZPX, ZPY, UNK, IMP, ABY, IMP, UNK, ABX, ABX, ABY, UNK,
///* Cx */    IMM, IDX, UNK, UNK, ZPG, ZPG, ZPG, UNK, IMP, IMM, IMP, UNK, ABS, ABS, ABS, UNK,
///* Dx */    IMP, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
///* Ex */    IMM, IDX, UNK, UNK, ZPG, ZPG, ZPG, UNK, IMP, IMM, IMP, UNK, ABS, ABS, ABS, UNK,
///* Fx */    IMP, IDY, UNK, UNK, UNK, ZPX, ZPX, UNK, IMP, ABY, UNK, UNK, UNK, ABX, ABX, UNK,
//    };
//}
