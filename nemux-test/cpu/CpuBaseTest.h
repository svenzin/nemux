#pragma once

#include <gtest/gtest.h>

#include "CpuTestBench.h"
#include "Cpu.h"
#include "Ricoh_RP2A03.h"
#include "cpu/RP2A03.h"

struct CpuBaseTest : public ::testing::Test {

    using CpuT = RP2A03; //Ricoh_RP2A03; //Cpu;
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
        size_t count{ 0 };
        bool done{ false };
        do {
            done = cpu->Tick();
            ++count;
        } while (!done && (count < 10));
    }
};
