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

static const auto MEMORY_COUNT = 0x10000;

typedef int Byte;
typedef int Word;

typedef int Address;

class Mapper {
public:
    explicit Mapper(const std::string &name, const size_t size);

    typedef std::vector<Byte> Buffer;
    const Buffer & data() const;

    std::string Name;

    Byte GetByteAt(const Address address) const;
    void SetByteAt(const Address address, Byte value);

    Word GetWordAt(const Address address) const;
    void SetWordAt(const Address address, Word value);

private:
    Buffer m_memory;
};

#endif /* MAPPER_H_ */
