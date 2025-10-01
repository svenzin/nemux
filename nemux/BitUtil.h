#ifndef BIT_UTIL_H_
#define BIT_UTIL_H_

#include <cstddef>

#include "Types.h"

template <std::size_t bit> constexpr Byte Mask(const bool & isset) {
    return isset ? 0x01 << bit : 0x00;
}

template <std::size_t bit> constexpr Flag Bit(const Byte & value) {
    return (value >> bit) & 0x01;
}

template <std::size_t bit> constexpr bool IsBitSet(const Byte & value) {
    return Bit<bit>(value) == 1;
}

template <std::size_t bit> constexpr bool IsBitClear(const Byte & value) {
    return !IsBitSet<bit>(value);
}

#endif
