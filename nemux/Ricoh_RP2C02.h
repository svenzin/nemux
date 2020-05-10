#ifndef RICOH_RC2C02_H_
#define RICOH_RC2C02_H_

#include "Types.h"
#include "MemoryMap.h"
#include "CircularQueue.h"

#include <string>
#include <vector>

#include <iostream>

class Ricoh_RP2C02 {
    static constexpr Word XC = 0x001F;
    static constexpr Word YC = 0x03E0;
    static constexpr Word NT = 0x0C00;
    static constexpr Word YF = 0x7000;
public:
    Word v;
    Word t;
    Byte x;
    Flag w;

    void Write2000(const Byte & value) {
        VramIncrement = IsBitSet<2>(value) ? 0x0020 : 0x0001;
        BackgroundTable = IsBitSet<4>(value) ? 0x1000 : 0x0000;

        t = (t & ~NT) | ((value & 0x03) << 10);
    }
    void Read2002() {
        w = 0;
    }
    void Write2005(const Byte & value) {
        if (w == 0) {
            t = (t & ~XC) | ((value & 0xF8) >> 3);
            x = value & 0x07;
            w = 1;
        }
        else {
            t = (t & ~YF) | ((value & 0x07) << 12);
            t = (t & ~YC) | ((value & 0xF8) << 2);
            w = 0;
        }
    }
    void Write2006(const Byte & value) {
        if (w == 0) {
            t = (t & 0x00FF) | ((value & 0x3F) << 8);
            w = 1;
        }
        else {
            t = (t & 0x7F00) | value;
            v = t;
            w = 0;
        }
    }
    void Touch2007() {
        v += VramIncrement;
    }

    void HInc() {
        LatchBG();

        if ((v & XC) == XC) {
            v &= ~XC;
            v ^= 0x0400;
        }
        else {
            ++v;
        }
    }
    void HReset() {
        v &= ~XC;
        v |= (t & XC);
        v &= ~0x0400;
        v |= (t & 0x0400);
    }
    void VInc() {
        if ((v & YF) == YF) {
            v &= ~YF;
            if ((v & YC) == (29 << 5)) {
                v &= ~YC;
                v ^= 0x0800;
            }
            else if ((v & YC) == YC) {
                v &= ~YC;
            }
            else {
                v += 0x0020;
            }
        }
        else {
            v += 0x1000;
        }
    }
    void VReset() {
        v = (v & ~YC) | (t & YC); // vert(v) = vert(t)
        v = (v & ~YF) | (t & YF);
        v = (v & ~0x0800) | (t & 0x0800);
    }

    void ReadNT() {
        aNT = 0x2000 | (v & 0x0FFF);
        bNT = Map->GetByteAt(aNT);
    }
    void ReadAT() {
        aAT = 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x7);
        bAT = Map->GetByteAt(aAT);
    }
    void ReadBGLo() {
        bBGLo = Map->GetByteAt(BackgroundTable + 16 * bNT + ((v & YF) >> 12));
    }
    void ReadBGHi() {
        bBGHi = Map->GetByteAt(BackgroundTable + 16 * bNT + ((v & YF) >> 12) + 8);
    }
    void LatchBG() {
        patternLo &= 0xFF00;
        patternLo |= bBGLo;
        patternHi &= 0xFF00;
        patternHi |= bBGHi;
        Byte b = bAT;
        if ((v & 0b0000000000000010) != 0) b >>= 2;
        if ((v & 0b0000000001000000) != 0) b >>= 4;
        attrLo |= IsBitSet<0>(b) ? 0x00FF : 0x0000;
        attrHi |= IsBitSet<1>(b) ? 0x00FF : 0x0000;
    }
    void BuildBG() {
        const auto bitBGLo = (patternLo >> (15 - x)) & 0x01;
        const auto bitBGHi = (patternHi >> (15 - x)) & 0x01;
        const auto bitATLo = (attrLo >> (15 - x)) & 0x01;
        const auto bitATHi = (attrHi >> (15 - x)) & 0x01;
        bBG = bitBGLo | (bitBGHi << 1) | (bitATLo << 2) | (bitATHi << 3);

        patternLo <<= 1;
        patternHi <<= 1;
        attrLo <<= 1;
        attrHi <<= 1;
    }

    Word BackgroundTable;
    Byte VramIncrement;
    bool ShowBackground;
    bool ShowSprite;

    size_t Ticks;
    size_t Frame;
    bool VBlank;
    size_t ix=0, iy=0;
    void Tick() {
        /*const auto */ix = Ticks % VIDEO_WIDTH;
        /*const auto */iy = Ticks / VIDEO_WIDTH;

        if (Ticks == VBL_START) VBlank = true;
        if (Ticks == VBL_STOP) VBlank = false;

        if (iy < 240) { // Active scanlines
            if (ix == 0) { // Idle
            }
            else if (ix < 257) { // Active frame
                switch (ix % 8) {
                case 0: HInc(); break; // inc hori(v)
                case 1: ReadNT(); break; // fetch NT
                case 3: ReadAT(); break; // fetch AT
                case 5: ReadBGLo(); break; // fetch BGLo
                case 7: ReadBGHi(); break; // fetch BGHi
                }
                BuildBG();
            }
            else if (ix == 257) { // Next line
                HReset();
                VInc();
            }
            else if (ix < 321) { // Inactive
            }
            else if (ix < 337) {  // Prefetch next line
                switch (ix % 8) {
                case 0: HInc(); break; // inc hori(v)
                case 1: ReadNT(); break; // fetch NT
                case 3: ReadAT(); break; // fetch AT
                case 5: ReadBGLo(); break; // fetch BGLo
                case 7: ReadBGHi(); break; // fetch BGHi
                }
                BuildBG();
            }
            else { // Dummy NT fetches
                if (ix % 2 == 1) ReadNT();
            }
        }
        else if (iy < 261) { // Inactive scanlines
        }
        else {  // Prerender line
            // Clear VBlank
            if (ix == 0) { // Idle
            }
            else if (ix < 257) { // Active frame
                switch (ix % 8) {
                case 0: HInc(); break; // inc hori(v)
                case 1: ReadNT(); break; // fetch NT
                case 3: ReadAT(); break; // fetch AT
                case 5: ReadBGLo(); break; // fetch BGLo
                case 7: ReadBGHi(); break; // fetch BGHi
                }
                BuildBG();
            }
            else if (ix == 257) { // Next line
                HReset();
                VInc();
            }
            else if (ix < 280) { // Inactive
            }
            else if (ix < 305) { // Inactive but prepare v, t
                VReset();
            }
            else if (ix < 321) { // Inactive
            }
            else if (ix < 337) {  // Prefetch next line
                switch (ix % 8) {
                case 0: HInc(); break; // inc hori(v)
                case 1: ReadNT(); break; // fetch NT
                case 3: ReadAT(); break; // fetch AT
                case 5: ReadBGLo(); break; // fetch BGLo
                case 7: ReadBGHi(); break; // fetch BGHi
                }
                BuildBG();
            }
            else { // Dummy NT fetches
                if (ix % 2 == 1) ReadNT();
            }
        }

        ++Ticks;
        if (Ticks == VIDEO_SIZE) {
            ++Frame;
            if ((ShowBackground || ShowSprite)
                && (Frame % 2 == 0)) Ticks = 1;
            else Ticks = 0;
        }
    }

    Word aNT, aAT;
    Byte bNT, bAT, bBGLo, bBGHi;
    Byte bBG;
    Word patternLo, patternHi, attrLo, attrHi;
public:
    static constexpr char * Id = "2C02";
    static constexpr char * Name = "Ricoh RP2C02";

    static constexpr size_t VIDEO_WIDTH = 341;
    static constexpr size_t VIDEO_HEIGHT = 262;
    static constexpr size_t VIDEO_SIZE = VIDEO_WIDTH * VIDEO_HEIGHT;

    static constexpr size_t FRAME_WIDTH = 256;
    static constexpr size_t FRAME_HEIGHT = 240;

    static constexpr size_t VBL_START = 241 * 341 + 1;
    static constexpr size_t VBL_STOP = 261 * 341 + 1;

    MemoryMap * Map;

    explicit Ricoh_RP2C02()
        : Ticks(0), Frame(0),
        VBlank(false),
        ShowBackground(false), ShowSprite(false),
        VramIncrement(1),
        BackgroundTable(0x0000)
    {}
};

#endif /* RICOH_RC2C02_H_ */
