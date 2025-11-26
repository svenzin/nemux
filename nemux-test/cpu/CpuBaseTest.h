#pragma once

#include <gtest/gtest.h>

#include "CpuTestBench.h"


struct CpuBaseTest : public ::testing::Test {

    CpuTestBench bench;
    MemoryMap* memory{ &bench.memory };
    BaseCpu* cpu{ &bench.cpu };

    void ExecuteOne() {
        size_t count{ 0 };
        bool done{ false };
        do {
            done = cpu->Tick();
            ++count;
        } while (!done && (count < 10));
    }
};
