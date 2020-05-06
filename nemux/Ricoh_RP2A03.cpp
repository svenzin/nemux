#include "Ricoh_RP2A03.h"

#define M(f) (& Ricoh_RP2A03::f)

Ricoh_RP2A03::AddressingMode_f GetAddressingMode(Byte opcode) {
    const Byte opType = (opcode & 0b00000011);
    const Byte opMode = ((opcode & 0b00011100) >> 2);
    const Byte opCode = ((opcode & 0b11100000) >> 5);

    switch (opType) {
    case 0: // Control instructions
        switch (opMode) {
        case 0:                  return M(ModeImmediate);
        case 1: if (opCode == 4) return M(ModeZeropageWrite);
                else             return M(ModeZeropageRead);
        case 2:                  return M(ModeImplied);
        case 3: if (opCode == 4) return M(ModeAbsoluteWrite);
                else             return M(ModeAbsoluteRead);
        case 4:                  return M(ModeRelative);
        case 5: if (opCode == 4) return M(ModeZeropageXWrite);
                else             return M(ModeZeropageXRead);
        case 6:                  return M(ModeImplied);
        case 7: if (opCode == 4) return M(ModeAbsoluteXWrite);
                else             return M(ModeAbsoluteXRead);
        }
        break;
    case 1: // ALU instructions
        switch (opMode) {
        case 0: if (opCode == 4) return M(ModeIndirectXWrite);
                else             return M(ModeIndirectXRead);
        case 1: if (opCode == 4) return M(ModeZeropageWrite);
                else             return M(ModeZeropageRead);
        case 2:                  return M(ModeImmediate);
        case 3: if (opCode == 4) return M(ModeAbsoluteWrite);
                else             return M(ModeAbsoluteRead);
        case 4: if (opCode == 4) return M(ModeIndirectYWrite);
                else             return M(ModeIndirectYRead);
        case 5: if (opCode == 4) return M(ModeZeropageXWrite);
                else             return M(ModeZeropageXRead);
        case 6: if (opCode == 4) return M(ModeAbsoluteYWrite);
                else             return M(ModeAbsoluteYRead);
        case 7: if (opCode == 4) return M(ModeAbsoluteXWrite);
                else             return M(ModeAbsoluteXRead);
        }
        break;
    case 2: // RMW instructions
        switch (opMode) {
        case 0:                       return M(ModeImmediate);
        case 1: if      (opCode == 4) return M(ModeZeropageWrite);
                else if (opCode == 5) return M(ModeZeropageRead);
                else                  return M(ModeZeropageRMW);
        case 2:                       return M(ModeImplied);
        case 3: if      (opCode == 4) return M(ModeAbsoluteWrite);
                else if (opCode == 5) return M(ModeAbsoluteRead);
                else                  return M(ModeAbsoluteRMW);
        case 4:                       return M(ModeImplied);
        case 5: if      (opCode == 4) return M(ModeZeropageYWrite);
                else if (opCode == 5) return M(ModeZeropageYRead);
                else                  return M(ModeZeropageXRMW);
        case 6:                       return M(ModeImplied);
        case 7: if      (opCode == 4) return M(ModeAbsoluteYWrite);
                else if (opCode == 5) return M(ModeAbsoluteYRead);
                else                  return M(ModeAbsoluteXRMW);
        }
        break;
    case 3: // Combined ALU/RMW instructions
        switch (opMode) {
        case 0: if      (opCode == 4) return M(ModeIndirectXWrite);
                else if (opCode == 5) return M(ModeIndirectXRead);
                else                  return M(ModeIndirectXRMW);
        case 1: if      (opCode == 4) return M(ModeZeropageWrite);
                else if (opCode == 5) return M(ModeZeropageRead);
                else                  return M(ModeZeropageRMW);
        case 2:                       return M(ModeImmediate);
        case 3: if      (opCode == 4) return M(ModeAbsoluteWrite);
                else if (opCode == 5) return M(ModeAbsoluteRead);
                else                  return M(ModeAbsoluteRMW); 
        case 4: if      (opCode == 4) return M(ModeIndirectYWrite);
                else if (opCode == 5) return M(ModeIndirectYRead);
                else                  return M(ModeIndirectYRMW);
        case 5: if      (opCode == 4) return M(ModeZeropageYWrite);
                else if (opCode == 5) return M(ModeZeropageYRead);
                else                  return M(ModeZeropageXRMW);
        case 6: if      (opCode == 4) return M(ModeAbsoluteYWrite);
                else if (opCode == 5) return M(ModeAbsoluteYRead);
                else                  return M(ModeAbsoluteYRMW);
        case 7: if      (opCode == 4) return M(ModeAbsoluteYWrite);
                else if (opCode == 5) return M(ModeAbsoluteYRead);
                else                  return M(ModeAbsoluteXRMW);
        }
        break;
    }
}

void Ricoh_RP2A03::ModeImplied() {
    operations.push(M(read_PC_to_operand));
}
void Ricoh_RP2A03::ModeImmediate() {
    operations.push(M(read_PC_to_operand));
    operations.push(M(increment_PC));
}
void Ricoh_RP2A03::ModeRelative() {
    operations.push(M(read_PC_to_operand));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::ModeAbsoluteRead() {
    ModeAbsoluteWrite();
    operations.push(M(read_address_to_operand));
}
void Ricoh_RP2A03::ModeAbsoluteRMW() {
    ModeAbsoluteWrite();
    operations.push(M(read_address_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(write_operand_to_address));
}
void Ricoh_RP2A03::ModeAbsoluteWrite() {
    operations.push(M(read_PC_to_addressLo));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(read_PC_to_addressHi));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::ModeZeropageRead() {
    ModeZeropageWrite();
    operations.push(M(read_address_to_operand));
}
void Ricoh_RP2A03::ModeZeropageRMW() {
    ModeZeropageWrite();
    operations.push(M(read_address_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(write_operand_to_address));
}
void Ricoh_RP2A03::ModeZeropageWrite() {
    operations.push(M(read_PC_to_address));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::ModeZeropageXRead() {
    ModeZeropageXWrite();
    operations.push(M(read_address_to_operand));
}
void Ricoh_RP2A03::ModeZeropageXRMW() {
    ModeZeropageXWrite();
    operations.push(M(read_address_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(write_operand_to_address));
}
void Ricoh_RP2A03::ModeZeropageXWrite() {
    operations.push(M(read_PC_to_address));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(read_address_to_operand));
    operations.push(M(index_address_by_X));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::ModeZeropageYRead() {
    ModeZeropageYWrite();
    operations.push(M(read_address_to_operand));
}
void Ricoh_RP2A03::ModeZeropageYWrite() {
    operations.push(M(read_PC_to_address));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(read_address_to_operand));
    operations.push(M(index_address_by_Y));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::detailAbsoluteX() {
    operations.push(M(read_PC_to_addressLo));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(read_PC_to_addressHi));
    operations.push(M(increment_PC));
    operations.push(M(index_address_by_X));
    operations.push(M(end_cycle));
    operations.push(M(read_address_to_operand));
}
void Ricoh_RP2A03::ModeAbsoluteXRead() {
    detailAbsoluteX();
    operations.push(M(fix_address_indexed_by_X));
    operations.push(M(queue_read_if_address_fixed));
}
void Ricoh_RP2A03::ModeAbsoluteXRMW() {
    detailAbsoluteX();
    operations.push(M(fix_address_indexed_by_X));
    operations.push(M(end_cycle));
    operations.push(M(read_address_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(write_operand_to_address));
}
void Ricoh_RP2A03::ModeAbsoluteXWrite() {
    detailAbsoluteX();
    operations.push(M(fix_address_indexed_by_X));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::detailAbsoluteY() {
    operations.push(M(read_PC_to_addressLo));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(read_PC_to_addressHi));
    operations.push(M(increment_PC));
    operations.push(M(index_address_by_Y));
    operations.push(M(end_cycle));
    operations.push(M(read_address_to_operand));
}
void Ricoh_RP2A03::ModeAbsoluteYRead() {
    detailAbsoluteY();
    operations.push(M(fix_address_indexed_by_Y));
    operations.push(M(queue_read_if_address_fixed));
}
void Ricoh_RP2A03::ModeAbsoluteYRMW() {
    detailAbsoluteY();
    operations.push(M(fix_address_indexed_by_Y));
    operations.push(M(end_cycle));
    operations.push(M(read_address_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(write_operand_to_address));
}
void Ricoh_RP2A03::ModeAbsoluteYWrite() {
    detailAbsoluteY();
    operations.push(M(fix_address_indexed_by_Y));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::ModeIndirectXRead() {
    ModeIndirectXWrite();
    operations.push(M(read_address_to_operand));
}
void Ricoh_RP2A03::ModeIndirectXRMW() {
    ModeIndirectXWrite();
    operations.push(M(read_address_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(write_operand_to_address));
}
void Ricoh_RP2A03::ModeIndirectXWrite() {
    operations.push(M(read_PC_to_address));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(read_address_to_operand));
    operations.push(M(index_address_by_X));
    operations.push(M(end_cycle));
    operations.push(M(move_address_to_operand));
    operations.push(M(read_operand_to_addressLo));
    operations.push(M(end_cycle));
    operations.push(M(read_operand_1_to_addressHi));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::detailIndirectY() {
    operations.push(M(read_PC_to_operand));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(read_operand_to_addressLo));
    operations.push(M(end_cycle));
    operations.push(M(read_operand_1_to_addressHi));
    operations.push(M(index_address_by_Y));
    operations.push(M(end_cycle));
    operations.push(M(read_address_to_operand));
}
void Ricoh_RP2A03::ModeIndirectYRead() {
    detailIndirectY();
    operations.push(M(fix_address_indexed_by_Y));
    operations.push(M(queue_read_if_address_fixed));
}
void Ricoh_RP2A03::ModeIndirectYRMW() {
    detailIndirectY();
    operations.push(M(fix_address_indexed_by_Y));
    operations.push(M(end_cycle));
    operations.push(M(read_address_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(write_operand_to_address));
}
void Ricoh_RP2A03::ModeIndirectYWrite() {
    detailIndirectY();
    operations.push(M(fix_address_indexed_by_Y));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::ModePush() {
    operations.push(M(read_PC_to_operand));
    operations.push(M(end_cycle));
}
void Ricoh_RP2A03::ModePull() {
    operations.push(M(read_PC_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::ModeJump() {
    operations.push(M(read_PC_to_addressLo));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(read_PC_to_addressHi));

}
void Ricoh_RP2A03::ModeJumpIndirect() {
    operations.push(M(read_PC_to_addressLo));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(read_PC_to_addressHi));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(read_address_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(read_address_and_operand_to_address));

}

void Ricoh_RP2A03::trigger_interrupt(const Word & interruptVector, const bool & isBRK) {
    vector = interruptVector;
    Pflag =  isBRK ? 1 : 0;
    CheckInterrupts = false;
    operations.push(M(read_PC_to_operand));
    if (isBRK) operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(push_PCH));
    operations.push(M(end_cycle));
    operations.push(M(push_PCL));
    operations.push(M(end_cycle));
    operations.push(M(push_P));
    operations.push(M(end_cycle));
    operations.push(M(SEI));
    operations.push(M(read_vector_to_PCL));
    operations.push(M(end_cycle));
    operations.push(M(read_vector_to_PCH));
    operations.push(M(end_cycle));
}

void Ricoh_RP2A03::ModeBRK() {
    trigger_interrupt(VECTOR_IRQ, true);
}

void Ricoh_RP2A03::ModeJSR() {
    operations.push(M(read_PC_to_addressLo));
    operations.push(M(increment_PC));
    operations.push(M(end_cycle));
    operations.push(M(NOP));
    operations.push(M(end_cycle));
    operations.push(M(push_PCH));
    operations.push(M(end_cycle));
    operations.push(M(push_PCL));
    operations.push(M(end_cycle));
    operations.push(M(read_PC_to_addressHi));
    operations.push(M(JMP));
}

void Ricoh_RP2A03::ModeRTI() {
    operations.push(M(read_PC_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(NOP));
    operations.push(M(end_cycle));
    operations.push(M(pull_P));
    operations.push(M(end_cycle));
    operations.push(M(pull_PCL));
    operations.push(M(end_cycle));
    operations.push(M(pull_PCH));
}

void Ricoh_RP2A03::ModeRTS() {
    operations.push(M(read_PC_to_operand));
    operations.push(M(end_cycle));
    operations.push(M(NOP));
    operations.push(M(end_cycle));
    operations.push(M(pull_PCL));
    operations.push(M(end_cycle));
    operations.push(M(pull_PCH));
    operations.push(M(end_cycle));
    operations.push(M(increment_PC));
}

void Ricoh_RP2A03::queue_read_if_address_fixed() {
    if (AddressWasFixed) {
        // Reverse order because pushing in front
        operations.push_front(M(read_address_to_operand));
        operations.push_front(M(end_cycle));
    }
}

void Ricoh_RP2A03::ASL() { ROLwithFlag(operand, 0);  operations.push_front(M(end_cycle)); operations.push_front(M(write_operand_to_address));
}
void Ricoh_RP2A03::ROL() { ROLwithFlag(operand, C);  operations.push_front(M(end_cycle)); operations.push_front(M(write_operand_to_address)); }
void Ricoh_RP2A03::LSR() { RORwithFlag(operand, 0);  operations.push_front(M(end_cycle)); operations.push_front(M(write_operand_to_address)); }
void Ricoh_RP2A03::ROR() { RORwithFlag(operand, C);  operations.push_front(M(end_cycle)); operations.push_front(M(write_operand_to_address)); }
void Ricoh_RP2A03::DEC() { Transfer(operand - 1, operand);  operations.push_front(M(end_cycle)); operations.push_front(M(write_operand_to_address)); }
void Ricoh_RP2A03::INC() { Transfer(operand + 1, operand);  operations.push_front(M(end_cycle)); operations.push_front(M(write_operand_to_address)); }

void Ricoh_RP2A03::xSLO() {
    C = Bit<Left>(operand);
    Transfer(operand << 1, operand);
    Transfer(A | operand, A);
    operations.push_front(M(write_operand_to_address));
    operations.push_front(M(end_cycle));
}
void Ricoh_RP2A03::xRLA() {
    const auto c = Bit<Left>(operand);
    Transfer((operand << 1) | Mask<Right>(C), operand);
    C = c;
    Transfer(A & operand, A);
    operations.push_front(M(write_operand_to_address));
    operations.push_front(M(end_cycle));
}
void Ricoh_RP2A03::xSRE() {
    C = Bit<Right>(operand);
    Transfer(operand >> 1, operand);
    Transfer(A ^ operand, A);
    operations.push_front(M(write_operand_to_address));
    operations.push_front(M(end_cycle));
}
void Ricoh_RP2A03::xRRA() {
    const auto c = Bit<Right>(operand);
    Transfer((operand >> 1) | Mask<Left>(C), operand);
    C = c;
    AddWithCarry(operand);
    operations.push_front(M(write_operand_to_address));
    operations.push_front(M(end_cycle));
}
void Ricoh_RP2A03::xDCP() {
    Transfer(operand - 1, operand);
    Compare(A, operand);
    operations.push_front(M(write_operand_to_address));
    operations.push_front(M(end_cycle));
}
void Ricoh_RP2A03::xISC() {
    Transfer(operand + 1, operand);
    SubstractWithCarry(operand);
    operations.push_front(M(write_operand_to_address));
    operations.push_front(M(end_cycle));
}

void Ricoh_RP2A03::branch_fix_PCH() {
    if (IsBitSet<Neg>(operand)) {
        if ((PC & 0x00FF) >= operand) {
            PC -= 0x0100;
            //operations.push(M(FetchOpcodeAndIncrementPC);
            operations.push_front(M(end_cycle)); // For correct timing
        }
        else {
            //operations.push(M(FetchOpcodeAndIncrementPC);
            //ConsumeOne();
        }
    }
    else {
        if ((PC & 0x00FF) < operand) {
            PC += 0x0100;
            //operations.push(M(FetchOpcodeAndIncrementPC);
            operations.push_front(M(end_cycle)); // For correct timing
        }
        else {
            //operations.push(M(FetchOpcodeAndIncrementPC);
            //ConsumeOne();
        }
    }
}
void Ricoh_RP2A03::Branch(const bool condition) {
    if (condition) {
        PC = (PC & 0xFF00) + ((PC + operand) & 0x00FF);
        operations.push(M(branch_fix_PCH));
        operations.push(M(end_cycle));
    }
    else {
        //operations.push(M(read_PC_to_opcode_AND_increment_PC);
    }
}

void Ricoh_RP2A03::do_DMA() {
    if (dmaTicks > 0) {
        --dmaTicks;
        operations.push_front(M(end_cycle));
        operations.push_front(M(do_DMA));
    }
}

Ricoh_RP2A03::Ricoh_RP2A03()
    : PC{ 0 }, S{ 0 }, A{ 0 }, X{ 0 }, Y{ 0 },
    N{ 0 }, V{ 0 }, D{ 0 }, I{ 0 }, Z{ 0 }, C{ 0 },
    Ticks{ 0 },
    IRQ{ false }, IRQLevel{ false },
    NMI{ false }, NMIEdge{ false }, NMIFlipFlop{ false },
    CycleActive{ false }
{
    // Build addressing mode LUT from opcode decoding
    for (int opcode = 0; opcode < 0x100; ++opcode) {
        modes[opcode] = GetAddressingMode(opcode);
    }
    modes[0x00] = M(ModeBRK);          // BRK
    modes[0x20] = M(ModeJSR);          // JSR
    modes[0x40] = M(ModeRTI);          // RTI
    modes[0x60] = M(ModeRTS);          // RTS
    modes[0x08] = M(ModePush);         // PHP
    modes[0x28] = M(ModePull);         // PLP
    modes[0x48] = M(ModePush);         // PHA
    modes[0x68] = M(ModePull);         // PLA
    modes[0x4C] = M(ModeJump);         // JMP
    modes[0x6C] = M(ModeJumpIndirect); // JMP

    // Build instruction LUT
    uOpCode = {
//           x0      x1      x2      x3      x4      x5      x6      x7      x8      x9      xA       xB      xC      xD      xE      xF
/* 0x */ M( BRK),M( ORA),M(xHLT),M(xSLO),M(xNOP),M( ORA),M( ASL),M(xSLO),M( PHP),M( ORA),M( ASLa),M(xANC),M(xNOP),M( ORA),M( ASL),M(xSLO),
/* 1x */ M( BPL),M( ORA),M(xHLT),M(xSLO),M(xNOP),M( ORA),M( ASL),M(xSLO),M( CLC),M( ORA),M(xNOP), M(xSLO),M(xNOP),M( ORA),M( ASL),M(xSLO),
/* 2x */ M( JSR),M( AND),M(xHLT),M(xRLA),M( BIT),M( AND),M( ROL),M(xRLA),M( PLP),M( AND),M( ROLa),M(xANC),M( BIT),M( AND),M( ROL),M(xRLA),
/* 3x */ M( BMI),M( AND),M(xHLT),M(xRLA),M(xNOP),M( AND),M( ROL),M(xRLA),M( SEC),M( AND),M(xNOP), M(xRLA),M(xNOP),M( AND),M( ROL),M(xRLA),
/* 4x */ M( RTI),M( EOR),M(xHLT),M(xSRE),M(xNOP),M( EOR),M( LSR),M(xSRE),M( PHA),M( EOR),M( LSRa),M(xALR),M( JMP),M( EOR),M( LSR),M(xSRE),
/* 5x */ M( BVC),M( EOR),M(xHLT),M(xSRE),M(xNOP),M( EOR),M( LSR),M(xSRE),M( CLI),M( EOR),M(xNOP), M(xSRE),M(xNOP),M( EOR),M( LSR),M(xSRE),
/* 6x */ M( RTS),M( ADC),M(xHLT),M(xRRA),M(xNOP),M( ADC),M( ROR),M(xRRA),M( PLA),M( ADC),M( RORa),M(xARR),M( JMP),M( ADC),M( ROR),M(xRRA),
/* 7x */ M( BVS),M( ADC),M(xHLT),M(xRRA),M(xNOP),M( ADC),M( ROR),M(xRRA),M( SEI),M( ADC),M(xNOP), M(xRRA),M(xNOP),M( ADC),M( ROR),M(xRRA),
/* 8x */ M(xNOP),M( STA),M(xNOP),M(xSAX),M( STY),M( STA),M( STX),M(xSAX),M( DEY),M(xNOP),M( TXA), M(xXAA),M( STY),M( STA),M( STX),M(xSAX),
/* 9x */ M( BCC),M( STA),M(xHLT),M(xAHX),M( STY),M( STA),M( STX),M(xSAX),M( TYA),M( STA),M( TXS), M(xTAS),M(xSHY),M( STA),M(xSHX),M(xAHX),
/* Ax */ M( LDY),M( LDA),M( LDX),M(xLAX),M( LDY),M( LDA),M( LDX),M(xLAX),M( TAY),M( LDA),M( TAX), M(xLAX),M( LDY),M( LDA),M( LDX),M(xLAX),
/* Bx */ M( BCS),M( LDA),M(xHLT),M(xLAX),M( LDY),M( LDA),M( LDX),M(xLAX),M( CLV),M( LDA),M( TSX), M(xLAS),M( LDY),M( LDA),M( LDX),M(xLAX),
/* Cx */ M( CPY),M( CMP),M(xNOP),M(xDCP),M( CPY),M( CMP),M( DEC),M(xDCP),M( INY),M( CMP),M( DEX), M(xAXS),M( CPY),M( CMP),M( DEC),M(xDCP),
/* Dx */ M( BNE),M( CMP),M(xHLT),M(xDCP),M(xNOP),M( CMP),M( DEC),M(xDCP),M( CLD),M( CMP),M(xNOP), M(xDCP),M(xNOP),M( CMP),M( DEC),M(xDCP),
/* Ex */ M( CPX),M( SBC),M(xNOP),M(xISC),M( CPX),M( SBC),M( INC),M(xISC),M( INX),M( SBC),M( NOP), M(xSBC),M( CPX),M( SBC),M( INC),M(xISC),
/* Fx */ M( BEQ),M( SBC),M(xHLT),M(xISC),M(xNOP),M( SBC),M( INC),M(xISC),M( SED),M( SBC),M(xNOP), M(xISC),M(xNOP),M( SBC),M( INC),M(xISC),
    };
}

void Ricoh_RP2A03::Phi1() {
    if (Halted) return;

    ++Ticks;
    CycleActive = true;
    while (CycleActive) ConsumeOne();
    INSTR = (operations.empty() | (operations.first() == M(fetch_opcode)));
}

void Ricoh_RP2A03::Phi2() {
    if (Halted) return;

    IRQLevel = false;
    if (CheckInterrupts) {
        NMIFlipFlop |= (!NMIEdge && NMI);
        IRQLevel = IRQ;
    }
    NMIEdge = NMI;
}

void Ricoh_RP2A03::DMA(const Byte & fromHi, Byte * to, const Byte & offset) {
    operations.push_front(M(do_DMA));
    dmaSource = (fromHi << BYTE_WIDTH);
    dmaTarget = to;
    dmaOffset = offset;
    dmaTicks = 513;

    const Word base = dmaSource;
    for (Word i = 0; i < 0x0100; ++i) {
        to[(i + offset) & WORD_LO_MASK] = GetByteAt(base + i);
    }
}

void Ricoh_RP2A03::SetStatus(const Byte & status) {
    N = Bit<Neg>(status);
    V = Bit<Ovf>(status);
    D = Bit<Dec>(status);
    I = Bit<Int>(status);
    Z = Bit<Zer>(status);
    C = Bit<Car>(status);
}

Byte Ricoh_RP2A03::GetStatus(const Flag B) const {
    return
        Mask<Neg>(N) | Mask<Ovf>(V) |
        Mask<Unu>(1) | Mask<Brk>(B) |
        Mask<Dec>(D) | Mask<Int>(I) |
        Mask<Zer>(Z) | Mask<Car>(C);
}

void Ricoh_RP2A03::Push(const Byte & value) {
    SetByteAt(STACK_PAGE + S, value);
    --S;
}

Byte Ricoh_RP2A03::Pull() {
    ++S; 
    return GetByteAt(STACK_PAGE + S);
}
