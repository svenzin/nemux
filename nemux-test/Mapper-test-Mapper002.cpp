#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Mapper_2.h"

using namespace std;

struct Mapper002Test : public ::testing::Test {
    std::ifstream unrom_stream;
    NesFile unrom_file;
    Mapper_002 unrom;

    Mapper002Test()
        : unrom_stream("unrom.nes", std::ios::binary),
          unrom_file(unrom_stream),
          unrom(unrom_file)
    {}
};

TEST_F(Mapper002Test, Cpu_GetFromFirstBank) {
    for (Word addr = 0x0000; addr <= 0x3FFF; ++addr) {
        const auto address = unrom.TranslateCpu(0x8000 + addr);
        EXPECT_EQ(0, address.Bank);
        EXPECT_EQ(addr, address.Address);
    }
}

TEST_F(Mapper002Test, Cpu_GetFromLastBank) {
    const auto lastBank = unrom_file.Header.PrgRomPages - 1;
    for (Word addr = 0x0000; addr <= 0x3FFF; ++addr) {
        const auto address = unrom.TranslateCpu(0xC000 + addr);
        EXPECT_EQ(lastBank, address.Bank);
        EXPECT_EQ(addr, address.Address);
    }
}

TEST_F(Mapper002Test, Cpu_ChangeBank) {
    for (size_t bank = 0; bank < unrom_file.Header.PrgRomPages; ++bank) {
        unrom.SetCpuAt(0x8000, bank);
        const auto address = unrom.TranslateCpu(0x8000);
        EXPECT_EQ(bank, address.Bank);
        EXPECT_EQ(0x0000, address.Address);
    }
}

TEST_F(Mapper002Test, Ppu_IsRAM) {
    const auto data = unrom.GetPpuAt(0x0080);
    unrom.SetPpuAt(0x0080, data + 1);
    EXPECT_EQ(data + 1, unrom.GetPpuAt(0x0080));
}

TEST_F(Mapper002Test, NametableHorizontalMirroring) {
    NesFile dummy_nrom = unrom_file;
    dummy_nrom.Header.ScreenMode = NesFile::HeaderDesc::HorizontalMirroring;
    
    Mapper_002 mapper(dummy_nrom);
    for (Word addr = 0x0000; addr <= 0x0F; ++addr) {
        EXPECT_EQ(addr, mapper.NametableAddress(0x2000 + addr));
        EXPECT_EQ(addr, mapper.NametableAddress(0x2400 + addr));
        EXPECT_EQ(0x0400 + addr, mapper.NametableAddress(0x2800 + addr));
        EXPECT_EQ(0x0400 + addr, mapper.NametableAddress(0x2C00 + addr));
    }
}

TEST_F(Mapper002Test, NametableVerticalMirroring) {
    NesFile dummy_nrom = unrom_file;
    dummy_nrom.Header.ScreenMode = NesFile::HeaderDesc::VerticalMirroring;

    Mapper_002 mapper(dummy_nrom);
    for (Word addr = 0x0000; addr <= 0x03FF; ++addr) {
        EXPECT_EQ(addr, mapper.NametableAddress(0x2000 + addr));
        EXPECT_EQ(0x0400 + addr, mapper.NametableAddress(0x2400 + addr));
        EXPECT_EQ(addr, mapper.NametableAddress(0x2800 + addr));
        EXPECT_EQ(0x0400 + addr, mapper.NametableAddress(0x2C00 + addr));
    }
}
