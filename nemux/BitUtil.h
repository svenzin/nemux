#ifndef BIT_UTIL_H_
#define BIT_UTIL_H_

#include "Types.h"

template <std::size_t bit> Byte Mask(const bool & isset) {
    return isset ? 0x01 << bit : 0x00;
}

template <std::size_t bit> Flag Bit(const Byte & value) {
    return (value >> bit) & 0x01;
}

template <std::size_t bit> bool IsBitSet(const Byte & value) {
    return Bit<bit>(value) == 1;
}

template <std::size_t bit> bool IsBitClear(const Byte & value) {
    return !IsBitSet<bit>(value);
}

#endif
