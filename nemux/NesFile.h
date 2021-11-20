#ifndef NES_FILE_H_
#define NES_FILE_H_

#include "BitUtil.h"
#include "Error.h"

#include <iostream>
#include <fstream>
#include <array>
#include <vector>

#define NES_TAG "NES" "\x1A"

class NesFile {
public:
    explicit NesFile(std::istream & input) : Header(input) {
        Validate(Header);

        if (Header.HasTrainer) {
            if (!input.ignore(512)) throw invalid_format("Could not extract Trainer");
        }

        for (int i = 0; i < Header.PrgRomPages; ++i) {
            std::array<Byte, 0x4000> page;
            if (!input.read((char *) page.data(), 0x4000)) throw invalid_format("Could not extract PRG_ROM page");
            PrgRomPages.push_back(page);
        }

        for (int i = 0; i < Header.ChrRomPages; ++i) {
            std::array<Byte, 0x2000> page;
            if (!input.read((char *) page.data(), 0x2000)) throw invalid_format("Could not extract CHR_ROM page");
            ChrRomPages.push_back(page);
        }
    }

    struct HeaderDesc {
        uint8_t PrgRomPages;
        uint8_t ChrRomPages;

        enum Flags6Bits : std::size_t {
            Mirroring = 0,
            SRAM,
            Trainer,
            FourScreen,
        };
        enum Flags7Bits : std::size_t {
            Versus = 0,
            PC10,
            NES2lo,
            NES2hi,
        };
        bool HasTrainer;
        bool HasBattery;
        enum eScreenMode {
            HorizontalMirroring,
            VerticalMirroring,
            FourScreenMode,
        } ScreenMode;
        bool IsPlaychoice10;
        bool IsVsUnisystem;
        bool IsNES2Format;
        Word MapperNumber;

        explicit HeaderDesc(std::istream & input) {
            char header[16];
            if (!input.read(header, 16)) throw invalid_format("NES header tag not found");
            if (header[0] != NES_TAG[0] ||
                header[1] != NES_TAG[1] ||
                header[2] != NES_TAG[2] ||
                header[3] != NES_TAG[3]) throw invalid_format("NES header tag invalid");

            PrgRomPages = header[4];
            ChrRomPages = header[5];

            if (IsBitSet<FourScreen>(header[6])) {
                ScreenMode = FourScreenMode;
            } else if (IsBitSet<Mirroring>(header[6])) {
                ScreenMode = VerticalMirroring;
            } else {
                ScreenMode = HorizontalMirroring;
            }

            HasTrainer = IsBitSet<Trainer>(header[6]);
            HasBattery = IsBitSet<SRAM>(header[6]);

            IsPlaychoice10 = IsBitSet<PC10>(header[7]);
            IsVsUnisystem = IsBitSet<Versus>(header[7]);
            IsNES2Format = IsBitSet<NES2hi>(header[7])
                && IsBitClear<NES2lo>(header[7]);

            MapperNumber = (header[7] & 0xF0) + ((header[6] & 0xF0) >> 4);
        }
    };

    typedef std::array<Byte, 0x4000> PrgBank;
    typedef std::array<Byte, 0x2000> ChrBank;

    HeaderDesc Header;
    std::vector<PrgBank> PrgRomPages;
    std::vector<ChrBank> ChrRomPages;

    static void Validate(const HeaderDesc & header) {
        if (header.HasTrainer) throw unsupported_format("Trainer is not supported");
        if (header.IsPlaychoice10) throw unsupported_format("PC-10 is not supported");
        if (header.IsVsUnisystem) throw unsupported_format("VS System is not supported");
        if (header.HasBattery) throw unsupported_format("Backed-up SRAM is not supported");
        if (header.IsNES2Format) throw unsupported_format("NES2 format is not supported");
    }
};

#endif //NES_FILE_H_
