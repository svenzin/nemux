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
    Ppu ppu;
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
}
