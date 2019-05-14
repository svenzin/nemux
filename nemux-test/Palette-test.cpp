#include "gtest/gtest.h"

#include "Palette.h"

using namespace std;

////////////////////////////////////////////////////////////////

struct PaletteTest : public ::testing::Test {
    Palette palette;
};

TEST_F(PaletteTest, ReadWrite) {
    palette.WriteAt(0x01, 0x1E);
    palette.WriteAt(0x02, 0x2F);

    EXPECT_EQ(0x1E, palette.ReadAt(0x01));
    EXPECT_EQ(0x2F, palette.ReadAt(0x02));
}

TEST_F(PaletteTest, Write_Mirroring0x20) {
    palette.WriteAt(0x21, 0x09);
    palette.WriteAt(0x42, 0x1A);
    palette.WriteAt(0x63, 0x1B);
    palette.WriteAt(0x84, 0x2C);
    palette.WriteAt(0xA5, 0x2D);
    palette.WriteAt(0xC6, 0x3E);
    palette.WriteAt(0xE7, 0x3F);

    EXPECT_EQ(0x09, palette.ReadAt(0x01));
    EXPECT_EQ(0x1A, palette.ReadAt(0x02));
    EXPECT_EQ(0x1B, palette.ReadAt(0x03));
    EXPECT_EQ(0x2C, palette.ReadAt(0x04));
    EXPECT_EQ(0x2D, palette.ReadAt(0x05));
    EXPECT_EQ(0x3E, palette.ReadAt(0x06));
    EXPECT_EQ(0x3F, palette.ReadAt(0x07));
}

TEST_F(PaletteTest, Read_Mirroring0x20) {
    palette.WriteAt(0x01, 0x19);

    EXPECT_EQ(0x19, palette.ReadAt(0x21));
    EXPECT_EQ(0x19, palette.ReadAt(0x41));
    EXPECT_EQ(0x19, palette.ReadAt(0x61));
    EXPECT_EQ(0x19, palette.ReadAt(0x81));
    EXPECT_EQ(0x19, palette.ReadAt(0xA1));
    EXPECT_EQ(0x19, palette.ReadAt(0xC1));
    EXPECT_EQ(0x19, palette.ReadAt(0xE1));
}

TEST_F(PaletteTest, Write_MirroringSpriteColor0) {
    palette.WriteAt(0x10, 0x1E);
    EXPECT_EQ(0x1E, palette.ReadAt(0x00));

    palette.WriteAt(0x14, 0x1E);
    EXPECT_EQ(0x1E, palette.ReadAt(0x04));

    palette.WriteAt(0x18, 0x1E);
    EXPECT_EQ(0x1E, palette.ReadAt(0x08));

    palette.WriteAt(0x1C, 0x1E);
    EXPECT_EQ(0x1E, palette.ReadAt(0x0C));
}

TEST_F(PaletteTest, Read_MirroringSpriteColor0) {
    palette.WriteAt(0x00, 0x1E);
    EXPECT_EQ(0x1E, palette.ReadAt(0x10));

    palette.WriteAt(0x04, 0x1E);
    EXPECT_EQ(0x1E, palette.ReadAt(0x14));

    palette.WriteAt(0x08, 0x1E);
    EXPECT_EQ(0x1E, palette.ReadAt(0x18));

    palette.WriteAt(0x0C, 0x1E);
    EXPECT_EQ(0x1E, palette.ReadAt(0x1C));
}

TEST_F(PaletteTest, Read_Only6Bits) {
    palette.WriteAt(0x00, 0xFF);
    EXPECT_EQ(0x3F, palette.ReadAt(0x10));
}
