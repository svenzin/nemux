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
