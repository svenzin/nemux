#ifndef MAPPER_0_H_
#define MAPPER_0_H_

#include "Mapper.h"
#include "MemoryMap.h"
#include "NesFile.h"

struct BankedAddress {
    unsigned int Bank;
    Word Address;
};

class Mapper_000 : public NesMapper {
public:
    explicit Mapper_000(const NesFile & rom) {
        if (rom.Header.MapperNumber != 0) throw invalid_format("Invalid mapper");
        if (rom.Header.PrgRomPages != 1 && rom.Header.PrgRomPages != 2) throw unsupported_format("Not an NROM-128/256 rom");
        if (rom.Header.ChrRomPages > 1) throw unsupported_format("Not an NROM-128/256 rom");
        if ((rom.Header.ScreenMode != NesFile::HeaderDesc::HorizontalMirroring)
            && (rom.Header.ScreenMode != NesFile::HeaderDesc::VerticalMirroring))
            throw unsupported_format("Not an NROM-128/256 rom");

        Horizontal = (rom.Header.ScreenMode == NesFile::HeaderDesc::HorizontalMirroring);
        Mirror = (rom.Header.PrgRomPages == 1);
        PrgRom = rom.PrgRomPages;
        IsReadOnly = (rom.Header.ChrRomPages > 0);
        if (IsReadOnly) ChrRom = rom.ChrRomPages[0];
    }

    virtual ~Mapper_000() {}

    BankedAddress TranslateCpu(const Word address) const {
        const unsigned int bank = (address & 0x7FFF) / 0x4000;
        const Word addr = address & 0x3FFF;
        if (Mirror) return{ 0, addr };
        return{ bank, addr };
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

    }

    Byte GetPpuAt(const Word address) const override {
        const Word addr = TranslatePpu(address);
        return ChrRom[addr];
    }

    void SetPpuAt(const Word address, const Byte value) override {
        if (!IsReadOnly) ChrRom[TranslatePpu(address)] = value;
    }

private:
    bool Mirror;
    bool Horizontal;
    bool IsReadOnly;
    std::vector<NesFile::PrgBank> PrgRom;
    NesFile::ChrBank ChrRom;
};

#endif /* MAPPER_0_H_ */
