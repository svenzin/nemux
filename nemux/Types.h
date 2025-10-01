#pragma once

#include <cstdint>

using Flag = uint8_t;
using Byte = uint8_t;
using Word = uint16_t;

#define BYTE_WIDTH      8
#define BYTE_MASK       0xFF
#define BYTE_SIGN_BIT   7
#define WORD_MASK       0xFFFF
#define WORD_HI_MASK    0xFF00
#define WORD_LO_MASK    0x00FF

constexpr Byte LO(Word w) { return static_cast<Byte>(w); }
constexpr Byte HI(Word w) { return static_cast<Byte>(w >> BYTE_WIDTH); }

constexpr void SetLO(Word & w, Byte lo) { w = (w & WORD_HI_MASK) | lo; }
constexpr void SetHI(Word & w, Byte hi) { w = (hi << BYTE_WIDTH) | LO(w); }
