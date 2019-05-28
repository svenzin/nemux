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
    FAIL();
}

TEST_F(Mapper001Test, Mirroring_Single_Nametable1) {
    FAIL();
}

TEST_F(Mapper001Test, Mirroring_Horizontal) {
    FAIL();
}

TEST_F(Mapper001Test, Mirroring_Single_Vertical) {
    FAIL();
}

TEST_F(Mapper001Test, MMC1_WriteToRegisters) {
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
    FAIL();
}

TEST_F(Mapper001Test, CHR1Register) {
    FAIL();
}

TEST_F(Mapper001Test, PRGRegister) {
    FAIL();
}

TEST_F(Mapper001Test, CHR_8KBanks) {
    FAIL();
}

TEST_F(Mapper001Test, CHR_4KBanks) {
    FAIL();
}

TEST_F(Mapper001Test, CHR_IsROM) {
    FAIL();
}

TEST_F(Mapper001Test, PRG_32KBanks) {
    FAIL();
}

TEST_F(Mapper001Test, PRG_16KBanks_FirstBank) {
    FAIL();
}

TEST_F(Mapper001Test, PRG_16KBanks_LastBank) {
    FAIL();
}

TEST_F(Mapper001Test, PRG_IsROM) {
    FAIL();
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
