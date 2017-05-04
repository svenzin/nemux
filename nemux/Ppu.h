/*
* Cpu.h
*
*  Created on: 03 Apr 2017
*      Author: scorder
*/

#ifndef PPU_H_
#define PPU_H_

#include "Types.h"

#include <array>

typedef Byte Flag;

template <size_t bit> Byte Mask(const bool & isset) {
    return isset ? 0x01 << bit : 0x00;
}

template <size_t bit> Flag Bit(const Byte & value) {
    return (value >> bit) & 0x01;
}

template <size_t bit> bool IsBitSet(const Byte & value) {
    return Bit<bit>(value) == 1;
}

template <size_t bit> bool IsBitClear(const Byte & value) {
    return !IsBitSet<bit>(value);
}

class Ppu {
public:
    void WriteControl1(Byte value) {
        const auto bank = (value & 0x03);
        NameTable = 0x2000 + (bank * 0x0400);

        AddressIncrement = IsBitSet<2>(value) ? 0x0020 : 0x0001;
        SpriteTable      = IsBitSet<3>(value) ? 0x1000 : 0x0000;
        BackgroundTable  = IsBitSet<4>(value) ? 0x1000 : 0x0000;
        SpriteHeight     = IsBitSet<5>(value) ?     16 :      8;
        //Mode             = IsBitSet<6>(value) ? Master :  Slave;
        NMIOnVBlank      = Bit<7>(value);
    }

    void WriteControl2(Byte value) {
        IsColour       = IsBitClear<0>(value);
        ClipBackground = IsBitClear<1>(value);
        ClipSprite     = IsBitClear<2>(value);
        ShowBackground = IsBitSet<3>(value);
        ShowSprite     = IsBitSet<4>(value);
        ColourIntensity = ((value >> 5) & 0x07);
    }

    Byte ReadStatus() {
        Latch.Reset();
        return Mask<4>(IgnoreVramWrites)
            | Mask<5>(SpriteOverflow)
            | Mask<6>(SpriteZeroHit)
            | Mask<7>(VBlank);
    }

    void WriteOAMAddress(Byte value) {
        OAMAddress = value;
    }

    Byte ReadOAMData() const {
        return SprRam[OAMAddress];
    }

    void WriteOAMData(Byte value) {
        SprRam[OAMAddress] = value;
        ++OAMAddress;
    }

    void WriteScroll(Byte value) {
        if (Latch) {
            ScrollX = value;
        } else {
            ScrollY = value;
        }
        Latch.Step();
    }

    void WriteAddress(Byte value) {
        if (Latch) {
            Address = ((Address & WORD_LO_MASK)
                      | (value << BYTE_WIDTH)
                      & 0x3FFF);
        } else {
            Address = (Address & WORD_HI_MASK) | value;
        }
        Latch.Step();
    }

    explicit Ppu() {
        OAMAddress = 0x00;

        //IgnoreVramWrites;
        SpriteOverflow = true;
        SpriteZeroHit = false;
        VBlank = true;

        IsColour = true;
        ClipBackground = true;
        ClipSprite = true;
        ShowBackground = false;
        ShowSprite = false;
        ColourIntensity = 0x00;

        SpriteHeight = 8;

        NameTable = 0x2000;
        SpriteTable = 0x0000;
        BackgroundTable = 0x0000;
        AddressIncrement = 0x0001;
        NMIOnVBlank = 0;

        ScrollX = 0x00;
        ScrollY = 0x00;

        Address = 0x0000;
    }

    struct {
        bool Status = true;
        operator bool() const { return Status; }
        void Reset() { Status = true; }
        void Step() { Status = !Status; }
    } Latch;

    std::array<Byte, 0x0100> SprRam;
    Byte OAMAddress;

    bool IgnoreVramWrites;
    bool SpriteOverflow;
    bool SpriteZeroHit;
    bool VBlank;

    bool IsColour;
    bool ClipBackground;
    bool ClipSprite;
    bool ShowBackground;
    bool ShowSprite;
    Byte ColourIntensity;

    int SpriteHeight;
    
    Word NameTable;
    Word SpriteTable;
    Word BackgroundTable;
    Word AddressIncrement;
    Flag NMIOnVBlank;

    Byte ScrollX;
    Byte ScrollY;

    Word Address;
};

#endif /* PPU_H_ */
