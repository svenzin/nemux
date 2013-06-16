/*
 * NesRom.h
 *
 *  Created on: 12 Jun 2013
 *      Author: scorder
 */

#ifndef NESROM_H_
#define NESROM_H_

#include <string>
#include <istream>
#include <vector>
#include <array>

typedef char Byte;

template <std::size_t Size> class Section {
public:
    std::array<Byte, Size> Data;

    bool Read(std::istream &input) {
        if (!input.read(Data.data(), Data.size())) return false;
        return Initialize();
    }

    virtual bool Initialize() { return true; }

    virtual ~Section() {}
};

class SectionHeader : public Section<16> {
public:
    virtual ~SectionHeader();

    virtual bool Initialize();

    std::array<Byte, 4> NES_Tag;
    size_t PRG_PagesCount;
    size_t CHR_PagesCount;
    int MapperId;
    bool HasTrainer;
    bool HasBattery;
    bool HasFourScreen;
    enum class MirroringMode { Horizontal, Vertical } Mirroring;
//    bool IsPC10;
//    bool IsVS;
//    enum class RomVersion { One, Two } Version;
};
class SectionTrainer : public Section<512> {};
class Section_PRG : public Section<16384> {};
class Section_CHR : public Section<8192> {};
//class SectionPlaychoiceInstROM : public Section<8192> {};
//class SectionPlaychoicePROM : public Section<32> {};
//class SectionTitle : public Section<128> {};

class NesRom {
public:
    explicit NesRom(const std::string name);
    bool Read(std::istream &input);

    std::string Name;
    std::vector<SectionHeader> Header;
    std::vector<SectionTrainer> Trainer;
    std::vector<Section_PRG> PRG;
    std::vector<Section_CHR> CHR;
//    std::vector<SectionPlaychoiceInstROM> InstROM;
//    std::vector<SectionPlaychoicePROM> PROM;
//    std::vector<SectionTitle> Title;
};


#endif /* NESROM_H_ */
