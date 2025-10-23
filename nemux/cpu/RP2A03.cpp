#include "cpu/RP2A03.h"


// TODO remove when not needed anymore
namespace {
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
                    *(cycle++) = &RP2A03::Cycle_FetchOperand_IncrementPC;
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

////////////////////////////////////////////////////////////////////////////////

void RP2A03::Transfer(Byte value, Byte& to) {
    to = value;
    Z = (to == 0) ? 1 : 0;
    N = Bit<Neg>(to);
}

////////////////////////////////////////////////////////////////////////////////

void RP2A03::LDA() {
    Transfer(_ByteOperand, A);
}

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
