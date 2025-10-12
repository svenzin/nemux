#pragma once

#include <gtest/gtest.h>

#include "CpuTestBench.h"
#include "Cpu.h"

struct CpuBaseTest : public ::testing::Test {
    static constexpr Word BASE_PC{ 10 };
    static constexpr int BASE_TICKS{ 0 };

    CpuBaseTest()
    : cpu{ "6502", &memory }
    , bench{ &cpu, &memory }
    {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;

        for (size_t i{ 0 }; i < 0x10000; ++i)
            memory.SetByteAt(i, 0x55);
    }

    MemoryBlock<0x10000> memory;
    Cpu cpu;
    CpuTestBench bench;

    // TODO remove
    static auto Getter(Byte & b) {
        return [&b] () { return b; };
    }

    static auto Setter(Byte & a) {
        return [&a] (Byte value) { a = value; };
    }

    auto Setter(Word a) {
        return [this, a] (Byte value) { cpu.WriteByte(a, value); };
    }

    auto Getter(Word a) {
        return [this, a] () { return cpu.ReadByte(a); };
    }

    void ExecuteOne() {
        bool done{ cpu.Tick() };
        while (!done) {
            done = cpu.Tick();
        };
    }

    Byte GetStackValueAt(int offset) const {
        return memory.GetByteAt(static_cast<Word>(cpu.StackPage + cpu.S + offset));
    }
};
