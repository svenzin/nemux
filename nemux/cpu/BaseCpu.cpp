#include "cpu/BaseCpu.h"


#include "MemoryMap.h"

#include <iomanip>


size_t BaseCpu::GetTicks() const {
    return Ticks;
}

bool BaseCpu::IsStopped() const {
    return _isStopped;
}

Byte BaseCpu::ReadByte(Word address) const {
    return Map->GetByteAt(address);
}

void BaseCpu::WriteByte(Word address, Byte value) const {
    Map->SetByteAt(address, value);
}

std::string BaseCpu::ToString() const {
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

std::string BaseCpu::ToMiniString() const {
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
