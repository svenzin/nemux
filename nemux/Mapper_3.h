#ifndef MAPPER_3_H_
#define MAPPER_3_H_

#include "Mapper.h"
#include "MemoryMap.h"
#include "NesFile.h"

#include <algorithm>

class Mapper_003 : public NesMapper {
public:
    enum class CpuAddressType {
        Unexpected,
        OpenBus,
        PRG_RAM,
        PRG_ROM,
    };

    CpuAddressType GetCpuAddressType(const Word address) const {
        if (address <= 0x3FFF) return CpuAddressType::Unexpected;
        if (address <= 0x4014) return CpuAddressType::OpenBus;
        if (address <= 0x4017) return CpuAddressType::Unexpected;
        if (address <= 0x5FFF) return CpuAddressType::OpenBus;
        if (address <= 0x7FFF) return CpuAddressType::PRG_RAM;
        return CpuAddressType::PRG_ROM;
    }

    struct BankedAddress {
        unsigned int Bank;
        Word Address;
    };

    enum class Mirroring {
        Horizontal,
        Vertical,
    };

    typedef std::array<Byte, 0x4000> PrgRomBank;
    typedef std::array<Byte, 0x2000> ChrRamBank;
    typedef std::array<Byte, 0x2000> ChrRomBank;

    Mirroring ScreenMode;
    Byte ChrBank;
    
    std::vector<PrgRomBank> PrgBanks;
    std::vector<ChrRomBank> ChrBanks;

    bool HasChrRam;
    ChrRamBank ChrRam;

    explicit Mapper_003(const NesFile & rom) {
        if (rom.Header.MapperNumber != 3) throw invalid_format("Invalid mapper (expected 003)");
        
        if (rom.Header.PrgRomPages == 0) throw unsupported_format("PRG-ROM is required");
        if (rom.Header.PrgRomPages > 2) throw unsupported_format("PRG too large (max 32K)");
        
        //if (rom.Header.ChrRomPages == 0) throw unsupported_format("CHR-RAM not supported");
        if (rom.Header.ChrRomPages > 16) throw unsupported_format("CHR too large (max 128K)");
        
        if (rom.Header.ScreenMode == NesFile::HeaderDesc::FourScreenMode)
            throw unsupported_format("Four-Screen VRAM not supported");

        if (rom.Header.ScreenMode == NesFile::HeaderDesc::HorizontalMirroring) ScreenMode = Mirroring::Horizontal;
        if (rom.Header.ScreenMode == NesFile::HeaderDesc::VerticalMirroring) ScreenMode = Mirroring::Vertical;

        PrgBanks = rom.PrgRomPages;
        ChrBanks = rom.ChrRomPages;

        ChrBank = 0;
        HasChrRam = (rom.Header.ChrRomPages == 0);
    }

    virtual ~Mapper_003() {}

    BankedAddress ToPrgRom(const Word address) const {
        if (address < 0xC000) return{ 0, Word(address & 0x3FFF) };
        return{ PrgBanks.size() - 1, Word(address & 0x3FFF) };
    }

    BankedAddress ToChrRom(const Word address) const {
        return{ ChrBank, Word(address & 0x1FFF) };
    }

    Word ToChrRam(const Word address) const {
        return address & 0x1FFF;
    }

    Word NametableAddress(const Word address) const override {
        if (ScreenMode == Mirroring::Vertical) return (address & 0x7FF);
        if (ScreenMode == Mirroring::Horizontal) return (((address & 0x0800) >> 1) | (address & 0x03FF));
    }

    void WriteToCNROM(const Word address, const Byte value) {
        if (address < 0x8000) return;
        ChrBank = ((value & 0x03) % ChrBanks.size());
    }

    Byte GetCpuAt(const Word address) const override {
        const auto type = GetCpuAddressType(address);
        if (type == CpuAddressType::PRG_ROM) {
            const auto addr = ToPrgRom(address);
            return PrgBanks[addr.Bank][addr.Address];
        }
        return 0;
    }

    void SetCpuAt(const Word address, const Byte value) override {
        WriteToCNROM(address, value);
    }

    Byte GetPpuAt(const Word address) const override {
        if (HasChrRam) {
            const auto addr = ToChrRam(address);
            return ChrRam[addr];
        }
        else {
            const auto addr = ToChrRom(address);
            return ChrBanks[addr.Bank][addr.Address];
        }
    }

    void SetPpuAt(const Word address, const Byte value) override {
        if (HasChrRam) {
            const auto addr = ToChrRam(address);
            ChrRam[addr] = value;
        }
    }
};

#endif /* MAPPER_3_H_ */
