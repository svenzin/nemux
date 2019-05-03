#ifndef MEMORY_MAP_H_
#define MEMORY_MAP_H_

#include "Types.h"
//#include "Ppu.h"
#include "Palette.h"
#include "Mapper.h"
#include "Apu.h"

#include <array>

struct MemoryMap {
    virtual ~MemoryMap() {}

    virtual Byte GetByteAt(const Word address) const = 0;
    virtual void SetByteAt(const Word address, const Byte value) = 0;
};

template <std::size_t Size>
class MemoryBlock : public MemoryMap {
    std::array<Byte, Size> Data;

public:
    ~MemoryBlock() override {}

    Byte GetByteAt(const Word address) const override {
        return Data[address];
    }

    void SetByteAt(const Word address, const Byte value) override {
        Data[address] = value;
    }
};

template <class Cpu_t, class Ppu_t, class Controllers_t, class Apu_t>
class CpuMemoryMap : public MemoryMap {
public:
    std::array<Byte, 0x0800> RAM;
    Cpu_t * CPU;
    Ppu_t * PPU;
    NesMapper * Mapper;
    Controllers_t * Controllers;
    Apu_t * APU;
    
    CpuMemoryMap(Cpu_t * cpu, Apu_t * apu, Ppu_t * ppu, NesMapper * mapper, Controllers_t * controllers)
        : CPU(cpu), APU(apu), PPU(ppu), Mapper(mapper), Controllers(controllers)
    {}

    ~CpuMemoryMap() override {}

    Byte GetByteAt(const Word address) const override {
        if (address < 0x2000) {
            return RAM[address & 0x07FF];
        } else if (address < 0x4000) {
            const auto addr = address & 0x2007;
            if (addr == 0x2002) return PPU->ReadStatus();
            if (addr == 0x2004) return PPU->ReadOAMData();
            if (addr == 0x2007) return PPU->ReadData();
        } else if (address < 0x4020) {
            if (address == 0x4016) return Controllers->ReadP1();
            if (address == 0x4017) return Controllers->ReadP2();
            if (address == 0x4015) return APU->ReadStatus();
            return 0;
        } else {
            return Mapper->GetCpuAt(address);
        }
    }

    void SetByteAt(const Word address, const Byte value) override {
        if (address < 0x2000) {
            RAM[address & 0x07FF] = value;
        } else if (address < 0x4000) {
            const auto addr = address & 0x2007;
            if (addr == 0x2000) PPU->WriteControl1(value);
            if (addr == 0x2001) PPU->WriteControl2(value);
            if (addr == 0x2003) PPU->WriteOAMAddress(value);
            if (addr == 0x2004) PPU->WriteOAMData(value);
            if (addr == 0x2005) PPU->WriteScroll(value);
            if (addr == 0x2006) PPU->WriteAddress(value);
            if (addr == 0x2007) PPU->WriteData(value);
        } else if (address < 0x4020) {
            if (address == 0x4014) CPU->DMA(value, PPU->SprRam, PPU->OAMAddress);
            if (address == 0x4016) Controllers->Write(value);
            if (address == 0x4000) APU->WritePulse1Control(value);
            if (address == 0x4001) APU->WritePulse1Sweep(value);
            if (address == 0x4002) APU->WritePulse1PeriodLo(value);
            if (address == 0x4003) APU->WritePulse1PeriodHi(value);
            if (address == 0x4004) APU->WritePulse2Control(value);
            if (address == 0x4005) APU->WritePulse2Sweep(value);
            if (address == 0x4006) APU->WritePulse2PeriodLo(value);
            if (address == 0x4007) APU->WritePulse2PeriodHi(value);
            if (address == 0x4008) APU->WriteTriangleControl(value);
            if (address == 0x400A) APU->WriteTrianglePeriodLo(value);
            if (address == 0x400B) APU->WriteTrianglePeriodHi(value);
            if (address == 0x400C) APU->WriteNoiseControl(value);
            if (address == 0x400E) APU->WriteNoisePeriod(value);
            if (address == 0x400F) APU->WriteNoiseLength(value);
            if (address == 0x4010) APU->WriteDMCFrequency(value);
            if (address == 0x4011) APU->WriteDMCDAC(value);
            if (address == 0x4012) APU->WriteDMCAddress(value);
            if (address == 0x4013) APU->WriteDMCLength(value);
            if (address == 0x4015) APU->WriteCommonEnable(value);
            if (address == 0x4017) APU->WriteCommonControl(value);
        } else {
            Mapper->SetCpuAt(address, value);
        }
    }
};

template <class Palette_t>
class PpuMemoryMap : public MemoryMap {
public:
    //std::array<Byte, 0x0100> SprRam;
    std::array<Byte, 0x0800> Vram;
    Palette_t * PpuPalette;
    NesMapper * Mapper;

    PpuMemoryMap(Palette_t * palette, NesMapper * mapper)
        : PpuPalette(palette), Mapper(mapper)
    {}

    ~PpuMemoryMap() override {}

    Byte GetByteAt(const Word address) const override {
        if (address >= 0x3F00) {
            return PpuPalette->ReadAt(address - 0x3F00);
        }
        else if (address >= 0x2000) {
            const auto addr = Mapper->NametableAddress(address);
            return Vram[addr];
        }
        else {
            return Mapper->GetPpuAt(address);
        }
    }

    void SetByteAt(const Word address, const Byte value) override {
        if (address >= 0x3F00) {
            PpuPalette->WriteAt(address - 0x3F00, value);
        }
        else if (address >= 0x2000) {
            const auto addr = Mapper->NametableAddress(address);
            Vram[addr] = value;
        }
        else {
            return Mapper->SetPpuAt(address, value);
        }
    }
};

#endif // MEMORY_MAP_H_

