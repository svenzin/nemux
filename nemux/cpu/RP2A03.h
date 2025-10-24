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

protected:
    void Phi1();
    void Phi2();

    using CycleT = void(RP2A03::*)();

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
    CycleT _Operation;
    Byte _ByteOperand;
    Word _WordOperand;
    bool _IRQTriggered;
    bool _NMITriggered;
    bool _PreviousLineNMI;

    // TODO for debugging, remove
    InstructionSet_6502::Instruction _CurrentInstruction;
    CycleCounter _CurrentBegin;
    Byte _CurrentOpcode;

    void Cycle_Unreachable();
    void Cycle_FetchOpcode_IncrementPC();
    void Cycle_FetchDummy_Operation();
    void Cycle_FetchOperand_IncrementPC();
    void Cycle_FetchOperand_IncrementPC_Operation();
    void Cycle_FetchAddress_IncrementPC();
    void Cycle_FetchAddressLO_IncrementPC();
    void Cycle_FetchAddressHI_IncrementPC();
    void Cycle_FetchAddressHI_IndexX_IncrementPC();
    void Cycle_FetchAddressHI_IndexY_IncrementPC();
    void Cycle_ReadOperand_Operation(); // read
    void Cycle_ReadOperand();               // read-modify-write step 1
    void Cycle_WriteOperand_Operation();    // read-modify-write step 2
    void Cycle_WriteOperand();              // read-modify-write step 3
    void Cycle_ReadOperand_FixHI_IndexX();
    void Cycle_ReadOperand_FixHI_IndexX_TryOperation();
    void Cycle_ReadOperand_FixHI_IndexY();
    void Cycle_ReadOperand_FixHI_IndexY_TryOperation();
    void Cycle_Operation();
    void Cycle_ReadOperand_IndexX();
    void Cycle_ReadOperand_IndexY();
    void Cycle_ReadAddressLO();
    void Cycle_ReadAddressHI();
    void Cycle_ReadAddressHI_IndexY();
    void Cycle_FetchOperand_IncrementPC_Branch();
    void Cycle_TakeBranch();
    void Cycle_FixBranch();
    void Cycle_FetchAddressHI_Operation(); // TODO can increment PC before OP if useful for refactoring
    void Cycle_ReadAddressHI_Operation();

    void Transfer(Byte value, Byte& to);
    void Branch(bool condition);

    void LDA();
    void LDX();
    void LDY();
    void STA();
    void STX();
    void STY();

    void BCC();
    void BCS();
    void BEQ();
    void BMI();
    void BNE();
    void BPL();
    void BVC();
    void BVS();
    
    void TAX();
    void TAY();
    void TXA();
    void TYA();

    void JMP();
};
