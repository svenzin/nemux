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

static const auto MEMORY_COUNT = 0x10000;

class Mapper {
public:
    explicit Mapper(const std::string &name, const size_t size);

    typedef std::vector<Byte> Buffer;
    const Buffer & data() const;

    std::string Name;

    Byte GetByteAt(const Word address) const;
    void SetByteAt(const Word address, Byte value);

    Word GetWordAt(const Word address) const;
    void SetWordAt(const Word address, Word value);

private:
    Buffer m_memory;
};

#endif /* MAPPER_H_ */
