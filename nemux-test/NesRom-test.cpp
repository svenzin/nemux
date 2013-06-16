/*
 * NesRom-test.cpp
 *
 *  Created on: 16 Jun 2013
 *      Author: scorder
 */

#include "gtest/gtest.h"

#include "NesRom.h"

#include <sstream>

using namespace std;

////////////////////////////////////////////////////////////////

template <size_t Size> class MockedSection : public Section<Size> {
public:
    virtual ~MockedSection() {}
    virtual bool Initialize() { return false; }
};

class SectionTest : public ::testing::Test {
public:
    istringstream input{"ABCD"};
};

TEST_F(SectionTest, EnoughData) {
    array<Byte, 3> expected{'A', 'B', 'C'};

    Section<3> a;
    EXPECT_TRUE(a.Read(input));
    EXPECT_EQ(size_t{3}, a.Data.size());
    EXPECT_EQ(expected, a.Data);
}

TEST_F(SectionTest, NotEnoughData) {
    Section<5> a;
    EXPECT_FALSE(a.Read(input));
}

TEST_F(SectionTest, FailInitialization) {
    MockedSection<3> a;
    EXPECT_FALSE(a.Read(input));
}

////////////////////////////////////////////////////////////////

class SectionHeaderTest : public ::testing::Test {
public:
    SectionHeaderTest() : data{ 'N',  'E',  'S', 0x1A, // NES<EOF>
                               0x01, 0x02, 0x6F, 0x40, // 1 PRG, 2 CHR, Mapper 0x46
                                                       // Four screen, Trainer
                                                       // Battery, Vertical mirroring
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00}
    {}

    string data;
};

TEST_F(SectionHeaderTest, SuccessAll) {
    istringstream input(data);

    array<Byte, 4> expectedNesTag{'N',  'E',  'S', 0x1A};

    SectionHeader h;
    EXPECT_TRUE(h.Read(input));
    EXPECT_EQ(expectedNesTag, h.NES_Tag);
    EXPECT_EQ(1, h.PRG_PagesCount);
    EXPECT_EQ(2, h.CHR_PagesCount);
    EXPECT_EQ(0x46, h.MapperId);
    EXPECT_TRUE(h.HasTrainer);
    EXPECT_TRUE(h.HasBattery);
    EXPECT_TRUE(h.HasFourScreen);
    EXPECT_EQ(SectionHeader::MirroringMode::Vertical, h.Mirroring);
}

TEST_F(SectionHeaderTest, FailOnNesTag) {
    data[3] = 'T';
    istringstream input(data);

    SectionHeader h;
    EXPECT_FALSE(h.Read(input));
}

TEST_F(SectionHeaderTest, FailOnNoPRG) {
    data[4] = 0x00;
    istringstream input(data);

    SectionHeader h;
    EXPECT_FALSE(h.Read(input));
}

TEST_F(SectionHeaderTest, FailOnNoCHR) {
    data[5] = 0x00;
    istringstream input(data);

    SectionHeader h;
    EXPECT_FALSE(h.Read(input));
}

TEST_F(SectionHeaderTest, FailOnFlag8ExpectedZeros) {
    data[7] = 0xFF;
    istringstream input(data);

    SectionHeader h;
    EXPECT_FALSE(h.Read(input));
}

TEST_F(SectionHeaderTest, FailOnExpectedZeros) {
    data[12] = 0xFF;
    istringstream input(data);

    SectionHeader h;
    EXPECT_FALSE(h.Read(input));
}
