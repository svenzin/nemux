#include "gtest/gtest.h"

#include "NsfFile.h"
#include "BitUtil.h"
//#include <sstream>

struct NSFFileTest : public ::testing::Test {
    std::string data;
    NSFFileTest() {
        data = std::string(0x80, 0);
        data[0x00] = NSF_TAG[0];
        data[0x01] = NSF_TAG[1];
        data[0x02] = NSF_TAG[2];
        data[0x03] = NSF_TAG[3];
        data[0x04] = NSF_TAG[4];

        // Minimum valid header
        data[0x05] = 1; // Version
        data[0x06] = 1; // SongCount
        data[0x07] = 1; // FirstSong
        data[0x09] = 0x80; // LoadAddress
        data[0x0B] = 0x80; // InitAddress
        data[0x0D] = 0x80; // PlayAddress
        data[0x6E] = 1; // PeriodNTSC
        data[0x78] = 1; // PeriodPAL
    }
};

TEST_F(NSFFileTest, Header_Tag_Missing) {
    std::istringstream iss("");
    EXPECT_THROW(NsfFile::HeaderDesc header(iss), invalid_format);
}

TEST_F(NSFFileTest, Header_Tag_TooShort) {
    std::istringstream iss("NES");
    EXPECT_THROW(NsfFile::HeaderDesc header(iss), invalid_format);
}

TEST_F(NSFFileTest, Header_Tag_Present) {
    std::istringstream iss(data);
    EXPECT_NO_THROW(NsfFile::HeaderDesc header(iss));
}

TEST_F(NSFFileTest, Header_Version_Number) {
    data[0x05] = 1;
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ(1, header.Version);
}

TEST_F(NSFFileTest, Header_Song_Count) {
    data[0x06] = 10;
    data[0x07] = 1;
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ(10, header.SongCount);
}

TEST_F(NSFFileTest, Header_First_Song_Index) {
    // The first song index is 1-based in the NSF file
    // but stored as a 0-based index in the header
    data[0x06] = 10;
    data[0x07] = 3;
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ(2, header.FirstSong);
}

TEST_F(NSFFileTest, Header_Load_Address) {
    data[0x08] = 0x01;
    data[0x09] = 0x80;
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ(0x8001, header.LoadAddress);
}

TEST_F(NSFFileTest, Header_Init_Address) {
    data[0x0A] = 0x01;
    data[0x0B] = 0x80;
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ(0x8001, header.InitAddress);
}

TEST_F(NSFFileTest, Header_Play_Address) {
    data[0x0C] = 0x01;
    data[0x0D] = 0x80;
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ(0x8001, header.PlayAddress);
}

TEST_F(NSFFileTest, Header_Song_Name) {
    data[0x0E] = 'T';
    data[0x0F] = 'E';
    data[0x10] = 'S';
    data[0x11] = 'T';
    data[0x12] = '\0';
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ("TEST", header.SongName);
}

TEST_F(NSFFileTest, Header_Invalid_Song_Name) {
    for (int i = 0; i < NSF_HEADER_TEXT_FIELD_SIZE; ++i) {
        data[0x0E + i] = 'x';
    }
    std::istringstream iss(data);
    EXPECT_THROW(NsfFile::HeaderDesc header(iss), invalid_format);
}

TEST_F(NSFFileTest, Header_Artist_Name) {
    data[0x2E] = 'T';
    data[0x2F] = 'E';
    data[0x30] = 'S';
    data[0x31] = 'T';
    data[0x32] = '\0';
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ("TEST", header.ArtistName);
}

TEST_F(NSFFileTest, Header_Invalid_Artist_Name) {
    for (int i = 0; i < NSF_HEADER_TEXT_FIELD_SIZE; ++i) {
        data[0x2E + i] = 'x';
    }
    std::istringstream iss(data);
    EXPECT_THROW(NsfFile::HeaderDesc header(iss), invalid_format);
}

TEST_F(NSFFileTest, Header_Copyright_Holder_Name) {
    data[0x4E] = 'T';
    data[0x4F] = 'E';
    data[0x50] = 'S';
    data[0x51] = 'T';
    data[0x52] = '\0';
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ("TEST", header.CopyrightHolderName);
}

TEST_F(NSFFileTest, Header_Invalid_Copyright_Holder_Name) {
    for (int i = 0; i < NSF_HEADER_TEXT_FIELD_SIZE; ++i) {
        data[0x4E + i] = 'x';
    }
    std::istringstream iss(data);
    EXPECT_THROW(NsfFile::HeaderDesc header(iss), invalid_format);
}

TEST_F(NSFFileTest, Header_Period_NTSC) {
    data[0x6E] = 0x34;
    data[0x6F] = 0x12;
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ(0x1234, header.PeriodNTSC);
}

TEST_F(NSFFileTest, Header_No_Bankswitching) {
    for (int i = 0; i < NSF_BANK_COUNT; ++i) {
        data[0x70 + i] = 0;
    }
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_FALSE(header.UsesBankswitching);
    for (int i = 0; i < NSF_BANK_COUNT; ++i) {
        EXPECT_EQ(0, header.InitialBank[i]);
    }
}

TEST_F(NSFFileTest, Header_Has_Bankswitching) {
    for (int i = 0; i < NSF_BANK_COUNT; ++i) {
        data[0x70 + i] = i;
    }
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_TRUE(header.UsesBankswitching);
    for (int i = 0; i < NSF_BANK_COUNT; ++i) {
        EXPECT_EQ(i, header.InitialBank[i]);
    }
}

TEST_F(NSFFileTest, Header_Period_PAL) {
    data[0x78] = 0x34;
    data[0x79] = 0x12;
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ(0x1234, header.PeriodPAL);
}

TEST_F(NSFFileTest, Header_NTSC_PAL_Support) {
    typedef NsfFile::HeaderDesc::LocaleBits Bits;
    {
        data[0x7A] = Mask<Bits::PAL>(0) | Mask<Bits::Dual>(0);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_TRUE(header.SupportsNTSC);
        EXPECT_FALSE(header.SupportsPAL);
    }
    {
        data[0x7A] = Mask<Bits::PAL>(1) | Mask<Bits::Dual>(0);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_FALSE(header.SupportsNTSC);
        EXPECT_TRUE(header.SupportsPAL);
    }
    {
        data[0x7A] = Mask<Bits::PAL>(0) | Mask<Bits::Dual>(1);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_TRUE(header.SupportsNTSC);
        EXPECT_TRUE(header.SupportsPAL);
    }
    {
        data[0x7A] = Mask<Bits::PAL>(1) | Mask<Bits::Dual>(1);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_TRUE(header.SupportsNTSC);
        EXPECT_TRUE(header.SupportsPAL);
    }
    {
        // Any other bit is supposed to be 0
        data[0x7A] = ~(Mask<Bits::PAL>(1) | Mask<Bits::Dual>(1));
        std::istringstream iss(data);
        EXPECT_THROW(NsfFile::HeaderDesc header(iss), invalid_format);
    }
}

TEST_F(NSFFileTest, Header_Extra_Hardware_Usage) {
    typedef NsfFile::HeaderDesc::ExtraHardwareBits Bits;
    {
        data[0x7B] = 0;
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_FALSE(header.UsesVRC6);
        EXPECT_FALSE(header.UsesVRC7);
        EXPECT_FALSE(header.UsesFDS);
        EXPECT_FALSE(header.UsesMMC5);
        EXPECT_FALSE(header.UsesNamco163);
        EXPECT_FALSE(header.UsesSunsoft5B);
        EXPECT_FALSE(header.UsesVT02Plus);
    }
    {
        data[0x7B] = Mask<Bits::VRC6>(1);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_TRUE(header.UsesVRC6);
    }
    {
        data[0x7B] = Mask<Bits::VRC7>(1);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_TRUE(header.UsesVRC7);
    }
    {
        data[0x7B] = Mask<Bits::FDS>(1);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_TRUE(header.UsesFDS);
    }
    {
        data[0x7B] = Mask<Bits::MMC5>(1);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_TRUE(header.UsesMMC5);
    }
    {
        data[0x7B] = Mask<Bits::Namco163>(1);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_TRUE(header.UsesNamco163);
    }
    {
        data[0x7B] = Mask<Bits::Sunsoft5B>(1);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_TRUE(header.UsesSunsoft5B);
    }
    {
        data[0x7B] = Mask<Bits::VT02Plus>(1);
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        EXPECT_TRUE(header.UsesVT02Plus);
    }
    {
        // Any other bit is supposed to be 0
        data[0x7A] = Mask<Bits::Unused>(1);
        std::istringstream iss(data);
        EXPECT_THROW(NsfFile::HeaderDesc header(iss), invalid_format);
    }
}

TEST_F(NSFFileTest, Header_Data_Size) {
    data[0x7D] = 0x56;
    data[0x7E] = 0x34;
    data[0x7F] = 0x12;
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ(0x123456, header.DataSize);
}

TEST_F(NSFFileTest, NsfFile_Rejected) {
    auto GetValidHeader = [this]() {
        std::istringstream iss(data);
        NsfFile::HeaderDesc header(iss);
        return header;
    };
    EXPECT_NO_THROW(NsfFile::Validate(GetValidHeader()));
    {
        auto header = GetValidHeader();
        header.Version = 2;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
    {
        auto header = GetValidHeader();
        header.SongCount = 0;
        EXPECT_THROW(NsfFile::Validate(header), invalid_format);
    }
    {
        auto header = GetValidHeader();
        header.FirstSong = 1;
        EXPECT_THROW(NsfFile::Validate(header), invalid_format);
    }
    {
        auto header = GetValidHeader();
        header.LoadAddress = 0x7FFF;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
    {
        auto header = GetValidHeader();
        header.InitAddress = 0x7FFF;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
    {
        auto header = GetValidHeader();
        header.PlayAddress = 0x7FFF;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
    {
        auto header = GetValidHeader();
        header.PeriodNTSC = 0;
        header.SupportsNTSC = true;
        EXPECT_THROW(NsfFile::Validate(header), invalid_format);
        header.SupportsNTSC = false;
        EXPECT_NO_THROW(NsfFile::Validate(header));
    }
    {
        auto header = GetValidHeader();
        header.PeriodPAL = 0;
        header.SupportsPAL = true;
        EXPECT_THROW(NsfFile::Validate(header), invalid_format);
        header.SupportsPAL = false;
        EXPECT_NO_THROW(NsfFile::Validate(header));
    }
    {
        auto header = GetValidHeader();
        header.UsesVRC6 = true;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
    {
        auto header = GetValidHeader();
        header.UsesVRC7 = true;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
    {
        auto header = GetValidHeader();
        header.UsesFDS = true;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
    {
        auto header = GetValidHeader();
        header.UsesMMC5 = true;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
    {
        auto header = GetValidHeader();
        header.UsesNamco163 = true;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
    {
        auto header = GetValidHeader();
        header.UsesSunsoft5B = true;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
    {
        auto header = GetValidHeader();
        header.UsesVT02Plus = true;
        EXPECT_THROW(NsfFile::Validate(header), unsupported_format);
    }
}

TEST_F(NSFFileTest, NsfFile_TooSmall) {
    std::string file = data + std::string(0x1000, 0x00);
    {
        // 256 bytes data in a 4K file
        file[0x7E] = 0x01;
        std::istringstream iss(file);
        EXPECT_NO_THROW(NsfFile nsf(iss));
    }
    {
        // 8K data in a 4K file
        file[0x7E] = 0x20;
        std::istringstream iss(file);
        EXPECT_THROW(NsfFile nsf(iss), invalid_format);
    }
}

TEST_F(NSFFileTest, NsfFile_TooLarge) {
    // Trying to read 1 MiB of NSF data without bankswitching
    std::string file = data + std::string(0x100000, 0x00);
    {
        std::istringstream iss(file);
        EXPECT_THROW(NsfFile nsf(iss), invalid_format);
    }
    {
        file[0x7F] = 0x10;
        std::istringstream iss(file);
        EXPECT_THROW(NsfFile nsf(iss), invalid_format);
    }
}

TEST_F(NSFFileTest, NsfFile_NsfData) {
    std::string file = data + std::string(0x1000, 0x00);
    {
        // available data in a 4K file
        std::istringstream iss(file);
        NsfFile nsf(iss);
        EXPECT_EQ(0x1000, nsf.NsfData.size());
    }
    {
        // 256 bytes data in a 4K file
        file[0x7E] = 0x01;
        std::istringstream iss(file);
        NsfFile nsf(iss);
        EXPECT_EQ(0x100, nsf.NsfData.size());
    }
}

TEST_F(NSFFileTest, Header_Play_Address_Bug_0) {
    data[0x0C] = 0xB4;
    data[0x0D] = 0xB3;
    std::istringstream iss(data);
    NsfFile::HeaderDesc header(iss);
    EXPECT_EQ(0xB3B4, header.PlayAddress);
}

TEST_F(NSFFileTest, NsfFile_Banked_NotTooLarge) {
    // Trying to read 1 MiB of NSF data without bankswitching
    for (int i = 0; i < NSF_BANK_COUNT; ++i) {
        data[0x70 + i] = i;
    }
    std::string file = data + std::string(0x100000, 0x00);
    {
        // Read all 1M
        std::istringstream iss(file);
        NsfFile nsf(iss);
        EXPECT_TRUE(nsf.Header.UsesBankswitching);
        EXPECT_EQ(0x100000, nsf.NsfData.size());
    }
    {
        // Read first 512K
        file[0x7F] = 0x08;
        std::istringstream iss(file);
        NsfFile nsf(iss);
        EXPECT_TRUE(nsf.Header.UsesBankswitching);
        EXPECT_EQ(0x080000, nsf.NsfData.size());
    }
}

