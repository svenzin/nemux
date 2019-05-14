#ifndef CONTROLLERS_H_
#define CONTROLLERS_H_

#include "Types.h"
#include "BitUtil.h"

#include <array>

class Controllers {
public:
    explicit Controllers() {
    }

    Byte ReadP1() {
        if (Strobe) Latch();
        const auto value = Bit<0>(P1_Latch);
        P1_Latch = Mask<7>(true) | (P1_Latch >> 1);
        return value;
    }

    Byte ReadP2() {
        if (Strobe) Latch();
        const auto value = Bit<0>(P2_Latch);
        P2_Latch = Mask<7>(true) | (P2_Latch >> 1);
        return value;
    }

    void Write(const Byte value) {
        const auto newStrobe = IsBitSet<0>(value);
        if (Strobe && !newStrobe) Latch();
        Strobe = newStrobe;
    }

    void Latch() {
        P1_Latch = Mask<0>(P1_A)
            | Mask<1>(P1_B)
            | Mask<2>(P1_Select)
            | Mask<3>(P1_Start)
            | Mask<4>(P1_Up)
            | Mask<5>(P1_Down)
            | Mask<6>(P1_Left)
            | Mask<7>(P1_Right);
        P2_Latch = Mask<0>(P2_A)
            | Mask<1>(P2_B)
            | Mask<2>(P2_Select)
            | Mask<3>(P2_Start)
            | Mask<4>(P2_Up)
            | Mask<5>(P2_Down)
            | Mask<6>(P2_Left)
            | Mask<7>(P2_Right);
    }

    bool Strobe = false;

    Byte P1_Latch = 0x00;
    bool P1_Up = false;
    bool P1_Down = false;
    bool P1_Left = false;
    bool P1_Right = false;
    bool P1_Start = false;
    bool P1_Select = false;
    bool P1_A = false;
    bool P1_B = false;

    Byte P2_Latch = 0x00;
    bool P2_Up = false;
    bool P2_Down = false;
    bool P2_Left = false;
    bool P2_Right = false;
    bool P2_Start = false;
    bool P2_Select = false;
    bool P2_A = false;
    bool P2_B = false;
};

#endif /* CONTROLLERS_H_ */
