#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MemoryMap.h"

#include "Apu.h"
#include "Ppu.h"
#include "Controllers.h"

#include <functional>
#include <typeinfo>

struct MonitoredCpu {
    MOCK_METHOD3(DMA, void(const Byte page,
                           std::array<Byte, 0x0100> & target,
                           const Byte offset)
    );
};

struct MonitoredApu : public Apu {
    MOCK_CONST_METHOD0(ReadStatus, Byte());

    MOCK_METHOD1(WritePulse1Control, void(const Byte value));
    MOCK_METHOD1(WritePulse1Sweep, void(const Byte value));
    MOCK_METHOD1(WritePulse1PeriodLo, void(const Byte value));
    MOCK_METHOD1(WritePulse1PeriodHi, void(const Byte value));
    
    MOCK_METHOD1(WritePulse2Control, void(const Byte value));
    MOCK_METHOD1(WritePulse2Sweep, void(const Byte value));
    MOCK_METHOD1(WritePulse2PeriodLo, void(const Byte value));
    MOCK_METHOD1(WritePulse2PeriodHi, void(const Byte value));
    
    MOCK_METHOD1(WriteTriangleControl, void(const Byte value));
    MOCK_METHOD1(WriteTrianglePeriodLo, void(const Byte value));
    MOCK_METHOD1(WriteTrianglePeriodHi, void(const Byte value));
    
    MOCK_METHOD1(WriteNoiseControl, void(const Byte value));
    MOCK_METHOD1(WriteNoisePeriod, void(const Byte value));
    MOCK_METHOD1(WriteNoiseLength, void(const Byte value));
    
    MOCK_METHOD1(WriteDMCFrequency, void(const Byte value));
    MOCK_METHOD1(WriteDMCDAC, void(const Byte value));
    MOCK_METHOD1(WriteDMCAddress, void(const Byte value));
    MOCK_METHOD1(WriteDMCLength, void(const Byte value));
    
    MOCK_METHOD1(WriteCommonEnable, void(const Byte value));
    MOCK_METHOD1(WriteCommonControl, void(const Byte value));
};

struct MonitoredPpu : public Ppu {
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
    MOCK_CONST_METHOD1(NametableAddress, Word(const Word address));
    MOCK_CONST_METHOD1(GetCpuAt, Byte(const Word address));
    MOCK_METHOD2(SetCpuAt, void(const Word address, const Byte value));
    MOCK_CONST_METHOD1(GetPpuAt, Byte(const Word address));
    MOCK_METHOD2(SetPpuAt, void(const Word address, const Byte value));
};

struct MonitoredControllers : public Controllers {
    MOCK_METHOD0(ReadP1, Byte());
    MOCK_METHOD0(ReadP2, Byte());
    MOCK_METHOD1(Write, void(const Byte value));
};

struct CpuMemoryMapTest : public ::testing::Test {
    MonitoredCpu cpu;
    MonitoredApu apu;
    MonitoredPpu ppu;
    MonitoredNesMapper mapper;
    MonitoredControllers controllers;
    CpuMemoryMap<MonitoredCpu, MonitoredPpu, MonitoredControllers, MonitoredApu> cpumap;

    CpuMemoryMapTest() : cpumap(&cpu, &apu, &ppu, &mapper, &controllers) {}
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

TEST_F(CpuMemoryMapTest, CPU_OAMDMA) {
    ppu.OAMAddress = 0x04;
    EXPECT_CALL(cpu, DMA(0x02, ppu.SprRam, 0x04));
    cpumap.SetByteAt(0x4014, 0x02);
}

TEST_F(CpuMemoryMapTest, Controllers_ReadWrite) {
    {
        EXPECT_CALL(controllers, Write(0x1E));
        cpumap.SetByteAt(0x4016, 0x1E);
    }
    {
        EXPECT_CALL(controllers, ReadP1());
        cpumap.GetByteAt(0x4016);
    }
    {
        EXPECT_CALL(controllers, ReadP2());
        cpumap.GetByteAt(0x4017);
    }
}

TEST_F(CpuMemoryMapTest, APU_Registers) {
    // Pulse 1
    {
        EXPECT_CALL(apu, WritePulse1Control(0));
        cpumap.SetByteAt(0x4000, 0x00);
    }
    {
        EXPECT_CALL(apu, WritePulse1Sweep(0));
        cpumap.SetByteAt(0x4001, 0x00);
    }
    {
        EXPECT_CALL(apu, WritePulse1PeriodLo(0));
        cpumap.SetByteAt(0x4002, 0x00);
    }
    {
        EXPECT_CALL(apu, WritePulse1PeriodHi(0));
        cpumap.SetByteAt(0x4003, 0x00);
    }
    // Pulse 2
    {
        EXPECT_CALL(apu, WritePulse2Control(0));
        cpumap.SetByteAt(0x4004, 0x00);
    }
    {
        EXPECT_CALL(apu, WritePulse2Sweep(0));
        cpumap.SetByteAt(0x4005, 0x00);
    }
    {
        EXPECT_CALL(apu, WritePulse2PeriodLo(0));
        cpumap.SetByteAt(0x4006, 0x00);
    }
    {
        EXPECT_CALL(apu, WritePulse2PeriodHi(0));
        cpumap.SetByteAt(0x4007, 0x00);
    }
    // Triangle
    {
        EXPECT_CALL(apu, WriteTriangleControl(0));
        cpumap.SetByteAt(0x4008, 0x00);
    }
    {
        EXPECT_CALL(apu, WriteTrianglePeriodLo(0));
        cpumap.SetByteAt(0x400A, 0x00);
    }
    {
        EXPECT_CALL(apu, WriteTrianglePeriodHi(0));
        cpumap.SetByteAt(0x400B, 0x00);
    }
    // Noise
    {
        EXPECT_CALL(apu, WriteNoiseControl(0));
        cpumap.SetByteAt(0x400C, 0x00);
    }
    {
        EXPECT_CALL(apu, WriteNoisePeriod(0));
        cpumap.SetByteAt(0x400E, 0x00);
    }
    {
        EXPECT_CALL(apu, WriteNoiseLength(0));
        cpumap.SetByteAt(0x400F, 0x00);
    }
    // DMC
    {
        EXPECT_CALL(apu, WriteDMCFrequency(0));
        cpumap.SetByteAt(0x4010, 0x00);
    }
    {
        EXPECT_CALL(apu, WriteDMCDAC(0));
        cpumap.SetByteAt(0x4011, 0x00);
    }
    {
        EXPECT_CALL(apu, WriteDMCAddress(0));
        cpumap.SetByteAt(0x4012, 0x00);
    }
    {
        EXPECT_CALL(apu, WriteDMCLength(0));
        cpumap.SetByteAt(0x4013, 0x00);
    }
    // Common
    {
        EXPECT_CALL(apu, WriteCommonEnable(0));
        cpumap.SetByteAt(0x4015, 0x00);
    }
    {
        EXPECT_CALL(apu, WriteCommonControl(0));
        cpumap.SetByteAt(0x4017, 0x00);
    }
    // Status
    {
        EXPECT_CALL(apu, ReadStatus());
        cpumap.GetByteAt(0x4015);
    }
}
