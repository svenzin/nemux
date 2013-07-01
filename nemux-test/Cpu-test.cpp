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
    static const Word BASE_PC = 10;
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
        EXPECT_EQ(Flag{0}, f);
    }

    template<typename Getter, typename Setter>
    void Test_DEC(Getter get, Setter set, Opcode op) {
        const auto initialPC = cpu.PC;
        const auto initialTicks = cpu.Ticks;

        set(10);
        cpu.PC = initialPC;
        cpu.Ticks = initialTicks;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(Byte{9}, get());
        EXPECT_EQ(Flag{0}, cpu.Z);
        EXPECT_EQ(Flag{0}, cpu.N);

        set(1);
        cpu.PC = initialPC;
        cpu.Ticks = initialTicks;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(Byte{0}, get());
        EXPECT_EQ(Flag{1}, cpu.Z);
        EXPECT_EQ(Flag{0}, cpu.N);

        set(0);
        cpu.PC = initialPC;
        cpu.Ticks = initialTicks;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(Byte{0xFF}, get());
        EXPECT_EQ(Flag{0}, cpu.Z);
        EXPECT_EQ(Flag{1}, cpu.N);

        set(-128);
        cpu.PC = initialPC;
        cpu.Ticks = initialTicks;
        cpu.Execute(op);
        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(Byte{127}, get());
        EXPECT_EQ(Flag{0}, cpu.Z);
        EXPECT_EQ(Flag{0}, cpu.N);
    }

    template<typename Getter, typename Setter>
    void Test_ASL(Getter get, Setter set, Opcode op) {
        auto Tester = [&] (Byte m, Byte expM, Flag expC, Flag expZ, Flag expN) {
            set(m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, get());
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        Tester(0x24, 0x48, 0, 0, 0); // Shift

        Tester(0x00, 0x00, 0, 1, 0); // Zero flag
        Tester(0x80, 0x00, 1, 1, 0); // Zero flag

        Tester(0x01, 0x02, 0, 0, 0); // Carry flag
        Tester(0x81, 0x02, 1, 0, 0); // Carry flag

        Tester(0xA0, 0x40, 1, 0, 0); // Negative flag
        Tester(0xF0, 0xE0, 1, 0, 1); // Negative flag
    }

    template<typename Setter>
    void Test_AND(Setter set, Opcode op, bool extra) {
        const auto expectedCycles = extra ? op.Cycles + 1 : op.Cycles;
        auto tester = [&] (Byte a, Byte m, Byte expA, Flag expZ, Flag expN) {
            set(m);
            cpu.A = a;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + expectedCycles, cpu.Ticks);
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x66, 0x00, 0x00, 1, 0); // Zero
        tester(0xFF, 0x80, 0x80, 0, 1); // Negative
        tester(0xAA, 0x24, 0x20, 0, 0); // Normal
    }

    template<typename Setter>
    void Test_ADC(Setter set, Opcode op, bool extra) {
        const auto expectedCycles = extra ? op.Cycles + 1 : op.Cycles;
        auto tester = [&] (Byte a, Byte m, Flag c, Byte expA, Flag expC, Flag expZ, Flag expV, Flag expN) {
            set(m);
            cpu.A = a;
            cpu.C = c;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + expectedCycles, cpu.Ticks);
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expV, cpu.V);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x10, 0x20, 0, 0x30, 0, 0, 0, 0); // pos pos > pos
        tester(0x10, 0x20, 1, 0x31, 0, 0, 0, 0); // pos pos C > pos
        tester(0x00, 0x00, 0, 0x00, 0, 1, 0, 0); // Zero
        tester(0x7F, 0x80, 1, 0x00, 1, 1, 0, 0); // Zero by carry
        tester(0x80, 0x80, 0, 0x00, 1, 1, 1, 0); // Zero by overflow
        tester(0x20, 0xF0, 0, 0x10, 1, 0, 0, 0); // Carry w/o overflow w/o sign change
        tester(0xF0, 0x20, 0, 0x10, 1, 0, 0, 0); // Carry w/o overflow w/  sign change
        tester(0xA0, 0xA0, 0, 0x40, 1, 0, 1, 0); // Carry with overflow
        tester(0x70, 0x10, 0, 0x80, 0, 0, 1, 1); // Overflow pos > neg
        tester(0xB0, 0xB0, 0, 0x60, 1, 0, 1, 0); // Overflow neg > pos
        tester(0x00, 0xF0, 0, 0xF0, 0, 0, 0, 1); // Negative
    }

    void Test_BCC() {
        const auto op = Opcode(InstructionName::BCC, AddressingType::Relative, 2, 2);

        auto tester = [&] (Word pc, Byte offset, Flag c, Word expPC, int extra) {
            cpu.Memory.SetByteAt(pc, 0xFF);
            cpu.Memory.SetByteAt(pc + 1, offset);
            cpu.C = c;

            cpu.PC = pc;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(expPC, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
        };

        tester(0x0080, 0x20, 1, 0x0082, 0); // Fail
        tester(0x0080, 0x20, 0, 0x00A0, 1); // Success, positive offset
        tester(0x0080, 0xE0, 0, 0x0060, 1); // Success, negative offset
        tester(0x00F0, 0x20, 0, 0x0110, 3); // Success, positive offset, crossing page
        tester(0x0110, 0xE0, 0, 0x00F0, 3); // Success, negative offset, crossing page
    };

    void Test_BCS() {
        const auto op = Opcode(InstructionName::BCS, AddressingType::Relative, 2, 2);

        auto tester = [&] (Word pc, Byte offset, Flag c, Word expPC, int extra) {
            cpu.Memory.SetByteAt(pc, 0xFF);
            cpu.Memory.SetByteAt(pc + 1, offset);
            cpu.C = c;

            cpu.PC = pc;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(expPC, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
        };

        tester(0x0080, 0x20, 0, 0x0082, 0); // Fail
        tester(0x0080, 0x20, 1, 0x00A0, 1); // Success, positive offset
        tester(0x0080, 0xE0, 1, 0x0060, 1); // Success, negative offset
        tester(0x00F0, 0x20, 1, 0x0110, 3); // Success, positive offset, crossing page
        tester(0x0110, 0xE0, 1, 0x00F0, 3); // Success, negative offset, crossing page
    };

    void Test_BEQ() {
        const auto op = Opcode(InstructionName::BEQ, AddressingType::Relative, 2, 2);

        auto tester = [&] (Word pc, Byte offset, Flag z, Word expPC, int extra) {
            cpu.Memory.SetByteAt(pc, 0xFF);
            cpu.Memory.SetByteAt(pc + 1, offset);
            cpu.Z = z;

            cpu.PC = pc;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(expPC, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
        };

        tester(0x0080, 0x20, 0, 0x0082, 0); // Fail
        tester(0x0080, 0x20, 1, 0x00A0, 1); // Success, positive offset
        tester(0x0080, 0xE0, 1, 0x0060, 1); // Success, negative offset
        tester(0x00F0, 0x20, 1, 0x0110, 3); // Success, positive offset, crossing page
        tester(0x0110, 0xE0, 1, 0x00F0, 3); // Success, negative offset, crossing page
    };

    void Test_BMI() {
            const auto op = Opcode(InstructionName::BMI, AddressingType::Relative, 2, 2);

            auto tester = [&] (Word pc, Byte offset, Flag n, Word expPC, int extra) {
                cpu.Memory.SetByteAt(pc, 0xFF);
                cpu.Memory.SetByteAt(pc + 1, offset);
                cpu.N = n;

                cpu.PC = pc;
                cpu.Ticks = BASE_TICKS;
                cpu.Execute(op);

                EXPECT_EQ(expPC, cpu.PC);
                EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            };

            tester(0x0080, 0x20, 0, 0x0082, 0); // Fail
            tester(0x0080, 0x20, 1, 0x00A0, 1); // Success, positive offset
            tester(0x0080, 0xE0, 1, 0x0060, 1); // Success, negative offset
            tester(0x00F0, 0x20, 1, 0x0110, 3); // Success, positive offset, crossing page
            tester(0x0110, 0xE0, 1, 0x00F0, 3); // Success, negative offset, crossing page
        };

};

TEST_F(CpuTest, BCC) {
    Test_BCC();
}

TEST_F(CpuTest, BCS) {
    Test_BCS();
}

TEST_F(CpuTest, BEQ) {
    Test_BEQ();
}

TEST_F(CpuTest, BMI) {
    Test_BMI();
}

TEST_F(CpuTest, ASL_Accumulator) {
    Test_ASL(
        [&]              { return cpu.A; },
        [&] (Byte value) { cpu.A = value; },
        Opcode(InstructionName::ASL, AddressingType::Accumulator, 1, 2));
}

TEST_F(CpuTest, ASL_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_ASL(
        [&]              { return cpu.Memory.GetByteAt(0x20); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x20, value); },
        Opcode(InstructionName::ASL, AddressingType::ZeroPage, 2, 5));
}

TEST_F(CpuTest, ASL_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_ASL(
        [&]              { return cpu.Memory.GetByteAt(0x28); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x28, value); },
        Opcode(InstructionName::ASL, AddressingType::ZeroPageX, 2, 6));
}

TEST_F(CpuTest, ASL_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_ASL(
        [&]              { return cpu.Memory.GetByteAt(0x0120); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0120, value); },
        Opcode(InstructionName::ASL, AddressingType::Absolute, 3, 6));
}

TEST_F(CpuTest, ASL_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_ASL(
        [&]              { return cpu.Memory.GetByteAt(0x0128); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0128, value); },
        Opcode(InstructionName::ASL, AddressingType::AbsoluteX, 3, 7));
}

TEST_F(CpuTest, AND_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(BASE_PC + 1, m); },
             Opcode(InstructionName::AND, AddressingType::Immediate, 2, 2),
             false);
}

TEST_F(CpuTest, AND_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x20, m); },
             Opcode(InstructionName::AND, AddressingType::ZeroPage, 2, 3),
             false);
}

TEST_F(CpuTest, AND_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x28, m); },
             Opcode(InstructionName::AND, AddressingType::ZeroPageX, 2, 4),
             false);
}

TEST_F(CpuTest, AND_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0120, m); },
             Opcode(InstructionName::AND, AddressingType::Absolute, 3, 4),
             false);
}

TEST_F(CpuTest, AND_AbsoluteX) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0128, m); },
             Opcode(InstructionName::AND, AddressingType::AbsoluteX, 3, 4, true),
             false);
}

TEST_F(CpuTest, AND_AbsoluteX_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0210, m); },
             Opcode(InstructionName::AND, AddressingType::AbsoluteX, 3, 4, true),
             true);
}

TEST_F(CpuTest, AND_AbsoluteY) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0128, m); },
             Opcode(InstructionName::AND, AddressingType::AbsoluteY, 3, 4, true),
             false);
}

TEST_F(CpuTest, AND_AbsoluteY_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0210, m); },
             Opcode(InstructionName::AND, AddressingType::AbsoluteY, 3, 4, true),
             true);
}

TEST_F(CpuTest, AND_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.Memory.SetWordAt(0x28, 0x0120);

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0120, m); },
             Opcode(InstructionName::AND, AddressingType::IndexedIndirect, 2, 6),
             false);
}

TEST_F(CpuTest, AND_IndirectIndexed) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x120);
    cpu.Y = 0x08;
    cpu.Memory.SetWordAt(0x0128, 0x0200);

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0200, m); },
             Opcode(InstructionName::AND, AddressingType::IndirectIndexed, 2, 5, true),
             false);
}

TEST_F(CpuTest, AND_IndirectIndexed_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x120);
    cpu.Y = 0xF0;
    cpu.Memory.SetWordAt(0x0210, 0x0200);

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0200, m); },
             Opcode(InstructionName::AND, AddressingType::IndirectIndexed, 2, 5, true),
             true);
}

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

TEST_F(CpuTest, ADC_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(BASE_PC + 1, value); },
        Opcode(InstructionName::ADC, AddressingType::Immediate, 2, 2),
        false
        );
}

TEST_F(CpuTest, ADC_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(0x20, value); },
        Opcode(InstructionName::ADC, AddressingType::ZeroPage, 2, 3),
        false
        );
}

TEST_F(CpuTest, ADC_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(0x28, value); },
        Opcode(InstructionName::ADC, AddressingType::ZeroPageX, 2, 4),
        false
        );
}

TEST_F(CpuTest, ADC_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0120, value); },
        Opcode(InstructionName::ADC, AddressingType::Absolute, 3, 4),
        false
        );
}

TEST_F(CpuTest, ADC_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0128, value); },
        Opcode(InstructionName::ADC, AddressingType::AbsoluteX, 3, 4, true),
        false
        );
}

TEST_F(CpuTest, ADC_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0210, value); },
        Opcode(InstructionName::ADC, AddressingType::AbsoluteX, 3, 4, true),
        true
        );
}

TEST_F(CpuTest, ADC_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0128, value); },
        Opcode(InstructionName::ADC, AddressingType::AbsoluteY, 3, 4, true),
        false
        );
}

TEST_F(CpuTest, ADC_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0210, value); },
        Opcode(InstructionName::ADC, AddressingType::AbsoluteY, 3, 4, true),
        true
        );
}

TEST_F(CpuTest, ADC_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.Memory.SetWordAt(0x28, 0x0120);

    Test_ADC(
        [&] (Byte m) { cpu.Memory.SetByteAt(0x0120, m); },
        Opcode(InstructionName::ADC, AddressingType::IndexedIndirect, 2, 6),
        false
        );
}

TEST_F(CpuTest, ADC_IndirectIndexed) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x120);
    cpu.Y = 0x08;
    cpu.Memory.SetWordAt(0x0128, 0x0200);

    Test_ADC(
        [&] (Byte m) { cpu.Memory.SetByteAt(0x0200, m); },
        Opcode(InstructionName::ADC, AddressingType::IndirectIndexed, 2, 5, true),
        false
        );
}

TEST_F(CpuTest, ADC_IndirectIndexed_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x120);
    cpu.Y = 0xF0;
    cpu.Memory.SetWordAt(0x0210, 0x0200);

    Test_ADC(
        [&] (Byte m) { cpu.Memory.SetByteAt(0x0200, m); },
        Opcode(InstructionName::ADC, AddressingType::IndirectIndexed, 2, 5, true),
        true
        );
}

TEST_F(CpuTest, DEC_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_DEC(
        [&]              { return cpu.Memory.GetByteAt(0x20); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x20, value); },
        Opcode(InstructionName::DEC, AddressingType::ZeroPage, 2, 5)
    );
}

TEST_F(CpuTest, DEC_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_DEC(
        [&]              { return cpu.Memory.GetByteAt(0x28); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x28, value); },
        Opcode(InstructionName::DEC, AddressingType::ZeroPageX, 2, 6)
    );
}

TEST_F(CpuTest, DEC_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_DEC(
        [&]              { return cpu.Memory.GetByteAt(0x0120); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0120, value); },
        Opcode(InstructionName::DEC, AddressingType::Absolute, 2, 6)
    );
}

TEST_F(CpuTest, DEC_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_DEC(
        [&]              { return cpu.Memory.GetByteAt(0x0128); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0128, value); },
        Opcode(InstructionName::DEC, AddressingType::AbsoluteX, 2, 7)
    );
}

TEST_F(CpuTest, DEX) {
    Test_DEC(
        [&]              { return cpu.X; },
        [&] (Byte value) { cpu.X = value; },
        Opcode(InstructionName::DEX, AddressingType::Implicit, 1, 2)
    );
}

TEST_F(CpuTest, DEY) {
    Test_DEC(
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


