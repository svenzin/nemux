//#include "gtest/gtest.h"
//
//#include "Apu.h"
//
//using namespace std;
//
//////////////////////////////////////////////////////////////////
//
//struct ApuTest : public ::testing::Test {
//    MemoryBlock<0x3000> Apumap;
//    Apu Apu;
//
//    ApuTest() : Apu(&Apumap) {}
//};
//
//TEST_F(ApuTest, WriteCtrl1_NameTable) {
//    Apu.WriteControl1(0x00);
//    EXPECT_EQ(Word(0x2000), Apu.NameTable);
//
//    Apu.WriteControl1(0x01);
//    EXPECT_EQ(Word(0x2400), Apu.NameTable);
//
//    Apu.WriteControl1(0x02);
//    EXPECT_EQ(Word(0x2800), Apu.NameTable);
//
//    Apu.WriteControl1(0x03);
//    EXPECT_EQ(Word(0x2C00), Apu.NameTable);
//}
//
//TEST_F(ApuTest, WriteCtrl1_Increment) {
//    Apu.WriteControl1(0x00);
//    EXPECT_EQ(Byte(1), Apu.AddressIncrement);
//
//    Apu.WriteControl1(0x04);
//    EXPECT_EQ(Byte(32), Apu.AddressIncrement);
//}
//
//TEST_F(ApuTest, WriteCtrl1_SpriteTable) {
//    Apu.WriteControl1(0x00);
//    EXPECT_EQ(Word(0x0000), Apu.SpriteTable);
//
//    Apu.WriteControl1(0x08);
//    EXPECT_EQ(Word(0x1000), Apu.SpriteTable);
//}
//
//TEST_F(ApuTest, WriteCtrl1_BackgroundTable) {
//    Apu.WriteControl1(0x00);
//    EXPECT_EQ(Word(0x0000), Apu.BackgroundTable);
//
//    Apu.WriteControl1(0x10);
//    EXPECT_EQ(Word(0x1000), Apu.BackgroundTable);
//}
//
//TEST_F(ApuTest, WriteCtrl1_SpriteSize) {
//    Apu.WriteControl1(0x00);
//    EXPECT_EQ(8, Apu.SpriteHeight);
//
//    Apu.WriteControl1(0x20);
//    EXPECT_EQ(16, Apu.SpriteHeight);
//}
//
//TEST_F(ApuTest, WriteCtrl1_NMIOnVBlank) {
//    Apu.WriteControl1(0x00);
//    EXPECT_EQ(Flag(0), Apu.NMIOnVBlank);
//
//    Apu.WriteControl1(0x80);
//    EXPECT_EQ(Flag(1), Apu.NMIOnVBlank);
//}
//
//TEST_F(ApuTest, WriteCtrl2_Colour) {
//    Apu.WriteControl2(0x00);
//    EXPECT_EQ(true, Apu.IsColour);
//
//    Apu.WriteControl2(0x01);
//    EXPECT_EQ(false, Apu.IsColour);
//}
//
//TEST_F(ApuTest, WriteCtrl2_ClipBackground) {
//    Apu.WriteControl2(0x00);
//    EXPECT_EQ(true, Apu.ClipBackground);
//
//    Apu.WriteControl2(0x02);
//    EXPECT_EQ(false, Apu.ClipBackground);
//}
//
//TEST_F(ApuTest, WriteCtrl2_ClipSprite) {
//    Apu.WriteControl2(0x00);
//    EXPECT_EQ(true, Apu.ClipSprite);
//
//    Apu.WriteControl2(0x04);
//    EXPECT_EQ(false, Apu.ClipSprite);
//}
//
//TEST_F(ApuTest, WriteCtrl2_ShowBackground) {
//    Apu.WriteControl2(0x00);
//    EXPECT_EQ(false, Apu.ShowBackground);
//
//    Apu.WriteControl2(0x08);
//    EXPECT_EQ(true, Apu.ShowBackground);
//}
//
//TEST_F(ApuTest, WriteCtrl2_ShowSprite) {
//    Apu.WriteControl2(0x00);
//    EXPECT_EQ(false, Apu.ShowSprite);
//
//    Apu.WriteControl2(0x10);
//    EXPECT_EQ(true, Apu.ShowSprite);
//}
//
//TEST_F(ApuTest, WriteCtrl2_Intensity) {
//    for (Byte b = Byte(0x00); b <= Byte(0x07); ++b) {
//        Apu.WriteControl2(b << 5);
//        EXPECT_EQ(b, Apu.ColourIntensity);
//    }
//}
//
//TEST_F(ApuTest, ReadStatus_IgnoreVramWrites) {
//    Apu.IgnoreVramWrites = false;
//    EXPECT_EQ(0x00, Apu.ReadStatus() & 0x10);
//    Apu.IgnoreVramWrites = true;
//    EXPECT_EQ(0x10, Apu.ReadStatus() & 0x10);
//}
//
//TEST_F(ApuTest, ReadStatus_SpriteOverflow) {
//    Apu.SpriteOverflow = false;
//    EXPECT_EQ(0x00, Apu.ReadStatus() & 0x20);
//    Apu.SpriteOverflow = true;
//    EXPECT_EQ(0x20, Apu.ReadStatus() & 0x20);
//}
//
//TEST_F(ApuTest, ReadStatus_SpriteZeroHit) {
//    Apu.SpriteZeroHit = false;
//    EXPECT_EQ(0x00, Apu.ReadStatus() & 0x40);
//    Apu.SpriteZeroHit = true;
//    EXPECT_EQ(0x40, Apu.ReadStatus() & 0x40);
//}
//
//TEST_F(ApuTest, ReadStatus_VBlank) {
//    Apu.VBlank = false;
//    EXPECT_EQ(0x00, Apu.ReadStatus() & 0x80);
//    Apu.VBlank = true;
//    EXPECT_EQ(0x80, Apu.ReadStatus() & 0x80);
//}
//
//TEST_F(ApuTest, PowerUpState) {
//    // Ctrl1
//    EXPECT_EQ(0x2000, Apu.NameTable);
//    EXPECT_EQ(0x0001, Apu.AddressIncrement);
//    EXPECT_EQ(0x0000, Apu.SpriteTable);
//    EXPECT_EQ(0x0000, Apu.BackgroundTable);
//    EXPECT_EQ(8, Apu.SpriteHeight);
//    EXPECT_EQ(Flag(0), Apu.NMIOnVBlank);
//
//    // Ctrl2
//    EXPECT_EQ(true, Apu.IsColour);
//    EXPECT_EQ(true, Apu.ClipBackground);
//    EXPECT_EQ(true, Apu.ClipSprite);
//    EXPECT_EQ(false, Apu.ShowBackground);
//    EXPECT_EQ(false, Apu.ShowSprite);
//    EXPECT_EQ(0, Apu.ColourIntensity);
//
//    // Status
//    EXPECT_EQ(true, Apu.SpriteOverflow);
//    EXPECT_EQ(false, Apu.SpriteZeroHit);
//    EXPECT_EQ(true, Apu.VBlank);
//
//    // OAM address
//    EXPECT_EQ(0x00, Apu.OAMAddress);
//
//    // Scroll
//    EXPECT_EQ(0x00, Apu.ScrollX);
//    EXPECT_EQ(0x00, Apu.ScrollY);
//
//    // Address
//    EXPECT_EQ(0x0000, Apu.Address);
//    EXPECT_EQ(0x00, Apu.ReadData());
//
//    // NMI line
//    EXPECT_EQ(false, Apu.NMIActive);
//}
//
//TEST_F(ApuTest, WriteOAMAdress) {
//    Apu.WriteOAMAddress(0x00);
//    EXPECT_EQ(0x00, Apu.OAMAddress);
//
//    Apu.WriteOAMAddress(0xA5);
//    EXPECT_EQ(0xA5, Apu.OAMAddress);
//}
//
//TEST_F(ApuTest, ReadWriteOAM_SingleData) {
//    Apu.WriteOAMAddress(0xA5);
//    Apu.WriteOAMData(0xBE);
//
//    Apu.WriteOAMAddress(0xA5);
//    EXPECT_EQ(0xBE, Apu.ReadOAMData());
//}
//
//TEST_F(ApuTest, ReadWriteOAM_IncreaseAddressOnWrite) {
//    Apu.WriteOAMAddress(0xA5);
//    Apu.WriteOAMData(0xBE);
//    Apu.WriteOAMData(0xEF);
//
//    Apu.WriteOAMAddress(0xA5);
//    EXPECT_EQ(0xBE, Apu.ReadOAMData());
//    Apu.WriteOAMAddress(0xA6);
//    EXPECT_EQ(0xEF, Apu.ReadOAMData());
//}
//
//TEST_F(ApuTest, ReadWriteOAM_DontIncreaseAddressOnRead) {
//    Apu.WriteOAMAddress(0xA5);
//    Apu.WriteOAMData(0xBE);
//    Apu.WriteOAMData(0xEF);
//
//    Apu.WriteOAMAddress(0xA5);
//    EXPECT_EQ(0xBE, Apu.ReadOAMData());
//    EXPECT_EQ(0xBE, Apu.ReadOAMData());
//}
//
//TEST_F(ApuTest, ReadWriteOAM_WrapAroundOnWrite) {
//    Apu.WriteOAMAddress(0xFF);
//    Apu.WriteOAMData(0xBE);
//    Apu.WriteOAMData(0xEF);
//
//    Apu.WriteOAMAddress(0xFF);
//    EXPECT_EQ(0xBE, Apu.ReadOAMData());
//    Apu.WriteOAMAddress(0x00);
//    EXPECT_EQ(0xEF, Apu.ReadOAMData());
//}
//
//TEST_F(ApuTest, WriteScroll_XandY) {
//    Apu.WriteScroll(0xAB);
//    EXPECT_EQ(0xAB, Apu.ScrollX);
//
//    Apu.WriteScroll(0xCD);
//    EXPECT_EQ(0xCD, Apu.ScrollY);
//}
//
//TEST_F(ApuTest, WriteScroll_FlipFlop) {
//    Apu.WriteScroll(0xAB);
//    Apu.WriteScroll(0xCD);
//    EXPECT_EQ(0xAB, Apu.ScrollX);
//    EXPECT_EQ(0xCD, Apu.ScrollY);
//
//    Apu.WriteScroll(0x21);
//    Apu.WriteScroll(0x84);
//    EXPECT_EQ(0x21, Apu.ScrollX);
//    EXPECT_EQ(0x84, Apu.ScrollY);
//}
//
//TEST_F(ApuTest, WriteScroll_ResetLatch) {
//    Apu.WriteScroll(0xAB);
//    Apu.WriteScroll(0xCD);
//    EXPECT_EQ(0xAB, Apu.ScrollX);
//    EXPECT_EQ(0xCD, Apu.ScrollY);
//
//    Apu.WriteScroll(0x21);
//    Apu.ReadStatus();
//    Apu.WriteScroll(0x84);
//    EXPECT_EQ(0x84, Apu.ScrollX);
//    EXPECT_EQ(0xCD, Apu.ScrollY);
//}
//
//TEST_F(ApuTest, WriteAddress_Address) {
//    Apu.WriteAddress(0x12);
//    Apu.WriteAddress(0x34);
//    EXPECT_EQ(0x1234, Apu.Address);
//}
//
//TEST_F(ApuTest, WriteAddress_FlipFlop) {
//    Apu.WriteAddress(0x12);
//    Apu.WriteAddress(0x34);
//    EXPECT_EQ(0x1234, Apu.Address);
//
//    Apu.WriteAddress(0x01);
//    Apu.WriteAddress(0x23);
//    EXPECT_EQ(0x0123, Apu.Address);
//}
//
//TEST_F(ApuTest, WriteAddress_ResetLatch) {
//    Apu.WriteAddress(0x12);
//    Apu.WriteAddress(0x34);
//    EXPECT_EQ(0x1234, Apu.Address);
//
//    Apu.WriteAddress(0x01);
//    Apu.ReadStatus();
//    Apu.WriteAddress(0x23);
//    EXPECT_EQ(0x2334, Apu.Address);
//}
//
//TEST_F(ApuTest, WriteAddress_Mirroring) {
//    Apu.WriteAddress(0x12);
//    Apu.WriteAddress(0x34);
//    EXPECT_EQ(0x1234, Apu.Address);
//
//    Apu.WriteAddress(0x52);
//    Apu.WriteAddress(0x34);
//    EXPECT_EQ(0x1234, Apu.Address);
//
//    Apu.WriteAddress(0x92);
//    Apu.WriteAddress(0x34);
//    EXPECT_EQ(0x1234, Apu.Address);
//
//    Apu.WriteAddress(0xD2);
//    Apu.WriteAddress(0x34);
//    EXPECT_EQ(0x1234, Apu.Address);
//}
//
//TEST_F(ApuTest, ReadData_IncrementAddress) {
//    Apu.WriteControl1(0x00);
//    Apu.WriteAddress(0x12);
//    Apu.WriteAddress(0x34);
//    Apu.ReadData();
//    EXPECT_EQ(0x1234 + 1, Apu.Address);
//
//    Apu.WriteControl1(0x04);
//    Apu.WriteAddress(0x12);
//    Apu.WriteAddress(0x34);
//    Apu.ReadData();
//    EXPECT_EQ(0x1234 + 32, Apu.Address);
//}
//
//TEST_F(ApuTest, ReadData_IncrementAddress_Mirroring) {
//    Apu.WriteControl1(0x00);
//    Apu.WriteAddress(0x3F);
//    Apu.WriteAddress(0xFF);
//    Apu.ReadData();
//    EXPECT_EQ(0x0000, Apu.Address);
//
//    Apu.WriteControl1(0x04);
//    Apu.WriteAddress(0x3F);
//    Apu.WriteAddress(0xFF);
//    Apu.ReadData();
//    EXPECT_EQ(0x001F, Apu.Address);
//}
//
//TEST_F(ApuTest, WriteData_IncrementAddress) {
//    Apu.WriteControl1(0x00);
//    Apu.WriteAddress(0x12);
//    Apu.WriteAddress(0x34);
//    Apu.WriteData(0x00);
//    EXPECT_EQ(0x1234 + 1, Apu.Address);
//
//    Apu.WriteControl1(0x04);
//    Apu.WriteAddress(0x12);
//    Apu.WriteAddress(0x34);
//    Apu.WriteData(0x00);
//    EXPECT_EQ(0x1234 + 32, Apu.Address);
//}
//
//TEST_F(ApuTest, WriteData_IncrementAddress_Mirroring) {
//    Apu.WriteControl1(0x00);
//    Apu.WriteAddress(0x3F);
//    Apu.WriteAddress(0xFF);
//    Apu.WriteData(0x00);
//    EXPECT_EQ(0x0000, Apu.Address);
//
//    Apu.WriteControl1(0x04);
//    Apu.WriteAddress(0x3F);
//    Apu.WriteAddress(0xFF);
//    Apu.WriteData(0x00);
//    EXPECT_EQ(0x001F, Apu.Address);
//}
//
//TEST_F(ApuTest, ReadData_ReadBuffer) {
//    Apu.WriteAddress(0x10);
//    Apu.WriteAddress(0x00);
//    Apu.WriteData(0xA5);
//    Apu.WriteData(0xAA);
//
//    // At power up, the buffer is 0x00 (already tested)
//    // When reading, the buffer is returned
//    // and re-filled with the data at the current address.
//    // Then the address is increased
//    Apu.WriteAddress(0x10);
//    Apu.WriteAddress(0x00);
//    Apu.ReadData(); // Expected to be 0x00
//    EXPECT_EQ(0x1001, Apu.Address);
//    EXPECT_EQ(0xA5, Apu.ReadData());
//    EXPECT_EQ(0x1002, Apu.Address);
//    EXPECT_EQ(0xAA, Apu.ReadData());
//    EXPECT_EQ(0x1003, Apu.Address);
//}
//
//TEST_F(ApuTest, ReadData_ReadBufferOnHighAddress) {
//    Apu.WriteAddress(0x2F);
//    Apu.WriteAddress(0x00);
//    Apu.WriteData(0xBE);
//
//    Apu.WriteAddress(0x3F);
//    Apu.WriteAddress(0x00);
//    Apu.WriteData(0x15);
//    Apu.WriteData(0x1A);
//
//    // At power up, the buffer is 0x00 (already tested)
//    // When reading in addresses > 0x3EFF, the data in memory is returned
//    Apu.WriteAddress(0x3F);
//    Apu.WriteAddress(0x00);
//    EXPECT_EQ(0x15, Apu.ReadData());
//    EXPECT_EQ(0x3F01, Apu.Address);
//    EXPECT_EQ(0x1A, Apu.ReadData());
//    EXPECT_EQ(0x3F02, Apu.Address);
//
//    // The buffer is still filled based on mirrored-down address
//    Apu.WriteAddress(0x3F);
//    Apu.WriteAddress(0x00);
//    EXPECT_EQ(0x15, Apu.ReadData());
//    EXPECT_EQ(0x3F01, Apu.Address);
//    Apu.WriteAddress(0x00);
//    Apu.WriteAddress(0x00);
//    EXPECT_EQ(0xBE, Apu.ReadData());
//}
//
//TEST_F(ApuTest, NMI_DetailedActivation) {
//    Apu.WriteControl1(Mask<7>(true));
//    EXPECT_EQ(true, Apu.NMIOnVBlank);
//
//    int i;
//    EXPECT_EQ(false, Apu.NMIActive); // NMI inactive at start
//
//                                     // Frame 0, pixels (0, 0) to (341, 240)
//    for (i = 0; i <= 82180; ++i) {
//        Apu.Tick();
//        EXPECT_EQ(false, Apu.NMIActive) << i;
//    }
//
//    // Frame 0, pixels (0, 241) to (341, 260)
//    // First tick of scanline 241 has not triggered NMI yet
//    // Second tick of scanline 241 triggers NMI
//    Apu.Tick();
//    EXPECT_EQ(false, Apu.NMIActive) << i;
//    for (i = 82182; i <= 89000; i++) {
//        Apu.Tick();
//        EXPECT_EQ(true, Apu.NMIActive) << i;
//    }
//
//    // Frame 0, pixels (0, 261) to (341, 261)
//    for (i = 89001; i <= 89341; ++i) {
//        Apu.Tick();
//        EXPECT_EQ(false, Apu.NMIActive) << i;
//    }
//
//    // Frame 1, pixels (0, 0) to (341, 240)
//    for (i = 89342; i <= 171522; ++i) {
//        Apu.Tick();
//        EXPECT_EQ(false, Apu.NMIActive) << i;
//    }
//
//    // Frame 1, pixels (0, 241) to (341, 260)
//    Apu.Tick();
//    EXPECT_EQ(false, Apu.NMIActive) << i;
//    for (i = 171524; i <= 178342; i++) {
//        Apu.Tick();
//        EXPECT_EQ(true, Apu.NMIActive) << i;
//    }
//
//    // Frame 1, pixels (0, 261) to (341, 261)
//    for (i = 178343; i <= 178683; ++i) {
//        Apu.Tick();
//        EXPECT_EQ(false, Apu.NMIActive) << i;
//    }
//}
//
//TEST_F(ApuTest, NMI_Disabled) {
//    EXPECT_EQ(false, Apu.NMIOnVBlank);
//    for (int i = 0; i < VIDEO_SIZE; ++i) {
//        EXPECT_EQ(false, Apu.NMIActive);
//        Apu.Tick();
//    }
//}
//
//TEST_F(ApuTest, NMI_MultipleTriggers) {
//    Apu.WriteControl1(Mask<7>(true));
//    EXPECT_EQ(true, Apu.NMIOnVBlank);
//
//    while (!Apu.NMIActive)
//        Apu.Tick();
//
//    EXPECT_EQ(true, Apu.NMIActive);
//
//    Apu.WriteControl1(0);
//    Apu.Tick();
//    EXPECT_EQ(false, Apu.NMIActive);
//
//    Apu.WriteControl1(Mask<7>(true));
//    Apu.Tick();
//    EXPECT_EQ(true, Apu.NMIActive);
//
//    Apu.Tick();
//    EXPECT_EQ(true, Apu.NMIActive);
//}
//
//TEST_F(ApuTest, NMI_StopOnStatusRead) {
//    Apu.WriteControl1(Mask<7>(true));
//    EXPECT_EQ(true, Apu.NMIOnVBlank);
//
//    while (!Apu.NMIActive)
//        Apu.Tick();
//
//    EXPECT_EQ(true, Apu.NMIActive);
//
//    Apu.ReadStatus();
//    Apu.Tick();
//    EXPECT_EQ(false, Apu.NMIActive);
//}
//
//TEST_F(ApuTest, NMI_MultipleTriggersStopOnStatusRead) {
//    Apu.WriteControl1(Mask<7>(true));
//    EXPECT_EQ(true, Apu.NMIOnVBlank);
//
//    while (!Apu.NMIActive)
//        Apu.Tick();
//
//    EXPECT_EQ(true, Apu.NMIActive);
//
//    Apu.WriteControl1(0);
//    Apu.Tick();
//    EXPECT_EQ(false, Apu.NMIActive);
//
//    Apu.WriteControl1(Mask<7>(true));
//    Apu.Tick();
//    EXPECT_EQ(true, Apu.NMIActive);
//
//    Apu.ReadStatus();
//    Apu.Tick();
//    Apu.WriteControl1(0);
//    Apu.Tick();
//    Apu.WriteControl1(Mask<7>(true));
//    Apu.Tick();
//    EXPECT_EQ(false, Apu.NMIActive);
//}
//
//TEST_F(ApuTest, ApuFrameTime_DisabledRendering) {
//    // 1 pixel per Apu cycle
//    const auto T0 = VIDEO_SIZE;
//
//    // Disabled rendering
//    EXPECT_EQ(false, Apu.ShowBackground);
//    EXPECT_EQ(false, Apu.ShowSprite);
//
//    // Even frame (Frame 0)
//    for (int i = 0; i < T0; ++i) {
//        EXPECT_EQ(0, Apu.FrameCount);
//        EXPECT_EQ(i, Apu.FrameTicks);
//        Apu.Tick();
//    }
//
//    // Odd frame (Frame 1)
//    for (int i = 0; i < T0; ++i) {
//        EXPECT_EQ(1, Apu.FrameCount);
//        EXPECT_EQ(i, Apu.FrameTicks);
//        Apu.Tick();
//    }
//
//    EXPECT_EQ(2, Apu.FrameCount);
//    EXPECT_EQ(0, Apu.FrameTicks);
//}
//
//TEST_F(ApuTest, ApuFrameTime_EnabledRendering) {
//    // 1 pixel per Apu cycle
//    const auto T0 = VIDEO_SIZE;
//
//    // Disabled rendering*
//    Apu.WriteControl2(Mask<3>(true) | Mask<4>(true));
//    EXPECT_EQ(true, Apu.ShowBackground);
//    EXPECT_EQ(true, Apu.ShowSprite);
//
//    // Even frame (Frame 0)
//    for (int i = 0; i < T0; ++i) {
//        EXPECT_EQ(0, Apu.FrameCount);
//        EXPECT_EQ(i, Apu.FrameTicks);
//        Apu.Tick();
//    }
//
//    // Odd frame (Frame 1)
//    for (int i = 0; i < T0 - 1; ++i) {
//        EXPECT_EQ(1, Apu.FrameCount);
//        EXPECT_EQ(i, Apu.FrameTicks);
//        Apu.Tick();
//    }
//
//    EXPECT_EQ(2, Apu.FrameCount);
//    EXPECT_EQ(0, Apu.FrameTicks);
//}
//
//TEST_F(ApuTest, Sprite0Hit_DetectHit) {
//    FAIL();
//}
//
//TEST_F(ApuTest, Sprite0Hit_NoHitConditions) {
//    FAIL();
//}
//
//TEST_F(ApuTest, Sprite0Hit_Reset) {
//    Apu.SpriteZeroHit = true;
//
//    // Reset at first pixel of pre-render scanline (261)
//    for (size_t i = 0; i < 261 * 341; i++) {
//        Apu.Tick();
//        EXPECT_EQ(true, Apu.SpriteZeroHit);
//    }
//    Apu.Tick();
//    EXPECT_EQ(false, Apu.SpriteZeroHit);
//}