#include "cpu/RP2A03.h"


// TODO remove when not needed anymore
namespace {
    static constexpr bool BRANCH_USING_WORDOPERAND{ false };

    void NOT_IMPLEMENTED() { throw std::runtime_error("not implemented"); }
    void UNREACHABLE() { throw std::runtime_error("unreachable"); }

    enum class AddressingType {
        Unused,
        Read,
        Write,
        ReadModifyWrite,
    };
    constexpr AddressingType OpcodeType(Byte opcode) {
        const auto type{ (opcode & 0b11100000) >> 5 };
        if (type == 4) return AddressingType::Write;
        if (type == 5) return AddressingType::Read;
        return AddressingType::ReadModifyWrite;
    }
}

////////////////////////////////////////////////////////////////////////////////

RP2A03::RP2A03(const std::string& name, MemoryMap* map)
    : BaseCpu{}
{
    Name = name;
    Map = map;
    
    _InstructionsCycles.fill(&RP2A03::Cycle_Unreachable);
    
    CycleCounter counter;
    for (
        size_t opcode{ 0 };
        opcode < InstructionSet_6502::INSTRUCTION_COUNT;
        ++opcode
    ) {
        const auto instr{ InstructionSet_6502::Decode(opcode) };
        const auto type{ OpcodeType(opcode) };
        
        using enum AddressingType;
        using enum InstructionSet_6502::AddressingMode;
        static auto FillWithModeCycles = [](CycleT* cycle, auto mode, auto type) {
            switch (mode) {

                case IMP: [[fallthrough]];
                case ACC: {
                    *(cycle++) = &RP2A03::Cycle_FetchDummy_Operation;
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case IMM: {
                    *(cycle++) = &RP2A03::Cycle_FetchOperand_IncrementPC_Operation;
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case ZPG: {
                    *(cycle++) = &RP2A03::Cycle_FetchAddress_IncrementPC;
                    switch (type) {
                        case Read: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_Operation;
                            break;
                        }
                        case Write: {
                            *(cycle++) = &RP2A03::Cycle_Operation;
                            break;
                        }
                        case ReadModifyWrite: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand_Operation;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand;
                            break;
                        }
                        default: UNREACHABLE();
                    }
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case ZPX: {
                    *(cycle++) = &RP2A03::Cycle_FetchAddress_IncrementPC;
                    *(cycle++) = &RP2A03::Cycle_ReadOperand_IndexX;
                    switch (type) {
                        case Read: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_Operation;
                            break;
                        }
                        case Write: {
                            *(cycle++) = &RP2A03::Cycle_Operation;
                            break;
                        }
                        case ReadModifyWrite: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand_Operation;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand;
                            break;
                        }
                        default: UNREACHABLE();
                    }
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case ZPY: {
                    *(cycle++) = &RP2A03::Cycle_FetchAddress_IncrementPC;
                    *(cycle++) = &RP2A03::Cycle_ReadOperand_IndexY;
                    switch (type) {
                        case Read: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_Operation;
                            break;
                        }
                        case Write: {
                            *(cycle++) = &RP2A03::Cycle_Operation;
                            break;
                        }
                        case ReadModifyWrite: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand_Operation;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand;
                            break;
                        }
                        default: UNREACHABLE();
                    }
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case ABS: {
                    *(cycle++) = &RP2A03::Cycle_FetchAddressLO_IncrementPC;
                    *(cycle++) = &RP2A03::Cycle_FetchAddressHI_IncrementPC;
                    switch (type) {
                        case Read: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_Operation;
                            break;
                        }
                        case Write: {
                            *(cycle++) = &RP2A03::Cycle_Operation;
                            break;
                        }
                        case ReadModifyWrite: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand_Operation;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand;
                            break;
                        }
                        default: UNREACHABLE();
                    }
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case ABX: {
                    *(cycle++) = &RP2A03::Cycle_FetchAddressLO_IncrementPC;
                    *(cycle++) = &RP2A03::Cycle_FetchAddressHI_IndexX_IncrementPC;
                    switch (type) {
                        case Read: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexX_TryOperation;
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_Operation;
                            break;
                        }
                        case Write: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexX;
                            *(cycle++) = &RP2A03::Cycle_Operation;
                            break;
                        }
                        case ReadModifyWrite: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexX;
                            *(cycle++) = &RP2A03::Cycle_ReadOperand;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand_Operation;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand;
                            break;
                        }
                        default: UNREACHABLE();
                    }
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case ABY: {
                    *(cycle++) = &RP2A03::Cycle_FetchAddressLO_IncrementPC;
                    *(cycle++) = &RP2A03::Cycle_FetchAddressHI_IndexY_IncrementPC;
                    switch (type) {
                        case Read: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY_TryOperation;
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_Operation;
                            break;
                        }
                        case Write: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY;
                            *(cycle++) = &RP2A03::Cycle_Operation;
                            break;
                        }
                        case ReadModifyWrite: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY;
                            *(cycle++) = &RP2A03::Cycle_ReadOperand;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand_Operation;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand;
                            break;
                        }
                        default: UNREACHABLE();
                    }
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case REL: {
                    *(cycle++) = &RP2A03::Cycle_FetchOperand_IncrementPC_Branch;
                    *(cycle++) = &RP2A03::Cycle_FetchDummy_TakeBranch;
                    *(cycle++) = &RP2A03::Cycle_FetchDummy_FixBranch;
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case IDX: {
                    *(cycle++) = &RP2A03::Cycle_FetchAddress_IncrementPC;
                    *(cycle++) = &RP2A03::Cycle_ReadOperand_IndexX;
                    *(cycle++) = &RP2A03::Cycle_ReadAddressLO;
                    *(cycle++) = &RP2A03::Cycle_ReadAddressHI;
                    switch (type) {
                        case Read: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_Operation;
                            break;
                        }
                        case Write: {
                            *(cycle++) = &RP2A03::Cycle_Operation;
                            break;
                        }
                        case ReadModifyWrite: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand_Operation;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand;
                            break;
                        }
                        default: UNREACHABLE();
                    }
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case IDY: {
                    *(cycle++) = &RP2A03::Cycle_FetchAddress_IncrementPC;
                    *(cycle++) = &RP2A03::Cycle_ReadAddressLO;
                    *(cycle++) = &RP2A03::Cycle_ReadAddressHI_IndexY;
                    switch (type) {
                        case Read: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY_TryOperation;
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_Operation;
                            break;
                        }
                        case Write: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY;
                            *(cycle++) = &RP2A03::Cycle_Operation;
                            break;
                        }
                        case ReadModifyWrite: {
                            *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY;
                            *(cycle++) = &RP2A03::Cycle_ReadOperand;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand_Operation;
                            *(cycle++) = &RP2A03::Cycle_WriteOperand;
                            break;
                        }
                        default: UNREACHABLE();
                    }
                    *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                    break;
                }

                case IND: [[fallthrough]]; // JMP is already covered in the special cases
                default: UNREACHABLE();
            }
        };

        auto counter{ CycleCounter::ValueFrom(opcode, 0) };
        auto* cycle{ _InstructionsCycles.begin() + counter };
        switch (instr.Name) {
            using enum InstructionSet_6502::OpName;
            case JMP: {
                switch (instr.Mode) {
                    case ABS: {
                        *(cycle++) = &RP2A03::Cycle_FetchAddressLO_IncrementPC;
                        *(cycle++) = &RP2A03::Cycle_FetchAddressHI_Operation;
                        *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                        break;
                    }
                    case IND: {
                        *(cycle++) = &RP2A03::Cycle_FetchAddressLO_IncrementPC;
                        *(cycle++) = &RP2A03::Cycle_FetchAddressHI_IncrementPC;
                        *(cycle++) = &RP2A03::Cycle_ReadAddressLO;
                        *(cycle++) = &RP2A03::Cycle_ReadAddressHI_Operation;
                        *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                        break;
                    }
                    default: UNREACHABLE();
                }
                break;
            }
            case JSR: {
                *(cycle++) = &RP2A03::Cycle_FetchAddressLO_IncrementPC;
                *(cycle++) = &RP2A03::Cycle_ReadDummyStack;
                *(cycle++) = &RP2A03::Cycle_PushPCH_DecrementS;
                *(cycle++) = &RP2A03::Cycle_PushPCL_DecrementS;
                *(cycle++) = &RP2A03::Cycle_FetchAddressHI_Operation;
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }
            case RTS: {
                *(cycle++) = &RP2A03::Cycle_FetchDummy;
                *(cycle++) = &RP2A03::Cycle_ReadDummyStack_IncrementS;
                *(cycle++) = &RP2A03::Cycle_PullPCL_IncrementS;
                *(cycle++) = &RP2A03::Cycle_PullPCH;
                *(cycle++) = &RP2A03::Cycle_IncrementPC_Operation;
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }
            case PHA: [[fallthrough]];
            case PHP: {
                *(cycle++) = &RP2A03::Cycle_FetchDummy;
                *(cycle++) = &RP2A03::Cycle_Operation_DecrementS;
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }
            case PLA: [[fallthrough]];
            case PLP: {
                *(cycle++) = &RP2A03::Cycle_FetchDummy;
                *(cycle++) = &RP2A03::Cycle_ReadDummyStack_IncrementS;
                *(cycle++) = &RP2A03::Cycle_Operation;
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }
            default: {
                FillWithModeCycles(cycle, instr.Mode, type);
                break;
            }
        }
    }

    _CurrentCycle.Set(0xEA, 1);
    _Operation = &RP2A03::Cycle_Unreachable;

    _IRQTriggered = false;
    _NMITriggered = false;
    _PreviousLineNMI = LineNMI;
}

////////////////////////////////////////////////////////////////////////////////

Byte RP2A03::ReadByteFromStack() {
    return ReadByte(StackPage + S);
}

void RP2A03::WriteByteToStack(Byte value) {
    WriteByte(StackPage + S, value);
}

////////////////////////////////////////////////////////////////////////////////

void RP2A03::Cycle_Unreachable() {
    UNREACHABLE();
}

void RP2A03::Cycle_FetchOpcode_IncrementPC() {
    const Byte opcode{ ReadByte(PC) };
    ++PC;
    
    _CurrentCycle.Set(opcode, 0);
    const auto opname{ InstructionSet_6502::LUT::OpcodeNames[opcode] };
    const auto opmode{ InstructionSet_6502::LUT::OpcodeAddressingModes[opcode] };
    switch (opname) {
        using enum InstructionSet_6502::OpName;
        using enum InstructionSet_6502::AddressingMode;

        #define CASE(OP) case OP: { _Operation = &RP2A03::OP; break; }
        #define CASE_A(OP) case OP: { _Operation = (opmode == ACC) ? &RP2A03::OP##_a : &RP2A03::OP; break; }

        CASE(LDA)   CASE(LDX)   CASE(LDY)
        CASE(STA)   CASE(STX)   CASE(STY)
        CASE(BCC)   CASE(BCS)   CASE(BEQ)   CASE(BMI)
        CASE(BNE)   CASE(BPL)   CASE(BVC)   CASE(BVS)
        CASE(TAX)   CASE(TAY)   CASE(TXA)   CASE(TYA)
        CASE(JMP)   CASE(JSR)   CASE(RTS)
        CASE(TSX)   CASE(TXS)
        CASE(PHA)   CASE(PLA)   CASE(PHP)   CASE(PLP)
        CASE_A(ASL) CASE_A(LSR) CASE_A(ROL) CASE_A(ROR)
        CASE(DEC)   CASE(DEX)   CASE(DEY)   CASE(INC)   CASE(INX)   CASE(INY)
        CASE(CLC)   CASE(CLD)   CASE(CLI)   CASE(CLV)
        CASE(SEC)   CASE(SED)   CASE(SEI)
        default: _Operation = &RP2A03::Cycle_Unreachable; break;

        #undef CASE_A
        #undef CASE
    }
    
    _CurrentOpcode = opcode;
    _CurrentInstruction = InstructionSet_6502::Decode(opcode);
    _CurrentBegin.Set(opcode, 0);

}

void RP2A03::Cycle_FetchDummy_Operation() {
    ReadByte(PC);
    (this->*_Operation)();
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchOperand_IncrementPC() {
    _ByteOperand = ReadByte(PC);
    ++PC;
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchOperand_IncrementPC_Operation() {
    _ByteOperand = ReadByte(PC);
    ++PC;
    (this->*_Operation)();
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchAddress_IncrementPC() {
    // TODO might be replaced by Cycle_FetchOperand_IncrementPC and using _ByteOperand as address
    _WordOperand = ReadByte(PC);
    ++PC;
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchAddressLO_IncrementPC() {
    // TODO might be replaced by Cycle_FetchOperand_IncrementPC and using _ByteOperand as address
    SetLO(_WordOperand, ReadByte(PC));
    ++PC;
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchAddressHI_IncrementPC() {
    SetHI(_WordOperand, ReadByte(PC));
    ++PC;
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchAddressHI_IndexX_IncrementPC() {
    _WordOperand = MakeWord(LO(_WordOperand) + X, ReadByte(PC));
    ++PC;
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchAddressHI_IndexY_IncrementPC() {
    _WordOperand = MakeWord(LO(_WordOperand) + Y, ReadByte(PC));
    ++PC;
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadOperand_Operation() {
    _ByteOperand = ReadByte(_WordOperand);
    (this->*_Operation)();
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadOperand() {
    _ByteOperand = ReadByte(_WordOperand);
    ++_CurrentCycle;
}

void RP2A03::Cycle_WriteOperand_Operation() {
    WriteByte(_WordOperand, _ByteOperand);
    (this->*_Operation)();
    ++_CurrentCycle;
}

void RP2A03::Cycle_WriteOperand() {
    WriteByte(_WordOperand, _ByteOperand);
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadOperand_IndexX() {
    _ByteOperand = ReadByte(_WordOperand);
    SetLO(_WordOperand, LO(_WordOperand) + X);
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadOperand_IndexY() {
    _ByteOperand = ReadByte(_WordOperand);
    SetLO(_WordOperand, LO(_WordOperand) + Y);
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadOperand_FixHI_IndexX() {
    _ByteOperand = ReadByte(_WordOperand);
    if (LO(_WordOperand) < X) {
        _WordOperand += Word{ 0x0100 };
    }
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadOperand_FixHI_IndexX_TryOperation() {
    _ByteOperand = ReadByte(_WordOperand);
    if (LO(_WordOperand) < X) {
        _WordOperand += Word{ 0x0100 };
    } else {
        (this->*_Operation)();
        ++_CurrentCycle; // skip the extra cycle
    }
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadOperand_FixHI_IndexY() {
    _ByteOperand = ReadByte(_WordOperand);
    if (LO(_WordOperand) < Y) {
        _WordOperand += Word{ 0x0100 };
    }
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadOperand_FixHI_IndexY_TryOperation() {
    _ByteOperand = ReadByte(_WordOperand);
    if (LO(_WordOperand) < Y) {
        _WordOperand += Word{ 0x0100 };
    } else {
        (this->*_Operation)();
        ++_CurrentCycle; // skip the extra cycle
    }
    ++_CurrentCycle;
}

void RP2A03::Cycle_Operation() {
    (this->*_Operation)();
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadAddressLO() {
    _ByteOperand = ReadByte(_WordOperand);
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadAddressHI() {
    // Only used in IndirectX and IndirectY addressing modes
    // so reading from zero page is good enough.
    // const auto hi{ ReadByte(LO(_WordOperand + 1)) };

    // The same "logic" of reading "address HI" is also used in Indirect (JMP)
    // which might read from any memory page. So for the sake of making refactoring
    // easier, we handle any memory page.
    SetLO(_WordOperand, _WordOperand + 1);
    _WordOperand = MakeWord(_ByteOperand, ReadByte(_WordOperand));
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadAddressHI_IndexY() {
    // Same remark as for Cycle_ReadAddressHI()
    // const auto hi{ ReadByte(LO(_WordOperand + 1)) };
    SetLO(_WordOperand, _WordOperand + 1);
    _WordOperand = MakeWord(_ByteOperand + Y, ReadByte(_WordOperand));
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchOperand_IncrementPC_Branch() {
    _ByteOperand = ReadByte(PC);
    ++PC;
    (this->*_Operation)();
    if (PC == _WordOperand) {
        ++_CurrentCycle;    // branch is not taken
        ++_CurrentCycle;    // skip both branching cycles
    }
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchDummy_TakeBranch() {
    ReadByte(PC);
    SetLO(PC, PC + _ByteOperand);
    if constexpr (BRANCH_USING_WORDOPERAND) {
        if (PC == _WordOperand) {
            ++_CurrentCycle;    // no page crossing: skip PCH fix
        }
    } else {
        if (IsSignBitClear(_ByteOperand)) {
            if (LO(PC) >= _ByteOperand) {
                ++_CurrentCycle;    // no page crossing: skip PCH fix
            }
        } else {
            if (LO(PC) < _ByteOperand) {
                ++_CurrentCycle;    // no page crossing: skip PCH fix
            }
        }
    }
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchDummy_FixBranch() {
    ReadByte(PC);
    if constexpr (BRANCH_USING_WORDOPERAND) {
        PC = _WordOperand;
    } else {
        if (IsSignBitClear(_ByteOperand)) {
            PC += Word{ 0x0100 };
        } else {
            PC -= Word{ 0x0100 };
        }
    }
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchAddressHI_Operation() {
    SetHI(_WordOperand, ReadByte(PC));
    (this->*_Operation)();
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadAddressHI_Operation() {
    SetLO(_WordOperand, _WordOperand + 1);
    _WordOperand = MakeWord(_ByteOperand, ReadByte(_WordOperand));
    (this->*_Operation)();
    ++_CurrentCycle;
}

void RP2A03::Cycle_Operation_DecrementS() {
    (this->*_Operation)();
    --S;
    ++_CurrentCycle;
}

void RP2A03::Cycle_PushPCL_DecrementS() {
    WriteByteToStack(LO(PC));
    --S;
    ++_CurrentCycle;
}

void RP2A03::Cycle_PushPCH_DecrementS() {
    WriteByteToStack(HI(PC));
    --S;
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadDummyStack() {
    ReadByte(StackPage + S);
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchDummy() {
    ReadByte(PC);
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadDummyStack_IncrementS() {
    ReadByte(StackPage + S);
    ++S;
    ++_CurrentCycle;
}

void RP2A03::Cycle_PullPCL_IncrementS() {
    SetLO(PC, ReadByteFromStack());
    ++S;
    ++_CurrentCycle;
}

void RP2A03::Cycle_PullPCH() {
    SetHI(PC, ReadByteFromStack());
    ++_CurrentCycle;
}

void RP2A03::Cycle_IncrementPC_Operation() {
    ++PC;
    (this->*_Operation)();
    ++_CurrentCycle;
}

////////////////////////////////////////////////////////////////////////////////

void RP2A03::Transfer(Byte value, Byte& to) {
    to = value;
    Z = (to == 0) ? 1 : 0;
    N = SignBit(to);
}

void RP2A03::Branch(bool condition) {
    if (condition) {
        if constexpr (BRANCH_USING_WORDOPERAND) {
            _WordOperand = PC + SignExtend(_ByteOperand);
        } else {
            _WordOperand = PC + _ByteOperand;
        }
    } else {
        _WordOperand = PC;
    }
}

void RP2A03::Jump() {
    PC = _WordOperand;
}

void RP2A03::RotateLeft(Byte& value, Flag bit) {
    C = Bit<BYTE_MSB_BIT>(value);
    const auto rotated{ static_cast<Byte>(
        (value << 1) | Mask<BYTE_LSB_BIT>(bit)
    )};
    Transfer(rotated, value);
}

void RP2A03::RotateRight(Byte& value, Flag bit) {
    C = Bit<BYTE_LSB_BIT>(value);
    const auto rotated{ static_cast<Byte>(
        (value >> 1) | Mask<BYTE_MSB_BIT>(bit)
    )};
    Transfer(rotated, value);
}


////////////////////////////////////////////////////////////////////////////////

// TODO - Performance
// Can those uses of _ByteOperand be replaced by direct memory reads?
//
// For example, set up _WordOperand with the address to read or write to
// before calling LDA, in order to cover immediate, absolute and indirect
// addressing modes.
//
// Alternatively, have the memory operation moved into the step-by-step
// and all operations must use _ByteOperand and _WordOperand instead.

void RP2A03::LDA() { Transfer(_ByteOperand, A); }
void RP2A03::LDX() { Transfer(_ByteOperand, X); }
void RP2A03::LDY() { Transfer(_ByteOperand, Y); }

void RP2A03::STA() { WriteByte(_WordOperand, A); }
void RP2A03::STX() { WriteByte(_WordOperand, X); }
void RP2A03::STY() { WriteByte(_WordOperand, Y); }

void RP2A03::BCC() { Branch(C == 0); }
void RP2A03::BCS() { Branch(C == 1); }
void RP2A03::BEQ() { Branch(Z == 1); }
void RP2A03::BMI() { Branch(N == 1); }
void RP2A03::BNE() { Branch(Z == 0); }
void RP2A03::BPL() { Branch(N == 0); }
void RP2A03::BVC() { Branch(V == 0); }
void RP2A03::BVS() { Branch(V == 1); }

void RP2A03::TAX() { Transfer(A, X); }
void RP2A03::TAY() { Transfer(A, Y); }
void RP2A03::TXA() { Transfer(X, A); }
void RP2A03::TYA() { Transfer(Y, A); }

void RP2A03::JMP() { Jump(); }
void RP2A03::JSR() { Jump(); }
void RP2A03::RTS() {} // TODO to call Jump() here, set up _WordOperand in the step-by-step

void RP2A03::TSX() { Transfer(S, X); }
void RP2A03::TXS() { S = X; }

void RP2A03::PHA() { WriteByteToStack(A); }
void RP2A03::PLA() { Transfer(ReadByteFromStack(), A); }
void RP2A03::PHP() { WriteByteToStack(GetStatusByte(1)); }
void RP2A03::PLP() { SetStatusByte(ReadByteFromStack()); }

void RP2A03::ASL() { RotateLeft(_ByteOperand, 0); }
void RP2A03::LSR() { RotateRight(_ByteOperand, 0); }
void RP2A03::ROL() { RotateLeft(_ByteOperand, C); }
void RP2A03::ROR() { RotateRight(_ByteOperand, C); }

void RP2A03::ASL_a() { RotateLeft(A, 0); }
void RP2A03::LSR_a() { RotateRight(A, 0); }
void RP2A03::ROL_a() { RotateLeft(A, C); }
void RP2A03::ROR_a() { RotateRight(A, C); }

void RP2A03::DEC() { Transfer(_ByteOperand - 1, _ByteOperand); }
void RP2A03::DEX() { Transfer(X - 1, X); }
void RP2A03::DEY() { Transfer(Y - 1, Y); }
void RP2A03::INC() { Transfer(_ByteOperand + 1, _ByteOperand); }
void RP2A03::INX() { Transfer(X + 1, X); }
void RP2A03::INY() { Transfer(Y + 1, Y); }

void RP2A03::CLC() { C = 0; }
void RP2A03::CLD() { D = 0; }
void RP2A03::CLI() { I = 0; }
void RP2A03::CLV() { V = 0; }
void RP2A03::SEC() { C = 1; }
void RP2A03::SED() { D = 1; }
void RP2A03::SEI() { I = 1; }

////////////////////////////////////////////////////////////////////////////////

void RP2A03::PowerUp() {
    NOT_IMPLEMENTED();
}

void RP2A03::Reset() {
    NOT_IMPLEMENTED();
}

bool RP2A03::Tick() {
    if (IsStopped()) return false;

    Phi1();
    Phi2();

    return (_InstructionsCycles[_CurrentCycle.Get()]
        == &RP2A03::Cycle_FetchOpcode_IncrementPC);
}

void RP2A03::DMA(Byte page, Byte* target, Byte offset) {
    NOT_IMPLEMENTED();
}

void RP2A03::Phi1() {
    ++Ticks;
    (this->*_InstructionsCycles[_CurrentCycle.Get()])();
}

void RP2A03::Phi2() {
    // TODO Ricoh_RP2A03.h/cpp uses a CheckInterrupts boolean
    // check what this is and if it's actually needed

    // IRQ is level-triggered
    _IRQTriggered = LineIRQ;

    // NMI is edge-triggered
    _NMITriggered = _NMITriggered || (!_PreviousLineNMI && LineNMI);
    _PreviousLineNMI = LineNMI;
}
