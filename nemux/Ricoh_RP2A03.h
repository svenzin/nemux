#ifndef RICOH_RP2A03_H_
#define RICOH_RP2A03_H_

#include "Types.h"
#include "MemoryMap.h"

#include <string>
#include <vector>

#include <iostream>

class Ricoh_RP2A03 {

    template <typename _T, size_t _Size>
    class CircularQueue {
        std::array<_T, _Size> items;
        size_t head, tail;
    
    public:
        explicit CircularQueue() : head(0), tail(0) {}
        
        bool empty() const {
            return head == tail;
        }

        void push(const _T & value) {
            items[head] = value;
            head = (head + 1) % _Size;
        }

        _T pop() {
            const _T value = items[tail];
            tail = (tail + 1) % _Size;
            return value;
        }

        void push_front(const _T & value) {
            tail = (tail - 1 + _Size) % _Size;
            items[tail] = value;
        }

        _T first() const {
            return items[tail];
        }

        _T last() const {
            return items[head];
        }
    };

    static constexpr Word VECTOR_NMI = 0xFFFA;
    static constexpr Word VECTOR_RST = 0xFFFC;
    static constexpr Word VECTOR_IRQ = 0xFFFE;
    static constexpr Word STACK_PAGE = 0x0100;

    enum Bits : size_t {
        Car, Zer, Int, Dec, Brk, Unu, Ovf, Neg,
        Left = 7, Right = 0,
    };

    inline void index_address(const Byte & index) {
        address = (address & WORD_HI_MASK)
            | ((address + index) & WORD_LO_MASK);
    }
    inline void fix_indexed_address(const Byte & index)
    {
        AddressWasFixed = ((address & WORD_LO_MASK) < index);
        if (AddressWasFixed) address += 0x0100;
    }

    inline void fetch_opcode() {
        check_for_interrupts();
        opcode = Map->GetByteAt(PC);
        increment_PC();
        ProcessOpcode();
    }
    inline void increment_PC() { ++PC; }
    inline void read_PC_to_operand() { operand = Map->GetByteAt(PC); }
    inline void push_PCL() { Push(PC); }
    inline void push_PCH() { Push(PC >> 8); }
    inline void push_P() { Push(GetStatus(Pflag)); }
    inline void pull_PCL() { PC = (PC & WORD_HI_MASK) | Pull(); }
    inline void pull_PCH() { PC = (Pull() << BYTE_WIDTH) | (PC & WORD_LO_MASK); }
    inline void pull_P() { SetStatus(Pull()); }
    inline void read_PC_to_address() { address = Map->GetByteAt(PC); }
    inline void read_PC_to_addressLo() { address = (address & WORD_HI_MASK) | Map->GetByteAt(PC); }
    inline void read_PC_to_addressHi() { address = (Map->GetByteAt(PC) << BYTE_WIDTH) | (address & WORD_LO_MASK); }
    inline void read_address_to_operand() { operand = Map->GetByteAt(address); }
    inline void read_operand_to_addressLo() { address = (address & WORD_HI_MASK) | Map->GetByteAt(operand); }
    inline void read_operand_1_to_addressHi() { address = (Map->GetByteAt((operand + 1) & WORD_LO_MASK) << BYTE_WIDTH) | (address & WORD_LO_MASK); }
    inline void index_address_by_X() { index_address(X); }
    inline void index_address_by_Y() { index_address(Y); }
    inline void fix_address_indexed_by_X() { fix_indexed_address(X); }
    inline void fix_address_indexed_by_Y() { fix_indexed_address(Y); }
    inline void write_operand_to_address() { Map->SetByteAt(address, operand); }
    inline void read_vector_to_PCL() { PC = (PC & WORD_HI_MASK) | Map->GetByteAt(vector); }
    inline void read_vector_to_PCH() { PC = (Map->GetByteAt(vector + 1) << BYTE_WIDTH) | (PC & WORD_LO_MASK); }

    inline void read_address_and_operand_to_address() {
        address = (address & WORD_HI_MASK) | ((address + 1) & WORD_LO_MASK);
        address = (Map->GetByteAt(address) << BYTE_WIDTH) | operand;
    }

    inline void move_address_to_operand() { operand = address; }
    inline void queue_read_if_address_fixed();

    inline void poll_interrupts() {
        CheckInterrupts = true;
    }

    inline void end_cycle() { CycleActive = false; }

    // RP2A03 instructions
    ////////////////////////////////////////////////////////////////

    inline void NOP() {}
    
    inline void JMP() { PC = address; }

    inline void BRK() {}
    inline void JSR() {}
    inline void RTI() {}
    inline void RTS() {}

    inline void PHP() { Push(GetStatus(1)); }
    inline void PLP() { SetStatus(Pull()); }
    inline void PHA() { Push(A); }
    inline void PLA() { Transfer(Pull(), A); }

    inline void BCC() { Branch(C == 0); }
    inline void BCS() { Branch(C == 1); }
    inline void BNE() { Branch(Z == 0); }
    inline void BEQ() { Branch(Z == 1); }
    inline void BPL() { Branch(N == 0); }
    inline void BMI() { Branch(N == 1); }
    inline void BVC() { Branch(V == 0); }
    inline void BVS() { Branch(V == 1); }

    inline void CLC() { C = 0; }
    inline void SEC() { C = 1; }
    inline void CLI() { I = 0; }
    inline void SEI() { I = 1; }
    inline void CLV() { V = 0; }
    inline void CLD() { D = 0; }
    inline void SED() { D = 1; }

    inline void INX() { Transfer(X + 1, X); }
    inline void DEX() { Transfer(X - 1, X); }
    inline void INY() { Transfer(Y + 1, Y); }
    inline void DEY() { Transfer(Y - 1, Y); }

    inline void LDA() { Transfer(operand, A); }
    inline void LDX() { Transfer(operand, X); }
    inline void LDY() { Transfer(operand, Y); }

    inline void STA() { Map->SetByteAt(address, A); }
    inline void STX() { Map->SetByteAt(address, X); }
    inline void STY() { Map->SetByteAt(address, Y); }
    
    inline void TAX() { Transfer(A, X); }
    inline void TAY() { Transfer(A, Y); }
    inline void TXA() { Transfer(X, A); }
    inline void TYA() { Transfer(Y, A); }
    inline void TSX() { Transfer(S, X); }
    inline void TXS() { S = X; }

    inline void CMP() { Compare(A, operand); }
    inline void CPX() { Compare(X, operand); }
    inline void CPY() { Compare(Y, operand); }

    inline void ORA() { Transfer(A | operand, A); }
    inline void AND() { Transfer(A & operand, A); }
    inline void EOR() { Transfer(A ^ operand, A); }
    inline void ADC() { AddWithCarry(operand); }
    inline void SBC() { SubstractWithCarry(operand); }
    
    inline void ASLa() { ROLwithFlag(A, 0); }
    inline void ROLa() { ROLwithFlag(A, C); }
    inline void LSRa() { RORwithFlag(A, 0); }
    inline void RORa() { RORwithFlag(A, C); }
    
    inline void ASL();
    inline void ROL();
    inline void LSR();
    inline void ROR();
    inline void DEC();
    inline void INC();

    inline void BIT() { const auto mask = Map->GetByteAt(address); Z = (mask & A) == 0 ? 1 : 0; V = Bit<Ovf>(mask); N = Bit<Neg>(mask); }

    ////////////////////////////////////////////////////////////////

    inline void xNOP() {}
    inline void xHLT() {}

    inline void xSHX() {
        const Byte addrLo = (address & WORD_LO_MASK);
        const Byte addrHi = ((address & WORD_HI_MASK) >> BYTE_WIDTH);
        const auto M = (X & (addrHi + 1));
        if (AddressWasFixed) {
            // In case the resulting addres crosses a page
            // The bahviour is corrupted
            // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
            const Word addr = (M << BYTE_WIDTH) | addrLo;
            Map->SetByteAt(addr, M);
        }
        else {
            const Word addr = (addrHi << BYTE_WIDTH) | addrLo;
            Map->SetByteAt(addr, M);
        }
    }
    inline void xSHY() {
        const Byte addrLo = (address & WORD_LO_MASK);
        const Byte addrHi = ((address & WORD_HI_MASK) >> BYTE_WIDTH);
        const auto M = (Y & (addrHi + 1));
        if (AddressWasFixed) {
            // In case the resulting addres crosses a page
            // The bahviour is corrupted
            // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
            const Word addr = (M << BYTE_WIDTH) | addrLo;
            Map->SetByteAt(addr, M);
        }
        else {
            const Word addr = (addrHi << BYTE_WIDTH) | addrLo;
            Map->SetByteAt(addr, M);
        }
    }

    inline void xSLO();
    inline void xRLA();
    inline void xSRE();
    inline void xRRA();
    inline void xDCP();
    inline void xISC();

    inline void xANC() { Transfer(A & operand, A); C = Bit<Left>(A); }
    inline void xALR() { Transfer(A & operand, A); C = Bit<Right>(A); Transfer(A >> 1, A); }
    inline void xARR() { Transfer(A & operand, A); Transfer((A >> 1) | Mask<Left>(C), A);
        switch ((A >> 5) & 0x03) {
        case 0: { C = 0; V = 0; break; }
        case 1: { C = 0; V = 1; break; }
        case 2: { C = 1; V = 1; break; }
        case 3: { C = 1; V = 0; break; }
        }
    }
    inline void xXAA() {}
    inline void xAHX() { const auto H = ((address & WORD_HI_MASK) >> BYTE_WIDTH); Map->SetByteAt(address, A & X & H); }
    inline void xTAS() { const auto H = ((address & WORD_HI_MASK) >> BYTE_WIDTH); S = (A & X); Map->SetByteAt(address, A & X & H); }
    inline void xLAS() { Transfer(operand & S, S); Transfer(S, A); Transfer(S, X); }
    inline void xAXS() { X = (A & X); Compare(X, operand); X = X - operand; }
    inline void xSBC() { SubstractWithCarry(operand); }
    inline void xSAX() { Map->SetByteAt(address, A & X); }
    inline void xLAX() { Transfer(operand, A); Transfer(operand, X); }

public:
    typedef void(Ricoh_RP2A03::* MicroOp_f)();
    typedef void(Ricoh_RP2A03::* AddressingMode_f)();
    std::array<MicroOp_f, 0x100> uOpCode;

    std::array<AddressingMode_f, 0x100> modes;

    CircularQueue<MicroOp_f, 64> operations;

    Word PC;
    Byte S, A, X, Y;
    Flag N, V, D, I, Z, C;

    MemoryMap * Map;

    size_t Ticks;

    bool CheckInterrupts = true;
    bool IRQ;
    bool NMI;

    bool NMIEdge;
    bool NMIFlipFlop;
    bool IRQLevel;

    Word vector;

    Word address;
    Byte operand;

    inline void branch_fix_PCH();
    void Branch(const bool condition);

    void Transfer(const Byte & from, Byte & to) {
        to = from;
        Z = (to == 0) ? 1 : 0;
        N = Bit<Neg>(to);
    }

    void Compare(const Byte & lhs, const Byte & rhs) {
        const auto r = lhs - rhs;
        C = (r >= 0) ? 1 : 0;
        Z = (r == 0) ? 1 : 0;
        N = (IsBitSet<BYTE_SIGN_BIT>(r)) ? 1 : 0;
    }

    void ROLwithFlag(Byte & value, const Flag flag) {
        C = Bit<Left>(value);
        Transfer((value << 1) | Mask<Right>(flag), value);
    }

    void RORwithFlag(Byte & value, const Flag flag) {
        C = Bit<Right>(value);
        Transfer((value >> 1) | Mask<Left>(flag), value);
    }

    void AddWithCarry(const Byte & value) {
        Word a = A + value + C;
        C = (a > BYTE_MASK) ? 1 : 0;
        V = ~Bit<Neg>(A ^ value) & Bit<Neg>(A ^ a);
        Transfer(a & BYTE_MASK, A);
    }
    void SubstractWithCarry(const Byte & value) {
        Word a = A - value - (1 - C);              // Word a = A + ~value + C
        C = (a > BYTE_MASK) ? 0 : 1;               // C = ~((a > BYTE_MASK) ? 1 : 0);
        V = Bit<Neg>(A ^ value) & Bit<Neg>(A ^ a); // V = ~Bit<Neg>(A ^ ~value) & Bit<Neg>(A ^ a);
        Transfer(a & BYTE_MASK, A);                // Transfer(a & BYTE_MASK, A);
    }

    Byte opcode;
    Flag Pflag;
    bool Halted = false;
    bool AddressWasFixed = false;

    inline void ModeImplied();
    inline void ModeImmediate();
    inline void ModeRelative();
    inline void ModeAbsoluteRead();
    inline void ModeAbsoluteRMW();
    inline void ModeAbsoluteWrite();
    inline void ModeZeropageRead();
    inline void ModeZeropageRMW();
    inline void ModeZeropageWrite();
    inline void ModeZeropageXRead();
    inline void ModeZeropageXRMW();
    inline void ModeZeropageXWrite();
    inline void ModeZeropageYRead();
    inline void ModeZeropageYWrite();
    inline void detailAbsoluteX();
    inline void ModeAbsoluteXRead();
    inline void ModeAbsoluteXRMW();
    inline void ModeAbsoluteXWrite();
    inline void detailAbsoluteY();
    inline void ModeAbsoluteYRead();
    inline void ModeAbsoluteYRMW();
    inline void ModeAbsoluteYWrite();
    inline void ModeIndirectXRead();
    inline void ModeIndirectXRMW();
    inline void ModeIndirectXWrite();
    inline void detailIndirectY();
    inline void ModeIndirectYRead();
    inline void ModeIndirectYRMW();
    inline void ModeIndirectYWrite();

    inline void ModePush();
    inline void ModePull();
    inline void ModeJump();
    inline void ModeJumpIndirect();
    inline void ModeBRK();
    inline void ModeRTI();
    inline void ModeRTS();
    inline void ModeJSR();
    
    bool ProcessOpcode() {
        AddressingMode_f mode = modes[opcode];
        (this->*mode)();
        
        operations.push(uOpCode[opcode]);
        operations.push(&Ricoh_RP2A03::end_cycle);
        
        return true;
    }

    inline void trigger_interrupt(const Word & interruptVector, const bool & isBRK);

    inline void check_for_interrupts() {
        if (CheckInterrupts) {
            if (NMIFlipFlop) {
                std::cout << "NMI" << std::endl;
                NMIFlipFlop = false;
                trigger_interrupt(VECTOR_NMI, false);
                return;
            }
            if (IRQLevel) {
                std::cout << "IRQ" << std::endl;
                trigger_interrupt(VECTOR_IRQ, false);
                return;
            }
        }
    }
    inline void do_DMA();
    bool INSTR = false;
    void ConsumeOne() {
        if (operations.empty()) return;

        const auto op = operations.pop();
        (this->*op)();
    }

    Word dmaSource;
    Byte * dmaTarget;
    Byte dmaOffset;
    int dmaTicks;

    bool CycleActive;
private:
    void Push(const Byte & value);
    Byte Pull();
public:
    explicit Ricoh_RP2A03();
    void SetStatus(const Byte & status);
    Byte GetStatus(const Flag B) const;
    void DMA(const Byte & fromHi, Byte * to, const Byte & offset);
    void Phi1();
    void Phi2();
};

#endif /* RICOH_RP2A03_H_ */
