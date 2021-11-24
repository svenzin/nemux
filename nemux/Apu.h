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

////////////////////////////////////////////////////////////////////////////////
// Clock divider

struct Divider {
    int Period;
    int Counter;

    void SetPeriod(const int p) {
        Period = p;
    }
    
    void Reset() {
        Counter = Period;
    }
    
    bool Tick() {
        --Counter;
        if (Counter <= 0) {
            Counter = Period;
            return true;
        }
        return false;
    }
};

////////////////////////////////////////////////////////////////////////////////

struct FrameCounter {
    struct Clock {
        bool HalfFrame;
        bool QuarterFrame;
    };

    int Ticks = 0;
    int Mode = 0;
    bool HideInterrupt = false;
    bool Interrupt = false;

    Clock Tick() {
        static constexpr int SetInterruptTick = 29828;
        static constexpr int ResetInterruptTick = 1;
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

        if (Mode == 1 || HideInterrupt) {
            Interrupt = false;
        }
        else {
            if (Ticks == SetInterruptTick) Interrupt = true;
            if (Ticks == ResetInterruptTick) Interrupt = false;
        }
        
        Ticks = ((Ticks + 1) % Periods[Mode]);

        return result;
    }

    void WriteControl(const Byte value) {
        Mode = Bit<7>(value);
        HideInterrupt = IsBitSet<6>(value);
    }
};

struct EnvelopeGenerator {
    bool Restart = false;
    bool Loop = false;
    bool Enabled = true;
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

struct LengthCounter {
    int Count = 0x0A;   // Lengths[0]
    bool Halt = false;

    void SetCountIndex(const int value) {
        static constexpr Byte Lengths[0x20] = {
            0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
            0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
            0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
            0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
        };
        Count = Lengths[value];
    }

    Byte Tick(const bool halfFrame) {
        if (Count == 0) return 0;
        if (halfFrame && !Halt) --Count;
        return 1;
    }

    void Clear() {
        Count = 0;
    }
};

struct Sequencer {
    int Duty = 0;
    int Phase = 0;

    Byte Tick(const bool timer) {
        static constexpr Byte Sequences[4][8] = {
            { 0, 1, 0, 0, 0, 0, 0, 0 },
            { 0, 1, 1, 0, 0, 0, 0, 0 },
            { 0, 1, 1, 1, 1, 0, 0, 0 },
            { 1, 0, 0, 1, 1, 1, 1, 1 }
        };
        const auto value = Sequences[Duty][Phase];
        if (timer) Phase = (Phase + 1) % 8;
        return value;
    }
};

struct TriangleSequencer {
    int Phase = 0;

    Byte Tick(const bool timer) {
        static constexpr Byte Sequence[32] = {
            0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8,
            0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
            0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
            0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
        };
        const auto value = Sequence[Phase];
        if (timer)
            Phase = (Phase + 1) % 32;
        return value;
    }
};

struct LinearCounter {
    int Count = 0;
    int ReloadValue = 0;
    bool Reload = false;
    bool Control = false;

    int Tick(const bool quarterFrame) {
        const int value = ((Count > 0) ? 1 : 0);
        if (quarterFrame) {
            if (Reload) {
                Count = ReloadValue;
            }
            else {
                if (Count > 0) --Count;
            }
            if (!Control) Reload = false;
        }
        return value;
    }
};

struct FlipFlop {
    bool State = false;
    operator bool() {
        State = !State;
        return State;
    }
};

struct Timer {
    Word Period = 1;
    Word T = 0;

    bool Tick() {
        if (T == 0) {
            T = Period - 1;
            return true;
        }
        --T;
        return false;
    }

    void SetPeriodLow(const Byte lo) {
        auto t = Period - 1;
        t = (t & WORD_HI_MASK) | lo;
        Period = t + 1;
    }
    
    void SetPeriodHigh(const Byte hi) {
        auto t = Period - 1;
        t = (t & WORD_LO_MASK) | (hi << BYTE_WIDTH);
        Period = t + 1;
    }
};

struct Pulse {
    explicit Pulse() { //T.Period = 0; }
        WriteControl(0);
        WriteSweep(0);
        WritePeriodLow(0);
        WritePeriodHigh(0);
    }
    
    EnvelopeGenerator Envelope;
    LengthCounter Length;
    Sequencer Sequence;
    Timer T;
    FlipFlop flipflop;

    void WriteControl(const Byte value) {
        Sequence.Duty = value >> 6;

        Length.Halt = IsBitSet<5>(value);

        Envelope.Enabled = IsBitClear<4>(value);
        Envelope.Loop = IsBitSet<5>(value);
        Envelope.Volume = (value & 0x0F);
    }

    void WritePeriodLow(const Byte value) {
        T.SetPeriodLow(value);
    }

    void WritePeriodHigh(const Byte value) {
        T.SetPeriodHigh(value & 0x07);
        
        Sequence.Phase = 0;

        Length.SetCountIndex(value >> 3);

        Envelope.Restart = true;
    }

    bool Enabled = false;
    void Enable(bool enabled) {
        Enabled = enabled;
        if (!Enabled) Length.Clear();
    }

    bool SweepEnabled = false;
    Byte SweepPeriod = 1;
    Byte SweepT = 0;
    bool SweepNegate = false;
    bool SweepAlternativeNegate = false;
    Byte SweepAmount = 0;
    int SweepTargetPeriod = 0;
    bool SweepReload = false;
    void WriteSweep(const Byte value) {
        SweepEnabled = IsBitSet<7>(value);
        SweepPeriod = ((value >> 4) & 0x07) + 1;
        SweepNegate = IsBitSet<3>(value);
        SweepAmount = value & 0x07;
        SweepReload = true;
    }

    Byte TickSweep(const bool halfFrame) {
        const auto period = T.Period;
        auto result = 1;
        if (SweepNegate) {
            SweepTargetPeriod = period - (period >> SweepAmount);
            if (SweepAlternativeNegate) {
                SweepTargetPeriod -= 1;
            }
        }
        else {
            SweepTargetPeriod = period + (period >> SweepAmount);
        }
        if (SweepTargetPeriod >= 0x0800) result = 0;
        if (period < 9) result = 0;

        if (halfFrame) {
            if ((SweepT == 0) && SweepEnabled && (result == 1) && (SweepAmount > 0)) {
                T.Period = SweepTargetPeriod;
            }

            if ((SweepT == 0) || SweepReload) {
                SweepT = SweepPeriod - 1;
                SweepReload = false;
            }
            else {
                --SweepT;
            }
        }
        return result;
    }

    Byte Tick(const FrameCounter::Clock clock) {
        if (!Enabled) return 0;

        const auto volume = Envelope.Tick(clock.QuarterFrame);
        const auto sweep = TickSweep(clock.HalfFrame);
        const auto timer = (flipflop) ? T.Tick() : false;
        const auto sequence = Sequence.Tick(timer);
        const auto length = Length.Tick(clock.HalfFrame);
        return volume * sequence * length * sweep;
    }
};

struct Triangle {
    explicit Triangle() { T.Period = 0; }

    bool Enabled = false;
    TriangleSequencer Sequence;
    LengthCounter Length;
    LinearCounter Counter;
    Timer T;

    void WriteControl(const Byte value) {
        Counter.Control = IsBitSet<7>(value);
        Counter.ReloadValue = (value & 0x7F);
        Counter.Reload = true;
        
        Length.Halt = IsBitSet<7>(value);
    }

    void WritePeriodLow(const Byte value) {
        T.SetPeriodLow(value);
    }

    void WritePeriodHigh(const Byte value) {
        T.SetPeriodHigh(value & 0x07);

        Length.SetCountIndex(value >> 3);

        Counter.Reload = true;
    }

    Byte Tick(const FrameCounter::Clock clock) {
        if (!Enabled) return Sequence.Tick(false);

        const bool mute = (T.Period <= 2);
        
        const bool timer = T.Tick();
        const auto counter = Counter.Tick(clock.QuarterFrame);
        const auto length = Length.Tick(clock.HalfFrame);
        const auto valid = timer && (counter * length > 0);
        const auto volume = Sequence.Tick(valid && !mute);
        return volume;
    }
};

struct ShiftRegister {
    bool Mode = false;
    int Value = 1;
    Byte Tick(bool timer) {
        const Byte bit0 = Bit<0>(Value);
        const Byte bitN = (Mode ? Bit<6>(Value) : Bit<1>(Value));
        const int mask = (bit0 ^ bitN) << 13;
        Value = (Value >> 1) | mask;
        return bit0;
    }
};

struct Noise {
    explicit Noise() { T.Period = 4; WriteLength(0); } // Periods[0]
    
    bool Enabled = false;
    Timer T;
    EnvelopeGenerator Envelope;
    LengthCounter Length;
    FlipFlop flipflop;
    ShiftRegister Shifter;

    void WriteControl(const Byte value) {
        Length.Halt = IsBitSet<5>(value);

        Envelope.Enabled = IsBitClear<4>(value);
        Envelope.Loop = IsBitSet<5>(value);
        Envelope.Volume = (value & 0x0F);
    }

    void WritePeriod(const Byte value) {
        static constexpr Word Periods[0x10] = {
            0x0004, 0x0008, 0x0010, 0x0020,
            0x0040, 0x0060, 0x0080, 0x00A0,
            0x00CA, 0x00FE, 0x017C, 0x01FC,
            0x02FA, 0x03F8, 0x07F2, 0x0FE4
        };
        T.Period = Periods[value & 0x0F];

        Shifter.Mode = IsBitSet<7>(value);
    }

    void WriteLength(const Byte value) {
        Length.SetCountIndex(value >> 3);

        Envelope.Restart = true;
    }

    Byte Tick(const FrameCounter::Clock clock) {
        if (!Enabled) return 0;

        const auto volume = Envelope.Tick(clock.QuarterFrame);
        const auto timer = (flipflop) ? T.Tick() : false;
        const auto shifter = Shifter.Tick(timer);
        const auto length = Length.Tick(clock.HalfFrame);
        return volume * length * (1 - shifter);
    }
};

struct SampleBuffer {
    bool IsEmpty;
    Byte Sample;
};

template <class Cpu_t>
struct DMAReader {
    Cpu_t * CPU;

    Word SampleAddress = 0xC000;
    Word SampleLength = 1;

    Word Address;
    Word Length = 0;

    bool LoopSample = false;
    bool InterruptEnabled = false;

    bool Interrupt = false;

    SampleBuffer GetSample() {
        if (Length == 0) return{ true, 0 }; 
        
        const auto sample = CPU->ReadByteAt(Address);
        CPU->Ticks += 4;

        if (Address == 0xFFFF) Address = 0x8000;
        else ++Address;
        
        --Length;
        if (Length == 0) {
            if (LoopSample) {
                Start();
            }
            else {
                Interrupt = InterruptEnabled;
            }
        }

        return{ false, sample };
    }

    void Start() {
        Address = SampleAddress;
        Length = SampleLength;
    }
};

template <class Cpu_t>
struct DMCOutput {
    Byte Value = 0;

    DMAReader<Cpu_t> DMA;

    int BitsRemaining = 0;
    bool Silent = true;
    Byte Sample;

    void Start() {
        BitsRemaining = 8;
        const auto buffer = DMA.GetSample();
        Sample = buffer.Sample;
        Silent = buffer.IsEmpty;
    }

    Byte Tick(const bool timer) {
        if (timer) {
            if (!Silent) {
                if (IsBitSet<0>(Sample)) {
                    if (Value <= 125) Value += 2;
                }
                else {
                    if (Value >= 2) Value -= 2;
                }
                Sample = (Sample >> 1);
            }

            --BitsRemaining;
            if (BitsRemaining <= 0) Start();
        }
        
        return Value;
    }
};

template <class Cpu_t>
struct DMC {
    explicit DMC() { T.Period = 0x1AC; } // Periods[0]
    
    DMCOutput<Cpu_t> Output;
    Timer T;

    void WriteDAC(const Byte value) {
        Output.Value = value & 0x7F;
    }

    void WriteFrequency(const Byte value) {
        static constexpr Word Periods[0x10] = {
            0x01AC, 0x017C, 0x0154, 0x0140,
            0x011E, 0x00FE, 0x00E2, 0x00D6,
            0x00BE, 0x00A0, 0x008E, 0x0080,
            0x006A, 0x0054, 0x0048, 0x0036
        };
        T.Period = Periods[value & 0x0F];

        Output.DMA.LoopSample = IsBitSet<6>(value);
        Output.DMA.InterruptEnabled = IsBitSet<7>(value);
        Output.DMA.Interrupt = (Output.DMA.Interrupt && Output.DMA.InterruptEnabled);
    }

    void WriteAddress(const Byte value) {
        Output.DMA.SampleAddress = 0xC000 | (value << 6);
    }
    
    void WriteLength(const Byte value) {
        Output.DMA.SampleLength = 0x0001 | (value << 4);
    }

    Byte Tick(const FrameCounter::Clock clock) {
        const bool timer = T.Tick();
        const auto output = Output.Tick(timer);
        return output;
    }
};

template <class Cpu_t>
class Apu {
public:
    explicit Apu() {
        WriteCommonEnable(0);
        WriteCommonControl(0);
        
        Pulse1.SweepAlternativeNegate = true;
        WritePulse1Control(0);
        WritePulse1Sweep(0);
        WritePulse1PeriodLo(0);
        WritePulse1PeriodHi(0);

        WritePulse2Control(0);
        WritePulse2Sweep(0);
        WritePulse2PeriodLo(0);
        WritePulse2PeriodHi(0);

        WriteTriangleControl(0);
        WriteTrianglePeriodLo(0);
        WriteTrianglePeriodHi(0);

        WriteNoiseControl(0);
        WriteNoisePeriod(0);
        WriteNoiseLength(0);

        WriteDMCFrequency(0);
        WriteDMCDAC(0);
        WriteDMCAddress(0);
        WriteDMCLength(0);
    }

    void WritePulse1Control(const Byte value) { Pulse1.WriteControl(value); }
    void WritePulse1Sweep(const Byte value) { Pulse1.WriteSweep(value); }
    void WritePulse1PeriodLo(const Byte value) { Pulse1.WritePeriodLow(value); }
    void WritePulse1PeriodHi(const Byte value) { Pulse1.WritePeriodHigh(value); }
    
    void WritePulse2Control(const Byte value) { Pulse2.WriteControl(value); }
    void WritePulse2Sweep(const Byte value) { Pulse2.WriteSweep(value); }
    void WritePulse2PeriodLo(const Byte value) { Pulse2.WritePeriodLow(value); }
    void WritePulse2PeriodHi(const Byte value) { Pulse2.WritePeriodHigh(value); }

    void WriteTriangleControl(const Byte value) { Triangle1.WriteControl(value); }
    void WriteTrianglePeriodLo(const Byte value) { Triangle1.WritePeriodLow(value); }
    void WriteTrianglePeriodHi(const Byte value) { Triangle1.WritePeriodHigh(value); }

    void WriteNoiseControl(const Byte value) { Noise1.WriteControl(value); }
    void WriteNoisePeriod(const Byte value) { Noise1.WritePeriod(value); }
    void WriteNoiseLength(const Byte value) { Noise1.WriteLength(value); }

    void WriteDMCFrequency(const Byte value) { DMC1.WriteFrequency(value); }
    void WriteDMCDAC(const Byte value) { DMC1.WriteDAC(value); }
    void WriteDMCAddress(const Byte value) { DMC1.WriteAddress(value); }
    void WriteDMCLength(const Byte value) { DMC1.WriteLength(value); }
    
    void WriteCommonEnable(const Byte value) {
        Pulse1.Enable(IsBitSet<0>(value));
        Pulse2.Enable(IsBitSet<1>(value));
        Triangle1.Enabled = IsBitSet<2>(value);
        Noise1.Enabled = IsBitSet<3>(value);
        
        const bool dmcEnabled = IsBitSet<4>(value);
        if (dmcEnabled) {
            if (DMC1.Output.DMA.Length == 0) DMC1.Output.DMA.Start();
        }
        else {
            DMC1.Output.DMA.Length = 0;
        }
        DMC1.Output.DMA.Interrupt = false;
    }
    void WriteCommonControl(const Byte value) {
        Frame.WriteControl(value);
    }

    Byte ReadStatus() {
        const auto FrameInterrupt = Frame.Interrupt;
        Frame.Interrupt = false;
        return Mask<0>(Pulse1.Length.Count > 0)
            + Mask<1>(Pulse2.Length.Count > 0)
            + Mask<2>(Triangle1.Length.Count > 0)
            + Mask<3>(Noise1.Length.Count > 0)
            + Mask<4>(DMC1.Output.DMA.Length > 0)
            + Mask<6>(FrameInterrupt)
            + Mask<7>(DMC1.Output.DMA.Interrupt);
    }

    float Tick() {
        const auto clock = Frame.Tick();

        const auto square1 = Pulse1Output * Pulse1.Tick(clock);
        const auto square2 = Pulse2Output * Pulse2.Tick(clock);

        const auto squareOut = 95.88f / (100.0f + 8128.0f / (square1 + square2));

        const auto triangle = Triangle1Output * Triangle1.Tick(clock);
        const auto noise = Noise1Output * Noise1.Tick(clock);
        const auto dmc = DMC1Output * DMC1.Tick(clock);

        const auto tndOut = 159.79f / (100.0f + 1.0f / (triangle / 8227.0f + noise / 12241.0f + dmc / 22638.0f));

        return squareOut + tndOut;
    }

    FrameCounter Frame;

    int Pulse1Output = 1;
    Pulse Pulse1;

    int Pulse2Output = 1;
    Pulse Pulse2;

    int Triangle1Output = 1;
    Triangle Triangle1;

    int Noise1Output = 1;
    Noise Noise1;

    int DMC1Output = 1;
    DMC<Cpu_t> DMC1;
};

#endif /* APU_H_ */
