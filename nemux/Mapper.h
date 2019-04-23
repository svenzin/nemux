/*
 * Mapper.h
 *
 *  Created on: 11 Jun 2013
 *      Author: scorder
 */

#ifndef MAPPER_H_
#define MAPPER_H_

#include <string>
#include <vector>

#include "Types.h"

class NesMapper {
public:

    virtual Byte GetCpuAt(const Word address) const = 0;

    virtual void SetCpuAt(const Word address, const Byte value) = 0;

    virtual Byte GetPpuAt(const Word address) const = 0;

    virtual void SetPpuAt(const Word address, const Byte value) = 0;
};

#endif /* MAPPER_H_ */
