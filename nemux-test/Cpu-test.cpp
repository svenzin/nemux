/*
 * Cpu-test.cpp
 *
 *  Created on: 19 Jun 2013
 *      Author: scorder
 */

#include "gtest/gtest.h"

#include "Cpu.h"

//#include <sstream>
#include <vector>
#include <map>

using namespace std;

////////////////////////////////////////////////////////////////
//
//template <size_t Size> class MockedSection : public Section<Size> {
//public:
//    virtual ~MockedSection() {}
//    virtual bool Initialize() { return false; }
//};

class CpuTest : public ::testing::Test {
public:
    static const Register_16 BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTest() : cpu("6502") {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
    }

    Cpu cpu;

    void Test_ClearFlag(InstructionName inst, Flag &f) {
        f = 1;
        cpu.Execute(Opcode(inst, AddressingType::Implicit, 1, 2));//, {});

        EXPECT_EQ(BASE_PC + 1, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
        EXPECT_EQ(0, f);
    }

    template<typename T> void Test_Decrement(T &target, Opcode op) {
        target = 10;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + 1 * op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 1 * op.Cycles, cpu.Ticks);
        EXPECT_EQ(9, target);
        EXPECT_EQ(0, cpu.Z);
        EXPECT_EQ(0, cpu.N);

        target = 1;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + 2 * op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 2 * op.Cycles, cpu.Ticks);
        EXPECT_EQ(0, target);
        EXPECT_EQ(1, cpu.Z);
        EXPECT_EQ(0, cpu.N);

        target = 0;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + 3 * op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 3 * op.Cycles, cpu.Ticks);
        EXPECT_EQ(-1, target);
        EXPECT_EQ(0, cpu.Z);
        EXPECT_EQ(1, cpu.N);

        target = -128;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + 4 * op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 4 * op.Cycles, cpu.Ticks);
        EXPECT_EQ(127, target);
        EXPECT_EQ(0, cpu.Z);
        EXPECT_EQ(0, cpu.N);
    }

};

void FillMapper(Mapper & m, const Address & a, const vector<Byte> & data) {
    for (auto i = 0; i < data.size(); ++i) {
        m.SetByteAt(a + i, data[i]);
    }
}

//TEST_F(CpuTest, FetchOpdata) {
//    Mapper m("Test", 20);
//    FillMapper(m, {
//        00, 01, 02, 03, 04, 05, 06, 07, 08, 09,
//        10, 11, 12, 13, 14, 15, 16, 17, 18, 19
//    });
////    cpu.Fetch()
//}
//
//TEST_F(CpuTest, FetchOpdata) {
//
//}

TEST_F(CpuTest, BIT_ZeroPage) {
    Mapper map("Test", 0x400);
    for (auto i = 0; i < 0x100; ++i) {
        map.SetByteAt(i, i);
        map.SetByteAt(0x100 + i, i);
    }
    cpu.Memory = map;

    for (auto a = 0; a < 0x100; ++a) {
        for (auto m = 0; m < 0x100; ++m) {
            // ZeroPage
            cpu.PC = 0x200;
            cpu.Memory.SetByteAt(0x200, 0xFF);
            cpu.Memory.SetByteAt(0x201, m);

            cpu.A = a;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(Opcode(InstructionName::BIT, AddressingType::ZeroPage, 2, 3));

            EXPECT_EQ(0x200 + 2, cpu.PC);
            EXPECT_EQ(BASE_TICKS + 3, cpu.Ticks);
            EXPECT_EQ((m & a   ) == 0 ? 1 : 0, cpu.Z);
            EXPECT_EQ((m & 0x40) == 0 ? 0 : 1, cpu.V);
            EXPECT_EQ((m & 0x80) == 0 ? 0 : 1, cpu.N);
        }
    }
}

TEST_F(CpuTest, BIT_Absolute) {
    Mapper map("Test", 0x400);
    for (auto i = 0; i < 0x100; ++i) {
        map.SetByteAt(i, i);
        map.SetByteAt(0x100 + i, i);
    }
    cpu.Memory = map;

    for (auto a = 0; a < 0x100; ++a) {
        for (auto m = 0; m < 0x100; ++m) {
            cpu.PC = 0x200;
            cpu.Memory.SetByteAt(0x200, 0xFF);
            cpu.Memory.SetWordAt(0x201, 0x100 + m);

            cpu.A = a;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(Opcode(InstructionName::BIT, AddressingType::Absolute, 3, 4));

            EXPECT_EQ(0x200 + 3, cpu.PC);
            EXPECT_EQ(BASE_TICKS + 4, cpu.Ticks);
            EXPECT_EQ((m & a ) == 0 ? 1 : 0, cpu.Z);
            EXPECT_EQ((m & 0x40) == 0 ? 0 : 1, cpu.V);
            EXPECT_EQ((m & 0x80) == 0 ? 0 : 1, cpu.N);
        }
    }
}

TEST_F(CpuTest, CLC) {
    Test_ClearFlag(InstructionName::CLC, cpu.C);
}

TEST_F(CpuTest, CLD) {
    Test_ClearFlag(InstructionName::CLD, cpu.D);
}

TEST_F(CpuTest, CLI) {
    Test_ClearFlag(InstructionName::CLI, cpu.I);
}

TEST_F(CpuTest, CLV) {
    Test_ClearFlag(InstructionName::CLV, cpu.V);
}

TEST_F(CpuTest, DEX) {
    Test_Decrement(cpu.X, Opcode(InstructionName::DEX, AddressingType::Implicit, 1, 2));
}

TEST_F(CpuTest, DEY) {
    Test_Decrement(cpu.Y, Opcode(InstructionName::DEY, AddressingType::Implicit, 1, 2));
}

TEST_F(CpuTest, NOP) {
    cpu.Execute(Opcode(InstructionName::NOP, AddressingType::Implicit, 1, 2));

    EXPECT_EQ(BASE_PC + 1, cpu.PC);
    EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
}


