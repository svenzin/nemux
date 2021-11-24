#ifndef RICOH_RC2C02_H_
#define RICOH_RC2C02_H_

#include "Types.h"
#include "MemoryMap.h"
#include "CircularQueue.h"

#include <string>
#include <vector>

#include <iostream>

class Ricoh_RP2C02 {
public:
    Word v;
    Word t;
    Byte x;
    Flag w;

    std::array<Byte, 8> OAM2_SpriteId;
    std::array<Byte, 64> OAM2;

    static void SetCoarseX  (Word & w, const Byte & b) { w = (w & ~0x001F) | (b & 0x1F); }
    static void SetCoarseY  (Word & w, const Byte & b) { w = (w & ~0x03E0) | ((b & 0x1F) << 5); }
    static void SetFineY    (Word & w, const Byte & b) { w = (w & ~0x7000) | ((b & 0x07) << 12); }
    static void SetNametable(Word & w, const Byte & b) { w = (w & ~0x0C00) | ((b & 0x03) << 10); }
    static void SetNTX      (Word & w, const Byte & b) { w = (w & ~0x0400) | ((b & 0x01) << 10); }
    static void SetNTY      (Word & w, const Byte & b) { w = (w & ~0x0800) | ((b & 0x01) << 11); }

    static Byte GetCoarseX  (const Word & w) { return w & 0x001F; }
    static Byte GetCoarseY  (const Word & w) { return (w & 0x03E0) >> 5; }
    static Byte GetFineY    (const Word & w) { return (w & 0x7000) >> 12; }
    static Byte GetNametable(const Word & w) { return (w & 0x0C00) >> 10; }
    static Byte GetNTX      (const Word & w) { return (w & 0x0400) >> 10; }
    static Byte GetNTY      (const Word & w) { return (w & 0x0800) >> 11; }

    static Byte Reverse(const Byte & b) {
        return (b * 0x0202020202ULL & 0x010884422010ULL) % 1023;
    }

    Byte Backdrop() const {
        if (RenderingEnabled()) return 0;
        if (v < 0x3F00) return 0;
        return (v & 0x00FF);
    }

    bool RenderingEnabled() const { return ShowBackground || ShowSprite; }

    Byte GetPixel(const Byte & bg, const Byte & fg) {
        if ((bg & 0x03) != 0) {
            if ((fg & 0x03) != 0) {
                if (BGPriority) return bg;
                return fg;
            }
            return bg;
        }
        if ((fg & 0x03) != 0) return fg;
        return Backdrop();
    }
    
    void Read2002() {
        w = 0;
    }
    void Write2005(const Byte & value) {
        if (w == 0) {
            SetCoarseX(t, value >> 3);
            x = value & 0x07;
            w = 1;
        }
        else {
            SetFineY(t, value);
            SetCoarseY(t, value >> 3);
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
        if (RenderingEnabled()) {
            if (GetCoarseX(v) == 31) {
                SetCoarseX(v, 0);
                SetNTX(v, GetNTX(v) + 1);
            }
            else {
                SetCoarseX(v, GetCoarseX(v) + 1);
            }
        }
    }
    void HReset() {
        if (RenderingEnabled()) {
            SetCoarseX(v, GetCoarseX(t));
            SetNTX(v, GetNTX(t));
        }
    }
    void VInc() {
        if (RenderingEnabled()) {
            if (GetFineY(v) == 7) {
                SetFineY(v, 0);
                if (GetCoarseY(v) == 29) {
                    SetCoarseY(v, 0);
                    SetNTY(v, GetNTY(v) + 1);
                }
                else if (GetCoarseY(v) == 31) {
                    SetCoarseY(v, 0);
                }
                else {
                    SetCoarseY(v, GetCoarseY(v) + 1);
                }
            }
            else {
                SetFineY(v, GetFineY(v) + 1);
            }
        }
    }
    void VReset() {
        if (RenderingEnabled()) {
            SetCoarseY(v, GetCoarseY(t));
            SetFineY(v, GetFineY(t));
            SetNTY(v, GetNTY(t));
        }
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
        bBGLo = Map->GetByteAt(BackgroundTable + 16 * bNT + GetFineY(v));
    }
    void ReadBGHi() {
        bBGHi = Map->GetByteAt(BackgroundTable + 16 * bNT + GetFineY(v) + 8);
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
        bBG = 0;
        if (ShowBackground) {
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
    }
    void BuildSprite() {
        bSprite = 0;
        if (ShowSprite) {
            for (int n = 7; n >= 0; --n) {
                if (Sprites[n].X > 0) {
                    --Sprites[n].X;
                }
                else if (Sprites[n].Width > 0) {
                    if ((Bit<0>(Sprites[n].Lo) != 0) || (Bit<0>(Sprites[n].Hi) != 0)) {
                        bSprite = (Sprites[n].Lo & 0x01)
                            | ((Sprites[n].Hi & 0x01) << 1)
                            | ((Sprites[n].Attributes & 0x03) << 2)
                            | SPRITE_PALETTE;
                        BGPriority = IsBitSet<5>(Sprites[n].Attributes);
                        if (Sprites[n].Id == 0) SpriteZeroHit = SpriteZeroHit || SpriteHit(bBG, bSprite, ix);
                    }
                    Sprites[n].Lo >>= 1;
                    Sprites[n].Hi >>= 1;
                    --Sprites[n].Width;
                }
            }
        }
    }
    bool SpriteHit(const Byte & background, const Byte & foreground, const size_t & x) const {
        if (x == 255) return false;
        if (!ShowBackground || !ShowSprite) return false;
        //if ((ClipBackground || ClipSprite) && (x < 8)) return false;

        const bool hit = (((background & 0x03) > 0) && ((foreground & 0x03) > 0));
        return hit;
    }

    // Scanline patterns
    void SL_PrepareBG() {
        if      (ix == 0)  {} // Idle
        else if (ix < 256) {  // Active frame
            switch (ix % 8) {
            //case 0: LatchBG(); break; // inc hori(v)
            case 1: ReadNT(); break; // fetch NT
            case 3: ReadAT(); break; // fetch AT
            case 5: ReadBGLo(); break; // fetch BGLo
            case 7: ReadBGHi(); break; // fetch BGHi
            }
        }
        else if (ix < 321) {} // Inactive
        else if (ix < 337) {  // Prefetch next line
            switch (ix % 8) {
            //case 0: LatchBG(); break; // inc hori(v)
            case 1: ReadNT(); break; // fetch NT
            case 3: ReadAT(); break; // fetch AT
            case 5: ReadBGLo(); break; // fetch BGLo
            case 7: ReadBGHi(); break; // fetch BGHi
            }
        }
        else { // Dummy NT fetches
            if (ix % 2 == 1) ReadNT();
        }
    }

    void SL_TickBG() {
        if (ix == 0) {} // Idle
        else if (ix < 256) { // Active frame
            if ((ix % 8) == 0) {
                LatchBG();
                HInc();
            }
        }
        else if (ix == 256) { // Next line
            LatchBG();
            HInc();
            VInc();
        }
        else if (ix == 257) { // Next line
            HReset();
        }
        else if (ix < 321) {} // Inactive
        else if (ix < 337) {  // Prefetch next line
            if ((ix % 8) == 0) {
                LatchBG();
                HInc();
            }
        }
    }

    void SL_BuildBG() {
        if      (ix == 0)  {}         // Idle
        else if (ix < 257) BuildBG(); // Active frame
        else if (ix < 321) {}         // Inactive
        else if (ix < 337) BuildBG(); // Prefetch next line
    }

    void SL_PrepareSprite() {
        if (ix == 0) {} // Idle
        else if (ix <= 64) OAM2[ix - 1] = 0xFF; // OAM2 clear
        else if (ix <= 256) { // Sprite evaluation
            if (ix == 256) {
                iSprite = 0;
                for (int n = 0; n < 64; ++n) {
                    OAM2_SpriteId[iSprite] = n;
                    OAM2[4 * iSprite] = (*pOAM)[4 * n];
                    if ((*pOAM)[4 * n] <= iy && iy < (*pOAM)[4 * n] + SpriteHeight) {
                        OAM2[4 * iSprite + 1] = (*pOAM)[4 * n + 1];
                        OAM2[4 * iSprite + 2] = (*pOAM)[4 * n + 2];
                        OAM2[4 * iSprite + 3] = (*pOAM)[4 * n + 3];
                        ++iSprite;
                        if (iSprite == 8) break;
                    }
                }
                iSprite = 0;
            }
        }
        else if (ix <= 320) { // Sprite loading
            switch (ix % 8) { // Starting at 1 because first cycle is 257
            case 1: Sprites[iSprite].Y = OAM2[4 * iSprite];     break; // Read Y, Fetch garbage NT
            case 2: Sprites[iSprite].Tile = OAM2[4 * iSprite + 1]; break; // Read tile number
            case 3: Sprites[iSprite].Attributes = OAM2[4 * iSprite + 2]; break; // Read attribute, Fetch garbage AT
            case 4: { // Read X
                Sprites[iSprite].X = OAM2[4 * iSprite + 3];
                Sprites[iSprite].Width = SPRITE_WIDTH;
                Sprites[iSprite].Id = OAM2_SpriteId[iSprite];
                if (SpriteHeight == 8) {
                    Sprites[iSprite].aLo = SpriteTable;
                }
                else {
                    Sprites[iSprite].aLo = 0x1000 * Bit<0>(Sprites[iSprite].Tile);
                    Sprites[iSprite].Tile &= 0xFE;
                }
                Sprites[iSprite].aLo += 16 * Sprites[iSprite].Tile;
                if (SpriteHeight == 16) {
                    if ((iy - Sprites[iSprite].Y) >= 8) {
                        Sprites[iSprite].aLo += 16;
                    }
                }
                Sprites[iSprite].aHi = Sprites[iSprite].aLo + 8;
                break;
            }
            case 5: { // Read X, Read Sprite Lo
                if (IsBitSet<7>(Sprites[iSprite].Attributes)) {
                    Sprites[iSprite].Lo = Map->GetByteAt(Sprites[iSprite].aLo + 7 - ((iy - Sprites[iSprite].Y) % 8));
                }
                else {
                    Sprites[iSprite].Lo = Map->GetByteAt(Sprites[iSprite].aLo + ((iy - Sprites[iSprite].Y) % 8));
                }
                if (IsBitClear<6>(Sprites[iSprite].Attributes)) {
                    Sprites[iSprite].Lo = Reverse(Sprites[iSprite].Lo);
                }
                break;
            }
            case 6: break; // Read X
            case 7: { // Read X, Read Sprite Hi
                if (IsBitSet<7>(Sprites[iSprite].Attributes)) {
                    Sprites[iSprite].Hi = Map->GetByteAt(Sprites[iSprite].aHi + 7 - ((iy - Sprites[iSprite].Y) % 8));
                }
                else {
                    Sprites[iSprite].Hi = Map->GetByteAt(Sprites[iSprite].aHi + ((iy - Sprites[iSprite].Y) % 8));
                }
                if (IsBitClear<6>(Sprites[iSprite].Attributes)) {
                    Sprites[iSprite].Hi = Reverse(Sprites[iSprite].Hi);
                }
                break;
            }
            case 0: ++iSprite; break; // Read X
            }
        }
    }

    // TODO: Fix active frame to 257 (see ninja gaiden)
    void SL_BuildSprite() {
        if      (ix == 0)  {}             // Idle
        else if (ix < 257) BuildSprite(); // Active frame
    }

    // $2000
    Byte VramIncrement;
    Word SpriteTable;
    Word BackgroundTable;
    Byte SpriteHeight;

    bool SpriteZeroHit;

    bool ShowBackground;
    bool ShowSprite;
    bool IsGreyscale;

    size_t Ticks;
    size_t Frame;
    bool VBlank;
    size_t ix=0, iy=0;
    void Tick() {
        /*const auto */ix = Ticks % VIDEO_WIDTH;
        /*const auto */iy = Ticks / VIDEO_WIDTH;

        if (Ticks == VBL_START) VBlank = true;
        if (Ticks == VBL_STOP) {
            VBlank = false;
            SpriteZeroHit = false;
        }

        bBG = bSprite = 0;

        ////////////////////////////////
        // Prepare BG
        if      (iy < 240) SL_PrepareBG(); // Active scanlines
        else if (iy < 261) {}              // Inactive scanlines
        else {                             // Prerender line
            SL_PrepareBG();
            if (280 <= ix && ix < 305) VReset(); // Inactive but prepare v, t
        }

        ////////////////////////////////
        // Display BG
        if      (iy < 240) SL_BuildBG(); // Active scanlines
        else if (iy < 261) {}            // Inactive scanlines
        else               SL_BuildBG(); // Prerender line

        ////////////////////////////////
        // Tick BG
        if      (iy < 240) SL_TickBG(); // Active scanlines
        else if (iy < 261) {}           // Inactive scanlines
        else               SL_TickBG(); // Prerender line

        ////////////////////////////////
        // Prepare Sprite
        if      (iy < 240) SL_PrepareSprite(); // Active scanlines
        else if (iy < 261) {}                  // Inactive scanlines
        else               SL_PrepareSprite(); // Prerender line

        ////////////////////////////////
        // Display Sprite
        if (iy < 240) SL_BuildSprite(); // Active scanlines


        pixel = GetPixel(bBG, bSprite);

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
    Byte pixel;

    size_t iSprite, iByte;
    Byte bSprite;
    bool BGPriority;
    struct SpriteUnit {
        Byte Id;
        Byte X, Y;
        Byte Width;
        Byte Tile;
        Byte Attributes;
        Word aLo, aHi;
        Byte Lo, Hi;
    };
    std::array<SpriteUnit, 8> Sprites;
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

    static constexpr Byte SPRITE_WIDTH = 8;
    static constexpr Byte SPRITE_PALETTE = 0x10;

    MemoryMap * Map;
    std::array<Byte, 0x0100> * pOAM;


private:
public:

    explicit Ricoh_RP2C02()
        : Ticks(0), Frame(0),
        VBlank(false),
        SpriteZeroHit(false),

        // $2000 Control
        // Nametable
        VramIncrement(1),
        SpriteTable(0x0000),
        BackgroundTable(0x0000),
        SpriteHeight(8),
        // NMIOnVBlank

        // $2001 Mask
        IsGreyscale(false),
        // ClipBG
        // ClipSprite
        ShowBackground(false),
        ShowSprite(false)
        // ColourEmphasis
    {}

    // Control register
    void Write2000(const Byte & value) {
        VramIncrement = IsBitSet<2>(value) ? 0x0020 : 0x0001;
        SpriteTable = IsBitSet<3>(value) ? 0x1000 : 0x0000;
        BackgroundTable = IsBitSet<4>(value) ? 0x1000 : 0x0000;
        SpriteHeight = IsBitSet<5>(value) ? 16 : 8;
        SetNametable(t, value);
    }
    
    // Mask register
    void Write2001(const Byte & value) {
        IsGreyscale = IsBitSet<0>(value);
        ShowBackground = IsBitSet<3>(value);
        ShowSprite = IsBitSet<4>(value);
    }
};

#endif /* RICOH_RC2C02_H_ */
