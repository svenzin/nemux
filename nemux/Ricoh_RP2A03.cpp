#include "Ricoh_RP2A03.h"

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
