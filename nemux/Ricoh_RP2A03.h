#ifndef RICOH_RP2A03_H_
#define RICOH_RP2A03_H_

#include "Types.h"
#include "MemoryMap.h"
#include "CircularQueue.h"

#include <string>
#include <vector>

#include <iostream>

class Ricoh_RP2A03 {


    Byte GetLo(const Word & w) {
        return Byte(w);
    }

    Byte GetHi(const Word & w) {
        return Byte(w >> BYTE_WIDTH);
    }

    void SetLo(Word & w, const Byte & lo) {
        w = (w & WORD_HI_MASK) | lo;
    }

    void SetHi(Word & w, const Byte & hi) {
        w = (hi << BYTE_WIDTH) | Byte(w);
    }

    enum Bits : size_t {
        Car, Zer, Int, Dec, Brk, Unu, Ovf, Neg,
        Left = 7, Right = 0,
    };

    inline Byte GetByteAt(const Word & address) const { return Map->GetByteAt(address); }
    inline void SetByteAt(const Word & address, const Byte & value) {
        //if (address == 0x2000) std::cout << "$2000 <- $" << std::hex << int(value) << std::endl;
        Map->SetByteAt(address, value);
    }

    // µops state
    Word vector;
    Word address;
    Byte index;
    Byte operand;
    Flag Pflag;
    bool AddressWasFixed = false;
    bool CheckInterrupts = true;

    inline void increment_PC()                        { ++PC; }
    inline void read_PC_to_operand()                  { operand = GetByteAt(PC); }
    inline void push_PCL()                            { Push(PC); }
    inline void push_PCH()                            { Push(PC >> 8); }
    inline void push_P()                              { Push(GetStatus(Pflag)); }
    inline void pull_PCL()                            { SetLo(PC, Pull()); }
    inline void pull_PCH()                            { SetHi(PC, Pull()); }
    inline void pull_P()                              { SetStatus(Pull()); }
    inline void read_PC_to_address()                  { address = GetByteAt(PC); }
    inline void read_PC_to_addressLo()                { SetLo(address, GetByteAt(PC)); }
    inline void read_PC_to_addressHi()                { SetHi(address, GetByteAt(PC)); }
    inline void read_address_to_operand()             { operand = GetByteAt(address); }
    inline void read_operand_to_addressLo()           { SetLo(address, GetByteAt(operand)); }
    inline void read_operand_1_to_addressHi()         { SetHi(address, GetByteAt(Byte(operand + 1))); }
    inline void index_address()                       { SetLo(address, address + index); }
    inline void fix_indexed_address()                 { AddressWasFixed = (Byte(address) < index); if (AddressWasFixed) address += 0x0100; }
    inline void write_operand_to_address()            { SetByteAt(address, operand); }
    inline void read_vector_to_PCL()                  { SetLo(PC, GetByteAt(vector)); }
    inline void read_vector_to_PCH()                  { SetHi(PC, GetByteAt(vector + 1)); }
    inline void read_address_and_operand_to_address() { SetLo(address, address + 1); SetHi(address, GetByteAt(address)); SetLo(address, operand); }
    inline void move_address_to_operand()             { operand = address; }
    inline void decrement_S()                         { --S; }
    
    inline void r_m_w(); // Read-Modify-Write

    inline bool interrupted() {
        // If an interrupt sequence was running (CheckInterrupts == false)
        // do not queue another interrupt but let one instruction run
        static int nmic = 0;
        if (CheckInterrupts) {
            if (NMIFlipFlop) {
                //std::cout << Id << " NMI " << nmic++ << std::endl;
                NMIFlipFlop = false;
                trigger_interrupt(VECTOR_NMI, false, false);
                return true;
            }
            if (IRQLevel) {
                //std::cout << Id << " IRQ" << std::endl;
                trigger_interrupt(VECTOR_IRQ, false, false);
                return true;
            }
        }
        CheckInterrupts = true;

        return false;
    }
    inline void fetch_opcode() {
        if (interrupted()) return;
        
        opcode = GetByteAt(PC);
        increment_PC();
        ProcessOpcode();
    }
    inline void queue_read_if_address_fixed();

    inline void poll_interrupts() {
        CheckInterrupts = true;
    }

    inline void end_cycle() { CycleActive = false; }

    // RP2A03 instructions
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    inline void NOP() {}
    
    inline void JMP() { PC = address; }

    inline void BRK() { trigger_interrupt(VECTOR_IRQ, true, false); }
    inline void JSR() { JMP(); }
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

    inline void STA() { SetByteAt(address, A); }
    inline void STX() { SetByteAt(address, X); }
    inline void STY() { SetByteAt(address, Y); }
    
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
    inline void SBC() { AddWithCarry(~operand); }
    
    inline void ASLa() { ROLc(A, 0); }
    inline void ROLa() { ROLc(A, C); }
    inline void LSRa() { RORc(A, 0); }
    inline void RORa() { RORc(A, C); }
    
    inline void ASL() { r_m_w(); ROLc(operand, 0); }
    inline void ROL() { r_m_w(); ROLc(operand, C); }
    inline void LSR() { r_m_w(); RORc(operand, 0); }
    inline void ROR() { r_m_w(); RORc(operand, C); }
    inline void DEC() { r_m_w(); Transfer(operand - 1, operand); }
    inline void INC() { r_m_w(); Transfer(operand + 1, operand); }

    inline void BIT() {
        Z = (operand & A) == 0 ? 1 : 0;
        V = Bit<Ovf>(operand);
        N = Bit<Neg>(operand);
    }

    ////////////////////////////////////////////////////////////////

    inline void xNOP() {}
    inline void xHLT() { Halted = true; }

    inline void xSHX() {
        const auto M = (X & (GetHi(address) + 1));
        if (AddressWasFixed) SetHi(address, M);
        SetByteAt(address, M);
    }
    inline void xSHY() {
        const auto M = (Y & (GetHi(address) + 1));
        if (AddressWasFixed) SetHi(address, M);
        SetByteAt(address, M);
    }

    inline void xSLO() { ASL(); ORA(); }
    inline void xRLA() { ROL(); AND(); }
    inline void xSRE() { LSR(); EOR(); }
    inline void xRRA() { ROR(); ADC(); }
    inline void xDCP() { DEC(); CMP(); }
    inline void xISC() { INC(); SBC(); }

    inline void xANC() { AND(); C = Bit<Left>(A); }
    inline void xALR() { AND(); LSRa(); }
    inline void xARR() { AND(); RORa(); C = Bit<6>(A); V = (C ^ Bit<5>(A)); }
    inline void xXAA() {}
    inline void xAHX() { SetByteAt(address, A & X & GetHi(address)); }
    inline void xTAS() { S = (A & X); xAHX(); }
    inline void xLAS() { Transfer(operand & S, S); Transfer(S, A); TSX(); }
    inline void xAXS() { X = (A & X); CPX(); X = X - operand; }
    inline void xSBC() { SBC(); }
    inline void xSAX() { SetByteAt(address, A & X); }
    inline void xLAX() { LDA(); TAX(); }

public:
    typedef void(Ricoh_RP2A03::* MicroOp_f)();
    typedef void(Ricoh_RP2A03::* AddressingMode_f)();
    std::array<MicroOp_f, 0x100> uOpCode;

    std::array<AddressingMode_f, 0x100> modes;

    CircularQueue<MicroOp_f, 64> operations;
    void Cycle();
    void Cycle(const MicroOp_f & op);
    void Cycle(const MicroOp_f & op1,
               const MicroOp_f & op2);
    void Cycle(const MicroOp_f & op1,
               const MicroOp_f & op2,
               const MicroOp_f & op3);
    void Start(const MicroOp_f & op);
    void Start(const MicroOp_f & op1,
               const MicroOp_f & op2);
    void Start(const MicroOp_f & op1,
               const MicroOp_f & op2,
               const MicroOp_f & op3);
    void Finish(const MicroOp_f & op);


    bool NMIEdge;
    bool NMIFlipFlop;
    bool IRQLevel;



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
        N = Bit<Neg>(r);
    }

    void ROLc(Byte & value, const Flag flag) {
        C = Bit<Left>(value);
        Transfer((value << 1) | Mask<Right>(flag), value);
    }

    void RORc(Byte & value, const Flag flag) {
        C = Bit<Right>(value);
        Transfer((value >> 1) | Mask<Left>(flag), value);
    }

    void AddWithCarry(const Byte & value) {
        Word a = A + value + C;
        C = Bit<Right>(GetHi(a));
        V = ~Bit<Neg>(A ^ value) & Bit<Neg>(A ^ a);
        Transfer(Byte(a), A);
    }

    Byte opcode;
    bool Halted = false;

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

    inline void ModeReturn(const bool & pullP, const bool & incPC);
    inline void ModePush();
    inline void ModePull();
    inline void ModeJump();
    inline void ModeJumpIndirect();
    inline void ModeRTI();
    inline void ModeRTS();
    inline void ModeJSR();
    
    bool ProcessOpcode() {
        AddressingMode_f mode = modes[opcode];
        
        (this->*mode)();
        Finish(uOpCode[opcode]);
        
        return true;
    }

    inline void trigger_interrupt(const Word & interruptVector,
                                  const bool & isBRK,
                                  const bool & isRST);

    inline void do_DMA();
    bool INSTR = false;
    void ConsumeOne() {
        if (operations.empty()) {
            Cycle(&Ricoh_RP2A03::fetch_opcode);
        }
        const auto op = operations.pop();
        (this->*op)();
    }

    Word dmaSource;
    Byte * dmaTarget;
    Byte dmaOffset;
    int dmaTicks;

    bool CycleActive;

private:
    Byte Pull();
    void Push(const Byte & value);


public:
    // CPU state
    Byte GetStatus(const Flag B) const;
    void SetStatus(const Byte & status);
    size_t Ticks;
    Word PC;
    Byte S, A, X, Y;
    Flag N, V, D, I, Z, C;

    static constexpr char * Id = "2A03";
    static constexpr char * Name = "Ricoh RP2A03";

    static constexpr Word VECTOR_NMI = 0xFFFA;
    static constexpr Word VECTOR_RST = 0xFFFC;
    static constexpr Word VECTOR_IRQ = 0xFFFE;
    static constexpr Word STACK_PAGE = 0x0100;

    bool IRQ;
    bool NMI;

    MemoryMap * Map;

    explicit Ricoh_RP2A03();
    void Reset();
    void DMA(const Byte & fromHi, Byte * to, const Byte & offset);
    void Phi1();
    void Phi2();
};

#endif /* RICOH_RP2A03_H_ */
