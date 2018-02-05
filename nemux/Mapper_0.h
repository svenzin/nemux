#ifndef MAPPER_0_H_
#define MAPPER_0_H_

#include "MemoryMap.h"
#include "NesFile.h"

class Mapper_0 : public MemoryMap {
public:
    explicit Mapper_0(const NesFile & rom) {
        if (rom.Header.MapperNumber != 0) throw invalid_format("Invalid mapper");
        if (rom.Header.PrgRomPages != 1) throw unsupported_format("Not an NROM-128 rom");
        
        PrgRom = rom.PrgRomPages[0];
    }

    ~Mapper_0() override {}

    Word Translate(const Word address) const {
        return address & 0x3FFF;
    }
    
    Byte GetByteAt(const Word address) const override {
        const Word addr = Translate(address);
        return PrgRom[addr];
    }

    void SetByteAt(const Word address, const Byte value) override {

    }
    
private:
    NesFile::PrgBank PrgRom;
};

#endif /* MAPPER_0_H_ */
