#pragma once

#include "Types.h"

template <size_t bit>
constexpr Byte Mask(bool isset) {
    return isset ? 0x01 << bit : 0x00;
}

template <size_t bit>
constexpr Flag Bit(Byte value) {
    return (value >> bit) & 0x01;
}

template <size_t bit>
constexpr bool IsBitSet(Byte value) {
    return Bit<bit>(value) == 1;
}

template <size_t bit>
constexpr bool IsBitClear(Byte value) {
    return !IsBitSet<bit>(value);
}

constexpr Flag SignBit(Byte value) {
    return Bit<BYTE_SIGN_BIT>(value);
}

constexpr bool IsSignBitSet(Byte value) {
    return IsBitSet<BYTE_SIGN_BIT>(value);
}

constexpr bool IsSignBitClear(Byte value) {
    return IsBitClear<BYTE_SIGN_BIT>(value);
}

constexpr Word SignExtend(Byte value) {
    return MakeWord(
        value,
        IsSignBitSet(value) ? 0xFF : 0x00
    );
}
