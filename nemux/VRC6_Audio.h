#ifndef VRC6_AUDIO_H_
#define VRC6_AUDIO_H_

#include "Types.h"
#include "BitUtil.h"

struct VRC6_Audio {

    Byte Tick() {
        if (Halted) return 0;
        // Note we start at 1 to keep the value returned by the last tick
        for (int i = 1; i < Scale; ++i) {
            Pulse1.Tick();
            Pulse2.Tick();
            Saw.Tick();
        }
        const Byte pulse1 = Pulse1.Tick();
        const Byte pulse2 = Pulse2.Tick();
        const Byte saw = Saw.Tick();
        return pulse1 + pulse2 + saw;
    }
    
    bool Halted = false;
    int Scale = 1;
    void WriteFrequencyScaling(const Byte value) {
        Halted = IsBitSet<0>(value);
        Scale = 1;
        if (IsBitSet<1>(value)) Scale = 16;
        if (IsBitSet<2>(value)) Scale = 256;
    }
    
    struct PulseChannel {
        Byte Volume = 0;
        bool IgnoreDuty = false;
        bool Enabled = false;
        int Duty = 0;
        int Phase = 0;
        Word Period = 0;
        Word T = 0;
        
        void WriteControl(const Byte value) {
            Volume = value & 0xF;
            Duty = (value >> 4) & 0x7;
            IgnoreDuty = IsBitSet<7>(value);
        }
        
        void WritePeriodLo(const Byte value) {
            Period = (Period & WORD_HI_MASK) | value;
        }
        
        void WritePeriodHi(const Byte value) {
            Enabled = IsBitSet<7>(value);
            if (!Enabled) Phase = 0;
            const Byte hi = value & 0x0F;
            Period = (hi << BYTE_WIDTH) | (Period & WORD_LO_MASK);
        }
        
        Byte Tick() {
            Byte value = 0;
            if (Enabled) {
                if (Phase >= 15 - Duty) value = Volume;
                if (IgnoreDuty) value = Volume;
            }
            if (T == Period) {
                T = 0;
                Phase = (Phase + 1) % 16;
            } else {
                ++T;
            }
            return value;
        }
    };

    PulseChannel Pulse1;
    PulseChannel Pulse2;
    
    struct SawChannel {
        Byte Accumulator = 0;
        Byte Rate = 0;
        Byte Step = 0;
        Word Period = 0;
        Word T = 0;
        bool Enabled = false;

        void WriteControl(const Byte value) {
            Rate = value & 0x3F;
        }

        void WritePeriodLo(const Byte value) {
            Period = (Period & WORD_HI_MASK) | value;
        }

        void WritePeriodHi(const Byte value) {
            Enabled = IsBitSet<7>(value);
            if (!Enabled) {
                Step = 0;
                Accumulator = 0;
            }
            const Byte hi = value & 0x0F;
            Period = (hi << BYTE_WIDTH) | (Period & WORD_LO_MASK);
        }

        Byte Tick() {
            if (Enabled) {
                if (T == 0) {
                    if (Step == 0) {
                        Accumulator = 0;
                    } else {
                        if (Step % 2 == 0) {
                            Accumulator += Rate;
                        }
                    }
                    if (Step == 13) {
                        Step = 0;
                    } else {
                        ++Step;
                    }
                }
            }
            if (T == Period) {
                T = 0;
            } else {
                ++T;
            }
            return Accumulator >> 3;
        }
    };
    
    SawChannel Saw;
};

#endif /* VRC6_AUDIO_H_ */
