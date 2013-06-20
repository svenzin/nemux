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

        Mapper map("Test", 0x400);
        cpu.Memory = map;
    }

    Cpu cpu;

    void Test_ClearFlag(InstructionName inst, Flag &f) {
        f = 1;
        cpu.Execute(Opcode(inst, AddressingType::Implicit, 1, 2));//, {});

        EXPECT_EQ(BASE_PC + 1, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
        EXPECT_EQ(0, f);
    }

    template<typename Getter, typename Setter>
    void Test_Decrement(Getter get, Setter set, Opcode op) {
        const auto initialPC = cpu.PC;
        const auto initialTicks = cpu.Ticks;

        set(10);
        cpu.PC = initialPC;
        cpu.Ticks = initialTicks;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(9, get());
        EXPECT_EQ(0, cpu.Z);
        EXPECT_EQ(0, cpu.N);

        set(1);
        cpu.PC = initialPC;
        cpu.Ticks = initialTicks;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(0, get());
        EXPECT_EQ(1, cpu.Z);
        EXPECT_EQ(0, cpu.N);

        set(0);
        cpu.PC = initialPC;
        cpu.Ticks = initialTicks;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(-1, get());
        EXPECT_EQ(0, cpu.Z);
        EXPECT_EQ(1, cpu.N);

        set(-128);
        cpu.PC = initialPC;
        cpu.Ticks = initialTicks;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(127, get());
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
    for (auto i = 0; i < 0x100; ++i) {
        cpu.Memory.SetByteAt(i, i);
        cpu.Memory.SetByteAt(0x100 + i, i);
    }

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
    for (auto i = 0; i < 0x100; ++i) {
        cpu.Memory.SetByteAt(i, i);
        cpu.Memory.SetByteAt(0x100 + i, i);
    }

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

TEST_F(CpuTest, DEC_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Decrement(
        [&]              { return cpu.Memory.GetByteAt(0x20); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x20, value); },
        Opcode(InstructionName::DEC, AddressingType::ZeroPage, 2, 5)
    );
}

TEST_F(CpuTest, DEC_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_Decrement(
        [&]              { return cpu.Memory.GetByteAt(0x28); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x28, value); },
        Opcode(InstructionName::DEC, AddressingType::ZeroPageX, 2, 6)
    );
}

TEST_F(CpuTest, DEC_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_Decrement(
        [&]              { return cpu.Memory.GetByteAt(0x0120); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0120, value); },
        Opcode(InstructionName::DEC, AddressingType::Absolute, 2, 6)
    );
}

TEST_F(CpuTest, DEC_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_Decrement(
        [&]              { return cpu.Memory.GetByteAt(0x0128); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0128, value); },
        Opcode(InstructionName::DEC, AddressingType::AbsoluteX, 2, 7)
    );
}

TEST_F(CpuTest, DEX) {
    Test_Decrement(
        [&]              { return cpu.X; },
        [&] (Byte value) { cpu.X = value; },
        Opcode(InstructionName::DEX, AddressingType::Implicit, 1, 2)
    );
}

TEST_F(CpuTest, DEY) {
    Test_Decrement(
        [&]              { return cpu.Y; },
        [&] (Byte value) { cpu.Y = value; },
        Opcode(InstructionName::DEY, AddressingType::Implicit, 1, 2)
    );
}

TEST_F(CpuTest, NOP) {
    cpu.Execute(Opcode(InstructionName::NOP, AddressingType::Implicit, 1, 2));

    EXPECT_EQ(BASE_PC + 1, cpu.PC);
    EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
}


