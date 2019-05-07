#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Mapper_0.h"

using namespace std;

struct Mapper000Test : public ::testing::Test {
    std::ifstream unrom_stream;
    NesFile unrom_file;
    Mapper_000 unrom;

    std::ifstream nrom256_stream;
    NesFile nrom256_file;
    Mapper_000 nrom256;

    Mapper000Test()
        : unrom_stream("nrom128.nes", std::ios::binary),
          unrom_file(unrom_stream),
          unrom(unrom_file),
          nrom256_stream("nrom256.nes", std::ios::binary),
          nrom256_file(nrom256_stream),
          nrom256(nrom256_file)
    {}

    void ExpectBank0(const Word expected,
                     const BankedAddress actual) {
        EXPECT_EQ(0, actual.Bank);
        EXPECT_EQ(expected, actual.Address);
    }
    void ExpectBank1(const Word expected,
        const BankedAddress actual) {
        EXPECT_EQ(1, actual.Bank);
        EXPECT_EQ(expected, actual.Address);
    }
};

TEST_F(Mapper000Test, NROM128_Cpu_GetFromFirstBank) {
    for (Word addr = 0x0000; addr <= 0x3FFF; ++addr) {
        ExpectBank0(addr, unrom.TranslateCpu(0x8000 + addr));
    }
}

TEST_F(Mapper000Test, NROM128_Cpu_GetFromMirror) {
    for (Word addr = 0x0000; addr <= 0x3FFF; ++addr) {
        ExpectBank0(addr, unrom.TranslateCpu(0xC000 + addr));
    }
}

TEST_F(Mapper000Test, NROM128_Cpu_IsReadOnly) {
    const auto data = unrom.GetCpuAt(0x8000);
    unrom.SetCpuAt(0x8000, data + 1);
    EXPECT_EQ(data, unrom.GetCpuAt(0x8000));
}

TEST_F(Mapper000Test, NROM128_Ppu_GetFromChrBank) {
    for (Word addr = 0x0000; addr <= 0x1FFF; ++addr) {
        EXPECT_EQ(addr, unrom.TranslatePpu(addr));
    }
}

//TEST_F(Mapper000Test, NROM128_Ppu_GetFromVram) {
//	for (Word addr = 0x0000; addr <= 0x0FFF; ++addr) {
//		EXPECT_EQ(addr, nrom128.TranslatePpu(0x2000 + addr));
//	}
//}
//
//TEST_F(Mapper000Test, NROM128_Ppu_GetFromVramMirror) {
//	for (Word addr = 0x0000; addr <= 0x0FFF; ++addr) {
//		EXPECT_EQ(addr, nrom128.TranslatePpu(0x3000 + addr));
//	}
//}

TEST_F(Mapper000Test, NROM128_Ppu_IsReadOnly) {
    const auto data = unrom.GetPpuAt(0x0080);
    unrom.SetPpuAt(0x0080, data + 1);
    EXPECT_EQ(data, unrom.GetPpuAt(0x0080));
}

TEST_F(Mapper000Test, NROM128_Ppu_NoPageIsRAM) {
    NesFile dummy_nrom = unrom_file;
    dummy_nrom.Header.ChrRomPages = 0;
    
    Mapper_000 mapper(dummy_nrom);
    const auto data = mapper.GetPpuAt(0x0080);
    mapper.SetPpuAt(0x0080, data + 1);
    EXPECT_EQ(data + 1, mapper.GetPpuAt(0x0080));
}

TEST_F(Mapper000Test, NROM256_Cpu_GetFromSecondBank) {
    for (Word addr = 0x0000; addr <= 0x3FFF; ++addr) {
        ExpectBank1(addr, nrom256.TranslateCpu(0xC000 + addr));
    }
}

TEST_F(Mapper000Test, NametableHorizontalMirroring) {
    NesFile dummy_nrom = unrom_file;
    dummy_nrom.Header.ScreenMode = NesFile::HeaderDesc::HorizontalMirroring;
    
    Mapper_000 mapper(dummy_nrom);
    for (Word addr = 0x0000; addr <= 0x0F; ++addr) {
        EXPECT_EQ(addr, mapper.NametableAddress(0x2000 + addr));
        EXPECT_EQ(addr, mapper.NametableAddress(0x2400 + addr));
        EXPECT_EQ(0x0400 + addr, mapper.NametableAddress(0x2800 + addr));
        EXPECT_EQ(0x0400 + addr, mapper.NametableAddress(0x2C00 + addr));
    }
}

TEST_F(Mapper000Test, NametableVerticalMirroring) {
    NesFile dummy_nrom = unrom_file;
    dummy_nrom.Header.ScreenMode = NesFile::HeaderDesc::VerticalMirroring;

    Mapper_000 mapper(dummy_nrom);
    for (Word addr = 0x0000; addr <= 0x03FF; ++addr) {
        EXPECT_EQ(addr, mapper.NametableAddress(0x2000 + addr));
        EXPECT_EQ(0x0400 + addr, mapper.NametableAddress(0x2400 + addr));
        EXPECT_EQ(addr, mapper.NametableAddress(0x2800 + addr));
        EXPECT_EQ(0x0400 + addr, mapper.NametableAddress(0x2C00 + addr));
    }
}
