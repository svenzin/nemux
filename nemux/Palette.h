#ifndef PALETTE_H_
#define PALETTE_H_

#include "Types.h"

#include <array>

class Palette {
    Byte Mirror(const Byte Address) const {
        if ((Address & 0x03) == 0) {
            return Address & 0x0F;
        }
        return Address & 0x1F;
    }

public:
    void WriteAt(const Byte Address, const Byte value) {
        Data[Mirror(Address)] = value;
    }

    Byte ReadAt(const Byte Address) const {
        return Data[Mirror(Address)] & 0x3F;
    }

    std::array<Byte, 0x20> Data;
};

#endif // PALETTE_H_
