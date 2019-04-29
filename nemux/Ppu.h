/*
* Cpu.h
*
*  Created on: 03 Apr 2017
*      Author: scorder
*/

#ifndef PPU_H_
#define PPU_H_

#include "Types.h"
#include "BitUtil.h"
#include "Palette.h"
#include "MemoryMap.h"

#include <array>

static constexpr size_t FRAME_WIDTH = 256;
static constexpr size_t FRAME_HEIGHT = 240;
static constexpr size_t VIDEO_WIDTH = 341;
static constexpr size_t VIDEO_HEIGHT = 262;
static constexpr size_t VIDEO_SIZE = VIDEO_WIDTH * VIDEO_HEIGHT;

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

    Byte ReadData() {
        Byte data;
        if (Address >= 0x3F00) {
            data = PpuPalette.ReadAt(Address - 0x3F00);
            ReadDataBuffer = Map->GetByteAt(Address & 0x2FFF);
        } else {
            data = ReadDataBuffer;
            ReadDataBuffer = Map->GetByteAt(Address);
        }
        Address = (Address + AddressIncrement) & 0x3FFF;
        return data;
    }

    void WriteData(Byte value) {
        if (Address >= 0x3F00) {
            PpuPalette.WriteAt(Address - 0x3F00, value);
        } else {
            Map->SetByteAt(Address, value);
        }
        Address = (Address + AddressIncrement) & 0x3FFF;
    }

    void Tick() {
        const auto y = FrameTicks / VIDEO_WIDTH;
        const auto x = FrameTicks % VIDEO_WIDTH;
        //if ((FrameCount%2)==0)
        if ((y < FRAME_HEIGHT) && (x < FRAME_WIDTH)) {
            const auto tx = x / 8; const auto xx = x % 8;
            const auto ty = y / 8; const auto yy = y % 8;
            const auto td = Map->GetByteAt(0x2000 + 32 * ty + tx);
            const auto taddr = BackgroundTable + 16 * td + yy;
            auto b = Map->GetByteAt(taddr);
            auto v = (b >> (7 - xx)) & 0x01;
            b = Map->GetByteAt(taddr + 8);
            v += ((b >> (7 - xx)) & 0x01) << 1;
            const auto atx = x / 32; const auto aty = y / 32;
            const auto a = Map->GetByteAt(0x23C0 + 8 * aty + atx);
            v += ((a >> (2 * (x / 16 % 2) + 4 * (y / 16 % 2))) & 0x3) << 2;
            const auto ci = PpuPalette.ReadAt(v);
            Frame[VIDEO_WIDTH * y + x] = ci;

            for (auto s = 0; s < 64; s++) {
                const auto sy = y - SprRam[4 * s + 0] - 1;
                if ((0 <= sy) && (sy < 8)) {
                    const auto sx = x - SprRam[4 * s + 3];
                    if ((0 <= sx) && (sx < 8)) {
                        const auto td = SprRam[4 * s + 1];
                        const auto at = SprRam[4 * s + 2];
                        const auto taddr = SpriteTable + 16 * td + sy;
                        auto b = Map->GetByteAt(taddr);
                        auto v = (b >> (7 - sx)) & 0x01;
                        b = Map->GetByteAt(taddr + 8);
                        v += ((b >> (7 - sx)) & 0x01) << 1;
                        v += (at & 0x3) << 2;
                        v += 0x10;
                        if ((v & 0x03) != 0) {
                            const auto ci = PpuPalette.ReadAt(v);
                            Frame[VIDEO_WIDTH * y + x] = ci;
                        }
                    }
                }
            }
        }
        if ((y == 241) && (x == 0))
            NMIActive = true;
        if ((y == 260) && (x == 340))
            NMIActive = false;
        ++FrameTicks;
        if (FrameTicks == VIDEO_SIZE) {
            FrameTicks = 0;
            ++FrameCount;
        }
    }

    std::array<Byte, 89342> FrameBuffer;
    void Render() {
        FrameBuffer.fill(0);
    }

    explicit Ppu(MemoryMap * map = nullptr) {
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

        Map = map;

        Address = 0x0000;
        ReadDataBuffer = 0x00;

        NMIActive = false;
        Ticks = 0;
        Frames = 0;

        FrameTicks = 0;
        FrameCount = 0;
        Frame.fill(0x00);
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

    MemoryMap * Map;

    Byte ReadDataBuffer;

    Palette PpuPalette;

    bool NMIActive;
    int Ticks;
    int Frames;

    size_t FrameTicks;
    size_t FrameCount;
    std::array<Byte, VIDEO_SIZE> Frame;
};

#endif /* PPU_H_ */
