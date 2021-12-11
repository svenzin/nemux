#include "gtest/gtest.h"
//#include "gmock/gmock.h"

#include "Mapper_Nsf.h"

#include <fstream>

using namespace std;

struct MapperNSFTest : public ::testing::Test {
    std::ifstream nsf_stream;
    NsfFile nsf_file;
    Mapper_NSF nsf;

    MapperNSFTest()
        : nsf_stream("nsf.nsf", std::ios::binary),
          nsf_file(nsf_stream),
          nsf(nsf_file)
    {}
};

TEST_F(MapperNSFTest, CpuAddressType) {
    for (int addr = 0x0000; addr <= 0x4019; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::Unexpected, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x4020; addr <= 0x40FF; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::Invalid, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x4100; addr <= 0x410F; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::PlayerRAM, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x4110; addr <= 0x42FF; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::PlayerROM, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x4300; addr <= 0x5FFF; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::Invalid, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x6000; addr <= 0x7FFF; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::RAM, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x8000; addr <= 0xFFF9; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::ROM, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0xFFFA; addr <= 0xFFFF; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::Vector, nsf.GetCpuAddressType(addr));
    }
}

TEST_F(MapperNSFTest, RAM_Is_Zero_Initialized) {
    for (int addr = 0x6000; addr <= 0x7FFF; ++addr) {
        const auto b = nsf.GetCpuAt(addr);
        EXPECT_EQ(0, b);
    }
}

TEST_F(MapperNSFTest, RAM_At_6000_8000) {
    for (int addr = 0x6000; addr <= 0x7FFF; ++addr) {
        const auto b = nsf.GetCpuAt(addr);
        nsf.SetCpuAt(addr, b + 1);
        EXPECT_EQ(b + 1, nsf.GetCpuAt(addr));
    }
}

TEST_F(MapperNSFTest, ROM_At_8000_Plus) {
    for (int addr = 0x8000; addr <= 0xFFFF; ++addr) {
        const auto b = nsf.GetCpuAt(addr);
        nsf.SetCpuAt(addr, b + 1);
        EXPECT_EQ(b, nsf.GetCpuAt(addr));
    }
}

TEST_F(MapperNSFTest, PlayerRAM) {
    for (int addr = 0x4100; addr <= 0x410F; ++addr) {
        const auto b = nsf.GetCpuAt(addr);
        nsf.SetCpuAt(addr, b + 1);
        EXPECT_EQ(b + 1, nsf.GetCpuAt(addr));
    }
}

TEST_F(MapperNSFTest, PlayerROM) {
    for (int addr = 0x4110; addr <= 0x42FF; ++addr) {
        const auto b = nsf.GetCpuAt(addr);
        nsf.SetCpuAt(addr, b + 1);
        EXPECT_EQ(b, nsf.GetCpuAt(addr));
    }
}

TEST_F(MapperNSFTest, LoadAddress_8000) {
    nsf_file.Header.LoadAddress = 0x8000;
    Mapper_NSF nsf(nsf_file);
    for (int i = 0; i < nsf_file.NsfData.size(); ++i) {
        EXPECT_EQ(nsf_file.NsfData[i], nsf.GetCpuAt(0x8000 + i));
    }
}

TEST_F(MapperNSFTest, LoadAddress_Not_8000) {
    nsf_file.Header.LoadAddress = 0x8421;
    Mapper_NSF nsf(nsf_file);
    for (int i = 0; i < nsf_file.NsfData.size(); ++i) {
        EXPECT_EQ(nsf_file.NsfData[i], nsf.GetCpuAt(0x8421 + i));
    }
}

TEST_F(MapperNSFTest, IsNotBanked) {
    EXPECT_FALSE(nsf.IsBanked);
}

struct MapperNSFBankedTest : public ::testing::Test {
    std::ifstream nsf_stream;
    NsfFile nsf_file;
    Mapper_NSF nsf;

    std::ifstream large_stream;
    NsfFile large_file;

    MapperNSFBankedTest()
        : nsf_stream("nsf.banked.nsf", std::ios::binary),
        nsf_file(nsf_stream),
        nsf(nsf_file),
        large_stream("nsf.banked.large.nsf", std::ios::binary),
        large_file(large_stream)
    {}
};

TEST_F(MapperNSFBankedTest, IsBanked) {
    EXPECT_TRUE(nsf.IsBanked);
}

TEST_F(MapperNSFBankedTest, Change_Banks) {
    for (int index = 0x8; index < 0x10; ++index) {
        nsf.SetCpuAt(0x5FF0 + index, 0);
        EXPECT_EQ(0, nsf.GetBank(index * 0x1000));
        nsf.SetCpuAt(0x5FF0 + index, 1);
        EXPECT_EQ(1, nsf.GetBank(index * 0x1000));
    }
}

TEST_F(MapperNSFBankedTest, Address_Translation) {
    for (int i = 0; i < 0x10; ++i) {
        nsf.SetBank(i, i);
    }
    for (int i = 0; i < 0x10; ++i) {
        for (int j = 0; j < 0x1000; ++j) {
            const auto addr = i * 0x1000 + j;
            EXPECT_EQ(addr, nsf.Translate(addr));
        }
    }

    for (int i = 0; i < 0x10; ++i) {
        nsf.SetBank(i, 0xF - i);
    }
    for (int i = 0; i < 0x10; ++i) {
        for (int j = 0; j < 0x1000; ++j) {
            const auto addr = i * 0x1000 + j;
            EXPECT_EQ((0xF - i) * 0x1000 + j, nsf.Translate(addr));
        }
    }
}

TEST_F(MapperNSFBankedTest, LoadAddress_8000) {
    nsf_file.Header.LoadAddress = 0x8000;
    nsf_file.Header.InitialBank[0] = 1;
    nsf_file.Header.InitialBank[1] = 2;
    nsf_file.Header.InitialBank[2] = 1;
    nsf_file.Header.InitialBank[3] = 2;
    nsf_file.Header.InitialBank[4] = 1;
    nsf_file.Header.InitialBank[5] = 2;
    nsf_file.Header.InitialBank[6] = 1;
    nsf_file.Header.InitialBank[7] = 2;
    Mapper_NSF nsf(nsf_file);
    for (int i = 0; i < 0x1000; ++i) {
        EXPECT_EQ(nsf_file.NsfData[0x1000 + i], nsf.GetCpuAt(0x8000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x2000 + i], nsf.GetCpuAt(0x9000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x1000 + i], nsf.GetCpuAt(0xA000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x2000 + i], nsf.GetCpuAt(0xB000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x1000 + i], nsf.GetCpuAt(0xC000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x2000 + i], nsf.GetCpuAt(0xD000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x1000 + i], nsf.GetCpuAt(0xE000 + i));
        if (i >= 0x0FFA) continue; // skip interrupt vectors
        EXPECT_EQ(nsf_file.NsfData[0x2000 + i], nsf.GetCpuAt(0xF000 + i));
    }
}

TEST_F(MapperNSFBankedTest, LoadAddress_Not_8000) {
    const Word offset = 0x0421;
    nsf_file.Header.LoadAddress = 0x8000 + offset;
    nsf_file.Header.InitialBank[0] = 1;
    nsf_file.Header.InitialBank[1] = 2;
    nsf_file.Header.InitialBank[2] = 1;
    nsf_file.Header.InitialBank[3] = 2;
    nsf_file.Header.InitialBank[4] = 1;
    nsf_file.Header.InitialBank[5] = 2;
    nsf_file.Header.InitialBank[6] = 1;
    nsf_file.Header.InitialBank[7] = 2;
    Mapper_NSF nsf(nsf_file);
    for (int i = 0; i < 0x1000; ++i) {
        EXPECT_EQ(nsf_file.NsfData[0x1000 + i - offset], nsf.GetCpuAt(0x8000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x2000 + i - offset], nsf.GetCpuAt(0x9000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x1000 + i - offset], nsf.GetCpuAt(0xA000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x2000 + i - offset], nsf.GetCpuAt(0xB000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x1000 + i - offset], nsf.GetCpuAt(0xC000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x2000 + i - offset], nsf.GetCpuAt(0xD000 + i));
        EXPECT_EQ(nsf_file.NsfData[0x1000 + i - offset], nsf.GetCpuAt(0xE000 + i));
        if (i >= 0x0FFA) continue; // skip interrupt vectors
        EXPECT_EQ(nsf_file.NsfData[0x2000 + i - offset], nsf.GetCpuAt(0xF000 + i));
    }
}

TEST_F(MapperNSFBankedTest, LoadAddress_Not_8000_First_Bank) {
    const Word offset = 0x0421;
    nsf_file.Header.LoadAddress = 0x8000 + offset;
    nsf_file.Header.InitialBank[0] = 0;
    Mapper_NSF nsf(nsf_file);
    for (int i = 0; i < 0x1000 - offset; ++i) {
        EXPECT_EQ(nsf_file.NsfData[i], nsf.GetCpuAt(0x8000 + offset + i));
    }
}

TEST_F(MapperNSFBankedTest, CpuAddressType) {
    for (int addr = 0x0000; addr <= 0x4019; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::Unexpected, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x4020; addr <= 0x40FF; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::Invalid, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x4100; addr <= 0x410F; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::PlayerRAM, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x4110; addr <= 0x42FF; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::PlayerROM, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x4300; addr <= 0x5FF7; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::Invalid, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x5FF8; addr <= 0x5FFF; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::Register, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x6000; addr <= 0x7FFF; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::RAM, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0x8000; addr <= 0xFFF9; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::ROM, nsf.GetCpuAddressType(addr));
    }
    for (int addr = 0xFFFA; addr <= 0xFFFF; ++addr) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::Vector, nsf.GetCpuAddressType(addr));
    }
}

TEST_F(MapperNSFBankedTest, Handle_Large_Number_Of_Banks) {
    large_file.Header.LoadAddress = 0x8000;
    large_file.Header.InitialBank[0] = 8;
    large_file.Header.InitialBank[1] = 8;
    large_file.Header.InitialBank[2] = 8;
    large_file.Header.InitialBank[3] = 8;
    large_file.Header.InitialBank[4] = 8;
    large_file.Header.InitialBank[5] = 8;
    large_file.Header.InitialBank[6] = 8;
    large_file.Header.InitialBank[7] = 8;
    Mapper_NSF nsf(large_file);
    for (int i = 0; i < 0x1000; ++i) {
        EXPECT_EQ(large_file.NsfData[0x8000 + i], nsf.GetCpuAt(0x8000 + i));
        EXPECT_EQ(large_file.NsfData[0x8000 + i], nsf.GetCpuAt(0x9000 + i));
        EXPECT_EQ(large_file.NsfData[0x8000 + i], nsf.GetCpuAt(0xA000 + i));
        EXPECT_EQ(large_file.NsfData[0x8000 + i], nsf.GetCpuAt(0xB000 + i));
        EXPECT_EQ(large_file.NsfData[0x8000 + i], nsf.GetCpuAt(0xC000 + i));
        EXPECT_EQ(large_file.NsfData[0x8000 + i], nsf.GetCpuAt(0xD000 + i));
        EXPECT_EQ(large_file.NsfData[0x8000 + i], nsf.GetCpuAt(0xE000 + i));
        if (i >= 0x0FFA) continue; // skip interrupt vectors
        EXPECT_EQ(large_file.NsfData[0x8000 + i], nsf.GetCpuAt(0xF000 + i));
    }
}

struct MapperNSFVRC6Test : public ::testing::Test {
    std::ifstream nsf_stream;
    NsfFile nsf_file;
    Mapper_NSF nsf;

    MapperNSFVRC6Test()
        : nsf_stream("nsf.vrc6.nsf", std::ios::binary),
        nsf_file(nsf_stream),
        nsf(nsf_file)
    {}
};

TEST_F(MapperNSFVRC6Test, CpuAddressType_VRC6) {
    for (auto addr : {
        0x9000, 0x9001, 0x9002, 0x9003,
        0xA000, 0xA001, 0xA002,
        0xB000, 0xB001, 0xB002 }) {
        EXPECT_EQ(Mapper_NSF::CpuAddressType::VRC6, nsf.GetCpuAddressType(addr));
    }
}

