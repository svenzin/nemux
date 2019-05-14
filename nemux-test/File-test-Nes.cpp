#include "gtest/gtest.h"

#include "NesFile.h"
#include "BitUtil.h"
#include <sstream>

struct NesFileTest : public ::testing::Test {
    std::string data;
    NesFileTest() {
        data = std::string(16, 0);
        data[0] = NES_TAG[0];
        data[1] = NES_TAG[1];
        data[2] = NES_TAG[2];
        data[3] = NES_TAG[3];
    }
};

TEST_F(NesFileTest, Header_Missing) {
    std::istringstream iss("");
    EXPECT_THROW(NesFile::HeaderDesc header(iss), invalid_format);
}

TEST_F(NesFileTest, Header_TooShort) {
    std::istringstream iss("NES");
    EXPECT_THROW(NesFile::HeaderDesc header(iss), invalid_format);
}

TEST_F(NesFileTest, Header_Present) {
    std::istringstream iss(NES_TAG"456789ABCDEF");
    EXPECT_NO_THROW(NesFile::HeaderDesc header(iss));
}

TEST_F(NesFileTest, Header_PrgRomPages) {
    data[4] = 0x02;

    std::istringstream iss(data);
    NesFile::HeaderDesc header(iss);
    ASSERT_EQ(2, header.PrgRomPages);
}

TEST_F(NesFileTest, Header_ChrRomPages) {
    data[5] = 0x03;

    std::istringstream iss(data);
    NesFile::HeaderDesc header(iss);
    ASSERT_EQ(3, header.ChrRomPages);
}

TEST_F(NesFileTest, Header_ScreenMode) {
    typedef NesFile::HeaderDesc::Flags6Bits Bits;
    {
        data[6] = Mask<Bits::Mirroring>(0)
            | Mask<Bits::FourScreen>(0);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(NesFile::HeaderDesc::HorizontalMirroring,
            header.ScreenMode);
    }
    {
        data[6] = Mask<Bits::Mirroring>(1)
            | Mask<Bits::FourScreen>(0);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(NesFile::HeaderDesc::VerticalMirroring,
            header.ScreenMode);
    }
    {
        data[6] = Mask<Bits::Mirroring>(0)
            | Mask<Bits::FourScreen>(1);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(NesFile::HeaderDesc::FourScreenMode,
            header.ScreenMode);
    }
    {
        data[6] = Mask<Bits::Mirroring>(1)
            | Mask<Bits::FourScreen>(1);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(NesFile::HeaderDesc::FourScreenMode,
            header.ScreenMode);
    }
}

TEST_F(NesFileTest, Header_HasTrainer) {
    typedef NesFile::HeaderDesc::Flags6Bits Bits;
    {
        data[6] = Mask<Bits::Trainer>(0);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(false, header.HasTrainer);
    }
    {
        data[6] = Mask<Bits::Trainer>(1);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(true, header.HasTrainer);
    }
}

TEST_F(NesFileTest, Header_HasBattery) {
    typedef NesFile::HeaderDesc::Flags6Bits Bits;
    {
        data[6] = Mask<Bits::SRAM>(0);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(false, header.HasBattery);
    }
    {
        data[6] = Mask<Bits::SRAM>(1);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(true, header.HasBattery);
    }
}

TEST_F(NesFileTest, Header_IsPC10) {
    typedef NesFile::HeaderDesc::Flags7Bits Bits;
    {
        data[7] = Mask<Bits::PC10>(0);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(false, header.IsPlaychoice10);
    }
    {
        data[7] = Mask<Bits::PC10>(1);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(true, header.IsPlaychoice10);
    }
}

TEST_F(NesFileTest, Header_IsVs) {
    typedef NesFile::HeaderDesc::Flags7Bits Bits;
    {
        data[7] = Mask<Bits::Versus>(0);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(false, header.IsVsUnisystem);
    }
    {
        data[7] = Mask<Bits::Versus>(1);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(true, header.IsVsUnisystem);
    }
}

TEST_F(NesFileTest, Header_IsNES2Format) {
    typedef NesFile::HeaderDesc::Flags7Bits Bits;
    {
        data[7] = Mask<Bits::NES2lo>(0) | Mask<Bits::NES2hi>(0);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(false, header.IsNES2Format);
    }
    {
        data[7] = Mask<Bits::NES2lo>(1) | Mask<Bits::NES2hi>(0);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(false, header.IsNES2Format);
    }
    {
        data[7] = Mask<Bits::NES2lo>(1) | Mask<Bits::NES2hi>(1);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(false, header.IsNES2Format);
    }
    {
        data[7] = Mask<Bits::NES2lo>(0) | Mask<Bits::NES2hi>(1);
        std::istringstream iss(data);
        NesFile::HeaderDesc header(iss);
        EXPECT_EQ(true, header.IsNES2Format);
    }
}

TEST_F(NesFileTest, Header_MapperNumber) {
    typedef NesFile::HeaderDesc::Flags7Bits Bits;
    data[6] = 0x50;
    data[7] = 0xA0;
    std::istringstream iss(data);
    NesFile::HeaderDesc header(iss);
    ASSERT_EQ(0xA5, header.MapperNumber);
}

TEST_F(NesFileTest, NesFile_Rejected) {
    {
        std::istringstream iss(data);
        NesFile::HeaderDesc desc(iss);
        EXPECT_NO_THROW(NesFile::Validate(desc));
    }
    {
        std::istringstream iss(data);
        NesFile::HeaderDesc desc(iss);
        desc.HasTrainer = true;
        EXPECT_THROW(NesFile::Validate(desc), unsupported_format);
    }
    {
        std::istringstream iss(data);
        NesFile::HeaderDesc desc(iss);
        desc.HasBattery = true;
        EXPECT_THROW(NesFile::Validate(desc), unsupported_format);
    }
    {
        std::istringstream iss(data);
        NesFile::HeaderDesc desc(iss);
        desc.IsPlaychoice10 = true;
        EXPECT_THROW(NesFile::Validate(desc), unsupported_format);
    }
    {
        std::istringstream iss(data);
        NesFile::HeaderDesc desc(iss);
        desc.IsVsUnisystem = true;
        EXPECT_THROW(NesFile::Validate(desc), unsupported_format);
    }
    {
        std::istringstream iss(data);
        NesFile::HeaderDesc desc(iss);
        desc.IsNES2Format = true;
        EXPECT_THROW(NesFile::Validate(desc), unsupported_format);
    }
}

TEST_F(NesFileTest, NesFile_TooSmall) {
    {
        std::string file(20, 0xA5);
        for (int i = 0; i < 16; ++i) file[i] = data[i];
        file[4] = 0x01;
        file[5] = 0x01;

        std::istringstream iss(file);
        EXPECT_THROW(NesFile nes(iss), invalid_format);
    }
    {
        std::string file(0x4100, 0xA5);
        for (int i = 0; i < 16; ++i) file[i] = data[i];
        file[4] = 0x01;
        file[5] = 0x01;

        std::istringstream iss(file);
        EXPECT_THROW(NesFile nes(iss), invalid_format);
    }
}

TEST_F(NesFileTest, NesFile_Success) {
    std::string file(16 + 0x4000 + 0x2000, 0x00);
    for (int i = 0; i < 16; ++i) file[i] = data[i];
    file[4] = 0x01;
    file[5] = 0x01;
    for (int i = 0; i < 0x4000; i += 4) {
        file[16 + i + 0] = 0x0B;
        file[16 + i + 1] = 0xAD;
        file[16 + i + 2] = 0xBE;
        file[16 + i + 3] = 0xEF;
    }
    for (int i = 0; i < 0x2000; i += 4) {
        file[16 + 0x4000 + i + 0] = 0x0B;
        file[16 + 0x4000 + i + 1] = 0xAD;
        file[16 + 0x4000 + i + 2] = 0xBA;
        file[16 + 0x4000 + i + 3] = 0xBE;
    }

    std::istringstream iss(file);
    NesFile nes(iss);
    ASSERT_EQ(1, nes.PrgRomPages.size());
    for (int i = 0; i < 0x4000; i += 4) {
        EXPECT_EQ(0x0B, nes.PrgRomPages[0][i + 0]);
        EXPECT_EQ(0xAD, nes.PrgRomPages[0][i + 1]);
        EXPECT_EQ(0xBE, nes.PrgRomPages[0][i + 2]);
        EXPECT_EQ(0xEF, nes.PrgRomPages[0][i + 3]);
    }
    ASSERT_EQ(1, nes.ChrRomPages.size());
    for (int i = 0; i < 0x2000; i += 4) {
        EXPECT_EQ(0x0B, nes.ChrRomPages[0][i + 0]);
        EXPECT_EQ(0xAD, nes.ChrRomPages[0][i + 1]);
        EXPECT_EQ(0xBA, nes.ChrRomPages[0][i + 2]);
        EXPECT_EQ(0xBE, nes.ChrRomPages[0][i + 3]);
    }
}
