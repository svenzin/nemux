#ifndef RICOH_RP2A03_H_
#define RICOH_RP2A03_H_

#include "Types.h"
#include "MemoryMap.h"

#include <string>
#include <vector>

class Ricoh_RP2A03 {
    enum Bits : size_t {
        Car, Zer, Int, Dec, Brk, Unu, Ovf, Neg,
        Left = 7, Right = 0,
    };

    void Push(const Byte & value) { Map->SetByteAt(0x0100 + S, value); --S; }
    Byte Pull() { ++S; return Map->GetByteAt(0x0100 + S); }

    inline void read_PC_to_opcode() { opcode = Map->GetByteAt(PC); }
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
    inline void index_address_by_X() { address = (address & WORD_HI_MASK) | ((address + X) & WORD_LO_MASK); }
    inline void index_address_by_Y() { address = (address & WORD_HI_MASK) | ((address + Y) & WORD_LO_MASK); }
    inline void fix_address_indexed_by_X() { AddressWasFixed = ((address & WORD_LO_MASK) < X); if (AddressWasFixed) address += 0x0100; }
    inline void fix_address_indexed_by_Y() { AddressWasFixed = ((address & WORD_LO_MASK) < Y); if (AddressWasFixed) address += 0x0100; }
    inline void write_operand_to_address() { Map->SetByteAt(address, operand); }
    inline void read_vector_to_PCL() { PC = (PC & WORD_HI_MASK) | Map->GetByteAt(vector); }
    inline void read_vector_to_PCH() { PC = (Map->GetByteAt(vector + 1) << BYTE_WIDTH) | (PC & WORD_LO_MASK); }

public:
    static constexpr Word VECTOR_NMI = 0xFFFA;
    static constexpr Word VECTOR_RST = 0xFFFC;
    static constexpr Word VECTOR_IRQ = 0xFFFE;

    enum uOp {
        read_PC_to_opcode_AND_increment_PC,
        read_PC_to_operand_AND_increment_PC,
        read_PC_to_operand_AND_increment_PC_AND,
        push_PCH_,
        push_PCL_,
        push_P_,
        read_vector_to_PCL_,
        read_vector_to_PCH_,
        set_I_AND,
        read_PC_to_operand_,
        read_PC_to_operand_AND,
        wait,
        pull_P_,
        pull_PCL_,
        pull_PCH_,
        increment_PC_,
        read_PC_to_AddressLo_AND_increment_PC,
        read_PC_to_AddressHi_AND_increment_PC,
        read_PC_to_AddressHi_AND,
        read_PC_to_Address_AND_increment_PC,
        read_Address_to_operand,
        read_Address_to_operand_AND,
        write_operand_to_Address_AND,
        write_operand_to_Address,
        index_Address_by_X,
        index_Address_by_Y,
        read_PC_to_AddressHi_AND_increment_PC_AND,
        fix_AddressHi_indexed_by_X_ReRead_AND,
        fix_AddressHi_indexed_by_X,
        fix_AddressHi_indexed_by_Y_ReRead_AND,
        fix_AddressHi_indexed_by_Y,
        move_Address_to_operand_AND,
        read_operand_to_AddressLo,
        read_operand_1_to_AddressHi,
        read_operand_1_to_AddressHi_AND,
        read_Address_1_to_AddressHi_AND_move_operand_to_AddressLo_AND,
        do_DMA,
        interrupt_disable_AND, interrupt_enable_AND,

        BranchFixPCH,

        opASL, opROL, opLSR, opROR,
        opJMP, opSTA, opBIT, opCPY, opCPX,
        opDEC, opINC, opLDX, opSTX, opLDY, opSTY,

        opORA, opAND, opEOR, opADC, opLDA, opCMP, opSBC,

        opASLa, opROLa, opLSRa, opRORa,
        opTXA, opTAX, opDEX, opNOP, opTXS, opTSX,
        opPHA, opPLA, opPHP, opPLP, opDEY, opTAY, opINY, opINX,
        opCLC, opSEC, opCLI, opSEI, opTYA, opCLV, opCLD, opSED,
        opBCC, opBCS, opBEQ, opBMI, opBNE, opBPL, opBVC, opBVS,

        xxNOP,
        xxSHY, xxSHX,
        xxSLO, xxANC, xxRLA, xxSRE, xxALR, xxRRA, xxARR,
        xxSAX, xxXAA, xxAHX, xxTAS, xxLAX, xxLAS,
        xxDCP, xxAXS, xxISC, xxSBC,

        opBRK, opJSR, opRTI, opRTS, xxHLT,
    };

    static constexpr uOp uOpCode[0x100] = {
        opBRK, opORA, xxHLT, xxSLO, xxNOP, opORA, opASL, xxSLO, opPHP, opORA, opASLa,xxANC, xxNOP, opORA, opASL, xxSLO,
        opBPL, opORA, xxHLT, xxSLO, xxNOP, opORA, opASL, xxSLO, opCLC, opORA, xxNOP, xxSLO, xxNOP, opORA, opASL, xxSLO,
        opJSR, opAND, xxHLT, xxRLA, opBIT, opAND, opROL, xxRLA, opPLP, opAND, opROLa,xxANC, opBIT, opAND, opROL, xxRLA,
        opBMI, opAND, xxHLT, xxRLA, xxNOP, opAND, opROL, xxRLA, opSEC, opAND, xxNOP, xxRLA, xxNOP, opAND, opROL, xxRLA,
        opRTI, opEOR, xxHLT, xxSRE, xxNOP, opEOR, opLSR, xxSRE, opPHA, opEOR, opLSRa,xxALR, opJMP, opEOR, opLSR, xxSRE,
        opBVC, opEOR, xxHLT, xxSRE, xxNOP, opEOR, opLSR, xxSRE, opCLI, opEOR, xxNOP, xxSRE, xxNOP, opEOR, opLSR, xxSRE,
        opRTS, opADC, xxHLT, xxRRA, xxNOP, opADC, opROR, xxRRA, opPLA, opADC, opRORa,xxARR, opJMP, opADC, opROR, xxRRA,
        opBVS, opADC, xxHLT, xxRRA, xxNOP, opADC, opROR, xxRRA, opSEI, opADC, xxNOP, xxRRA, xxNOP, opADC, opROR, xxRRA,
        xxNOP, opSTA, xxNOP, xxSAX, opSTY, opSTA, opSTX, xxSAX, opDEY, xxNOP, opTXA, xxXAA, opSTY, opSTA, opSTX, xxAHX,
        opBCC, opSTA, xxHLT, xxAHX, opSTY, opSTA, opSTX, xxSAX, opTYA, opSTA, opTXS, xxTAS, xxSHY, opSTA, xxSHX, xxAHX,
        opLDY, opLDA, opLDX, xxLAX, opLDY, opLDA, opLDX, xxLAX, opTAY, opLDA, opTAX, xxLAX, opLDY, opLDA, opLDX, xxLAX,
        opBCS, opLDA, xxHLT, xxLAX, opLDY, opLDA, opLDX, xxLAX, opCLV, opLDA, opTSX, xxLAS, opLDY, opLDA, opLDX, xxLAX,
        opCPY, opCMP, xxNOP, xxDCP, opCPY, opCMP, opDEC, xxDCP, opINY, opCMP, opDEX, xxAXS, opCPY, opCMP, opDEC, xxDCP,
        opBNE, opCMP, xxHLT, xxDCP, xxNOP, opCMP, opDEC, xxDCP, opCLD, opCMP, xxNOP, xxDCP, xxNOP, opCMP, opDEC, xxDCP,
        opCPX, opSBC, xxNOP, xxISC, opCPX, opSBC, opINC, xxISC, opINX, opSBC, opNOP, xxSBC, opCPX, opSBC, opINC, xxISC,
        opBEQ, opSBC, xxHLT, xxISC, xxNOP, opSBC, opINC, xxISC, opSED, opSBC, xxNOP, xxISC, xxNOP, opSBC, opINC, xxISC,
    };

    typedef void(* AddressingMode_f)(Ricoh_RP2A03 &);
    AddressingMode_f modes[0x100];

    std::array<uOp, 16> todo;
    int ihead, itail;
    bool todo_empty() { return ihead == itail; }
    void todo_push_first(const uOp op) {
        itail = (itail - 1 + todo.max_size()) % todo.max_size();
        todo[itail] = op;
    }
    void todo_push(const uOp op) {
        todo[ihead] = op;
        ihead = (ihead + 1) % todo.max_size();
    }
    uOp todo_pop() {
        const uOp op = todo[itail];
        itail = (itail + 1) % todo.max_size();
        return op;
    }

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

    Byte GetStackTop() const { return Map->GetByteAt(0x0100 + S); }
    void SetStackTop(const Byte value) const { Map->SetByteAt(0x0100 + S, value); }

    void Branch(const bool condition) {
        if (condition) {
            PC = (PC & 0xFF00) + ((PC + operand) & 0x00FF);
            todo_push(BranchFixPCH);
        }
        else {
            //todo_push(read_PC_to_opcode_AND_increment_PC);
        }
    }

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
        Word a = A - value - (1 - C);
        C = (a > BYTE_MASK) ? 0 : 1;
        V = Bit<Neg>(A ^ value) & Bit<Neg>(A ^ a);
        Transfer(a & BYTE_MASK, A);
    }

    Byte opcode;
    Flag Pflag;
    bool Halted = false;
    bool AddressWasFixed = false;

    static inline void ModeImplied(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_operand_AND);
    }
    static inline void ModeImmediate(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_operand_AND_increment_PC_AND);
    }
    static inline void ModeRelative(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_operand_AND_increment_PC);
    }
    static inline void ModeAbsoluteRead(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_AddressLo_AND_increment_PC);
        cpu.todo_push(read_PC_to_AddressHi_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand_AND);
    }
    static inline void ModeAbsoluteRMW(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_AddressLo_AND_increment_PC);
        cpu.todo_push(read_PC_to_AddressHi_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand);
        cpu.todo_push(write_operand_to_Address_AND);
    }
    static inline void ModeAbsoluteWrite(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_AddressLo_AND_increment_PC);
        cpu.todo_push(read_PC_to_AddressHi_AND_increment_PC);
    }
    static inline void ModeZeropageRead(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand_AND);
    }
    static inline void ModeZeropageRMW(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand);
        cpu.todo_push(write_operand_to_Address_AND);
    }
    static inline void ModeZeropageWrite(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
    }
    static inline void ModeZeropageXRead(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(index_Address_by_X);
        cpu.todo_push(read_Address_to_operand_AND);
    }
    static inline void ModeZeropageXRMW(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(index_Address_by_X);
        cpu.todo_push(read_Address_to_operand);
        cpu.todo_push(write_operand_to_Address_AND);
    }
    static inline void ModeZeropageXWrite(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(index_Address_by_X);
    }
    static inline void ModeZeropageYRead(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(index_Address_by_Y);
        cpu.todo_push(read_Address_to_operand_AND);
    }
    static inline void ModeZeropageYWrite(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(index_Address_by_Y);
    }
    static inline void ModeAbsoluteXRead(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_AddressLo_AND_increment_PC);
        cpu.todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        cpu.todo_push(index_Address_by_X);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(fix_AddressHi_indexed_by_X_ReRead_AND);
    }
    static inline void ModeAbsoluteXRMW(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_AddressLo_AND_increment_PC);
        cpu.todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        cpu.todo_push(index_Address_by_X);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(fix_AddressHi_indexed_by_X);
        cpu.todo_push(read_Address_to_operand);
        cpu.todo_push(write_operand_to_Address_AND);
    }
    static inline void ModeAbsoluteXWrite(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_AddressLo_AND_increment_PC);
        cpu.todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        cpu.todo_push(index_Address_by_X);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(fix_AddressHi_indexed_by_X);
    }
    static inline void ModeAbsoluteYRead(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_AddressLo_AND_increment_PC);
        cpu.todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        cpu.todo_push(index_Address_by_Y);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(fix_AddressHi_indexed_by_Y_ReRead_AND);
    }
    static inline void ModeAbsoluteYRMW(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_AddressLo_AND_increment_PC);
        cpu.todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        cpu.todo_push(index_Address_by_Y);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(fix_AddressHi_indexed_by_Y);
        cpu.todo_push(read_Address_to_operand);
        cpu.todo_push(write_operand_to_Address_AND);
    }
    static inline void ModeAbsoluteYWrite(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_AddressLo_AND_increment_PC);
        cpu.todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        cpu.todo_push(index_Address_by_Y);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(fix_AddressHi_indexed_by_Y);
    }
    static inline void ModeIndirectXRead(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(index_Address_by_X);
        cpu.todo_push(move_Address_to_operand_AND);
        cpu.todo_push(read_operand_to_AddressLo);
        cpu.todo_push(read_operand_1_to_AddressHi);
        cpu.todo_push(read_Address_to_operand_AND);
    }
    static inline void ModeIndirectXRMW(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(index_Address_by_X);
        cpu.todo_push(move_Address_to_operand_AND);
        cpu.todo_push(read_operand_to_AddressLo);
        cpu.todo_push(read_operand_1_to_AddressHi);
        cpu.todo_push(read_Address_to_operand);
        cpu.todo_push(write_operand_to_Address_AND);
    }
    static inline void ModeIndirectXWrite(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_Address_AND_increment_PC);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(index_Address_by_X);
        cpu.todo_push(move_Address_to_operand_AND);
        cpu.todo_push(read_operand_to_AddressLo);
        cpu.todo_push(read_operand_1_to_AddressHi);
    }
    static inline void ModeIndirectYRead(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_operand_AND_increment_PC);
        cpu.todo_push(read_operand_to_AddressLo);
        cpu.todo_push(read_operand_1_to_AddressHi_AND);
        cpu.todo_push(index_Address_by_Y);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(fix_AddressHi_indexed_by_Y_ReRead_AND);
    }
    static inline void ModeIndirectYRMW(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_operand_AND_increment_PC);
        cpu.todo_push(read_operand_to_AddressLo);
        cpu.todo_push(read_operand_1_to_AddressHi_AND);
        cpu.todo_push(index_Address_by_Y);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(fix_AddressHi_indexed_by_Y);
        cpu.todo_push(read_Address_to_operand);
        cpu.todo_push(write_operand_to_Address_AND);
    }
    static inline void ModeIndirectYWrite(Ricoh_RP2A03 & cpu) {
        cpu.todo_push(read_PC_to_operand_AND_increment_PC);
        cpu.todo_push(read_operand_to_AddressLo);
        cpu.todo_push(read_operand_1_to_AddressHi_AND);
        cpu.todo_push(index_Address_by_Y);
        cpu.todo_push(read_Address_to_operand_AND);
        cpu.todo_push(fix_AddressHi_indexed_by_Y);
    }
    bool ProcessOpcode() {
        switch (opcode) {
        case 0x02: case 0x22: case 0x42: case 0x62:
        case 0x12: case 0x32: case 0x52: case 0x72:
        case 0x92: case 0xB2: case 0xD2: case 0xF2: { // HLT
            Halted = true;
            return true;
        }
        case 0x4C: { // JMP
            todo_push(read_PC_to_AddressLo_AND_increment_PC);
            todo_push(read_PC_to_AddressHi_AND);
            todo_push(opJMP);
            return true;
        }
        case 0x6C: { // JMP indirect
            todo_push(read_PC_to_AddressLo_AND_increment_PC);
            todo_push(read_PC_to_AddressHi_AND_increment_PC);
            todo_push(read_Address_to_operand);
            todo_push(read_Address_1_to_AddressHi_AND_move_operand_to_AddressLo_AND);
            todo_push(opJMP);
            return true;
        }
        case 0x00: { // BRK
            vector = VECTOR_IRQ;
            Pflag = 1;
            todo_push(read_PC_to_operand_AND_increment_PC);
            todo_push(push_PCH_);
            todo_push(push_PCL_);
            todo_push(push_P_);
            todo_push(set_I_AND);
            todo_push(read_vector_to_PCL_);
            todo_push(read_vector_to_PCH_);
            return true;
        }
        case 0x40: { // RTI
            todo_push(read_PC_to_operand_);
            todo_push(wait);
            todo_push(pull_P_);
            todo_push(pull_PCL_);
            todo_push(pull_PCH_);
            return true;
        }
        case 0x60: { // RTS
            todo_push(read_PC_to_operand_);
            todo_push(wait);
            todo_push(pull_PCL_);
            todo_push(pull_PCH_);
            todo_push(increment_PC_);
            return true;
        }
        case 0x20: { // JSR
            todo_push(read_PC_to_AddressLo_AND_increment_PC);
            todo_push(wait);
            todo_push(push_PCH_);
            todo_push(push_PCL_);
            todo_push(read_PC_to_AddressHi_AND);
            todo_push(opJMP);
            return true;
        }
        case 0x08: { // PHP
            todo_push(wait);
            todo_push(opPHP);
            return true;
        }
        case 0x28: { // PLP
            todo_push(wait);
            todo_push(wait);
            todo_push(opPLP);
            return true;
        }
        case 0x48: { // PHA
            todo_push(wait);
            todo_push(opPHA);
            return true;
        }
        case 0x68: { // PLA
            todo_push(wait);
            todo_push(wait);
            todo_push(opPLA);
            return true;
        }
        }

        AddressingMode_f mode = modes[opcode];
        mode(*this);
        todo_push(uOpCode[opcode]);
        return true;

        throw std::runtime_error("Unknown opcode");
        return false;
    }


    bool INSTR = false;
    void ConsumeOne() {
        if (todo.empty()) return;

        const auto op = todo_pop();
        switch (op) {
        case pull_P_: pull_P(); break;
        case pull_PCL_: pull_PCL(); break;
        case pull_PCH_: pull_PCH(); break;
        case push_P_: push_P(); break;
        case push_PCL_: push_PCL(); break;
        case push_PCH_: push_PCH(); break;
        case read_vector_to_PCL_: read_vector_to_PCL(); break;
        case read_vector_to_PCH_: read_vector_to_PCH(); break;
        case increment_PC_: increment_PC(); break;
        case read_PC_to_Address_AND_increment_PC: { read_PC_to_address(); increment_PC(); break; }
        case read_PC_to_AddressLo_AND_increment_PC: { read_PC_to_addressLo(); increment_PC(); break; }
        case read_PC_to_AddressHi_AND: { read_PC_to_addressHi(); ConsumeOne(); break; }
        case read_PC_to_AddressHi_AND_increment_PC: { read_PC_to_addressHi(); increment_PC(); break; }
        case read_PC_to_AddressHi_AND_increment_PC_AND: { read_PC_to_addressHi(); increment_PC(); ConsumeOne(); break; }
        case read_PC_to_operand_: read_PC_to_operand(); break;
        case read_PC_to_operand_AND: { read_PC_to_operand(); ConsumeOne(); break; }
        case read_PC_to_operand_AND_increment_PC: { read_PC_to_operand(); increment_PC(); break; }
        case read_PC_to_operand_AND_increment_PC_AND: { read_PC_to_operand(); increment_PC(); ConsumeOne(); break; }
        case read_Address_to_operand: read_address_to_operand(); break;
        case read_Address_to_operand_AND: { read_address_to_operand(); ConsumeOne(); break; }
        case read_operand_to_AddressLo: read_operand_to_addressLo(); break;
        case read_operand_1_to_AddressHi: read_operand_1_to_addressHi(); break;
        case read_operand_1_to_AddressHi_AND: { read_operand_1_to_addressHi(); ConsumeOne(); break; }
        case write_operand_to_Address: write_operand_to_address(); break;
        case write_operand_to_Address_AND: { write_operand_to_address(); ConsumeOne(); break; }
        case index_Address_by_X: index_address_by_X(); break;
        case index_Address_by_Y: index_address_by_Y(); break;
        case fix_AddressHi_indexed_by_X: fix_address_indexed_by_X(); break;
        case fix_AddressHi_indexed_by_Y: fix_address_indexed_by_Y(); break;
        case fix_AddressHi_indexed_by_X_ReRead_AND: {
            fix_address_indexed_by_X();
            if (AddressWasFixed) {
                todo_push_first(wait);
                todo_push_first(read_Address_to_operand_AND);
            }
            ConsumeOne();
            break;
        }
        case fix_AddressHi_indexed_by_Y_ReRead_AND: {
            fix_address_indexed_by_Y();
            if (AddressWasFixed) {
                todo_push_first(wait);
                todo_push_first(read_Address_to_operand_AND);
            }
            ConsumeOne();
            break;
        }

        case set_I_AND: {
            I = 1;
            ConsumeOne();
            break;
        }
        case wait: {
            break;
        }
        case move_Address_to_operand_AND: {
            operand = address;
            ConsumeOne();
            break;
        }
        case read_Address_1_to_AddressHi_AND_move_operand_to_AddressLo_AND: {
            address = (Map->GetByteAt((address & WORD_HI_MASK) | ((address + 1) & WORD_LO_MASK)) << BYTE_WIDTH) | operand;
            ConsumeOne();
            break;
        }


        case read_PC_to_opcode_AND_increment_PC: {
            if (CheckInterrupts) {
                if (NMIFlipFlop) {
                    NMIFlipFlop = false;

                    vector = VECTOR_NMI;
                    Pflag = 0;
                    CheckInterrupts = false;
                    todo_push(read_PC_to_operand_);
                    todo_push(push_PCH_);
                    todo_push(push_PCL_);
                    todo_push(push_P_);
                    todo_push(set_I_AND);
                    todo_push(read_vector_to_PCL_);
                    todo_push(read_vector_to_PCH_);
                    todo_push(read_PC_to_opcode_AND_increment_PC); // Make sure one instruction from the interrupt handler gets executed
                    todo_push(interrupt_enable_AND);
                    return;
                }
                if (IRQLevel) {
                    vector = VECTOR_IRQ;
                    Pflag = 0;
                    CheckInterrupts = false;
                    todo_push(read_PC_to_operand_);
                    todo_push(push_PCH_);
                    todo_push(push_PCL_);
                    todo_push(push_P_);
                    todo_push(set_I_AND);
                    todo_push(read_vector_to_PCL_);
                    todo_push(read_vector_to_PCH_);
                    todo_push(read_PC_to_opcode_AND_increment_PC); // Make sure one instruction from the interrupt handler gets executed
                    todo_push(interrupt_enable_AND);
                    return;
                }
            }
            opcode = Map->GetByteAt(PC);
            increment_PC();
            ProcessOpcode();
            break;
        }
        case opBCC: Branch(C == 0); break;
        case opBCS: Branch(C == 1); break;
        case opBNE: Branch(Z == 0); break;
        case opBEQ: Branch(Z == 1); break;
        case opBPL: Branch(N == 0); break;
        case opBMI: Branch(N == 1); break;
        case opBVC: Branch(V == 0); break;
        case opBVS: Branch(V == 1); break;
        case BranchFixPCH: {
            if (IsBitSet<Neg>(operand)) {
                if ((PC & 0x00FF) >= operand) {
                    PC -= 0x0100;
                    //todo_push(FetchOpcodeAndIncrementPC);
                    todo_push(wait); // For correct timing
                }
                else {
                    //todo_push(FetchOpcodeAndIncrementPC);
                    //ConsumeOne();
                }
            }
            else {
                if ((PC & 0x00FF) < operand) {
                    PC += 0x0100;
                    //todo_push(FetchOpcodeAndIncrementPC);
                    todo_push(wait); // For correct timing
                }
                else {
                    //todo_push(FetchOpcodeAndIncrementPC);
                    //ConsumeOne();
                }
            }


            break;
        }

                           ////////////////////////////////////////////////////////////////

        case opPHA: { SetStackTop(A); --S; break; }
        case opPLA: { ++S; Transfer(GetStackTop(), A); break; }

        case opPHP: { SetStackTop(GetStatus(1)); --S; break; }
        case opPLP: { ++S; SetStatus(GetStackTop()); break; }

        case opJMP: PC = address; break;

            ////////////////////////////////////////////////////////////////
            // Implied or Accumulator

        case opDEY: Transfer(Y - 1, Y); break;
        case opTAY: Transfer(A, Y);     break;
        case opINY: Transfer(Y + 1, Y); break;
        case opINX: Transfer(X + 1, X); break;

        case opCLC: C = 0; break;
        case opSEC: C = 1; break;
        case opCLI: I = 0; break;
        case opSEI: I = 1; break;
        case opTYA: Transfer(Y, A); break;
        case opCLV: V = 0; break;
        case opCLD: D = 0; break;
        case opSED: D = 1; break;

        case opASLa: ROLwithFlag(A, 0); break;
        case opROLa: ROLwithFlag(A, C); break;
        case opLSRa: RORwithFlag(A, 0); break;
        case opRORa: RORwithFlag(A, C); break;
        case opTXA: Transfer(X, A);     break;
        case opTAX: Transfer(A, X);     break;
        case opDEX: Transfer(X - 1, X); break;
        case opNOP: break;

        case opTXS: S = X;          break;
        case opTSX: Transfer(S, X); break;

            ////////////////////////////////////////////////////////////////
            // Immediate

        case opLDY: Transfer(operand, Y); break;
        case opCPY: Compare(Y, operand); break;
        case opCPX: Compare(X, operand); break;

        case opORA: Transfer(A | operand, A); break;
        case opAND: Transfer(A & operand, A); break;
        case opEOR: Transfer(A ^ operand, A); break;
        case opADC: AddWithCarry(operand); break;
        case opLDA: Transfer(operand, A); break;
        case opCMP: Compare(A, operand); break;
        case opSBC: SubstractWithCarry(operand); break;

        case opLDX: Transfer(operand, X); break;

            ////////////////////////////////////////////////////////////////
            // Absolute

        case opBIT: {
            const auto mask = Map->GetByteAt(address);
            Z = (mask & A) == 0 ? 1 : 0;
            V = Bit<Ovf>(mask);
            N = Bit<Neg>(mask);
            break;
        }
        case opSTY: Map->SetByteAt(address, Y); break;
        case opSTA: Map->SetByteAt(address, A); break;
        case opSTX: Map->SetByteAt(address, X); break;

        case opASL: { ROLwithFlag(operand, 0); todo_push_first(write_operand_to_Address); break; }
        case opROL: { ROLwithFlag(operand, C); todo_push_first(write_operand_to_Address); break; }
        case opLSR: { RORwithFlag(operand, 0); todo_push_first(write_operand_to_Address); break; }
        case opROR: { RORwithFlag(operand, C); todo_push_first(write_operand_to_Address); break; }
        case opDEC: { Transfer(operand - 1, operand); todo_push_first(write_operand_to_Address); break; }
        case opINC: { Transfer(operand + 1, operand); todo_push_first(write_operand_to_Address); break; }

                    //
        case xxNOP: break;
        case xxSHY: {
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
            break;
        }
        case xxSHX: {
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
            break;
        }
        case xxSLO: {
            C = Bit<Left>(operand);
            Transfer(operand << 1, operand);
            Transfer(A | operand, A);
            todo_push_first(write_operand_to_Address);
            break;
        }
        case xxANC: { Transfer(A & operand, A); C = Bit<Left>(A); break; }
        case xxRLA: {
            const auto c = Bit<Left>(operand);
            Transfer((operand << 1) | Mask<Right>(C), operand);
            C = c;
            Transfer(A & operand, A);
            todo_push_first(write_operand_to_Address);
            break;
        }
        case xxSRE: {
            C = Bit<Right>(operand);
            Transfer(operand >> 1, operand);
            Transfer(A ^ operand, A);
            todo_push_first(write_operand_to_Address);
            break;
        }
        case xxALR: { Transfer(A & operand, A); C = Bit<Right>(A); Transfer(A >> 1, A); break; }
        case xxRRA: {
            const auto c = Bit<Right>(operand);
            Transfer((operand >> 1) | Mask<Left>(C), operand);
            C = c;
            AddWithCarry(operand);
            todo_push_first(write_operand_to_Address);
            break;
        }
        case xxARR: { Transfer(A & operand, A); Transfer((A >> 1) | Mask<Left>(C), A);
            switch ((A >> 5) & 0x03) {
            case 0: { C = 0; V = 0; break; }
            case 1: { C = 0; V = 1; break; }
            case 2: { C = 1; V = 1; break; }
            case 3: { C = 1; V = 0; break; }
            }
            break;
        }
        case xxXAA: break;
        case xxAHX: { const auto H = ((address & WORD_HI_MASK) >> BYTE_WIDTH); Map->SetByteAt(address, A & X & H); break; }
        case xxTAS: { const auto H = ((address & WORD_HI_MASK) >> BYTE_WIDTH); S = (A & X); Map->SetByteAt(address, A & X & H); break; }
        case xxLAS: { Transfer(operand & S, S); Transfer(S, A); Transfer(S, X); break; }
        case xxDCP: {
            Transfer(operand - 1, operand);
            Compare(A, operand);
            todo_push_first(write_operand_to_Address);
            break;
        }
        case xxAXS: { X = (A & X); Compare(X, operand); X = X - operand; break; }
        case xxISC: {
            Transfer(operand + 1, operand);
            SubstractWithCarry(operand);
            todo_push_first(write_operand_to_Address);
            break;
        }
        case xxSBC: SubstractWithCarry(operand); break;

        case xxSAX: { Map->SetByteAt(address, A & X); break; }
        case xxLAX: { Transfer(operand, A); Transfer(operand, X); break; }

        case do_DMA: {
            if (dmaTicks > 0) {
                --dmaTicks;
                todo_push_first(do_DMA);
            }
            break;
        }
        case interrupt_enable_AND: CheckInterrupts = true; break;
        case interrupt_disable_AND: CheckInterrupts = false; break;
        }
    }

    Word dmaSource;
    Byte * dmaTarget;
    Byte dmaOffset;
    int dmaTicks;

    explicit Ricoh_RP2A03();
    void SetStatus(const Byte & status);
    Byte GetStatus(const Flag B) const;
    void DMA(const Byte & fromHi, Byte * to, const Byte & offset);
    void Phi1();
    void Phi2();
};

#endif /* RICOH_RP2A03_H_ */
