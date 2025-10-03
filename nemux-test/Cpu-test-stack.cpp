#include "CpuBaseTest.h"

#include <vector>
#include <map>
#include <array>
#include <functional>

using namespace std;
using namespace Instructions;
using namespace Addressing;

struct CpuTestStack : public CpuBaseTest {
    template<typename Getter, typename Setter>
    void Test_Transfer(Getter get, Setter set, Opcode op) {
        auto tester = [&] (Byte value, Flag expZ, Flag expN) {
            set(value);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(value, get());
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x20, 0, 0);
        tester(0xA0, 0, 1);
        tester(0x00, 1, 0);
    }
};

TEST_F(CpuTestStack, TSX) {
    Test_Transfer(
        [&]              { return cpu.X; },
        [&] (Byte value) { cpu.S = value; },
        Opcode(TSX, Implicit, 1, 2)
    );
}

TEST_F(CpuTestStack, TXS) {
    // TXS does not change the flags
    auto op = Opcode(TXS, Implicit, 1, 2);
    cpu.X = 0x20;

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0x20, cpu.S);
}

TEST_F(CpuTestStack, PHP_FlagB) {
    // PHP is software instruction pushing the Status -> B is set
    const auto op = Opcode(PHP, Implicit, 1, 3);
    cpu.WriteByte(BASE_PC, 0xFF);

    cpu.StackPage = 0x0100;
    cpu.S = 0xF0;

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0x10, cpu.ReadByte(0x01F0) & 0x10);
    EXPECT_EQ(0xEF, cpu.S);
}

TEST_F(CpuTestStack, PLP_FlagB) {
    const auto op = Opcode(PLP, Implicit, 1, 4);
    cpu.WriteByte(BASE_PC, 0xFF);

    cpu.StackPage = 0x0100;
    cpu.S = 0xEF;
    cpu.WriteByte(0x01F0, 0xFF);

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0xF0, cpu.S);
}

TEST_F(CpuTestStack, PHA) {
    const auto op = Opcode(PHA, Implicit, 1, 3);

    cpu.WriteByte(BASE_PC, 0xFF);

    cpu.A = 0x20;
    cpu.StackPage = 0x0100;
    cpu.S = 0xF0;

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0x20, cpu.ReadByte(0x01F0));
    EXPECT_EQ(0xEF, cpu.S);
}

TEST_F(CpuTestStack, PLA) {
    const auto op = Opcode(PLA, Implicit, 1, 4);
    cpu.WriteByte(BASE_PC, 0xFF);

    auto tester = [&] (Byte m, Flag expZ, Flag expN) {
        cpu.StackPage = 0x0100;
        cpu.S = 0xEF;
        cpu.WriteByte(0x01F0, m);

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(m, cpu.A);
        EXPECT_EQ(0xF0, cpu.S);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expN, cpu.N);
    };

    tester(0x20, 0, 0);
    tester(0x00, 1, 0);
    tester(0x80, 0, 1);
}

TEST_F(CpuTestStack, PLP) {
    const auto op = Opcode(PLP, Implicit, 1, 4);
    cpu.WriteByte(BASE_PC, 0xFF);

    auto tester = [&] (Byte m, Flag expN, Flag expV, Flag expD, Flag expI, Flag expZ, Flag expC) {
        cpu.StackPage = 0x0100;
        cpu.S = 0xEF;
        cpu.WriteByte(0x01F0, m);

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(0xF0, cpu.S);
        EXPECT_EQ(expN, cpu.N);
        EXPECT_EQ(expV, cpu.V);
        EXPECT_EQ(expD, cpu.D);
        EXPECT_EQ(expI, cpu.I);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expC, cpu.C);
    };

    tester(0xB5, 1, 0, 0, 1, 0, 1); // Status 10xx0101b
    tester(0x6A, 0, 1, 1, 0, 1, 0); // Status 01xx1010b
}

TEST_F(CpuTestStack, PHP) {
    const auto op = Opcode(PHP, Implicit, 1, 3);
    cpu.WriteByte(BASE_PC, 0xFF);

    auto tester = [&] (Flag n, Flag v, Flag d, Flag i, Flag z, Flag c) {
        cpu.N = n;
        cpu.V = v;
        cpu.D = d;
        cpu.I = i;
        cpu.Z = z;
        cpu.C = c;
        cpu.StackPage = 0x0100;
        cpu.S = 0xF0;

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        const auto expected = n * 0x80 + v * 0x40 + d * 0x08 +
                              i * 0x04 + z * 0x02 + c * 0x01;
        const auto flagsMask = 0xCF;
        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expected & flagsMask, cpu.ReadByte(0x01F0) & flagsMask);
        EXPECT_EQ(0xEF, cpu.S);
    };

    tester(0, 1, 1, 0, 1, 0);
    tester(1, 0, 0, 1, 0, 1);
}
