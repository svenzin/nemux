#ifndef MAPPER_2_H_
#define MAPPER_2_H_

#include "Mapper.h"
#include "MemoryMap.h"
#include "NesFile.h"

class Mapper_002 : public NesMapper {
public:
    struct BankedAddress {
        unsigned int Bank;
        Word Address;
    };

    const char * RomName;
    explicit Mapper_002(const NesFile & rom) {
        if (rom.Header.MapperNumber != 2) throw invalid_format("Invalid mapper");
        if (rom.Header.PrgRomPages == 0 && rom.Header.PrgRomPages > 16) throw unsupported_format("Not a UxROM");
        if (rom.Header.ChrRomPages > 0) throw unsupported_format("Not a UxROM");
        if ((rom.Header.ScreenMode != NesFile::HeaderDesc::HorizontalMirroring)
            && (rom.Header.ScreenMode != NesFile::HeaderDesc::VerticalMirroring))
            throw unsupported_format("Not a UxROM");

        if (rom.Header.PrgRomPages <= 16) RomName = "UNROM";
        else RomName = "UOROM";

        CurrentBank = 0;
        Horizontal = (rom.Header.ScreenMode == NesFile::HeaderDesc::HorizontalMirroring);
        Mirror = (rom.Header.PrgRomPages == 1);
        PrgRom = rom.PrgRomPages;
    }

    virtual ~Mapper_002() {}

    BankedAddress TranslateCpu(const Word address) const {
        const Word addr = address & 0x3FFF;
        const unsigned int bank = (address & 0x7FFF) / 0x4000;
        if (bank == 1) return{ PrgRom.size() - 1, addr };
        return{ CurrentBank, addr };
    }

    Word TranslatePpu(const Word address) const {
        return address & 0x3FFF;
    }

    Word NametableAddress(const Word address) const {
        if (Horizontal) return ((address & 0x0800) >> 1) | (address & 0x03FF);
        return address & 0x07FF;
    }

    Byte GetCpuAt(const Word address) const override {
        const auto addr = TranslateCpu(address);
        return PrgRom[addr.Bank][addr.Address];
    }

    void SetCpuAt(const Word address, const Byte value) override {
        CurrentBank = value;
    }

    Byte GetPpuAt(const Word address) const override {
        const Word addr = TranslatePpu(address);
        return ChrRam[addr];
    }

    void SetPpuAt(const Word address, const Byte value) override {
        const Word addr = TranslatePpu(address);
        ChrRam[addr] = value;
    }

private:
    bool Mirror;
    bool Horizontal;
    unsigned int CurrentBank;
    std::vector<NesFile::PrgBank> PrgRom;
    NesFile::ChrBank ChrRam;
};

#endif /* MAPPER_2_H_ */
