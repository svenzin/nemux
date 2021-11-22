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

    // @ $00
    // s_clear_ram:
    //   LDA #$00
    //   LDX #$60
    //   TAY
    //   STA $00
    //     next_hi_page:
    //     STX $01
    //       loop_hi_page:
    //       STA ($00), Y
    //       INY
    //       BNE loop_hi_page
    //     INX
    //     CPX #$80
    //     BNE next_hi_page
    //   LDX #$02
    //     next_lo_page:
    //     STX $01
    //       loop_lo_page:
    //       STA ($00), Y
    //       INY
    //       BNE loop_lo_page
    //     INX
    //     CPX #$08
    //     BNE next_lo_page
    //   LDX #$00
    //     loop_zero_page:
    //     STA $00,X
    //     INX
    //     BNE loop_zero_page
    // RTS
    // 
    // @ $30
    // s_reset_apu:
    //   LDA #$00
    //   LDX #$14
    //     loop_apu:
    //     DEX
    //     STA $4000,X
    //     BNE loop_apu
    //   STA $4015
    //   LDX #$0F
    //   STX $4015
    //   LDX #$40
    //   STX $4017
    // RTS
    // 
    // @ $50
    // v_reset:                                 <<< BOOT_CALL
    //   LDA #$00          ; disable_nmi
    //   STA $2000
    //   LDX #$FF          ; clear stack
    //   TXS
    //   LDA #$00  ; set A=0, X=0
    //   PHA       ; $01FF, song
    //   PHA       ; $01FE, locale
    //   PHA       ; $01FD, sync
    //   PHA       ; $01FC, pad
    //   PHA       ; $01FB, stack
    //   SEI
    //   JMP f_start_song
    // 
    // @ $70
    // s_input:
    //   LDY #$01
    //   STY $4016
    //   DEY
    //   STY $4016
    //   LDA #$01
    //     loop_input:
    //     TAY
    //     LDA $4016
    //     ROR
    //     TYA
    //     ROL
    //     BCC loop_input
    //   STA $01FC
    // RTS
    // 
    // @ $90
    // v_nmi:
    //   LDA #$01  ; set sync
    //   STA $01FD
    // RTI
    // 
    // @ $A0
    // f_start_song:
    //   LDA #$00    ; disable_nmi
    //   STA $2000
    //   JSR s_clear_ram
    //   JSR s_reset_apu
    //   TSX
    //   STX $01FB
    //   LDA $01FF   ; A=song_number
    //   LDX $01FE   ; X=locale
    //   JSR $FFFF   ; call INIT                <<< INIT_CALL
    //   LDX $01FB
    //   TXS
    //   JMP main_loop
    // 
    // @ $C0
    // main_loop:
    //   LDA #$80    ; enable_nmi
    //   STA $2000
    //     inner_loop:
    //     LDA $01FD 
    //     BEQ inner_loop
    //   LDA #$00    ; reset sync
    //   STA $01FD
    //   TSX
    //   STX $01FB
    //   JSR $FFFF   ; call PLAY                <<< PLAY_CALL
    //   LDX $01FB
    //   TXS
    //   JSR s_input ; handle input
    //   LDA $01FC
    //   BEQ inner_loop
    //   LDX $01FF   ; song_number = 
    //   INX         ;   (song_number + 1)
    //   CPX #$FF    ;   % song_count           <<< SONG_COUNT
    //   BNE valid_song
    //   LDX #$00
    //   valid_song:
    //   STX $01FF
    //   JMP f_start_song


#define BOOT_ADDR 0x4100
#define BOOT_SIZE 0x0100
#define BOOT_PAGE HI(BOOT_ADDR)

#define BOOT_CALL 0x50
#define INIT_CALL 0xB6
#define PLAY_CALL 0xD4
#define SONG_COUNT 0xE7
#define _NOP 0xEA

    std::array<Byte, 0x100> Firmware{
        0xA9, 0x00, 0xA2, 0x60, 0xA8, 0x85, 0x00, 0x86, 0x01, 0x91, 0x00, 0xC8, 0xD0, 0xFB, 0xE8, 0xE0,
        0x80, 0xD0, 0xF4, 0xA2, 0x02, 0x86, 0x01, 0x91, 0x00, 0xC8, 0xD0, 0xFB, 0xE8, 0xE0, 0x08, 0xD0,
        0xF4, 0xA2, 0x00, 0x95, 0x00, 0xE8, 0xD0, 0xFB, 0x60, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xA9, 0x00, 0xA2, 0x14, 0xCA, 0x9D, 0x00, 0x40, 0xD0, 0xFA, 0x8D, 0x15, 0x40, 0xA2, 0x0F, 0x8E,
        0x15, 0x40, 0xA2, 0x40, 0x8E, 0x17, 0x40, 0x60, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xA9, 0x00, 0x8D, 0x00, 0x20, 0xA2, 0xFF, 0x9A, 0xA9, 0x00, 0x48, 0x48, 0x48, 0x48, 0x48, 0x78,
        0x4C, 0xA0, 0x41, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xA0, 0x01, 0x8C, 0x16, 0x40, 0x88, 0x8C, 0x16, 0x40, 0xA9, 0x01, 0xA8, 0xAD, 0x16, 0x40, 0x6A,
        0x98, 0x2A, 0x90, 0xF7, 0x8D, 0xFC, 0x01, 0x60, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xA9, 0x01, 0x8D, 0xFD, 0x01, 0x40, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xA9, 0x00, 0x8D, 0x00, 0x20, 0x20, 0x00, 0x41, 0x20, 0x30, 0x41, 0xBA, 0x8E, 0xFB, 0x01, 0xAD,
        0xFF, 0x01, 0xAE, 0xFE, 0x01, 0x20, 0xFF, 0xFF, 0xAE, 0xFB, 0x01, 0x9A, 0x4C, 0xC0, 0x41, 0xEA,
        0xA9, 0x80, 0x8D, 0x00, 0x20, 0xAD, 0xFD, 0x01, 0xF0, 0xFB, 0xA9, 0x00, 0x8D, 0xFD, 0x01, 0xBA,
        0x8E, 0xFB, 0x01, 0x20, 0xFF, 0xFF, 0xAE, 0xFB, 0x01, 0x9A, 0x20, 0x70, 0x41, 0xAD, 0xFC, 0x01,
        0xF0, 0xE3, 0xAE, 0xFF, 0x01, 0xE8, 0xE0, 0xFF, 0xD0, 0x02, 0xA2, 0x00, 0x8E, 0xFF, 0x01, 0x4C,
        0xA0, 0x41, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
    };
    
    explicit Mapper_NSF(const NsfFile & nsf) {
        Ram.fill(0);
        
        const auto n = nsf.NsfData.size();
        std::copy_n(nsf.NsfData.cbegin(), n, Rom.begin() + nsf.Header.LoadAddress - 0x8000);

        Rom[0x7FFA] = PLAY_CALL-1;
        Rom[0x7FFB] = BOOT_PAGE;
        Rom[0x7FFC] = BOOT_CALL;
        Rom[0x7FFD] = BOOT_PAGE;
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
