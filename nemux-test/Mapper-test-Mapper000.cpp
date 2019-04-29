#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Mapper_0.h"

using namespace std;

struct Mapper000Test : public ::testing::Test {
    std::ifstream nrom128_stream;
    NesFile nrom128_file;
    Mapper_000 nrom128;

    std::ifstream nrom256_stream;
    NesFile nrom256_file;
    Mapper_000 nrom256;

    Mapper000Test()
        : nrom128_stream("nrom128.nes", std::ios::binary),
          nrom128_file(nrom128_stream),
          nrom128(nrom128_file),
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
        ExpectBank0(addr, nrom128.TranslateCpu(0x8000 + addr));
    }
}

TEST_F(Mapper000Test, NROM128_Cpu_GetFromMirror) {
    for (Word addr = 0x0000; addr <= 0x3FFF; ++addr) {
        ExpectBank0(addr, nrom128.TranslateCpu(0xC000 + addr));
    }
}

TEST_F(Mapper000Test, NROM128_Cpu_IsReadOnly) {
    const auto data = nrom128.GetCpuAt(0x8000);
    nrom128.SetCpuAt(0x8000, data + 1);
    EXPECT_EQ(data, nrom128.GetCpuAt(0x8000));
}

TEST_F(Mapper000Test, NROM128_Ppu_GetFromChrBank) {
    for (Word addr = 0x0000; addr <= 0x1FFF; ++addr) {
        EXPECT_EQ(addr, nrom128.TranslatePpu(addr));
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
    const auto data = nrom128.GetPpuAt(0x0080);
    nrom128.SetPpuAt(0x0080, data + 1);
    EXPECT_EQ(data, nrom128.GetPpuAt(0x0080));
}

TEST_F(Mapper000Test, NROM256_Cpu_GetFromSecondBank) {
    for (Word addr = 0x0000; addr <= 0x3FFF; ++addr) {
        ExpectBank1(addr, nrom256.TranslateCpu(0xC000 + addr));
    }
}
