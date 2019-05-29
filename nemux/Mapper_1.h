#ifndef MAPPER_1_H_
#define MAPPER_1_H_

#include "Mapper.h"
#include "MemoryMap.h"
#include "NesFile.h"

class Mapper_001 : public NesMapper {
public:
    struct BankedAddress {
        unsigned int Bank;
        Word Address;
    };

    enum class Mirroring {
        Screen0,
        Screen1,
        Horizontal,
        Vertical,
    };

    enum class PRGBankingMode {
        SwitchAllBanks,
        SwitchFirstBank,
        SwitchLastBank,
    };

    enum class CHRBankingMode {
        OneBank,
        TwoBanks,
    };

    Mirroring ScreenMode;
    PRGBankingMode PrgMode;
    CHRBankingMode ChrMode;
    Byte PrgBank;
    Byte ChrBank0;
    Byte ChrBank1;
    bool RamEnabled;

    std::vector<NesFile::PrgBank> PrgPages;
    std::vector<NesFile::ChrBank> ChrPages;

    explicit Mapper_001(const NesFile & rom) {
        if (rom.Header.MapperNumber != 1) throw invalid_format("Invalid mapper (expected 001)");
        
        if (rom.Header.PrgRomPages == 0) throw unsupported_format("PRG-RAM not supported");
        if (rom.Header.PrgRomPages > 16) throw unsupported_format("PRG too large (max 256K)");
        
        if (rom.Header.ChrRomPages == 0) throw unsupported_format("CHR-RAM not supported");
        if (rom.Header.ChrRomPages > 16) throw unsupported_format("CHR too large (max 128K)");
        
        if (rom.Header.ScreenMode == NesFile::HeaderDesc::FourScreenMode)
            throw unsupported_format("Four-Screen VRAM not supported");

        PrgMode = PRGBankingMode::SwitchFirstBank;
        ChrMode = CHRBankingMode::OneBank;

        PrgPages = rom.PrgRomPages;
        ChrPages = rom.ChrRomPages;
    }

    virtual ~Mapper_001() {}

    BankedAddress TranslateCpu(const Word address) const {
        if (PrgMode == PRGBankingMode::SwitchFirstBank) {
            if (address < 0xC000) return{ PrgBank, (address & 0x3FFF) };
            return{ PrgPages.size() - 1, (address & 0x3FFF) };
        }
        if (PrgMode == PRGBankingMode::SwitchLastBank) {
            if (address < 0xC000) return{ 0, (address & 0x3FFF) };
            return{ PrgBank, (address & 0x3FFF) };
        }
        if (PrgMode == PRGBankingMode::SwitchAllBanks) {
            const auto evenBank = PrgBank & 0xFE;
            if (address < 0xC000) return{ evenBank, (address & 0x3FFF) };
            return{ evenBank + 1, (address & 0x3FFF) };
        }
    }
    
    BankedAddress TranslatePpu(const Word address) const {
        if (ChrMode == CHRBankingMode::OneBank) return{ ChrBank0, (address & 0x1FFF) };
        if (ChrMode == CHRBankingMode::TwoBanks) {
            const auto chr = ((address < 0x1000) ? ChrBank0 : ChrBank1);
            return{ chr / 2, (chr % 2) * 0x1000 + (address & 0x0FFF) };
        }
    }

    Word NametableAddress(const Word address) const override {
        if (ScreenMode == Mirroring::Screen0) return (address & 0x03FF);
        if (ScreenMode == Mirroring::Screen1) return (0x0400 | (address & 0x03FF));
        if (ScreenMode == Mirroring::Vertical) return (address & 0x7FF);
        if (ScreenMode == Mirroring::Horizontal) return (((address & 0x0800) >> 1) | (address & 0x03FF));
    }

    Byte GetCpuAt(const Word address) const override {
        const auto addr = TranslateCpu(address);
        return PrgPages[addr.Bank][addr.Address];
    }

    void SetCpuAt(const Word address, const Byte value) override {
    }

    Byte GetPpuAt(const Word address) const override {
        const auto addr = TranslatePpu(address);
        return ChrPages[addr.Bank][addr.Address];
    }

    void SetPpuAt(const Word address, const Byte value) override {
    }
};

#endif /* MAPPER_1_H_ */
