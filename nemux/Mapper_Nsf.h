#ifndef MAPPER_NSF_H_
#define MAPPER_NSF_H_

#include "Mapper.h"
//#include "MemoryMap.h"
#include "NsfFile.h"

//#include <algorithm>

class Mapper_NSF : public NesMapper {
public:
    std::array<Byte, 0x2000> Ram;
    std::array<Byte, 0x8000> Rom;

// "Firmware"
// @ $00
// clear_hiram: LDA #$00
//              STA $2000
//              LDX #$60
//              TAY
//              STA $00
//      L0007 : STX $01
//      L0009 : STA($00), Y
//              INY
//              BNE L0009
//              INX
//              CPX #$80
//              BNE L0007
// end_clear_hiram:
//
// clear_loram: LDA #$00
//              TAX
//      L0016 : STA $00, X
//              INX
//              BNE L0016
// end_clear_loram:
//
// clear_stack: 
//      L001B : STA $0100, X
//              INX
//              BNE L001B
//              TXS
// end_clear_stack:
//
// initialization: LDA #$00
//                 TAX
//         L0025 : STA $4000, X
//                 INX
//                 CPX #$14
//                 BNE L0025
//                 STA $4015
//                 LDA #$FF
//                 STA $4015
//                 LDA #$40
//                 STA $4017
// end_initialization:
//
// configuration: LDA #$00
//                TAX
// end_configuration:
//
// @ $40
// init_call: JSR $FFFF
// enable_nmi: LDY #$80
//             STY $2000
//             NOP NOP NOP NOP NOP
// init_loop: JMP $414D
//
// @ $50
// play_call: JSR $FFFF
//            NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP 
// play_loop: JMP $415D

#define BOOT_ADDR 0x4100
#define BOOT_SIZE 0x0100
#define BOOT_PAGE HI(BOOT_ADDR)

#define BOOT_CALL 0x50
#define INIT_CALL 0xB2
#define PLAY_CALL 0x91
#define SONG_COUNT 0xD4
#define _NOP 0xEA

    std::array<Byte, 0x100> Firmware{
/*0xA9*/0xA9, 0x00, 0xA2, 0x60, 0xA8, 0x85, 0x00, 0x86, 0x01, 0x91, 0x00, 0xC8, 0xD0, 0xFB, 0xE8, 0xE0,
        0x80, 0xD0, 0xF4, 0xA2, 0x02, 0x86, 0x01, 0x91, 0x00, 0xC8, 0xD0, 0xFB, 0xE8, 0xE0, 0x08, 0xD0,
        0xF4, 0xA2, 0x00, 0x95, 0x00, 0xE8, 0xD0, 0xFB, 0x60, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP,
        0xA9, 0x00, 0xA2, 0x14, 0xCA, 0x9D, 0x00, 0x40, 0xD0, 0xFA, 0x8D, 0x15, 0x40, 0xA2, 0x0F, 0x8E,
        0x15, 0x40, 0xA2, 0x40, 0x8E, 0x17, 0x40, 0x60, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP,
        0xA9, 0x00, 0x8D, 0x00, 0x20, 0xA2, 0xFF, 0x9A, 0xA9, 0x00, 0x48, 0x48, 0x48, 0x4C, 0xA0, 0x41,
        _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP,
        0xA0, 0x01, 0x8C, 0x16, 0x40, 0x88, 0x8C, 0x16, 0x40, 0xA9, 0x01, 0xA8, 0xAD, 0x16, 0x40, 0x6A,
        0x98, 0x2A, 0x90, 0xF7, 0x8D, 0xFD, 0x01, 0x60, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP,
        0x20, 0xFF, 0xFF, 0x20, 0x70, 0x41, 0x40, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP,
        0xA9, 0x00, 0x8D, 0x00, 0x20, 0x20, 0x00, 0x41, 0x20, 0x30, 0x41, 0xAD, 0xFF, 0x01, 0xAE, 0xFE,
        0x01, 0x20, 0xFF, 0xFF, 0x4C, 0xC0, 0x41, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP,
        0xA9, 0x80, 0x8D, 0x00, 0x20, 0xAD, 0xFD, 0x01, 0xF0, 0xFB, 0xA9, 0x00, 0x8D, 0xFD, 0x01, 0xAE,
        0xFF, 0x01, 0xE8, 0xC9, 0xFF, 0xD0, 0x02, 0xA2, 0x00, 0x8E, 0xFF, 0x01, 0x4C, 0xA0, 0x41, _NOP,
        _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP,
        _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP, _NOP,
    };
    
    explicit Mapper_NSF(const NsfFile & nsf) {
        Ram.fill(0);
        
        const auto n = nsf.NsfData.size();
        std::copy_n(nsf.NsfData.cbegin(), n, Rom.begin() + nsf.Header.LoadAddress - 0x8000);

        Rom[0x7FFA] = PLAY_CALL-1; // nsf.Header.PlayAddress & BYTE_MASK;
        Rom[0x7FFB] = BOOT_PAGE; // (nsf.Header.PlayAddress >> BYTE_WIDTH) & BYTE_MASK;
        Rom[0x7FFC] = BOOT_CALL; // nsf.Header.InitAddress & BYTE_MASK;
        Rom[0x7FFD] = BOOT_PAGE; // (nsf.Header.InitAddress >> BYTE_WIDTH) & BYTE_MASK;
        Firmware[INIT_CALL + 0] = nsf.Header.InitAddress & BYTE_MASK;
        Firmware[INIT_CALL + 1] = (nsf.Header.InitAddress >> BYTE_WIDTH) & BYTE_MASK;
        Firmware[PLAY_CALL + 0] = nsf.Header.PlayAddress & BYTE_MASK;
        Firmware[PLAY_CALL + 1] = (nsf.Header.PlayAddress >> BYTE_WIDTH) & BYTE_MASK;
        Firmware[SONG_COUNT] = nsf.Header.SongCount;
    }
    
    Word NametableAddress(const Word address) const override { return 0; }

    Byte GetCpuAt(const Word address) const override {
        const auto atype = GetCpuAddressType(address);
        switch (atype) {
        case CpuAddressType::RAM:
            return Ram[address & 0x1FFF];
        case CpuAddressType::ROM:
            return Rom[address & 0x7FFF];
        case CpuAddressType::Firmware:
            return Firmware[address % BOOT_SIZE];
        }
        return 0;
    }

    void SetCpuAt(const Word address, const Byte value) override {
        const auto atype = GetCpuAddressType(address);
        if (atype == CpuAddressType::RAM) Ram[address & 0x1FFF] = value;
    }

    Byte GetPpuAt(const Word address) const override { return 0; }

    void SetPpuAt(const Word address, const Byte value) override {}

    enum class CpuAddressType {
        Unexpected,
        Invalid,
        Firmware,
        RAM,
        ROM,
    };

    CpuAddressType GetCpuAddressType(const Word address) const {
        if (address < 0x4020) return CpuAddressType::Unexpected;
        if (address < BOOT_ADDR) return CpuAddressType::Invalid;
        if (address < (BOOT_ADDR + BOOT_SIZE)) return CpuAddressType::Firmware;
        if (address < 0x6000) return CpuAddressType::Invalid;
        if (address < 0x8000) return CpuAddressType::RAM;
        return CpuAddressType::ROM;
    }

};

#endif /* MAPPER_NSF_H_ */
