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

Byte Mapper::GetByteAt(const Address address) const {
    return m_memory.at(address);
}

void Mapper::SetByteAt(const Address address, Byte value) {
    m_memory.at(address) = value;
}

Word Mapper::GetWordAt(const Address address) const {
    return m_memory.at(address) + 0x100 * m_memory.at(address + 1);
}

void Mapper::SetWordAt(const Address address, Word value) {
    m_memory.at(address) = value % 0x100;
    m_memory.at(address + 1) = (value / 0x100) % 0x00FF;
}
