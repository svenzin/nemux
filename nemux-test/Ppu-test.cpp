/*
* Ppu-test.cpp
*
*  Created on: 03 Apr 2017
*      Author: scorder
*/

#include "gtest/gtest.h"

#include "Ppu.h"

using namespace std;

////////////////////////////////////////////////////////////////

struct PpuTest : public ::testing::Test {
    MemoryBlock<0x3000> ppumap;
    Ppu ppu;
    
    PpuTest() : ppu(&ppumap) {}
};

TEST_F(PpuTest, WriteCtrl1_NameTable) {
    ppu.WriteControl1(0x00);
    EXPECT_EQ(Word(0x2000), ppu.NameTable);
    
    ppu.WriteControl1(0x01);
    EXPECT_EQ(Word(0x2400), ppu.NameTable);
    
    ppu.WriteControl1(0x02);
    EXPECT_EQ(Word(0x2800), ppu.NameTable);
    
    ppu.WriteControl1(0x03);
    EXPECT_EQ(Word(0x2C00), ppu.NameTable);
}

TEST_F(PpuTest, WriteCtrl1_Increment) {
    ppu.WriteControl1(0x00);
    EXPECT_EQ(Byte(1), ppu.AddressIncrement);

    ppu.WriteControl1(0x04);
    EXPECT_EQ(Byte(32), ppu.AddressIncrement);
}

TEST_F(PpuTest, WriteCtrl1_SpriteTable) {
    ppu.WriteControl1(0x00);
    EXPECT_EQ(Word(0x0000), ppu.SpriteTable);

    ppu.WriteControl1(0x08);
    EXPECT_EQ(Word(0x1000), ppu.SpriteTable);
}

TEST_F(PpuTest, WriteCtrl1_BackgroundTable) {
    ppu.WriteControl1(0x00);
    EXPECT_EQ(Word(0x0000), ppu.BackgroundTable);

    ppu.WriteControl1(0x10);
    EXPECT_EQ(Word(0x1000), ppu.BackgroundTable);
}

TEST_F(PpuTest, WriteCtrl1_SpriteSize) {
    ppu.WriteControl1(0x00);
    EXPECT_EQ(8, ppu.SpriteHeight);

    ppu.WriteControl1(0x20);
    EXPECT_EQ(16, ppu.SpriteHeight);
}

TEST_F(PpuTest, WriteCtrl1_NMIOnVBlank) {
    ppu.WriteControl1(0x00);
    EXPECT_EQ(Flag(0), ppu.NMIOnVBlank);

    ppu.WriteControl1(0x80);
    EXPECT_EQ(Flag(1), ppu.NMIOnVBlank);
}

TEST_F(PpuTest, WriteCtrl2_Colour) {
    ppu.WriteControl2(0x00);
    EXPECT_EQ(true, ppu.IsColour);

    ppu.WriteControl2(0x01);
    EXPECT_EQ(false, ppu.IsColour);
}

TEST_F(PpuTest, WriteCtrl2_ClipBackground) {
    ppu.WriteControl2(0x00);
    EXPECT_EQ(true, ppu.ClipBackground);

    ppu.WriteControl2(0x02);
    EXPECT_EQ(false, ppu.ClipBackground);
}

TEST_F(PpuTest, WriteCtrl2_ClipSprite) {
    ppu.WriteControl2(0x00);
    EXPECT_EQ(true, ppu.ClipSprite);

    ppu.WriteControl2(0x04);
    EXPECT_EQ(false, ppu.ClipSprite);
}

TEST_F(PpuTest, WriteCtrl2_ShowBackground) {
    ppu.WriteControl2(0x00);
    EXPECT_EQ(false, ppu.ShowBackground);

    ppu.WriteControl2(0x08);
    EXPECT_EQ(true, ppu.ShowBackground);
}

TEST_F(PpuTest, WriteCtrl2_ShowSprite) {
    ppu.WriteControl2(0x00);
    EXPECT_EQ(false, ppu.ShowSprite);

    ppu.WriteControl2(0x10);
    EXPECT_EQ(true, ppu.ShowSprite);
}

TEST_F(PpuTest, WriteCtrl2_Intensity) {
    for (Byte b = Byte(0x00); b <= Byte(0x07); ++b) {
        ppu.WriteControl2(b << 5);
        EXPECT_EQ(b, ppu.ColourIntensity);
    }
}

TEST_F(PpuTest, ReadStatus_IgnoreVramWrites) {
    ppu.IgnoreVramWrites = false;
    EXPECT_EQ(0x00, ppu.ReadStatus() & 0x10);
    ppu.IgnoreVramWrites = true;
    EXPECT_EQ(0x10, ppu.ReadStatus() & 0x10);
}

TEST_F(PpuTest, ReadStatus_SpriteOverflow) {
    ppu.SpriteOverflow = false;
    EXPECT_EQ(0x00, ppu.ReadStatus() & 0x20);
    ppu.SpriteOverflow = true;
    EXPECT_EQ(0x20, ppu.ReadStatus() & 0x20);
}

TEST_F(PpuTest, ReadStatus_SpriteZeroHit) {
    ppu.SpriteZeroHit = false;
    EXPECT_EQ(0x00, ppu.ReadStatus() & 0x40);
    ppu.SpriteZeroHit = true;
    EXPECT_EQ(0x40, ppu.ReadStatus() & 0x40);
}

TEST_F(PpuTest, ReadStatus_VBlank) {
    ppu.VBlank = false;
    EXPECT_EQ(0x00, ppu.ReadStatus() & 0x80);
    ppu.VBlank = true;
    EXPECT_EQ(0x80, ppu.ReadStatus() & 0x80);
}

TEST_F(PpuTest, ReadStatus_VBlankClearedAfterRead) {
    ppu.VBlank = true;
    EXPECT_EQ(0x80, ppu.ReadStatus() & 0x80);
    EXPECT_EQ(0x00, ppu.ReadStatus() & 0x80);
}

TEST_F(PpuTest, PowerUpState) {
    // Ctrl1
    EXPECT_EQ(0x2000, ppu.NameTable);
    EXPECT_EQ(0x0001, ppu.AddressIncrement);
    EXPECT_EQ(0x0000, ppu.SpriteTable);
    EXPECT_EQ(0x0000, ppu.BackgroundTable);
    EXPECT_EQ(8, ppu.SpriteHeight);
    EXPECT_EQ(Flag(0), ppu.NMIOnVBlank);
    
    // Ctrl2
    EXPECT_EQ(true, ppu.IsColour);
    EXPECT_EQ(true, ppu.ClipBackground);
    EXPECT_EQ(true, ppu.ClipSprite);
    EXPECT_EQ(false, ppu.ShowBackground);
    EXPECT_EQ(false, ppu.ShowSprite);
    EXPECT_EQ(0, ppu.ColourIntensity);
    
    // Status
    EXPECT_EQ(true, ppu.SpriteOverflow);
    EXPECT_EQ(false, ppu.SpriteZeroHit);
    EXPECT_EQ(true, ppu.VBlank);

    // OAM address
    EXPECT_EQ(0x00, ppu.OAMAddress);

    // Scroll
    EXPECT_EQ(0x00, ppu.ScrollX);
    EXPECT_EQ(0x00, ppu.ScrollY);

    // Address
    EXPECT_EQ(0x0000, ppu.Address);
    EXPECT_EQ(0x00, ppu.ReadData());

    // NMI line
    EXPECT_EQ(false, ppu.NMIActive);
}

TEST_F(PpuTest, WriteOAMAdress) {
    ppu.WriteOAMAddress(0x00);
    EXPECT_EQ(0x00, ppu.OAMAddress);

    ppu.WriteOAMAddress(0xA5);
    EXPECT_EQ(0xA5, ppu.OAMAddress);
}

TEST_F(PpuTest, ReadWriteOAM_SingleData) {
    ppu.WriteOAMAddress(0xA5);
    ppu.WriteOAMData(0xBE);
    
    ppu.WriteOAMAddress(0xA5);
    EXPECT_EQ(0xBE, ppu.ReadOAMData());
}

TEST_F(PpuTest, ReadWriteOAM_IncreaseAddressOnWrite) {
    ppu.WriteOAMAddress(0xA5);
    ppu.WriteOAMData(0xBE);
    ppu.WriteOAMData(0xEF);

    ppu.WriteOAMAddress(0xA5);
    EXPECT_EQ(0xBE, ppu.ReadOAMData());
    ppu.WriteOAMAddress(0xA6);
    EXPECT_EQ(0xEF, ppu.ReadOAMData());
}

TEST_F(PpuTest, ReadWriteOAM_DontIncreaseAddressOnRead) {
    ppu.WriteOAMAddress(0xA5);
    ppu.WriteOAMData(0xBE);
    ppu.WriteOAMData(0xEF);

    ppu.WriteOAMAddress(0xA5);
    EXPECT_EQ(0xBE, ppu.ReadOAMData());
    EXPECT_EQ(0xBE, ppu.ReadOAMData());
}

TEST_F(PpuTest, ReadWriteOAM_WrapAroundOnWrite) {
    ppu.WriteOAMAddress(0xFF);
    ppu.WriteOAMData(0xBE);
    ppu.WriteOAMData(0xEF);

    ppu.WriteOAMAddress(0xFF);
    EXPECT_EQ(0xBE, ppu.ReadOAMData());
    ppu.WriteOAMAddress(0x00);
    EXPECT_EQ(0xEF, ppu.ReadOAMData());
}

TEST_F(PpuTest, WriteScroll_XandY) {
    ppu.WriteScroll(0xAB);
    EXPECT_EQ(0xAB, ppu.ScrollX);

    ppu.WriteScroll(0xCD);
    EXPECT_EQ(0xCD, ppu.ScrollY);
}

TEST_F(PpuTest, WriteScroll_FlipFlop) {
    ppu.WriteScroll(0xAB);
    ppu.WriteScroll(0xCD);
    EXPECT_EQ(0xAB, ppu.ScrollX);
    EXPECT_EQ(0xCD, ppu.ScrollY);

    ppu.WriteScroll(0x21);
    ppu.WriteScroll(0x84);
    EXPECT_EQ(0x21, ppu.ScrollX);
    EXPECT_EQ(0x84, ppu.ScrollY);
}

TEST_F(PpuTest, WriteScroll_ResetLatch) {
    ppu.WriteScroll(0xAB);
    ppu.WriteScroll(0xCD);
    EXPECT_EQ(0xAB, ppu.ScrollX);
    EXPECT_EQ(0xCD, ppu.ScrollY);

    ppu.WriteScroll(0x21);
    ppu.ReadStatus();
    ppu.WriteScroll(0x84);
    EXPECT_EQ(0x84, ppu.ScrollX);
    EXPECT_EQ(0xCD, ppu.ScrollY);
}

TEST_F(PpuTest, WriteAddress_Address) {
    ppu.WriteAddress(0x12);
    ppu.WriteAddress(0x34);
    EXPECT_EQ(0x1234, ppu.Address);
}

TEST_F(PpuTest, WriteAddress_FlipFlop) {
    ppu.WriteAddress(0x12);
    ppu.WriteAddress(0x34);
    EXPECT_EQ(0x1234, ppu.Address);

    ppu.WriteAddress(0x01);
    ppu.WriteAddress(0x23);
    EXPECT_EQ(0x0123, ppu.Address);
}

TEST_F(PpuTest, WriteAddress_ResetLatch) {
    ppu.WriteAddress(0x12);
    ppu.WriteAddress(0x34);
    EXPECT_EQ(0x1234, ppu.Address);

    ppu.WriteAddress(0x01);
    ppu.ReadStatus();
    ppu.WriteAddress(0x23);
    EXPECT_EQ(0x2334, ppu.Address);
}

TEST_F(PpuTest, WriteAddress_Mirroring) {
    ppu.WriteAddress(0x12);
    ppu.WriteAddress(0x34);
    EXPECT_EQ(0x1234, ppu.Address);

    ppu.WriteAddress(0x52);
    ppu.WriteAddress(0x34);
    EXPECT_EQ(0x1234, ppu.Address);

    ppu.WriteAddress(0x92);
    ppu.WriteAddress(0x34);
    EXPECT_EQ(0x1234, ppu.Address);

    ppu.WriteAddress(0xD2);
    ppu.WriteAddress(0x34);
    EXPECT_EQ(0x1234, ppu.Address);
}

TEST_F(PpuTest, ReadData_IncrementAddress) {
    ppu.WriteControl1(0x00);
    ppu.WriteAddress(0x12);
    ppu.WriteAddress(0x34);
    ppu.ReadData();
    EXPECT_EQ(0x1234 + 1, ppu.Address);

    ppu.WriteControl1(0x04);
    ppu.WriteAddress(0x12);
    ppu.WriteAddress(0x34);
    ppu.ReadData();
    EXPECT_EQ(0x1234 + 32, ppu.Address);
}

TEST_F(PpuTest, ReadData_IncrementAddress_Mirroring) {
    ppu.WriteControl1(0x00);
    ppu.WriteAddress(0x3F);
    ppu.WriteAddress(0xFF);
    ppu.ReadData();
    EXPECT_EQ(0x0000, ppu.Address);

    ppu.WriteControl1(0x04);
    ppu.WriteAddress(0x3F);
    ppu.WriteAddress(0xFF);
    ppu.ReadData();
    EXPECT_EQ(0x001F, ppu.Address);
}

TEST_F(PpuTest, WriteData_IncrementAddress) {
    ppu.WriteControl1(0x00);
    ppu.WriteAddress(0x12);
    ppu.WriteAddress(0x34);
    ppu.WriteData(0x00);
    EXPECT_EQ(0x1234 + 1, ppu.Address);

    ppu.WriteControl1(0x04);
    ppu.WriteAddress(0x12);
    ppu.WriteAddress(0x34);
    ppu.WriteData(0x00);
    EXPECT_EQ(0x1234 + 32, ppu.Address);
}

TEST_F(PpuTest, WriteData_IncrementAddress_Mirroring) {
    ppu.WriteControl1(0x00);
    ppu.WriteAddress(0x3F);
    ppu.WriteAddress(0xFF);
    ppu.WriteData(0x00);
    EXPECT_EQ(0x0000, ppu.Address);

    ppu.WriteControl1(0x04);
    ppu.WriteAddress(0x3F);
    ppu.WriteAddress(0xFF);
    ppu.WriteData(0x00);
    EXPECT_EQ(0x001F, ppu.Address);
}

TEST_F(PpuTest, ReadData_ReadBuffer) {
    ppu.WriteAddress(0x10);
    ppu.WriteAddress(0x00);
    ppu.WriteData(0xA5);
    ppu.WriteData(0xAA);

    // At power up, the buffer is 0x00 (already tested)
    // When reading, the buffer is returned
    // and re-filled with the data at the current address.
    // Then the address is increased
    ppu.WriteAddress(0x10);
    ppu.WriteAddress(0x00);
    ppu.ReadData(); // Expected to be 0x00
    EXPECT_EQ(0x1001, ppu.Address);
    EXPECT_EQ(0xA5, ppu.ReadData());
    EXPECT_EQ(0x1002, ppu.Address);
    EXPECT_EQ(0xAA, ppu.ReadData());
    EXPECT_EQ(0x1003, ppu.Address);
}

TEST_F(PpuTest, ReadData_ReadBufferOnHighAddress) {
    ppu.WriteAddress(0x2F);
    ppu.WriteAddress(0x00);
    ppu.WriteData(0xBE);
    
    ppu.WriteAddress(0x3F);
    ppu.WriteAddress(0x00);
    ppu.WriteData(0x15);
    ppu.WriteData(0x1A);

    // At power up, the buffer is 0x00 (already tested)
    // When reading in addresses > 0x3EFF, the data in memory is returned
    ppu.WriteAddress(0x3F);
    ppu.WriteAddress(0x00);
    EXPECT_EQ(0x15, ppu.ReadData());
    EXPECT_EQ(0x3F01, ppu.Address);
    EXPECT_EQ(0x1A, ppu.ReadData());
    EXPECT_EQ(0x3F02, ppu.Address);

    // The buffer is still filled based on mirrored-down address
    ppu.WriteAddress(0x3F);
    ppu.WriteAddress(0x00);
    EXPECT_EQ(0x15, ppu.ReadData());
    EXPECT_EQ(0x3F01, ppu.Address);
    ppu.WriteAddress(0x00);
    ppu.WriteAddress(0x00);
    EXPECT_EQ(0xBE, ppu.ReadData());
}

TEST_F(PpuTest, NMI_DetailedActivation) {
    ppu.WriteControl1(Mask<7>(true));
    EXPECT_EQ(true, ppu.NMIOnVBlank);

    int i;
    EXPECT_EQ(false, ppu.NMIActive); // NMI inactive at start
    
    // Frame 0, pixels (0, 0) to (341, 240)
    for (i = 0; i <= 82180; ++i) {
        ppu.Tick();
        EXPECT_EQ(false, ppu.NMIActive) << i;
    }

    // Frame 0, pixels (0, 241) to (341, 260)
    // First tick of scanline 241 has not triggered NMI yet
    // Second tick of scanline 241 triggers NMI
    ppu.Tick();
    EXPECT_EQ(false, ppu.NMIActive) << i;
    for (i = 82182; i <= 89000; i++) {
        ppu.Tick();
        EXPECT_EQ(true, ppu.NMIActive) << i;
    }

    // Frame 0, pixels (0, 261) to (341, 261)
    for (i = 89001; i <= 89341; ++i) {
        ppu.Tick();
        EXPECT_EQ(false, ppu.NMIActive) << i;
    }

    // Frame 1, pixels (0, 0) to (341, 240)
    for (i = 89342; i <= 171522; ++i) {
        ppu.Tick();
        EXPECT_EQ(false, ppu.NMIActive) << i;
    }

    // Frame 1, pixels (0, 241) to (341, 260)
    ppu.Tick();
    EXPECT_EQ(false, ppu.NMIActive) << i;
    for (i = 171524; i <= 178342; i++) {
        ppu.Tick();
        EXPECT_EQ(true, ppu.NMIActive) << i;
    }

    // Frame 1, pixels (0, 261) to (341, 261)
    for (i = 178343; i <= 178683; ++i) {
        ppu.Tick();
        EXPECT_EQ(false, ppu.NMIActive) << i;
    }
}

TEST_F(PpuTest, NMI_Disabled) {
    EXPECT_EQ(false, ppu.NMIOnVBlank);
    for (int i = 0; i < VIDEO_SIZE; ++i) {
        EXPECT_EQ(false, ppu.NMIActive);
        ppu.Tick();
    }
}

TEST_F(PpuTest, NMI_MultipleTriggers) {
    ppu.WriteControl1(Mask<7>(true));
    EXPECT_EQ(true, ppu.NMIOnVBlank);
    
    while (!ppu.NMIActive)
        ppu.Tick();
    
    EXPECT_EQ(true, ppu.NMIActive);

    ppu.WriteControl1(0);
    ppu.Tick();
    EXPECT_EQ(false, ppu.NMIActive);

    ppu.WriteControl1(Mask<7>(true));
    ppu.Tick();
    EXPECT_EQ(true, ppu.NMIActive);

    ppu.Tick();
    EXPECT_EQ(true, ppu.NMIActive);
}

TEST_F(PpuTest, NMI_StopOnStatusRead) {
    ppu.WriteControl1(Mask<7>(true));
    EXPECT_EQ(true, ppu.NMIOnVBlank);

    while (!ppu.NMIActive)
        ppu.Tick();

    EXPECT_EQ(true, ppu.NMIActive);

    ppu.ReadStatus();
    ppu.Tick();
    EXPECT_EQ(false, ppu.NMIActive);
}

TEST_F(PpuTest, NMI_MultipleTriggersStopOnStatusRead) {
    ppu.WriteControl1(Mask<7>(true));
    EXPECT_EQ(true, ppu.NMIOnVBlank);

    while (!ppu.NMIActive)
        ppu.Tick();

    EXPECT_EQ(true, ppu.NMIActive);

    ppu.WriteControl1(0);
    ppu.Tick();
    EXPECT_EQ(false, ppu.NMIActive);

    ppu.WriteControl1(Mask<7>(true));
    ppu.Tick();
    EXPECT_EQ(true, ppu.NMIActive);

    ppu.ReadStatus();
    ppu.Tick();
    ppu.WriteControl1(0);
    ppu.Tick();
    ppu.WriteControl1(Mask<7>(true));
    ppu.Tick();
    EXPECT_EQ(false, ppu.NMIActive);
}

TEST_F(PpuTest, PpuFrameTime_DisabledRendering) {
    // 1 pixel per PPU cycle
    const auto T0 = VIDEO_SIZE;

    // Disabled rendering
    EXPECT_EQ(false, ppu.ShowBackground);
    EXPECT_EQ(false, ppu.ShowSprite);

    // Even frame (Frame 0)
    for (int i = 0; i < T0; ++i) {
        EXPECT_EQ(0, ppu.FrameCount);
        EXPECT_EQ(i, ppu.FrameTicks);
        ppu.Tick();
    }

    // Odd frame (Frame 1)
    for (int i = 0; i < T0; ++i) {
        EXPECT_EQ(1, ppu.FrameCount);
        EXPECT_EQ(i, ppu.FrameTicks);
        ppu.Tick();
    }

    EXPECT_EQ(2, ppu.FrameCount);
    EXPECT_EQ(0, ppu.FrameTicks);
}

TEST_F(PpuTest, PpuFrameTime_EnabledRendering) {
    // 1 pixel per PPU cycle
    const auto T0 = VIDEO_SIZE;

    // Disabled rendering*
    ppu.WriteControl2(Mask<3>(true) | Mask<4>(true));
    EXPECT_EQ(true, ppu.ShowBackground);
    EXPECT_EQ(true, ppu.ShowSprite);

    // Even frame (Frame 0)
    for (int i = 0; i < T0; ++i) {
        EXPECT_EQ(0, ppu.FrameCount);
        EXPECT_EQ(i, ppu.FrameTicks);
        ppu.Tick();
    }

    // Odd frame (Frame 1)
    for (int i = 0; i < T0 - 1; ++i) {
        EXPECT_EQ(1, ppu.FrameCount);
        EXPECT_EQ(i, ppu.FrameTicks);
        ppu.Tick();
    }

    EXPECT_EQ(2, ppu.FrameCount);
    EXPECT_EQ(0, ppu.FrameTicks);
}

TEST_F(PpuTest, Sprite0Hit_DetectHit) {
    const auto foreground = 0x05;
    const auto background = 0x06;
    // Except on last pixel (255)
    ppu.ShowBackground = true;
    ppu.ShowSprite = true;
    ppu.ClipBackground = false;
    ppu.ClipSprite = false;
    for (size_t i = 0; i < 255; i++) {
        EXPECT_EQ(true, ppu.SpriteHit(foreground, background, i));
    }
    EXPECT_EQ(false, ppu.SpriteHit(foreground, background, 255));
}

TEST_F(PpuTest, Sprite0Hit_NoHitConditions) {
    const auto transparent = 0x04;
    const auto solid = 0x05;
    
    ppu.ShowBackground = true;
    ppu.ShowSprite = true;
    EXPECT_EQ(false, ppu.SpriteHit(transparent, solid, 0));
    EXPECT_EQ(false, ppu.SpriteHit(solid, transparent, 0));
    EXPECT_EQ(false, ppu.SpriteHit(transparent, transparent, 0));

    ppu.ShowBackground = true;
    ppu.ShowSprite = false;
    EXPECT_EQ(false, ppu.SpriteHit(solid, solid, 0));

    ppu.ShowBackground = false;
    ppu.ShowSprite = true;
    EXPECT_EQ(false, ppu.SpriteHit(solid, solid, 0));

    ppu.ShowBackground = false;
    ppu.ShowSprite = false;
    EXPECT_EQ(false, ppu.SpriteHit(solid, solid, 0));

    ppu.ClipBackground = true;
    ppu.ClipSprite = false;
    for (size_t i = 0; i < 8; i++) {
        EXPECT_EQ(false, ppu.SpriteHit(solid, solid, i));
    }

    ppu.ClipBackground = false;
    ppu.ClipSprite = true;
    for (size_t i = 0; i < 8; i++) {
        EXPECT_EQ(false, ppu.SpriteHit(solid, solid, i));
    }

    ppu.ClipBackground = true;
    ppu.ClipSprite = true;
    for (size_t i = 0; i < 8; i++) {
        EXPECT_EQ(false, ppu.SpriteHit(solid, solid, i));
    }
}

TEST_F(PpuTest, Sprite0Hit_Reset) {
    ppu.SpriteZeroHit = true;

    // Reset at first pixel of pre-render scanline (261)
    for (size_t i = 0; i < 261 * 341; i++) {
        ppu.Tick();
        EXPECT_EQ(true, ppu.SpriteZeroHit);
    }
    ppu.Tick();
    EXPECT_EQ(false, ppu.SpriteZeroHit);
}

TEST_F(PpuTest, SpritePriorityMultiplexer) {
    const auto transparent = 0x04;
    const auto foreground = 0x05;
    const auto background = 0x06;
    EXPECT_EQ(0, ppu.SpriteMultiplexer(transparent, transparent, true));
    EXPECT_EQ(0, ppu.SpriteMultiplexer(transparent, transparent, false));
    EXPECT_EQ(foreground, ppu.SpriteMultiplexer(transparent, foreground, true));
    EXPECT_EQ(foreground, ppu.SpriteMultiplexer(transparent, foreground, false));
    EXPECT_EQ(background, ppu.SpriteMultiplexer(background, transparent, true));
    EXPECT_EQ(background, ppu.SpriteMultiplexer(background, transparent, false));
    EXPECT_EQ(background, ppu.SpriteMultiplexer(background, foreground, true));
    EXPECT_EQ(foreground, ppu.SpriteMultiplexer(background, foreground, false));
}
