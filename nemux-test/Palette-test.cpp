#include "gtest/gtest.h"

#include "Palette.h"

using namespace std;

////////////////////////////////////////////////////////////////

struct PaletteTest : public ::testing::Test {
    Palette palette;
};

TEST_F(PaletteTest, ReadWrite) {
    palette.WriteAt(0x01, 0xBE);
    palette.WriteAt(0x02, 0xEF);

    EXPECT_EQ(0xBE, palette.ReadAt(0x01));
    EXPECT_EQ(0xEF, palette.ReadAt(0x02));
}

TEST_F(PaletteTest, Write_Mirroring0x20) {
    palette.WriteAt(0x21, 0x89);
    palette.WriteAt(0x42, 0x9A);
    palette.WriteAt(0x63, 0xAB);
    palette.WriteAt(0x84, 0xBC);
    palette.WriteAt(0xA5, 0xCD);
    palette.WriteAt(0xC6, 0xDE);
    palette.WriteAt(0xE7, 0xEF);

    EXPECT_EQ(0x89, palette.ReadAt(0x01));
    EXPECT_EQ(0x9A, palette.ReadAt(0x02));
    EXPECT_EQ(0xAB, palette.ReadAt(0x03));
    EXPECT_EQ(0xBC, palette.ReadAt(0x04));
    EXPECT_EQ(0xCD, palette.ReadAt(0x05));
    EXPECT_EQ(0xDE, palette.ReadAt(0x06));
    EXPECT_EQ(0xEF, palette.ReadAt(0x07));
}

TEST_F(PaletteTest, Read_Mirroring0x20) {
    palette.WriteAt(0x01, 0x89);

    EXPECT_EQ(0x89, palette.ReadAt(0x21));
    EXPECT_EQ(0x89, palette.ReadAt(0x41));
    EXPECT_EQ(0x89, palette.ReadAt(0x61));
    EXPECT_EQ(0x89, palette.ReadAt(0x81));
    EXPECT_EQ(0x89, palette.ReadAt(0xA1));
    EXPECT_EQ(0x89, palette.ReadAt(0xC1));
    EXPECT_EQ(0x89, palette.ReadAt(0xE1));
}

TEST_F(PaletteTest, Write_MirroringSpriteColor0) {
    palette.WriteAt(0x10, 0xBE);
    EXPECT_EQ(0xBE, palette.ReadAt(0x00));

    palette.WriteAt(0x14, 0xBE);
    EXPECT_EQ(0xBE, palette.ReadAt(0x04));

    palette.WriteAt(0x18, 0xBE);
    EXPECT_EQ(0xBE, palette.ReadAt(0x08));

    palette.WriteAt(0x1C, 0xBE);
    EXPECT_EQ(0xBE, palette.ReadAt(0x0C));
}

TEST_F(PaletteTest, Read_MirroringSpriteColor0) {
    palette.WriteAt(0x00, 0xBE);
    EXPECT_EQ(0xBE, palette.ReadAt(0x10));

    palette.WriteAt(0x04, 0xBE);
    EXPECT_EQ(0xBE, palette.ReadAt(0x14));

    palette.WriteAt(0x08, 0xBE);
    EXPECT_EQ(0xBE, palette.ReadAt(0x18));

    palette.WriteAt(0x0C, 0xBE);
    EXPECT_EQ(0xBE, palette.ReadAt(0x1C));
}
