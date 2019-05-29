#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Mapper_1.h"

using namespace std;

struct Mapper001Test : public ::testing::Test {
    std::ifstream mmc1_stream;
    NesFile mmc1_file;
    Mapper_001 mmc1;

    Mapper001Test()
        : mmc1_stream("mmc1.nes", std::ios::binary),
          mmc1_file(mmc1_stream),
          mmc1(mmc1_file)
    {}
};

TEST_F(Mapper001Test, DefaultState) {
    EXPECT_EQ(Mapper_001::PRGBankingMode::SwitchFirstBank, mmc1.PrgMode);
    EXPECT_EQ(Mapper_001::CHRBankingMode::OneBank, mmc1.ChrMode);
}

TEST_F(Mapper001Test, MMC1_IgnoreConsecutiveWrites) {
    FAIL();
}

TEST_F(Mapper001Test, Mirroring_Single_Nametable0) {
    mmc1.ScreenMode = Mapper_001::Mirroring::Screen0;
    for (Word addr = 0x0000; addr < 0x0400; ++addr) {
        EXPECT_EQ(addr, mmc1.NametableAddress(0x2000 + addr));
        EXPECT_EQ(addr, mmc1.NametableAddress(0x2400 + addr));
        EXPECT_EQ(addr, mmc1.NametableAddress(0x2800 + addr));
        EXPECT_EQ(addr, mmc1.NametableAddress(0x2C00 + addr));
    }
}

TEST_F(Mapper001Test, Mirroring_Single_Nametable1) {
    mmc1.ScreenMode = Mapper_001::Mirroring::Screen1;
    for (Word addr = 0x0000; addr < 0x0400; ++addr) {
        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2000 + addr));
        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2400 + addr));
        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2800 + addr));
        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2C00 + addr));
    }
}

TEST_F(Mapper001Test, Mirroring_Horizontal) {
    mmc1.ScreenMode = Mapper_001::Mirroring::Horizontal;
    for (Word addr = 0x0000; addr < 0x0400; ++addr) {
        EXPECT_EQ(addr, mmc1.NametableAddress(0x2000 + addr));
        EXPECT_EQ(addr, mmc1.NametableAddress(0x2400 + addr));
        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2800 + addr));
        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2C00 + addr));
    }
}

TEST_F(Mapper001Test, Mirroring_Vertical) {
    mmc1.ScreenMode = Mapper_001::Mirroring::Vertical;
    for (Word addr = 0x0000; addr < 0x0400; ++addr) {
        EXPECT_EQ(addr, mmc1.NametableAddress(0x2000 + addr));
        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2400 + addr));
        EXPECT_EQ(addr, mmc1.NametableAddress(0x2800 + addr));
        EXPECT_EQ(0x0400 + addr, mmc1.NametableAddress(0x2C00 + addr));
    }
}

TEST_F(Mapper001Test, MMC1_WriteToRegisters) {
    FAIL();
}

TEST_F(Mapper001Test, MMC1_WriteIsAbove0x8000) {
    FAIL();
}

TEST_F(Mapper001Test, MMC1_OnlyLastWriteSelectsRegister) {
    FAIL();
}

TEST_F(Mapper001Test, MMC1_ResetWrite) {
    FAIL();
}

TEST_F(Mapper001Test, ControlRegister) {
    FAIL();
}

TEST_F(Mapper001Test, CHR0Register) {
    // Include mirroring for banks too high
    FAIL();
    //mmc1.ChrPages.resize(4);
    //mmc1.WriteCHR0Register();
}

TEST_F(Mapper001Test, CHR1Register) {
    // Include mirroring for banks too high
    FAIL();
}

TEST_F(Mapper001Test, PRGRegister) {
    // Include mirroring for banks too high
    FAIL();
}

TEST_F(Mapper001Test, CHR_8KBanks) {
    mmc1.ChrMode = Mapper_001::CHRBankingMode::OneBank;
    
    mmc1.ChrBank0 = 0;
    mmc1.ChrBank1 = 0;
    for (Word addr = 0; addr < 0x2000; addr++) {
        const auto ba = mmc1.TranslatePpu(addr);
        EXPECT_EQ(0, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }

    mmc1.ChrBank0 = 1;
    mmc1.ChrBank1 = 0;
    for (Word addr = 0; addr < 0x2000; addr++) {
        const auto ba = mmc1.TranslatePpu(addr);
        EXPECT_EQ(1, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }

    // Also check that CHR1 bank is ignored
    mmc1.ChrBank0 = 1;
    mmc1.ChrBank1 = 1;
    for (Word addr = 0; addr < 0x2000; addr++) {
        const auto ba = mmc1.TranslatePpu(addr);
        EXPECT_EQ(1, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
}

TEST_F(Mapper001Test, CHR_4KBanks) {
    mmc1.ChrMode = Mapper_001::CHRBankingMode::TwoBanks;

    mmc1.ChrBank0 = 0;
    mmc1.ChrBank1 = 1;
    for (Word addr = 0; addr < 0x1000; addr++) {
        const auto ba = mmc1.TranslatePpu(addr);
        EXPECT_EQ(0, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
    for (Word addr = 0; addr < 0x1000; addr++) {
        const auto ba = mmc1.TranslatePpu(0x1000 + addr);
        EXPECT_EQ(0, ba.Bank);
        EXPECT_EQ(0x1000 + addr, ba.Address);
    }

    mmc1.ChrBank0 = 3;
    mmc1.ChrBank1 = 2;
    for (Word addr = 0; addr < 0x1000; addr++) {
        const auto ba = mmc1.TranslatePpu(addr);
        EXPECT_EQ(1, ba.Bank);
        EXPECT_EQ(0x1000 + addr, ba.Address);
    }
    for (Word addr = 0; addr < 0x1000; addr++) {
        const auto ba = mmc1.TranslatePpu(0x1000 + addr);
        EXPECT_EQ(1, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
}

TEST_F(Mapper001Test, CHR_IsROM) {
    const auto b = mmc1.GetPpuAt(0x0000);
    mmc1.SetPpuAt(0x0000, b + 1);
    EXPECT_EQ(b, mmc1.GetPpuAt(0x0000));
}

TEST_F(Mapper001Test, PRG_32KBanks) {
    mmc1.PrgMode = Mapper_001::PRGBankingMode::SwitchAllBanks;

    mmc1.PrgBank = 0;
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0x8000 + addr);
        EXPECT_EQ(0, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0xC000 + addr);
        EXPECT_EQ(1, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }

    mmc1.PrgBank = 2;
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0x8000 + addr);
        EXPECT_EQ(2, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0xC000 + addr);
        EXPECT_EQ(3, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }

    // Bit 0 of the bank is ignored
    mmc1.PrgBank = 1;
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0x8000 + addr);
        EXPECT_EQ(0, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0xC000 + addr);
        EXPECT_EQ(1, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
}

TEST_F(Mapper001Test, PRG_16KBanks_FirstBank) {
    const auto LastBank = mmc1.PrgPages.size() - 1;
    mmc1.PrgMode = Mapper_001::PRGBankingMode::SwitchFirstBank;

    mmc1.PrgBank = 0;
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0x8000 + addr);
        EXPECT_EQ(0, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0xC000 + addr);
        EXPECT_EQ(LastBank, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }

    mmc1.PrgBank = 1;
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0x8000 + addr);
        EXPECT_EQ(1, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0xC000 + addr);
        EXPECT_EQ(LastBank, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
}

TEST_F(Mapper001Test, PRG_16KBanks_LastBank) {
    mmc1.PrgMode = Mapper_001::PRGBankingMode::SwitchLastBank;

    mmc1.PrgBank = 0;
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0x8000 + addr);
        EXPECT_EQ(0, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0xC000 + addr);
        EXPECT_EQ(0, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }

    mmc1.PrgBank = 1;
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0x8000 + addr);
        EXPECT_EQ(0, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
    for (Word addr = 0; addr < 0x4000; addr++) {
        const auto ba = mmc1.TranslateCpu(0xC000 + addr);
        EXPECT_EQ(1, ba.Bank);
        EXPECT_EQ(addr, ba.Address);
    }
}

TEST_F(Mapper001Test, PRG_IsROM) {
    const auto b = mmc1.GetCpuAt(0x8000);
    mmc1.SetCpuAt(0x8000, b + 1);
    EXPECT_EQ(b, mmc1.GetCpuAt(0x8000));
}

TEST_F(Mapper001Test, LivePRGModeChange) {
    FAIL();
}

TEST_F(Mapper001Test, LiveCHRModeChange) {
    FAIL();
}

TEST_F(Mapper001Test, RAM) {
    FAIL();
}

TEST_F(Mapper001Test, OtherMMC1Versions) {
    FAIL();
}
