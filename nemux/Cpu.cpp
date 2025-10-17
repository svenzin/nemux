#include "Cpu.h"


#include "BitUtil.h"
#include "Controllers.h"
#include "Ppu.h"


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
    : InterruptCycles{ 7 }
    , BaseCpu{}
{
    Name = name;

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
    PreviousNMI = LineNMI;
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

void Cpu::Reset() {
    Ticks += InterruptCycles;
    Interrupt(0, VectorRST, true);
}

void Cpu::NMI() {
    Ticks += InterruptCycles;
    Interrupt(0, VectorNMI, false);
}

void Cpu::IRQ() {
    Ticks += InterruptCycles;
    Interrupt(0, VectorIRQ, false);
}

////////////////////////////////////////////////////////////////////////////////

bool Cpu::Tick() {
    ++CurrentTick;
    {
        if (LineRST) {
            PendingInterrupt = InterruptType::Rst;
        } else if (!PreviousNMI && LineNMI && (PendingInterrupt != InterruptType::Rst)) {
            PendingInterrupt = InterruptType::Nmi;
        } else if ((I == 0) && LineIRQ && (PendingInterrupt == InterruptType::None)) {
            PendingInterrupt = InterruptType::Irq;
        }
        PreviousNMI = LineNMI;
    }
    
    if (CurrentTick > Ticks) {
        if (PendingInterrupt == InterruptType::Rst) {
            PendingInterrupt = InterruptType::None;
            Reset();
        }
        else if (PendingInterrupt == InterruptType::Nmi) {
            PendingInterrupt = InterruptType::None;
            NMI();
        }
        else if (PendingInterrupt == InterruptType::Irq) {
            PendingInterrupt = InterruptType::None;
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

        default: {
            throw std::runtime_error("Unknown addressing mode");
        }
    }
}

void Cpu::Execute(const Instruction &op) {
    if (IsStopped()) return;

    const auto operand{ BuildAddress(op) };
    Ticks += op.Cycles;
    PC += op.Bytes;

    switch (op.Name) {
        using InstructionSet_6502::AddressingMode::ACC;
        using enum InstructionSet_6502::OpName;
        case BRK: {
            if (I == 0) {
                Interrupt(1, VectorIRQ, false);
            }
            break;
        }
        case JMP: {
            Jump(operand.Address);
            break;
        }
        case JSR: {
            PushWord(PC - 1);
            Jump(operand.Address);
            break;
        }
        case RTS: {
            Jump(PullWord() + 1);
            break;
        }
        case RTI: {
            SetStatusByte(Pull());
            Jump(PullWord());
            break;
        }
        case PLP: {
            SetStatusByte(Pull());
            break;
        }
        case PHP: {
            Push(GetStatusByte(1));
            break;
        }
        case PHA: {
            Push(A);
            break;
        }
        case PLA: {
            Transfer(Pull(), A);
            break;
        }
        case LDA: {
            Transfer(ReadByte(operand.Address), A);
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case LDX: {
            Transfer(ReadByte(operand.Address), X);
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case LDY: {
            Transfer(ReadByte(operand.Address), Y);
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case EOR: {
            Transfer(A ^ ReadByte(operand.Address), A);
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case ORA: {
            const auto m = ReadByte(operand.Address);
            Transfer(A | m, A);
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case CMP: {
            Compare(A, ReadByte(operand.Address));
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case CPX: {
            Compare(X, ReadByte(operand.Address));
            break;
        }
        case CPY: {
            Compare(Y, ReadByte(operand.Address));
            break;
        }
        case TAX: {
            Transfer(A, X);
            break;
        }
        case TAY: {
            Transfer(A, Y);
            break;
        }
        case TSX: {
            Transfer(S, X);
            break;
        }
        case TXA: {
            Transfer(X, A);
            break;
        }
        case TXS: {
            // TXS does not change the flags
            S = X;
            break;
        }
        case TYA: {
            Transfer(Y, A);
            break;
        }
        case STA: {
            WriteByte(operand.Address, A);
            break;
        }
        case STX: {
            WriteByte(operand.Address, X);
            break;
        }
        case STY: {
            WriteByte(operand.Address, Y);
            break;
        }
        case BCC: {
            BranchIf(C == 0, operand);
            break;
        }
        case BCS: {
            BranchIf(C == 1, operand);
            break;
        }
        case BEQ: {
            BranchIf(Z == 1, operand);
            break;
        }
        case BMI: {
            BranchIf(N == 1, operand);
            break;
        }
        case BNE: {
            BranchIf(Z == 0, operand);
            break;
        }
        case BPL: {
            BranchIf(N == 0, operand);
            break;
        }
        case BVC: {
            BranchIf(V == 0, operand);
            break;
        }
        case BVS: {
            BranchIf(V == 1, operand);
            break;
        }
        case ADC: {
            AddWithCarry(ReadByte(operand.Address));
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case uSBC: [[fallthrough]];
        case SBC: {
            SubstractWithCarry(ReadByte(operand.Address));
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case ASL: {
            if (op.Mode == ACC) {
                C = Bit<BYTE_MSB_BIT>(A);
                Transfer(A << 1, A);
            } else {
                auto M{ ReadByte(operand.Address) };
                C = Bit<BYTE_MSB_BIT>(M);
                Transfer(M << 1, M);
                WriteByte(operand.Address, M);
            }
            break;
        }
        case LSR: {
            if (op.Mode == ACC) {
                C = Bit<BYTE_LSB_BIT>(A);
                Transfer(A >> 1, A);
            } else {
                auto M{ ReadByte(operand.Address) };
                C = Bit<BYTE_LSB_BIT>(M);
                Transfer(M >> 1, M);
                WriteByte(operand.Address, M);
            }
            break;
        }
        case ROL: {
            if (op.Mode == ACC) {
                const auto c{ Bit<BYTE_MSB_BIT>(A) };
                Transfer((A << 1) | Mask<BYTE_LSB_BIT>(C == 1), A);
                C = c;
            } else {
                auto M{ ReadByte(operand.Address) };
                const auto c{ Bit<BYTE_MSB_BIT>(M) };
                Transfer((M << 1) | Mask<BYTE_LSB_BIT>(C == 1), M);
                C = c;
                WriteByte(operand.Address, M);
            }
            break;
        }
        case ROR: {
            if (op.Mode == ACC) {
                const auto c{ Bit<BYTE_LSB_BIT>(A) };
                Transfer((A >> 1) | Mask<BYTE_MSB_BIT>(C == 1), A);
                C = c;
            } else {
                auto M{ ReadByte(operand.Address) };
                const auto c{ Bit<BYTE_LSB_BIT>(M) };
                Transfer((M >> 1) | Mask<BYTE_MSB_BIT>(C == 1), M);
                C = c;
                WriteByte(operand.Address, M);
            }
            break;
        }
        case AND: {
            Transfer(A & ReadByte(operand.Address), A);
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case BIT: {
            const auto mask{ ReadByte(operand.Address) };
            Z = ((mask & A) == 0) ? 1 : 0;
            V = Bit<Ovf>(mask);
            N = Bit<Neg>(mask);
            break;
        }
        case CLC: {
            C = 0;
            break;
        }
        case CLD: {
            D = 0;
            break;
        }
        case CLI: {
            I = 0;
            break;
        }
        case CLV: {
            V = 0;
            break;
        }
        case SEC: {
            C = 1;
            break;
        }
        case SED: {
            D = 1;
            break;
        }
        case SEI: {
            I = 1;
            break;
        }
        case DEC: {
            auto M{ ReadByte(operand.Address) };
            Decrement(M);
            WriteByte(operand.Address, M);
            break;
        }
        case DEX: {
            Decrement(X);
            break;
        }
        case DEY: {
            Decrement(Y);
            break;
        }
        case INC: {
            auto M{ ReadByte(operand.Address) };
            Increment(M);
            WriteByte(operand.Address, M);
            break;
        }
        case INX: {
            Increment(X);
            break;
        }
        case INY: {
            Increment(Y);
            break;
        }
        case uSTP: {
            _isStopped = true;
            break;
        }
        case uSLO: {
            auto M{ ReadByte(operand.Address) };
            C = Bit<BYTE_MSB_BIT>(M);
            Transfer(M << 1, M);
            WriteByte(operand.Address, M);
            Transfer(A | M, A);
            break;
        }
        case uANC: {
            const auto M{ ReadByte(operand.Address) };
            Transfer(A & M, A);
            C = Bit<BYTE_MSB_BIT>(A);
            break;
        }
        case uRLA: {
            auto M{ ReadByte(operand.Address) };
            const auto c{ Bit<BYTE_MSB_BIT>(M) };
            Transfer((M << 1) | Mask<BYTE_LSB_BIT>(C == 1), M);
            C = c;
            WriteByte(operand.Address, M);
            Transfer(A & M, A);
            break;
        }
        case uSRE: {
            auto M{ ReadByte(operand.Address) };
            C = Bit<BYTE_LSB_BIT>(M);
            Transfer(M >> 1, M);
            WriteByte(operand.Address, M);
            Transfer(A ^ M, A);
            break;
        }
        case uALR: {
            const auto M{ ReadByte(operand.Address) };
            Transfer(A & M, A);
            C = Bit<BYTE_LSB_BIT>(A);
            Transfer(A >> 1, A);
            break;
        }
        case uRRA: {
            auto M{ ReadByte(operand.Address) };
            const auto c{ Bit<BYTE_LSB_BIT>(M) };
            Transfer((M >> 1) | Mask<BYTE_MSB_BIT>(C == 1), M);
            C = c;
            WriteByte(operand.Address, M);
            AddWithCarry(M);
            break;
        }
        case uARR: {
            const auto M{ ReadByte(operand.Address) };
            Transfer(A & M, A);
            Transfer((A >> 1) | Mask<BYTE_MSB_BIT>(C == 1), A);
            switch ((A >> 5) & 0x03) {
                case 0: { C = 0; V = 0; break; }
                case 1: { C = 0; V = 1; break; }
                case 2: { C = 1; V = 1; break; }
                case 3: { C = 1; V = 0; break; }
            }
            break;
        }
        case uSAX: {
            WriteByte(operand.Address, A & X);
            break;
        }
        case uAHX: {
            const auto H{ HI(operand.Address) };
            WriteByte(operand.Address, A & X & H);
            break;
        }
        case uTAS: {
            // Don't call Transfer() because flags are not updated
            S = (A & X);
            const auto H{ HI(operand.Address) };
            WriteByte(operand.Address, A & X & H);
            break;
        }
        case uSHY: {
            const auto H{ HI(operand.Address) };
            const auto M{ Y & (H + 1) };
            if (operand.HasCrossedPage) {
                // In case the resulting addres crosses a page
                // The behaviour is corrupted
                // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
                const auto address{ MakeWord(LO(operand.Address), M) };
                WriteByte(address, M);
            } else {
                WriteByte(operand.Address, M);
            }
            break;
        }
        case uSHX: {
            const auto H{ HI(operand.Address) };
            const auto M{ X & (H + 1) };
            if (operand.HasCrossedPage) {
                // In case the resulting addres crosses a page
                // The behaviour is corrupted
                // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
                const auto address{ MakeWord(LO(operand.Address), M) };
                WriteByte(address, M);
            } else {
                WriteByte(operand.Address, M);
            }
            break;
        }
        case uLAX: {
            const auto M{ ReadByte(operand.Address) };
            Transfer(M, A);
            Transfer(M, X);
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case uLAS: {
            const auto M{ ReadByte(operand.Address) };
            Transfer(M & S, S);
            Transfer(S, A);
            Transfer(S, X);
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        case uDCP: {
            auto M{ ReadByte(operand.Address) };
            Decrement(M);
            WriteByte(operand.Address, M);
            Compare(A, M);
            break;
        }
        case uAXS: {
            const auto M{ ReadByte(operand.Address) };
            X = (A & X);
            Compare(X, M); // Flags are set like CMP
            X = X - M;
            break;
        }
        case uISC: {
            auto M{ ReadByte(operand.Address) };
            Increment(M);
            WriteByte(operand.Address, M);
            SubstractWithCarry(M);
            break;
        }
        case NOP: [[fallthrough]];
        case uNOP: [[fallthrough]];
        case uXAA: {
            if (operand.HasCrossedPage) ++Ticks;
            break;
        }
        default: {
            throw std::runtime_error("unknown instruction name");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void Cpu::PowerUp() {
    PC = ReadWordAt(VectorRST);
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
