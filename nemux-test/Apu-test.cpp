#include "gtest/gtest.h"

#include "Apu.h"

//using namespace std;

////////////////////////////////////////////////////////////////////////////////

struct ApuTest : public ::testing::Test {
    Apu apu;
    Pulse pulse;
    FrameCounter frame;

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
    for (int i = 0; i < 100; ++i) isSilent = (pulse.Tick() == 0) && isSilent;
    EXPECT_FALSE(isSilent);

    pulse.WritePeriodLow(0x07);
    isSilent = true;
    for (int i = 0; i < 100; ++i) isSilent = (pulse.Tick() == 0) && isSilent;
    EXPECT_TRUE(isSilent);
}

TEST_F(ApuTest, Pulse_SilentIfDisabled) {
    pulse.Enable(true);
    pulse.WritePeriodLow(0x08);
    pulse.WritePeriodHigh(0xF0);
    pulse.WriteControl(0x2A);
    bool isSilent = true;
    for (int i = 0; i < 100; ++i) {
        isSilent = (pulse.Tick() == 0) && isSilent;
    }
    EXPECT_FALSE(isSilent);

    pulse.Enable(false);
    isSilent = true;
    for (int i = 0; i < 100; ++i) {
        isSilent = (pulse.Tick() == 0) && isSilent;
    }
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

TEST_F(ApuTest, FrameCounter_Mode4) {
    frame.WriteControl(0x00);
    const auto ExpectCounter = [](bool expHalf, bool expQuarter, FrameCounter::Clock actual) {
        EXPECT_EQ(expHalf, actual.HalfFrame);
        EXPECT_EQ(expQuarter, actual.QuarterFrame);
    };
    for (size_t i = 0; i < 29830; i++) {
        if (i == 7457) ExpectCounter(false, true, frame.Tick());
        else if (i == 14913) ExpectCounter(true, true, frame.Tick());
        else if (i == 22371) ExpectCounter(false, true, frame.Tick());
        else if (i == 29829) ExpectCounter(true, true, frame.Tick());
        else ExpectCounter(false, false, frame.Tick());
    }
    // Loop
    for (size_t i = 0; i < 29830; i++) {
        if (i == 7457) ExpectCounter(false, true, frame.Tick());
        else if (i == 14913) ExpectCounter(true, true, frame.Tick());
        else if (i == 22371) ExpectCounter(false, true, frame.Tick());
        else if (i == 29829) ExpectCounter(true, true, frame.Tick());
        else ExpectCounter(false, false, frame.Tick());
    }
}

TEST_F(ApuTest, FrameCounter_Mode5) {
    frame.WriteControl(0x80);
    const auto ExpectCounter = [](bool expHalf, bool expQuarter, FrameCounter::Clock actual) {
        EXPECT_EQ(expHalf, actual.HalfFrame);
        EXPECT_EQ(expQuarter, actual.QuarterFrame);
    };
    for (size_t i = 0; i < 37282; i++) {
        if (i == 7457) ExpectCounter(false, true, frame.Tick());
        else if (i == 14913) ExpectCounter(true, true, frame.Tick());
        else if (i == 22371) ExpectCounter(false, true, frame.Tick());
        else if (i == 37281) ExpectCounter(true, true, frame.Tick());
        else ExpectCounter(false, false, frame.Tick());
    }
    // Loop
    for (size_t i = 0; i < 37282; i++) {
        if (i == 7457) ExpectCounter(false, true, frame.Tick());
        else if (i == 14913) ExpectCounter(true, true, frame.Tick());
        else if (i == 22371) ExpectCounter(false, true, frame.Tick());
        else if (i == 37281) ExpectCounter(true, true, frame.Tick());
        else ExpectCounter(false, false, frame.Tick());
    }
}

TEST_F(ApuTest, Pulse_Sweep_SetValues) {
    pulse.WriteSweep(0x00);
    EXPECT_EQ(false, pulse.SweepEnabled);

    pulse.WriteSweep(0x80);
    EXPECT_EQ(true, pulse.SweepEnabled);

    pulse.WriteSweep(0x00);
    EXPECT_EQ(false, pulse.SweepNegate);

    pulse.WriteSweep(0x08);
    EXPECT_EQ(true, pulse.SweepNegate);

    pulse.WriteSweep(0x50);
    EXPECT_EQ(5, pulse.SweepPeriod);

    pulse.WriteSweep(0x05);
    EXPECT_EQ(5, pulse.SweepAmount);
}

TEST_F(ApuTest, Pulse_Sweep_TargetPeriods) {
    // Shift 2 bits, add
    pulse.Period = 0x0142;
    pulse.WriteSweep(0x02);
    pulse.TickSweep(true);
    EXPECT_EQ(0x0142 + 0x0050, pulse.SweepTargetPeriod);

    // Shift 5 bits, substract
    pulse.Period = 0x0142;
    pulse.WriteSweep(0x0D);
    pulse.TickSweep(true);
    EXPECT_EQ(0x0142 - 0x000A, pulse.SweepTargetPeriod);
}

TEST_F(ApuTest, Pulse_Sweep_MuteOnInvalidTargetPeriod) {
    // Shift 0 bits, add
    pulse.Period = 0x03FF;
    pulse.WriteSweep(0x00);
    EXPECT_EQ(1, pulse.TickSweep(true));
    
    pulse.Period = 0x0400;
    pulse.WriteSweep(0x00);
    EXPECT_EQ(0, pulse.TickSweep(true));

    // Mute on period < 8 (shift 8 bits, negate -> no period change)
    pulse.Period = 0x04;
    pulse.WriteSweep(0x0F);
    EXPECT_EQ(0, pulse.TickSweep(true));
}

TEST_F(ApuTest, Pulse_Sweep_UpdatePeriod) {
    // Enabled, Period = 4 half-frames
    pulse.Period = 0x0142;
    pulse.WriteSweep(0xC2);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142 + 0x0050, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142 + 0x0050, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142 + 0x0050, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142 + 0x0050, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142 + 0x0050 + 0x0064, pulse.Period);

    // Disabled, Period = 4 half-frames
    pulse.Period = 0x0142;
    pulse.WriteSweep(0x72);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);

}

TEST_F(ApuTest, Pulse_Sweep_NoUpdateWhenMuting) {
    // Enabled, Period = 4 half-frames
    // Muted
    pulse.Period = 0x0800;
    pulse.WriteSweep(0xC2);
    EXPECT_EQ(0, pulse.TickSweep(true));
    EXPECT_EQ(0x0800, pulse.Period);
    EXPECT_EQ(0, pulse.TickSweep(true));
    EXPECT_EQ(0x0800, pulse.Period);
    EXPECT_EQ(0, pulse.TickSweep(true));
    EXPECT_EQ(0x0800, pulse.Period);
    EXPECT_EQ(0, pulse.TickSweep(true));
    EXPECT_EQ(0x0800, pulse.Period);
}

TEST_F(ApuTest, Pulse_Sweep_NoUpdateWhenShiftIsZero) {
    // Enabled, Period = 4 half-frames
    pulse.Period = 0x0142;
    pulse.WriteSweep(0xC0);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.Period);
}

TEST_F(ApuTest, Pulse_Sweep_Reload) {
    // Enabled, Period = 5
    pulse.Period = 0x0100;
    pulse.WriteSweep(0xD1);
    EXPECT_EQ(5, pulse.SweepPeriod);
    EXPECT_EQ(0, pulse.SweepT);
    pulse.TickSweep(true);
    EXPECT_EQ(5, pulse.SweepPeriod);
    EXPECT_EQ(5, pulse.SweepT);
    pulse.WriteSweep(0xE1);
    EXPECT_EQ(6, pulse.SweepPeriod);
    EXPECT_EQ(5, pulse.SweepT);
    pulse.TickSweep(true);
    EXPECT_EQ(6, pulse.SweepPeriod);
    EXPECT_EQ(6, pulse.SweepT);
}

TEST_F(ApuTest, Pulse_Envelope_ConstantVolume) {
    // Set Volume to constant at 0x0A
    pulse.WriteControl(0x1A);
    for (size_t i = 0; i < 20; i++) {
        EXPECT_EQ(0x0A, pulse.Envelope.Tick(false));
    }
    for (size_t i = 0; i < 20; i++) {
        EXPECT_EQ(0x0A, pulse.Envelope.Tick(true));
    }
}

TEST_F(ApuTest, Pulse_Envelope_DecreasingEnvelope) {
    pulse.WriteControl(0x00);

    // Write CounterLength sets restart flag
    EXPECT_FALSE(pulse.Envelope.Restart);
    pulse.WritePeriodHigh(0x22);
    EXPECT_TRUE(pulse.Envelope.Restart);

    // First tick reloads
    EXPECT_EQ(15, pulse.Envelope.Tick(true));
    EXPECT_FALSE(pulse.Envelope.Restart);

    // Non-looping envelope
    for (size_t i = 1   ; i <= 15; i++) {
        EXPECT_EQ(15 - i, pulse.Envelope.Tick(true));
    }
    for (size_t i = 0; i <= 15; i++) {
        EXPECT_EQ(0, pulse.Envelope.Tick(true));
    }
}

TEST_F(ApuTest, Pulse_Envelope_DecreasingEnvelopeNotOnNonQuarterFrames) {
    pulse.WriteControl(0x00);
    pulse.WritePeriodHigh(0x22);

    // First tick reloads
    EXPECT_EQ(15, pulse.Envelope.Tick(true));

    // Further non-QFrame ticks don't change anything
    for (size_t i = 1; i <= 15; i++) {
        EXPECT_EQ(15, pulse.Envelope.Tick(false));
    }
    for (size_t i = 0; i <= 15; i++) {
        EXPECT_EQ(15, pulse.Envelope.Tick(false));
    }
}

TEST_F(ApuTest, Pulse_Envelope_DecreasingEnvelopeOnLoop) {
    // Looping envelope
    pulse.WriteControl(0x20);
    pulse.WritePeriodHigh(0x22);
    for (size_t i = 0; i <= 15; i++) {
        EXPECT_EQ(15 - i, pulse.Envelope.Tick(true));
    }
    for (size_t i = 0; i <= 15; i++) {
        EXPECT_EQ(15 - i, pulse.Envelope.Tick(true));
    }
}

TEST_F(ApuTest, Pulse_Envelope_DividerWithLoop) {
    // Looping envelope, Period 4+1
    pulse.WriteControl(0x24);

    // Write CounterLength sets restart flag
    pulse.WritePeriodHigh(0x22);
    EXPECT_EQ(15, pulse.Envelope.Tick(true));
    EXPECT_EQ(4 + 1, pulse.Envelope.Divider);
    
    for (size_t i = 0; i <= 15; i++) {
        for (size_t j = 0; j < 4+1; j++)
        {
            if (i == 0 && j == 0) continue;
            EXPECT_EQ(15 - i, pulse.Envelope.Tick(true));
        }
    }
    for (size_t i = 0; i <= 15; i++) {
        for (size_t j = 0; j < 4+1; j++)
        {
            EXPECT_EQ(15 - i, pulse.Envelope.Tick(true));
        }
    }
}
