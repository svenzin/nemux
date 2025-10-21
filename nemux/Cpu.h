#pragma once


#include "cpu/BaseCpu.h"
#include "cpu/InstructionSet_6502.h"

#include <string>
#include <vector>


enum class InterruptType {
    None, Irq, Nmi, Rst,
};


struct address_t {
    Word Address;
    bool HasCrossedPage;
};


class Cpu : public BaseCpu {
protected:
    using Instruction = InstructionSet_6502::Instruction;
    std::array<Instruction, InstructionSet_6502::INSTRUCTION_COUNT> m_opcodes;

public:
    // TODO still needed for DMC DMA, find a way to remove it
    using BaseCpu::Ticks;

protected:
    size_t InterruptCycles;
    size_t CurrentTick;
    InterruptType PendingInterrupt;
    bool PreviousNMI;

public:
    explicit Cpu(const std::string& name,
                 MemoryMap* map);

    ////////////////////////////////////////////////////////////
    // BaseCpu overrides
    void PowerUp() override;
    void Reset() override;
    [[nodiscard]] bool Tick() override;
    void DMA(Byte page, Byte* target, Byte offset) override;

protected:
    ////////////////////////////////////////////////////////////
    // Word-sized helpers
    Word ReadWordAt(Word address) const;
    void WriteWordAt(Word address, Word value);
    
    void PushWord(Word value);
    Word PullWord();

    ////////////////////////////////////////////////////////////
    // Basic operations
    void Decrement(Byte& value);
    void Increment(Byte& value);
    void Transfer(Byte value, Byte & to);
    void Compare(Byte lhs, Byte rhs);
    void BranchIf(bool condition, address_t target);
    void AddWithCarry(Byte value);
    void SubstractWithCarry(Byte value);
    void Jump(Word address);
    void Push(Byte value);
    Byte Pull();
    void Interrupt(bool isBRK, Word vector, bool isReadOnly);
    
    void NMI();
    void IRQ();

    ////////////////////////////////////////////////////////////
    // Main loop
    address_t BuildAddress(const Instruction& op) const;
    void Execute(const Instruction &op);
};
