#pragma once

#include "cpu/BaseCpu.h"
#include "cpu/InstructionSet_6502.h"

using is6502 = InstructionSet_6502;

static constexpr Word ORIGIN{ 10 };

class CpuTestBench {
    BaseCpu* cpu;
    MemoryMap* memory;
    Word current;
    
public:
    Word PC;
    size_t Ticks;
    is6502::Instruction op;

    CpuTestBench(BaseCpu* _cpu, MemoryMap* map)
    : cpu{ _cpu }
    , memory{ map }
    {
        origin(ORIGIN);
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
        Ticks = cpu->GetTicks();
    }
};
