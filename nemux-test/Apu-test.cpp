#include "gtest/gtest.h"

#include "Apu.h"

//using namespace std;

////////////////////////////////////////////////////////////////////////////////

struct ApuTest : public ::testing::Test {
    Apu apu;
    Pulse pulse;

    ApuTest() {}
};

TEST_F(ApuTest, Pulse_WritePeriod) {
    pulse.WritePeriodLow(0x95);
    pulse.WritePeriodHigh(0x95);
    EXPECT_EQ(0x0595 + 1, pulse.Period);

    pulse.WritePeriodLow(0xAA);
    EXPECT_EQ(0x05AA + 1, pulse.Period);
}

TEST_F(ApuTest, Pulse_WritePeriodHiRestartSequencer) {
    pulse.WriteControl(0x00);
    for (Byte x : { 0, 1, 0, 0 }) {
        EXPECT_EQ(x, pulse.TickSequence());
    }
    pulse.WritePeriodHigh(0x22);
    for (Byte x : { 0, 1, 0, 0, 0, 0, 0, 0 }) {
        EXPECT_EQ(x, pulse.TickSequence());
    }
}

TEST_F(ApuTest, Pulse_SilentIfPeriodSmallerThan8) {
    pulse.Enable(true);
    pulse.WriteControl(0x0F);
    pulse.WritePeriodLow(0x08);
    pulse.WritePeriodHigh(0x00);
    
    bool isSilent = true;
    for (int i = 0; i < 100; ++i) isSilent = isSilent && (pulse.Tick() == 0);
    EXPECT_FALSE(isSilent);

    pulse.WritePeriodLow(0x07);
    isSilent = true;
    for (int i = 0; i < 100; ++i) isSilent = isSilent && (pulse.Tick() == 0);
    EXPECT_TRUE(isSilent);
}

TEST_F(ApuTest, Pulse_SilentIfDisabled) {
    pulse.Enable(true);
    pulse.WritePeriodLow(0x08);
    pulse.WriteControl(0x0A);
    bool isSilent = true;
    for (int i = 0; i < 100; ++i) isSilent = isSilent && (pulse.Tick() == 0);
    EXPECT_FALSE(isSilent);

    pulse.Enable(false);
    isSilent = true;
    for (int i = 0; i < 100; ++i) isSilent = isSilent && (pulse.Tick() == 0);
    EXPECT_TRUE(isSilent);
}

TEST_F(ApuTest, Pulse_TimerTicksEveryTwoClocks) {
    pulse.WritePeriodLow(0x08);
    for (size_t n = 0; n < 4; n++) {
        EXPECT_TRUE(pulse.TickTimer()) << n;
        for (size_t i = 0; i < 17; i++)
        {
            EXPECT_FALSE(pulse.TickTimer()) << n << " " << i;
        }
    }
}

TEST_F(ApuTest, Pulse_Sequence) {
    pulse.WriteControl(0x00);
    for (Byte x : { 0, 1, 0, 0, 0, 0, 0, 0 }) {
        EXPECT_EQ(x, pulse.TickSequence());
    }
    
    pulse.WriteControl(0x40);
    for (Byte x : { 0, 1, 1, 0, 0, 0, 0, 0 }) {
        EXPECT_EQ(x, pulse.TickSequence());
    }

    pulse.WriteControl(0x80);
    for (Byte x : { 0, 1, 1, 1, 1, 0, 0, 0 }) {
        EXPECT_EQ(x, pulse.TickSequence());
    }

    pulse.WriteControl(0xC0);
    for (Byte x : { 1, 0, 0, 1, 1, 1, 1, 1 }) {
        EXPECT_EQ(x, pulse.TickSequence());
    }
}

TEST_F(ApuTest, Pulse_SetVolume) {
    pulse.WriteControl(0x0A);
    EXPECT_EQ(0x0A, pulse.Volume);
}

TEST_F(ApuTest, Pulse_WriteLength) {
    Byte counters[0x20] = {
        10,254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
        12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
    };
    for (int i = 0; i < 0x20; ++i) {
        pulse.WritePeriodHigh(i << 3);
        EXPECT_EQ(counters[i], pulse.Length);
    }
}

TEST_F(ApuTest, Pulse_LengthClearedOnDisable) {
    pulse.WritePeriodHigh(0x80);
    EXPECT_TRUE(pulse.Length > 0);

    pulse.Enable(false);
    EXPECT_TRUE(pulse.Length == 0);
}

TEST_F(ApuTest, Pulse_SilentAfterLength) {
    pulse.Enable(true);
    pulse.WritePeriodLow(0x08);
    pulse.WriteControl(0x0A);
    pulse.WritePeriodHigh(0x80); // Length is 12 (index 0x10 in the lookup table)
    
    bool isSilent = true;
    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(1, pulse.TickLength());
    }

    isSilent = true;
    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(0, pulse.TickLength());
    }
}

TEST_F(ApuTest, Pulse_LengthPausedDuringHalt) {
    pulse.Enable(true);
    pulse.WritePeriodLow(0x08);
    pulse.WriteControl(0x0A);
    pulse.WritePeriodHigh(0x80); // Length is 12 (index 0x10 in the lookup table)

    bool isSilent = true;
    for (int i = 0; i < 6; ++i) EXPECT_EQ(1, pulse.TickLength());

    pulse.WriteControl(0x2A);
    isSilent = true;
    for (int i = 0; i < 6; ++i) EXPECT_EQ(1, pulse.TickLength());
    
    pulse.WriteControl(0x0A);
    isSilent = true;
    for (int i = 0; i < 6; ++i) EXPECT_EQ(1, pulse.TickLength());

    isSilent = true;
    for (int i = 0; i < 12; ++i) EXPECT_EQ(0, pulse.TickLength());
}
