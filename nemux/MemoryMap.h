#ifndef MEMORY_MAP_H_
#define MEMORY_MAP_H_

#include "Types.h"
#include "Ppu.h"
#include "Palette.h"
#include "Mapper.h"

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

template <class Cpu_t, class Ppu_t>
class CpuMemoryMap : public MemoryMap {
public:
    std::array<Byte, 0x0800> RAM;
    Cpu_t * CPU;
    Ppu_t * PPU;
    NesMapper * Mapper;
    
    CpuMemoryMap(Cpu_t * cpu, Ppu_t * ppu, NesMapper * mapper)
        : CPU(cpu), PPU(ppu), Mapper(mapper)
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
            if (address == 0x4014) {
                CPU->DMA(value, PPU->SprRam, PPU->OAMAddress);
            } // OAM DMA
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
            const auto addr = address & 0x07FF;
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
            const auto addr = address & 0x07FF;
            Vram[addr] = value;
        }
        else {
            return Mapper->SetPpuAt(address, value);
        }
    }
};

#endif // MEMORY_MAP_H_

