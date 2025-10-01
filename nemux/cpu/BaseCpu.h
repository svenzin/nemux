#pragma once

#include "Types.h"
#include "MemoryMap.h"
#include "CircularQueue.h"

#include <string>
#include <vector>

#include <iostream>

namespace Address {
    constexpr Word VECTOR_NMI = 0xFFFA;
    constexpr Word VECTOR_RST = 0xFFFC;
    constexpr Word VECTOR_IRQ = 0xFFFE;
    constexpr Word STACK_PAGE = 0x0100;
}

struct BaseCpu {
    enum Bits : size_t {
        Car, Zer, Int, Dec, Brk, Unu, Ovf, Neg,
        Left = 7, Right = 0,
    };

    Word VectorRST{ Address::VECTOR_RST };
    Word VectorNMI{ Address::VECTOR_NMI };
    Word VectorIRQ{ Address::VECTOR_IRQ };
    Word StackPage{ Address::STACK_PAGE };
    
    Word PC;
    Byte S, A, X, Y;
    Flag N, V, D, I, Z, C;
    bool IRQ, NMI;

    constexpr void SetStatusByte(Byte status) {
        N = Bit<Neg>(status);
        V = Bit<Ovf>(status);
        D = Bit<Dec>(status);
        I = Bit<Int>(status);
        Z = Bit<Zer>(status);
        C = Bit<Car>(status);
    }

    constexpr Byte GetStatusByte(const Flag B) const {
        return
            Mask<Neg>(N) | Mask<Ovf>(V) |
            Mask<Unu>(1) | Mask<Brk>(B) |
            Mask<Dec>(D) | Mask<Int>(I) |
            Mask<Zer>(Z) | Mask<Car>(C);
    }

    virtual void PowerUp() = 0;
    virtual void Reset() = 0;
};
