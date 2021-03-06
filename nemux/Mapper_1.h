#ifndef MAPPER_1_H_
#define MAPPER_1_H_

#include "Mapper.h"
#include "MemoryMap.h"
#include "NesFile.h"

#include <algorithm>

class Mapper_001 : public NesMapper {
public:
    enum class CpuAddressType {
        Unexpected,
        OpenBus,
        PRG_RAM,
        PRG_ROM,
    };

    CpuAddressType GetCpuAddressType(const Word address) const {
        if (address <= 0x3FFF) return CpuAddressType::Unexpected;
        if (address <= 0x4014) return CpuAddressType::OpenBus;
        if (address <= 0x4017) return CpuAddressType::Unexpected;
        if (address <= 0x5FFF) return CpuAddressType::OpenBus;
        if (address <= 0x7FFF) return CpuAddressType::PRG_RAM;
        return CpuAddressType::PRG_ROM;
    }

    struct BankedAddress {
        unsigned int Bank;
        Word Address;
    };

    enum class Mirroring {
        Screen0,
        Screen1,
        Horizontal,
        Vertical,
    };

    enum class PRGBankingMode {
        SwitchAllBanks,
        SwitchFirstBank,
        SwitchLastBank,
    };

    enum class CHRBankingMode {
        OneBank,
        TwoBanks,
    };

    enum class MMCRegister {
        None,
        Control,
        CHR0,
        CHR1,
        PRG,
    };

    struct MMCWrite {
        MMCRegister Register;
        Byte Value;
    };

    typedef std::array<Byte, 0x2000> PrgRamBank;
    typedef std::array<Byte, 0x4000> PrgRomBank;
    typedef std::array<Byte, 0x2000> ChrRamBank;
    typedef std::array<Byte, 0x1000> ChrRomBank;

    Mirroring ScreenMode;
    PRGBankingMode PrgMode;
    CHRBankingMode ChrMode;
    Byte PrgBank;
    Byte ChrBank0;
    Byte ChrBank1;
    
    bool HasPrgRam;
    PrgRamBank PrgRam;

    std::vector<PrgRomBank> PrgBanks;
    std::vector<ChrRomBank> ChrBanks;

    Byte Register = 0x00;
    int RegisterBit = 0;

    bool HasChrRam;
    ChrRamBank ChrRam;

    explicit Mapper_001(const NesFile & rom) {
        if (rom.Header.MapperNumber != 1) throw invalid_format("Invalid mapper (expected 001)");
        
        if (rom.Header.PrgRomPages == 0) throw unsupported_format("PRG-RAM not supported");
        if (rom.Header.PrgRomPages > 16) throw unsupported_format("PRG too large (max 256K)");
        
        //if (rom.Header.ChrRomPages == 0) throw unsupported_format("CHR-RAM not supported");
        if (rom.Header.ChrRomPages > 16) throw unsupported_format("CHR too large (max 128K)");
        
        if (rom.Header.ScreenMode == NesFile::HeaderDesc::FourScreenMode)
            throw unsupported_format("Four-Screen VRAM not supported");

        if (rom.Header.ScreenMode == NesFile::HeaderDesc::HorizontalMirroring) ScreenMode = Mirroring::Horizontal;
        if (rom.Header.ScreenMode == NesFile::HeaderDesc::VerticalMirroring) ScreenMode = Mirroring::Vertical;

        PrgBanks = rom.PrgRomPages;
        PrgMode = PRGBankingMode::SwitchFirstBank;
        PrgBank = 0;

        for (const auto bank : rom.ChrRomPages) {
            ChrRomBank bank1;
            const auto size = bank1.size();
            std::copy_n(bank.cbegin(), size, bank1.begin());
            ChrBanks.push_back(bank1);

            ChrRomBank bank2;
            std::copy_n(bank.cbegin() + size, size, bank2.begin());
            ChrBanks.push_back(bank2);
        }
        ChrMode = CHRBankingMode::OneBank;
        ChrBank0 = 0;
        ChrBank1 = 0;

        HasPrgRam = true;
        HasChrRam = (rom.Header.ChrRomPages == 0);
    }

    virtual ~Mapper_001() {}

    BankedAddress ToPrgRom(const Word address) const {
        if (PrgMode == PRGBankingMode::SwitchFirstBank) {
            if (address < 0xC000) return{ PrgBank, Word(address & 0x3FFF) };
            return{ PrgBanks.size() - 1, Word(address & 0x3FFF) };
        }
        if (PrgMode == PRGBankingMode::SwitchLastBank) {
            if (address < 0xC000) return{ 0, Word(address & 0x3FFF) };
            return{ PrgBank, Word(address & 0x3FFF) };
        }
        if (PrgMode == PRGBankingMode::SwitchAllBanks) {
            const unsigned int evenBank = PrgBank & 0xFE;
            if (address < 0xC000) return{ evenBank, Word(address & 0x3FFF) };
            return{ evenBank + 1, Word(address & 0x3FFF) };
        }
    }

    Word ToPrgRam(const Word address) const {
        return address & 0x1FFF;
    }
    
    BankedAddress ToChrRom(const Word address) const {
        if (ChrMode == CHRBankingMode::OneBank) {
            if (address < 0x1000) return{ ChrBank0, address };
            return{ unsigned(ChrBank0 + 1), Word(address & 0x0FFF) };
        }
        if (ChrMode == CHRBankingMode::TwoBanks) {
            if (address < 0x1000) return{ ChrBank0, address };
            return{ ChrBank1, Word(address & 0x0FFF) };
        }
    }

    Word ToChrRam(const Word address) const {
        return address & 0x1FFF;
    }

    Word NametableAddress(const Word address) const override {
        if (ScreenMode == Mirroring::Screen0) return (address & 0x03FF);
        if (ScreenMode == Mirroring::Screen1) return (0x0400 | (address & 0x03FF));
        if (ScreenMode == Mirroring::Vertical) return (address & 0x7FF);
        if (ScreenMode == Mirroring::Horizontal) return (((address & 0x0800) >> 1) | (address & 0x03FF));
    }

    MMCWrite ResetMMC1() {
        Register = 0x00;
        RegisterBit = 0;

        MMCWrite result = { MMCRegister::Control, 0x0C };

        if (ScreenMode == Mirroring::Screen0) result.Value = (0x00 | result.Value);
        if (ScreenMode == Mirroring::Screen1) result.Value = (0x01 | result.Value);
        if (ScreenMode == Mirroring::Vertical) result.Value = (0x02 | result.Value);
        if (ScreenMode == Mirroring::Horizontal) result.Value = (0x03 | result.Value);

        if (ChrMode == CHRBankingMode::OneBank) result.Value = (0x00 | result.Value);
        if (ChrMode == CHRBankingMode::TwoBanks) result.Value = (0x10 | result.Value);

        return result;
    }
    
    MMCWrite WriteToMMC1(const Word address, const Byte value) {
        if (address < 0x8000) return{ MMCRegister::None, 0x00 };

        if (IsBitSet<7>(value)) return ResetMMC1();

        if (RegisterBit == 4) {
            MMCWrite result;
            
            if (address < 0xA000) result.Register = MMCRegister::Control;
            else if (address < 0xC000) result.Register = MMCRegister::CHR0;
            else if (address < 0xE000) result.Register = MMCRegister::CHR1;
            else result.Register = MMCRegister::PRG;
            
            result.Value = Register | ((value & 0x01) << 4);
            Register = 0x00;
            RegisterBit = 0;

            return result;
        }

        Register = Register | ((value & 0x01) << RegisterBit);
        ++RegisterBit;
        return{ MMCRegister::None, 0x00 };
    }

    void WriteControl(const Byte value) {
        switch (value & 0x03) {
        case 0: ScreenMode = Mirroring::Screen0; break;
        case 1: ScreenMode = Mirroring::Screen1; break;
        case 2: ScreenMode = Mirroring::Vertical; break;
        case 3: ScreenMode = Mirroring::Horizontal; break;
        }

        switch ((value >> 2) & 0x03) {
        case 0:
        case 1: PrgMode = PRGBankingMode::SwitchAllBanks; break;
        case 2: PrgMode = PRGBankingMode::SwitchLastBank; break;
        case 3: PrgMode = PRGBankingMode::SwitchFirstBank; break;
        }

        switch ((value >> 4) & 0x01) {
        case 0: ChrMode = CHRBankingMode::OneBank; break;
        case 1: ChrMode = CHRBankingMode::TwoBanks; break;
        }
    }

    void WriteCHR0(const Byte value) {
        if (ChrMode == CHRBankingMode::OneBank) ChrBank0 = (value & 0x1E);
        if (ChrMode == CHRBankingMode::TwoBanks) ChrBank0 = (value & 0x1F);
        if (ChrBanks.size() > 0)
            ChrBank0 = (ChrBank0 % ChrBanks.size());
    }

    void WriteCHR1(const Byte value) {
        if (ChrMode == CHRBankingMode::TwoBanks)
            if (ChrBanks.size() > 0)
                ChrBank1 = ((value & 0x1F) % ChrBanks.size());
    }

    void WritePRG(const Byte value) {
        if (PrgMode == PRGBankingMode::SwitchAllBanks) PrgBank = value & 0x0E;
        else PrgBank = value & 0x0F;
        PrgBank = (PrgBank % PrgBanks.size());

        HasPrgRam = IsBitClear<4>(value);
    }

    Byte GetCpuAt(const Word address) const override {
        const auto type = GetCpuAddressType(address);
        if (type == CpuAddressType::PRG_RAM) {
            if (HasPrgRam) {
                const auto addr = ToPrgRam(address);
                return PrgRam[addr];
            }
            // else 'open bus'
        }
        if (type == CpuAddressType::PRG_ROM) {
            const auto addr = ToPrgRom(address);
            return PrgBanks[addr.Bank][addr.Address];
        }
        return 0;
    }

    void SetCpuAt(const Word address, const Byte value) override {
        const auto write = WriteToMMC1(address, value);
        switch (write.Register) {
        case MMCRegister::Control: WriteControl(write.Value); break;
        case MMCRegister::CHR0: WriteCHR0(write.Value); break;
        case MMCRegister::CHR1: WriteCHR1(write.Value); break;
        case MMCRegister::PRG: WritePRG(write.Value); break;
        case MMCRegister::None:
        default: break;
        }

        const auto type = GetCpuAddressType(address);
        if (type == CpuAddressType::PRG_RAM) {
            if (HasPrgRam) {
                const auto addr = ToPrgRam(address);
                PrgRam[addr] = value;
            }
        }
    }

    Byte GetPpuAt(const Word address) const override {
        if (HasChrRam) {
            const auto addr = ToChrRam(address);
            return ChrRam[addr];
        }
        else {
            const auto addr = ToChrRom(address);
            if (ChrBanks.size() > 0)
                return ChrBanks[addr.Bank][addr.Address];
            else
                return 0;
        }
    }

    void SetPpuAt(const Word address, const Byte value) override {
        if (HasChrRam) {
            const auto addr = ToChrRam(address);
            ChrRam[addr] = value;
        }
    }
};

#endif /* MAPPER_1_H_ */
