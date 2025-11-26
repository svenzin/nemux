#pragma once


#include "cpu/BaseCpu.h"
#include "cpu/InstructionSet_6502.h"

#include <array>
#include <limits>


class RP2A03 : public BaseCpu {
public:
    explicit RP2A03(const std::string& name,
                    MemoryMap* map);

    void PowerUp() override;
    void Reset() override;
    [[nodiscard]] bool Tick() override;
    void DMA(Byte page, Byte* target, Byte offset) override;

    void TriggerDMA(Word from, size_t count, Word to);

protected:
    void Phi1();
    void Phi2();

    using CycleT = void(RP2A03::*)();
    using CycleCounter = uint16_t;

    static constexpr size_t MAX_CYCLES_PER_INSTRUCTION{ 8 };
    static constexpr size_t MAX_CYCLE_COUNT{
        InstructionSet_6502::INSTRUCTION_COUNT * MAX_CYCLES_PER_INSTRUCTION
        + 1 // initial state
    };
    static_assert(MAX_CYCLE_COUNT < std::numeric_limits<CycleCounter>::max());

    std::array<CycleT, MAX_CYCLE_COUNT> _InstructionsCycles;
    CycleCounter _CurrentCycle;
    CycleT _Operation;
    int _SuspendCount;
    Word _WordOperand;
    Byte _ByteOperand;
    Flag _FlagOperand;
    bool _IRQTriggered;
    bool _NMITriggered;
    bool _PreviousLineNMI;

    enum class DMACycleType : bool {
        Get = true,
        Put = false,
    };
    static DMACycleType Flip(DMACycleType value) { return DMACycleType{ !std::to_underlying(value) }; }
    DMACycleType _DMAType;

    // TODO for debugging, remove
    InstructionSet_6502::Instruction _CurrentInstruction;
    CycleCounter _CurrentBegin;
    Byte _CurrentOpcode;

    Byte ReadByteFromStack();
    void WriteByteToStack(Byte value);

    // TODO all fetch/read dummys might be replaced by reading the _ByteOperand and not doing anything with it
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
    void Cycle_ReadDummy_FixHI_IndexX();
    void Cycle_ReadOperand_FixHI_IndexX_TryOperation();
    void Cycle_ReadDummy_FixHI_IndexY();
    void Cycle_ReadOperand_FixHI_IndexY_TryOperation();
    void Cycle_Operation();
    void Cycle_ReadDummy_IndexX();
    void Cycle_ReadDummy_IndexY();
    void Cycle_ReadAddressLO();
    void Cycle_ReadAddressHI();
    void Cycle_ReadAddressHI_IndexY();
    void Cycle_FetchOperand_IncrementPC_Branch();
    void Cycle_FetchDummy_TakeBranch();
    void Cycle_FetchDummy_FixBranch();
    void Cycle_FetchAddressHI_Operation(); // TODO can increment PC before OP if useful for refactoring
    void Cycle_ReadAddressHI_Operation();
    void Cycle_Operation_DecrementS();
    void Cycle_PushPCL_DecrementS();
    void Cycle_PushPCH_DecrementS();
    void Cycle_ReadDummyStack();
    void Cycle_FetchDummy();
    void Cycle_ReadDummyStack_IncrementS();
    void Cycle_PullPCL_IncrementS();
    void Cycle_PullPCH();
    void Cycle_IncrementPC_Operation();
    void Cycle_FetchDummy_MaybeIncrementPC_Operation();
    void Cycle_PushP_DecrementS_SelectVector();
    void Cycle_ReadPCL();
    void Cycle_ReadPCH_ClearNMI();
    void Cycle_PullP_IncrementS();

    void Transfer(Byte value, Byte& to);
    void Branch(bool condition);
    void Jump();
    void RotateLeft(Byte& value, Flag bit);
    void RotateRight(Byte& value, Flag bit);
    void Compare(Byte lhs, Byte rhs);
    void AddWithCarry(Byte value);

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
    void JSR();
    void RTS();

    void TSX();
    void TXS();

    void PHA();
    void PLA();
    void PHP();
    void PLP();

    void ASL();
    void LSR();
    void ROL();
    void ROR();

    void ASL_a();
    void LSR_a();
    void ROL_a();
    void ROR_a();

    void DEC();
    void DEX();
    void DEY();
    void INC();
    void INX();
    void INY();

    void CLC();
    void CLD();
    void CLI();
    void CLV();
    void SEC();
    void SED();
    void SEI();

    void CPX();
    void CPY();
    void CMP();
    void ADC();
    void SBC();

    void EOR();
    void ORA();
    void AND();
    void BIT();

    void NOP();

    void BRK();
    void RTI();

    void uNOP();
    void uSTP();
    void uSLO();
    void uANC();
    void uRLA();
    void uSRE();
    void uALR();
    void uRRA();
    void uARR();
    void uSAX();
    void uXAA();
    void uAHX();
    void uTAS();
    void uSHY();
    void uSHX();
    void uLAX();
    void uLAS();
    void uDCP();
    void uAXS();
    void uISC();
    void uSBC();
};
