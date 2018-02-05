#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Mapper_0.h"

using namespace std;

struct Mapper000Test : public ::testing::Test {
    std::ifstream nrom128_stream;
    NesFile nrom128_file;
    Mapper_0 nrom128;

    Mapper000Test()
        : nrom128_stream("nrom128.nes", std::ios::binary),
          nrom128_file(nrom128_stream),
          nrom128(nrom128_file)
    {}
};

TEST_F(Mapper000Test, NROM128_GetFromFirstBank) {
    for (Word addr = 0x0000; addr <= 0x3FFF; ++addr) {
        EXPECT_EQ(addr, nrom128.Translate(0x8000 + addr));
    }
}

TEST_F(Mapper000Test, NROM128_GetFromMirror) {
    for (Word addr = 0x0000; addr <= 0x3FFF; ++addr) {
        EXPECT_EQ(addr, nrom128.Translate(0xC000 + addr));
    }
}

TEST_F(Mapper000Test, NROM128_IsReadOnly) {
    const auto data = nrom128.GetByteAt(0x8000);
    nrom128.SetByteAt(0x8000, data + 1);
    EXPECT_EQ(data, nrom128.GetByteAt(0x8000));
}
