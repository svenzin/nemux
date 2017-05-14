/*
 * Mapper.cpp
 *
 *  Created on: 18 Jun 2013
 *      Author: scorder
 */

#include "Mapper.h"

using namespace std;

Mapper::Mapper(const std::string &name, const size_t size)
    : Name {name} {
    m_memory.resize(size, 0);
}

const Mapper::Buffer & Mapper::data() const {
    return m_memory;
}

Byte Mapper::GetByteAt(const Word address) const {
    return m_memory.at(address);
}

void Mapper::SetByteAt(const Word address, Byte value) {
    m_memory.at(address) = value;
}
