#include "Types.h"

Byte LO(const Word & w) { return Byte(w); }
Byte HI(const Word & w) { return Byte(w >> BYTE_WIDTH); }
