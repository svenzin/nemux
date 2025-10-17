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
    size_t InterruptCycles;
    size_t CurrentTick;

public:
    // BaseCpu overrides
    void PowerUp() override;
    void Reset() override;
    [[nodiscard]] bool Tick() override;
    void DMA(Byte page, Byte* target, Byte offset) override;

public:
    // TODO still needed for DMC DMA, find a way to remove it
    using BaseCpu::Ticks;

    using Instruction = InstructionSet_6502::Instruction;

public: // TODO make it protected
    ////////////////////////////////////////////////////////////
    // Word-sized helpers
    Word ReadWordAt(Word address) const;
    void WriteWordAt(Word address, Word value);
    
    void PushWord(Word value);
    Word PullWord();

public:
    ////////////////////////////////////////////////////////////
    explicit Cpu(const std::string& name,
                 MemoryMap* map);

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
    void Interrupt(bool isBRK, Word vector, bool isReadOnly = false);
////////////////////////////////////////////////////////////////////////////////
    address_t BuildAddress(const Instruction& op) const;
////////////////////////////////////////////////////////////////////////////////

    std::string Name;

    void Execute(const Instruction &op);

    std::string ToString() const;
    std::string ToMiniString() const;





    InterruptType PendingInterrupt;
    void NMI();
    void IRQ();

private:
    std::array<Instruction, InstructionSet_6502::INSTRUCTION_COUNT> m_opcodes;
};
