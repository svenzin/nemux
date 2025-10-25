#pragma once


#include <cstdint>


using Flag = uint8_t;
using Byte = uint8_t;
using Word = uint16_t;

#define BYTE_WIDTH      8
#define BYTE_MASK       0xFF
#define BYTE_OVF_BIT    6
#define BYTE_SIGN_BIT   7
#define BYTE_LSB_BIT    0
#define BYTE_MSB_BIT    7
#define BYTE_MAX_VALUE  0xFF

#define WORD_WIDTH      16
#define WORD_MASK       0xFFFF
#define WORD_HI_MASK    0xFF00
#define WORD_LO_MASK    0x00FF
#define WORD_SIGN_BIT   15

constexpr Word MakeWord(Byte lo, Byte hi) { return static_cast<Word>((hi << BYTE_WIDTH) | lo); }

constexpr Byte LO(Word w) { return static_cast<Byte>(w); }
constexpr Byte HI(Word w) { return static_cast<Byte>(w >> BYTE_WIDTH); }

constexpr void SetLO(Word& w, Byte lo) { w = static_cast<Word>((w & WORD_HI_MASK) | lo); } // MakeWord(lo, HI(w)); }
constexpr void SetHI(Word& w, Byte hi) { w = static_cast<Word>((hi << BYTE_WIDTH) | (w & WORD_LO_MASK)); } // MakeWord(LO(w), hi); }
