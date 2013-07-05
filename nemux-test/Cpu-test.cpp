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
#include <array>
#include <functional>

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

    void Test_SetFlag(InstructionName inst, Flag &f) {
        f = 0;
        cpu.Execute(Opcode(inst, AddressingType::Implicit, 1, 2));//, {});

        EXPECT_EQ(BASE_PC + 1, cpu.PC);
        EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
        EXPECT_EQ(Flag{1}, f);
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
    void Test_INC(Getter get, Setter set, Opcode op) {
        auto tester = [&] (Byte m, Byte expM, Flag expZ, Flag expN) {
            set(m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, get());
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x10, 0x11, 0, 0);
        tester(0xFF, 0x00, 1, 0);
        tester(0x7F, 0x80, 0, 1);
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

    template<typename Setter>
    void Test_Branch(Setter set, Flag success, Flag failure, Opcode op) {
        auto tester = [&] (Word pc, Byte offset, Flag c, Word expPC, int extra) {
            cpu.Memory.SetByteAt(pc, 0xFF);
            cpu.Memory.SetByteAt(pc + 1, offset);
            set(c);

            cpu.PC = pc;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(expPC, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
        };

        tester(0x0080, 0x20, failure, 0x0082, 0); // Fail
        tester(0x0080, 0x20, success, 0x00A0, 1); // Success, positive offset
        tester(0x0080, 0xE0, success, 0x0060, 1); // Success, negative offset
        tester(0x00F0, 0x20, success, 0x0110, 3); // Success, positive offset, crossing page
        tester(0x0110, 0xE0, success, 0x00F0, 3); // Success, negative offset, crossing page
    }

    template<typename Getter, typename Setter>
    void Test_Set(Getter get, Setter set, Opcode op) {
        set(0x80);
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(0x80, get());
    }

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

    template<typename Setter>
    void Test_Compare(Setter set, Opcode op, int extra) {
        auto tester = [&] (Byte r, Byte m, Flag expC, Flag expZ, Flag expN) {
            set(r, m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x20, 0x10, 1, 0, 0); // Greater
        tester(0x20, 0x20, 1, 1, 0); // Equal
        tester(0x20, 0x40, 0, 0, 1); // Lower

        tester(0xF0, 0xE0, 1, 0, 0); // Greater
        tester(0xF0, 0xF0, 1, 1, 0); // Equal
        tester(0xF0, 0xF8, 0, 0, 1); // Lower
    }

    template<typename Setter>
    void Test_EOR(Setter set, Opcode op, int extra) {
        auto tester = [&] (Byte a, Byte m, Byte expA, Flag expZ, Flag expN) {
            set(m);
            cpu.A = a;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x0C, 0x0A, 0x06, 0, 0); // 1100b XOR 1010b = 0110b
        tester(0x0C, 0x0C, 0x00, 1, 0); // 1100b XOR 1100b = 0000b
        tester(0x8C, 0x0C, 0x80, 0, 1); // 10001100b XOR 00001100b = 10000000b
    }

    template<typename Getter, typename Setter>
    void Test_Load(Getter get, Setter set, Opcode op, int extra) {
        auto tester = [&] (Byte m, Flag expZ, Flag expN) {
            set(m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            EXPECT_EQ(m, get());
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expN, cpu.N);
        };

        tester(0x20, 0, 0);
        tester(0x00, 1, 0);
        tester(0x80, 0, 1);
    }

    function<void (Byte)> Setter(Word a) {
        return [=] (Byte value) { cpu.Memory.SetByteAt(a, value); };
    }
    function<Byte ()> Getter(Byte & b) {
        return [&] () { return b; };
    }
};

TEST_F(CpuTest, LDX_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    Test_Load(Getter(cpu.X), Setter(BASE_PC + 1),
              Opcode(InstructionName::LDX, AddressingType::Immediate, 2, 2), 0);
}

TEST_F(CpuTest, LDX_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_Load(Getter(cpu.X), Setter(0x0020),
              Opcode(InstructionName::LDX, AddressingType::ZeroPage, 2, 3), 0);
}

TEST_F(CpuTest, LDX_ZeroPageY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Y = 0x08;
    Test_Load(Getter(cpu.X), Setter(0x0028),
              Opcode(InstructionName::LDX, AddressingType::ZeroPageY, 2, 4), 0);
}

TEST_F(CpuTest, LDX_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    Test_Load(Getter(cpu.X), Setter(0x0120),
              Opcode(InstructionName::LDX, AddressingType::Absolute, 3, 4), 0);
}

TEST_F(CpuTest, LDX_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_Load(Getter(cpu.X), Setter(0x0128),
              Opcode(InstructionName::LDX, AddressingType::AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTest, LDX_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_Load(Getter(cpu.X), Setter(0x0210),
              Opcode(InstructionName::LDX, AddressingType::AbsoluteY, 3, 4), 1);
}


TEST_F(CpuTest, LDY_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    Test_Load(Getter(cpu.Y), Setter(BASE_PC + 1),
              Opcode(InstructionName::LDY, AddressingType::Immediate, 2, 2), 0);
}

TEST_F(CpuTest, LDY_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_Load(Getter(cpu.Y), Setter(0x0020),
              Opcode(InstructionName::LDY, AddressingType::ZeroPage, 2, 3), 0);
}

TEST_F(CpuTest, LDY_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_Load(Getter(cpu.Y), Setter(0x0028),
              Opcode(InstructionName::LDY, AddressingType::ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTest, LDY_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    Test_Load(Getter(cpu.Y), Setter(0x0120),
              Opcode(InstructionName::LDY, AddressingType::Absolute, 3, 4), 0);
}

TEST_F(CpuTest, LDY_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_Load(Getter(cpu.Y), Setter(0x0128),
              Opcode(InstructionName::LDY, AddressingType::AbsoluteX, 3, 4), 0);
}

TEST_F(CpuTest, LDY_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_Load(Getter(cpu.Y), Setter(0x0210),
              Opcode(InstructionName::LDY, AddressingType::AbsoluteX, 3, 4), 1);
}

TEST_F(CpuTest, EOR_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    Test_EOR(Setter(BASE_PC + 1), Opcode(InstructionName::EOR, AddressingType::Immediate, 2, 2), 0);
}

TEST_F(CpuTest, EOR_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    Test_EOR(Setter(0x0020), Opcode(InstructionName::EOR, AddressingType::ZeroPage, 2, 3), 0);
}

TEST_F(CpuTest, EOR_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_EOR(Setter(0x0028), Opcode(InstructionName::EOR, AddressingType::ZeroPageX, 2, 4), 0);
}

TEST_F(CpuTest, EOR_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    Test_EOR(Setter(0x0120), Opcode(InstructionName::EOR, AddressingType::Absolute, 3, 4), 0);
}

TEST_F(CpuTest, EOR_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_EOR(Setter(0x0128), Opcode(InstructionName::EOR, AddressingType::AbsoluteX, 3, 4), 0);
}

TEST_F(CpuTest, EOR_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_EOR(Setter(0x0210), Opcode(InstructionName::EOR, AddressingType::AbsoluteX, 3, 4), 1);
}

TEST_F(CpuTest, EOR_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_EOR(Setter(0x0128), Opcode(InstructionName::EOR, AddressingType::AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTest, EOR_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_EOR(Setter(0x0210), Opcode(InstructionName::EOR, AddressingType::AbsoluteY, 3, 4), 1);
}

TEST_F(CpuTest, EOR_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.Memory.SetWordAt(0x28, 0x0120);
    Test_EOR(Setter(0x0120), Opcode(InstructionName::EOR, AddressingType::IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTest, EOR_IndirectIndexed) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    cpu.Memory.SetWordAt(0x0128, 0x0200);
    Test_EOR(Setter(0x0200), Opcode(InstructionName::EOR, AddressingType::IndirectIndexed, 2, 5), 0);
}

TEST_F(CpuTest, EOR_IndirectIndexed_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    cpu.Memory.SetWordAt(0x0210, 0x0200);
    Test_EOR(Setter(0x0200), Opcode(InstructionName::EOR, AddressingType::IndirectIndexed, 2, 5), 1);
}

TEST_F(CpuTest, CPX_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.X = r; cpu.Memory.SetByteAt(BASE_PC + 1, m); },
        Opcode(InstructionName::CPX, AddressingType::Immediate, 2, 2), 0
    );
}

TEST_F(CpuTest, CPX_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.X = r; cpu.Memory.SetByteAt(0x20, m); },
        Opcode(InstructionName::CPX, AddressingType::ZeroPage, 2, 3), 0
    );
}

TEST_F(CpuTest, CPX_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.X = r; cpu.Memory.SetByteAt(0x0120, m); },
        Opcode(InstructionName::CPX, AddressingType::Absolute, 3, 4), 0
    );
}

TEST_F(CpuTest, CPY_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.Y = r; cpu.Memory.SetByteAt(BASE_PC + 1, m); },
        Opcode(InstructionName::CPY, AddressingType::Immediate, 2, 2), 0
    );
}

TEST_F(CpuTest, CPY_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.Y = r; cpu.Memory.SetByteAt(0x20, m); },
        Opcode(InstructionName::CPY, AddressingType::ZeroPage, 2, 3), 0
    );
}

TEST_F(CpuTest, CPY_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.Y = r; cpu.Memory.SetByteAt(0x0120, m); },
        Opcode(InstructionName::CPY, AddressingType::Absolute, 3, 4), 0
    );
}

TEST_F(CpuTest, CMP_Immediate) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(BASE_PC + 1, m); },
        Opcode(InstructionName::CMP, AddressingType::Immediate, 2, 2), 0
    );
}

TEST_F(CpuTest, CMP_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x20);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(0x20, m); },
        Opcode(InstructionName::CMP, AddressingType::ZeroPage, 2, 3), 0
    );
}

TEST_F(CpuTest, CMP_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(0x28, m); },
        Opcode(InstructionName::CMP, AddressingType::ZeroPageX, 2, 4), 0
    );
}

TEST_F(CpuTest, CMP_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(0x0120, m); },
        Opcode(InstructionName::CMP, AddressingType::Absolute, 3, 4), 0
    );
}

TEST_F(CpuTest, CMP_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(0x0128, m); },
        Opcode(InstructionName::CMP, AddressingType::AbsoluteX, 3, 4), 0
    );
}

TEST_F(CpuTest, CMP_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(0x0210, m); },
        Opcode(InstructionName::CMP, AddressingType::AbsoluteX, 3, 4), 1
    );
}

TEST_F(CpuTest, CMP_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(0x0128, m); },
        Opcode(InstructionName::CMP, AddressingType::AbsoluteY, 3, 4), 0
    );
}

TEST_F(CpuTest, CMP_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(0x0210, m); },
        Opcode(InstructionName::CMP, AddressingType::AbsoluteY, 3, 4), 1
    );
}

TEST_F(CpuTest, CMP_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.Memory.SetWordAt(0x28, 0x0120);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(0x0120, m); },
        Opcode(InstructionName::CMP, AddressingType::IndexedIndirect, 2, 6), 0
    );
}

TEST_F(CpuTest, CMP_IndirectIndexed) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x120);
    cpu.Y = 0x08;
    cpu.Memory.SetWordAt(0x0128, 0x0200);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(0x0200, m); },
        Opcode(InstructionName::CMP, AddressingType::IndirectIndexed, 2, 5), 0
    );
}

TEST_F(CpuTest, CMP_IndirectIndexed_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x120);
    cpu.Y = 0xF0;
    cpu.Memory.SetWordAt(0x0210, 0x0200);

    Test_Compare(
        [&] (Byte r, Byte m) { cpu.A = r; cpu.Memory.SetByteAt(0x0200, m); },
        Opcode(InstructionName::CMP, AddressingType::IndirectIndexed, 2, 5), 1
    );
}

//
//TEST_F(CpuTest, Opcode_Instruction) {
//    typedef InstructionName i;
//    std::array<InstructionName, 0x100> opcodes {
///*           x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xA  xB  xC  xD  xE  xF */
///* 0x */    i::BRK, i::ORA, i::UNK, i::UNK, i::UNK, i::ORA, i::ASL, i::UNK, i::PHP, i::ORA, i::ASL, i::UNK, i::UNK, i::ORA, i::ASL, i::UNK,
///* 1x */    i::BPL, i::ORA, i::UNK, i::UNK, i::UNK, i::ORA, i::ASL, i::UNK, i::CLC, i::ORA, i::UNK, i::UNK, i::UNK, i::ORA, i::ASL, i::UNK,
///* 2x */    i::JSR, i::AND, i::UNK, i::UNK, i::BIT, i::AND, i::ROL, i::UNK, i::PLP, i::AND, i::ROL, i::UNK, i::BIT, i::AND, i::ROL, i::UNK,
///* 3x */    i::BMI, i::AND, i::UNK, i::UNK, i::UNK, i::AND, i::ROL, i::UNK, i::SEC, i::AND, i::UNK, i::UNK, i::UNK, i::AND, i::ROL, i::UNK,
///* 4x */    i::RTI, i::EOR, i::UNK, i::UNK, i::UNK, i::EOR, i::LSR, i::UNK, i::PHA, i::EOR, i::LSR, i::UNK, i::JMP, i::EOR, i::LSR, i::UNK,
///* 5x */    i::BVC, i::EOR, i::UNK, i::UNK, i::UNK, i::EOR, i::LSR, i::UNK, i::CLI, i::EOR, i::UNK, i::UNK, i::UNK, i::EOR, i::LSR, i::UNK,
///* 6x */    i::RTS, i::ADC, i::UNK, i::UNK, i::UNK, i::ADC, i::ROR, i::UNK, i::PLA, i::ADC, i::ROR, i::UNK, i::JMP, i::ADC, i::ROR, i::UNK,
///* 7x */    i::BVS, i::ADC, i::UNK, i::UNK, i::UNK, i::ADC, i::ROR, i::UNK, i::SEI, i::ADC, i::UNK, i::UNK, i::UNK, i::ADC, i::ROR, i::UNK,
///* 8x */    i::UNK, i::STA, i::UNK, i::UNK, i::STY, i::STA, i::STX, i::UNK, i::DEY, i::UNK, i::TXA, i::UNK, i::STY, i::STA, i::STX, i::UNK,
///* 9x */    i::BCC, i::STA, i::UNK, i::UNK, i::STY, i::STA, i::STX, i::UNK, i::TYA, i::STA, i::TXS, i::UNK, i::UNK, i::STA, i::UNK, i::UNK,
///* Ax */    i::LDY, i::LDA, i::LDX, i::UNK, i::LDY, i::LDA, i::LDX, i::UNK, i::TAY, i::LDA, i::TAX, i::UNK, i::LDY, i::LDA, i::LDX, i::UNK,
///* Bx */    i::BCS, i::LDA, i::UNK, i::UNK, i::LDY, i::LDA, i::LDX, i::UNK, i::CLV, i::LDA, i::TSX, i::UNK, i::LDY, i::LDA, i::LDX, i::UNK,
///* Cx */    i::CPY, i::CMP, i::UNK, i::UNK, i::CPY, i::CMP, i::DEC, i::UNK, i::INY, i::CMP, i::DEX, i::UNK, i::CPY, i::CMP, i::DEC, i::UNK,
///* Dx */    i::BNE, i::CMP, i::UNK, i::UNK, i::UNK, i::CMP, i::DEC, i::UNK, i::CLD, i::CMP, i::UNK, i::UNK, i::UNK, i::CMP, i::DEC, i::UNK,
///* Ex */    i::CPX, i::SBC, i::UNK, i::UNK, i::CPX, i::SBC, i::INC, i::UNK, i::INX, i::SBC, i::NOP, i::UNK, i::CPX, i::SBC, i::INC, i::UNK,
///* Fx */    i::BEQ, i::SBC, i::UNK, i::UNK, i::UNK, i::SBC, i::INC, i::UNK, i::SED, i::SBC, i::UNK, i::UNK, i::UNK, i::SBC, i::INC, i::UNK,
//    };
//}
//TEST_F(CpuTest, Opcode_Addressing) {
//
//}

TEST_F(CpuTest, BCC) {
    Test_Branch(
        [&] (Byte & value) { cpu.C = value; },
        0, 1,
        Opcode(InstructionName::BCC, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTest, BCS) {
    Test_Branch(
        [&] (Byte & value) { cpu.C = value; },
        1, 0,
        Opcode(InstructionName::BCS, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTest, BEQ) {
    Test_Branch(
        [&] (Byte & value) { cpu.Z = value; },
        1, 0,
        Opcode(InstructionName::BEQ, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTest, BMI) {
    Test_Branch(
        [&] (Byte & value) { cpu.N = value; },
        1, 0,
        Opcode(InstructionName::BMI, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTest, BNE) {
    Test_Branch(
        [&] (Byte & value) { cpu.Z = value; },
        0, 1,
        Opcode(InstructionName::BNE, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTest, BPL) {
    Test_Branch(
        [&] (Byte & value) { cpu.N = value; },
        0, 1,
        Opcode(InstructionName::BPL, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTest, BVC) {
    Test_Branch(
        [&] (Byte & value) { cpu.V = value; },
        0, 1,
        Opcode(InstructionName::BVC, AddressingType::Relative, 2, 2));
}

TEST_F(CpuTest, BVS) {
    Test_Branch(
        [&] (Byte & value) { cpu.V = value; },
        1, 0,
        Opcode(InstructionName::BVS, AddressingType::Relative, 2, 2));
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
             Opcode(InstructionName::AND, AddressingType::AbsoluteX, 3, 4),
             false);
}

TEST_F(CpuTest, AND_AbsoluteX_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0210, m); },
             Opcode(InstructionName::AND, AddressingType::AbsoluteX, 3, 4),
             true);
}

TEST_F(CpuTest, AND_AbsoluteY) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0128, m); },
             Opcode(InstructionName::AND, AddressingType::AbsoluteY, 3, 4),
             false);
}

TEST_F(CpuTest, AND_AbsoluteY_CrossingPage) {
    // Crossing page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_AND([&] (Byte m) { cpu.Memory.SetByteAt(0x0210, m); },
             Opcode(InstructionName::AND, AddressingType::AbsoluteY, 3, 4),
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
             Opcode(InstructionName::AND, AddressingType::IndirectIndexed, 2, 5),
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
             Opcode(InstructionName::AND, AddressingType::IndirectIndexed, 2, 5),
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

TEST_F(CpuTest, SEC) {
    Test_SetFlag(InstructionName::SEC, cpu.C);
}

TEST_F(CpuTest, SED) {
    Test_SetFlag(InstructionName::SED, cpu.D);
}

TEST_F(CpuTest, SEI) {
    Test_SetFlag(InstructionName::SEI, cpu.I);
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
        Opcode(InstructionName::ADC, AddressingType::AbsoluteX, 3, 4),
        false
        );
}

TEST_F(CpuTest, ADC_AbsoluteX_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0210, value); },
        Opcode(InstructionName::ADC, AddressingType::AbsoluteX, 3, 4),
        true
        );
}

TEST_F(CpuTest, ADC_AbsoluteY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0128, value); },
        Opcode(InstructionName::ADC, AddressingType::AbsoluteY, 3, 4),
        false
        );
}

TEST_F(CpuTest, ADC_AbsoluteY_CrossingPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;

    Test_ADC(
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0210, value); },
        Opcode(InstructionName::ADC, AddressingType::AbsoluteY, 3, 4),
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
        Opcode(InstructionName::ADC, AddressingType::IndirectIndexed, 2, 5),
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
        Opcode(InstructionName::ADC, AddressingType::IndirectIndexed, 2, 5),
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


TEST_F(CpuTest, INC_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_INC(
        [&]              { return cpu.Memory.GetByteAt(0x20); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x20, value); },
        Opcode(InstructionName::INC, AddressingType::ZeroPage, 2, 5)
    );
}

TEST_F(CpuTest, INC_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_INC(
        [&]              { return cpu.Memory.GetByteAt(0x28); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x28, value); },
        Opcode(InstructionName::INC, AddressingType::ZeroPageX, 2, 6)
    );
}

TEST_F(CpuTest, INC_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_INC(
        [&]              { return cpu.Memory.GetByteAt(0x0120); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0120, value); },
        Opcode(InstructionName::INC, AddressingType::Absolute, 2, 6)
    );
}

TEST_F(CpuTest, INC_AbsoluteX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_INC(
        [&]              { return cpu.Memory.GetByteAt(0x0128); },
        [&] (Byte value) { cpu.Memory.SetByteAt(0x0128, value); },
        Opcode(InstructionName::INC, AddressingType::AbsoluteX, 2, 7)
    );
}

TEST_F(CpuTest, INX) {
    Test_INC(
        [&]              { return cpu.X; },
        [&] (Byte value) { cpu.X = value; },
        Opcode(InstructionName::INX, AddressingType::Implicit, 1, 2)
    );
}

TEST_F(CpuTest, INY) {
    Test_INC(
        [&]              { return cpu.Y; },
        [&] (Byte value) { cpu.Y = value; },
        Opcode(InstructionName::INY, AddressingType::Implicit, 1, 2)
    );
}

TEST_F(CpuTest, NOP) {
    cpu.Execute(Opcode(InstructionName::NOP, AddressingType::Implicit, 1, 2));

    EXPECT_EQ(BASE_PC + 1, cpu.PC);
    EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
}

TEST_F(CpuTest, STX_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x20); },
        [&] (Byte value) { cpu.X = value; },
        Opcode(InstructionName::STX, AddressingType::ZeroPage, 2, 3)
    );
}

TEST_F(CpuTest, STX_ZeroPageY) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Y = 0x08;

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x28); },
        [&] (Byte value) { cpu.X = value; },
        Opcode(InstructionName::STX, AddressingType::ZeroPageY, 2, 4)
    );
}

TEST_F(CpuTest, STX_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x0120); },
        [&] (Byte value) { cpu.X = value; },
        Opcode(InstructionName::STX, AddressingType::Absolute, 3, 4)
    );
}

TEST_F(CpuTest, STY_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x20); },
        [&] (Byte value) { cpu.Y = value; },
        Opcode(InstructionName::STY, AddressingType::ZeroPage, 2, 3)
    );
}

TEST_F(CpuTest, STY_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x28); },
        [&] (Byte value) { cpu.Y = value; },
        Opcode(InstructionName::STY, AddressingType::ZeroPageX, 2, 4)
    );
}

TEST_F(CpuTest, STY_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x0120); },
        [&] (Byte value) { cpu.Y = value; },
        Opcode(InstructionName::STY, AddressingType::Absolute, 3, 4)
    );
}

TEST_F(CpuTest, STA_ZeroPage) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x20); },
        [&] (Byte value) { cpu.A = value; },
        Opcode(InstructionName::STA, AddressingType::ZeroPage, 2, 3)
    );
}

TEST_F(CpuTest, STA_ZeroPageX) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x28); },
        [&] (Byte value) { cpu.A = value; },
        Opcode(InstructionName::STA, AddressingType::ZeroPageX, 2, 4)
    );
}

TEST_F(CpuTest, STA_Absolute) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x0120); },
        [&] (Byte value) { cpu.A = value; },
        Opcode(InstructionName::STA, AddressingType::Absolute, 3, 4)
    );
}

TEST_F(CpuTest, STA_AbsoluteX) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x0128); },
        [&] (Byte value) { cpu.A = value; },
        Opcode(InstructionName::STA, AddressingType::AbsoluteX, 3, 4)
    );
}

TEST_F(CpuTest, STA_AbsoluteY) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x0128); },
        [&] (Byte value) { cpu.A = value; },
        Opcode(InstructionName::STA, AddressingType::AbsoluteY, 3, 4)
    );
}

TEST_F(CpuTest, STA_IndexedIndirect) {
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.Memory.SetWordAt(0x28, 0x0120);

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x0120); },
        [&] (Byte value) { cpu.A = value; },
        Opcode(InstructionName::STA, AddressingType::IndexedIndirect, 2, 6)
    );
}

TEST_F(CpuTest, STA_IndirectIndexed) {
    // Same page
    cpu.Memory.SetByteAt(BASE_PC, 0xFF);
    cpu.Memory.SetByteAt(BASE_PC + 1, 0x20);
    cpu.Memory.SetWordAt(0x20, 0x120);
    cpu.Y = 0x08;
    cpu.Memory.SetWordAt(0x0128, 0x0200);

    Test_Set(
        [&]              { return cpu.Memory.GetByteAt(0x0200); },
        [&] (Byte value) { cpu.A = value; },
        Opcode(InstructionName::STA, AddressingType::IndirectIndexed, 2, 6)
    );
}

TEST_F(CpuTest, TAX) {
    Test_Transfer(
        [&]              { return cpu.X; },
        [&] (Byte value) { cpu.A = value; },
        Opcode(InstructionName::TAX, AddressingType::Implicit, 1, 2)
    );
}


TEST_F(CpuTest, TAY) {
    Test_Transfer(
        [&]              { return cpu.Y; },
        [&] (Byte value) { cpu.A = value; },
        Opcode(InstructionName::TAY, AddressingType::Implicit, 1, 2)
    );
}

TEST_F(CpuTest, TSX) {
    Test_Transfer(
        [&]              { return cpu.X; },
        [&] (Byte value) { cpu.SP = value; },
        Opcode(InstructionName::TSX, AddressingType::Implicit, 1, 2)
    );
}

TEST_F(CpuTest, TXA) {
    Test_Transfer(
        [&]              { return cpu.A; },
        [&] (Byte value) { cpu.X = value; },
        Opcode(InstructionName::TXA, AddressingType::Implicit, 1, 2)
    );
}

TEST_F(CpuTest, TXS) {
    // TXS does not change the flags
    auto op = Opcode(InstructionName::TXS, AddressingType::Implicit, 1, 2);
    cpu.X = 0x20;

    cpu.PC = BASE_PC;
    cpu.Ticks = BASE_TICKS;
    cpu.Execute(op);

    EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
    EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
    EXPECT_EQ(0x20, cpu.SP);
}

TEST_F(CpuTest, TYA) {
    Test_Transfer(
        [&]              { return cpu.A; },
        [&] (Byte value) { cpu.Y = value; },
        Opcode(InstructionName::TYA, AddressingType::Implicit, 1, 2)
    );
}
