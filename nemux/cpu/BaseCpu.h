#pragma once

#include "BitUtil.h"
#include "MemoryMap.h"
#include "Types.h"

#include <iomanip>


class MemoryMap;


namespace Addresses {
    constexpr Word VECTOR_NMI{ 0xFFFA };
    constexpr Word VECTOR_RST{ 0xFFFC };
    constexpr Word VECTOR_IRQ{ 0xFFFE };
    constexpr Word STACK_PAGE{ 0x0100 };
}


class BaseCpu {
protected:
    size_t Ticks{}; // TODO rename
    bool _isStopped{ false };

public:
    size_t GetTicks() const { return Ticks; }
    bool IsStopped() const { return _isStopped; }
    void SetStopped(bool stopped) { _isStopped = stopped; }// TODO remove when CPUs have been separated

    enum Bits : size_t {
        Car, Zer, Int, Dec, Brk, Unu, Ovf, Neg,
    };

    std::string Name;

    Word VectorRST{ Addresses::VECTOR_RST };
    Word VectorNMI{ Addresses::VECTOR_NMI };
    Word VectorIRQ{ Addresses::VECTOR_IRQ };
    Word StackPage{ Addresses::STACK_PAGE };

    MemoryMap* Map{};

    Word PC; // Program Counter
    Byte S;  // Stack Pointer

    Byte A;  // Accumulator
    Byte X;  // Index Register X
    Byte Y;  // Index Register Y

    Flag N;  // Negative Flag
    Flag V;  // Overflow Flag
    Flag D;  // Decimal Mode
    Flag I;  // Interrupt Disable
    Flag Z;  // Zero Flag
    Flag C;  // Carry Flag

    // TODO rename and redesign for multiple sources
    bool LineRST; // Is triggering reset using a "line" the right way?
                  // a straight Reset() method call is quite simple
    bool LineIRQ; // Interrupt ReQuest line
    bool LineNMI; // Non-Maskable Interrupt line

    constexpr void SetStatusByte(Byte status) {
        N = Bit<Neg>(status);
        V = Bit<Ovf>(status);
        D = Bit<Dec>(status);
        I = Bit<Int>(status);
        Z = Bit<Zer>(status);
        C = Bit<Car>(status);
    }

    constexpr Byte GetStatusByte(const Flag B) const {
        return
            Mask<Neg>(N) | Mask<Ovf>(V) |
            Mask<Unu>(1) | Mask<Brk>(B) |
            Mask<Dec>(D) | Mask<Int>(I) |
            Mask<Zer>(Z) | Mask<Car>(C);
    }
    
    Byte ReadByte(Word address) const {
        return Map->GetByteAt(address);
    }
    
    void WriteByte(Word address, Byte value) const {
        Map->SetByteAt(address, value);
    }
    
    virtual void PowerUp() = 0;
    virtual void Reset() = 0;
    [[nodiscard]] virtual bool Tick() = 0;
    virtual void DMA(Byte page, Byte* target, Byte offset) = 0;

    // TODO move this in a "debug" place
    std::string ToString() const {
        using std::hex, std::dec, std::boolalpha;
        using std::setfill, std::setw;
        using std::endl;
        std::ostringstream value;
        value << "Cpu " << Name << std::endl
            << "- Registers PC 0x" << hex << setfill('0') << setw(4) << PC << "(" << dec << PC << ")" << endl
            << "            SP 0x" << hex << setfill('0') << setw(2) << S << "(" << dec << S << ")" << endl
            << "             A 0x" << hex << setfill('0') << setw(2) << A << "(" << dec << A << ")" << endl
            << "             X 0x" << hex << setfill('0') << setw(2) << X << "(" << dec << X << ")" << endl
            << "             Y 0x" << hex << setfill('0') << setw(2) << Y << "(" << dec << Y << ")" << endl
            << "- Flags C " << setw(5) << boolalpha << (C != 0) << endl
            << "        Z " << setw(5) << boolalpha << (Z != 0) << endl
            << "        I " << setw(5) << boolalpha << (I != 0) << endl
            << "        D " << setw(5) << boolalpha << (D != 0) << endl
            << "        V " << setw(5) << boolalpha << (V != 0) << endl
            << "        N " << setw(5) << boolalpha << (N != 0) << endl;
        return value.str();
    }

    std::string ToMiniString() const {
        using std::hex, std::setfill, std::setw;
        std::ostringstream value;
        const auto P = GetStatusByte(0);
        value << "Cpu " << Name
            << " " << Ticks
            << " PC=$" << hex << setfill('0') << setw(4) << PC
            << " S=$" << hex << setfill('0') << setw(2) << Word{S}
            << " A=$" << hex << setfill('0') << setw(2) << Word{A}
            << " X=$" << hex << setfill('0') << setw(2) << Word{X}
            << " Y=$" << hex << setfill('0') << setw(2) << Word{Y}
            << " P=$" << hex << setfill('0') << setw(2) << Word{P}
            << " "
            << (C == 0 ? 'c' : 'C')
            << (Z == 0 ? 'z' : 'Z')
            << (I == 0 ? 'i' : 'I')
            << (D == 0 ? 'd' : 'D')
            << (V == 0 ? 'v' : 'V')
            << (N == 0 ? 'n' : 'N');
        return value.str();
    }
};
