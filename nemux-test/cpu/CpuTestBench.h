#pragma once

#include "cpu/BaseCpu.h"
#include "cpu/InstructionSet_6502.h"
#include "MemoryMap.h"

#include <functional>

using is6502 = InstructionSet_6502;
using AMode = InstructionSet_6502::AddressingMode;

static constexpr Word ORIGIN{ 10 };

class CpuTestBench {
    BaseCpu* cpu;
    MemoryMap* memory;
    Word current;
    
public:
    Word PC;
    Byte S;
    size_t Ticks;
    is6502::Instruction op;

    std::optional<Word> target;
    std::function<Byte ()> get_target;
    std::function<void (Byte)> set_target;

    CpuTestBench()
    : cpu{}
    , memory{}
    , target{}
    , get_target{}
    , set_target{}
    {
        origin(ORIGIN);
    }

    void initialize(MemoryMap* _memory, BaseCpu* _cpu) {
        memory = _memory;
        cpu = _cpu;
    }

    CpuTestBench& origin(Word address) {
        PC = address;
        return at(address);
    }

    CpuTestBench& at(Word address) {
        current = address;
        return *this;
    }

    CpuTestBench& db(Byte b) {
        memory->SetByteAt(current, b);
        ++current;
        return *this;
    }

    CpuTestBench& dw(Word w) {
        db(LO(w));
        return db(HI(w));
    }

    CpuTestBench& encode(is6502::OpName name, is6502::AddressingMode mode) {
        const auto opcode{ is6502::Encode(name, mode) };
        op = is6502::Decode(opcode);
        return db(opcode);
    }

    void start() {
        // Set initial CPU state
        cpu->PC = PC;
        // Get CPU ticks at the beginning of the test
        S = cpu->S;
        Ticks = cpu->GetTicks();
    }

    auto initialize_memory_target(Word address) {
        target = address;
        get_target = [this] { return memory->GetByteAt(*target); };
        set_target = [this](Byte value) { memory->SetByteAt(*target, value); };
    }

    auto initialize_register_target(Byte& r) {
        target = std::nullopt;
        get_target = [&r] { return r; };
        set_target = [&r](Byte value) { r = value; };
    }

    using addressing_mode_setup = CpuTestBench& (CpuTestBench::*)(is6502::OpName op);
    
    CpuTestBench& accumulator(is6502::OpName op) {
        initialize_register_target(cpu->A);
        return encode(op, AMode::ACC);
    }
    
    addressing_mode_setup implicit_target(Byte& r) {
        initialize_register_target(r);
        return &CpuTestBench::implicit;
    }
    
    CpuTestBench& implicit(is6502::OpName op) {
        return encode(op, AMode::IMP);
    }
    
    CpuTestBench& immediate(is6502::OpName op) {
        initialize_memory_target(current + 1);
        return encode(op, AMode::IMM);
    }

    CpuTestBench& zeropage(is6502::OpName op) {
        initialize_memory_target(0x0020);
        return encode(op, AMode::ZPG)
            .db(0x20);
    }

    CpuTestBench& zeropage_x(is6502::OpName op) {
        initialize_memory_target(0x0028);
        cpu->X = 0x08;
        return encode(op, AMode::ZPX)
            .db(0x20);
    }

    CpuTestBench& zeropage_x_wraparound(is6502::OpName op) {
        initialize_memory_target(0x0000);
        cpu->X = 0x10;
        return encode(op, AMode::ZPX)
            .db(0xF0);
    }

    CpuTestBench& zeropage_y(is6502::OpName op) {
        initialize_memory_target(0x0028);
        cpu->Y = 0x08;
        return encode(op, AMode::ZPY)
            .db(0x20);
    }

    CpuTestBench& zeropage_y_wraparound(is6502::OpName op) {
        initialize_memory_target(0x0000);
        cpu->Y = 0x10;
        return encode(op, AMode::ZPY)
            .db(0xF0);
    }

    CpuTestBench& absolute(is6502::OpName op) {
        initialize_memory_target(0x0120);
        return encode(op, AMode::ABS)
            .dw(0x0120);
    }

    CpuTestBench& absolute_x(is6502::OpName op) {
        initialize_memory_target(0x0128);
        cpu->X = 0x08;
        return encode(op, AMode::ABX)
            .dw(0x0120);
    }

    CpuTestBench& absolute_x_crossing_page(is6502::OpName op) {
        initialize_memory_target(0x0210);
        cpu->X = 0xF0;
        return encode(op, AMode::ABX)
            .dw(0x0120);
    }

    CpuTestBench& absolute_y(is6502::OpName op) {
        initialize_memory_target(0x0128);
        cpu->Y = 0x08;
        return encode(op, AMode::ABY)
            .dw(0x0120);
    }

    CpuTestBench& absolute_y_crossing_page(is6502::OpName op) {
        initialize_memory_target(0x0210);
        cpu->Y = 0xF0;
        return encode(op, AMode::ABY)
            .dw(0x0120);
    }

    CpuTestBench& indirect_x(is6502::OpName op) {
        initialize_memory_target(0x0120);
        cpu->X = 0x08;
        return encode(op, AMode::IDX)
            .db(0x20)
            .at(0x0028).dw(0x0120);
    }

    CpuTestBench& indirect_x_wraparound(is6502::OpName op) {
        initialize_memory_target(0x0120);
        cpu->X = 0x0F;
        return encode(op, AMode::IDX)
            .db(0xF0)
            .at(0x00FF).db(0x20)
            .at(0x0000).db(0x01);
    }

    CpuTestBench& indirect_y(is6502::OpName op) {
        initialize_memory_target(0x0128);
        cpu->Y = 0x08;
        return encode(op, AMode::IDY)
            .db(0x20)
            .at(0x0020).dw(0x0120);
    }

    CpuTestBench& indirect_y_crossing_page(is6502::OpName op) {
        initialize_memory_target(0x0210);
        cpu->Y = 0xF0;
        return encode(op, AMode::IDY)
            .db(0x20)
            .at(0x0020).dw(0x0120);
    }

    CpuTestBench& indirect_y_crossing_word_size(is6502::OpName op) {
        initialize_memory_target(0x000F);
        cpu->Y = 0x10;
        return encode(op, AMode::IDY)
            .db(0x20)
            .at(0x0020).dw(0xFFFF);
    }

    CpuTestBench& indirect_y_base_from_zeropage(is6502::OpName op) {
        initialize_memory_target(0x0130);
        cpu->Y = 0x10;
        return encode(op, AMode::IDY)
            .db(0xFF)
            .at(0x00FF).db(0x20)
            .at(0x0000).db(0x01);
    }

    // expectations
    Word expected_PC() const { return PC + op.Bytes; }
    size_t expected_ticks(int extra) const { return Ticks + op.Cycles + extra; }
};
