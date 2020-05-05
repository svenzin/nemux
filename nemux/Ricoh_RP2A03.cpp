#include "Ricoh_RP2A03.h"

Ricoh_RP2A03::Ricoh_RP2A03()
    : PC{ 0 }, S{ 0 }, A{ 0 }, X{ 0 }, Y{ 0 },
    N{ 0 }, V{ 0 }, D{ 0 }, I{ 0 }, Z{ 0 }, C{ 0 },
    Ticks{ 0 },
    IRQ{ false }, IRQLevel{ false },
    NMI{ false }, NMIEdge{ false }, NMIFlipFlop{ false },
    ihead{ 0 }, itail{ 0 }
{}

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
