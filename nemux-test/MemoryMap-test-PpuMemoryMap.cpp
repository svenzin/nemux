#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MemoryMap.h"

#include <functional>
#include <typeinfo>

using testing::_;

struct MonitoredPalette {
    MOCK_CONST_METHOD1(ReadAt, Byte(const Byte address));
    MOCK_METHOD2(WriteAt, void(const Byte Address, const Byte value));
};

struct MonitoredNesMapper : public NesMapper {
    MOCK_CONST_METHOD1(NametableAddress, Word(const Word address));
    MOCK_CONST_METHOD1(GetCpuAt, Byte(const Word address));
    MOCK_METHOD2(SetCpuAt, void(const Word address, const Byte value));
    MOCK_CONST_METHOD1(GetPpuAt, Byte(const Word address));
    MOCK_METHOD2(SetPpuAt, void(const Word address, const Byte value));
};

struct PpuMemoryMapTest : public ::testing::Test {
    MonitoredPalette palette;
    MonitoredNesMapper mapper;
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

TEST_F(PpuMemoryMapTest, Mapper_GetPattern) {
    {
        EXPECT_CALL(mapper, GetPpuAt(0x0000));
        ppumap.GetByteAt(0x0000);
    }
    {
        EXPECT_CALL(mapper, GetPpuAt(0x1FFF));
        ppumap.GetByteAt(0x1FFF);
    }
}

TEST_F(PpuMemoryMapTest, Mapper_SetPattern) {
    {
        EXPECT_CALL(mapper, SetPpuAt(0x0000, 0));
        ppumap.SetByteAt(0x0000, 0);
    }
    {
        EXPECT_CALL(mapper, SetPpuAt(0x1FFF, 0));
        ppumap.SetByteAt(0x1FFF, 0);
    }
}

// Nametables will be limited to VRAM for now, no routing through the mapper yet
// Instead offer only the address mirroring in the mapper
TEST_F(PpuMemoryMapTest, Mapper_GetNametable) {
    {
        EXPECT_CALL(mapper, GetPpuAt(_)).Times(0);
        ppumap.GetByteAt(0x2000);
    }
    {
        EXPECT_CALL(mapper, GetPpuAt(_)).Times(0);
        ppumap.GetByteAt(0x2FFF);
    }
    {
        EXPECT_CALL(mapper, NametableAddress(0x2000));
        ppumap.GetByteAt(0x2000);
    }
}

TEST_F(PpuMemoryMapTest, Mapper_SetNametable) {
    {
        EXPECT_CALL(mapper, SetPpuAt(_, _)).Times(0);
        ppumap.SetByteAt(0x2000, 0);
    }
    {
        EXPECT_CALL(mapper, SetPpuAt(_, _)).Times(0);
        ppumap.SetByteAt(0x2FFF, 0);
    }
    {
        EXPECT_CALL(mapper, NametableAddress(0x2000));
        ppumap.SetByteAt(0x2000, 0);
    }
}
