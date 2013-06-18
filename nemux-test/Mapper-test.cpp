/*
 * Mapper-test.cpp
 *
 *  Created on: 18 Jun 2013
 *      Author: scorder
 */

#include "gtest/gtest.h"

#include "Mapper.h"

//#include <sstream>
#include <stdexcept>

using namespace std;

////////////////////////////////////////////////////////////////

//template <size_t Size> class MockedSection : public Section<Size> {
//public:
//    virtual ~MockedSection() {}
//    virtual bool Initialize() { return false; }
//};
//
//class SectionTest : public ::testing::Test {
//public:
//    istringstream input{"ABCD"};
//};

TEST(MapperTest, CreateBuffer) {
    Mapper m("MyBuffer", 12);
    EXPECT_EQ(size_t{12}, m.data().size());
}

TEST(MapperTest, SetGetByte) {
    Mapper m("MyBuffer", 12);
    Byte b;
    EXPECT_NO_THROW(b = m.GetByteAt(3) + 1);
    EXPECT_NO_THROW(m.SetByteAt(3, b));
    EXPECT_EQ(b, m.GetByteAt(3));
}

TEST(MapperTest, SetByteOutOfRange) {
    Mapper m("MyBuffer", 12);
    EXPECT_THROW(m.SetByteAt(-1, 0), out_of_range);
    EXPECT_THROW(m.SetByteAt(12, 0), out_of_range);
}

TEST(MapperTest, GetByteOutOfRange) {
    Mapper m("MyBuffer", 12);
    EXPECT_THROW(m.GetByteAt(-1), out_of_range);
    EXPECT_THROW(m.GetByteAt(12), out_of_range);
}

TEST(MapperTest, SetGetWord) {
    Mapper m("MyBuffer", 12);
    Word w;
    EXPECT_NO_THROW(w = m.GetWordAt(3) + 1);
    EXPECT_NO_THROW(m.SetWordAt(3, w));
    EXPECT_EQ(w, m.GetWordAt(3));
}

TEST(MapperTest, SetWordOutOfRange) {
    Mapper m("MyBuffer", 12);
    EXPECT_THROW(m.SetWordAt(-1, 0), out_of_range);
    EXPECT_THROW(m.SetWordAt(11, 0), out_of_range);
}

TEST(MapperTest, GetWordOutOfRange) {
    Mapper m("MyBuffer", 12);
    EXPECT_THROW(m.GetWordAt(-1), out_of_range);
    EXPECT_THROW(m.GetWordAt(11), out_of_range);
}

TEST(MapperTest, LittleEndianWord) {
    Mapper m("MyBuffer", 12);
    m.SetWordAt(3, Word{0xBEEF});
    EXPECT_EQ(Byte{0xEF}, m.GetByteAt(3));
    EXPECT_EQ(Byte{0xBE}, m.GetByteAt(4));
}
