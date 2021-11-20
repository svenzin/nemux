#ifndef NSF_FILE_H_
#define NSF_FILE_H_

//
//#include <iostream>
//#include <stdexcept>
//#include <fstream>
//#include <array>
//#include <vector>

#include "Types.h"
#include "Error.h"
#include "BitUtil.h"

#include <array>

#define NSF_TAG "NESM" "\x1A"
#define NSF_BANK_COUNT 8
#define NSF_HEADER_SIZE 0x80
#define NSF_HEADER_TEXT_FIELD_SIZE 32

class NsfFile {
public:
    struct HeaderDesc {
        enum LocaleBits : std::size_t {
            PAL = 0,
            Dual
        };
        enum ExtraHardwareBits : std::size_t {
            VRC6 = 0,
            VRC7,
            FDS,
            MMC5,
            Namco163,
            Sunsoft5B,
            VT02Plus,
            Unused
        };
        int Version;
        unsigned int SongCount;
        unsigned int FirstSong;
        Word LoadAddress;
        Word InitAddress;
        Word PlayAddress;
        std::string SongName;
        std::string ArtistName;
        std::string CopyrightHolderName;
        bool SupportsNTSC;
        unsigned int PeriodNTSC;
        bool SupportsPAL;
        unsigned int PeriodPAL;
        bool UsesVRC6;
        bool UsesVRC7;
        bool UsesFDS;
        bool UsesMMC5;
        bool UsesNamco163;
        bool UsesSunsoft5B;
        bool UsesVT02Plus;
        std::size_t DataSize;
        bool UsesBankswitching;
        std::array<Byte, NSF_BANK_COUNT> InitialBank;

        explicit HeaderDesc(std::istream & input) {
            char header[NSF_HEADER_SIZE];
            if (!input.read(reinterpret_cast<char*>(header), NSF_HEADER_SIZE)) {
                throw invalid_format("NSF header not found");
            }
            if ((header[0] != NSF_TAG[0]) ||
                (header[1] != NSF_TAG[1]) ||
                (header[2] != NSF_TAG[2]) ||
                (header[3] != NSF_TAG[3]) ||
                (header[4] != NSF_TAG[4])) throw invalid_format("NSF header tag invalid");

            Version = header[0x05];
            SongCount = header[0x06];
            FirstSong = header[0x07] - 1;
            if (FirstSong >= SongCount) throw invalid_format("Initial song in NSF header is invalid");

            auto ReadWordAt = [&header](std::size_t index) {
                const Word lo = header[index];
                const Word hi = header[index + 1];
                const Word addr = lo + (hi << BYTE_WIDTH);
                return addr;
            };
            LoadAddress = ReadWordAt(0x08);
            if (LoadAddress < 0x8000) throw invalid_format("Load address in NSF header is invalid");
            InitAddress = ReadWordAt(0x0A);
            if (InitAddress < 0x8000) throw invalid_format("Init address in NSF header is invalid");
            PlayAddress = ReadWordAt(0x0C);
            if (PlayAddress < 0x8000) throw invalid_format("Play address in NSF header is invalid");

            auto ReadStringAt = [&header](std::size_t index) {
                std::size_t len = 0;
                while ((header[index + len] != '\0')
                    && (len < NSF_HEADER_TEXT_FIELD_SIZE)) {
                    ++len;
                }
                if (len == NSF_HEADER_TEXT_FIELD_SIZE) throw invalid_format("Text field in NSF header is invalid");
                return std::string(&header[index]);
            };
            SongName = ReadStringAt(0x0E);
            ArtistName = ReadStringAt(0x2E);
            CopyrightHolderName = ReadStringAt(0x4E);

            PeriodNTSC = ReadWordAt(0x6E);
            if (PeriodNTSC == 0) throw invalid_format("NTSC play period in NSF header is invalid");
            PeriodPAL = ReadWordAt(0x78);
            if (PeriodPAL == 0) throw invalid_format("PAL play period in NSF header is invalid");

            for (int i = 0; i < NSF_BANK_COUNT; ++i) {
                InitialBank[i] = header[0x70 + i];
            }
            UsesBankswitching = (InitialBank[0] != 0)
                || (InitialBank[1] != 0)
                || (InitialBank[2] != 0)
                || (InitialBank[3] != 0)
                || (InitialBank[4] != 0)
                || (InitialBank[5] != 0)
                || (InitialBank[6] != 0)
                || (InitialBank[7] != 0);
            
            const auto localeMask = Mask<Dual>(1) | Mask<PAL>(1);
            SupportsNTSC = IsBitSet<Dual>(header[0x7A]) || IsBitClear<PAL>(header[0x7A]);
            SupportsPAL = IsBitSet<Dual>(header[0x7A]) || IsBitSet<PAL>(header[0x7A]);
            if ((header[0x7A] & ~localeMask) != 0) throw invalid_format("NTSC/PAL support bits in NSF header are invalid");

            UsesVRC6 = IsBitSet<VRC6>(header[0x7B]);
            UsesVRC7 = IsBitSet<VRC7>(header[0x7B]);
            UsesFDS = IsBitSet<FDS>(header[0x7B]);
            UsesMMC5 = IsBitSet<MMC5>(header[0x7B]);
            UsesNamco163 = IsBitSet<Namco163>(header[0x7B]);
            UsesSunsoft5B = IsBitSet<Sunsoft5B>(header[0x7B]);
            UsesVT02Plus = IsBitSet<VT02Plus>(header[0x7B]);

            DataSize = header[0x7D]
                + (header[0x7E] << BYTE_WIDTH)
                + (header[0x7F] << (2 * BYTE_WIDTH));
        }
    };
};

#endif //NSF_FILE_H_
