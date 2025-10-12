#pragma once

#include "Types.h"

template <std::size_t bit>
constexpr Byte Mask(bool isset) {
    return isset ? 0x01 << bit : 0x00;
}

template <std::size_t bit>
constexpr Flag Bit(Byte value) {
    return (value >> bit) & 0x01;
}

template <std::size_t bit>
constexpr bool IsBitSet(Byte value) {
    return Bit<bit>(value) == 1;
}

template <std::size_t bit>
constexpr bool IsBitClear(Byte value) {
    return !IsBitSet<bit>(value);
}

constexpr Word SignExtend(Byte value) {
    return MakeWord(
        value,
        IsBitSet<BYTE_SIGN_BIT>(value) ? 0xFF : 0x00
    );
}
