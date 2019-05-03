#ifndef APU_H_
#define APU_H_

#include "Types.h"
//#include "BitUtil.h"
//#include "Palette.h"
//#include "MemoryMap.h"
//
//#include <array>
//#include <iostream>
//#include <iomanip>
//
//static constexpr size_t FRAME_WIDTH = 256;
//static constexpr size_t FRAME_HEIGHT = 240;
//static constexpr size_t VIDEO_WIDTH = 341;
//static constexpr size_t VIDEO_HEIGHT = 262;
//static constexpr size_t VIDEO_SIZE = VIDEO_WIDTH * VIDEO_HEIGHT;

class Apu {
public:
    void WritePulse1Control(const Byte value) {}
    void WritePulse1Sweep(const Byte value) {}
    void WritePulse1PeriodLo(const Byte value) {}
    void WritePulse1PeriodHi(const Byte value) {}
    
    void WritePulse2Control(const Byte value) {}
    void WritePulse2Sweep(const Byte value) {}
    void WritePulse2PeriodLo(const Byte value) {}
    void WritePulse2PeriodHi(const Byte value) {}

    void WriteTriangleControl(const Byte value) {}
    void WriteTrianglePeriodLo(const Byte value) {}
    void WriteTrianglePeriodHi(const Byte value) {}

    void WriteNoiseControl(const Byte value) {}
    void WriteNoisePeriod(const Byte value) {}
    void WriteNoiseLength(const Byte value) {}

    void WriteDMCFrequency(const Byte value) {}
    void WriteDMCDAC(const Byte value) {}
    void WriteDMCAddress(const Byte value) {}
    void WriteDMCLength(const Byte value) {}
    
    void WriteCommonEnable(const Byte value) {}
    void WriteCommonControl(const Byte value) {}

    Byte ReadStatus() const {}

//    void WriteControl1(Byte value) {
//        const auto bank = (value & 0x03);
//        NameTable = 0x2000 + (bank * 0x0400);
//
//        AddressIncrement = IsBitSet<2>(value) ? 0x0020 : 0x0001;
//        SpriteTable = IsBitSet<3>(value) ? 0x1000 : 0x0000;
//        BackgroundTable = IsBitSet<4>(value) ? 0x1000 : 0x0000;
//        SpriteHeight = IsBitSet<5>(value) ? 16 : 8;
//        //Mode             = IsBitSet<6>(value) ? Master :  Slave;
//        NMIOnVBlank = Bit<7>(value);
//
//        //std::cout << std::dec << FrameTicks / 341 << " " << FrameTicks % 341 << " Write $2000 " << std::hex << int(value) << std::endl;
//    }
//
//    void WriteControl2(Byte value) {
//        IsColour = IsBitClear<0>(value);
//        ClipBackground = IsBitClear<1>(value);
//        ClipSprite = IsBitClear<2>(value);
//        ShowBackground = IsBitSet<3>(value);
//        ShowSprite = IsBitSet<4>(value);
//        ColourIntensity = ((value >> 5) & 0x07);
//    }
//
//    Byte ReadStatus() {
//        Latch.Reset();
//        Blanking = false;
//        return Mask<4>(IgnoreVramWrites)
//            | Mask<5>(SpriteOverflow)
//            | Mask<6>(SpriteZeroHit)
//            | Mask<7>(VBlank);
//    }
//
//    void WriteOAMAddress(Byte value) {
//        OAMAddress = value;
//    }
//
//    Byte ReadOAMData() const {
//        return SprRam[OAMAddress];
//    }
//
//    void WriteOAMData(Byte value) {
//        SprRam[OAMAddress] = value;
//        ++OAMAddress;
//    }
//
//    void WriteScroll(Byte value) {
//        if (Latch) {
//            ScrollX = value;
//        }
//        else {
//            ScrollY = value;
//        }
//        Latch.Step();
//    }
//
//    void WriteAddress(Byte value) {
//        if (Latch) {
//            Address = ((Address & WORD_LO_MASK)
//                | (value << BYTE_WIDTH)
//                & 0x3FFF);
//            NameTable = 0x2000 + (((value >> 2) & 0x03) * 0x0400);
//            //std::cout << std::dec << FrameTicks / 341 << " " << FrameTicks % 341 << " Write $2006 " << std::hex << int(value) << " Nametable " << NameTable << std::endl;
//        }
//        else {
//            Address = (Address & WORD_HI_MASK) | value;
//            //std::cout << std::dec << FrameTicks / 341 << " " << FrameTicks % 341 << " Write $2006 " << std::hex << int(value) << std::endl;
//        }
//        Latch.Step();
//    }
//
//    Byte ReadData() {
//        Byte data;
//        if (Address >= 0x3F00) {
//            data = PpuPalette.ReadAt(Address - 0x3F00);
//            ReadDataBuffer = Map->GetByteAt(Address & 0x2FFF);
//        }
//        else {
//            data = ReadDataBuffer;
//            ReadDataBuffer = Map->GetByteAt(Address);
//        }
//        Address = (Address + AddressIncrement) & 0x3FFF;
//        return data;
//    }
//
//    void WriteData(Byte value) {
//        if (Address >= 0x3F00) {
//            PpuPalette.WriteAt(Address - 0x3F00, value);
//        }
//        else {
//            Map->SetByteAt(Address, value);
//        }
//        Address = (Address + AddressIncrement) & 0x3FFF;
//    }
//
//    void Tick() {
//        const auto y = FrameTicks / VIDEO_WIDTH;
//        const auto x = FrameTicks % VIDEO_WIDTH;
//        //if ((FrameCount%2)==0)
//        if ((y < FRAME_HEIGHT) && (x < FRAME_WIDTH)) {
//            auto bg_chr = 0;
//            if (ShowBackground) {
//                auto tx = (x + ScrollX) / 8; const auto xx = (x + ScrollX) % 8;
//                auto ty = (y + ScrollY) / 8; const auto yy = (y + ScrollY) % 8;
//                auto nt = NameTable;
//                if (tx >= 32) { tx -= 32; nt ^= 0x0400; }
//                if (ty >= 30) { ty -= 30; nt ^= 0x0800; }
//                const auto td = Map->GetByteAt(nt + 32 * ty + tx);
//                const auto taddr = BackgroundTable + 16 * td + yy;
//                auto b = Map->GetByteAt(taddr);
//                auto v = (b >> (7 - xx)) & 0x01;
//                b = Map->GetByteAt(taddr + 8);
//                v += ((b >> (7 - xx)) & 0x01) << 1;
//                const auto atx = tx / 4; const auto aty = ty / 4;
//                const auto a = Map->GetByteAt(nt + 0x03C0 + 8 * aty + atx);
//                v += ((a >> (2 * (tx / 2 % 2) + 4 * (ty / 2 % 2))) & 0x3) << 2;
//                const auto ci = PpuPalette.ReadAt(v);
//                bg_chr = (v & 0x03);
//                Frame[VIDEO_WIDTH * y + x] = ci;
//            }
//            if (ShowSprite) {
//                for (auto s = 0; s < 64; s++) {
//                    const auto sy = y - SprRam[4 * s + 0] - 1;
//                    if ((0 <= sy) && (sy < SpriteHeight)) {
//                        const auto sx = x - SprRam[4 * s + 3];
//                        if ((0 <= sx) && (sx < 8)) {
//                            const auto td = SprRam[4 * s + 1];
//                            const auto at = SprRam[4 * s + 2];
//                            const auto flipX = IsBitSet<6>(at);
//                            const auto flipY = IsBitSet<7>(at);
//                            int taddr;
//                            if (flipY) taddr = (SpriteHeight - 1 - sy);
//                            else taddr = sy;
//                            if (SpriteHeight == 8) taddr += SpriteTable + 16 * td;
//                            else {
//                                if (taddr >= 8) taddr += 8;
//                                taddr += ((td % 1) * 0x1000) + 16 * (td & 0xFE);
//                            }
//                            auto b = Map->GetByteAt(taddr);
//                            int v;
//                            if (flipX) v = (b >> sx) & 0x01;
//                            else v = (b >> (7 - sx)) & 0x01;
//                            b = Map->GetByteAt(taddr + 8);
//                            if (flipX) v += ((b >> sx) & 0x01) << 1;
//                            else v += ((b >> (7 - sx)) & 0x01) << 1;
//                            v += (at & 0x3) << 2;
//                            v += 0x10;
//                            if ((v & 0x03) != 0) {
//                                if (s == 0)
//                                    if (bg_chr != 0)
//                                        if (!SpriteZeroHit) {
//                                            SpriteZeroHit = ShowSprite && ShowBackground;
//                                        }
//                                const auto ci = PpuPalette.ReadAt(v);
//                                Frame[VIDEO_WIDTH * y + x] = ci;
//                                break;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//
//        //// NMI is activated on tick 1 (second tick) of scanline 241
//        //if ((y == 241) && (x == 1) && NMIOnVBlank)
//        //    NMIActive = true;
//        //// NMI is deactivated after the last tick of scanline 260 (i.e. on (0, 261) (?))
//        //if ((y == 261) && (x == 0))
//        //    NMIActive = false;
//        static constexpr auto NMI_START = 241 * 341 + 1;
//        static constexpr auto NMI_STOP = 261 * 341;
//        if (FrameTicks == NMI_START) Blanking = true;
//        if (FrameTicks == NMI_STOP) Blanking = false;
//        NMIActive = (NMIOnVBlank && Blanking);
//
//        static const auto SPRITE_ZERO_HIT_RESET = 261 * 341;
//        if (FrameTicks == SPRITE_ZERO_HIT_RESET) SpriteZeroHit = false;
//
//        ++FrameTicks;
//        if (FrameCount % 2 == 1 && FrameTicks == VIDEO_SIZE - 1 && (ShowBackground || ShowSprite))
//            ++FrameTicks;
//        if (FrameTicks == VIDEO_SIZE) {
//            FrameTicks = 0;
//            ++FrameCount;
//        }
//    }
//
//    std::array<Byte, 89342> FrameBuffer;
//    void Render() {
//        FrameBuffer.fill(0);
//    }
//
//    explicit Ppu(MemoryMap * map = nullptr) {
//        OAMAddress = 0x00;
//
//        //IgnoreVramWrites;
//        SpriteOverflow = true;
//        SpriteZeroHit = false;
//        VBlank = true;
//
//        IsColour = true;
//        ClipBackground = true;
//        ClipSprite = true;
//        ShowBackground = false;
//        ShowSprite = false;
//        ColourIntensity = 0x00;
//
//        SpriteHeight = 8;
//
//        NameTable = 0x2000;
//        SpriteTable = 0x0000;
//        BackgroundTable = 0x0000;
//        AddressIncrement = 0x0001;
//        NMIOnVBlank = 0;
//
//        ScrollX = 0x00;
//        ScrollY = 0x00;
//
//        Map = map;
//
//        Address = 0x0000;
//        ReadDataBuffer = 0x00;
//
//        Blanking = false;
//        NMIActive = false;
//        Ticks = 0;
//        Frames = 0;
//
//        FrameTicks = 0;
//        FrameCount = 0;
//        Frame.fill(0x00);
//    }
//
//    struct {
//        bool Status = true;
//        operator bool() const { return Status; }
//        void Reset() { Status = true; }
//        void Step() { Status = !Status; }
//    } Latch;
//
//    std::array<Byte, 0x0100> SprRam;
//    Byte OAMAddress;
//
//    bool IgnoreVramWrites;
//    bool SpriteOverflow;
//    bool SpriteZeroHit;
//    bool VBlank;
//
//    bool IsColour;
//    bool ClipBackground;
//    bool ClipSprite;
//    bool ShowBackground;
//    bool ShowSprite;
//    Byte ColourIntensity;
//
//    int SpriteHeight;
//
//    Word NameTable;
//    Word SpriteTable;
//    Word BackgroundTable;
//    Word AddressIncrement;
//    Flag NMIOnVBlank;
//
//    Byte ScrollX;
//    Byte ScrollY;
//
//    Word Address;
//
//    MemoryMap * Map;
//
//    Byte ReadDataBuffer;
//
//    Palette PpuPalette;
//
//    bool Blanking;
//    bool NMIActive;
//    int Ticks;
//    int Frames;
//
//    size_t FrameTicks;
//    size_t FrameCount;
//    std::array<Byte, VIDEO_SIZE> Frame;
};

#endif /* APU_H_ */
