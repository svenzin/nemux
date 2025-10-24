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

    template <RP2A03::ActionT OP>
    void FillReadWriteRMW(RP2A03::CycleT*& cycle, AddressingType type) {
        switch (type) {
            using enum AddressingType;
            
            case Read: {
                *(cycle++) = &RP2A03::Cycle_ReadOperand<OP>;
                break;
            }
            case Write: {
                *(cycle++) = &RP2A03::Cycle<OP>;
                break;
            }
            case ReadModifyWrite: {
                *(cycle++) = &RP2A03::Cycle_ReadOperand<>;
                *(cycle++) = &RP2A03::Cycle_ReadOperand<OP>;
                *(cycle++) = &RP2A03::Cycle_WriteOperand<>;
                break;
            }
            default: UNREACHABLE();
        }
    }
    
    template <RP2A03::ActionT OP>
    void FillCycles(RP2A03::CycleT* cycle,
                    InstructionSet_6502::AddressingMode mode,
                    AddressingType type)
    {
        switch (mode) {
            using enum InstructionSet_6502::AddressingMode;
            using enum AddressingType;
            
            case IMP: [[fallthrough]];
            case ACC: {
                *(cycle++) = &RP2A03::Cycle_FetchDummy<OP>;
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case IMM: {
                *(cycle++) = &RP2A03::Cycle_FetchOperand_IncrementPC<OP>;
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case ZPG: {
                *(cycle++) = &RP2A03::Cycle_FetchZeroPageAddress_IncrementPC;
                FillReadWriteRMW<OP>(cycle, type);
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case ZPX: {
                *(cycle++) = &RP2A03::Cycle_FetchZeroPageAddress_IncrementPC;
                *(cycle++) = &RP2A03::Cycle_ReadOperand_IndexX;
                FillReadWriteRMW<OP>(cycle, type);
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case ZPY: {
                *(cycle++) = &RP2A03::Cycle_FetchZeroPageAddress_IncrementPC;
                *(cycle++) = &RP2A03::Cycle_ReadOperand_IndexY;
                FillReadWriteRMW<OP>(cycle, type);
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case ABS: {
                *(cycle++) = &RP2A03::Cycle_FetchAddressLO_IncrementPC;
                *(cycle++) = &RP2A03::Cycle_FetchAddressHI_IncrementPC;
                FillReadWriteRMW<OP>(cycle, type);
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case ABX: {
                *(cycle++) = &RP2A03::Cycle_FetchAddressLO_IncrementPC;
                *(cycle++) = &RP2A03::Cycle_FetchAddressHI_IndexX_IncrementPC;
                switch (type) {
                    case Read: {
                        *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexX_Try<OP>;
                        break;
                    }
                    case Write: {
                        *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexX;
                        break;
                    }
                    case ReadModifyWrite: {
                        *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexX;
                        break;
                    }
                    default: UNREACHABLE();
                }
                FillReadWriteRMW<OP>(cycle, type);
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case ABY: {
                *(cycle++) = &RP2A03::Cycle_FetchAddressLO_IncrementPC;
                *(cycle++) = &RP2A03::Cycle_FetchAddressHI_IndexY_IncrementPC;
                switch (type) {
                    case Read: {
                        *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY_Try<OP>;
                        break;
                    }
                    case Write: {
                        *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY;
                        break;
                    }
                    case ReadModifyWrite: {
                        *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY;
                        break;
                    }
                    default: UNREACHABLE();
                }
                FillReadWriteRMW<OP>(cycle, type);
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case REL: {
                *(cycle++) = &RP2A03::Cycle_FetchOperand_IncrementPC<>;
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case IDX: {
                *(cycle++) = &RP2A03::Cycle_FetchZeroPageAddress_IncrementPC;
                *(cycle++) = &RP2A03::Cycle_ReadOperand_IndexX; // Dummy operand read needed to apply index
                *(cycle++) = &RP2A03::Cycle_ReadAddressLO;
                *(cycle++) = &RP2A03::Cycle_ReadAddressHI;
                FillReadWriteRMW<OP>(cycle, type);
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case IDY: {
                *(cycle++) = &RP2A03::Cycle_FetchZeroPageAddress_IncrementPC;
                *(cycle++) = &RP2A03::Cycle_ReadAddressLO;
                *(cycle++) = &RP2A03::Cycle_ReadAddressHI_IndexY;
                switch (type) {
                    case Read: {
                        *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY_Try<OP>;
                        break;
                    }
                    case Write: {
                        *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY;
                        break;
                    }
                    case ReadModifyWrite: {
                        *(cycle++) = &RP2A03::Cycle_ReadOperand_FixHI_IndexY;
                        break;
                    }
                    default: UNREACHABLE();
                }
                FillReadWriteRMW<OP>(cycle, type);
                *(cycle++) = &RP2A03::Cycle_FetchOpcode_IncrementPC;
                break;
            }

            case IND: {
                break;
            }

            default: UNREACHABLE();
        }
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
        const auto counter{ CycleCounter::ValueFrom(opcode, 0) };
        const auto opname{ InstructionSet_6502::LUT::OpcodeNames[opcode] };
        [opname] {
            switch (opname) {
                using enum InstructionSet_6502::OpName;
                case LDA: return FillCycles<&RP2A03::LDA>;
                case LDX: return FillCycles<&RP2A03::LDX>;
                case LDY: return FillCycles<&RP2A03::LDY>;
                case STA: return FillCycles<&RP2A03::STA>;
                case STX: return FillCycles<&RP2A03::STX>;
                case STY: return FillCycles<&RP2A03::STY>;
                default:  return FillCycles<&RP2A03::Cycle_Unreachable>;
            }
        }()(_InstructionsCycles.begin() + counter, instr.Mode, type);
    }

    _CurrentCycle.Set(0xEA, 1);

    _IRQTriggered = false;
    _NMITriggered = false;
    _PreviousLineNMI = LineNMI;
}

////////////////////////////////////////////////////////////////////////////////

void RP2A03::Cycle_GetAddressHI(Word from, Byte index) {
    _WordOperand = MakeWord(_ByteOperand + index, ReadByte(from));
    ++_CurrentCycle;
}

////////////////////////////////////////////////////////////////////////////////

void RP2A03::Cycle_Unreachable() {
    UNREACHABLE();
}

void RP2A03::Cycle_FetchOpcode_IncrementPC() {
    const Byte opcode{ ReadByte(PC) };
    ++PC;
    
    _CurrentCycle.Set(opcode, 0);

    _CurrentOpcode = opcode;
    _CurrentInstruction = InstructionSet_6502::Decode(opcode);
    _CurrentBegin.Set(opcode, 0);
}

void RP2A03::Cycle_FetchZeroPageAddress_IncrementPC() {
    // TODO might be replaced by Cycle_FetchOperand_IncrementPC and using _ByteOperand as address
    _WordOperand = ReadByte(PC);
    ++PC;
    ++_CurrentCycle;
}

void RP2A03::Cycle_FetchAddressLO_IncrementPC()        { Cycle_FetchOperand_IncrementPC<>(); }

void RP2A03::Cycle_FetchAddressHI_IncrementPC()        { Cycle_GetAddressHI(PC++, 0); }
void RP2A03::Cycle_FetchAddressHI_IndexX_IncrementPC() { Cycle_GetAddressHI(PC++, X); }
void RP2A03::Cycle_FetchAddressHI_IndexY_IncrementPC() { Cycle_GetAddressHI(PC++, Y); }

void RP2A03::Cycle_ReadAddressLO()        { Cycle_ReadOperand<>(); }

void RP2A03::Cycle_ReadAddressHI()        { Cycle_GetAddressHI(LO(_WordOperand + 1), 0); }
void RP2A03::Cycle_ReadAddressHI_IndexY() { Cycle_GetAddressHI(LO(_WordOperand + 1), Y); }

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

void RP2A03::Cycle_ReadOperand_FixHI_IndexY() {
    _ByteOperand = ReadByte(_WordOperand);
    if (LO(_WordOperand) < Y) {
        _WordOperand += Word{ 0x0100 };
    }
    ++_CurrentCycle;
}

////////////////////////////////////////////////////////////////////////////////

template <RP2A03::ActionT OP = (RP2A03::ActionT)nullptr>
void RP2A03::Cycle() {
    if constexpr (OP) {
        (this->*OP)();
    }
    ++_CurrentCycle;
}

template <RP2A03::ActionT OP = (RP2A03::ActionT)nullptr>
void RP2A03::Cycle_FetchDummy() {
    ReadByte(PC);
    if constexpr (OP) {
        (this->*OP)();
    }
    ++_CurrentCycle;
}

template <RP2A03::ActionT OP = (RP2A03::ActionT)nullptr>
void RP2A03::Cycle_FetchOperand_IncrementPC() {
    _ByteOperand = ReadByte(PC);
    ++PC;
    if constexpr (OP) {
        (this->*OP)();
    }
    ++_CurrentCycle;
}

template <RP2A03::ActionT OP = (RP2A03::ActionT)nullptr>
void RP2A03::Cycle_ReadOperand() {
    _ByteOperand = ReadByte(_WordOperand);
    if constexpr (OP) {
        (this->*OP)();
    }
    ++_CurrentCycle;
}

template <RP2A03::ActionT OP = (RP2A03::ActionT)nullptr>
void RP2A03::Cycle_WriteOperand() {
    WriteByte(_WordOperand, _ByteOperand);
    if constexpr (OP) {
        (this->*OP)();
    }
    ++_CurrentCycle;
}

template <RP2A03::ActionT OP>
void RP2A03::Cycle_ReadOperand_FixHI_IndexX_Try() {
    _ByteOperand = ReadByte(_WordOperand);
    if (LO(_WordOperand) < X) {
        _WordOperand += Word{ 0x0100 };
    } else {
        if constexpr (OP) {
            (this->*OP)();
        }
        ++_CurrentCycle; // skip the extra cycle
    }
    ++_CurrentCycle;
}

template <RP2A03::ActionT OP>
void RP2A03::Cycle_ReadOperand_FixHI_IndexY_Try() {
    _ByteOperand = ReadByte(_WordOperand);
    if (LO(_WordOperand) < Y) {
        _WordOperand += Word{ 0x0100 };
    } else {
        if constexpr (OP) {
            (this->*OP)();
        }
        ++_CurrentCycle; // skip the extra cycle
    }
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

void RP2A03::LDX() {
    Transfer(_ByteOperand, X);
}

void RP2A03::LDY() {
    Transfer(_ByteOperand, Y);
}

void RP2A03::STA() {
    WriteByte(_WordOperand, A);
}

void RP2A03::STX() {
    WriteByte(_WordOperand, X);
}

void RP2A03::STY() {
    WriteByte(_WordOperand, Y);
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

    const auto current{ _InstructionsCycles[_CurrentCycle.Get()] };
    return (current == &RP2A03::Cycle_FetchOpcode_IncrementPC);
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
