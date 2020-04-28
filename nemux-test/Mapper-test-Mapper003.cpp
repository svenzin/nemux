//#include "gtest/gtest.h"
//#include "gmock/gmock.h"
//
//#include "Mapper_3.h"
//
//using namespace std;
//
//struct Mapper003Test : public ::testing::Test {
//    std::ifstream mmc1_stream;
//    NesFile mmc1_file;
//    Mapper_003 mmc1;
//
//    Mapper003Test()
//        : mmc1_stream("mmc1.nes", std::ios::binary),
//          mmc1_file(mmc1_stream),
//          mmc1(mmc1_file)
//    {}
//};
//
//TEST_F(Mapper003Test, DefaultState) {
//    EXPECT_EQ(Mapper_003::PRGBankingMode::SwitchFirstBank, mmc1.PrgMode);
//    EXPECT_EQ(0, mmc1.PrgBank);
//
//    EXPECT_EQ(Mapper_003::CHRBankingMode::OneBank, mmc1.ChrMode);
//    EXPECT_EQ(0, mmc1.ChrBank0);
//    EXPECT_EQ(0, mmc1.ChrBank1);
//}
//
//TEST_F(Mapper003Test, MMC1_IgnoreConsecutiveWrites) {
//    FAIL();
//}
//
//TEST_F(Mapper003Test, Mirroring_Single_Nametable0) {
//    mmc1.ScreenMode = Mapper_003::Mirroring::Screen0;
//    for (Word addr = 0x0000; addr < 0x0400; ++addr) {
//        EXPECT_EQ(addr, mmc1.NametableAddress(0x2000 + addr));
//        EXPECT_EQ(addr, mmc1.NametableAddress(0x2400 + addr));
//        EXPECT_EQ(addr, mmc1.NametableAddress(0x2800 + addr));
//        EXPECT_EQ(addr, mmc1.NametableAddress(0x2C00 + addr));
//    }
//}
//
//TEST_F(Mapper003Test, Mirroring_Single_Nametable1) {
//    mmc1.ScreenMode = Mapper_003::Mirroring::Screen1;
//    for (Word addr = 0x0000; addr < 0x0400; ++addr) {
//        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2000 + addr));
//        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2400 + addr));
//        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2800 + addr));
//        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2C00 + addr));
//    }
//}
//
//TEST_F(Mapper003Test, Mirroring_Horizontal) {
//    mmc1.ScreenMode = Mapper_003::Mirroring::Horizontal;
//    for (Word addr = 0x0000; addr < 0x0400; ++addr) {
//        EXPECT_EQ(addr, mmc1.NametableAddress(0x2000 + addr));
//        EXPECT_EQ(addr, mmc1.NametableAddress(0x2400 + addr));
//        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2800 + addr));
//        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2C00 + addr));
//    }
//}
//
//TEST_F(Mapper003Test, Mirroring_Vertical) {
//    mmc1.ScreenMode = Mapper_003::Mirroring::Vertical;
//    for (Word addr = 0x0000; addr < 0x0400; ++addr) {
//        EXPECT_EQ(addr, mmc1.NametableAddress(0x2000 + addr));
//        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2400 + addr));
//        EXPECT_EQ(addr, mmc1.NametableAddress(0x2800 + addr));
//        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2C00 + addr));
//    }
//}
//
//TEST_F(Mapper003Test, MMC1_WriteToRegisters) {
//    // Control register
//    { // bits 00000 = 0x00
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//        const auto write = mmc1.WriteToMMC1(0x8000, 0x00);
//        EXPECT_EQ(Mapper_003::MMCRegister::Control, write.Register);
//        EXPECT_EQ(0x00, write.Value);
//    }
//    { // bits 01010 = 0x0A
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x9FFF, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x9FFF, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x9FFF, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x9FFF, 0x01).Register);
//        const auto write = mmc1.WriteToMMC1(0x9FFF, 0x00);
//        EXPECT_EQ(Mapper_003::MMCRegister::Control, write.Register);
//        EXPECT_EQ(0x0A, write.Value);
//    }
//    // CHR0 register
//    { // bits 00011 = 0x03
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xA000, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xA000, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xA000, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xA000, 0x00).Register);
//        const auto write = mmc1.WriteToMMC1(0xA000, 0x00);
//        EXPECT_EQ(Mapper_003::MMCRegister::CHR0, write.Register);
//        EXPECT_EQ(0x03, write.Value);
//    }
//    { // bits 11100 = 0x1C
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xBFFF, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xBFFF, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xBFFF, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xBFFF, 0x01).Register);
//        const auto write = mmc1.WriteToMMC1(0xBFFF, 0x01);
//        EXPECT_EQ(Mapper_003::MMCRegister::CHR0, write.Register);
//        EXPECT_EQ(0x1C, write.Value);
//    }
//    // CHR1 register
//    { // bits 10010 = 0x12
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xC000, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xC000, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xC000, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xC000, 0x00).Register);
//        const auto write = mmc1.WriteToMMC1(0xC000, 0x01);
//        EXPECT_EQ(Mapper_003::MMCRegister::CHR1, write.Register);
//        EXPECT_EQ(0x12, write.Value);
//    }
//    { // bits 01101 = 0x0D
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xDFFF, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xDFFF, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xDFFF, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xDFFF, 0x01).Register);
//        const auto write = mmc1.WriteToMMC1(0xDFFF, 0x00);
//        EXPECT_EQ(Mapper_003::MMCRegister::CHR1, write.Register);
//        EXPECT_EQ(0x0D, write.Value);
//    }
//    // PRG register
//    { // bits 11111 = 0x1F
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xE000, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xE000, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xE000, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xE000, 0x01).Register);
//        const auto write = mmc1.WriteToMMC1(0xE000, 0x01);
//        EXPECT_EQ(Mapper_003::MMCRegister::PRG, write.Register);
//        EXPECT_EQ(0x1F, write.Value);
//    }
//    { // bits 00100 = 0x04
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xFFFF, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xFFFF, 0x00).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xFFFF, 0x01).Register);
//        EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xFFFF, 0x00).Register);
//        const auto write = mmc1.WriteToMMC1(0xFFFF, 0x00);
//        EXPECT_EQ(Mapper_003::MMCRegister::PRG, write.Register);
//        EXPECT_EQ(0x04, write.Value);
//    }
//}
//
//TEST_F(Mapper003Test, MMC1_WriteIsAbove0x8000) {
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x7FFF, 0x00).Register);
//    
//    EXPECT_EQ(Mapper_003::MMCRegister::Control, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//}
//
//TEST_F(Mapper003Test, MMC1_OnlyLastWriteSelectsRegister) {
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xFFFF, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xFFFF, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xFFFF, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0xFFFF, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::Control, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::CHR0, mmc1.WriteToMMC1(0xA000, 0x00).Register);
//
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::CHR1, mmc1.WriteToMMC1(0xC000, 0x00).Register);
//
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::None, mmc1.WriteToMMC1(0x8000, 0x00).Register);
//    EXPECT_EQ(Mapper_003::MMCRegister::PRG, mmc1.WriteToMMC1(0xE000, 0x00).Register);
//}
//
//TEST_F(Mapper003Test, MMC1_ResetWrite) {
//    // Reset (supposedly) does an OR of the control register with 0x0C
//    Mapper_003::MMCWrite write;
//
//    mmc1.WriteControl(0x00);
//    write = mmc1.WriteToMMC1(0x8000, 0x80);
//    EXPECT_EQ(Mapper_003::MMCRegister::Control, write.Register);
//    EXPECT_EQ(0x0C, write.Value);
//
//    mmc1.WriteControl(0x01);
//    write = mmc1.WriteToMMC1(0xA000, 0x80);
//    EXPECT_EQ(Mapper_003::MMCRegister::Control, write.Register);
//    EXPECT_EQ(0x0D, write.Value);
//
//    mmc1.WriteControl(0x0F);
//    write = mmc1.WriteToMMC1(0xC000, 0x80);
//    EXPECT_EQ(Mapper_003::MMCRegister::Control, write.Register);
//    EXPECT_EQ(0x0F, write.Value);
//
//    mmc1.WriteControl(0x10);
//    write = mmc1.WriteToMMC1(0xD000, 0x80);
//    EXPECT_EQ(Mapper_003::MMCRegister::Control, write.Register);
//    EXPECT_EQ(0x1C, write.Value);
//}
//
//TEST_F(Mapper003Test, ControlRegister) {
//    mmc1.WriteControl(0x00);
//    EXPECT_EQ(Mapper_003::Mirroring::Screen0, mmc1.ScreenMode);
//    mmc1.WriteControl(0x01);
//    EXPECT_EQ(Mapper_003::Mirroring::Screen1, mmc1.ScreenMode);
//    mmc1.WriteControl(0x02);
//    EXPECT_EQ(Mapper_003::Mirroring::Vertical, mmc1.ScreenMode);
//    mmc1.WriteControl(0x03);
//    EXPECT_EQ(Mapper_003::Mirroring::Horizontal, mmc1.ScreenMode);
//
//    mmc1.WriteControl(0x00);
//    EXPECT_EQ(Mapper_003::PRGBankingMode::SwitchAllBanks, mmc1.PrgMode);
//    mmc1.WriteControl(0x04);
//    EXPECT_EQ(Mapper_003::PRGBankingMode::SwitchAllBanks, mmc1.PrgMode);
//    mmc1.WriteControl(0x08);
//    EXPECT_EQ(Mapper_003::PRGBankingMode::SwitchLastBank, mmc1.PrgMode);
//    mmc1.WriteControl(0x0C);
//    EXPECT_EQ(Mapper_003::PRGBankingMode::SwitchFirstBank, mmc1.PrgMode);
//
//    mmc1.WriteControl(0x00);
//    EXPECT_EQ(Mapper_003::CHRBankingMode::OneBank, mmc1.ChrMode);
//    mmc1.WriteControl(0x10);
//    EXPECT_EQ(Mapper_003::CHRBankingMode::TwoBanks, mmc1.ChrMode);
//}
//
//TEST_F(Mapper003Test, CHR0Register) {
//    // Include mirroring for banks too high
//    mmc1.ChrBanks.resize(4); // This means 8 banks
//    
//    { // Lowest bit ignored in 8K mode
//        mmc1.ChrMode = Mapper_003::CHRBankingMode::OneBank;
//
//        mmc1.WriteCHR0(0);
//        EXPECT_EQ(0, mmc1.ChrBank0);
//
//        mmc1.WriteCHR0(1);
//        EXPECT_EQ(0, mmc1.ChrBank0);
//
//        mmc1.WriteCHR0(2);
//        EXPECT_EQ(2, mmc1.ChrBank0);
//
//        mmc1.WriteCHR0(3);
//        EXPECT_EQ(2, mmc1.ChrBank0);
//
//        mmc1.WriteCHR0(6);
//        EXPECT_EQ(2, mmc1.ChrBank0);
//
//        mmc1.WriteCHR0(10);
//        EXPECT_EQ(2, mmc1.ChrBank0);
//    }
//    {
//        mmc1.ChrMode = Mapper_003::CHRBankingMode::TwoBanks;
//
//        mmc1.WriteCHR0(0);
//        EXPECT_EQ(0, mmc1.ChrBank0);
//
//        mmc1.WriteCHR0(1);
//        EXPECT_EQ(1, mmc1.ChrBank0);
//
//        mmc1.WriteCHR0(6);
//        EXPECT_EQ(2, mmc1.ChrBank0);
//
//        mmc1.WriteCHR0(10);
//        EXPECT_EQ(2, mmc1.ChrBank0);
//    }
//}
//
//TEST_F(Mapper003Test, CHR1Register) {
//    // Include mirroring for banks too high
//    mmc1.ChrBanks.resize(4);
//
//    { // Ignored in 8K mode
//        mmc1.ChrMode = Mapper_003::CHRBankingMode::OneBank;
//        mmc1.ChrBank1 = 1;
//
//        mmc1.WriteCHR1(0);
//        EXPECT_EQ(1, mmc1.ChrBank1);
//
//        mmc1.WriteCHR1(6);
//        EXPECT_EQ(1, mmc1.ChrBank1);
//    }
//    {
//        mmc1.ChrMode = Mapper_003::CHRBankingMode::TwoBanks;
//
//        mmc1.WriteCHR1(0);
//        EXPECT_EQ(0, mmc1.ChrBank1);
//
//        mmc1.WriteCHR1(1);
//        EXPECT_EQ(1, mmc1.ChrBank1);
//
//        mmc1.WriteCHR1(6);
//        EXPECT_EQ(2, mmc1.ChrBank1);
//
//        mmc1.WriteCHR1(10);
//        EXPECT_EQ(2, mmc1.ChrBank1);
//    }
//}
//
//TEST_F(Mapper003Test, PRGRegister) {
//    // Include mirroring for banks too high
//    mmc1.PrgBanks.resize(4);
//    
//    { // Lowest bit ignored in 32K mode
//        mmc1.PrgMode = Mapper_003::PRGBankingMode::SwitchAllBanks;
//
//        mmc1.WritePRG(0);
//        EXPECT_EQ(0, mmc1.PrgBank);
//
//        mmc1.WritePRG(1);
//        EXPECT_EQ(0, mmc1.PrgBank);
//
//        mmc1.WritePRG(2);
//        EXPECT_EQ(2, mmc1.PrgBank);
//
//        mmc1.WritePRG(3);
//        EXPECT_EQ(2, mmc1.PrgBank);
//
//        mmc1.WritePRG(6);
//        EXPECT_EQ(2, mmc1.PrgBank);
//    }
//    {
//        mmc1.PrgMode = Mapper_003::PRGBankingMode::SwitchFirstBank;
//
//        mmc1.WritePRG(0);
//        EXPECT_EQ(0, mmc1.PrgBank);
//
//        mmc1.WritePRG(1);
//        EXPECT_EQ(1, mmc1.PrgBank);
//
//        mmc1.WritePRG(2);
//        EXPECT_EQ(2, mmc1.PrgBank);
//
//        mmc1.WritePRG(3);
//        EXPECT_EQ(3, mmc1.PrgBank);
//
//        mmc1.WritePRG(6);
//        EXPECT_EQ(2, mmc1.PrgBank);
//    }
//    {
//        mmc1.WritePRG(0x00);
//        EXPECT_TRUE(mmc1.HasPrgRam);
//
//        mmc1.WritePRG(0x10);
//        EXPECT_FALSE(mmc1.HasPrgRam);
//    }
//}
//
//TEST_F(Mapper003Test, CHR_8KBanks) {
//    mmc1.ChrMode = Mapper_003::CHRBankingMode::OneBank;
//    
//    // Note that each CHR bank is 4K in size (0x1000)
//    mmc1.ChrBank0 = 0;
//    mmc1.ChrBank1 = 0;
//    for (Word addr = 0; addr < 0x1000; addr++) {
//        const auto ba = mmc1.ToChrRom(addr);
//        EXPECT_EQ(0, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x1000; addr++) {
//        const auto ba = mmc1.ToChrRom(0x1000 + addr);
//        EXPECT_EQ(1, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//
//    mmc1.ChrBank0 = 2;
//    mmc1.ChrBank1 = 0;
//    for (Word addr = 0; addr < 0x1000; addr++) {
//        const auto ba = mmc1.ToChrRom(addr);
//        EXPECT_EQ(2, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x1000; addr++) {
//        const auto ba = mmc1.ToChrRom(0x1000 + addr);
//        EXPECT_EQ(3, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//
//    // Also check that CHR1 bank is ignored
//    mmc1.ChrBank0 = 0;
//    mmc1.ChrBank1 = 2;
//    for (Word addr = 0; addr < 0x1000; addr++) {
//        const auto ba = mmc1.ToChrRom(addr);
//        EXPECT_EQ(0, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x1000; addr++) {
//        const auto ba = mmc1.ToChrRom(0x1000 + addr);
//        EXPECT_EQ(1, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//}
//
//TEST_F(Mapper003Test, CHR_4KBanks) {
//    mmc1.ChrMode = Mapper_003::CHRBankingMode::TwoBanks;
//
//    mmc1.ChrBank0 = 0;
//    mmc1.ChrBank1 = 1;
//    for (Word addr = 0; addr < 0x1000; addr++) {
//        const auto ba = mmc1.ToChrRom(addr);
//        EXPECT_EQ(0, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x1000; addr++) {
//        const auto ba = mmc1.ToChrRom(0x1000 + addr);
//        EXPECT_EQ(1, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//
//    mmc1.ChrBank0 = 3;
//    mmc1.ChrBank1 = 2;
//    for (Word addr = 0; addr < 0x1000; addr++) {
//        const auto ba = mmc1.ToChrRom(addr);
//        EXPECT_EQ(3, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x1000; addr++) {
//        const auto ba = mmc1.ToChrRom(0x1000 + addr);
//        EXPECT_EQ(2, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//}
//
//TEST_F(Mapper003Test, CHR_IsROM) {
//    const auto b = mmc1.GetPpuAt(0x0000);
//    mmc1.SetPpuAt(0x0000, b + 1);
//    EXPECT_EQ(b, mmc1.GetPpuAt(0x0000));
//}
//
//TEST_F(Mapper003Test, PRG_32KBanks) {
//    mmc1.PrgMode = Mapper_003::PRGBankingMode::SwitchAllBanks;
//
//    mmc1.PrgBank = 0;
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0x8000 + addr);
//        EXPECT_EQ(0, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0xC000 + addr);
//        EXPECT_EQ(1, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//
//    mmc1.PrgBank = 2;
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0x8000 + addr);
//        EXPECT_EQ(2, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0xC000 + addr);
//        EXPECT_EQ(3, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//
//    // Bit 0 of the bank is ignored
//    mmc1.PrgBank = 1;
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0x8000 + addr);
//        EXPECT_EQ(0, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0xC000 + addr);
//        EXPECT_EQ(1, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//}
//
//TEST_F(Mapper003Test, PRG_16KBanks_FirstBank) {
//    const auto LastBank = mmc1.PrgBanks.size() - 1;
//    mmc1.PrgMode = Mapper_003::PRGBankingMode::SwitchFirstBank;
//
//    mmc1.PrgBank = 0;
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0x8000 + addr);
//        EXPECT_EQ(0, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0xC000 + addr);
//        EXPECT_EQ(LastBank, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//
//    mmc1.PrgBank = 1;
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0x8000 + addr);
//        EXPECT_EQ(1, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0xC000 + addr);
//        EXPECT_EQ(LastBank, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//}
//
//TEST_F(Mapper003Test, PRG_16KBanks_LastBank) {
//    mmc1.PrgMode = Mapper_003::PRGBankingMode::SwitchLastBank;
//
//    mmc1.PrgBank = 0;
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0x8000 + addr);
//        EXPECT_EQ(0, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0xC000 + addr);
//        EXPECT_EQ(0, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//
//    mmc1.PrgBank = 1;
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0x8000 + addr);
//        EXPECT_EQ(0, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//    for (Word addr = 0; addr < 0x4000; addr++) {
//        const auto ba = mmc1.ToPrgRom(0xC000 + addr);
//        EXPECT_EQ(1, ba.Bank);
//        EXPECT_EQ(addr, ba.Address);
//    }
//}
//
//TEST_F(Mapper003Test, PRG_IsROM) {
//    const auto b = mmc1.GetCpuAt(0x8000);
//    mmc1.SetCpuAt(0x8000, b + 1);
//    EXPECT_EQ(b, mmc1.GetCpuAt(0x8000));
//}
//
//TEST_F(Mapper003Test, LivePRGModeChange) {
//    FAIL();
//}
//
//TEST_F(Mapper003Test, LiveCHRModeChange) {
//    FAIL();
//}
//
//TEST_F(Mapper003Test, PRG_RAM) {
//    // Enabled
//    mmc1.WritePRG(0x00);
//    EXPECT_TRUE(mmc1.HasPrgRam);
//    for (Word addr = 0x6000; addr <= 0x7FFF; ++addr) {
//        const auto b = mmc1.GetCpuAt(addr);
//        mmc1.SetCpuAt(addr, b + 1);
//        EXPECT_EQ(Byte(b + 1), mmc1.GetCpuAt(addr));
//    }
//
//    // Disabled
//    mmc1.WritePRG(0x10);
//    EXPECT_FALSE(mmc1.HasPrgRam);
//    for (Word addr = 0x6000; addr <= 0x7FFF; ++addr) {
//        const auto b = mmc1.GetCpuAt(addr);
//        mmc1.SetCpuAt(addr, b + 1);
//        EXPECT_EQ(b, mmc1.GetCpuAt(addr));
//    }
//}
//
//TEST_F(Mapper003Test, OtherMMC1Versions) {
//    FAIL();
//}
//
//TEST_F(Mapper003Test, NoChrMeansChrRam) {
//    std::ifstream mmc1_stream("mmc1.nes", std::ios::binary);
//    NesFile mmc1_file(mmc1_stream);
//    mmc1_file.ChrRomPages.clear();
//    mmc1_file.Header.ChrRomPages = 0;
//    
//    Mapper_003 mmc1(mmc1_file);
//
//    EXPECT_TRUE(mmc1.HasChrRam);
//    const auto b = mmc1.GetPpuAt(0x0000);
//    mmc1.SetPpuAt(0x0000, b + 1);
//    EXPECT_EQ(b + 1, mmc1.GetPpuAt(0x0000));
//}
//
//TEST_F(Mapper003Test, OpenBus) {
//    FAIL();
//}
//
//TEST_F(Mapper003Test, CpuAddressType) {
//    // Primary RAM
//    for (size_t addr = 0x0000; addr <= 0x1FFF; addr++) {
//        EXPECT_EQ(Mapper_003::CpuAddressType::Unexpected, mmc1.GetCpuAddressType(addr));
//    }
//    // PPU I/O
//    for (size_t addr = 0x2000; addr <= 0x3FFF; addr++) {
//        EXPECT_EQ(Mapper_003::CpuAddressType::Unexpected, mmc1.GetCpuAddressType(addr));
//    }
//    // Not decoded
//    for (size_t addr = 0x4000; addr <= 0x4014; addr++) {
//        EXPECT_EQ(Mapper_003::CpuAddressType::OpenBus, mmc1.GetCpuAddressType(addr));
//    }
//    // Decoded APU registers
//    EXPECT_EQ(Mapper_003::CpuAddressType::Unexpected, mmc1.GetCpuAddressType(0x4015));
//    // Decoded Controller registers
//    EXPECT_EQ(Mapper_003::CpuAddressType::Unexpected, mmc1.GetCpuAddressType(0x4016));
//    EXPECT_EQ(Mapper_003::CpuAddressType::Unexpected, mmc1.GetCpuAddressType(0x4017));
//    // Not decoded
//    for (size_t addr = 0x4018; addr <= 0x5FFF; addr++) {
//        EXPECT_EQ(Mapper_003::CpuAddressType::OpenBus, mmc1.GetCpuAddressType(addr));
//    }
//    // Cartridge RAM
//    for (size_t addr = 0x6000; addr <= 0x7FFF; addr++) {
//        EXPECT_EQ(Mapper_003::CpuAddressType::PRG_RAM, mmc1.GetCpuAddressType(addr));
//    }
//    // Cartridge ROM
//    for (size_t addr = 0x8000; addr <= 0xFFFF; addr++) {
//        EXPECT_EQ(Mapper_003::CpuAddressType::PRG_ROM, mmc1.GetCpuAddressType(addr));
//    }
//}
