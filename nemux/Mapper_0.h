#ifndef MAPPER_0_H_
#define MAPPER_0_H_

#include "Mapper.h"
#include "MemoryMap.h"
#include "NesFile.h"

class Mapper_000 : public NesMapper {
public:
    explicit Mapper_000(const NesFile & rom) {
        if (rom.Header.MapperNumber != 0) throw invalid_format("Invalid mapper");
        if (rom.Header.PrgRomPages != 1) throw unsupported_format("Not an NROM-128 rom");
        if (rom.Header.ChrRomPages != 1) throw unsupported_format("Not an NROM-128 rom");

        PrgRom = rom.PrgRomPages[0];
        ChrRom = rom.ChrRomPages[0];
    }

    virtual ~Mapper_000() {}

    Word TranslateCpu(const Word address) const {
        return address & 0x3FFF;
    }

    Word TranslatePpu(const Word address) const {
        return address & 0x3FFF;
    }

    Byte GetCpuAt(const Word address) const override {
        const Word addr = TranslateCpu(address);
        return PrgRom[addr];
    }

    void SetCpuAt(const Word address, const Byte value) override {

    }

    Byte GetPpuAt(const Word address) const override {
        const Word addr = TranslatePpu(address);
        return PrgRom[addr];
    }

    void SetPpuAt(const Word address, const Byte value) override {

    }

private:
    NesFile::PrgBank PrgRom;
    NesFile::ChrBank ChrRom;
};

#endif /* MAPPER_0_H_ */
