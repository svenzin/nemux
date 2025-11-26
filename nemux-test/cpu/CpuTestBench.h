#pragma once

#include "MemoryMap.h"

#include "Cpu.h"
#include "Ricoh_RP2A03.h"
#include "cpu/RP2A03.h"

#include <functional>
#include <numeric>

using is6502 = InstructionSet_6502;
using AMode = InstructionSet_6502::AddressingMode;

static constexpr Word ORIGIN{ 10 };

class CpuTestBench {
    friend class CpuBaseTest;

    static constexpr size_t MEMORY_SIZE{ 0x10000 };

    using CpuT = RP2A03; //Ricoh_RP2A03; //Cpu;
    using MemoryMapT = MemoryCallbacks<MEMORY_SIZE>;

    static Byte InvalidGetter()     { throw std::runtime_error{ "memory get at unexpected address" }; }
    static void InvalidSetter(Byte) { throw std::runtime_error{ "memory set at unexpected address" }; }

    auto Getter(Word address) {
        return [this, address]() { return block.GetByteAt(address); };
    }

    auto Setter(Word address) {
        return [this, address](Byte value) { block.SetByteAt(address, value); };
    }

    MemoryBlock<MEMORY_SIZE> block;
    MemoryMapT memory;
    CpuT cpu;
    
    Word current;
    
public:
    Word PC;
    Byte S;
    size_t Ticks;

    std::vector<is6502::Instruction> instructions;
    std::optional<Word> target_address;
    std::function<Byte ()> get_target;
    std::function<void (Byte)> set_target;

    CpuTestBench()
    : memory{ InvalidGetter, InvalidSetter }
    , cpu{ "6502", &memory }
    , target_address{}
    , get_target{}
    , set_target{}
    {
        origin(ORIGIN);
    }

    CpuTestBench& origin(Word address) {
        PC = address;
        return reset();
    }

    CpuTestBench& reset() {
        memory.Fill(InvalidGetter, InvalidSetter);

        instructions.clear();
        return at(PC);
    }

    CpuTestBench& at(Word address) {
        current = address;
        return *this;
    }

    CpuTestBench& _r(Byte b) {
        Setter(current)(b);
        memory.SetGetterAt(current, Getter(current));
        return *this;
    }
    CpuTestBench& _w() {
        memory.SetSetterAt(current, Setter(current));
        return *this;
    }
    CpuTestBench& _rw(Byte b) {
        return _r(b)._w();
    }

    CpuTestBench& db(Byte b) {
        return db_r(b);
    }

    CpuTestBench& db_r(Byte b) {
        _r(b);
        ++current;
        return *this;
    }

    CpuTestBench& db_w() {
        _w();
        ++current;
        return *this;
    }

    CpuTestBench& db_rw(Byte b) {
        _rw(b);
        ++current;
        return *this;
    }

    CpuTestBench& dw(Word w) {
        db(LO(w));
        return db(HI(w));
    }

    CpuTestBench& stack_at(Byte stack) {
        current = cpu.StackPage + stack;
        return *this;
    }

    CpuTestBench& will_push(int count) {
        while (count-- > 0) {
            _rw(0xFF);
            --current;
        }
        return *this;
    }

    CpuTestBench& will_pull(std::vector<Byte>&& data) {
        if (!data.empty()) {
            db(0xFF); // dummy read during pre-increment
            for (int i = 0; i < data.size(); ++i) {
                db(data[i]);
            }
        }
        return *this;
    }

    CpuTestBench& prepare_interrupt(Word vector, Word target, Byte stack) {
        return at(vector).dw(target)
               .stack_at(stack).will_push(3);
    }

    CpuTestBench& RTI(Byte status, Word target, Byte stack) {
        return implicit(is6502::OpName::RTI)
               .stack_at(stack)
               .will_pull({ status, LO(target), HI(target) });
    }

    CpuTestBench& encode(is6502::OpName name, is6502::AddressingMode mode) {
        const auto opcode{ is6502::Encode(name, mode) };
        instructions.push_back(is6502::Decode(opcode));
        return db(opcode);
    }

    void start() {
        // Set initial CPU state
        cpu.PC = PC;
        // Get CPU ticks at the beginning of the test
        S = cpu.S;
        Ticks = cpu.GetTicks();
    }

    auto initialize_memory_target(Word address) {
        target_address = address;
        get_target = Getter(address);
        set_target = Setter(address);

        memory.SetGetterAt(address, get_target);
        memory.SetSetterAt(address, set_target);
    }

    auto initialize_register_target(Byte& r) {
        target_address = std::nullopt;
        get_target = [&r] { return r; };
        set_target = [&r](Byte value) { r = value; };
    }

    using addressing_mode_setup = CpuTestBench& (CpuTestBench::*)(is6502::OpName op);
    
    // during accumulator execution
    // the 6502 reads the byte at PC+1 but does not increment PC
    // resulting in a 1 byte opcode

    CpuTestBench& accumulator(is6502::OpName op) {
        initialize_register_target(cpu.A);
        return encode(op, AMode::ACC)
               .db(0xFF);
    }
    
    addressing_mode_setup implicit_target(Byte& r) {
        initialize_register_target(r);
        return &CpuTestBench::implicit;
    }
    
    CpuTestBench& implicit(is6502::OpName op) {
        return encode(op, AMode::IMP)
               .db(0xFF);
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

    // during zeropage indexed execution
    // the 6502 makes a dummy read at the non-indexed address

    CpuTestBench& zeropage_x(is6502::OpName op) {
        initialize_memory_target(0x0028);
        cpu.X = 0x08;
        return encode(op, AMode::ZPX)
            .db(0x20)
            .at(0x20).db(0xFF);
    }

    CpuTestBench& zeropage_x_wraparound(is6502::OpName op) {
        initialize_memory_target(0x0000);
        cpu.X = 0x10;
        return encode(op, AMode::ZPX)
            .db(0xF0)
            .at(0xF0).db(0xFF);
    }

    CpuTestBench& zeropage_y(is6502::OpName op) {
        initialize_memory_target(0x0028);
        cpu.Y = 0x08;
        return encode(op, AMode::ZPY)
            .db(0x20)
            .at(0x20).db(0xFF);
    }

    CpuTestBench& zeropage_y_wraparound(is6502::OpName op) {
        initialize_memory_target(0x0000);
        cpu.Y = 0x10;
        return encode(op, AMode::ZPY)
            .db(0xF0)
            .at(0xF0).db(0xFF);
    }

    CpuTestBench& absolute(is6502::OpName op) {
        initialize_memory_target(0x0120);
        return encode(op, AMode::ABS)
            .dw(0x0120);
    }

    // during absolute indexed execution
    // the 6502 makes a dummy read at an address where only
    // the low byte has the index added

    CpuTestBench& absolute_x(is6502::OpName op) {
        initialize_memory_target(0x0128);
        cpu.X = 0x08;
        return encode(op, AMode::ABX)
            .dw(0x0120)
            .at(0x0120).db(0xFF);
    }

    CpuTestBench& absolute_x_crossing_page(is6502::OpName op) {
        initialize_memory_target(0x0210);
        cpu.X = 0xF0;
        return encode(op, AMode::ABX)
            .dw(0x0120)
            .at(0x0110).db(0xFF);
    }

    CpuTestBench& absolute_y(is6502::OpName op) {
        initialize_memory_target(0x0128);
        cpu.Y = 0x08;
        return encode(op, AMode::ABY)
            .dw(0x0120)
            .at(0x0120).db(0xFF);
    }

    CpuTestBench& absolute_y_crossing_page(is6502::OpName op) {
        initialize_memory_target(0x0210);
        cpu.Y = 0xF0;
        return encode(op, AMode::ABY)
            .dw(0x0120)
            .at(0x0110).db(0xFF);
    }

    // during indirect x execution
    // the 6502 will make a dummy read at the non-indexed address

    CpuTestBench& indirect_x(is6502::OpName op) {
        initialize_memory_target(0x0120);
        cpu.X = 0x08;
        return encode(op, AMode::IDX)
            .db(0x20)
            .at(0x0028).dw(0x0120)
            .at(0x0020).db(0xFF);
    }

    CpuTestBench& indirect_x_wraparound(is6502::OpName op) {
        initialize_memory_target(0x0120);
        cpu.X = 0x0F;
        return encode(op, AMode::IDX)
            .db(0xF0)
            .at(0x00FF).db(0x20)
            .at(0x0000).db(0x01)
            .at(0x00F0).db(0xFF);
    }

    // during indirect y execution
    // the 6502 will index the low byte and read from the address
    // before fixing the high byte

    CpuTestBench& indirect_y(is6502::OpName op) {
        initialize_memory_target(0x0128);
        cpu.Y = 0x08;
        return encode(op, AMode::IDY)
            .db(0x20)
            .at(0x0020).dw(0x0120);
    }

    CpuTestBench& indirect_y_crossing_page(is6502::OpName op) {
        initialize_memory_target(0x0210);
        cpu.Y = 0xF0;
        return encode(op, AMode::IDY)
            .db(0x20)
            .at(0x0020).dw(0x0120)
            .at(0x0110).db(0xFF);
    }

    CpuTestBench& indirect_y_crossing_word_size(is6502::OpName op) {
        initialize_memory_target(0x000F);
        cpu.Y = 0x10;
        return encode(op, AMode::IDY)
            .db(0x20)
            .at(0x0020).dw(0xFFFF)
            .at(0xFF0F).db(0xFF); // before "fix", 0xFFFF + 0x10 = 0xFF0F
    }

    CpuTestBench& indirect_y_base_from_zeropage(is6502::OpName op) {
        initialize_memory_target(0x0130);
        cpu.Y = 0x10;
        return encode(op, AMode::IDY)
            .db(0xFF)
            .at(0x00FF).db(0x20)
            .at(0x0000).db(0x01);
    }

    // during relative execution
    // the 6502 might wrongly read at PC+2 and "un-fixed" PC+offset

    CpuTestBench& relative(is6502::OpName op, Byte offset) {
        const Word unfixedPC{ MakeWord(LO(current + offset + 2), HI(current)) };
        return encode(op, AMode::REL)
               .db(offset)
               .db(0xFF)
               .at(unfixedPC).db(0xFF);
    }

    // expectations
    Word expected_PC() const {
        auto bytes = std::accumulate(
            instructions.cbegin(),
            instructions.cend(),
            0,
            [] (auto acc, auto op) { return acc + op.Bytes; }
        );
        return PC + bytes;
    }
    
    size_t expected_ticks(int extra) const {
        auto cycles = std::accumulate(
            instructions.cbegin(),
            instructions.cend(),
            0,
            [] (auto acc, auto op) { return acc + op.Cycles; }
        );
        return Ticks + cycles + extra;
    }
};
