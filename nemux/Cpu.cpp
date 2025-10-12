#include "Cpu.h"

#include "BitUtil.h"
#include "Ppu.h"
#include "Controllers.h"

#include <iomanip>
#include <sstream>
#include <iostream>

Word Cpu::ReadWordAt(const Word address) const {
    const auto lo{ ReadByte(address) };
    const auto hi{ ReadByte(address + 1) };
    return MakeWord(lo, hi);
}

void Cpu::WriteWordAt(const Word address, const Word value) {
    WriteByte(address, LO(value));
    WriteByte(address + 1, HI(value));
}

/* explicit */ Cpu::Cpu(std::string name, MemoryMap * map)
    : Name{name}
    , InterruptCycles{7}
    , BaseCpu{}
{
    Map = map;

    m_opcodes.resize(
        InstructionSet_6502::INSTRUCTION_COUNT,
        Instruction{ InstructionSet_6502::OpName::UNK }
    );
    
    for (size_t i{ 0 }; i < InstructionSet_6502::INSTRUCTION_COUNT; ++i) {
        m_opcodes[i] = InstructionSet_6502::Decode(i);
    }

    // Power up state
    S = 0xFD;
    SetStatusByte(0x24);
    CurrentTick = GetTicks();
    PendingInterrupt = InterruptType::None;
}

bool Cpu::Tick() {
    ++CurrentTick;
    static auto m = dynamic_cast<CpuMemoryMap<Cpu, Ppu, Controllers, Apu<Cpu>> *>(Map);
    static bool nmi = false;
    static bool nmiDelayed1 = false;
    static bool nmiDelayed2 = false;
    static bool nmiDelayed3 = false;
    static bool nmiDelayed4 = false;
    if (m != nullptr) {
        if (!nmiDelayed4 && nmiDelayed3) {
            TriggerNMI();
        }
        nmiDelayed4 = nmiDelayed3;
        nmiDelayed3 = nmiDelayed2;
        nmiDelayed2 = nmiDelayed1;
        nmiDelayed1 = nmi;
        nmi = m->PPU->NMIActive;

        if (I == 0 && (
            m->APU->Frame.Interrupt ||
            m->APU->DMC1.Output.DMA.Interrupt)) {
            TriggerIRQ();
        }

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

address_t Cpu::BuildAddress(InstructionSet_6502::AddressingMode mode) const {
    using enum InstructionSet_6502::AddressingMode;
    const Word PC_1 = PC + 1;
    switch (mode) {
        case IMM:
        case REL: {
            return { PC_1, false };
        }
        case ZPG: {
            return { ReadByte(PC_1), false };
        }
        case ZPX: {
            const auto address = (ReadByte(PC_1) + X) & WORD_LO_MASK;
            return { static_cast<Word>(address), false };
        }
        case ZPY: {
            const auto address = (ReadByte(PC_1) + Y) & WORD_LO_MASK;
            return { static_cast<Word>(address), false };
        }
        case ABS: {
            return { ReadWordAt(PC_1), false };
        }
        case ABX: {
            const Word address = ReadWordAt(PC_1) + X;
            const bool crossed = (X > (address & BYTE_MASK));
            if (crossed) ReadByte(address - 0x0100); // Dummy read
            return { address, crossed };
        }
        case ABY: {
            const Word address = ReadWordAt(PC_1) + Y;
            const bool crossed = (Y > (address & BYTE_MASK));
            if (crossed) ReadByte(address - 0x0100); // Dummy read
            return{ address, crossed };
        }
        case IDX: {
            const Byte base = ReadByte(PC_1) + X;
            const Byte lo = base;
            const Byte hi = (base + Byte(1));
            // const Word base = ReadByte(PC_1) + X;
            // const Word lo = base & WORD_LO_MASK;
            // const Word hi = (base + 1) & WORD_LO_MASK;
            const Word addr = MakeWord(ReadByte(lo), ReadByte(hi));
            return { addr, false };
        }
        case IDY: {
            const Word base = ReadByte(PC_1);
            const Word lo = ReadByte(base);
            const Word hi = ReadByte((base + 1) & WORD_LO_MASK);
            const Word addr = (hi << BYTE_WIDTH) + lo + Y;
            const bool crossed = (Y > (addr & BYTE_MASK));
            if (crossed) ReadByte(addr - 0x0100); // Dummy read
            return { addr, crossed };
        }
        case IND: {
            const Word base = ReadWordAt(PC_1);
            const Word lo = base;
            const Word hi = (base & WORD_HI_MASK) | ((base + 1) & WORD_LO_MASK);
            const Word addr = ReadByte(hi) << BYTE_WIDTH | ReadByte(lo);
            return { addr, false };
        }
        case IMP:
        case ACC: {
            // Dummy fetch of the next opcode
            ReadByte(PC_1);
            return { static_cast<Word>(-1), false };
        }
        default: throw std::runtime_error("Unknown addressing mode");
    }
}

void Cpu::Decrement(Byte & value) {
    Transfer(value - 1, value);
}
void Cpu::Increment(Byte & value) {
    Transfer(value + 1, value);
}
void Cpu::Transfer(const Byte & from, Byte & to) {
    to = from;
    Z = (to == 0) ? 1 : 0;
    N = Bit<Neg>(to);
}
void Cpu::Compare(const Byte lhs, const Byte rhs) {
    const auto r = lhs - rhs;
    C = (r >= 0) ? 1 : 0;
    Z = (r == 0) ? 1 : 0;
    N = (IsBitSet<BYTE_SIGN_BIT>(r)) ? 1 : 0;
}

void Cpu::BranchIf(bool condition, Byte offset) {
    const auto basePC = PC;
    if (condition) {
        Word wOffset = SignExtend(offset);
        PC = (PC + wOffset) & WORD_MASK;
        Ticks += 1;
        if ((PC & WORD_HI_MASK) != (basePC & WORD_HI_MASK)) {
            Ticks += 1;
        }
    }
}

void Cpu::AddWithCarry(const Byte value) {
    Word a = A + value + C;
    C = (a > BYTE_MASK) ? 1 : 0;
    V = ~Bit<Neg>(A ^ value) & Bit<Neg>(A ^ a);
    Transfer(a & BYTE_MASK, A);
}
void Cpu::SubstractWithCarry(const Byte value) {
    Word a = A - value - (1 - C);
    C = (a > BYTE_MASK) ? 0 : 1;
    V = Bit<Neg>(A ^ value) & Bit<Neg>(A ^ a);
    Transfer(a & BYTE_MASK, A);
}
void Cpu::Jump(const Word address) {
    PC = address;
}
void Cpu::Push(const Byte & value) {
    WriteByte(StackPage + S, value);
    --S;
}
Byte Cpu::Pull() {
    ++S;
    return ReadByte(StackPage + S);
}
void Cpu::PushWord(const Word & value) {
    Push((value >> BYTE_WIDTH) & BYTE_MASK);
    Push(value & BYTE_MASK);
}
Word Cpu::PullWord() {
    return Pull() | (Pull() << BYTE_WIDTH);
}
void Cpu::Interrupt(const Flag & isBRK,
                    const Word & vector,
                    const bool readOnly /*= false*/) {
    if (readOnly) {
        S -= 3;
    } else {
        PushWord(PC);
        Push(GetStatusByte(isBRK));
    }
    I = 1;
    PC = ReadWordAt(vector);
    Ticks += InterruptCycles;
}

void Cpu::PowerUp() {
    PC = ReadWordAt(VectorRST);
}

void Cpu::Reset() {
    std::cout << "RST" << std::endl;
    PendingInterrupt = InterruptType::None;
    Interrupt(0, VectorRST, true);
}

void Cpu::NMI() {
    static int nmic{ 0 };
    std::cout << "NMI " << nmic++ << std::endl;
    PendingInterrupt = InterruptType::None;
    Interrupt(0, VectorNMI);
}
void Cpu::IRQ() {
    std::cout << "IRQ" << std::endl;
    PendingInterrupt = InterruptType::None;
    Interrupt(0, VectorIRQ);
}
void Cpu::TriggerReset() {
    PendingInterrupt = InterruptType::Rst;
}
void Cpu::TriggerNMI() {
    if (PendingInterrupt != InterruptType::Rst) {
        PendingInterrupt = InterruptType::Nmi;
    }
}
void Cpu::TriggerIRQ() {
    if (PendingInterrupt == InterruptType::None) {
        PendingInterrupt = InterruptType::Irq;
    }
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

    const auto a = BuildAddress(op.Mode);
    Ticks += op.Cycles;
    PC += op.Bytes;

    switch (op.Name) {
    using enum InstructionSet_6502::OpName;
    using enum InstructionSet_6502::AddressingMode;
    case BRK: Interrupt(1, VectorIRQ); break;
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
    case BCC: BranchIf(C == 0, ReadByte(a.Address)); break;
    case BCS: BranchIf(C == 1, ReadByte(a.Address)); break;
    case BEQ: BranchIf(Z == 1, ReadByte(a.Address)); break;
    case BMI: BranchIf(N == 1, ReadByte(a.Address)); break;
    case BNE: BranchIf(Z == 0, ReadByte(a.Address)); break;
    case BPL: BranchIf(N == 0, ReadByte(a.Address)); break;
    case BVC: BranchIf(V == 0, ReadByte(a.Address)); break;
    case BVS: BranchIf(V == 1, ReadByte(a.Address)); break;
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
