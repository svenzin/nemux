#include "CpuBaseTest.h"

using enum InstructionSet_6502::OpName;
using enum InstructionSet_6502::AddressingMode;
using enum BaseCpu::Bits;

struct CpuTestStack : public CpuBaseTest {
    template<InstructionSet_6502::OpName OP>
    void Test_Transfer(Byte& from, Byte& to) {
        auto tester = [&] (Byte value, Flag expZ, Flag expN) {
            bench.start();

            from = value;
            ExecuteOne();

            EXPECT_EQ(bench.expected_PC(), cpu->PC);
            EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
            EXPECT_EQ(value, to);
            EXPECT_EQ(expZ, cpu->Z);
            EXPECT_EQ(expN, cpu->N);
        };

        bench.implicit(OP);
        tester(0x20, 0, 0);
        tester(0xA0, 0, 1);
        tester(0x00, 1, 0);
    }
};

TEST_F(CpuTestStack, TSX) {
    Test_Transfer<TSX>(cpu->S, cpu->X );
}

TEST_F(CpuTestStack, TXS) {
    // TXS does not change the flags
    auto flags{ cpu->GetStatusByte(0) };
    bench.implicit(TXS).start();
    cpu->X = 0x20;
    ExecuteOne();

    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
    EXPECT_EQ(0x20, cpu->S);
    EXPECT_EQ(flags, cpu->GetStatusByte(0));
}

TEST_F(CpuTestStack, PHA) {
    cpu->A = 0x20;
    cpu->S = 0xF0;
    bench.implicit(PHA)
        //  .at(0x01F0).db_rw(0xFF) // simple write, read while checking expectations
         .stack_at(cpu->S).will_push(1)
         .start();
    ExecuteOne();

    EXPECT_EQ(bench.expected_PC(), cpu->PC);
    EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
    EXPECT_EQ(0xEF, cpu->S);
    EXPECT_EQ(0xEF, cpu->S);
    EXPECT_EQ(0x20, memory->GetByteAt(0x01F0));
}

TEST_F(CpuTestStack, PLA) {
    auto tester = [&] (Byte m, Flag expZ, Flag expN) {
        cpu->S = 0xEF;
        bench.reset()
             .implicit(PLA)
            //  .at(0x01F0).db(m)
            //  .at(0x01EF).db(0xFF) // dummy read before S increment
             .stack_at(cpu->S).will_pull({ m })
             .start();
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(m, cpu->A);
        EXPECT_EQ(0xF0, cpu->S);
        EXPECT_EQ(expZ, cpu->Z);
        EXPECT_EQ(expN, cpu->N);
    };

    bench.implicit(PLA);
    tester(0x20, 0, 0);
    tester(0x00, 1, 0);
    tester(0x80, 0, 1);
}

TEST_F(CpuTestStack, PLP) {
    auto tester = [&] (Byte m, Flag expN, Flag expV, Flag expD, Flag expI, Flag expZ, Flag expC) {
        cpu->S = 0xEF;
        bench.reset()
             .implicit(PLP)
            //  .at(0x01F0).db(m)
            //  .at(0x01EF).db(0xFF) // dummy read before S increment
             .stack_at(cpu->S).will_pull({ m })
             .start();
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(0xF0, cpu->S);
        EXPECT_EQ(expN, cpu->N);
        EXPECT_EQ(expV, cpu->V);
        EXPECT_EQ(expD, cpu->D);
        EXPECT_EQ(expI, cpu->I);
        EXPECT_EQ(expZ, cpu->Z);
        EXPECT_EQ(expC, cpu->C);
    };

    tester(0xB5, 1, 0, 0, 1, 0, 1); // Status 10xx0101b
    tester(0x6A, 0, 1, 1, 0, 1, 0); // Status 01xx1010b
}

TEST_F(CpuTestStack, PHP) {
    auto tester = [&] (Flag n, Flag v, Flag d, Flag i, Flag z, Flag c) {
        cpu->N = n;
        cpu->V = v;
        cpu->D = d;
        cpu->I = i;
        cpu->Z = z;
        cpu->C = c;
        cpu->S = 0xF0;

        bench.reset()
             .implicit(PHP)
            //  .at(0x01F0).db_rw(0xFF) // simple write, read while checking expectations
             .stack_at(cpu->S).will_push(1)
             .start();
        ExecuteOne();

        EXPECT_EQ(bench.expected_PC(), cpu->PC);
        EXPECT_EQ(bench.expected_ticks(0), cpu->GetTicks());
        EXPECT_EQ(0xEF, cpu->S);

        // PHP pushes Status byte with B flag and Unused bit set
        auto status{ memory->GetByteAt(0x01F0) };
        EXPECT_EQ(1, Bit<Brk>(status));
        EXPECT_EQ(1, Bit<Unu>(status));
        EXPECT_EQ(n, Bit<Neg>(status));
        EXPECT_EQ(v, Bit<Ovf>(status));
        EXPECT_EQ(d, Bit<Dec>(status));
        EXPECT_EQ(i, Bit<Int>(status));
        EXPECT_EQ(z, Bit<Zer>(status));
        EXPECT_EQ(c, Bit<Car>(status));
    };

    tester(0, 1, 1, 0, 1, 0);
    tester(1, 0, 0, 1, 0, 1);
}
