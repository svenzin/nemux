#pragma once

#include <gtest/gtest.h>

#include "CpuTestBench.h"
#include "Cpu.h"

struct CpuBaseTest : public ::testing::Test {
    CpuBaseTest()
    : cpu{ "6502", &memory }
    , bench{ &cpu, &memory }
    {
        for (size_t i{ 0 }; i < 0x10000; ++i)
            memory.SetByteAt(i, 0x55);
    }

    MemoryBlock<0x10000> memory;
    Cpu cpu;
    CpuTestBench bench;

    void ExecuteOne() {
        bool done{ cpu.Tick() };
        while (!done) {
            done = cpu.Tick();
        };
    }
};
