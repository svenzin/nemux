#include "gtest/gtest.h"

#include "VRC6_Audio.h"

#define CHANNEL_ENABLE  Mask<7>(true)
#define CHANNEL_DISABLE Mask<7>(false)

struct VRC6AudioTest : public ::testing::Test {
    VRC6_Audio vrc6;

    VRC6AudioTest() {}
};

TEST_F(VRC6AudioTest, Pulse_Duty) {
    // Enable, normal scaling
    vrc6.WriteFrequencyScaling(0);

    // Period 0 => each tick triggers the duty cycle
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Pulse1.WritePeriodLo(0);
    
    const auto checkDuty = [this](const int n) {
        // Check two complete cycles
        for (int cycle = 0; cycle < 2; ++cycle) {
            for (int i = 0; i < 16 - n; ++i) EXPECT_EQ(0b0000, vrc6.Pulse1.Tick()) << "Duty " << n;
            for (int i = 16 - n; i < 16; ++i) EXPECT_EQ(0b1111, vrc6.Pulse1.Tick()) << "Duty " << n;
        }
    };
    
    // Duty 1/16
    vrc6.Pulse1.WriteControl(0b00001111);
    checkDuty(1);

    // Duty 2/16
    vrc6.Pulse1.WriteControl(0b00011111);
    checkDuty(2);

    // Duty 3/16
    vrc6.Pulse1.WriteControl(0b00101111);
    checkDuty(3);

    // Duty 4/16
    vrc6.Pulse1.WriteControl(0b00111111);
    checkDuty(4);

    // Duty 5/16
    vrc6.Pulse1.WriteControl(0b01001111);
    checkDuty(5);

    // Duty 6/16
    vrc6.Pulse1.WriteControl(0b01011111);
    checkDuty(6);

    // Duty 7/16
    vrc6.Pulse1.WriteControl(0b01101111);
    checkDuty(7);

    // Duty 8/16
    vrc6.Pulse1.WriteControl(0b01111111);
    checkDuty(8);
}

TEST_F(VRC6AudioTest, Pulse_Volume) {
    // Enable, normal scaling
    vrc6.WriteFrequencyScaling(0);

    // Period 0 => each tick triggers the duty cycle
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Pulse1.WritePeriodLo(0);

    const auto checkVolume = [this](const int v) {
        // Check two complete cycles
        for (int cycle = 0; cycle < 2; ++cycle) {
            for (int i = 0; i < 8; ++i) EXPECT_EQ(0, vrc6.Pulse1.Tick());
            for (int i = 8; i < 16; ++i) EXPECT_EQ(v, vrc6.Pulse1.Tick());
        }
    };

    // Duty 8/16, Volume 0
    vrc6.Pulse1.WriteControl(0b01110000);
    checkVolume(0);
    
    // Duty 8/16, Volume 3
    vrc6.Pulse1.WriteControl(0b01110011);
    checkVolume(3);
    
    // Duty 8/16, Volume 6
    vrc6.Pulse1.WriteControl(0b01110110);
    checkVolume(6);

    // Duty 8/16, Volume max
    vrc6.Pulse1.WriteControl(0b01111111);
    checkVolume(15);
}

TEST_F(VRC6AudioTest, Pulse_Mode) {
    // Enable, normal scaling
    vrc6.WriteFrequencyScaling(0);

    // Period 0 => each tick triggers the duty cycle
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Pulse1.WritePeriodLo(0);

    const auto checkMode = [this](const int v) {
        // Check two complete cycles
        for (int cycle = 0; cycle < 2; ++cycle) {
            for (int i = 0; i < 16; ++i) EXPECT_EQ(v, vrc6.Pulse1.Tick());
        }
    };

    // Mode 1, Volume 0
    vrc6.Pulse1.WriteControl(0b10000000);
    checkMode(0);

    // Mode 1, Volume 3
    vrc6.Pulse1.WriteControl(0b10000011);
    checkMode(3);

    // Duty 8/16, Volume max
    vrc6.Pulse1.WriteControl(0b10001111);
    checkMode(15);
}

TEST_F(VRC6AudioTest, Pulse_Period) {
    // Enable, normal scaling
    vrc6.WriteFrequencyScaling(0);

    // Duty 4/16, Volume 1
    vrc6.Pulse1.WriteControl(0b00110001);

    const auto checkPeriod = [this](const int p) {
        // Check two complete cycles
        for (int cycle = 0; cycle < 2; ++cycle) {
            for (int i = 0; i < 12 * p; ++i) EXPECT_EQ(0, vrc6.Pulse1.Tick());
            for (int i = 12 * p; i < 16 * p; ++i) EXPECT_EQ(1, vrc6.Pulse1.Tick());
        }
    };

    // Period 0 => each tick triggers the duty cycle
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Pulse1.WritePeriodLo(0);
    checkPeriod(1);

    // Period 1 => duty cycle is triggered every 2 ticks
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Pulse1.WritePeriodLo(1);
    checkPeriod(2);

    // Period 10 => duty cycle is triggered every 11 ticks
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Pulse1.WritePeriodLo(10);
    checkPeriod(11);

    // Period 0x0248 => duty cycle is triggered every 0x249 ticks
    vrc6.Pulse1.WritePeriodHi(0x02 | CHANNEL_ENABLE);
    vrc6.Pulse1.WritePeriodLo(0x48);
    checkPeriod(0x0249);

    // Period 0x2248 => only first 4 bits of Hi are used => same as Period 0x0248
    vrc6.Pulse1.WritePeriodHi(0x22 | CHANNEL_ENABLE);
    vrc6.Pulse1.WritePeriodLo(0x48);
    checkPeriod(0x0249);
}

TEST_F(VRC6AudioTest, Pulse_Enable) {
    // Enable, normal scaling
    vrc6.WriteFrequencyScaling(0);

    // Period 0 => each tick triggers the duty cycle
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_DISABLE);
    vrc6.Pulse1.WritePeriodLo(0);
    
    // Duty 4/16
    vrc6.Pulse1.WriteControl(0b00110001);

    // Pulse is disable => Volume is 0
    for (int i = 0; i < 16; ++i) EXPECT_EQ(0, vrc6.Pulse1.Tick());
    
    // Phase is reset
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_ENABLE);
    for (int i = 0; i < 12; ++i) EXPECT_EQ(0, vrc6.Pulse1.Tick());
    for (int i = 12; i < 14; ++i) EXPECT_EQ(1, vrc6.Pulse1.Tick());

    // Reset after 8 out of 16 steps
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_DISABLE);
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_ENABLE);
    for (int i = 0; i < 12; ++i) EXPECT_EQ(0, vrc6.Pulse1.Tick());
    for (int i = 12; i < 16; ++i) EXPECT_EQ(1, vrc6.Pulse1.Tick());
}

TEST_F(VRC6AudioTest, Saw_Accumulator) {
    // Enable, normal scaling
    vrc6.WriteFrequencyScaling(0);

    // Period 0 => each other tick triggers accumulator
    vrc6.Saw.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Saw.WritePeriodLo(0);

    // Step 0x08
    vrc6.Saw.WriteControl(0x08);
    for (int cycle = 0; cycle < 2; ++cycle) {
        for (Byte volume = 0; volume < 7; ++volume) {
            EXPECT_EQ(volume, vrc6.Saw.Tick());
            EXPECT_EQ(volume, vrc6.Saw.Tick());
        }
    }

    // Step 0x06
    // Accumulator sequence is 0, 0, 6, 6, 12, 12, 18, 18, 24, 24, 30, 30, 36, 36
    // Output sequence is      0, 0, 0, 0,  1,  1,  2,  2,  3,  3,  3,  3,  4,  4
    vrc6.Saw.WriteControl(0x06);
    for (int cycle = 0; cycle < 2; ++cycle) {
        for (Byte volume : { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4 }) {
            EXPECT_EQ(volume, vrc6.Saw.Tick());
        }
    }

    // Step 50 with overflow
    // Accumulator sequence is 0, 0, 50, 50, 100, 100, 150, 150, 200, 200, 250, 250, 44, 44
    // Output sequence is      0, 0,  6,  6,  12,  12,  18,  18,  25,  25,  31,  31,  5,  5
    vrc6.Saw.WriteControl(50);
    for (int cycle = 0; cycle < 2; ++cycle) {
        for (Byte volume : { 0, 0, 6, 6, 12, 12, 18, 18, 25, 25, 31, 31, 5, 5 }) {
            EXPECT_EQ(volume, vrc6.Saw.Tick());
        }
    }

    // Step over 6 bits
    vrc6.Saw.WriteControl(0x88);
    for (int cycle = 0; cycle < 2; ++cycle) {
        for (Byte volume = 0; volume < 7; ++volume) {
            EXPECT_EQ(volume, vrc6.Saw.Tick());
            EXPECT_EQ(volume, vrc6.Saw.Tick());
        }
    }
}

TEST_F(VRC6AudioTest, Saw_Period) {
    // Enable, normal scaling
    vrc6.WriteFrequencyScaling(0);

    // Step 0x08
    vrc6.Saw.WriteControl(0x08);

    const auto checkPeriod = [this](const int p) {
        // Check two complete cycles
        for (int cycle = 0; cycle < 2; ++cycle) {
            for (Byte volume = 0; volume < 7; ++volume) {
                for (int i = 0; i < 2 * p; ++i) EXPECT_EQ(volume, vrc6.Saw.Tick());
            }
        }
    };

    // Period 0 => each other tick triggers the accumulator
    vrc6.Saw.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Saw.WritePeriodLo(0);
    checkPeriod(1);

    // Period 1 => accumulator is triggered every other 2 ticks
    vrc6.Saw.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Saw.WritePeriodLo(1);
    checkPeriod(2);

    // Period 10 => accumulator is triggered every other 11 ticks
    vrc6.Saw.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Saw.WritePeriodLo(10);
    checkPeriod(11);

    // Period 0x0248 => accumulator is triggered every other 0x249 ticks
    vrc6.Saw.WritePeriodHi(0x02 | CHANNEL_ENABLE);
    vrc6.Saw.WritePeriodLo(0x48);
    checkPeriod(0x0249);

    // Period 0x2248 => only first 4 bits of Hi are used => same as Period 0x0248
    vrc6.Saw.WritePeriodHi(0x22 | CHANNEL_ENABLE);
    vrc6.Saw.WritePeriodLo(0x48);
    checkPeriod(0x0249);
}

TEST_F(VRC6AudioTest, Saw_Enable) {
    // Enable, normal scaling
    vrc6.WriteFrequencyScaling(0);

    // Step 0x08
    vrc6.Saw.WriteControl(0x08);

    const auto checkPeriod = [this](const int p) {
        // Check two complete cycles
        for (int cycle = 0; cycle < 2; ++cycle) {
            for (Byte volume = 0; volume < 7; ++volume) {
                for (int i = 0; i < 2 * p; ++i) EXPECT_EQ(volume, vrc6.Saw.Tick());
            }
        }
    };

    // Period 0 => each other tick triggers the accumulator
    vrc6.Saw.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Saw.WritePeriodLo(0);

    // Check two accumulations (four steps)
    for (Byte volume : { 0, 0, 1, 1 }) EXPECT_EQ(volume, vrc6.Saw.Tick());

    // Disabled => Accumulator forced to 0
    // Looking at the remaining steps of this cycle
    vrc6.Saw.WritePeriodHi(0 | CHANNEL_DISABLE);
    for (int i = 0; i < 16; ++i) EXPECT_EQ(0, vrc6.Saw.Tick());

    // Enabled => Phase is reset
    vrc6.Saw.WritePeriodHi(0 | CHANNEL_ENABLE);
    for (Byte volume : { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6 })
        EXPECT_EQ(volume, vrc6.Saw.Tick());
}

TEST_F(VRC6AudioTest, Saw_Disable_For_Partial_Period) {
    // Enable, normal scaling
    vrc6.WriteFrequencyScaling(0);

    // Step 0x08
    vrc6.Saw.WriteControl(0x08);

    const auto checkPeriod = [this](const int p) {
        // Check two complete cycles
        for (int cycle = 0; cycle < 2; ++cycle) {
            for (Byte volume = 0; volume < 7; ++volume) {
                for (int i = 0; i < 2 * p; ++i) EXPECT_EQ(volume, vrc6.Saw.Tick());
            }
        }
    };

    // Check divider is not stopped during disable
    // Disable for 2.25 steps then Enable
    // Phase is reset, but first step is shortened
    // A full cycle is still 14 steps or 7 accumulations
    // With Period 3, each step is 4 ticks
    vrc6.Saw.WritePeriodHi(0 | CHANNEL_DISABLE);
    vrc6.Saw.WritePeriodLo(3);
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(0, vrc6.Saw.Tick());
    // 2.25 steps were skipped, so the first step will be 3 ticks long
    // The following ones 4 ticks long
    vrc6.Saw.WritePeriodHi(0 | CHANNEL_ENABLE);
    EXPECT_EQ(0, vrc6.Saw.Tick());
    EXPECT_EQ(0, vrc6.Saw.Tick());
    EXPECT_EQ(0, vrc6.Saw.Tick());
    for (Byte volume : { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6 }) {
        EXPECT_EQ(volume, vrc6.Saw.Tick());
        EXPECT_EQ(volume, vrc6.Saw.Tick());
        EXPECT_EQ(volume, vrc6.Saw.Tick());
        EXPECT_EQ(volume, vrc6.Saw.Tick());
    }
    // The next cycle is complete
    for (Byte volume : { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6 }) {
        EXPECT_EQ(volume, vrc6.Saw.Tick());
        EXPECT_EQ(volume, vrc6.Saw.Tick());
        EXPECT_EQ(volume, vrc6.Saw.Tick());
        EXPECT_EQ(volume, vrc6.Saw.Tick());
    }
}

TEST_F(VRC6AudioTest, Frequency_Scaling) {
    // Pulse 1 enable, normal scaling, duty 1/16, volume 15 (max), period 0
    // Pulse 2 disable, Saw disable
    vrc6.Pulse1.WriteControl(0b00111111);
    vrc6.Pulse1.WritePeriodLo(0);
    vrc6.Pulse1.WritePeriodHi(0 | CHANNEL_ENABLE);
    vrc6.Pulse2.WritePeriodHi(CHANNEL_DISABLE);
    vrc6.Saw.WritePeriodHi(CHANNEL_DISABLE);

    // We will compensate the frequency shift with the channel's period
    // And ignore the actual output mix by only check ==0 or !=0
    const auto checkActive = [this]() {
        for (int i = 0; i < 12; ++i) EXPECT_EQ(0, vrc6.Tick());
        for (int i = 12; i < 16; ++i) EXPECT_NE(0, vrc6.Tick());
    };

    const auto checkHalted = [this]() {
        for (int i = 0; i < 16; ++i) EXPECT_EQ(0, vrc6.Tick());
    };

    // Normal, period 0
    vrc6.WriteFrequencyScaling(0b000);
    vrc6.Pulse1.WritePeriodLo(0);
    checkActive();

    // x16, compensate with period 15
    vrc6.WriteFrequencyScaling(0b010);
    vrc6.Pulse1.WritePeriodLo(15);
    checkActive();

    // x256, compensate with period 255
    vrc6.WriteFrequencyScaling(0b100);
    vrc6.Pulse1.WritePeriodLo(255);
    checkActive();

    // x256 takes precedence over x16
    vrc6.WriteFrequencyScaling(0b110);
    vrc6.Pulse1.WritePeriodLo(255);
    checkActive();

    // Halted
    vrc6.WriteFrequencyScaling(0b001);
    vrc6.Pulse1.WritePeriodLo(0);
    checkHalted();

    // Halted takes precedence
    vrc6.WriteFrequencyScaling(0b111);
    vrc6.Pulse1.WritePeriodLo(255);
    checkHalted();
}


TEST_F(VRC6AudioTest, Output_Mix) {
    vrc6.WriteFrequencyScaling(0);
    
    // Pulse 1 enable, duty 4/16, volume 15, period 0
    vrc6.Pulse1.WriteControl(0b00111111);
    vrc6.Pulse1.WritePeriodLo(0);
    vrc6.Pulse1.WritePeriodHi(CHANNEL_ENABLE);

    // Pulse 2 enable, duty 8/16, volume 15, period 0
    vrc6.Pulse2.WriteControl(0b01111111);
    vrc6.Pulse2.WritePeriodLo(0);
    vrc6.Pulse2.WritePeriodHi(CHANNEL_ENABLE);

    // Saw 1 enable, rate 0x20, period 0
    vrc6.Saw.WriteControl(0x20);
    vrc6.Saw.WritePeriodLo(0);
    vrc6.Saw.WritePeriodHi(CHANNEL_ENABLE);

    // Pulse 1 sequence     0   0   0   0   0   0   0   0   0   0   0   0   15  15  15  15
    // Pulse 2 sequence     0   0   0   0   0   0   0   0   15  15  15  15  15  15  15  15
    // Saw sequence         0   0   4   4   8   8   12  12  16  16  20  20  24  24  0   0
    // Expected output      0   0   4   4   8   8   12  12  31  31  35  35  54  54  30  30
    for (auto volume : { 0, 0, 4, 4, 8, 8, 12, 12, 31, 31, 35, 35, 54, 54, 30, 30 }) {
        EXPECT_EQ(volume, vrc6.Tick());
    }
}
