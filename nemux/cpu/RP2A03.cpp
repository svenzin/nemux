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
                    *(cycle++) = &RP2A03::Cycle_TakeBranch;
                    *(cycle++) = &RP2A03::Cycle_FixBranch;
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

                case IND: {
                    break;
                }

                default: UNREACHABLE();
            }
        };

        auto counter{ CycleCounter::ValueFrom(opcode, 0) };
        FillWithModeCycles(_InstructionsCycles.begin() + counter, instr.Mode, type);
    }

    _CurrentCycle.Set(0xEA, 1);
    _Operation = &RP2A03::Cycle_Unreachable;

    _IRQTriggered = false;
    _NMITriggered = false;
    _PreviousLineNMI = LineNMI;
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
    switch (opname) {
        using enum InstructionSet_6502::OpName;
        case LDA: _Operation = &RP2A03::LDA; break;
        case LDX: _Operation = &RP2A03::LDX; break;
        case LDY: _Operation = &RP2A03::LDY; break;
        case STA: _Operation = &RP2A03::STA; break;
        case STX: _Operation = &RP2A03::STX; break;
        case STY: _Operation = &RP2A03::STY; break;
        case BCC: _Operation = &RP2A03::BCC; break;
        case BCS: _Operation = &RP2A03::BCS; break;
        case BEQ: _Operation = &RP2A03::BEQ; break;
        case BMI: _Operation = &RP2A03::BMI; break;
        case BNE: _Operation = &RP2A03::BNE; break;
        case BPL: _Operation = &RP2A03::BPL; break;
        case BVC: _Operation = &RP2A03::BVC; break;
        case BVS: _Operation = &RP2A03::BVS; break;
        case TAX: _Operation = &RP2A03::TAX; break;
        case TAY: _Operation = &RP2A03::TAY; break;
        case TXA: _Operation = &RP2A03::TXA; break;
        case TYA: _Operation = &RP2A03::TYA; break;
        default: _Operation = &RP2A03::Cycle_Unreachable; break;
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
    const auto hi{ ReadByte(LO(_WordOperand + 1)) };
    _WordOperand = MakeWord(_ByteOperand, hi);
    ++_CurrentCycle;
}

void RP2A03::Cycle_ReadAddressHI_IndexY() {
    const auto hi{ ReadByte(LO(_WordOperand + 1)) };
    _WordOperand = MakeWord(_ByteOperand + Y, hi);
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

void RP2A03::Cycle_TakeBranch() {
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

void RP2A03::Cycle_FixBranch() {
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

////////////////////////////////////////////////////////////////////////////////

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
