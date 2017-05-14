/*
 * Types.h
 *
 *  Created on: 20 Jun 2013
 *      Author: scorder
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <cstdint>

typedef uint_fast8_t  Byte;
typedef uint_fast16_t Word;

typedef Byte Flag;

#define BYTE_WIDTH      8
#define BYTE_MASK       0xFF
#define WORD_MASK       0xFFFF
#define WORD_HI_MASK    0xFF00
#define WORD_LO_MASK    0x00FF

#endif /* TYPES_H_ */
