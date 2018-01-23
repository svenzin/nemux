#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MemoryMap.h"

#include <functional>
#include <typeinfo>

struct MonitoredPalette {
    MOCK_CONST_METHOD1(ReadAt, Byte(const Byte address));
    MOCK_METHOD2(WriteAt, void(const Byte Address, const Byte value));
};

struct MonitoredMapper : public MemoryMap {
    MOCK_CONST_METHOD1(GetByteAt, Byte(const Word address));
    MOCK_METHOD2(SetByteAt, void(const Word address, const Byte value));
};

struct PpuMemoryMapTest : public ::testing::Test {
    MonitoredPalette palette;
    MonitoredMapper mapper;
    PpuMemoryMap<MonitoredPalette> ppumap;

    PpuMemoryMapTest() : ppumap(&palette, &mapper) {}
};

TEST_F(PpuMemoryMapTest, Palette_Get) {
    {
        EXPECT_CALL(palette, ReadAt(0x00));
        ppumap.GetByteAt(0x3F00);
    }
    {
        EXPECT_CALL(palette, ReadAt(0xFF));
        ppumap.GetByteAt(0x3FFF);
    }
}

TEST_F(PpuMemoryMapTest, Palette_Set) {
    {
        EXPECT_CALL(palette, WriteAt(0x00, 0));
        ppumap.SetByteAt(0x3F00, 0);
    }
    {
        EXPECT_CALL(palette, WriteAt(0xFF, 0));
        ppumap.SetByteAt(0x3FFF, 0);
    }
}

TEST_F(PpuMemoryMapTest, Mapper_Get) {
    {
        EXPECT_CALL(mapper, GetByteAt(0x0000));
        ppumap.GetByteAt(0x0000);
    }
    {
        EXPECT_CALL(mapper, GetByteAt(0x3EFF));
        ppumap.GetByteAt(0x3EFF);
    }
}

TEST_F(PpuMemoryMapTest, Mapper_Set) {
    {
        EXPECT_CALL(mapper, SetByteAt(0x0000, 0));
        ppumap.SetByteAt(0x0000, 0);
    }
    {
        EXPECT_CALL(mapper, SetByteAt(0x3EFF, 0));
        ppumap.SetByteAt(0x3EFF, 0);
    }
}
