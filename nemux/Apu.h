#ifndef APU_H_
#define APU_H_

#include "Types.h"
#include "BitUtil.h"
//#include "Palette.h"
//#include "MemoryMap.h"
//
//#include <array>
//#include <iostream>
//#include <iomanip>
//
//static constexpr size_t FRAME_WIDTH = 256;
//static constexpr size_t FRAME_HEIGHT = 240;
//static constexpr size_t VIDEO_WIDTH = 341;
//static constexpr size_t VIDEO_HEIGHT = 262;
//static constexpr size_t VIDEO_SIZE = VIDEO_WIDTH * VIDEO_HEIGHT;

struct Pulse {
    Word Period = 1;
    Word T = 0;

    Byte Volume = 0;
    void WriteControl(const Byte value) {
        Duty = value >> 6;
        Volume = value & 0x0F;
    }

    void WritePeriodLow(const Byte value) {
        auto t = Period - 1;
        t = (t & WORD_HI_MASK) | value;
        Period = t + 1;
    }

    void WritePeriodHigh(const Byte value) {
        auto t = Period - 1;
        t = (t & WORD_LO_MASK) | ((value & 0x07) << BYTE_WIDTH);
        Period = t + 1;
        Phase = 0;
    }

    bool Enabled = false;
    void Enable(bool enabled) {
        Enabled = enabled;
    }

    bool FlipFlop = false;
    bool TickTimer() {
        FlipFlop = !FlipFlop;
        if (FlipFlop) {
            if (T == 0) {
                T = Period - 1;
                return true;
            }
            --T;
        }
        return false;
    }

    int Duty = 0;
    int Phase = 0;
    Byte TickSequence() {
        static constexpr Byte Sequences[4][8] = {
            { 0, 1, 0, 0, 0, 0, 0, 0 },
            { 0, 1, 1, 0, 0, 0, 0, 0 },
            { 0, 1, 1, 1, 1, 0, 0, 0 },
            { 1, 0, 0, 1, 1, 1, 1, 1 }
        };
        const auto value = Sequences[Duty][Phase];
        Phase = (Phase + 1) % 8;
        return value;
    }

    Byte Sequence = 0;
    Byte Tick() {
        if (!Enabled) return 0;
        if (Period < 9) return 0;
        Byte v = Volume;
        if (TickTimer()) Sequence = TickSequence();
        return v * Sequence;
    }
};

class Apu {
public:
    void WritePulse1Control(const Byte value) { Pulse1.WriteControl(value); }
    void WritePulse1Sweep(const Byte value) {}
    void WritePulse1PeriodLo(const Byte value) { Pulse1.WritePeriodLow(value); }
    void WritePulse1PeriodHi(const Byte value) { Pulse1.WritePeriodHigh(value); }
    
    void WritePulse2Control(const Byte value) { Pulse2.WriteControl(value); }
    void WritePulse2Sweep(const Byte value) {}
    void WritePulse2PeriodLo(const Byte value) { Pulse2.WritePeriodLow(value); }
    void WritePulse2PeriodHi(const Byte value) { Pulse2.WritePeriodHigh(value); }

    void WriteTriangleControl(const Byte value) {}
    void WriteTrianglePeriodLo(const Byte value) {}
    void WriteTrianglePeriodHi(const Byte value) {}

    void WriteNoiseControl(const Byte value) {}
    void WriteNoisePeriod(const Byte value) {}
    void WriteNoiseLength(const Byte value) {}

    void WriteDMCFrequency(const Byte value) {}
    void WriteDMCDAC(const Byte value) {}
    void WriteDMCAddress(const Byte value) {}
    void WriteDMCLength(const Byte value) {}
    
    void WriteCommonEnable(const Byte value) {
        Pulse1.Enable(IsBitSet<0>(value));
        Pulse2.Enable(IsBitSet<1>(value));
    }
    void WriteCommonControl(const Byte value) {}

    Byte ReadStatus() const { return 0; }

    float Tick() {
        const auto square1 = Pulse1.Tick();
        const auto square2 = Pulse2.Tick();

        const auto squareOut = 95.88f / (100.0f + 8128.0f / (square1 + square2));
        
        return squareOut;
    }

    Pulse Pulse1;
    Pulse Pulse2;
};

#endif /* APU_H_ */
