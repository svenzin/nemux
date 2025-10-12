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
    size_t InterruptCycles{ 7 };
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

    Word ReadWordAt(const Word address) const;
    void WriteWordAt(const Word address, const Word value);

    explicit Cpu(std::string name, MemoryMap * map = nullptr);

    std::string Name;

    address_t BuildAddress(InstructionSet_6502::AddressingMode mode) const;
    void Execute(const Instruction &op);

    std::string ToString() const;
    std::string ToMiniString() const;

    void Decrement(Byte & value);
    void Increment(Byte & value);
    void Compare(const Byte lhs, const Byte rhs);
    void Transfer(const Byte & from, Byte & to);
    void BranchIf(bool condition, Byte offset);
    void AddWithCarry(const Byte value);
    void SubstractWithCarry(const Byte value);

    void Jump(const Word address);

    void Push(const Byte & value);
    Byte Pull();

    void PushWord(const Word & value);
    Word PullWord();

    void Interrupt(const Flag & isBRK, const Word & vector, const bool readOnly = false);

    void NMI();
    void IRQ();

    InterruptType PendingInterrupt;
    void TriggerReset();
    void TriggerNMI();
    void TriggerIRQ();

private:
    std::vector<Instruction> m_opcodes;
};
