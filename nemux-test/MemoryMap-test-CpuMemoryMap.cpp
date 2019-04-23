#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MemoryMap.h"

#include <functional>
#include <typeinfo>

struct MonitoredPpu {
    MOCK_METHOD1(WriteControl1, void(Byte value));
    MOCK_METHOD1(WriteControl2, void(Byte value));
    MOCK_METHOD0(ReadStatus, Byte());
    MOCK_METHOD1(WriteOAMAddress, void(Byte value));
    MOCK_METHOD0(ReadOAMData, Byte());
    MOCK_METHOD1(WriteOAMData, void(Byte value));
    MOCK_METHOD1(WriteScroll, void(Byte value));
    MOCK_METHOD1(WriteAddress, void(Byte value));
    MOCK_METHOD0(ReadData, Byte());
    MOCK_METHOD1(WriteData, void(Byte value));
};

struct MonitoredNesMapper : public NesMapper {
    MOCK_CONST_METHOD1(GetCpuAt, Byte(const Word address));
    MOCK_METHOD2(SetCpuAt, void(const Word address, const Byte value));
    MOCK_CONST_METHOD1(GetPpuAt, Byte(const Word address));
    MOCK_METHOD2(SetPpuAt, void(const Word address, const Byte value));
};

struct CpuMemoryMapTest : public ::testing::Test {
    MonitoredPpu ppu;
    MonitoredNesMapper mapper;
    CpuMemoryMap<MonitoredPpu> cpumap;

    CpuMemoryMapTest() : cpumap(&ppu, &mapper) {}
};

TEST_F(CpuMemoryMapTest, RAM_ReadMirroring) {
    cpumap.SetByteAt(0x0000, 0xBE);
    EXPECT_EQ(0xBE, cpumap.GetByteAt(0x0000));
    EXPECT_EQ(0xBE, cpumap.GetByteAt(0x0800));
    EXPECT_EQ(0xBE, cpumap.GetByteAt(0x1000));
    EXPECT_EQ(0xBE, cpumap.GetByteAt(0x1800));
    
    cpumap.SetByteAt(0x07FF, 0xEF);
    EXPECT_EQ(0xEF, cpumap.GetByteAt(0x07FF));
    EXPECT_EQ(0xEF, cpumap.GetByteAt(0x0FFF));
    EXPECT_EQ(0xEF, cpumap.GetByteAt(0x17FF));
    EXPECT_EQ(0xEF, cpumap.GetByteAt(0x1FFF));
}

TEST_F(CpuMemoryMapTest, RAM_WriteMirroring) {
    cpumap.SetByteAt(0x0000, 0x0B);
    cpumap.SetByteAt(0x0801, 0xAD);
    cpumap.SetByteAt(0x1002, 0xBE);
    cpumap.SetByteAt(0x1803, 0xEF);
    EXPECT_EQ(0x0B, cpumap.GetByteAt(0x0000));
    EXPECT_EQ(0xAD, cpumap.GetByteAt(0x0001));
    EXPECT_EQ(0xBE, cpumap.GetByteAt(0x0002));
    EXPECT_EQ(0xEF, cpumap.GetByteAt(0x0003));
}

TEST_F(CpuMemoryMapTest, PPU_Registers) {
    {
        EXPECT_CALL(ppu, WriteControl1(0));
        cpumap.SetByteAt(0x2000, 0x00);
    }
    {
        EXPECT_CALL(ppu, WriteControl2(0));
        cpumap.SetByteAt(0x2001, 0x00);
    }
    {
        EXPECT_CALL(ppu, ReadStatus());
        cpumap.GetByteAt(0x2002);
    }
    {
        EXPECT_CALL(ppu, WriteOAMAddress(0));
        cpumap.SetByteAt(0x2003, 0x00);
    }
    {
        EXPECT_CALL(ppu, ReadOAMData());
        cpumap.GetByteAt(0x2004);
    }
    {
        EXPECT_CALL(ppu, WriteOAMData(0));
        cpumap.SetByteAt(0x2004, 0x00);
    }
    {
        EXPECT_CALL(ppu, WriteScroll(0));
        cpumap.SetByteAt(0x2005, 0x00);
    }
    {
        EXPECT_CALL(ppu, WriteAddress(0));
        cpumap.SetByteAt(0x2006, 0x00);
    }
    {
        EXPECT_CALL(ppu, ReadData());
        cpumap.GetByteAt(0x2007);
    }
    {
        EXPECT_CALL(ppu, WriteData(0));
        cpumap.SetByteAt(0x2007, 0x00);
    }
}

TEST_F(CpuMemoryMapTest, PPU_MirroredRegisters) {
    for (Word base = 0x2000; base < 0x4000; base += 0x0008) {
        {
            EXPECT_CALL(ppu, WriteControl1(0));
            cpumap.SetByteAt(base, 0x00);
        }
        {
            EXPECT_CALL(ppu, WriteControl2(0));
            cpumap.SetByteAt(base + 1, 0x00);
        }
        {
            EXPECT_CALL(ppu, ReadStatus());
            cpumap.GetByteAt(base + 2);
        }
        {
            EXPECT_CALL(ppu, WriteOAMAddress(0));
            cpumap.SetByteAt(base + 3, 0x00);
        }
        {
            EXPECT_CALL(ppu, ReadOAMData());
            cpumap.GetByteAt(base + 4);
        }
        {
            EXPECT_CALL(ppu, WriteOAMData(0));
            cpumap.SetByteAt(base + 4, 0x00);
        }
        {
            EXPECT_CALL(ppu, WriteScroll(0));
            cpumap.SetByteAt(base + 5, 0x00);
        }
        {
            EXPECT_CALL(ppu, WriteAddress(0));
            cpumap.SetByteAt(base + 6, 0x00);
        }
        {
            EXPECT_CALL(ppu, ReadData());
            cpumap.GetByteAt(base + 7);
        }
        {
            EXPECT_CALL(ppu, WriteData(0));
            cpumap.SetByteAt(base + 7, 0x00);
        }
    }
}

TEST_F(CpuMemoryMapTest, Mapper_Get) {
    {
        EXPECT_CALL(mapper, GetCpuAt(0x4020));
        cpumap.GetByteAt(0x4020);
    }
    {
        EXPECT_CALL(mapper, GetCpuAt(0xFFFF));
        cpumap.GetByteAt(0xFFFF);
    }
}

TEST_F(CpuMemoryMapTest, Mapper_Set) {
    {
        EXPECT_CALL(mapper, SetCpuAt(0x4020, 0x00));
        cpumap.SetByteAt(0x4020, 0x00);
    }
    {
        EXPECT_CALL(mapper, SetCpuAt(0xFFFF, 0x00));
        cpumap.SetByteAt(0xFFFF, 0x00);
    }
}
