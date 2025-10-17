#include "Cpu.h"


#include "BitUtil.h"
#include "Controllers.h"
#include "Ppu.h"

#include <iomanip>
#include <iostream>
#include <sstream>


////////////////////////////////////////////////////////////////////////////////

Word Cpu::ReadWordAt(Word address) const {
    const auto lo{ ReadByte(address) };
    const auto hi{ ReadByte(address + 1) };
    return MakeWord(lo, hi);
}

void Cpu::WriteWordAt(Word address, Word value) {
    WriteByte(address, LO(value));
    WriteByte(address + 1, HI(value));
}

void Cpu::PushWord(Word value) {
    Push(HI(value));
    Push(LO(value));
}

Word Cpu::PullWord() {
    const auto lo{ Pull() };
    const auto hi{ Pull() };
    return MakeWord(lo, hi);
}

////////////////////////////////////////////////////////////////////////////////

Cpu::Cpu(const std::string& name, MemoryMap* map)
    : Name{ name }
    , InterruptCycles{ 7 }
    , BaseCpu{}
{
    Map = map;

    // m_opcodes.fill({ InstructionSet_6502::OpName::UNK });
    for (size_t i{ 0 }; i < m_opcodes.size(); ++i) {
        m_opcodes[i] = InstructionSet_6502::Decode(i);
    }

    // Power up state
    S = 0xFD;
    SetStatusByte(0x24);
    CurrentTick = GetTicks();
    PendingInterrupt = InterruptType::None;
}

void Cpu::Decrement(Byte& value) {
    Transfer(value - 1, value);
}

void Cpu::Increment(Byte& value) {
    Transfer(value + 1, value);
}

void Cpu::Transfer(Byte value, Byte & to) {
    to = value;
    Z = (to == 0) ? 1 : 0;
    N = SignBit(to);
}

void Cpu::Compare(Byte lhs, Byte rhs) {
    C = (lhs >= rhs) ? 1 : 0;
    Z = (lhs == rhs) ? 1 : 0;
    N = SignBit(lhs - rhs);
}

void Cpu::BranchIf(bool condition, address_t target) {
    if (condition) {
        PC = target.Address;
        Ticks += target.HasCrossedPage ? 2 : 1;
    }
}

void Cpu::AddWithCarry(Byte value) {
    // Overflow checks that the sign has improperly changed
    // See https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
    const auto a{ static_cast<Word>(A + value + C) };
    C = (a > BYTE_MASK) ? 1 : 0;
    V = ~SignBit(A ^ value) & SignBit(A ^ a);
    Transfer(LO(a), A);
}

void Cpu::SubstractWithCarry(Byte value) {
    // Overflow checks that the sign has improperly changed
    // See https://stackoverflow.com/questions/16845912/determining-carry-and-overflow-flag-in-6502-emulation-in-java
    const auto a{ static_cast<Word>(A - value - (1 - C)) };
    C = (a > BYTE_MASK) ? 0 : 1;
    V = SignBit(A ^ value) & SignBit(A ^ a);
    Transfer(LO(a), A);
}

void Cpu::Jump(Word address) {
    PC = address;
}

void Cpu::Push(Byte value) {
    WriteByte(StackPage + S, value);
    --S;
}

Byte Cpu::Pull() {
    ++S;
    return ReadByte(StackPage + S);
}

void Cpu::Interrupt(bool isBRK, Word vector, bool isReadOnly) {
    if (isReadOnly) {
        S -= 3;
    } else {
        PushWord(PC);
        Push(GetStatusByte(isBRK));
    }
    I = 1;
    PC = ReadWordAt(vector);
}

////////////////////////////////////////////////////////////////////////////////

address_t Cpu::BuildAddress(const Instruction& op) const {
    const auto PC_1{ static_cast<Word>(PC + 1) };

    switch (op.Mode) {
        using enum InstructionSet_6502::AddressingMode;

        case IMP: [[fallthrough]];
        case ACC: {
            // Dummy fetch of the next opcode
            ReadByte(PC_1);
            return { static_cast<Word>(-1), false };
        }

        case IMM: {
            return { PC_1, false };
        }

        case ZPG: {
            return { ReadByte(PC_1), false };
        }

        case ZPX: {
            return { LO(ReadByte(PC_1) + X), false };
        }

        case ZPY: {
            return { LO(ReadByte(PC_1) + Y), false };
        }

        case ABS: {
            return { ReadWordAt(PC_1), false };
        }

        case ABX: {
            const auto address{ static_cast<Word>(ReadWordAt(PC_1) + X) };
            const bool crossed{ X > LO(address) };
            if (crossed) ReadByte(address - 0x0100); // Dummy read
            return { address, crossed };
        }

        case ABY: {
            const auto address{ static_cast<Word>(ReadWordAt(PC_1) + Y) };
            const bool crossed{ Y > LO(address) };
            if (crossed) ReadByte(address - 0x0100); // Dummy read
            return{ address, crossed };
        }

        case IDX: {
            // "indirect X" reads an address strictly from ZeroPage at (operand + X)
            // wrapping around to the beginning of ZeroPage if necessary
            const Byte addrLO{ LO(ReadByte(PC_1) + X) };
            const Byte addrHI{ LO(addrLO + 1) };
            const Byte lo{ ReadByte(addrLO) };
            const Byte hi{ ReadByte(addrHI) };
            return { MakeWord(lo, hi), false };
        }

        case IDY: {
            // "indirect Y" reads an address strictly from ZeroPage at operand
            // wrapping around to the beginning of ZeroPage if necessary
            // and then applies the Y offset
            const Byte addrLO{ ReadByte(PC_1) };
            const Byte addrHI{ LO(addrLO + 1) };
            const Byte lo{ ReadByte(addrLO) };
            const Byte hi{ ReadByte(addrHI) };
            const auto addr{ static_cast<Word>(MakeWord(lo, hi) + Y) };
            const bool crossed{ Y > LO(addr) };
            if (crossed) ReadByte(addr - 0x0100); // Dummy read
            return { addr, crossed };
        }

        case IND: {
            // Indirect will read both LO and HI parts of the address from the same page
            const Word addrLO{ ReadWordAt(PC_1) };
            const Word addrHI{ MakeWord(LO(addrLO + 1), HI(addrLO)) };
            const Byte lo{ ReadByte(addrLO) };
            const Byte hi{ ReadByte(addrHI) };
            return { MakeWord(lo, hi), false };
        }

        case REL: {
            const auto offset{ SignExtend(ReadByte(PC_1)) };
            const auto address{ static_cast<Word>(PC + offset + op.Bytes) };
            return { address, HI(PC) != HI(address) };
        }

        default: throw std::runtime_error("Unknown addressing mode");
    }
}

////////////////////////////////////////////////////////////////////////////////

bool Cpu::Tick() {
    ++CurrentTick;
    {
        static bool nmi = false;
        if (LineRST) {
            PendingInterrupt = InterruptType::Rst;
        } else if (!nmi && LineNMI && (PendingInterrupt != InterruptType::Rst)) {
            PendingInterrupt = InterruptType::Nmi;
        } else if ((I == 0) && LineIRQ && (PendingInterrupt == InterruptType::None)) {
            PendingInterrupt = InterruptType::Irq;
        }
        nmi = LineNMI;
    }
    
    if (CurrentTick > Ticks) {
        if (PendingInterrupt == InterruptType::Rst) {
            Reset();
        }
        else if (PendingInterrupt == InterruptType::Nmi) {
            NMI();
        }
        else if (PendingInterrupt == InterruptType::Irq) {
            IRQ();
        }
        else {
            const auto instruction = ReadByte(PC);
            const auto opcode = InstructionSet_6502::Decode(instruction);
            Execute(opcode);
        }
    }
    return CurrentTick >= Ticks;
}


void Cpu::PowerUp() {
    PC = ReadWordAt(VectorRST);
}

void Cpu::Reset() {
    std::cout << "RST" << std::endl;
    PendingInterrupt = InterruptType::None;
    Ticks += InterruptCycles;
    Interrupt(0, VectorRST, true);
}

void Cpu::NMI() {
    static int nmic{ 0 };
    std::cout << "NMI " << nmic++ << std::endl;
    PendingInterrupt = InterruptType::None;
    Ticks += InterruptCycles;
    Interrupt(0, VectorNMI);
}
void Cpu::IRQ() {
    std::cout << "IRQ" << std::endl;
    PendingInterrupt = InterruptType::None;
    Ticks += InterruptCycles;
    Interrupt(0, VectorIRQ);
}

void Cpu::DMA(Byte page, Byte* target, Byte offset) {
    const Word base = page << BYTE_WIDTH;
    for (Word i = 0; i < 0x0100; ++i) {
        target[LO(i + offset)] = ReadByte(base + i);
    }
    Ticks += 513;
    if (CurrentTick % 2 == 1) {
        ++Ticks;
    }
}

void Cpu::Execute(const Instruction &op) {
    if (IsStopped()) return;

    const auto a = BuildAddress(op);
    Ticks += op.Cycles;
    PC += op.Bytes;

    switch (op.Name) {
    using enum InstructionSet_6502::OpName;
    using enum InstructionSet_6502::AddressingMode;
    case BRK: {
        if (I == 0) {
            Interrupt(1, VectorIRQ);
        }
        break;
    }
    case JMP: Jump(a.Address); break;
    case JSR: {
        PushWord(PC - 1);
        Jump(a.Address);
        break;
    }
    case RTS: Jump(PullWord() + 1); break;
    case RTI: {
        SetStatusByte(Pull());
        Jump(PullWord());
        break;
    }
    case PLP: SetStatusByte(Pull()); break;
    case PHP: {
        Push(GetStatusByte(1));
        break;
    }
    case PHA: Push(A); break;
    case PLA: Transfer(Pull(), A); break;
    case LDA: {
        Transfer(ReadByte(a.Address), A);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case LDX: {
        Transfer(ReadByte(a.Address), X);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case LDY: {
        Transfer(ReadByte(a.Address), Y);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case EOR: {
        Transfer(A ^ ReadByte(a.Address), A);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case ORA: {
        const auto m = ReadByte(a.Address);
        Transfer(A | m, A);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case CMP: {
        Compare(A, ReadByte(a.Address));
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case CPX: Compare(X, ReadByte(a.Address)); break;
    case CPY: Compare(Y, ReadByte(a.Address)); break;
    case TAX: Transfer(A, X); break;
    case TAY: Transfer(A, Y); break;
    case TSX: Transfer(S, X); break;
    case TXA: Transfer(X, A); break;
    case TXS: S = X;          break; // TXS does not change the flags
    case TYA: Transfer(Y, A); break;
    case STA: WriteByte(a.Address, A); break;
    case STX: WriteByte(a.Address, X); break;
    case STY: WriteByte(a.Address, Y); break;
    case BCC: BranchIf(C == 0, a); break;
    case BCS: BranchIf(C == 1, a); break;
    case BEQ: BranchIf(Z == 1, a); break;
    case BMI: BranchIf(N == 1, a); break;
    case BNE: BranchIf(Z == 0, a); break;
    case BPL: BranchIf(N == 0, a); break;
    case BVC: BranchIf(V == 0, a); break;
    case BVS: BranchIf(V == 1, a); break;
    case ADC: {
        const auto M = ReadByte(a.Address);
        AddWithCarry(M);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case uSBC:
    case SBC: {
        const auto M = ReadByte(a.Address);
        SubstractWithCarry(M);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case ASL: {
        const auto address = a.Address;
        if (op.Mode == ACC) {
            C = Bit<Left>(A);
            Transfer(A << 1, A);
        }
        else {
            auto M = ReadByte(address);
            C = Bit<Left>(M);
            Transfer(M << 1, M);
            WriteByte(address, M);
        }
        break;
    }
    case LSR: {
        const auto address = a.Address;
        if (op.Mode == ACC) {
            C = Bit<Right>(A);
            Transfer(A >> 1, A);
        }
        else {
            auto M = ReadByte(address);
            C = Bit<Right>(M);
            Transfer(M >> 1, M);
            WriteByte(address, M);
        }
        break;
    }
    case ROL: {
        const auto address = a.Address;
        if (op.Mode == ACC) {
            const auto c = Bit<Left>(A);
            Transfer((A << 1) | Mask<Right>(C), A);
            C = c;
        }
        else {
            auto M = ReadByte(address);
            const auto c = Bit<Left>(M);
            Transfer((M << 1) | Mask<Right>(C), M);
            C = c;
            WriteByte(address, M);
        }
        break;
    }
    case ROR: {
        const auto address = a.Address;
        if (op.Mode == ACC) {
            const auto c = Bit<Right>(A);
            Transfer((A >> 1) | Mask<Left>(C), A);
            C = c;
        }
        else {
            auto M = ReadByte(address);
            const auto c = Bit<Right>(M);
            Transfer((M >> 1) | Mask<Left>(C), M);
            C = c;
            WriteByte(address, M);
        }
        break;
    }
    case AND: {
        const auto M = ReadByte(a.Address);
        Transfer(A & M, A);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case BIT: {
        const auto mask = ReadByte(a.Address);
        Z = (mask & A) == 0 ? 1 : 0;
        V = Bit<Ovf>(mask);
        N = Bit<Neg>(mask);
        break;
    }
    case CLC: C = 0; break;
    case CLD: D = 0; break;
    case CLI: I = 0; break;
    case CLV: V = 0; break;
    case SEC: C = 1; break;
    case SED: D = 1; break;
    case SEI: I = 1; break;
    case DEC: {
        auto M = ReadByte(a.Address);
        Decrement(M);
        WriteByte(a.Address, M);
        break;
    }
    case DEX: Decrement(X); break;
    case DEY: Decrement(Y); break;
    case INC: {
        auto M = ReadByte(a.Address);
        Increment(M);
        WriteByte(a.Address, M);
        break;
    }
    case INX: Increment(X); break;
    case INY: Increment(Y); break;
    case uSTP: _isStopped = true; break;
    case uSLO: {
        const auto address = a.Address;
        auto M = ReadByte(address);
        C = Bit<Left>(M);
        Transfer(M << 1, M);
        WriteByte(address, M);
        Transfer(A | M, A);
        break;
    }
    case uANC: {
        const auto M = ReadByte(a.Address);
        Transfer(A & M, A);
        C = Bit<Left>(A);
        break;
    }
    case uRLA: {
        const auto address = a.Address;
        auto M = ReadByte(address);
        const auto c = Bit<Left>(M);
        Transfer((M << 1) | Mask<Right>(C), M);
        C = c;
        WriteByte(address, M);
        Transfer(A & M, A);
        break;
    }
    case uSRE: {
        const auto address = a.Address;
        auto M = ReadByte(address);
        C = Bit<Right>(M);
        Transfer(M >> 1, M);
        WriteByte(address, M);
        Transfer(A ^ M, A);
        break;
    }
    case uALR: {
        const auto M = ReadByte(a.Address);
        Transfer(A & M, A);
        C = Bit<Right>(A);
        Transfer(A >> 1, A);
        break;
    }
    case uRRA: {
        const auto address = a.Address;
        auto M = ReadByte(address);
        const auto c = Bit<Right>(M);
        Transfer((M >> 1) | Mask<Left>(C), M);
        C = c;
        WriteByte(address, M);
        AddWithCarry(M);
        break;
    }
    case uARR: {
        const auto address = a.Address;
        const auto M = ReadByte(address);
        Transfer(A & M, A);
        Transfer((A >> 1) | Mask<Left>(C), A);
        switch ((A >> 5) & 0x03) {
        case 0: { C = 0; V = 0; break; }
        case 1: { C = 0; V = 1; break; }
        case 2: { C = 1; V = 1; break; }
        case 3: { C = 1; V = 0; break; }
        }
        break;
    }
    case uSAX: WriteByte(a.Address, A & X); break;
    case uAHX: {
        const auto address = a.Address;
        const auto H = ((address & WORD_HI_MASK) >> BYTE_WIDTH);
        WriteByte(address, A & X & H);
        break;
    }
    case uTAS: {
        const auto address = a.Address;
        const auto H = ((address & WORD_HI_MASK) >> BYTE_WIDTH);
        // Don't transfer because flags are not updated
        S = (A & X);
        WriteByte(address, A & X & H);
        break;
    }
    case uSHY: {
        const auto H = ((a.Address & WORD_HI_MASK) >> BYTE_WIDTH);
        const auto M = (Y & (H + 1));
        if (a.HasCrossedPage) {
            // In case the resulting addres crosses a page
            // The bahviour is corrupted
            // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
            const auto address = (M << BYTE_WIDTH) | (a.Address & WORD_LO_MASK);
            WriteByte(address, M);
        }
        else WriteByte(a.Address, M);
        break;
    }
    case uSHX: {
        const auto H = ((a.Address & WORD_HI_MASK) >> BYTE_WIDTH);
        const auto M = X & (H + 1);
        if (a.HasCrossedPage) {
            // In case the resulting addres crosses a page
            // The bahviour is corrupted
            // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
            const auto address = (M << BYTE_WIDTH) | (a.Address & WORD_LO_MASK);
            WriteByte(address, M);
        }
        else WriteByte(a.Address, M);
        break;
    }
    case uLAX: {
        const auto M = ReadByte(a.Address);
        Transfer(M, A);
        Transfer(M, X);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case uLAS: {
        const auto M = ReadByte(a.Address);
        Transfer(M & S, S);
        Transfer(S, A);
        Transfer(S, X);
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    case uDCP: {
        auto M = ReadByte(a.Address);
        Decrement(M);
        WriteByte(a.Address, M);
        Compare(A, M);
        break;
    }
    case uAXS: {
        auto M = ReadByte(a.Address);
        X = (A & X);
        Compare(X, M); // Flags are set like CMP
        X = X - M;
        break;
    }
    case uISC: {
        const auto address = a.Address;
        auto M = ReadByte(address);
        Increment(M);
        WriteByte(address, M);
        SubstractWithCarry(M);
        break;
    }

    case NOP:
    case uNOP:
    case uXAA: {
        if (a.HasCrossedPage) ++Ticks;
        break;
    }
    default: throw std::runtime_error("unknown instruction name");
    }
}

std::string Cpu::ToString() const {
    using std::hex, std::dec, std::boolalpha;
    using std::setfill, std::setw;
    using std::endl;
    std::ostringstream value;
    value << "Cpu " << Name << std::endl
          << "- Registers PC 0x" << hex << setfill('0') << setw(4) << PC << "(" << dec << PC << ")" << endl
          << "            SP 0x" << hex << setfill('0') << setw(2) << S << "(" << dec << S << ")" << endl
          << "             A 0x" << hex << setfill('0') << setw(2) << A << "(" << dec << A << ")" << endl
          << "             X 0x" << hex << setfill('0') << setw(2) << X << "(" << dec << X << ")" << endl
          << "             Y 0x" << hex << setfill('0') << setw(2) << Y << "(" << dec << Y << ")" << endl
          << "- Flags C " << setw(5) << boolalpha << (C != 0) << endl
          << "        Z " << setw(5) << boolalpha << (Z != 0) << endl
          << "        I " << setw(5) << boolalpha << (I != 0) << endl
          << "        D " << setw(5) << boolalpha << (D != 0) << endl
          << "        V " << setw(5) << boolalpha << (V != 0) << endl
          << "        N " << setw(5) << boolalpha << (N != 0) << endl;
    return value.str();
}

std::string Cpu::ToMiniString() const {
    using std::hex, std::setfill, std::setw;
    std::ostringstream value;
    const auto P = GetStatusByte(0);
    value << "Cpu " << Name
          << " " << CurrentTick << "@" << Ticks
          << " PC=$" << hex << setfill('0') << setw(4) << PC
          << " S=$" << hex << setfill('0') << setw(2) << Word{S}
          << " A=$" << hex << setfill('0') << setw(2) << Word{A}
          << " X=$" << hex << setfill('0') << setw(2) << Word{X}
          << " Y=$" << hex << setfill('0') << setw(2) << Word{Y}
          << " P=$" << hex << setfill('0') << setw(2) << Word{P}
          << " "
          << (C == 0 ? 'c' : 'C')
          << (Z == 0 ? 'z' : 'Z')
          << (I == 0 ? 'i' : 'I')
          << (D == 0 ? 'd' : 'D')
          << (V == 0 ? 'v' : 'V')
          << (N == 0 ? 'n' : 'N');
    return value.str();
}
