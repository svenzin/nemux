#ifndef MAPPER_NSF_H_
#define MAPPER_NSF_H_

#include "Mapper.h"
//#include "MemoryMap.h"
#include "NsfFile.h"

//#include <algorithm>

class Mapper_NSF : public NesMapper {
public:
    bool IsBanked;
    std::array<Byte, 0x10> Banks;

    std::array<Byte, 0x2000> Ram;
    std::vector<Byte> Rom;

    std::array<Byte, 0x6> Vectors;

#define PLAYER_ADDR 0x4100
#define PLAYER_SIZE 0x0200
#define PLAYER_PAGE HI(PLAYER_ADDR)

#define RST_HANDLER 0x10
#define NMI_HANDLER 0x20
#define BRK_HANDLER 0x30
#define S_CALL_INIT 0x4B
#define S_CALL_PLAY 0x65
#define VARIABLES 0x00
#define V_SONG_COUNT 0

    std::array<Byte, PLAYER_SIZE> PlayerSoftware{
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x4C, 0x70, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xA9, 0x01, 0x8D, 0x03, 0x41, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xBA, 0x8E, 0x05, 0x41, 0xAD, 0x01, 0x41, 0xAE, 0x02, 0x41, 0x20, 0xFF, 0xFF, 0xAE, 0x05, 0x41,
        0x9A, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xBA, 0x8E, 0x05, 0x41, 0x20, 0xFF, 0xFF, 0xAE, 0x05, 0x41, 0x9A, 0x60, 0x00, 0x00, 0x00, 0x00,
        0xA9, 0x00, 0x8D, 0x00, 0x20, 0x78, 0xA2, 0xFF, 0x9A, 0x8D, 0x01, 0x41, 0x8D, 0x02, 0x41, 0x8D,
        0x03, 0x41, 0x8D, 0x04, 0x41, 0x8E, 0x05, 0x41, 0x4C, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xA9, 0x00, 0xA2, 0x60, 0xA8, 0x85, 0x00, 0x86, 0x01, 0x91, 0x00, 0xC8, 0xD0, 0xFB, 0xE8, 0xE0,
        0x80, 0xD0, 0xF4, 0xA2, 0x02, 0x86, 0x01, 0x91, 0x00, 0xC8, 0xD0, 0xFB, 0xE8, 0xE0, 0x08, 0xD0,
        0xF4, 0xA2, 0x00, 0x95, 0x00, 0xE8, 0xD0, 0xFB, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xA9, 0x00, 0xA2, 0x14, 0xCA, 0x9D, 0x00, 0x40, 0xD0, 0xFA, 0x8D, 0x15, 0x40, 0xA2, 0x0F, 0x8E,
        0x15, 0x40, 0xA2, 0x40, 0x8E, 0x17, 0x40, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xA0, 0x01, 0x8C, 0x16, 0x40, 0x88, 0x8C, 0x16, 0x40, 0xA9, 0x01, 0xA8, 0xAD, 0x16, 0x40, 0x6A,
        0x98, 0x2A, 0x90, 0xF7, 0x8D, 0x04, 0x41, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x78, 0xA9, 0x00, 0x8D, 0x00, 0x20, 0x20, 0x90, 0x41, 0x20, 0xC0, 0x41, 0x20, 0x40, 0x41, 0x4C,
        0x19, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA9, 0x80, 0x8D, 0x00, 0x20, 0x58, 0xAD,
        0x03, 0x41, 0xF0, 0xFB, 0xA9, 0x00, 0x8D, 0x03, 0x41, 0x20, 0x60, 0x41, 0x20, 0xE0, 0x41, 0xAD,
        0x04, 0x41, 0xF0, 0xEB, 0xAE, 0x01, 0x41, 0xE8, 0xEC, 0x00, 0x41, 0xD0, 0x02, 0xA2, 0x00, 0x8E,
        0x01, 0x41, 0x4C, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
    };
    
    explicit Mapper_NSF(const NsfFile & nsf)
    : IsBanked(nsf.Header.UsesBankswitching) {
        for (int i = 0; i < 0x10; ++i) Banks[i] = i;
        if (IsBanked) {
            for (int i = 0; i < 8; ++i) Banks[8 + i] = nsf.Header.InitialBank[i];
        }
        
        Ram.fill(0);
        
        if (IsBanked) {
            const auto nhead = nsf.Header.LoadAddress & 0x0FFF;
            const auto n = nsf.NsfData.size();
            const auto nfoot = 0x0FFF - ((nhead + n - 1) & 0x0FFF);
            Rom.resize(nhead + n + nfoot);
            std::copy_n(nsf.NsfData.cbegin(), n, Rom.begin() + nhead);
        } else {
            Rom.resize(0x8000);
            const auto p = nsf.Header.LoadAddress - 0x8000;
            const auto n = nsf.NsfData.size();
            std::copy_n(nsf.NsfData.cbegin(), n, Rom.begin() + p);
        }

        Vectors[0] = NMI_HANDLER;
        Vectors[1] = PLAYER_PAGE;
        Vectors[2] = RST_HANDLER;
        Vectors[3] = PLAYER_PAGE;
        Vectors[4] = BRK_HANDLER;
        Vectors[5] = PLAYER_PAGE;
        PlayerSoftware[S_CALL_INIT + 0] = LO(nsf.Header.InitAddress);
        PlayerSoftware[S_CALL_INIT + 1] = HI(nsf.Header.InitAddress);
        PlayerSoftware[S_CALL_PLAY + 0] = LO(nsf.Header.PlayAddress);
        PlayerSoftware[S_CALL_PLAY + 1] = HI(nsf.Header.PlayAddress);
        PlayerSoftware[VARIABLES + V_SONG_COUNT] = nsf.Header.SongCount;
    }
    
    Word NametableAddress(const Word address) const override { return 0; }

    Byte GetCpuAt(const Word address) const override {
        const auto atype = GetCpuAddressType(address);
        switch (atype) {
        case CpuAddressType::RAM:
            return Ram[address & 0x1FFF];
        case CpuAddressType::ROM: {
            const auto addr = Translate(address);
            return Rom[addr & 0x7FFF];
        }
        case CpuAddressType::PlayerRAM:
        case CpuAddressType::PlayerROM:
            return PlayerSoftware[address - PLAYER_ADDR];
        case CpuAddressType::Vector:
            return Vectors[address - 0xFFFA];
        }
        return 0;
    }

    void SetCpuAt(const Word address, const Byte value) override {
        const auto atype = GetCpuAddressType(address);
        switch (atype) {
        case CpuAddressType::RAM: Ram[address & 0x1FFF] = value; break;
        case CpuAddressType::Register: SetBank(address & 0xF, value); break;
        case CpuAddressType::PlayerRAM: PlayerSoftware[address - PLAYER_ADDR] = value; break;
        }
    }

    Byte GetPpuAt(const Word address) const override { return 0; }

    void SetPpuAt(const Word address, const Byte value) override {}

    enum class CpuAddressType {
        Unexpected,
        Invalid,
        PlayerRAM,
        PlayerROM,
        RAM,
        ROM,
        Register,
        Vector,
    };

    CpuAddressType GetCpuAddressType(const Word address) const {
        if (address < 0x4020) return CpuAddressType::Unexpected;
        if (address < PLAYER_ADDR) return CpuAddressType::Invalid;
        if (address < (PLAYER_ADDR + 0x10)) return CpuAddressType::PlayerRAM;
        if (address < (PLAYER_ADDR + PLAYER_SIZE)) return CpuAddressType::PlayerROM;
        if (address < 0x5FF8) return CpuAddressType::Invalid;
        if (address < 0x6000) return IsBanked ? CpuAddressType::Register : CpuAddressType::Invalid;
        if (address < 0x8000) return CpuAddressType::RAM;
        if (address < 0xFFFA) return CpuAddressType::ROM;
        return CpuAddressType::Vector;
    }

    Byte GetBank(const Word address) const {
        const auto index = address >> 12;
        return Banks[index];
    }

    void SetBank(const Byte index, const Byte bank) {
        Banks[index] = bank;
    }

    Word Translate(const Word addr) const {
        return (GetBank(addr) << 12) + (addr & 0x0FFF);
    }
};

#endif /* MAPPER_NSF_H_ */
