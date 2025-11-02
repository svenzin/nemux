#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;

struct CpuTestUnofficial : public CpuBaseTest {
    void Test_SLO(CpuTestBench::addressing_mode_setup addressingMode) {
        auto tester = [&](Byte a, Byte m, Byte expA, Byte expM, Flag expN, Flag expZ, Flag expC) {
            bench.start();

            bench.set_target(m);
            cpu->A = a;
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ(expM, bench.get_target());
            EXPECT_EQ(expA, cpu->A);
            EXPECT_EQ(expN, cpu->N);
            EXPECT_EQ(expZ, cpu->Z);
            EXPECT_EQ(expC, cpu->C);
        };

        (bench.*addressingMode)(uSLO);
        tester(0x00, 0x00, 0x00, 0x00, 0, 1, 0);
        tester(0x80, 0x00, 0x80, 0x00, 1, 0, 0);
        tester(0x00, 0x40, 0x80, 0x80, 1, 0, 0);
        tester(0x10, 0x80, 0x10, 0x00, 0, 0, 1);
    }

    void Test_RLA(CpuTestBench::addressing_mode_setup addressingMode) {
        auto tester = [&](Byte a, Byte m, Byte c, Byte expA, Byte expM, Flag expN, Flag expZ, Flag expC) {
            bench.start();

            bench.set_target(m);
            cpu->A = a;
            cpu->C = c;
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ(expM, bench.get_target());
            EXPECT_EQ(expA, cpu->A);
            EXPECT_EQ(expN, cpu->N);
            EXPECT_EQ(expZ, cpu->Z);
            EXPECT_EQ(expC, cpu->C);
        };

        (bench.*addressingMode)(uRLA);
        tester(0x00, 0x10, 0, 0x00, 0x20, 0, 1, 0); // Shift
        tester(0x00, 0x10, 1, 0x00, 0x21, 0, 1, 0); // Shift w/ Carry
        tester(0x00, 0x80, 0, 0x00, 0x00, 0, 1, 1); // Shift w/ Carry out
        tester(0x08, 0x0F, 0, 0x08, 0x1E, 0, 0, 0); // Positive
        tester(0xFF, 0x40, 0, 0x80, 0x80, 1, 0, 0); // Negative
        tester(0xFF, 0x00, 0, 0x00, 0x00, 0, 1, 0); // Zero
    }

    void Test_SRE(CpuTestBench::addressing_mode_setup addressingMode) {
        auto tester = [&](Byte a, Byte m, Byte expA, Byte expM, Flag expN, Flag expZ, Flag expC) {
            bench.start();

            bench.set_target(m);
            cpu->A = a;
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ(expM, bench.get_target());
            EXPECT_EQ(expA, cpu->A);
            EXPECT_EQ(expN, cpu->N);
            EXPECT_EQ(expZ, cpu->Z);
            EXPECT_EQ(expC, cpu->C);
        };

        (bench.*addressingMode)(uSRE);
        tester(0x10, 0x10, 0x18, 0x08, 0, 0, 0); // Shift
        tester(0x10, 0x11, 0x18, 0x08, 0, 0, 1); // Shift w/ Carry out
        tester(0x08, 0x00, 0x08, 0x00, 0, 0, 0); // Positive
        tester(0x80, 0x10, 0x88, 0x08, 1, 0, 0); // Negative
        tester(0x00, 0x00, 0x00, 0x00, 0, 1, 0); // Zero
    }

    void Test_RRA(CpuTestBench::addressing_mode_setup addressingMode) {
        auto tester = [&](Byte a, Byte m, Byte c, Byte expA, Byte expM, Flag expC, Flag expZ, Flag expV, Flag expN) {
            bench.start();

            bench.set_target(m);
            cpu->A = a;
            cpu->C = c;
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ(expM, bench.get_target());
            EXPECT_EQ(expA, cpu->A);
            EXPECT_EQ(expN, cpu->N);
            EXPECT_EQ(expZ, cpu->Z);
            EXPECT_EQ(expC, cpu->C);
        };

        (bench.*addressingMode)(uRRA);

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

    void Test_SAX(CpuTestBench::addressing_mode_setup addressingMode, bool xIsSet) {
        // SAX can be called with IndexedIndirect addressing where cpu->X is already set
        (bench.*addressingMode)(uSAX)
            .start();
        
        cpu->A = 0x3F;
        if (!xIsSet) cpu->X = 0xF5;
        const auto expM{ static_cast<Byte>(cpu->A & cpu->X) };

        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(expM, bench.get_target());
    }

    void Test_AHX(CpuTestBench::addressing_mode_setup addressingMode) {
        (bench.*addressingMode)(uAHX)
            .start();

        cpu->A = 0x3F;
        cpu->X = 0xF5;
        const Byte H{ HI(bench.target.value()) };
        const auto expM{ static_cast<Byte>(cpu->A & cpu->X & H) };

        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(expM, bench.get_target());
    }

    void Test_TAS(CpuTestBench::addressing_mode_setup addressingMode) {
        (bench.*addressingMode)(uTAS)
            .start();

        cpu->A = 0x3F;
        cpu->X = 0xF5;
        const Byte H{ HI(bench.target.value()) };
        const auto expS{ static_cast<Byte>(cpu->A & cpu->X) };
        const auto expM{ static_cast<Byte>(cpu->A & cpu->X & H) };

        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(expS, cpu->S);
        EXPECT_EQ(expM, bench.get_target());
    }

    void Test_SHY(CpuTestBench::addressing_mode_setup addressingMode) {
        (bench.*addressingMode)(uSHY).start();

        const Word address{ bench.target.value() };

        cpu->Y = 0xF5;
        const Byte H{ HI(address) };
        const auto expM{ static_cast<Byte>(cpu->Y & (H + 1)) };

        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());

        const bool CrossedPage = (H != HI(address + cpu->X));
        if (CrossedPage) {
            // In case the resulting addres crosses a page
            // The bahviour is corrupted
            // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
            auto hi{ static_cast<Byte>(cpu->Y & (H + 1)) };
            auto addr{ MakeWord(LO(address), hi) };
            EXPECT_EQ(expM, memory->GetByteAt(addr));
        }
        else {
            EXPECT_EQ(expM, bench.get_target());
        }
    }

    void Test_SHX(CpuTestBench::addressing_mode_setup addressingMode) {
        (bench.*addressingMode)(uSHX).start();

        const Word address{ bench.target.value() };

        cpu->X = 0xF5;
        const Byte H{ HI(address) };
        const auto expM{ static_cast<Byte>(cpu->X & (H + 1)) };

        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());

        const bool CrossedPage = (H != HI(address + cpu->Y));
        if (CrossedPage) {
            // In case the resulting addres crosses a page
            // The bahviour is corrupted
            // See http://forums.nesdev.com/viewtopic.php?f=3&t=3831&start=30
            auto hi{ static_cast<Byte>(cpu->X & (H + 1)) };
            auto addr{ MakeWord(LO(address), hi) };
            EXPECT_EQ(expM, memory->GetByteAt(addr));
        }
        else {
            EXPECT_EQ(expM, bench.get_target());
        }
    }

    void Test_LAX(CpuTestBench::addressing_mode_setup addressingMode, int extra) {
        auto tester = [&](Byte m, Flag expN, Flag expZ) {
            bench.start();
            
            bench.set_target(m);
            ExecuteOne();
            
            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(extra), cpu->GetTicks());
            EXPECT_EQ(m, cpu->A);
            EXPECT_EQ(m, cpu->X);
            EXPECT_EQ(expN, cpu->N);
            EXPECT_EQ(expZ, cpu->Z);
        };
        
        (bench.*addressingMode)(uLAX);
        const auto x = cpu->X;
        tester(0x10, 0, 0);
        cpu->X = x;
        tester(0x00, 0, 1);
        cpu->X = x;
        tester(0x80, 1, 0);
    }

    void Test_LAS(CpuTestBench::addressing_mode_setup addressingMode, int extra) {
        auto tester = [&](Byte m, Byte s, Byte expAXS, Flag expN, Flag expZ) {
            bench.start();

            bench.set_target(m);
            cpu->S = s;
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(extra), cpu->GetTicks());
            EXPECT_EQ(expAXS, cpu->A);
            EXPECT_EQ(expAXS, cpu->X);
            EXPECT_EQ(expAXS, cpu->S);
            EXPECT_EQ(expN, cpu->N);
            EXPECT_EQ(expZ, cpu->Z);
        };

        (bench.*addressingMode)(uLAS);
        tester(0x10, 0xF0, 0x10, 0, 0);
        tester(0x10, 0x01, 0x00, 0, 1);
        tester(0x80, 0xC0, 0x80, 1, 0);
    }

    void Test_DCP(CpuTestBench::addressing_mode_setup addressingMode) {
        auto tester = [&](Byte a, Byte m, Byte expM, Flag expN, Flag expZ, Flag expC) {
            bench.start();
            
            bench.set_target(m);
            cpu->A = a;
            ExecuteOne();
            
            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ(expM, bench.get_target());
            EXPECT_EQ(expN, cpu->N);
            EXPECT_EQ(expZ, cpu->Z);
            EXPECT_EQ(expC, cpu->C);
        };

        (bench.*addressingMode)(uDCP);
        tester(0x20, 0x11, 0x10, 0, 0, 1);
        tester(0x20, 0x21, 0x20, 0, 1, 1);
        tester(0x20, 0x41, 0x40, 1, 0, 0);

        tester(0xF0, 0xE1, 0xE0, 0, 0, 1);
        tester(0xF0, 0xF1, 0xF0, 0, 1, 1);
        tester(0xF0, 0xF9, 0xF8, 1, 0, 0);

        tester(0x80, 0x01, 0x00, 1, 0, 1);
        tester(0x80, 0x00, 0xFF, 1, 0, 0);
    }

    void Test_ISC(CpuTestBench::addressing_mode_setup addressingMode) {
        auto tester = [&](Byte a, Byte m, Flag c, Byte expA, Byte expM, Flag expC, Flag expZ, Flag expV, Flag expN) {
            bench.start();
            
            bench.set_target(m);
            cpu->A = a;
            cpu->C = c;
            ExecuteOne();
            
            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ(expM, bench.get_target());
            EXPECT_EQ(expA, cpu->A);
            EXPECT_EQ(expC, cpu->C);
            EXPECT_EQ(expZ, cpu->Z);
            EXPECT_EQ(expV, cpu->V);
            EXPECT_EQ(expN, cpu->N);
        };

        (bench.*addressingMode)(uISC);
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

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uNOP) {
    bench.reset().implicit(uNOP).start();
    ExecuteOne();
    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());

    bench.reset().immediate(uNOP).start();
    ExecuteOne();
    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());

    bench.reset().zeropage(uNOP).start();
    ExecuteOne();
    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());

    bench.reset().zeropage_x(uNOP).start();
    ExecuteOne();
    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());

    bench.reset().absolute(uNOP).start();
    ExecuteOne();
    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());

    bench.reset().absolute_x(uNOP).start();
    ExecuteOne();
    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSTP) {
    bench.implicit(uSTP).start();
    ExecuteOne();
    
    const auto stopPC{ cpu->PC };
    EXPECT_TRUE(cpu->IsStopped());

    // Check that the CPU is dead
    bench.implicit(NOP);//.start();
    ExecuteOne();
    EXPECT_EQ(stopPC, cpu->PC);
    EXPECT_TRUE(cpu->IsStopped());
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSLO_ZeroPage) {
    Test_SLO(&CpuTestBench::zeropage);
}

TEST_F(CpuTestUnofficial, uSLO_ZeroPageX) {
    Test_SLO(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestUnofficial, uSLO_ZeroPageX_Wraparound) {
    Test_SLO(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestUnofficial, uSLO_Absolute) {
    Test_SLO(&CpuTestBench::absolute);
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteX) {
    Test_SLO(&CpuTestBench::absolute_x);
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteX_CrossingPage) {
    Test_SLO(&CpuTestBench::absolute_x_crossing_page);
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteY) {
    Test_SLO(&CpuTestBench::absolute_y);
}

TEST_F(CpuTestUnofficial, uSLO_AbsoluteY_CrossingPage) {
    Test_SLO(&CpuTestBench::absolute_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uSLO_IndexedIndirect) {
    Test_SLO(&CpuTestBench::indirect_x);
}

TEST_F(CpuTestUnofficial, uSLO_IndexedIndirect_Wraparound) {
    Test_SLO(&CpuTestBench::indirect_x_wraparound);
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed) {
    Test_SLO(&CpuTestBench::indirect_y);
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed_CrossingPage) {
    Test_SLO(&CpuTestBench::indirect_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed_CrossingWordsize) {
    Test_SLO(&CpuTestBench::indirect_y_crossing_word_size);
}

TEST_F(CpuTestUnofficial, uSLO_IndirectIndexed_BaseFromZeroPage) {
    Test_SLO(&CpuTestBench::indirect_y_base_from_zeropage);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uANC_Immediate) {
    auto tester = [&](Byte a, Byte m, Byte expA, Byte expZ, Byte expN, Byte expC) {
        bench.start();

        bench.set_target(m);
        cpu->A = a;
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(expA, cpu->A);
        EXPECT_EQ(expZ, cpu->Z);
        EXPECT_EQ(expN, cpu->N);
        EXPECT_EQ(expC, cpu->C);
    };

    bench.immediate(uANC);
    tester(0x0F, 0x00, 0x00, 1, 0, 0);
    tester(0x80, 0xFF, 0x80, 0, 1, 1);
    tester(0x44, 0x24, 0x04, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uRLA_ZeroPage) {
    Test_RLA(&CpuTestBench::zeropage);
}

TEST_F(CpuTestUnofficial, uRLA_ZeroPageX) {
    Test_RLA(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestUnofficial, uRLA_ZeroPageX_Wraparound) {
    Test_RLA(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestUnofficial, uRLA_Absolute) {
    Test_RLA(&CpuTestBench::absolute);
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteX) {
    Test_RLA(&CpuTestBench::absolute_x);
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteX_CrossingPage) {
    Test_RLA(&CpuTestBench::absolute_x_crossing_page);
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteY) {
    Test_RLA(&CpuTestBench::absolute_y);
}

TEST_F(CpuTestUnofficial, uRLA_AbsoluteY_CrossingPage) {
    Test_RLA(&CpuTestBench::absolute_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uRLA_IndexedIndirect) {
    Test_RLA(&CpuTestBench::indirect_x);
}

TEST_F(CpuTestUnofficial, uRLA_IndexedIndirect_Wraparound) {
    Test_RLA(&CpuTestBench::indirect_x_wraparound);
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed) {
    Test_RLA(&CpuTestBench::indirect_y);
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed_CrossingPage) {
    Test_RLA(&CpuTestBench::indirect_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed_CrossingWordsize) {
    Test_RLA(&CpuTestBench::indirect_y_crossing_word_size);
}

TEST_F(CpuTestUnofficial, uRLA_IndirectIndexed_BaseFromZeroPage) {
    Test_RLA(&CpuTestBench::indirect_y_base_from_zeropage);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSRE_ZeroPage) {
    Test_SRE(&CpuTestBench::zeropage);
}

TEST_F(CpuTestUnofficial, uSRE_ZeroPageX) {
    Test_SRE(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestUnofficial, uSRE_ZeroPageX_Wraparound) {
    Test_SRE(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestUnofficial, uSRE_Absolute) {
    Test_SRE(&CpuTestBench::absolute);
}

TEST_F(CpuTestUnofficial, uSRE_AbsoluteX) {
    Test_SRE(&CpuTestBench::absolute_x);
}

TEST_F(CpuTestUnofficial, uSRE_AbsoluteX_CrossingPage) {
    Test_SRE(&CpuTestBench::absolute_x_crossing_page);
}

TEST_F(CpuTestUnofficial, uSRE_AbsoluteY) {
    Test_SRE(&CpuTestBench::absolute_y);
}

TEST_F(CpuTestUnofficial, uSRE_AbsoluteY_CrossingPage) {
    Test_SRE(&CpuTestBench::absolute_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uSRE_IndexedIndirect) {
    Test_SRE(&CpuTestBench::indirect_x);
}

TEST_F(CpuTestUnofficial, uSRE_IndexedIndirect_Wraparound) {
    Test_SRE(&CpuTestBench::indirect_x_wraparound);
}

TEST_F(CpuTestUnofficial, uSRE_IndirectIndexed) {
    Test_SRE(&CpuTestBench::indirect_y);
}

TEST_F(CpuTestUnofficial, uSRE_IndirectIndexed_CrossingPage) {
    Test_SRE(&CpuTestBench::indirect_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uSRE_IndirectIndexed_CrossingWordsize) {
    Test_SRE(&CpuTestBench::indirect_y_crossing_word_size);
}

TEST_F(CpuTestUnofficial, uSRE_IndirectIndexed_BaseFromZeroPage) {
    Test_SRE(&CpuTestBench::indirect_y_base_from_zeropage);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uALR_Immediate) {
    auto tester = [&](Byte a, Byte m, Byte expA, Byte expZ, Byte expC) {
        bench.start();

        bench.set_target(m);
        cpu->A = a;
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(expA, cpu->A);
        EXPECT_EQ(expZ, cpu->Z);
        EXPECT_EQ(expC, cpu->C);
    };

    bench.immediate(uALR);
    tester(0x81, 0x00, 0x00, 1, 0);
    tester(0x81, 0xFF, 0x40, 0, 1);
    tester(0x44, 0x24, 0x02, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uRRA_ZeroPage) {
    Test_RRA(&CpuTestBench::zeropage);
}

TEST_F(CpuTestUnofficial, uRRA_ZeroPageX) {
    Test_RRA(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestUnofficial, uRRA_ZeroPageX_Wraparound) {
    Test_RRA(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestUnofficial, uRRA_Absolute) {
    Test_RRA(&CpuTestBench::absolute);
}

TEST_F(CpuTestUnofficial, uRRA_AbsoluteX) {
    Test_RRA(&CpuTestBench::absolute_x);
}

TEST_F(CpuTestUnofficial, uRRA_AbsoluteX_CrossingPage) {
    Test_RRA(&CpuTestBench::absolute_x_crossing_page);
}

TEST_F(CpuTestUnofficial, uRRA_AbsoluteY) {
    Test_RRA(&CpuTestBench::absolute_y);
}

TEST_F(CpuTestUnofficial, uRRA_AbsoluteY_CrossingPage) {
    Test_RRA(&CpuTestBench::absolute_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uRRA_IndexedIndirect) {
    Test_RRA(&CpuTestBench::indirect_x);
}

TEST_F(CpuTestUnofficial, uRRA_IndexedIndirect_Wraparound) {
    Test_RRA(&CpuTestBench::indirect_x_wraparound);
}

TEST_F(CpuTestUnofficial, uRRA_IndirectIndexed) {
    Test_RRA(&CpuTestBench::indirect_y);
}

TEST_F(CpuTestUnofficial, uRRA_IndirectIndexed_CrossingPage) {
    Test_RRA(&CpuTestBench::indirect_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uRRA_IndirectIndexed_CrossingWordsize) {
    Test_RRA(&CpuTestBench::indirect_y_crossing_word_size);
}

TEST_F(CpuTestUnofficial, uRRA_IndirectIndexed_BaseFromZeroPage) {
    Test_RRA(&CpuTestBench::indirect_y_base_from_zeropage);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uARR_Immediate) {
    // See http://nesdev.com/undocumented_opcodes.txt
    auto tester = [&](Byte a, Byte m, Flag c, Byte expA, Byte expN, Byte expV, Byte expZ, Byte expC) {
        bench.start();

        bench.set_target(m);
        cpu->A = a;
        cpu->C = c;
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(expA, cpu->A);
        EXPECT_EQ(expN, cpu->N);
        EXPECT_EQ(expV, cpu->V);
        EXPECT_EQ(expZ, cpu->Z);
        EXPECT_EQ(expC, cpu->C);
    };
    
    bench.immediate(uARR);
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
    Test_SAX(&CpuTestBench::zeropage, false);
}

TEST_F(CpuTestUnofficial, SAX_ZeroPageY) {
    Test_SAX(&CpuTestBench::zeropage_y, false);
}

TEST_F(CpuTestUnofficial, SAX_ZeroPageY_Wraparound) {
    Test_SAX(&CpuTestBench::zeropage_y_wraparound, false);
}

TEST_F(CpuTestUnofficial, SAX_Absolute) {
    Test_SAX(&CpuTestBench::absolute, false);
}

TEST_F(CpuTestUnofficial, SAX_IndexedIndirect) {
    Test_SAX(&CpuTestBench::indirect_x, true);
}

TEST_F(CpuTestUnofficial, SAX_IndexedIndirect_Wraparound) {
    Test_SAX(&CpuTestBench::indirect_x_wraparound, true);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uXAA_Immediate) {
    // Unstable and not well defined
    // Probably implement as No-op
    FAIL();
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uAHX_AbsoluteY) {
    Test_AHX(&CpuTestBench::absolute_y);
}

TEST_F(CpuTestUnofficial, uAHX_AbsoluteY_CrossingPage) {
    Test_AHX(&CpuTestBench::absolute_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uAHX_IndirectIndexed) {
    Test_AHX(&CpuTestBench::indirect_y);
}

TEST_F(CpuTestUnofficial, uAHX_IndirectIndexed_CrossingPage) {
    Test_AHX(&CpuTestBench::indirect_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uAHX_IndirectIndexed_CrossingWordsize) {
    Test_AHX(&CpuTestBench::indirect_y_crossing_word_size);
}

TEST_F(CpuTestUnofficial, uAHX_IndirectIndexed_BaseFromZeroPage) {
    Test_AHX(&CpuTestBench::indirect_y_base_from_zeropage);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uTAS_AbsoluteY) {
    Test_TAS(&CpuTestBench::absolute_y);
}

TEST_F(CpuTestUnofficial, uTAS_AbsoluteY_CrossingPage) {
    Test_TAS(&CpuTestBench::absolute_y_crossing_page);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSHY_AbsoluteX) {
    Test_SHY(&CpuTestBench::absolute_x);
}

TEST_F(CpuTestUnofficial, uSHY_AbsoluteX_CrossingPage) {
    Test_SHY(&CpuTestBench::absolute_x_crossing_page);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSHX_AbsoluteY) {
    Test_SHX(&CpuTestBench::absolute_y);
}

TEST_F(CpuTestUnofficial, uSHX_AbsoluteY_CrossingPage) {
    Test_SHX(&CpuTestBench::absolute_y_crossing_page);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uLAX_Immediate) {
    Test_LAX(&CpuTestBench::immediate, 0);
}

TEST_F(CpuTestUnofficial, uLAX_ZeroPage) {
    Test_LAX(&CpuTestBench::zeropage, 0);
}

TEST_F(CpuTestUnofficial, uLAX_ZeroPageY) {
    Test_LAX(&CpuTestBench::zeropage_y, 0);
}

TEST_F(CpuTestUnofficial, uLAX_ZeroPageY_Wraparound) {
    Test_LAX(&CpuTestBench::zeropage_y_wraparound, 0);
}

TEST_F(CpuTestUnofficial, uLAX_Absolute) {
    Test_LAX(&CpuTestBench::absolute, 0);
}

TEST_F(CpuTestUnofficial, uLAX_AbsoluteY) {
    Test_LAX(&CpuTestBench::absolute_y, 0);
}

TEST_F(CpuTestUnofficial, uLAX_AbsoluteY_CrossingPage) {
    Test_LAX(&CpuTestBench::absolute_y_crossing_page, 1);
}

TEST_F(CpuTestUnofficial, uLAX_IndexedIndirect) {
    Test_LAX(&CpuTestBench::indirect_x, 0);
}

TEST_F(CpuTestUnofficial, uLAX_IndexedIndirect_Wraparound) {
    Test_LAX(&CpuTestBench::indirect_x_wraparound, 0);
}

TEST_F(CpuTestUnofficial, uLAX_IndirectIndexed) {
    Test_LAX(&CpuTestBench::indirect_y, 0);
}

TEST_F(CpuTestUnofficial, uLAX_IndirectIndexed_CrossingPage) {
    Test_LAX(&CpuTestBench::indirect_y_crossing_page, 1);
}

TEST_F(CpuTestUnofficial, uLAX_IndirectIndexed_CrossingWordsize) {
    Test_LAX(&CpuTestBench::indirect_y_crossing_word_size, 1);
}

TEST_F(CpuTestUnofficial, uLAX_IndirectIndexed_BaseFromZeroPage) {
    Test_LAX(&CpuTestBench::indirect_y_base_from_zeropage, 0);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uLAS_AbsoluteY) {
    Test_LAS(&CpuTestBench::absolute_y, 0);
}

TEST_F(CpuTestUnofficial, uLAS_AbsoluteY_CrossingPage) {
    Test_LAS(&CpuTestBench::absolute_y_crossing_page, 1);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uDCP_ZeroPage) {
    Test_DCP(&CpuTestBench::zeropage);
}

TEST_F(CpuTestUnofficial, uDCP_ZeroPageX) {
    Test_DCP(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestUnofficial, uDCP_ZeroPageX_Wraparound) {
    Test_DCP(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestUnofficial, uDCP_Absolute) {
    Test_DCP(&CpuTestBench::absolute);
}

TEST_F(CpuTestUnofficial, uDCP_AbsoluteX) {
    Test_DCP(&CpuTestBench::absolute_x);
}

TEST_F(CpuTestUnofficial, uDCP_AbsoluteX_CrossingPage) {
    Test_DCP(&CpuTestBench::absolute_x_crossing_page);
}

TEST_F(CpuTestUnofficial, uDCP_AbsoluteY) {
    Test_DCP(&CpuTestBench::absolute_y);
}

TEST_F(CpuTestUnofficial, uDCP_AbsoluteY_CrossingPage) {
    Test_DCP(&CpuTestBench::absolute_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uDCP_IndexedIndirect) {
    Test_DCP(&CpuTestBench::indirect_x);
}

TEST_F(CpuTestUnofficial, uDCP_IndexedIndirect_Wraparound) {
    Test_DCP(&CpuTestBench::indirect_x_wraparound);
}

TEST_F(CpuTestUnofficial, uDCP_IndirectIndexed) {
    Test_DCP(&CpuTestBench::indirect_y);
}

TEST_F(CpuTestUnofficial, uDCP_IndirectIndexed_CrossingPage) {
    Test_DCP(&CpuTestBench::indirect_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uDCP_IndirectIndexed_CrossingWordsize) {
    Test_DCP(&CpuTestBench::indirect_y_crossing_word_size);
}

TEST_F(CpuTestUnofficial, uDCP_IndirectIndexed_BaseFromZeroPage) {
    Test_DCP(&CpuTestBench::indirect_y_base_from_zeropage);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uAXS_Immediate) {
    auto tester = [&](Byte a, Byte x, Byte m, Byte expX, Byte expN, Byte expZ, Byte expC) {
        bench.start();

        bench.set_target(m);
        cpu->A = a;
        cpu->X = x;
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(expX, cpu->X);
        EXPECT_EQ(expN, cpu->N);
        EXPECT_EQ(expZ, cpu->Z);
        EXPECT_EQ(expC, cpu->C);
    };

    bench.immediate(uAXS);

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
    Test_ISC(&CpuTestBench::zeropage);
}

TEST_F(CpuTestUnofficial, uISC_ZeroPageX) {
    Test_ISC(&CpuTestBench::zeropage_x);
}

TEST_F(CpuTestUnofficial, uISC_ZeroPageX_Wraparound) {
    Test_ISC(&CpuTestBench::zeropage_x_wraparound);
}

TEST_F(CpuTestUnofficial, uISC_Absolute) {
    Test_ISC(&CpuTestBench::absolute);
}

TEST_F(CpuTestUnofficial, uISC_AbsoluteX) {
    Test_ISC(&CpuTestBench::absolute_x);
}

TEST_F(CpuTestUnofficial, uISC_AbsoluteX_CrossingPage) {
    Test_ISC(&CpuTestBench::absolute_x_crossing_page);
}

TEST_F(CpuTestUnofficial, uISC_AbsoluteY) {
    Test_ISC(&CpuTestBench::absolute_y);
}

TEST_F(CpuTestUnofficial, uISC_AbsoluteY_CrossingPage) {
    Test_ISC(&CpuTestBench::absolute_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uISC_IndexedIndirect) {
    Test_ISC(&CpuTestBench::indirect_x);
}

TEST_F(CpuTestUnofficial, uISC_IndexedIndirect_Wraparound) {
    Test_ISC(&CpuTestBench::indirect_x_wraparound);
}

TEST_F(CpuTestUnofficial, uISC_IndirectIndexed) {
    Test_ISC(&CpuTestBench::indirect_y);
}

TEST_F(CpuTestUnofficial, uISC_IndirectIndexed_CrossingPage) {
    Test_ISC(&CpuTestBench::indirect_y_crossing_page);
}

TEST_F(CpuTestUnofficial, uISC_IndirectIndexed_CrossingWordsize) {
    Test_ISC(&CpuTestBench::indirect_y_crossing_word_size);
}

TEST_F(CpuTestUnofficial, uISC_IndirectIndexed_BaseFromZeroPage) {
    Test_ISC(&CpuTestBench::indirect_y_base_from_zeropage);
}

////////////////////////////////////////////////////////////////////////////////

TEST_F(CpuTestUnofficial, uSBC_Immediate) {
    auto tester = [&](Byte a, Byte m, Flag c, Byte expA, Flag expC, Flag expZ, Flag expV, Flag expN) {
        bench.start();

        bench.set_target(m);
        cpu->A = a;
        cpu->C = c;
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(expA, cpu->A);
        EXPECT_EQ(expC, cpu->C);
        EXPECT_EQ(expZ, cpu->Z);
        EXPECT_EQ(expV, cpu->V);
        EXPECT_EQ(expN, cpu->N);
    };

    bench.immediate(uSBC);
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
