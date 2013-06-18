/*
 * NesRom.cpp
 *
 *  Created on: 12 Jun 2013
 *      Author: scorder
 */

#include "NesRom.h"

using namespace std;

namespace Bits {
template<typename T, typename U> T GetBits(T value, U mask) {
    auto result = (value & mask);
    while ((mask & 1) == 0) {
        mask >>= 1;
        result >>= 1;
    }
    return result;
}
}

namespace Flags6 {
static const auto MAPPER_NUMBER_LOWER = 0xF0;
static const auto FOUR_SCREEN_MODE    = 0x08;
static const auto HAS_TRAINER         = 0x04;
static const auto HAS_BATTERY         = 0x02;
static const auto MIRRORING_TYPE      = 0x01;
}

namespace Flags7 {
static const auto MAPPER_NUMBER_HIGHER = 0xF0;
static const auto UNUSED               = 0x0F;
//static const Byte UNUSED               = 0x0C; // 10 = NES 2.0, other = NES 1.0
//static const Byte PLAYCHOICE_10        = 0x02;
//static const Byte VS_UNISYSTEM         = 0x01;
}

NesRom::NesRom(const string &name)
    : Name {name} {
}

bool NesRom::Read(istream &input) {
    // Read header
    SectionHeader h;
    if (!h.Read(input)) return false;
    Header.push_back(h);

    if (h.HasTrainer) {
        SectionTrainer t;
        if (!t.Read(input)) return false;
        Trainer.push_back(t);
    }

    for (size_t i = 0; i < h.PRG_PagesCount; ++i) {
        Section_PRG p;
        if (!p.Read(input)) return false;
        PRG.push_back(p);
    }

    for (size_t i = 0; i < h.CHR_PagesCount; ++i) {
        Section_CHR c;
        if (!c.Read(input)) return false;
        CHR.push_back(c);
    }

    return true;
}

SectionHeader::~SectionHeader() {

}

bool SectionHeader::Initialize() {
    // NES<EOF>
    for (size_t i = 0; i < NES_Tag.size(); ++i) NES_Tag[i] = Data[i];
    if (NES_Tag != std::array<Byte, 4> {'N', 'E', 'S', 0x1A}) return false;

    PRG_PagesCount = Data[4];
    if (PRG_PagesCount == 0) return false;

    CHR_PagesCount = Data[5];
    if (CHR_PagesCount == 0) return false;

    const auto flags6 = Data[6];
    const auto flags7 = Data[7];

    MapperId = Bits::GetBits(flags6, Flags6::MAPPER_NUMBER_LOWER) +
              (Bits::GetBits(flags7, Flags7::MAPPER_NUMBER_HIGHER) << 4);

    HasTrainer = Bits::GetBits(flags6, Flags6::HAS_TRAINER) != 0;
    HasBattery = Bits::GetBits(flags6, Flags6::HAS_BATTERY) != 0;
    HasFourScreen = Bits::GetBits(flags6, Flags6::FOUR_SCREEN_MODE) != 0;

    Mirroring = (Bits::GetBits(flags6, Flags6::MIRRORING_TYPE) == 0) ?
        MirroringMode::Horizontal : MirroringMode::Vertical;

    if (Bits::GetBits(flags7, Flags7::UNUSED) != 0) return false;
    for (auto i = 8u; i < Data.size(); ++i) {
        if (Data[i] != 0) return false;
    }
//    IsPC10 = Bits::GetBits(flags7, Flags7::PLAYCHOICE_10) != 0;
//    IsVS = Bits::GetBits(flags7, Flags7::VS_UNISYSTEM) != 0;

//    Version = (Bits::GetBits(flags7, Flags7::UNUSED) == 0x10) ?
//        RomVersion::Two : RomVersion::One;

    return true;
}

inline string MirroringModeString(const SectionHeader::MirroringMode m) {
    switch (m) {
        case SectionHeader::MirroringMode::Horizontal: return "Horizontal";
        case SectionHeader::MirroringMode::Vertical:   return "Vertical";
        default:                                       return "Unknown";
    }
}

string SectionHeader::ToString() const {
    ostringstream value;
    value << "SectionHeader[" << Data.size() << "]" << endl
          << "    PRG pages   " << PRG_PagesCount << endl
          << "    CHR pages   " << CHR_PagesCount << endl
          << "    Mapper      " << MapperId << endl
          << "    Trainer     " << boolalpha << HasTrainer<< endl
          << "    Battery     " << boolalpha << HasBattery << endl
          << "    Four screen " << boolalpha << HasFourScreen << endl
          << "    Mirroring   " << MirroringModeString(Mirroring) << endl;
    return value.str();
}
