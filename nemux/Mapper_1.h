#ifndef MAPPER_1_H_
#define MAPPER_1_H_

#include "Mapper.h"
#include "MemoryMap.h"
#include "NesFile.h"

class Mapper_001 : public NesMapper {
public:
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
    }

    virtual ~Mapper_001() {}

    Word NametableAddress(const Word address) const override {
    }

    Byte GetCpuAt(const Word address) const override {
    }

    void SetCpuAt(const Word address, const Byte value) override {
    }

    Byte GetPpuAt(const Word address) const override {
    }

    void SetPpuAt(const Word address, const Byte value) override {
    }
};

#endif /* MAPPER_1_H_ */
