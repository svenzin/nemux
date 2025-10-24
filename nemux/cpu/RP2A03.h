#pragma once


#include "cpu/BaseCpu.h"
#include "cpu/InstructionSet_6502.h"

#include <array>


class RP2A03 : public BaseCpu {
public:
    explicit RP2A03(const std::string& name,
                    MemoryMap* map);

    void PowerUp() override;
    void Reset() override;
    [[nodiscard]] bool Tick() override;
    void DMA(Byte page, Byte* target, Byte offset) override;

    using ActionT = void(RP2A03::*)();
    using CycleT = ActionT;

protected:
    void Phi1();
    void Phi2();

    static constexpr size_t MAX_CYCLES_PER_INSTRUCTION{ 8 };
    static constexpr size_t MAX_CYCLE_COUNT{
        InstructionSet_6502::INSTRUCTION_COUNT * MAX_CYCLES_PER_INSTRUCTION
    };

    // TODO optimize this, probably with MAX_CYCLES_PER_INSTRUCTION as a power of two
    struct CycleCounter {
        static uint16_t ValueFrom(Byte opcode, Byte step) { return opcode * MAX_CYCLES_PER_INSTRUCTION + step; }

        uint16_t _Value;

        auto Get() const { return _Value; }
        void Set(Byte opcode, Byte step) { _Value = ValueFrom(opcode, step); }

        Byte GetOpcode() const { return _Value / MAX_CYCLES_PER_INSTRUCTION; }
        void SetOpcode(Byte opcode) { Set(opcode, GetStep()); }

        Byte GetStep() const { return _Value % MAX_CYCLES_PER_INSTRUCTION; }
        void SetStep(Byte step) { Set(GetOpcode(), step); }

        CycleCounter& operator++() { ++_Value; return *this; }
    };

    std::array<CycleT, MAX_CYCLE_COUNT> _InstructionsCycles;
    CycleCounter _CurrentCycle;
    Byte _ByteOperand;
    Word _WordOperand;
    bool _IRQTriggered;
    bool _NMITriggered;
    bool _PreviousLineNMI;

    // TODO for debugging, remove
    InstructionSet_6502::Instruction _CurrentInstruction;
    CycleCounter _CurrentBegin;
    Byte _CurrentOpcode;

public:
    template <ActionT OP = (ActionT)nullptr> void Cycle();
    template <ActionT OP = (ActionT)nullptr> void Cycle_FetchDummy();
    template <ActionT OP = (ActionT)nullptr> void Cycle_FetchOperand_IncrementPC();
    template <ActionT OP = (ActionT)nullptr> void Cycle_ReadOperand();
    template <ActionT OP = (ActionT)nullptr> void Cycle_WriteOperand();
    template <ActionT OP> void Cycle_ReadOperand_FixHI_IndexX_Try();
    template <ActionT OP> void Cycle_ReadOperand_FixHI_IndexY_Try();

    void Cycle_GetAddressHI(Word from, Byte index);

    void Cycle_Unreachable();
    void Cycle_FetchOpcode_IncrementPC();
    void Cycle_FetchZeroPageAddress_IncrementPC();
    void Cycle_FetchAddressLO_IncrementPC();
    void Cycle_FetchAddressHI_IncrementPC();
    void Cycle_FetchAddressHI_IndexX_IncrementPC();
    void Cycle_FetchAddressHI_IndexY_IncrementPC();
    void Cycle_ReadOperand_IndexX();
    void Cycle_ReadOperand_IndexY();
    void Cycle_ReadOperand_FixHI_IndexX();
    void Cycle_ReadOperand_FixHI_IndexY();
    void Cycle_ReadAddressLO();
    void Cycle_ReadAddressHI();
    void Cycle_ReadAddressHI_IndexY();

    void Transfer(Byte value, Byte& to);

    void LDA();
    void LDX();
    void LDY();
    void STA();
    void STX();
    void STY();
};
