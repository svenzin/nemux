#pragma once

#include <gtest/gtest.h>

#include "CpuTestBench.h"
#include "Cpu.h"

struct CpuBaseTest : public ::testing::Test {

    using CpuT = Cpu;
    using MemoryMapT = MemoryBlock<0x10000>;

    CpuBaseTest() {
        memory = std::make_unique<MemoryMapT>();
        for (size_t i{ 0 }; i < 0x10000; ++i) {
            memory->SetByteAt(i, 0x55);
        }

        cpu = std::make_unique<CpuT>("6502", memory.get());

        bench.initialize(memory.get(), cpu.get());
    }

    std::unique_ptr<MemoryMap> memory;
    std::unique_ptr<BaseCpu> cpu;
    CpuTestBench bench;

    void ExecuteOne() {
        bool done{ false };
        do {
            done = cpu->Tick();
        } while (!done);
    }
};
