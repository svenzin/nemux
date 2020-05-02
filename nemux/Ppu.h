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
#include <iostream>
#include <iomanip>

static constexpr size_t FRAME_WIDTH = 256;
static constexpr size_t FRAME_HEIGHT = 240;
static constexpr size_t VIDEO_WIDTH = 341;
static constexpr size_t VIDEO_HEIGHT = 262;
static constexpr size_t VIDEO_SIZE = VIDEO_WIDTH * VIDEO_HEIGHT;

class Ppu {
public:
    class PpuBus {
        size_t Ticks0_4;
        size_t Ticks5_7;
        Byte content;
    public:
        size_t Ticks;
        void WriteLo(const Byte value) {
            content = (content & 0xE0) | (value & 0x1F);
            Ticks0_4 = Ticks;
        }
        void WriteHi(Byte value) {
            content = (content & 0x1F) | (value & 0xE0);
            Ticks5_7 = Ticks;
        }
        void Write(Byte value) {
            content = value;
            Ticks0_4 = Ticks5_7 = Ticks;
        }
        Byte Read() const {
            Byte bus = 0;
            if ((Ticks - Ticks0_4) < size_t{ 2700000 }) bus += (content & 0x1F);
            if ((Ticks - Ticks5_7) < size_t{ 2700000 }) bus += (content & 0xE0);
            return bus;
        }
        Byte ReadLo() const {
            if ((Ticks - Ticks0_4) < size_t{ 2700000 }) return (content & 0x1F);
            return 0;
        }
        Byte ReadPaletteHi() const {
            if ((Ticks - Ticks5_7) < size_t{ 2700000 }) return (content & 0xC0);
            return 0;
        }
    } Bus;

    void WriteControl1(Byte value) {
        Bus.Write(value);

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
        Bus.Write(value);

        IsColour       = IsBitClear<0>(value);
        ClipBackground = IsBitClear<1>(value);
        ClipSprite     = IsBitClear<2>(value);
        ShowBackground = IsBitSet<3>(value);
        ShowSprite     = IsBitSet<4>(value);
        ColourIntensity = ((value >> 5) & 0x07);
    }

    Byte ReadStatus() {
        Latch.Reset();
        const auto status = Bus.ReadLo()
            | Mask<5>(SpriteOverflow)
            | Mask<6>(SpriteZeroHit)
            | Mask<7>(VBlank);
        VBlank = false;
        
        StatusReadOn = FrameTicks;

        //WriteHiBus(status);
        return status;
    }

    void WriteOAMAddress(Byte value) {
        Bus.Write(value);

        OAMAddress = value;
    }

    Byte ReadOAMData() {
        auto x = SprRam[OAMAddress];
        if ((OAMAddress % 4) == 2) x &= 0xE3;
        Bus.Write(x);
        return x;
    }

    void WriteOAMData(Byte value) {
        Bus.Write(value);

        SprRam[OAMAddress] = value;
        ++OAMAddress;
    }

    void WriteScroll(Byte value) {
        Bus.Write(value);

        if (Latch) {
            ScrollX = value;
        } else {
            ScrollY = value;
        }
        Latch.Step();
    }

    void WriteAddress(Byte value) {
        Bus.Write(value);

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
            data = Bus.ReadPaletteHi() | PpuPalette.ReadAt(Address - 0x3F00);
            ReadDataBuffer = Map->GetByteAt(Address & 0x2FFF);
        } else {
            data = ReadDataBuffer;
            ReadDataBuffer = Map->GetByteAt(Address);
        }
        Address = (Address + AddressIncrement) & 0x3FFF;
        Bus.Write(data);
        return data;
    }

    void WriteData(Byte value) {
        Bus.Write(value);

        if (Address >= 0x3F00) {
            PpuPalette.WriteAt(Address - 0x3F00, value);
        } else {
            Map->SetByteAt(Address, value);
        }
        Address = (Address + AddressIncrement) & 0x3FFF;
    }

    Byte SpriteMultiplexer(Byte background, Byte sprite, bool isBehind) const {
        if ((background & 0x03) != 0) {
            if ((sprite & 0x03) != 0) {
                if (isBehind) {
                    return background;
                }
                else {
                    return sprite;
                }
            }
            else {
                return background;
            }
        }
        else {
            if ((sprite & 0x03) != 0) {
                return sprite;
            }
            else {
                return 0;
            }
        }
    }

    bool SpriteHit(Byte background, Byte foreground, unsigned int x) const {
        if (x == 255) return false;
        if (!ShowBackground || !ShowSprite) return false;
        if ((ClipBackground || ClipSprite) && (x < 8)) return false;

        const bool hit = (((background & 0x03) > 0) && ((foreground & 0x03) > 0));
        return hit;
    }

    std::vector<std::tuple<int, std::array<Byte, 4>>> sprites;
    void Tick() {
        ++Bus.Ticks;
        
        const auto y = FrameTicks / VIDEO_WIDTH;
        const auto x = FrameTicks % VIDEO_WIDTH;
        //if ((FrameCount%2)==0)
        if ((y < FRAME_HEIGHT) && (x < FRAME_WIDTH)) {
            if (x == 0) {
                sprites.clear();
                for (auto s = 0; s < 64; s++) {
                    const auto sy = y - SprRam[4 * s + 0] - 1;
                    if ((0 <= sy) && (sy < SpriteHeight)) {
                        sprites.push_back({
                            s,
                            {
                                SprRam[4 * s + 0], SprRam[4 * s + 1],
                                SprRam[4 * s + 2], SprRam[4 * s + 3]
                            }
                        });
                    }
                }
            }
            auto bg = 0;
            auto fg = 0;
            bool isbg = true;
            if (ShowBackground) {
                auto tx = (x + ScrollX) / 8; const auto xx = (x + ScrollX) % 8;
                auto ty = (y + ScrollY) / 8; const auto yy = (y + ScrollY) % 8;
                auto nt = NameTable;
                if (tx >= 32) { tx -= 32; nt ^= 0x0400; }
                if (ty >= 30) { ty -= 30; nt ^= 0x0800; }
                const auto td = Map->GetByteAt(nt + 32 * ty + tx);
                const auto taddr = BackgroundTable + 16 * td + yy;
                auto b = Map->GetByteAt(taddr);
                auto v = (b >> (7 - xx)) & 0x01;
                b = Map->GetByteAt(taddr + 8);
                v += ((b >> (7 - xx)) & 0x01) << 1;
                const auto atx = tx / 4; const auto aty = ty / 4;
                const auto a = Map->GetByteAt(nt + 0x03C0 + 8 * aty + atx);
                v += ((a >> (2 * (tx / 2 % 2) + 4 * (ty / 2 % 2))) & 0x3) << 2;
                bg = v;
            }
            if (ShowSprite) {
                for (auto i = 0; i < sprites.size(); ++i) {
                    auto sprite = sprites[i];
                    auto s = std::get<0>(sprite);
                    auto data = std::get<1>(sprite);
                    const auto sy = y - data[0] - 1;
                    if ((0 <= sy) && (sy < SpriteHeight)) {
                        const auto sx = x - data[3];
                        if ((0 <= sx) && (sx < 8)) {
                            const auto td = data[1];
                            const auto at = data[2];
                            const auto flipX = IsBitSet<6>(at);
                            const auto flipY = IsBitSet<7>(at);
                            isbg = IsBitSet<5>(at);
                            int taddr;
                            if (flipY) taddr = (SpriteHeight - 1 - sy);
                            else taddr = sy;
                            if (SpriteHeight == 8) taddr += SpriteTable + 16 * td;
                            else {
                                if (taddr >= 8) taddr += 8;
                                taddr += ((td % 2) * 0x1000) + 16 * (td & 0xFE);
                            }
                            auto b = Map->GetByteAt(taddr);
                            int v;
                            if (flipX) v = (b >> sx) & 0x01;
                            else v = (b >> (7 - sx)) & 0x01;
                            b = Map->GetByteAt(taddr + 8);
                            if (flipX) v += ((b >> sx) & 0x01) << 1;
                            else v += ((b >> (7 - sx)) & 0x01) << 1;
                            v += (at & 0x3) << 2;
                            v += 0x10;
                            fg = v;

                            if (s == 0)
                                SpriteZeroHit = SpriteZeroHit || SpriteHit(bg, fg, x);
                            if ((fg & 0x03) != 0) break;
                        }
                    }
                }
            }
            const auto ci = SpriteMultiplexer(bg, fg, isbg);
            Frame[VIDEO_WIDTH * y + x] = PpuPalette.ReadAt(ci);
        }

        //// NMI is activated on tick 1 (second tick) of scanline 241
        //if ((y == 241) && (x == 1) && NMIOnVBlank)
        //    NMIActive = true;
        //// NMI is deactivated after the last tick of scanline 260 (i.e. on (0, 261) (?))
        //if ((y == 261) && (x == 0))
        //    NMIActive = false;
        static constexpr size_t VBL_START = 241 * 341 + 1;
        static constexpr size_t VBL_STOP = 261 * 341 + 1;
        
        // VBlank buffers to account for delay in CPU detection
        static bool vblDelayed1 = false;
        static bool vblDelayed2 = false;
        vblDelayed2 = vblDelayed1;
        vblDelayed1 = VBlank;

        const bool SuppressVBlank = (StatusReadOn == VBL_START);
        if (FrameTicks == VBL_START) VBlank = !SuppressVBlank;
        if (FrameTicks == VBL_STOP) VBlank = false;

        NMIActive = (NMIOnVBlank && vblDelayed2);

        static const auto SPRITE_ZERO_HIT_RESET = 261 * 341;
        if (FrameTicks == SPRITE_ZERO_HIT_RESET) SpriteZeroHit = false;
        
        ++FrameTicks;
        if ((FrameCount % 2 == 1)
            && (FrameTicks == (VIDEO_SIZE - 2))
            && (ShowBackground || ShowSprite)
            )
            ++FrameTicks;

        if (FrameTicks == VIDEO_SIZE) {
            FrameTicks = 0;
            ++FrameCount;

            StatusReadOn = 0;
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
        VBlank = false;// true;

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

        StatusReadOn = 0;
    }

    struct {
        bool Status = true;
        operator bool() const { return Status; }
        void Reset() { Status = true; }
        void Step() { Status = !Status; }
    } Latch;

    std::array<Byte, 0x0100> SprRam;
    Byte OAMAddress;

    //bool IgnoreVramWrites;
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

    size_t StatusReadOn;
};

#endif /* PPU_H_ */
