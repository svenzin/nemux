#include "Ricoh_RP2A03.h"

Ricoh_RP2A03::AddressingMode_f GetAddressingMode(Byte opcode) {
    const Byte opType = (opcode & 0b00000011);
    const Byte opMode = ((opcode & 0b00011100) >> 2);
    const Byte opCode = ((opcode & 0b11100000) >> 5);

    switch (opType) {
    case 0: // Control instructions
        switch (opMode) {
        case 0:                  return & Ricoh_RP2A03::ModeImmediate;
        case 1: if (opCode == 4) return & Ricoh_RP2A03::ModeZeropageWrite;
                else             return & Ricoh_RP2A03::ModeZeropageRead;
        case 2:                  return & Ricoh_RP2A03::ModeImplied;
        case 3: if (opCode == 4) return & Ricoh_RP2A03::ModeAbsoluteWrite;
                else             return & Ricoh_RP2A03::ModeAbsoluteRead;
        case 4:                  return & Ricoh_RP2A03::ModeRelative;
        case 5: if (opCode == 4) return & Ricoh_RP2A03::ModeZeropageXWrite;
                else             return & Ricoh_RP2A03::ModeZeropageXRead;
        case 6:                  return & Ricoh_RP2A03::ModeImplied;
        case 7: if (opCode == 4) return & Ricoh_RP2A03::ModeAbsoluteXWrite;
                else             return & Ricoh_RP2A03::ModeAbsoluteXRead;
        }
        break;
    case 1: // ALU instructions
        switch (opMode) {
        case 0: if (opCode == 4) return & Ricoh_RP2A03::ModeIndirectXWrite;
                else             return & Ricoh_RP2A03::ModeIndirectXRead;
        case 1: if (opCode == 4) return & Ricoh_RP2A03::ModeZeropageWrite;
                else             return & Ricoh_RP2A03::ModeZeropageRead;
        case 2:                  return & Ricoh_RP2A03::ModeImmediate;
        case 3: if (opCode == 4) return & Ricoh_RP2A03::ModeAbsoluteWrite;
                else             return & Ricoh_RP2A03::ModeAbsoluteRead;
        case 4: if (opCode == 4) return & Ricoh_RP2A03::ModeIndirectYWrite;
                else             return & Ricoh_RP2A03::ModeIndirectYRead;
        case 5: if (opCode == 4) return & Ricoh_RP2A03::ModeZeropageXWrite;
                else             return & Ricoh_RP2A03::ModeZeropageXRead;
        case 6: if (opCode == 4) return & Ricoh_RP2A03::ModeAbsoluteYWrite;
                else             return & Ricoh_RP2A03::ModeAbsoluteYRead;
        case 7: if (opCode == 4) return & Ricoh_RP2A03::ModeAbsoluteXWrite;
                else             return & Ricoh_RP2A03::ModeAbsoluteXRead;
        }
        break;
    case 2: // RMW instructions
        switch (opMode) {
        case 0:                       return & Ricoh_RP2A03::ModeImmediate;
        case 1: if      (opCode == 4) return & Ricoh_RP2A03::ModeZeropageWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeZeropageRead;
                else                  return & Ricoh_RP2A03::ModeZeropageRMW;
        case 2:                       return & Ricoh_RP2A03::ModeImplied;
        case 3: if      (opCode == 4) return & Ricoh_RP2A03::ModeAbsoluteWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeAbsoluteRead;
                else                  return & Ricoh_RP2A03::ModeAbsoluteRMW;
        case 4:                       return & Ricoh_RP2A03::ModeImplied;
        case 5: if      (opCode == 4) return & Ricoh_RP2A03::ModeZeropageYWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeZeropageYRead;
                else                  return & Ricoh_RP2A03::ModeZeropageXRMW;
        case 6:                       return & Ricoh_RP2A03::ModeImplied;
        case 7: if      (opCode == 4) return & Ricoh_RP2A03::ModeAbsoluteYWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeAbsoluteYRead;
                else                  return & Ricoh_RP2A03::ModeAbsoluteXRMW;
        }
        break;
    case 3: // Combined ALU/RMW instructions
        switch (opMode) {
        case 0: if      (opCode == 4) return & Ricoh_RP2A03::ModeIndirectXWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeIndirectXRead;
                else                  return & Ricoh_RP2A03::ModeIndirectXRMW;
        case 1: if      (opCode == 4) return & Ricoh_RP2A03::ModeZeropageWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeZeropageRead;
                else                  return & Ricoh_RP2A03::ModeZeropageRMW; 
        case 2:                       return & Ricoh_RP2A03::ModeImmediate;
        case 3: if      (opCode == 4) return & Ricoh_RP2A03::ModeAbsoluteWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeAbsoluteRead;
                else                  return & Ricoh_RP2A03::ModeAbsoluteRMW; 
        case 4: if      (opCode == 4) return & Ricoh_RP2A03::ModeIndirectYWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeIndirectYRead;
                else                  return & Ricoh_RP2A03::ModeIndirectYRMW;
        case 5: if      (opCode == 4) return & Ricoh_RP2A03::ModeZeropageYWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeZeropageYRead;
                else                  return & Ricoh_RP2A03::ModeZeropageXRMW;
        case 6: if      (opCode == 4) return & Ricoh_RP2A03::ModeAbsoluteYWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeAbsoluteYRead;
                else                  return & Ricoh_RP2A03::ModeAbsoluteYRMW;
        case 7: if      (opCode == 4) return & Ricoh_RP2A03::ModeAbsoluteYWrite;
                else if (opCode == 5) return & Ricoh_RP2A03::ModeAbsoluteYRead;
                else                  return & Ricoh_RP2A03::ModeAbsoluteXRMW;
        }
        break;
    }
}

Ricoh_RP2A03::Ricoh_RP2A03()
    : PC{ 0 }, S{ 0 }, A{ 0 }, X{ 0 }, Y{ 0 },
    N{ 0 }, V{ 0 }, D{ 0 }, I{ 0 }, Z{ 0 }, C{ 0 },
    Ticks{ 0 },
    IRQ{ false }, IRQLevel{ false },
    NMI{ false }, NMIEdge{ false }, NMIFlipFlop{ false },
    ihead{ 0 }, itail{ 0 }
{
    // Build addressing mode LUT from opcode decoding
    for (int opcode = 0; opcode < 0x100; ++opcode) {
        modes[opcode] = GetAddressingMode(opcode);
    }
}

void Ricoh_RP2A03::Phi1() {
    if (Halted) return;

    ++Ticks;
    if (todo_empty())
        todo_push(read_PC_to_opcode_AND_increment_PC);
    ConsumeOne();
    INSTR = (todo_empty() | (todo[itail] == read_PC_to_opcode_AND_increment_PC));
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
    todo_push_first(do_DMA);
    dmaSource = (fromHi << BYTE_WIDTH);
    dmaTarget = to;
    dmaOffset = offset;
    dmaTicks = 513;

    const Word base = dmaSource;
    for (Word i = 0; i < 0x0100; ++i) {
        to[(i + offset) & WORD_LO_MASK] = Map->GetByteAt(base + i);
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
    return Mask<Neg>(N) | Mask<Ovf>(V) |
        Mask<Unu>(1) | Mask<Brk>(B) |
        Mask<Dec>(D) | Mask<Int>(I) |
        Mask<Zer>(Z) | Mask<Car>(C);
}
