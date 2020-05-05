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
    inline void read_to_PCL(const Word & vectorLo) { PC = (PC & WORD_HI_MASK) | Map->GetByteAt(vectorLo); }
    inline void read_to_PCH(const Word & vectorHi) { PC = (Map->GetByteAt(vectorHi) << BYTE_WIDTH) | (PC & WORD_LO_MASK); }

public:
    enum uOp {
        read_PC_to_opcode_AND_increment_PC,
        read_PC_to_operand_AND_increment_PC,
        read_PC_to_operand_AND_increment_PC_AND,
        push_PCH_,
        push_PCL_,
        push_P_,
        read_PCL_FFFE,
        read_PCH_FFFF,
        set_I_AND,
        read_PCL_FFFA,
        read_PCH_FFFB,
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
    };
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

    inline void ModeImplied() {
        todo_push(read_PC_to_operand_AND);
    }
    inline void ModeImmediate() {
        todo_push(read_PC_to_operand_AND_increment_PC_AND);
    }
    inline void ModeRelative() {
        todo_push(read_PC_to_operand_AND_increment_PC);
    }
    inline void ModeAbsoluteRead() {
        todo_push(read_PC_to_AddressLo_AND_increment_PC);
        todo_push(read_PC_to_AddressHi_AND_increment_PC);
        todo_push(read_Address_to_operand_AND);
    }
    inline void ModeAbsoluteRMW() {
        todo_push(read_PC_to_AddressLo_AND_increment_PC);
        todo_push(read_PC_to_AddressHi_AND_increment_PC);
        todo_push(read_Address_to_operand);
        todo_push(write_operand_to_Address_AND);
    }
    inline void ModeAbsoluteWrite() {
        todo_push(read_PC_to_AddressLo_AND_increment_PC);
        todo_push(read_PC_to_AddressHi_AND_increment_PC);
    }
    inline void ModeZeropageRead() {
        todo_push(read_PC_to_Address_AND_increment_PC);
        todo_push(read_Address_to_operand_AND);
    }
    inline void ModeZeropageRMW() {
        todo_push(read_PC_to_Address_AND_increment_PC);
        todo_push(read_Address_to_operand);
        todo_push(write_operand_to_Address_AND);
    }
    inline void ModeZeropageWrite() {
        todo_push(read_PC_to_Address_AND_increment_PC);
    }
    inline void ModeZeropageXRead() {
        todo_push(read_PC_to_Address_AND_increment_PC);
        todo_push(read_Address_to_operand_AND);
        todo_push(index_Address_by_X);
        todo_push(read_Address_to_operand_AND);
    }
    inline void ModeZeropageXRMW() {
        todo_push(read_PC_to_Address_AND_increment_PC);
        todo_push(read_Address_to_operand_AND);
        todo_push(index_Address_by_X);
        todo_push(read_Address_to_operand);
        todo_push(write_operand_to_Address_AND);
    }
    inline void ModeZeropageXWrite() {
        todo_push(read_PC_to_Address_AND_increment_PC);
        todo_push(read_Address_to_operand_AND);
        todo_push(index_Address_by_X);
    }
    inline void ModeZeropageYRead() {
        todo_push(read_PC_to_Address_AND_increment_PC);
        todo_push(read_Address_to_operand_AND);
        todo_push(index_Address_by_Y);
        todo_push(read_Address_to_operand_AND);
    }
    inline void ModeZeropageYWrite() {
        todo_push(read_PC_to_Address_AND_increment_PC);
        todo_push(read_Address_to_operand_AND);
        todo_push(index_Address_by_Y);
    }
    inline void ModeAbsoluteXRead() {
        todo_push(read_PC_to_AddressLo_AND_increment_PC);
        todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        todo_push(index_Address_by_X);
        todo_push(read_Address_to_operand_AND);
        todo_push(fix_AddressHi_indexed_by_X_ReRead_AND);
    }
    inline void ModeAbsoluteXRMW() {
        todo_push(read_PC_to_AddressLo_AND_increment_PC);
        todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        todo_push(index_Address_by_X);
        todo_push(read_Address_to_operand_AND);
        todo_push(fix_AddressHi_indexed_by_X);
        todo_push(read_Address_to_operand);
        todo_push(write_operand_to_Address_AND);
    }
    inline void ModeAbsoluteXWrite() {
        todo_push(read_PC_to_AddressLo_AND_increment_PC);
        todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        todo_push(index_Address_by_X);
        todo_push(read_Address_to_operand_AND);
        todo_push(fix_AddressHi_indexed_by_X);
    }
    inline void ModeAbsoluteYRead() {
        todo_push(read_PC_to_AddressLo_AND_increment_PC);
        todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        todo_push(index_Address_by_Y);
        todo_push(read_Address_to_operand_AND);
        todo_push(fix_AddressHi_indexed_by_Y_ReRead_AND);
    }
    inline void ModeAbsoluteYRMW() {
        todo_push(read_PC_to_AddressLo_AND_increment_PC);
        todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        todo_push(index_Address_by_Y);
        todo_push(read_Address_to_operand_AND);
        todo_push(fix_AddressHi_indexed_by_Y);
        todo_push(read_Address_to_operand);
        todo_push(write_operand_to_Address_AND);
    }
    inline void ModeAbsoluteYWrite() {
        todo_push(read_PC_to_AddressLo_AND_increment_PC);
        todo_push(read_PC_to_AddressHi_AND_increment_PC_AND);
        todo_push(index_Address_by_Y);
        todo_push(read_Address_to_operand_AND);
        todo_push(fix_AddressHi_indexed_by_Y);
    }
    inline void ModeIndirectXRead() {
        todo_push(read_PC_to_Address_AND_increment_PC);
        todo_push(read_Address_to_operand_AND);
        todo_push(index_Address_by_X);
        todo_push(move_Address_to_operand_AND);
        todo_push(read_operand_to_AddressLo);
        todo_push(read_operand_1_to_AddressHi);
        todo_push(read_Address_to_operand_AND);
    }
    inline void ModeIndirectXRMW() {
        todo_push(read_PC_to_Address_AND_increment_PC);
        todo_push(read_Address_to_operand_AND);
        todo_push(index_Address_by_X);
        todo_push(move_Address_to_operand_AND);
        todo_push(read_operand_to_AddressLo);
        todo_push(read_operand_1_to_AddressHi);
        todo_push(read_Address_to_operand);
        todo_push(write_operand_to_Address_AND);
    }
    inline void ModeIndirectXWrite() {
        todo_push(read_PC_to_Address_AND_increment_PC);
        todo_push(read_Address_to_operand_AND);
        todo_push(index_Address_by_X);
        todo_push(move_Address_to_operand_AND);
        todo_push(read_operand_to_AddressLo);
        todo_push(read_operand_1_to_AddressHi);
    }
    inline void ModeIndirectYRead() {
        todo_push(read_PC_to_operand_AND_increment_PC);
        todo_push(read_operand_to_AddressLo);
        todo_push(read_operand_1_to_AddressHi_AND);
        todo_push(index_Address_by_Y);
        todo_push(read_Address_to_operand_AND);
        todo_push(fix_AddressHi_indexed_by_Y_ReRead_AND);
    }
    inline void ModeIndirectYRMW() {
        todo_push(read_PC_to_operand_AND_increment_PC);
        todo_push(read_operand_to_AddressLo);
        todo_push(read_operand_1_to_AddressHi_AND);
        todo_push(index_Address_by_Y);
        todo_push(read_Address_to_operand_AND);
        todo_push(fix_AddressHi_indexed_by_Y);
        todo_push(read_Address_to_operand);
        todo_push(write_operand_to_Address_AND);
    }
    inline void ModeIndirectYWrite() {
        todo_push(read_PC_to_operand_AND_increment_PC);
        todo_push(read_operand_to_AddressLo);
        todo_push(read_operand_1_to_AddressHi_AND);
        todo_push(index_Address_by_Y);
        todo_push(read_Address_to_operand_AND);
        todo_push(fix_AddressHi_indexed_by_Y);
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
            Pflag = 1;
            todo_push(read_PC_to_operand_AND_increment_PC);
            todo_push(push_PCH_);
            todo_push(push_PCL_);
            todo_push(push_P_);
            todo_push(set_I_AND);
            todo_push(read_PCL_FFFE);
            todo_push(read_PCH_FFFF);
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
        }
        const Byte opType = (opcode & 0b00000011);
        const Byte opMode = ((opcode & 0b00011100) >> 2);
        const Byte opCode = ((opcode & 0b11100000) >> 5);

        switch (opType) {
        case 0: // Control instructions
            switch (opMode) {
            case 0: ModeImmediate(); break;                                                 // Immediate
            case 1: if (opCode == 4) ModeZeropageWrite();  else ModeZeropageRead();  break; // Zero page     R/W
            case 2: ModeImplied();   break;                                                 // Implied
            case 3: if (opCode == 4) ModeAbsoluteWrite();  else ModeAbsoluteRead();  break; // Absolute      R/W+JMP()
            case 4: ModeRelative();  break;                                                 // Relative
            case 5: if (opCode == 4) ModeZeropageXWrite(); else ModeZeropageXRead(); break; // Zero page X   R/W
            case 6: ModeImplied();   break;                                                 // Implied
            case 7: if (opCode == 4) ModeAbsoluteXWrite(); else ModeAbsoluteXRead(); break; // Absolute X    R/W
            }
            break;
        case 1: // ALU instructions
            switch (opMode) {
            case 0: if (opCode == 4) ModeIndirectXWrite(); else ModeIndirectXRead(); break; // Indirect X    R/W
            case 1: if (opCode == 4) ModeZeropageWrite();  else ModeZeropageRead();  break; // Zero page     R/W
            case 2: ModeImmediate(); break;                                                 // Immediate
            case 3: if (opCode == 4) ModeAbsoluteWrite();  else ModeAbsoluteRead();  break; // Absolute      R/W
            case 4: if (opCode == 4) ModeIndirectYWrite(); else ModeIndirectYRead(); break; // Indirect Y    R/W
            case 5: if (opCode == 4) ModeZeropageXWrite(); else ModeZeropageXRead(); break; // Zero page X   R/W
            case 6: if (opCode == 4) ModeAbsoluteYWrite(); else ModeAbsoluteYRead(); break; // Absolute Y    R/W
            case 7: if (opCode == 4) ModeAbsoluteXWrite(); else ModeAbsoluteXRead(); break; // Absolute X    R/W
            }
            break;
        case 2: // RMW instructions
            switch (opMode) {
            case 0: ModeImmediate(); break;                                                                                           // Immediate
            case 1: if (opCode == 4) ModeZeropageWrite();  else if (opCode == 5) ModeZeropageRead();  else ModeZeropageRMW();  break; // Zero page     R/RMW/W
            case 2: ModeImplied();   break;                                                                                           // Implied
            case 3: if (opCode == 4) ModeAbsoluteWrite();  else if (opCode == 5) ModeAbsoluteRead();  else ModeAbsoluteRMW();  break; // Absolute      R/RMW/W
            case 4: ModeImplied();   break;                                                                                           // Implied (HLT)
            case 5: if (opCode == 4) ModeZeropageYWrite(); else if (opCode == 5) ModeZeropageYRead(); else ModeZeropageXRMW(); break; // Zero page X/Y R/RMW/W
            case 6: ModeImplied();   break;                                                                                           // Implied
            case 7: if (opCode == 4) ModeAbsoluteYWrite(); else if (opCode == 5) ModeAbsoluteYRead(); else ModeAbsoluteXRMW(); break; // Absolute X/Y  R/RMW/W
            }
            break;
        case 3: // Combined ALU/RMW instructions
            switch (opMode) {
            case 0: if (opCode == 4) ModeIndirectXWrite(); else if (opCode == 5) ModeIndirectXRead(); else ModeIndirectXRMW(); break; // Indirect X    R/W
            case 1: if (opCode == 4) ModeZeropageWrite();  else if (opCode == 5) ModeZeropageRead();  else ModeZeropageRMW();  break; // Zero page     R/W
            case 2: ModeImmediate();  break;                                                                                          // Immediate
            case 3: if (opCode == 4) ModeAbsoluteWrite();  else if (opCode == 5) ModeAbsoluteRead();  else ModeAbsoluteRMW();  break; // Absolute      R/W
            case 4: if (opCode == 4) ModeIndirectYWrite(); else if (opCode == 5) ModeIndirectYRead(); else ModeIndirectYRMW(); break; // Indirect Y    R/W
            case 5: if (opCode == 4) ModeZeropageYWrite(); else if (opCode == 5) ModeZeropageYRead(); else ModeZeropageXRMW(); break; // Zero page X/Y R/W
            case 6: if (opCode == 4) ModeAbsoluteYWrite(); else if (opCode == 5) ModeAbsoluteYRead(); else ModeAbsoluteYRMW(); break; // Absolute Y    R/W
            case 7: if (opCode == 4) ModeAbsoluteYWrite(); else if (opCode == 5) ModeAbsoluteYRead(); else ModeAbsoluteXRMW(); break; // Absolute X/Y  R/W
            }
            break;
        }
        switch (opcode) {
        case 0x80: case 0x04: case 0x44: case 0x64:
        case 0x0C: case 0x14: case 0x34: case 0x54:
        case 0x74: case 0xD4: case 0xF4: case 0x1C:
        case 0x3C: case 0x5C: case 0x7C: case 0xDC:
        case 0xFC: case 0x89: case 0x82: case 0xC2:
        case 0xE2: case 0x1A: case 0x3A: case 0x5A:
        case 0x7A: case 0xDA: case 0xFA:
            todo_push(xxNOP); return true;
        case 0x9C: todo_push(xxSHY); return true;
        case 0x9E: todo_push(xxSHX); return true;

        case 0x03:
        case 0x07:
        case 0x0F:
        case 0x13:
        case 0x17:
        case 0x1B:
        case 0x1F: todo_push(xxSLO); return true;
        case 0x0B: todo_push(xxANC); return true;

        case 0x23:
        case 0x27:
        case 0x2F:
        case 0x33:
        case 0x37:
        case 0x3B:
        case 0x3F: todo_push(xxRLA); return true;
        case 0x2B: todo_push(xxANC); return true;

        case 0x43:
        case 0x47:
        case 0x4F:
        case 0x53:
        case 0x57:
        case 0x5B:
        case 0x5F: todo_push(xxSRE); return true;
        case 0x4B: todo_push(xxALR); return true;

        case 0x63:
        case 0x67:
        case 0x6F:
        case 0x73:
        case 0x77:
        case 0x7B:
        case 0x7F: todo_push(xxRRA); return true;
        case 0x6B: todo_push(xxARR); return true;

        case 0x83:
        case 0x87:
        case 0x8F:
        case 0x97: todo_push(xxSAX); return true;
        case 0x8B: todo_push(xxXAA); return true;
        case 0x93:
        case 0x9F: todo_push(xxAHX); return true;
        case 0x9B: todo_push(xxTAS); return true;

        case 0xA3:
        case 0xA7:
        case 0xAB:
        case 0xAF:
        case 0xB3:
        case 0xB7:
        case 0xBF: todo_push(xxLAX); return true;
        case 0xBB: todo_push(xxLAS); return true;

        case 0xC3:
        case 0xC7:
        case 0xCF:
        case 0xD3:
        case 0xD7:
        case 0xDB:
        case 0xDF: todo_push(xxDCP); return true;
        case 0xCB: todo_push(xxAXS); return true;

        case 0xE3:
        case 0xE7:
        case 0xEF:
        case 0xF3:
        case 0xF7:
        case 0xFB:
        case 0xFF: todo_push(xxISC); return true;
        case 0xEB: todo_push(xxSBC); return true;

        case 0x48: { todo_push(wait); todo_push(opPHA); return true; }
        case 0x08: { todo_push(wait); todo_push(opPHP); return true; }

        case 0x68: { todo_push(wait); todo_push(wait); todo_push(opPLA); return true; }
        case 0x28: { todo_push(wait); todo_push(wait); todo_push(opPLP); return true; }

            ////////////////////////////////
            // Implied or Accumulator
        case 0x88: todo_push(opDEY); return true;
        case 0xA8: todo_push(opTAY); return true;
        case 0xC8: todo_push(opINY); return true;
        case 0xE8: todo_push(opINX); return true;
        case 0x18: todo_push(opCLC); return true;
        case 0x38: todo_push(opSEC); return true;
        case 0x58: todo_push(opCLI); return true;
        case 0x78: todo_push(opSEI); return true;
        case 0x98: todo_push(opTYA); return true;
        case 0xB8: todo_push(opCLV); return true;
        case 0xD8: todo_push(opCLD); return true;
        case 0xF8: todo_push(opSED); return true;
        case 0x0A: todo_push(opASLa); return true;
        case 0x2A: todo_push(opROLa); return true;
        case 0x4A: todo_push(opLSRa); return true;
        case 0x6A: todo_push(opRORa); return true;
        case 0x8A: todo_push(opTXA); return true;
        case 0xAA: todo_push(opTAX); return true;
        case 0xCA: todo_push(opDEX); return true;
        case 0xEA: todo_push(opNOP); return true;
        case 0x9A: todo_push(opTXS); return true;
        case 0xBA: todo_push(opTSX); return true;

            ////////////////////////////////
            // Immediate
        case 0xA0: todo_push(opLDY); return true;
        case 0xC0: todo_push(opCPY); return true;
        case 0xE0: todo_push(opCPX); return true;
        case 0x09: todo_push(opORA); return true;
        case 0x29: todo_push(opAND); return true;
        case 0x49: todo_push(opEOR); return true;
        case 0x69: todo_push(opADC); return true;
        case 0xA9: todo_push(opLDA); return true;
        case 0xC9: todo_push(opCMP); return true;
        case 0xE9: todo_push(opSBC); return true;
        case 0xA2: todo_push(opLDX); return true;

            ////////////////////////////////
            // Absolute
        case 0x2C: todo_push(opBIT); return true;
        case 0x8C: todo_push(opSTY); return true;
        case 0xAC: todo_push(opLDY); return true;
        case 0xCC: todo_push(opCPY); return true;
        case 0xEC: todo_push(opCPX); return true;
        case 0x0D: todo_push(opORA); return true;
        case 0x2D: todo_push(opAND); return true;
        case 0x4D: todo_push(opEOR); return true;
        case 0x6D: todo_push(opADC); return true;
        case 0x8D: todo_push(opSTA); return true;
        case 0xAD: todo_push(opLDA); return true;
        case 0xCD: todo_push(opCMP); return true;
        case 0xED: todo_push(opSBC); return true;
        case 0x0E: todo_push(opASL); return true;
        case 0x2E: todo_push(opROL); return true;
        case 0x4E: todo_push(opLSR); return true;
        case 0x6E: todo_push(opROR); return true;
        case 0x8E: todo_push(opSTX); return true;
        case 0xAE: todo_push(opLDX); return true;
        case 0xCE: todo_push(opDEC); return true;
        case 0xEE: todo_push(opINC); return true;

            ////////////////////////////////
            // Zero page
        case 0x24: todo_push(opBIT); return true;
        case 0x84: todo_push(opSTY); return true;
        case 0xA4: todo_push(opLDY); return true;
        case 0xC4: todo_push(opCPY); return true;
        case 0xE4: todo_push(opCPX); return true;
        case 0x05: todo_push(opORA); return true;
        case 0x25: todo_push(opAND); return true;
        case 0x45: todo_push(opEOR); return true;
        case 0x65: todo_push(opADC); return true;
        case 0x85: todo_push(opSTA); return true;
        case 0xA5: todo_push(opLDA); return true;
        case 0xC5: todo_push(opCMP); return true;
        case 0xE5: todo_push(opSBC); return true;
        case 0x06: todo_push(opASL); return true;
        case 0x26: todo_push(opROL); return true;
        case 0x46: todo_push(opLSR); return true;
        case 0x66: todo_push(opROR); return true;
        case 0x86: todo_push(opSTX); return true;
        case 0xA6: todo_push(opLDX); return true;
        case 0xC6: todo_push(opDEC); return true;
        case 0xE6: todo_push(opINC); return true;

            ////////////////////////////////
            // Zero page, X
        case 0x94: todo_push(opSTY); return true;
        case 0xB4: todo_push(opLDY); return true;
        case 0x15: todo_push(opORA); return true;
        case 0x35: todo_push(opAND); return true;
        case 0x55: todo_push(opEOR); return true;
        case 0x75: todo_push(opADC); return true;
        case 0x95: todo_push(opSTA); return true;
        case 0xB5: todo_push(opLDA); return true;
        case 0xD5: todo_push(opCMP); return true;
        case 0xF5: todo_push(opSBC); return true;
        case 0x16: todo_push(opASL); return true;
        case 0x36: todo_push(opROL); return true;
        case 0x56: todo_push(opLSR); return true;
        case 0x76: todo_push(opROR); return true;
        case 0xD6: todo_push(opDEC); return true;
        case 0xF6: todo_push(opINC); return true;

            ////////////////////////////////
            // Zero page, Y
        case 0x96: todo_push(opSTX); return true;
        case 0xB6: todo_push(opLDX); return true;

            ////////////////////////////////
            // Absolute, X
        case 0xBC: todo_push(opLDY); return true;
        case 0x1D: todo_push(opORA); return true;
        case 0x3D: todo_push(opAND); return true;
        case 0x5D: todo_push(opEOR); return true;
        case 0x7D: todo_push(opADC); return true;
        case 0x9D: todo_push(opSTA); return true;
        case 0xBD: todo_push(opLDA); return true;
        case 0xDD: todo_push(opCMP); return true;
        case 0xFD: todo_push(opSBC); return true;
        case 0x1E: todo_push(opASL); return true;
        case 0x3E: todo_push(opROL); return true;
        case 0x5E: todo_push(opLSR); return true;
        case 0x7E: todo_push(opROR); return true;
        case 0xDE: todo_push(opDEC); return true;
        case 0xFE: todo_push(opINC); return true;

            ////////////////////////////////
            // Absolute, Y
        case 0x19: todo_push(opORA); return true;
        case 0x39: todo_push(opAND); return true;
        case 0x59: todo_push(opEOR); return true;
        case 0x79: todo_push(opADC); return true;
        case 0x99: todo_push(opSTA); return true;
        case 0xB9: todo_push(opLDA); return true;
        case 0xD9: todo_push(opCMP); return true;
        case 0xF9: todo_push(opSBC); return true;
        case 0xBE: todo_push(opLDX); return true;

            ////////////////////////////////
            // Relative
        case 0x10: todo_push(opBPL); return true;
        case 0x30: todo_push(opBMI); return true;
        case 0x50: todo_push(opBVC); return true;
        case 0x70: todo_push(opBVS); return true;
        case 0x90: todo_push(opBCC); return true;
        case 0xB0: todo_push(opBCS); return true;
        case 0xD0: todo_push(opBNE); return true;
        case 0xF0: todo_push(opBEQ); return true;

            ////////////////////////////////
            // Indexed indirect (X)
        case 0x01: todo_push(opORA); return true;
        case 0x21: todo_push(opAND); return true;
        case 0x41: todo_push(opEOR); return true;
        case 0x61: todo_push(opADC); return true;
        case 0x81: todo_push(opSTA); return true;
        case 0xA1: todo_push(opLDA); return true;
        case 0xC1: todo_push(opCMP); return true;
        case 0xE1: todo_push(opSBC); return true;

            ////////////////////////////////
            // Indirect indexed (Y)
        case 0x11: todo_push(opORA); return true;
        case 0x31: todo_push(opAND); return true;
        case 0x51: todo_push(opEOR); return true;
        case 0x71: todo_push(opADC); return true;
        case 0x91: todo_push(opSTA); return true;
        case 0xB1: todo_push(opLDA); return true;
        case 0xD1: todo_push(opCMP); return true;
        case 0xF1: todo_push(opSBC); return true;
        }
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
        case read_PCL_FFFA: read_to_PCL(0xFFFA); break;
        case read_PCH_FFFB: read_to_PCH(0xFFFB); break;
        case read_PCL_FFFE: read_to_PCL(0xFFFE); break;
        case read_PCH_FFFF: read_to_PCH(0xFFFF); break;
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

                    Pflag = 0;
                    CheckInterrupts = false;
                    todo_push(read_PC_to_operand_);
                    todo_push(push_PCH_);
                    todo_push(push_PCL_);
                    todo_push(push_P_);
                    todo_push(set_I_AND);
                    todo_push(read_PCL_FFFA);
                    todo_push(read_PCH_FFFB);
                    todo_push(read_PC_to_opcode_AND_increment_PC); // Make sure one instruction from the interrupt handler gets executed
                    todo_push(interrupt_enable_AND);
                    return;
                }
                if (IRQLevel) {
                    Pflag = 0;
                    CheckInterrupts = false;
                    todo_push(read_PC_to_operand_);
                    todo_push(push_PCH_);
                    todo_push(push_PCL_);
                    todo_push(push_P_);
                    todo_push(set_I_AND);
                    todo_push(read_PCL_FFFE);
                    todo_push(read_PCH_FFFF);
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
