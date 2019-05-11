#include "gtest/gtest.h"

#include "Apu.h"

//using namespace std;

////////////////////////////////////////////////////////////////////////////////

struct ApuTest : public ::testing::Test {
    Apu apu;
    Pulse pulse;
    FrameCounter frame;
    LengthCounter length;
    Timer timer;
    TriangleSequencer tri;
    LinearCounter linear;
    Noise noise;
    ShiftRegister shift;

    ApuTest() {}

    Byte CreatePulseControl(int duty, bool halt, bool constant, int volume) {
        return ((duty & 0x03) << 6) + Mask<5>(halt) + Mask<4>(constant) + (volume & 0x0F);
    }
};

TEST_F(ApuTest, Pulse_WritePeriod) {
    timer.SetPeriodLow(0x95);
    timer.SetPeriodHigh(0x05);
    EXPECT_EQ(0x0595 + 1, timer.Period);

    timer.SetPeriodLow(0xAA);
    EXPECT_EQ(0x05AA + 1, timer.Period);
}

TEST_F(ApuTest, Pulse_WritePeriodHiRestartSequencer) {
    const auto ctrl = CreatePulseControl(0, false, false, 0);
    pulse.WriteControl(ctrl);
    for (Byte x : { 0, 1, 0, 0 }) {
        EXPECT_EQ(x, pulse.Sequence.Tick(true));
    }
    pulse.WritePeriodHigh(0x22);
    for (Byte x : { 0, 1, 0, 0, 0, 0, 0, 0 }) {
        EXPECT_EQ(x, pulse.Sequence.Tick(true));
    }
}

TEST_F(ApuTest, Pulse_SilentIfPeriodSmallerThan8) {
    pulse.Enable(true);
    const auto ctrl = CreatePulseControl(0, false, true, 0xF);
    pulse.WriteControl(ctrl);
    pulse.WritePeriodLow(0x08);
    pulse.WritePeriodHigh(0x00);
    
    bool isSilent = true;
    for (int i = 0; i < 100; ++i) {
        auto clock = frame.Tick();
        isSilent = (pulse.Tick(clock) == 0) && isSilent;
    }
    EXPECT_FALSE(isSilent);

    pulse.WritePeriodLow(0x07);
    isSilent = true;
    for (int i = 0; i < 100; ++i) {
        auto clock = frame.Tick();
        isSilent = (pulse.Tick(clock) == 0) && isSilent;
    }
    EXPECT_TRUE(isSilent);
}

TEST_F(ApuTest, Pulse_SilentIfDisabled) {
    // Enabled, Period 0x0A08, Length 0, Duty 0, Halt length, Constant Volume, Volume A
    pulse.Enable(true);
    pulse.WritePeriodLow(0x08);
    pulse.WritePeriodHigh(0x00);
    const auto ctrl = CreatePulseControl(0, true, true, 0xA);
    pulse.WriteControl(ctrl);
    bool isSilent = true;
    for (int i = 0; i < 100; ++i) {
        auto clock = frame.Tick();
        isSilent = (pulse.Tick(clock) == 0) && isSilent;
    }
    EXPECT_FALSE(isSilent);

    pulse.Enable(false);
    isSilent = true;
    for (int i = 0; i < 100; ++i) {
        auto clock = frame.Tick();
        isSilent = (pulse.Tick(clock) == 0) && isSilent;
    }
    EXPECT_TRUE(isSilent);
}

TEST_F(ApuTest, Pulse_TimerTicksEveryTwoClocks) {
    pulse.Enable(true);
    pulse.WritePeriodLow(0x08);
    for (size_t n = 0; n < 4; n++) {
        EXPECT_EQ(0, pulse.T.T) << n;
        pulse.Tick(frame.Tick());
        for (size_t i = 0; i < 8; i++)
        {
            EXPECT_NE(0, pulse.T.T) << n << " " << i;
            pulse.Tick(frame.Tick());
            EXPECT_NE(0, pulse.T.T) << n << " " << i;
            pulse.Tick(frame.Tick());
        }
        EXPECT_EQ(0, pulse.T.T) << n;
        pulse.Tick(frame.Tick());
    }
}

TEST_F(ApuTest, Pulse_Sequence) {
    auto ctrl = CreatePulseControl(0, false, false, 0);
    pulse.WriteControl(ctrl);
    for (Byte x : { 0, 1, 0, 0, 0, 0, 0, 0 }) {
        EXPECT_EQ(x, pulse.Sequence.Tick(true));
    }
    
    ctrl = CreatePulseControl(1, false, false, 0);
    pulse.WriteControl(ctrl);
    for (Byte x : { 0, 1, 1, 0, 0, 0, 0, 0 }) {
        EXPECT_EQ(x, pulse.Sequence.Tick(true));
    }

    ctrl = CreatePulseControl(2, false, false, 0);
    pulse.WriteControl(ctrl);
    for (Byte x : { 0, 1, 1, 1, 1, 0, 0, 0 }) {
        EXPECT_EQ(x, pulse.Sequence.Tick(true));
    }

    ctrl = CreatePulseControl(3, false, false, 0);
    pulse.WriteControl(ctrl);
    for (Byte x : { 1, 0, 0, 1, 1, 1, 1, 1 }) {
        EXPECT_EQ(x, pulse.Sequence.Tick(true));
    }
}

TEST_F(ApuTest, Pulse_SequenceNoChangeWhenNotTimer) {
    auto ctrl = CreatePulseControl(0, false, false, 0);
    pulse.WriteControl(ctrl);
    for (Byte x : { 0, 1, 0, 0, 0, 0, 0, 0 }) {
        EXPECT_EQ(0, pulse.Sequence.Tick(false));
    }
}

TEST_F(ApuTest, Pulse_SetVolume) {
    const auto ctrl = CreatePulseControl(0, false, false, 0xA);
    pulse.WriteControl(ctrl);
    EXPECT_EQ(0x0A, pulse.Envelope.Volume);
}

TEST_F(ApuTest, Pulse_WriteLength) {
    Byte counters[0x20] = {
        10,254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
        12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
    };
    for (int i = 0; i < 0x20; ++i) {
        length.SetCountIndex(i);
        EXPECT_EQ(counters[i], length.Count);
    }
}

TEST_F(ApuTest, Pulse_LengthClearedOnDisable) {
    pulse.WritePeriodHigh(0x80);
    EXPECT_TRUE(pulse.Length.Count > 0);

    pulse.Enable(false);
    EXPECT_TRUE(pulse.Length.Count == 0);
}

TEST_F(ApuTest, Pulse_SilentAfterLength) {
    length.SetCountIndex(0x10);
    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(1, length.Tick(true));
    }
    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(0, length.Tick(true));
    }
}

TEST_F(ApuTest, Pulse_LengthNoChangeWhenNotHalfFrame) {
    length.SetCountIndex(0x10);

    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(1, length.Tick(false));
    }
    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(1, length.Tick(false));
    }
}

TEST_F(ApuTest, Pulse_LengthPausedDuringHalt) {
    length.SetCountIndex(0x10);
    for (int i = 0; i < 6; ++i) EXPECT_EQ(1, length.Tick(true));
    length.Halt = true;
    for (int i = 0; i < 6; ++i) EXPECT_EQ(1, length.Tick(true));
    length.Halt = false;
    for (int i = 0; i < 6; ++i) EXPECT_EQ(1, length.Tick(true));
    for (int i = 0; i < 12; ++i) EXPECT_EQ(0, length.Tick(true));
}

TEST_F(ApuTest, FrameCounter_Mode4WithoutInterrupt) {
    frame.WriteControl(0x40);
    EXPECT_TRUE(frame.HideInterrupt);
    const auto ExpectCounter = [](bool expHalf, bool expQuarter, FrameCounter::Clock actual) {
        EXPECT_EQ(expHalf, actual.HalfFrame);
        EXPECT_EQ(expQuarter, actual.QuarterFrame);
        EXPECT_FALSE(actual.Interrupt);
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

TEST_F(ApuTest, FrameCounter_Mode4WithInterrupt) {
    frame.WriteControl(0x00);
    EXPECT_FALSE(frame.HideInterrupt);
    const auto ExpectCounter = [](bool expHalf, bool expQuarter, bool expInterrupt, FrameCounter::Clock actual) {
        EXPECT_EQ(expHalf, actual.HalfFrame);
        EXPECT_EQ(expQuarter, actual.QuarterFrame);
        EXPECT_EQ(expInterrupt, actual.Interrupt);
    };
    for (size_t i = 0; i < 29830; i++) {
        if (i < 7457) ExpectCounter(false, false, false, frame.Tick());
        else if (i == 7457) ExpectCounter(false, true, false, frame.Tick());
        else if (i < 14913) ExpectCounter(false, false, false, frame.Tick());
        else if (i == 14913) ExpectCounter(true, true, false, frame.Tick());
        else if (i < 22371) ExpectCounter(false, false, false, frame.Tick());
        else if (i == 22371) ExpectCounter(false, true, false, frame.Tick());
        else if (i < 29828) ExpectCounter(false, false, false, frame.Tick());
        else if (i == 29828) ExpectCounter(false, false, true, frame.Tick());
        else if (i == 29829) ExpectCounter(true, true, true, frame.Tick());
    }
    // Loop
    for (size_t i = 0; i < 29830; i++) {
        if (i == 0) ExpectCounter(false, false, true, frame.Tick());
        else if (i < 7457) ExpectCounter(false, false, false, frame.Tick());
        else if (i == 7457) ExpectCounter(false, true, false, frame.Tick());
        else if (i < 14913) ExpectCounter(false, false, false, frame.Tick());
        else if (i == 14913) ExpectCounter(true, true, false, frame.Tick());
        else if (i < 22371) ExpectCounter(false, false, false, frame.Tick());
        else if (i == 22371) ExpectCounter(false, true, false, frame.Tick());
        else if (i < 29828) ExpectCounter(false, false, false, frame.Tick());
        else if (i == 29828) ExpectCounter(false, false, true, frame.Tick());
        else if (i == 29829) ExpectCounter(true, true, true, frame.Tick());
    }
}

TEST_F(ApuTest, FrameCounter_Mode5) {
    frame.WriteControl(0x80);
    const auto ExpectCounter = [](bool expHalf, bool expQuarter, FrameCounter::Clock actual) {
        EXPECT_EQ(expHalf, actual.HalfFrame);
        EXPECT_EQ(expQuarter, actual.QuarterFrame);
        EXPECT_FALSE(actual.Interrupt);
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

TEST_F(ApuTest, FrameCounter_ReadStatus) {
    apu.WriteCommonControl(0x00);
    
    // Reading status clears frame interrupt for the rest of the frame
    for (size_t i = 0; i < 29828; i++) {
        apu.Tick();
        EXPECT_FALSE(apu.FrameInterrupt);
    }
    EXPECT_FALSE(IsBitSet<6>(apu.ReadStatus()));

    apu.Tick();
    EXPECT_TRUE(apu.FrameInterrupt);
    EXPECT_TRUE(IsBitSet<6>(apu.ReadStatus()));

    apu.Tick();
    EXPECT_FALSE(apu.FrameInterrupt);
    EXPECT_FALSE(IsBitSet<6>(apu.ReadStatus()));

    // Next frame
    apu.Tick();
    EXPECT_FALSE(apu.FrameInterrupt); // First frame tick is previous frame's interrupt
    for (size_t i = 1; i < 29828; i++) {
        apu.Tick();
        EXPECT_FALSE(apu.FrameInterrupt);
    }
    apu.Tick();
    EXPECT_TRUE(apu.FrameInterrupt);
    apu.Tick();
    EXPECT_TRUE(apu.FrameInterrupt);
    apu.Tick();
    EXPECT_TRUE(apu.FrameInterrupt);
    apu.Tick();
    EXPECT_FALSE(apu.FrameInterrupt);
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
    EXPECT_FALSE(pulse.SweepAlternativeNegate);

    // Shift 2 bits, add
    pulse.T.Period = 0x0142;
    pulse.WriteSweep(0x02);
    pulse.TickSweep(true);
    EXPECT_EQ(0x0142 + 0x0050, pulse.SweepTargetPeriod);

    // Shift 5 bits, substract
    pulse.T.Period = 0x0142;
    pulse.WriteSweep(0x0D);
    pulse.TickSweep(true);
    EXPECT_EQ(0x0142 - 0x000A, pulse.SweepTargetPeriod);

    // Shift 5 bits, substract, alternative method (add -N-1)
    pulse.T.Period = 0x0142;
    pulse.WriteSweep(0x0D);
    pulse.SweepAlternativeNegate = true;
    pulse.TickSweep(true);
    EXPECT_EQ(0x0142 - 0x000A - 1, pulse.SweepTargetPeriod);
}

TEST_F(ApuTest, Pulse_Sweep_MuteOnInvalidTargetPeriod) {
    // Shift 0 bits, add
    pulse.T.Period = 0x03FF;
    pulse.WriteSweep(0x00);
    EXPECT_EQ(1, pulse.TickSweep(true));
    
    pulse.T.Period = 0x0400;
    pulse.WriteSweep(0x00);
    EXPECT_EQ(0, pulse.TickSweep(true));

    // Mute on period < 8 (shift 8 bits, negate -> no period change)
    pulse.T.Period = 0x04;
    pulse.WriteSweep(0x0F);
    EXPECT_EQ(0, pulse.TickSweep(true));
}

TEST_F(ApuTest, Pulse_Sweep_UpdatePeriod) {
    // Enabled, Period = 4 half-frames
    pulse.T.Period = 0x0142;
    pulse.WriteSweep(0xC2);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142 + 0x0050, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142 + 0x0050, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142 + 0x0050, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142 + 0x0050, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142 + 0x0050 + 0x0064, pulse.T.Period);

    // Disabled, Period = 4 half-frames
    pulse.T.Period = 0x0142;
    pulse.WriteSweep(0x72);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);

}

TEST_F(ApuTest, Pulse_Sweep_NoUpdateWhenMuting) {
    // Enabled, Period = 4 half-frames
    // Muted
    pulse.T.Period = 0x0800;
    pulse.WriteSweep(0xC2);
    EXPECT_EQ(0, pulse.TickSweep(true));
    EXPECT_EQ(0x0800, pulse.T.Period);
    EXPECT_EQ(0, pulse.TickSweep(true));
    EXPECT_EQ(0x0800, pulse.T.Period);
    EXPECT_EQ(0, pulse.TickSweep(true));
    EXPECT_EQ(0x0800, pulse.T.Period);
    EXPECT_EQ(0, pulse.TickSweep(true));
    EXPECT_EQ(0x0800, pulse.T.Period);
}

TEST_F(ApuTest, Pulse_Sweep_NoUpdateWhenShiftIsZero) {
    // Enabled, Period = 4 half-frames
    pulse.T.Period = 0x0142;
    pulse.WriteSweep(0xC0);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);
    EXPECT_EQ(1, pulse.TickSweep(true));
    EXPECT_EQ(0x0142, pulse.T.Period);}

TEST_F(ApuTest, Pulse_Sweep_Reload) {
    // Enabled, Period = 5
    pulse.T.Period = 0x0100;
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
    const auto ctrl = CreatePulseControl(0, false, true, 0xA);
    pulse.WriteControl(ctrl);
    for (size_t i = 0; i < 20; i++) {
        EXPECT_EQ(0x0A, pulse.Envelope.Tick(false));
    }
    for (size_t i = 0; i < 20; i++) {
        EXPECT_EQ(0x0A, pulse.Envelope.Tick(true));
    }
}

TEST_F(ApuTest, Pulse_Envelope_DecreasingEnvelope) {
    const auto ctrl = CreatePulseControl(0, false, false, 0);
    pulse.WriteControl(ctrl);

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
    const auto ctrl = CreatePulseControl(0, false, false, 0);
    pulse.WriteControl(ctrl);
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
    const auto ctrl = CreatePulseControl(0, true, false, 0);
    pulse.WriteControl(ctrl);
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
    const auto ctrl = CreatePulseControl(0, true, false, 4);
    pulse.WriteControl(ctrl);

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

TEST_F(ApuTest, Triangle_Sequence) {
    for (auto i : {
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    }) {
        EXPECT_EQ(i, tri.Tick(true));
    }
}

TEST_F(ApuTest, Triangle_SequenceNoChangeWhenNotTimer) {
    for (auto i : {
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    }) {
        EXPECT_EQ(i, tri.Tick(true));
    }
    EXPECT_EQ(11, tri.Tick(false));
    EXPECT_EQ(11, tri.Tick(false));
    EXPECT_EQ(11, tri.Tick(false));
    for (auto i : { 11, 12, 13, 14, 15 }) {
        EXPECT_EQ(i, tri.Tick(true));
    }
}

TEST_F(ApuTest, Triangle_LinearCounter) {
    linear.Count = 100;
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(1, linear.Tick(true));
    }
    EXPECT_EQ(0, linear.Tick(true));
    EXPECT_EQ(0, linear.Tick(true));
    EXPECT_EQ(0, linear.Tick(true));
    EXPECT_EQ(0, linear.Tick(true));
}

TEST_F(ApuTest, Triangle_LinearCounterReload) {
    linear.Count = 100;
    linear.ReloadValue = 100;
    linear.Tick(true);
    EXPECT_EQ(99, linear.Count);
    linear.Reload = true;
    linear.Tick(true);
    EXPECT_EQ(100, linear.Count);
    linear.Reload = false;
    linear.Tick(true);
    EXPECT_EQ(99, linear.Count);
}

TEST_F(ApuTest, Triangle_LinearCounterClearReloadOnControl) {
    EXPECT_FALSE(linear.Reload);
    EXPECT_TRUE(linear.Control);
    
    linear.Tick(true);
    EXPECT_FALSE(linear.Reload);

    linear.Reload = true;
    linear.Tick(true);
    EXPECT_TRUE(linear.Reload);

    linear.Control = false;
    linear.Tick(true);
    EXPECT_FALSE(linear.Reload);

    linear.Tick(true);
    EXPECT_FALSE(linear.Reload);
}

TEST_F(ApuTest, Triangle_LinearCounterNoChangeWhenNoQFrame) {
    linear.Count = 100;
    linear.ReloadValue = 100;

    EXPECT_EQ(1, linear.Tick(true));
    EXPECT_EQ(99, linear.Count);

    EXPECT_EQ(1, linear.Tick(false));
    EXPECT_EQ(99, linear.Count);

    linear.Reload = true;
    linear.Tick(false);
    EXPECT_EQ(99, linear.Count);
    linear.Tick(true);
    EXPECT_EQ(100, linear.Count);

    linear.Control = false;
    linear.Tick(false);
    EXPECT_TRUE(linear.Reload);
    linear.Tick(true);
    EXPECT_FALSE(linear.Reload);
}

TEST_F(ApuTest, Noise_RandomSetPeriod) {
    int Periods[0x10] = {
        4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
    };
    for (size_t i = 0; i < 0x10; i++) {
        noise.WritePeriod(i);
        EXPECT_EQ(Periods[i], noise.T.Period);
    }
}

TEST_F(ApuTest, Noise_PowerUpState) {
    // Loaded with 1 on power-up
    EXPECT_EQ(1, shift.Value);
}

TEST_F(ApuTest, Noise_Mode0) {
    // Mode 0 creates a new bit from bit0 XOR bit1 before shifting
    shift.Value = 0x29A6; //  **10 1001 1010 0110 b

    EXPECT_EQ(0, shift.Tick(true)); // Shift bit 0 out
    EXPECT_EQ(0x34D3, shift.Value); // 0 xor 1 = 1 -> **11 0100 1101 0011 b

    EXPECT_EQ(1, shift.Tick(true)); // Shift bit 0 out
    EXPECT_EQ(0x1A69, shift.Value); // 1 xor 1 = 0 -> **01 1010 0110 1001 b

    EXPECT_EQ(1, shift.Tick(true)); // Shift bit 0 out
    EXPECT_EQ(0x2D34, shift.Value); // 1 xor 0 = 1 -> **10 1101 0011 0100 b

    EXPECT_EQ(0, shift.Tick(true)); // Shift bit 0 out
    EXPECT_EQ(0x169A, shift.Value); // 0 xor 0 = 0 -> **01 0110 1001 1010 b
}

TEST_F(ApuTest, Noise_Mode1) {
    // Mode 1 creates a new bit from bit0 XOR bit6 before shifting
    shift.Value = 0x2956; //  **10 1001 0101 0110 b
    shift.Mode = true;

    EXPECT_EQ(0, shift.Tick(true)); // Shift bit 0 out
    EXPECT_EQ(0x34AB, shift.Value); // 0 xor 1 = 1 -> **11 0100 1010 1011 b

    EXPECT_EQ(1, shift.Tick(true)); // Shift bit 0 out
    EXPECT_EQ(0x3A55, shift.Value); // 1 xor 0 = 1 -> **11 1010 0101 0101 b

    EXPECT_EQ(1, shift.Tick(true)); // Shift bit 0 out
    EXPECT_EQ(0x1D2A, shift.Value); // 1 xor 1 = 0 -> **01 1101 0010 1010 b

    EXPECT_EQ(0, shift.Tick(true)); // Shift bit 0 out
    EXPECT_EQ(0x0E95, shift.Value); // 0 xor 0 = 0 -> **00 1110 1001 0101 b

    EXPECT_EQ(1, shift.Tick(true)); // Shift bit 0 out
    EXPECT_EQ(0x274A, shift.Value); // 1 xor 0 = 1 -> **10 0111 0100 1010 b

    EXPECT_EQ(0, shift.Tick(true)); // Shift bit 0 out
    EXPECT_EQ(0x33A5, shift.Value); // 0 xor 1 = 1 -> **11 0011 1010 0101 b
}

TEST_F(ApuTest, Status_ReadLengths) {
    apu.Pulse1.Length.Clear();
    EXPECT_FALSE(IsBitSet<0>(apu.ReadStatus()));
    apu.Pulse1.Length.Count = 10;
    EXPECT_TRUE(IsBitSet<0>(apu.ReadStatus()));

    apu.Pulse2.Length.Clear();
    EXPECT_FALSE(IsBitSet<1>(apu.ReadStatus()));
    apu.Pulse2.Length.Count = 10;
    EXPECT_TRUE(IsBitSet<1>(apu.ReadStatus()));

    apu.Triangle1.Length.Clear();
    EXPECT_FALSE(IsBitSet<2>(apu.ReadStatus()));
    apu.Triangle1.Length.Count = 10;
    EXPECT_TRUE(IsBitSet<2>(apu.ReadStatus()));

    apu.Noise1.Length.Clear();
    EXPECT_FALSE(IsBitSet<3>(apu.ReadStatus()));
    apu.Noise1.Length.Count = 10;
    EXPECT_TRUE(IsBitSet<3>(apu.ReadStatus()));
}
