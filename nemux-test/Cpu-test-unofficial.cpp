#include "gtest/gtest.h"

#include "Cpu.h"
#include "BitUtil.h"

//#include <sstream>
//#include <vector>
//#include <map>
//#include <array>
//#include <functional>

using namespace std;
using namespace Instructions;
using namespace Addressing;

class CpuTestUnofficial : public ::testing::Test {
public:
    static const Word BASE_PC = 10;
    static const int BASE_TICKS = 10;

    CpuTestUnofficial() : memory(), cpu("6502", &memory) {
        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
    }

    MemoryBlock<0x10000> memory;
    Cpu cpu;

    void Test_SLO(Word address, Opcode op) {
        auto tester = [&](Byte a, Byte m, Byte expA, Byte expM, Flag expN, Flag expZ, Flag expC) {
            cpu.WriteByteAt(address, m);
            cpu.A = a;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(address));
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expN, cpu.N);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expC, cpu.C);
        };

        tester(0x00, 0x00, 0x00, 0x00, 0, 1, 0);
        tester(0x80, 0x00, 0x80, 0x00, 1, 0, 0);
        tester(0x00, 0x40, 0x80, 0x80, 1, 0, 0);
        tester(0x10, 0x80, 0x10, 0x00, 0, 0, 1);
    }

    void Test_RLA(Word address, Opcode op) {
        auto tester = [&](Byte a, Byte m, Byte c, Byte expA, Byte expM, Flag expN, Flag expZ, Flag expC) {
            cpu.WriteByteAt(address, m);
            cpu.A = a;
            cpu.C = c;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(address));
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expN, cpu.N);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expC, cpu.C);
        };

        tester(0x00, 0x10, 0, 0x00, 0x20, 0, 1, 0); // Shift
        tester(0x00, 0x10, 1, 0x00, 0x21, 0, 1, 0); // Shift w/ Carry
        tester(0x00, 0x80, 0, 0x00, 0x00, 0, 1, 1); // Shift w/ Carry out
        tester(0x08, 0x0F, 0, 0x08, 0x1E, 0, 0, 0); // Positive
        tester(0xFF, 0x40, 0, 0x80, 0x80, 1, 0, 0); // Negative
        tester(0xFF, 0x00, 0, 0x00, 0x00, 0, 1, 0); // Zero
    }

    void Test_SRE(Word address, Opcode op) {
        auto tester = [&](Byte a, Byte m, Byte expA, Byte expM, Flag expN, Flag expZ, Flag expC) {
            cpu.WriteByteAt(address, m);
            cpu.A = a;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(address));
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expN, cpu.N);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expC, cpu.C);
        };

        tester(0x10, 0x10, 0x18, 0x08, 0, 0, 0); // Shift
        tester(0x10, 0x11, 0x18, 0x08, 0, 0, 1); // Shift w/ Carry out
        tester(0x08, 0x00, 0x08, 0x00, 0, 0, 0); // Positive
        tester(0x80, 0x10, 0x88, 0x08, 1, 0, 0); // Negative
        tester(0x00, 0x00, 0x00, 0x00, 0, 1, 0); // Zero
    }

    void Test_RRA(Word address, Opcode op) {
        auto tester = [&](Byte a, Byte m, Byte c, Byte expA, Byte expM, Flag expC, Flag expZ, Flag expV, Flag expN) {
            cpu.WriteByteAt(address, m);
            cpu.A = a;
            cpu.C = c;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(address));
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expN, cpu.N);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expC, cpu.C);
        };

        // Rotate right part
        tester(0x00, 0x10, 0, 0x08, 0x08, 0, 0, 0, 0); // Shift
        tester(0x00, 0x10, 1, 0x88, 0x88, 0, 0, 0, 1); // Shift w/ carry
        tester(0x00, 0x11, 0, 0x09, 0x08, 0, 0, 0, 0); // Shift w/ carry out

        // ADC part
        tester(0x10, 0x40, 0, 0x30, 0x20, 0, 0, 0, 0); // pos pos > pos
        tester(0x10, 0x41, 0, 0x31, 0x20, 0, 0, 0, 0); // pos pos C > pos
        tester(0x00, 0x00, 0, 0x00, 0x00, 0, 1, 0, 0); // Zero
        tester(0x7F, 0x01, 1, 0x00, 0x80, 1, 1, 0, 0); // Zero by carry
        tester(0x80, 0x00, 1, 0x00, 0x80, 1, 1, 1, 0); // Zero by overflow
        tester(0x20, 0xE0, 1, 0x10, 0xF0, 1, 0, 0, 0); // Carry w/o overflow w/o sign change
        tester(0xF0, 0x40, 0, 0x10, 0x20, 1, 0, 0, 0); // Carry w/o overflow w/  sign change
        tester(0xA0, 0x40, 1, 0x40, 0xA0, 1, 0, 1, 0); // Carry with overflow
        tester(0x70, 0x20, 0, 0x80, 0x10, 0, 0, 1, 1); // Overflow pos > neg
        tester(0xB0, 0x60, 1, 0x60, 0xB0, 1, 0, 1, 0); // Overflow neg > pos
        tester(0x00, 0xE0, 1, 0xF0, 0xF0, 0, 0, 0, 1); // Negative
    }

    void Test_SAX(Word address, Opcode op) {
        // SAX can be called with IndexedIndirect addressing where cpu.X is already set
        cpu.A = 0x5F;
        const Byte expM = (cpu.A & cpu.X);

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expM, cpu.ReadByteAt(address));
    }

    void Test_AHX(Word address, Opcode op) {
        cpu.A = 0x3F;
        cpu.X = 0xF5;
        const Byte H = ((address & WORD_HI_MASK) >> BYTE_WIDTH);
        const Byte expM = (cpu.A & cpu.X & H);

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expM, cpu.ReadByteAt(address));
    }

    void Test_TAS(Word address, Opcode op) {
        cpu.A = 0x3F;
        cpu.X = 0xF5;
        const Byte H = ((address & WORD_HI_MASK) >> BYTE_WIDTH);
        const Byte expS = (cpu.A & cpu.X);
        const Byte expM = (cpu.A & cpu.X & H);

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expS, cpu.SP);
        EXPECT_EQ(expM, cpu.ReadByteAt(address));
    }

    void Test_SHY(Word address, Opcode op) {
        cpu.Y = 0xF5;
        const Byte H = ((address & WORD_HI_MASK) >> BYTE_WIDTH);
        const Byte expM = (cpu.Y & (H + 1));

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        const bool CrossedPage = ((address & 0xFF00) != ((address + cpu.X) & 0xFF00));
        if (CrossedPage) {
            // In case the resulting addres crosses a page
            // The bahviour is corrupted
            // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
            Word addr = (cpu.Y & ((address >> 8) + 1));
            addr = (addr << 8) + (address & WORD_LO_MASK);
            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(addr));
        }
        else {
            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(address));
        }
    }

    void Test_SHX(Word address, Opcode op) {
        cpu.X = 0xF5;
        const Byte H = ((address & WORD_HI_MASK) >> BYTE_WIDTH);
        const Byte expM = (cpu.X & (H + 1));

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        const bool CrossedPage = ((address & 0xFF00) != ((address + cpu.Y) & 0xFF00));
        if (CrossedPage) {
            // In case the resulting addres crosses a page
            // The bahviour is corrupted
            // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
            Word addr = (cpu.X & ((address >> 8) + 1));
            addr = (addr << 8) + (address & WORD_LO_MASK);
            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(addr));
        }
        else {
            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(address));
        }
    }

    void Test_LAX(Word address, Opcode op, int extra) {
        auto tester = [&](Byte m, Flag expN, Flag expZ) {
            cpu.WriteByteAt(address, m);

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            EXPECT_EQ(m, cpu.A);
            EXPECT_EQ(m, cpu.X);
            EXPECT_EQ(expN, cpu.N);
            EXPECT_EQ(expZ, cpu.Z);
        };

        const auto x = cpu.X;
        tester(0x10, 0, 0);
        cpu.X = x;
        tester(0x00, 0, 1);
        cpu.X = x;
        tester(0x80, 1, 0);
    }

    void Test_LAS(Word address, Opcode op, int extra) {
        auto tester = [&](Byte m, Byte s, Byte expAXS, Flag expN, Flag expZ) {
            cpu.WriteByteAt(address, m);
            cpu.SP = s;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles + extra, cpu.Ticks);
            EXPECT_EQ(expAXS, cpu.A);
            EXPECT_EQ(expAXS, cpu.X);
            EXPECT_EQ(expAXS, cpu.SP);
            EXPECT_EQ(expN, cpu.N);
            EXPECT_EQ(expZ, cpu.Z);
        };

        tester(0x10, 0xF0, 0x10, 0, 0);
        tester(0x10, 0x01, 0x00, 0, 1);
        tester(0x80, 0xC0, 0x80, 1, 0);
    }

    void Test_DCP(Word address, Opcode op) {
        auto tester = [&](Byte a, Byte m, Byte expM, Flag expN, Flag expZ, Flag expC) {
            cpu.WriteByteAt(address, m);
            cpu.A = a;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(address));
            EXPECT_EQ(expN, cpu.N);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expC, cpu.C);
        };

        tester(0x20, 0x11, 0x10, 0, 0, 1);
        tester(0x20, 0x21, 0x20, 0, 1, 1);
        tester(0x20, 0x41, 0x40, 1, 0, 0);

        tester(0xF0, 0xE1, 0xE0, 0, 0, 1);
        tester(0xF0, 0xF1, 0xF0, 0, 1, 1);
        tester(0xF0, 0xF9, 0xF8, 1, 0, 0);

        tester(0x80, 0x01, 0x00, 1, 0, 1);
        tester(0x80, 0x00, 0xFF, 1, 0, 0);
    }

    void Test_ISC(Word address, Opcode op) {
        auto tester = [&](Byte a, Byte m, Flag c, Byte expA, Byte expM, Flag expC, Flag expZ, Flag expV, Flag expN) {
            cpu.WriteByteAt(address, m);
            cpu.A = a;
            cpu.C = c;

            cpu.PC = BASE_PC;
            cpu.Ticks = BASE_TICKS;
            cpu.Execute(op);

            EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
            EXPECT_EQ(BASE_TICKS + op.Cycles    , cpu.Ticks);
            EXPECT_EQ(expM, cpu.ReadByteAt(address));
            EXPECT_EQ(expA, cpu.A);
            EXPECT_EQ(expC, cpu.C);
            EXPECT_EQ(expZ, cpu.Z);
            EXPECT_EQ(expV, cpu.V);
            EXPECT_EQ(expN, cpu.N);
        };
        // ISC is INC+SBC, so SBC tests
        // 0x40 + 0xDF + 1 = 0x120
        tester(0x40, 0x1F, 1, 0x20, 0x20, 1, 0, 0, 0); // pos pos C > pos
        // 0x40 + 0xDF + 0 = 0x11F
        tester(0x40, 0x1F, 0, 0x1F, 0x20, 1, 0, 0, 0); // pos pos !C > pos
        // 0x00 + 0xFF + 1 = 0x100
        tester(0x00, 0xFF, 1, 0x00, 0x00, 1, 1, 0, 0); // Zero
        // 0x80 + 0x80 + 0 = 0x100
        tester(0x80, 0x7E, 0, 0x00, 0x7F, 1, 1, 1, 0); // Zero by carry
        // 0x00 + 0x00 + 0 = 0x000
        tester(0x00, 0xFE, 0, 0x00, 0xFF, 0, 1, 0, 0); // Zero by overflow
        // 0x20 + 0xBF + 1 = 0x0E0
        tester(0x20, 0x3F, 1, 0xE0, 0x40, 0, 0, 0, 1); // Carry w/o overflow
        // 0x20 + 0x5F + 1 = 0x080
        tester(0x20, 0x9F, 1, 0x80, 0xA0, 0, 0, 1, 1); // Carry w/ overflow
        // 0x00 + 0xFF + 0 = 0x0FF
        tester(0x00, 0xFF, 0, 0xFF, 0x00, 0, 0, 0, 1); // Carry by borrow
        // 0x20 + 0x5F + 1 = 0x080
        tester(0x20, 0x9F, 1, 0x80, 0xA0, 0, 0, 1, 1); // Overflow pos > neg
        // 0x80 + 0xDF + 1 = 0x160
        tester(0x80, 0x1F, 1, 0x60, 0x20, 1, 0, 1, 0); // Overflow neg > pos
        // 0x80 + 0xFF + 1 = 0x180
        tester(0x80, 0xFF, 1, 0x80, 0x00, 1, 0, 0, 1); // Negative
        // 0x40 + 0xBE + 1 = 0x0FF
        tester(0x40, 0x40, 1, 0xFF, 0x41, 0, 0, 0, 1); // Negative
    }
};
const Word CpuTestUnofficial::BASE_PC;
const int CpuTestUnofficial::BASE_TICKS;

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uNOP) {
    cpu.Execute(Opcode(uNOP, Implicit, 1, 2));

    EXPECT_EQ(BASE_PC + 1, cpu.PC);
    EXPECT_EQ(BASE_TICKS + 2, cpu.Ticks);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSTP) {
    cpu.Execute(Opcode(uSTP, Implicit, 1, 2));

    EXPECT_FALSE(cpu.IsAlive);

    // Check that the CPU is dead
    cpu.Execute(Opcode(uNOP, Implicit, 1, 2));
    EXPECT_EQ(BASE_PC, cpu.PC);
    EXPECT_EQ(BASE_TICKS, cpu.Ticks);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSLO_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_SLO(0x0020, Opcode(uSLO, ZeroPage, 2, 5));
}

TEST_F(CpuTestUnofficial, uSLO_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_SLO(0x0028, Opcode(uSLO, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uSLO_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;
    Test_SLO(0x0000, Opcode(uSLO, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uSLO_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_SLO(0x0120, Opcode(uSLO, Absolute, 3, 6));
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_SLO(0x0128, Opcode(uSLO, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_SLO(0x0210, Opcode(uSLO, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_SLO(0x0128, Opcode(uSLO, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_SLO(0x0210, Opcode(uSLO, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uSLO_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_SLO(0x0120, Opcode(uSLO, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uSLO_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_SLO(0x0120, Opcode(uSLO, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_SLO(0x0128, Opcode(uSLO, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_SLO(0x0210, Opcode(uSLO, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_SLO(0x000F, Opcode(uSLO, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;
    Test_SLO(0x0130, Opcode(uSLO, IndirectIndexed, 2, 8));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uANC_Immediate) {
    Opcode op = Opcode(uANC, Immediate, 2, 2);
    
    auto tester = [&](Byte a, Byte m, Byte expA, Byte expZ, Byte expN, Byte expC) {
        cpu.WriteByteAt(BASE_PC, 0xFF);
        cpu.WriteByteAt(BASE_PC + 1, m);
        cpu.A = a;

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expA, cpu.A);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expN, cpu.N);
        EXPECT_EQ(expC, cpu.C);
    };

    tester(0x0F, 0x00, 0x00, 1, 0, 0);
    tester(0x80, 0xFF, 0x80, 0, 1, 1);
    tester(0x44, 0x24, 0x04, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uRLA_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_RLA(0x0020, Opcode(uRLA, ZeroPage, 2, 5));
}

TEST_F(CpuTestUnofficial, uRLA_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_RLA(0x0028, Opcode(uRLA, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uRLA_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;
    Test_RLA(0x0000, Opcode(uRLA, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uRLA_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_RLA(0x0120, Opcode(uRLA, Absolute, 3, 6));
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_RLA(0x0128, Opcode(uRLA, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_RLA(0x0210, Opcode(uRLA, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_RLA(0x0128, Opcode(uRLA, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_RLA(0x0210, Opcode(uRLA, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uRLA_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_RLA(0x0120, Opcode(uRLA, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uRLA_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_RLA(0x0120, Opcode(uRLA, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_RLA(0x0128, Opcode(uRLA, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_RLA(0x0210, Opcode(uRLA, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_RLA(0x000F, Opcode(uRLA, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;
    Test_RLA(0x0130, Opcode(uRLA, IndirectIndexed, 2, 8));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSRE_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_SRE(0x0020, Opcode(uSRE, ZeroPage, 2, 5));
}

TEST_F(CpuTestUnofficial, uSRE_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_SRE(0x0028, Opcode(uSRE, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uSRE_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;
    Test_SRE(0x0000, Opcode(uSRE, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uSRE_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_SRE(0x0120, Opcode(uSRE, Absolute, 3, 6));
}

TEST_F(CpuTestUnofficial, uSRE_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_SRE(0x0128, Opcode(uSRE, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uSRE_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_SRE(0x0210, Opcode(uSRE, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uSRE_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_SRE(0x0128, Opcode(uSRE, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uSRE_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_SRE(0x0210, Opcode(uSRE, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uSRE_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_SRE(0x0120, Opcode(uSRE, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uSRE_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_SRE(0x0120, Opcode(uSRE, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uSRE_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_SRE(0x0128, Opcode(uSRE, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uSRE_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_SRE(0x0210, Opcode(uSRE, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uSRE_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_SRE(0x000F, Opcode(uSRE, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uSRE_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;
    Test_SRE(0x0130, Opcode(uSRE, IndirectIndexed, 2, 8));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uALR_Immediate) {
    Opcode op = Opcode(uALR, Immediate, 2, 2);

    auto tester = [&](Byte a, Byte m, Byte expA, Byte expZ, Byte expC) {
        cpu.WriteByteAt(BASE_PC, 0xFF);
        cpu.WriteByteAt(BASE_PC + 1, m);
        cpu.A = a;

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expA, cpu.A);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expC, cpu.C);
    };

    tester(0x81, 0x00, 0x00, 1, 0);
    tester(0x81, 0xFF, 0x40, 0, 1);
    tester(0x44, 0x24, 0x02, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uRRA_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_RRA(0x0020, Opcode(uRRA, ZeroPage, 2, 5));
}

TEST_F(CpuTestUnofficial, uRRA_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_RRA(0x0028, Opcode(uRRA, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uRRA_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;
    Test_RRA(0x0000, Opcode(uRRA, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uRRA_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_RRA(0x0120, Opcode(uRRA, Absolute, 3, 6));
}

TEST_F(CpuTestUnofficial, uRRA_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_RRA(0x0128, Opcode(uRRA, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uRRA_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_RRA(0x0210, Opcode(uRRA, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uRRA_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_RRA(0x0128, Opcode(uRRA, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uRRA_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_RRA(0x0210, Opcode(uRRA, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uRRA_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_RRA(0x0120, Opcode(uRRA, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uRRA_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_RRA(0x0120, Opcode(uRRA, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uRRA_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_RRA(0x0128, Opcode(uRRA, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uRRA_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_RRA(0x0210, Opcode(uRRA, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uRRA_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_RRA(0x000F, Opcode(uRRA, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uRRA_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;
    Test_RRA(0x0130, Opcode(uRRA, IndirectIndexed, 2, 8));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uARR_Immediate) {
    // See http://nesdev.com/undocumented_opcodes.txt
    Opcode op = Opcode(uARR, Immediate, 2, 2);

    auto tester = [&](Byte a, Byte m, Flag c, Byte expA, Byte expN, Byte expV, Byte expZ, Byte expC) {
        cpu.WriteByteAt(BASE_PC, 0xFF);
        cpu.WriteByteAt(BASE_PC + 1, m);
        cpu.A = a;
        cpu.C = c;

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expA, cpu.A);
        EXPECT_EQ(expN, cpu.N);
        EXPECT_EQ(expV, cpu.V);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expC, cpu.C);
    };

    tester(0x10, 0x10, 0, 0x08, 0, 0, 0, 0); // Positive
    tester(0x10, 0x10, 1, 0x88, 1, 0, 0, 0); // Negative
    tester(0x10, 0x01, 0, 0x00, 0, 0, 1, 0); // Zero
    tester(0x02, 0xFF, 0, 0x01, 0, 0, 0, 0); // No Carry, no Overflow (result x00x xxxx)
    tester(0x42, 0xFF, 0, 0x21, 0, 1, 0, 0); // No Carry, Overflow (result x01x xxxx)
    tester(0x82, 0xFF, 0, 0x41, 0, 1, 0, 1); // Carry, Overflow (result x10x xxxx)
    tester(0xC2, 0xFF, 0, 0x61, 0, 0, 0, 1); // Carry, no Overflow (result x11x xxxx)
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, SAX_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);

    Test_SAX(0x0020, Opcode(uSAX, ZeroPage, 2, 3));
}

TEST_F(CpuTestUnofficial, SAX_ZeroPageY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.Y = 0x08;
    Test_SAX(0x0028, Opcode(uSAX, ZeroPageY, 2, 4));
}

TEST_F(CpuTestUnofficial, SAX_ZeroPageY_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.Y = 0x10;
    Test_SAX(0x0000, Opcode(uSAX, ZeroPageY, 2, 4));
}

TEST_F(CpuTestUnofficial, SAX_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_SAX(0x0120, Opcode(uSAX, Absolute, 3, 4));
}

TEST_F(CpuTestUnofficial, SAX_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_SAX(0x0120, Opcode(uSAX, IndexedIndirect, 2, 6));
}

TEST_F(CpuTestUnofficial, SAX_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_SAX(0x0120, Opcode(uSAX, IndexedIndirect, 2, 6));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uXAA_Immediate) {
    // Unstable and not well defined
    // Probably implement as No-op
    FAIL();
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uAHX_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_AHX(0x0128, Opcode(uAHX, AbsoluteY, 3, 5));
}

TEST_F(CpuTestUnofficial, uAHX_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_AHX(0x0210, Opcode(uAHX, AbsoluteY, 3, 5));
}

TEST_F(CpuTestUnofficial, uAHX_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_AHX(0x0128, Opcode(uAHX, IndirectIndexed, 2, 6));
}

TEST_F(CpuTestUnofficial, uAHX_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_AHX(0x0210, Opcode(uAHX, IndirectIndexed, 2, 6));
}

TEST_F(CpuTestUnofficial, uAHX_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_AHX(0x000F, Opcode(uAHX, IndirectIndexed, 2, 6));
}

TEST_F(CpuTestUnofficial, uAHX_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;
    Test_AHX(0x0130, Opcode(uAHX, IndirectIndexed, 2, 6));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uTAS_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_TAS(0x0128, Opcode(uTAS, AbsoluteY, 3, 5));
}

TEST_F(CpuTestUnofficial, uTAS_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_TAS(0x0210, Opcode(uTAS, AbsoluteY, 3, 5));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSHY_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_SHY(0x0128, Opcode(uSHY, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uSHY_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_SHY(0x0210, Opcode(uSHY, AbsoluteX, 3, 7));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSHX_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_SHX(0x0128, Opcode(uSHX, AbsoluteY, 3, 5));
}

TEST_F(CpuTestUnofficial, uSHX_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_SHX(0x0210, Opcode(uSHX, AbsoluteY, 3, 5));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uLAX_Immediate) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    Test_LAX(BASE_PC + 1, Opcode(uLAX, Immediate, 2, 2), 0);
}

TEST_F(CpuTestUnofficial, uLAX_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_LAX(0x0020, Opcode(uLAX, ZeroPage, 2, 3), 0);
}

TEST_F(CpuTestUnofficial, uLAX_ZeroPageY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.Y = 0x08;
    Test_LAX(0x0028, Opcode(uLAX, ZeroPageY, 2, 4), 0);
}

TEST_F(CpuTestUnofficial, uLAX_ZeroPageY_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.Y = 0x10;
    Test_LAX(0x0000, Opcode(uLAX, ZeroPageY, 2, 4), 0);
}

TEST_F(CpuTestUnofficial, uLAX_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_LAX(0x0120, Opcode(uLAX, Absolute, 3, 4), 0);
}

TEST_F(CpuTestUnofficial, uLAX_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_LAX(0x0128, Opcode(uLAX, AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTestUnofficial, uLAX_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_LAX(0x0210, Opcode(uLAX, AbsoluteY, 3, 4), 1);
}

TEST_F(CpuTestUnofficial, uLAX_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_LAX(0x0120, Opcode(uLAX, IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTestUnofficial, uLAX_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_LAX(0x0120, Opcode(uLAX, IndexedIndirect, 2, 6), 0);
}

TEST_F(CpuTestUnofficial, uLAX_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_LAX(0x0128, Opcode(uLAX, IndirectIndexed, 2, 5), 0);
}

TEST_F(CpuTestUnofficial, uLAX_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_LAX(0x0210, Opcode(uLAX, IndirectIndexed, 2, 5), 1);
}

TEST_F(CpuTestUnofficial, uLAX_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_LAX(0x000F, Opcode(uLAX, IndirectIndexed, 2, 5), 1);
}

TEST_F(CpuTestUnofficial, uLAX_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;
    Test_LAX(0x0130, Opcode(uLAX, IndirectIndexed, 2, 5), 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uLAS_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_LAS(0x0128, Opcode(uLAS, AbsoluteY, 3, 4), 0);
}

TEST_F(CpuTestUnofficial, uLAS_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_LAS(0x0210, Opcode(uLAS, AbsoluteY, 3, 4), 1);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uDCP_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_DCP(0x0020, Opcode(uDCP, ZeroPage, 2, 5));
}

TEST_F(CpuTestUnofficial, uDCP_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_DCP(0x0028, Opcode(uDCP, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uDCP_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;
    Test_DCP(0x0000, Opcode(uDCP, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uDCP_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_DCP(0x0120, Opcode(uDCP, Absolute, 3, 6));
}

TEST_F(CpuTestUnofficial, uDCP_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_DCP(0x0128, Opcode(uDCP, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uDCP_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_DCP(0x0210, Opcode(uDCP, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uDCP_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_DCP(0x0128, Opcode(uDCP, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uDCP_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_DCP(0x0210, Opcode(uDCP, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uDCP_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_DCP(0x0120, Opcode(uDCP, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uDCP_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_DCP(0x0120, Opcode(uDCP, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uDCP_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_DCP(0x0128, Opcode(uDCP, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uDCP_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_DCP(0x0210, Opcode(uDCP, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uDCP_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_DCP(0x000F, Opcode(uDCP, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uDCP_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;
    Test_DCP(0x0130, Opcode(uDCP, IndirectIndexed, 2, 8));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uAXS_Immediate) {
    Opcode op = Opcode(uAXS, Immediate, 2, 2);

    auto tester = [&](Byte a, Byte x, Byte m, Byte expX, Byte expN, Byte expZ, Byte expC) {
        cpu.WriteByteAt(BASE_PC, 0xFF);
        cpu.WriteByteAt(BASE_PC + 1, m);
        cpu.A = a;
        cpu.X = x;

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expX, cpu.X);
        EXPECT_EQ(expN, cpu.N);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expC, cpu.C);
    };

    // Flags are set like CMP
    tester(0xFF, 0x20, 0x10, 0x10, 0, 0, 1); // Greater
    tester(0xFF, 0x20, 0x20, 0x00, 0, 1, 1); // Equal
    tester(0xFF, 0x20, 0x40, 0xE0, 1, 0, 0); // MSB set
    tester(0x20, 0xFF, 0x40, 0xE0, 1, 0, 0); // MSB set

    tester(0xFF, 0xF0, 0xE0, 0x10, 0, 0, 1); // Greater
    tester(0xFF, 0xF0, 0xF0, 0x00, 0, 1, 1); // Equal
    tester(0xFF, 0xF0, 0xF8, 0xF8, 1, 0, 0); // MSB set
    tester(0xF0, 0xFF, 0xF8, 0xF8, 1, 0, 0); // MSB set

    tester(0xFF, 0x80, 0x00, 0x80, 1, 0, 1); // MSB set
    tester(0x80, 0xFF, 0x00, 0x80, 1, 0, 1); // MSB set
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uISC_ZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    Test_ISC(0x0020, Opcode(uISC, ZeroPage, 2, 5));
}

TEST_F(CpuTestUnofficial, uISC_ZeroPageX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    Test_ISC(0x0028, Opcode(uISC, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uISC_ZeroPageX_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x10;
    Test_ISC(0x0000, Opcode(uISC, ZeroPageX, 2, 6));
}

TEST_F(CpuTestUnofficial, uISC_Absolute) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    Test_ISC(0x0120, Opcode(uISC, Absolute, 3, 6));
}

TEST_F(CpuTestUnofficial, uISC_AbsoluteX) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0x08;
    Test_ISC(0x0128, Opcode(uISC, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uISC_AbsoluteX_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.X = 0xF0;
    Test_ISC(0x0210, Opcode(uISC, AbsoluteX, 3, 7));
}

TEST_F(CpuTestUnofficial, uISC_AbsoluteY) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0x08;
    Test_ISC(0x0128, Opcode(uISC, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uISC_AbsoluteY_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteWordAt(BASE_PC + 1, 0x0120);
    cpu.Y = 0xF0;
    Test_ISC(0x0210, Opcode(uISC, AbsoluteY, 3, 7));
}

TEST_F(CpuTestUnofficial, uISC_IndexedIndirect) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.X = 0x08;
    cpu.WriteWordAt(0x28, 0x0120);
    Test_ISC(0x0120, Opcode(uISC, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uISC_IndexedIndirect_Wraparound) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xF0);
    cpu.X = 0x0F;
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    Test_ISC(0x0120, Opcode(uISC, IndexedIndirect, 2, 8));
}

TEST_F(CpuTestUnofficial, uISC_IndirectIndexed) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0x08;
    Test_ISC(0x0128, Opcode(uISC, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uISC_IndirectIndexed_CrossingPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0x0120);
    cpu.Y = 0xF0;
    Test_ISC(0x0210, Opcode(uISC, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uISC_IndirectIndexed_CrossingWordsize) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0x20);
    cpu.WriteWordAt(0x20, 0xFFFF);
    cpu.Y = 0x10;
    Test_ISC(0x000F, Opcode(uISC, IndirectIndexed, 2, 8));
}

TEST_F(CpuTestUnofficial, uISC_IndirectIndexed_BaseFromZeroPage) {
    cpu.WriteByteAt(BASE_PC, 0xFF);
    cpu.WriteByteAt(BASE_PC + 1, 0xFF);
    cpu.WriteByteAt(0xFF, 0x20);
    cpu.WriteByteAt(0x00, 0x01);
    cpu.Y = 0x10;
    Test_ISC(0x0130, Opcode(uISC, IndirectIndexed, 2, 8));
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSBC_Immediate) {
    Opcode op = Opcode(uSBC, Immediate, 2, 2);

    auto tester = [&](Byte a, Byte m, Flag c, Byte expA, Flag expC, Flag expZ, Flag expV, Flag expN) {
        cpu.WriteByteAt(BASE_PC + 1, m);
        cpu.A = a;
        cpu.C = c;

        cpu.PC = BASE_PC;
        cpu.Ticks = BASE_TICKS;
        cpu.Execute(op);

        EXPECT_EQ(BASE_PC + op.Bytes, cpu.PC);
        EXPECT_EQ(BASE_TICKS + op.Cycles, cpu.Ticks);
        EXPECT_EQ(expA, cpu.A);
        EXPECT_EQ(expC, cpu.C);
        EXPECT_EQ(expZ, cpu.Z);
        EXPECT_EQ(expV, cpu.V);
        EXPECT_EQ(expN, cpu.N);
    };

    // 0x40 + 0xDF + 1 = 0x120
    tester(0x40, 0x20, 1, 0x20, 1, 0, 0, 0); // pos pos C > pos
    // 0x40 + 0xDF + 0 = 0x11F
    tester(0x40, 0x20, 0, 0x1F, 1, 0, 0, 0); // pos pos !C > pos
    // 0x00 + 0xFF + 1 = 0x100
    tester(0x00, 0x00, 1, 0x00, 1, 1, 0, 0); // Zero
    // 0x80 + 0x80 + 0 = 0x100
    tester(0x80, 0x7F, 0, 0x00, 1, 1, 1, 0); // Zero by carry
    // 0x00 + 0x00 + 0 = 0x000
    tester(0x00, 0xFF, 0, 0x00, 0, 1, 0, 0); // Zero by overflow
    // 0x20 + 0xBF + 1 = 0x0E0
    tester(0x20, 0x40, 1, 0xE0, 0, 0, 0, 1); // Carry w/o overflow
    // 0x20 + 0x5F + 1 = 0x080
    tester(0x20, 0xA0, 1, 0x80, 0, 0, 1, 1); // Carry w/ overflow
    // 0x00 + 0xFF + 0 = 0x0FF
    tester(0x00, 0x00, 0, 0xFF, 0, 0, 0, 1); // Carry by borrow
    // 0x20 + 0x5F + 1 = 0x080
    tester(0x20, 0xA0, 1, 0x80, 0, 0, 1, 1); // Overflow pos > neg
    // 0x80 + 0xDF + 1 = 0x160
    tester(0x80, 0x20, 1, 0x60, 1, 0, 1, 0); // Overflow neg > pos
    // 0x80 + 0xFF + 1 = 0x180
    tester(0x80, 0x00, 1, 0x80, 1, 0, 0, 1); // Negative

    // 0x40 + 0xBE + 1 = 0x0FF
    tester(0x40, 0x41, 1, 0xFF, 0, 0, 0, 1); // Negative
}
