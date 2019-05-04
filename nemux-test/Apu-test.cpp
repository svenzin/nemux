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
