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

struct FrameCounter {
    struct Clock {
        bool HalfFrame;
        bool QuarterFrame;
    };

    int Ticks = 0;
    Clock Tick() {
        static constexpr int QuarterTicks[2][4] = {
            { 7457, 14913, 22371, 29829 },
            { 7457, 14913, 22371, 37281 }
        };
        static constexpr int HalfTicks[2][2] = {
            { 14913, 29829 },
            { 14913, 37281 }
        };
        static constexpr int Periods[2] = { 29830, 37282 };
        Clock result;
        result.HalfFrame =
            (Ticks == HalfTicks[Mode][0])
            || (Ticks == HalfTicks[Mode][1]);
        result.QuarterFrame =
            (Ticks == QuarterTicks[Mode][0])
            || (Ticks == QuarterTicks[Mode][1])
            || (Ticks == QuarterTicks[Mode][2])
            || (Ticks == QuarterTicks[Mode][3]);
        Ticks = (Ticks + 1) % Periods[Mode];
        return result;
    }

    int Mode = 0;
    void WriteControl(const Byte value) {
        Mode = Bit<7>(value);
    }
};

struct EnvelopeGenerator {
    bool Restart = false;
    bool Loop = false;
    bool Enabled = false;
    int Value = 0;
    int Divider = 1;
    int Volume = 0;
    
    Byte Tick(const bool quarterFrame) {
        if (quarterFrame) {
            --Divider;
            if (Divider == 0) {
                Divider = Volume + 1;
                if (Restart) {
                    Restart = false;
                    Value = 0xF;
                }
                else {
                    if (Value > 0) --Value;
                    else if (Loop) Value = 0x0F;
                }
            }
        }
        return (Enabled ? Value : Volume);
    }
};

struct Pulse {
    FrameCounter Frame;
    EnvelopeGenerator Envelope;

    Word Period = 1;
    Word T = 0;

    Byte Length;

    Byte Volume = 0;
    bool Halt = false;
    void WriteControl(const Byte value) {
        Duty = value >> 6;
        Volume = value & 0x0F;
        Halt = IsBitSet<5>(value);

        Envelope.Enabled = IsBitClear<4>(value);
        Envelope.Loop = IsBitSet<5>(value);
        Envelope.Volume = Volume;
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

        static constexpr Byte Lengths[0x20] = {
            0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
            0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
            0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
            0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
        };
        Length = Lengths[value >> 3];
        LengthCounter = (Length == 0) ? 0 : 1;

        Envelope.Restart = true;
    }

    bool Enabled = false;
    void Enable(bool enabled) {
        Enabled = enabled;
        if (!Enabled) Length = 0;
    }

    bool SweepEnabled = false;
    Byte SweepPeriod = 0;
    Byte SweepT = 0;
    bool SweepNegate = false;
    bool SweepAlternativeNegate = false;
    Byte SweepAmount = 0;
    int SweepTargetPeriod = 0;
    bool SweepReload = false;
    void WriteSweep(const Byte value) {
        SweepEnabled = IsBitSet<7>(value);
        SweepPeriod = (value >> 4) & 0x07;
        SweepNegate = IsBitSet<3>(value);
        SweepAmount = value & 0x07;
        SweepReload = true;
    }

    Byte TickSweep(const bool halfFrame) {
        auto result = 1;
        if (SweepNegate) {
            SweepTargetPeriod = Period - (Period >> SweepAmount);
            if (SweepAlternativeNegate) {
                SweepTargetPeriod -= 1;
            }
        }
        else {
            SweepTargetPeriod = Period + (Period >> SweepAmount);
        }
        if (SweepTargetPeriod >= 0x0800) result = 0;
        if (Period < 8) result = 0;

        if (halfFrame) {
            if ((SweepT == 0) && SweepEnabled && (result == 1) && (SweepAmount > 0)) {
                Period = SweepTargetPeriod;
            }

            if ((SweepT == 0) || SweepReload) {
                SweepT = SweepPeriod;
                SweepReload = false;
            }
            else {
                --SweepT;
            }
        }
        return result;
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

    Byte TickLength() {
        if (Length == 0) return 0;
        if (!Halt) --Length;
        return 1;
    }

    Byte LengthCounter = 0;
    Byte Sequence = 0;
    Byte Tick() {
        if (!Enabled) return 0;
        if (Period < 9) return 0;

        const auto clock = Frame.Tick();
        const auto volume = Envelope.Tick(clock.QuarterFrame);
        const auto sweep = TickSweep(clock.HalfFrame);
        if (TickTimer()) Sequence = TickSequence();
        if (clock.HalfFrame) LengthCounter = TickLength();
        return volume * Sequence * LengthCounter * sweep;
    }
};

class Apu {
public:
    explicit Apu() {
        Pulse1.SweepAlternativeNegate = true;
    }

    void WritePulse1Control(const Byte value) { Pulse1.WriteControl(value); }
    void WritePulse1Sweep(const Byte value) { Pulse1.WriteSweep(value); }
    void WritePulse1PeriodLo(const Byte value) { Pulse1.WritePeriodLow(value); }
    void WritePulse1PeriodHi(const Byte value) { Pulse1.WritePeriodHigh(value); }
    
    void WritePulse2Control(const Byte value) { Pulse2.WriteControl(value); }
    void WritePulse2Sweep(const Byte value) { Pulse2.WriteSweep(value); }
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
    void WriteCommonControl(const Byte value) {
        Pulse1.Frame.WriteControl(value);
        Pulse2.Frame.WriteControl(value);
    }

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
