#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <memory>
#include <iterator>

#include "SDL.h"

#include "NesFile.h"
#include "Mapper_0.h"
#include "Mapper_1.h"
#include "Mapper_2.h"
#include "Mapper_3.h"
#include "NsfFile.h"
#include "Mapper_Nsf.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Controllers.h"

using std::boolalpha;
using std::hex;
using std::dec;
using std::setfill;
using std::setw;
using std::endl;

#undef main

class Machine {
public:
    Controllers ctrl;
    std::unique_ptr<NesMapper> mapper;
    PpuMemoryMap<Palette> ppumap;
    Ppu ppu;
    Apu<Cpu> apu;
    CpuMemoryMap<Cpu, Ppu, Controllers, Apu<Cpu>> cpumap;
    Cpu cpu;

    explicit Machine(std::unique_ptr<NesMapper> & cartridge)
        : mapper(cartridge.release()),
        ppumap(nullptr, mapper.get()),
        ppu(&ppumap),
        cpumap(nullptr, &apu, &ppu, mapper.get(), &ctrl),
        cpu("6502", &cpumap) {
        cpumap.CPU = &cpu;
        apu.DMC1.Output.DMA.CPU = &cpu;
        cpu.PowerUp();
    }

    void Reset() { cpu.Reset(); }

    std::pair<bool, float> Step() {
        const auto frame = ppu.FrameCount;
        cpu.Tick();
        ppu.Tick();
        ppu.Tick();
        ppu.Tick();
        const auto cpuSample = apu.Tick();
        const auto sample = mapper->Tick(cpuSample);
        return{ ppu.FrameCount != frame, sample };
    }

    void StepOneCpuInstruction() {
        do {
            cpu.Tick();
            ppu.Tick();
            ppu.Tick();
            ppu.Tick();
            apu.Tick();
        } while (cpu.CurrentTick < cpu.Ticks);
    }
};

namespace replay {
    enum Commands : char {
        FrameStart = 0x0F,
        Player_1 = 0x01,
        CheckFrame = 0x81,
        FrameEnd = 0x00,
        Reset = 0x40,
    };
    Byte GetP1State(const Machine & nes) {
        return Mask<0>(nes.ctrl.P1_A)
             | Mask<1>(nes.ctrl.P1_B)
             | Mask<2>(nes.ctrl.P1_Select)
             | Mask<3>(nes.ctrl.P1_Start)
             | Mask<4>(nes.ctrl.P1_Up)
             | Mask<5>(nes.ctrl.P1_Down)
             | Mask<6>(nes.ctrl.P1_Left)
             | Mask<7>(nes.ctrl.P1_Right);
    }
    void SetP1State(Machine & nes, Byte p1) {
        nes.ctrl.P1_A      = IsBitSet<0>(p1);
        nes.ctrl.P1_B      = IsBitSet<1>(p1);
        nes.ctrl.P1_Select = IsBitSet<2>(p1);
        nes.ctrl.P1_Start  = IsBitSet<3>(p1);
        nes.ctrl.P1_Up     = IsBitSet<4>(p1);
        nes.ctrl.P1_Down   = IsBitSet<5>(p1);
        nes.ctrl.P1_Left   = IsBitSet<6>(p1);
        nes.ctrl.P1_Right  = IsBitSet<7>(p1);
    }
}
namespace debug {
    std::string GetCPUInstruction(const Cpu & cpu) {
        static std::string names[] = {
            "LDA", "LDX", "LDY", "STA", "STX", "STY",               // Load, Store
            "TAX", "TAY", "TXA", "TYA",                             // Register Transfer
            "TSX", "TXS", "PHA", "PLA", "PHP", "PLP",               // Stack
            "AND", "BIT", "EOR", "ORA",                             // Logical
            "ADC", "SBC", "CMP", "CPX", "CPY",                      // Arithmetic
            "DEC", "DEX", "DEY", "INC", "INX", "INY",               // Increment, Decrement
            "ASL", "LSR", "ROL", "ROR",                             // Shift
            "JMP", "JSR", "RTS",                                    // Jump, Call
            "BCC", "BCS", "BEQ", "BMI", "BNE", "BPL", "BVC", "BVS", // Branch
            "CLC", "CLD", "CLI", "CLV", "SEC", "SED", "SEI",        // Status Change
            "BRK", "NOP", "RTI",                                    // System
           "uSTP","uSLO","uNOP","uANC","uRLA","uSRE","uALR","uRRA","uARR", // Unofficial
           "uSAX", "uXAA", "uAHX", "uTAS", "uSHY", "uSHX", "uLAX", "uLAS",
           "uDCP", "uAXS", "uISC", "uSBC",
           "UNK",
        };

        std::stringstream oss;
        const auto instruction = cpu.ReadByteAt(cpu.PC);
        const auto opcode = cpu.Decode(instruction);
        const auto operandB = Word{ cpu.ReadByteAt(cpu.PC + 1) };
        const auto operandW = cpu.ReadWordAt(cpu.PC + 1);
        const auto address = cpu.BuildAddress(opcode.Addressing);
        oss << hex << setfill('0');
        oss << names[opcode.Instruction];
        switch (opcode.Addressing) {
        case Addressing::Immediate: oss << " #$" << setw(2) << operandB; break;
        case Addressing::ZeroPage:  oss << " $" << setw(2) << operandB; break;
        case Addressing::ZeroPageX: oss << " $" << setw(2) << operandB << ",X"; break;
        case Addressing::ZeroPageY: oss << " $" << setw(2) << operandB << ",Y"; break;
        case Addressing::Relative:  oss << " *+" << setw(2) << operandB; break;
        case Addressing::Absolute:  oss << " $" << setw(4) << operandW; break;
        case Addressing::AbsoluteX: oss << " $" << setw(4) << operandW << ",X"; break;
        case Addressing::AbsoluteY: oss << " $" << setw(4) << operandW << ",Y"; break;
        case Addressing::Indirect:  oss << " ($" << setw(4) << operandW << ")"; break;
        case Addressing::IndexedIndirect: oss << " ($" << setw(2) << operandB << ",X)"; break;
        case Addressing::IndirectIndexed: oss << " ($" << setw(2) << operandB << "),Y"; break;
        case Addressing::Implicit: break;
        case Addressing::Accumulator: break;
        case Addressing::Unknown: oss << " ????";
        }
        return oss.str();
    }

    std::string GetPlayer1(const Controllers & ctrl) {
        std::stringstream oss;
        oss << "P1 ";
        oss << (ctrl.P1_Up     ? "U" : "u");
        oss << (ctrl.P1_Down   ? "D" : "d");
        oss << (ctrl.P1_Left   ? "L" : "l");
        oss << (ctrl.P1_Right  ? "R" : "r");
        oss << (ctrl.P1_Select ? "S" : "s");
        oss << (ctrl.P1_Start  ? "S" : "s");
        oss << (ctrl.P1_B      ? "B" : "b");
        oss << (ctrl.P1_A      ? "A" : "a");
        return oss.str();
    }

    std::string GetAPU(const Apu<Cpu> & apu) {
        std::stringstream oss;
        oss << "APU Frame Counter " << endl
            << "        Mode " << ((apu.Frame.Mode == 0) ? 4 : 5) << " steps" << endl
            << "        Counter " << apu.Frame.Ticks << endl
            << "    Pulse 1 " << dec << apu.Pulse1.Enabled << endl
            << "        P " << apu.Pulse1.T.Period << " T " << apu.Pulse1.T.T << " D " << apu.Pulse1.Sequence.Duty << " " << apu.Pulse1.Sequence.Phase << std::endl
            << "        SE " << apu.Pulse1.SweepEnabled << " SP " << Word{ apu.Pulse1.SweepPeriod } << " ST " << Word{ apu.Pulse1.SweepT } << " SN " << apu.Pulse1.SweepNegate << " SA " << Word{ apu.Pulse1.SweepAmount } << endl
            << "        LC " << apu.Pulse1.Length.Count << " Halt " << apu.Pulse1.Length.Halt << endl
            << "    Pulse 2 " << dec << apu.Pulse2.Enabled << endl
            << "        P " << apu.Pulse2.T.Period << " T " << apu.Pulse2.T.T << " D " << apu.Pulse2.Sequence.Duty << " " << apu.Pulse2.Sequence.Phase << std::endl
            << "        SE " << apu.Pulse2.SweepEnabled << " SP " << Word{ apu.Pulse2.SweepPeriod } << " ST " << Word{ apu.Pulse2.SweepT } << " SN " << apu.Pulse2.SweepNegate << " SA " << Word{ apu.Pulse2.SweepAmount } << endl
            << "        LC " << apu.Pulse2.Length.Count << " Halt " << apu.Pulse2.Length.Halt << endl
            << "    Triangle " << apu.Triangle1.Enabled << endl
            << "        P " << apu.Triangle1.T.Period << " T " << apu.Triangle1.T.T << " Phase " << apu.Triangle1.Sequence.Phase << endl
            << "        CC " << apu.Triangle1.Counter.Count << " CRV " << apu.Triangle1.Counter.ReloadValue << " CR " << apu.Triangle1.Counter.Reload << " CC " << apu.Triangle1.Counter.Control << endl
            << "        LC " << apu.Triangle1.Length.Count << " Halt " << apu.Triangle1.Length.Halt << endl;
        return oss.str();
    }
}

Uint32 RGBA(int r, int g, int b, int a) {
    return (Uint32(r) << 24) + (Uint32(g) << 16) + (Uint32(b) << 8) + Uint32(a);
}

Uint32 RGB(int r, int g, int b) {
    return RGBA(r, g, b, SDL_ALPHA_OPAQUE);
}

Uint32 Grey(int x) {
    return RGBA(x, x, x, SDL_ALPHA_OPAQUE);
}

Uint32 RGBA(float r, float g, float b, float a) {
    return RGBA(Uint8(255 * r), Uint8(255 * g), Uint8(255 * b), Uint8(255 * a));
}

Uint32 RGB(float r, float g, float b) {
    return RGB(Uint8(255 * r), Uint8(255 * g), Uint8(255 * b));
}

std::array<Uint32, 0x40> base_palette{
    RGB( 84,  84,  84), RGB(  0,  30, 116), RGB(  8,  16, 144), RGB( 48,   0, 136),
    RGB( 68,   0, 100), RGB( 92,   0,  48), RGB( 84,   4,   0), RGB( 60,  24,   0),
    RGB( 32,  42,   0), RGB(  8,  58,   0), RGB(  0,  64,   0), RGB(  0,  60,   0),
    RGB(  0,  50,  60), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
    RGB(152, 150, 152), RGB(  8,  76, 196), RGB( 48,  50, 236), RGB( 92,  30, 228),
    RGB(136,  20, 176), RGB(160,  20, 100), RGB(152,  34,  32), RGB(120,  60,   0),
    RGB( 84,  90,   0), RGB( 40, 114,   0), RGB(  8, 124,   0), RGB(  0, 118,  40),
    RGB(  0, 102, 120), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
    RGB(236, 238, 236), RGB( 76, 154, 236), RGB(120, 124, 236), RGB(176,  98, 236),
    RGB(228,  84, 236), RGB(236,  88, 180), RGB(236, 106, 100), RGB(212, 136,  32),
    RGB(160, 170,   0), RGB(116, 196,   0), RGB( 76, 208,  32), RGB( 56, 204, 108),
    RGB( 56, 180, 204), RGB( 60,  60,  60), RGB(  0,   0,   0), RGB(  0,   0,   0),
    RGB(236, 238, 236), RGB(168, 204, 236), RGB(188, 188, 236), RGB(212, 178, 236),
    RGB(236, 174, 236), RGB(236, 174, 212), RGB(236, 180, 176), RGB(228, 196, 144),
    RGB(204, 210, 120), RGB(180, 222, 120), RGB(168, 226, 144), RGB(152, 226, 180),
    RGB(160, 214, 228), RGB(160, 162, 160), RGB(  0,   0,   0), RGB(  0,   0,   0)
};
std::array<Uint32, 0x400> palette;

void BuildPalette() {
    for (int i = 0; i < 0x40; ++i) {
        const auto c = base_palette[i];
        palette[0x000 + i] = c;
        palette[0x040 + i] = 0xFFE0E0FF & c;
        palette[0x080 + i] = 0xE0FFE0FF & c;
        palette[0x0C0 + i] = 0xFFFFE0FF & c;
        palette[0x100 + i] = 0xE0E0FFFF & c;
        palette[0x140 + i] = 0xFFE0FFFF & c;
        palette[0x180 + i] = 0xE0FFFFFF & c;
        palette[0x1C0 + i] = 0xFFFFFFFF & c;

        palette[0x200 + i] = palette[i & 0x30];
        palette[0x240 + i] = palette[i & 0x30];
        palette[0x280 + i] = palette[i & 0x30];
        palette[0x2C0 + i] = palette[i & 0x30];
        palette[0x300 + i] = palette[i & 0x30];
        palette[0x340 + i] = palette[i & 0x30];
        palette[0x380 + i] = palette[i & 0x30];
        palette[0x3C0 + i] = palette[i & 0x30];
    }
}

struct SDL {
    struct Window {

        explicit Window(int sx, int sy, const Uint32 * pixels) {
            Init(sx, sy);
            while (!Quit) {
                SDL_UpdateTexture(tex, NULL, pixels, sx * sizeof(Uint32));
                SDL_RenderCopy(ren, tex, NULL, NULL);
                SDL_RenderPresent(ren);
                SDL_UpdateWindowSurface(win);
                Pump();
            }
            Close();
        }
        ~Window() {}

        SDL_Window * win;
        SDL_Renderer * ren;
        SDL_Texture * tex;
        void Init(int sx, int sy) {
            win = SDL_CreateWindow("Software Renderer",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                SDL::get()->Scale * sx, SDL::get()->Scale * sy,
                SDL_WINDOW_SHOWN);
            ren = SDL_CreateRenderer(win, -1, 0);
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
            SDL_RenderSetLogicalSize(ren, sx, sy);
            tex = SDL_CreateTexture(ren,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_STREAMING,
                sx, sy);
        }
        void Close() {
            SDL_DestroyTexture(tex);
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
        }

        bool Quit = false;
        void Pump() {
            SDL_Event e;
            while (SDL_PollEvent(&e) > 0)
            {
                switch (e.type)
                {
                case SDL_QUIT:
                    Quit = true;
                    break;
                case SDL_KEYDOWN:
                    Quit = (e.key.keysym.sym == SDLK_ESCAPE);
                    break;
                }
            }
        }
    };

    int Scale;
    static void SetScale(int scale) { SDL::get()->Scale = scale; }
    static void Show(int sx, int sy, const Uint32 * pixels) {
        get();
        Window w(sx, sy, pixels);
    }

    static SDL * get() {
        static SDL instance;
        return &instance;
    }

    SDL() {
        SDL_Init(SDL_INIT_EVERYTHING);
    }
    
    ~SDL() {
        SDL_Quit();
    }
};

void usage() {
    std::cout << "NeMux emulator visualizer" << std::endl;
    std::cout << "Usage: nemux-vis [options] nesfile" << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "    nesfile    Path the the NES ROM file" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    -help            Print this help message" << std::endl;
}

void error(const std::string & message) {
    std::cout << message << std::endl;
    std::cout << std::endl;
    usage();
}

void log(const std::string & msg) {
    std::cout << "- " << msg << std::endl;
}

enum class Options
{
    Debug,
    Record,
    Replay,
    Test,
    NSF,
};

static int frames = 0;
static std::vector<Sint16> SDLaudio;
void PushFrameSamples(const std::vector<float> & frame) {
    frames++;
    //const int step = 37; // 1.789773 MHz / 48 kHz ~= 37.29 APU samples per audio sample
    // Single frame samples (1/60th of a second)
    //std::cout << " " << SDLaudio.size();
    //const float fstep = frame.size() / 800.0f;
    //float step = fstep;
    //int i = 0;
    //float s = 0.0f;
    //const int size = frame.size();
    //while (i < size) {
    //    s += frame[i];
    //    ++i;
    //    if (i > step) {
    //        Sint16 SampleValue = 20000 * (s / fstep) - 10000;
    //        SDLaudio.push_back(SampleValue);
    //        s = 0.0f;
    //        step += fstep;
    //    }
    //}
    //std::cout << " " << SDLaudio.size() << std::endl;
    //if ((size % step) != 0) {
    //    Sint16 SampleValue = 20000 * (s / float(size % step)) - 10000;
    //    SDLaudio.push_back(SampleValue);
    //}

    enum DownsamplingMethod {
        DropSamples,
        AverageSamples,
    };

    const DownsamplingMethod method = DropSamples;
    switch (method)
    {
    case AverageSamples: {
        const int size = frame.size();
        const int step = 37; // 1.789773 MHz / 48 kHz ~= 37.29 APU samples per audio sample
        for (size_t i = 0; i + step < size; i += step) {
            float s = 0.0f;
            for (size_t j = 0; j < step; j++)
                s += frame[i + j];
            Sint16 SampleValue = 20000 * (s / float(step)) - 10000;
            SDLaudio.push_back(SampleValue);
        }
    } break;
    case DropSamples:
    default: {
        const int step = 37; // 1.789773 MHz / 48 kHz ~= 37.29 APU samples per audio sample
        for (size_t i = 0; i < 800; i++) {
            const float s = frame[step * i];
            Sint16 SampleValue = 20000 * s - 10000;
            SDLaudio.push_back(SampleValue);
        }
    } break;
    }
}
void FillAudioDeviceBuffer(void* UserData, Uint8* DeviceBuffer, int Length)
{
    std::fill_n(DeviceBuffer, Length, Uint8(0));

    //if (frames < 2) return;
    const int SamplesToWrite = Length / 2;
    if (SDLaudio.size() < SamplesToWrite) {
        return;
    }
    if (SDLaudio.size() > 2 * SamplesToWrite) {
        SDLaudio.resize(2 * SamplesToWrite);
    }

    Sint16* SampleBuffer = (Sint16*)DeviceBuffer;
    for (int SampleIndex = 0;
        SampleIndex < SamplesToWrite;
        SampleIndex++)
    {
        Sint16 SampleValue = SDLaudio[SampleIndex];
        *SampleBuffer++ = SampleValue;
    }
    for (int i = 0; i + SamplesToWrite < SDLaudio.size(); ++i) {
        SDLaudio[i] = SDLaudio[i + SamplesToWrite];
    }
    //samples.resize(10000);
    SDLaudio.resize(SDLaudio.size() - SamplesToWrite);
    //samples.clear();
}

struct Fps {
    int fps = -1;
    int counter = 0;
    Uint32 ticks = 0;

    bool update(const Uint32 sdlTicks) {
        ++counter;
        if ((sdlTicks - ticks) > 1000) {
            fps = counter;
            counter = 0;
            ticks = sdlTicks;
            return true;
        }
        return false;
    }
};

int main(int argc, char ** argv) {
    BuildPalette();

    std::vector<std::string> positionals;
    std::set<Options> options;
    std::string recordFilename;
    for (int i = 1; i < argc; ++i) {
        std::string param(argv[i]);
        if (param[0] == '-') {
            if (param == "-help") {
                usage();
                return 0;
            }
            else if (param == "-debug") {
                options.insert(Options::Debug);
            }
            else if (param == "-record") {
                options.insert(Options::Record);
                recordFilename = argv[++i];
            }
            else if (param == "-replay") {
                options.insert(Options::Replay);
                recordFilename = argv[++i];
            }
            else if (param == "-test") {
                options.insert(Options::Test);
                recordFilename = argv[++i];
            }
            else if (param == "-nsf") {
                options.insert(Options::NSF);
            }
            else {
                error("Unrecognized parameter: " + param);
                return 1;
            }
        }
        else {
            positionals.push_back(param);
        }
    }

    auto IsSet = [&options](const Options & opt) { return options.count(opt) == 1; };

    Fps fps;
    bool showFps = false;
    int frameSkip = 0;

    try {
        std::string filepath;
        
        std::ifstream replay;
        if (IsSet(Options::Replay) || IsSet(Options::Test)) {
            replay.open(recordFilename);
            std::getline(replay, filepath, '\0');
        } else {
            if (positionals.size() != 1) {
                error("Please specify a NES ROM file");
                return 1;
            }
            filepath = positionals[0];
        }
        std::ofstream record;
        if (IsSet(Options::Record)) {
            record.open(recordFilename);
            record << filepath.c_str() << '\0';
        }

        std::unique_ptr<NesMapper> mapper;
        if (IsSet(Options::NSF)) {
            log("Opening NSF ROM at " + filepath + " ...");
            std::ifstream file(filepath, std::ios::binary);
            NsfFile rom(file);
            log("Done.");
            mapper.reset(new Mapper_NSF(rom));
        } else {
            log("Opening NES ROM at " + filepath + " ...");
            std::ifstream file(filepath, std::ios::binary);
            NesFile rom(file);
            log("Done.");
            switch (rom.Header.MapperNumber) {
            case 0: mapper.reset(new Mapper_000(rom)); break;
            case 1: mapper.reset(new Mapper_001(rom)); break;
            case 2: mapper.reset(new Mapper_002(rom)); break;
            case 3: mapper.reset(new Mapper_003(rom)); break;
            }
        }
        
        Machine nes(mapper);

        if (IsSet(Options::Debug)) {
            bool quit = false;
            long long step = 0;
            std::size_t counter = 1;
            std::string line;
            while (!quit) {
                if (step == 0) {
                    std::cout << dec << counter
                        << " " << setfill(' ') << setw(12) << std::left << debug::GetCPUInstruction(nes.cpu) << std::endl
                        << " " << nes.cpu.ToMiniString() << std::endl
                        << " " << debug::GetPlayer1(nes.ctrl) << std::endl
                        << " " << debug::GetAPU(nes.apu) << std::endl;
                }
                if (step == 0) {
                    std::cout << "> ";
                    std::getline(std::cin, line);
                    if (line.empty()) step = 1;
                    else if (line == "q") quit = true;
                    else if (line == "r") step = -1;
                    else if (line == "p") {
                        std::vector<Uint32> p(4 * 8);
                        std::cout << "Palette" << std::endl;
                        for (auto i = 0; i < 8; i++) {
                            for (auto c = 0; c < 4; c++) {
                                auto ci = nes.ppu.PpuPalette.ReadAt(4 * i + c);
                                p[4 * i + c] = palette[ci];
                                std::cout << hex << setfill('0') << setw(2) << Word{ ci } << ' ';
                            }
                            std::cout << std::endl;
                        }
                        SDL::SetScale(32);
                        SDL::Show(4, 8, p.data());
                    }
                    else if (line == "nt") {
                        std::vector<Uint32> p(64 * 60);
                        const Byte * nt = &nes.ppumap.Vram[0];
                        std::cout << "Nametable 0 @0x2000" << std::endl;
                        for (auto y = 0; y < 30; ++y) {
                            for (auto x = 0; x < 32; x++) {
                                const auto d = 32 * y + x;
                                p[64 * y + x] = Grey(nes.ppumap.GetByteAt(0x2000 + d));
                                p[64 * y + x + 32] = Grey(nes.ppumap.GetByteAt(0x2400 + d));
                                p[64 * y + x + 64 * 30] = Grey(nes.ppumap.GetByteAt(0x2800 + d));
                                p[64 * y + x + 64 * 30 + 32] = Grey(nes.ppumap.GetByteAt(0x2C00 + d));
                                std::cout << hex << setfill('0') << setw(2) << Word{ *(nt + d) } << ' ';
                                //++nt;
                            }
                            std::cout << std::endl;
                        }
                        SDL::SetScale(8);
                        SDL::Show(64, 60, p.data());
                    }
                    else if (line == "at") {
                        std::vector<Uint32> p(16 * 16);
                        const Byte * at = &nes.ppumap.Vram[0x3C0];
                        std::cout << "Attribute Table 0 @0x23C0" << std::endl;
                        for (auto y = 0; y < 8; ++y) {
                            for (auto x = 0; x < 8; x++) {
                                const auto d = 16 * y + x;
                                p[16 * y + x] = Grey(nes.ppumap.GetByteAt(0x23C0 + d));
                                p[16 * y + x + 8] = Grey(nes.ppumap.GetByteAt(0x27C0 + d));
                                p[16 * y + x + 16 * 8] = Grey(nes.ppumap.GetByteAt(0x2BC0 + d));
                                p[16 * y + x + 16 * 8 + 8] = Grey(nes.ppumap.GetByteAt(0x2FC0 + d));
                                std::cout << hex << setfill('0') << setw(2) << Word{ *at } << ' ';
                                ++at;
                            }
                            std::cout << std::endl;
                        }
                        SDL::SetScale(32);
                        SDL::Show(16, 16, p.data());
                    }
                    else if (line == "pt") {
                        std::vector<Uint32> p(128 * 256, 0);
                        std::cout << "Pattern Table @0x0000" << std::endl;
                        for (auto y = 0; y < 256; ++y) {
                            for (auto x = 0; x < 128; x++) {
                                auto tx = x / 8; auto xx = x % 8;
                                auto ty = y / 8; auto yy = y % 8;
                                auto td = 16 * ty + tx;
                                auto b = nes.ppumap.GetByteAt(16 * td + yy);
                                Uint8 v = (b >> (7 - xx)) & 0x01;
                                b = nes.ppumap.GetByteAt(16 * td + yy + 8);
                                v += ((b >> (7 - xx)) & 0x01) << 1;
                                p[128 * y + x] = RGB(v / 3.0f, v / 3.0f, v / 3.0f);
                            }
                        }
                        SDL::SetScale(2);
                        SDL::Show(128, 256, p.data());
                    }
                    else if (line == "bg") {
                        std::vector<Uint32> p(256 * 240, 0);
                        std::cout << "Background" << std::endl;
                        for (auto y = 0; y < 240; ++y) {
                            for (auto x = 0; x < 256; x++) {
                                auto tx = x / 8; auto xx = x % 8;
                                auto ty = y / 8; auto yy = y % 8;
                                auto td = nes.ppumap.Vram[32 * ty + tx];
                                auto taddr = nes.ppu.BackgroundTable + 16 * td + yy;
                                auto b = nes.ppumap.GetByteAt(taddr);
                                Uint8 v = (b >> (7 - xx)) & 0x01;
                                b = nes.ppumap.GetByteAt(taddr + 8);
                                v += ((b >> (7 - xx)) & 0x01) << 1;
                                auto atx = x / 32; auto aty = y / 32;
                                auto a = nes.ppumap.Vram[0x3C0 + 8 * aty + atx];
                                v += ((a >> (2 * (x / 16 % 2) + 4 * (y / 16 % 2))) & 0x3) << 2;
                                auto ci = nes.ppu.PpuPalette.ReadAt(v);
                                auto color = palette[ci];
                                p[256 * y + x] = color;
                            }
                        }

                        SDL::SetScale(4);
                        SDL::Show(256, 240, p.data());
                    }
                    else if (line == "fg") {
                        std::vector<Uint32> p(256 * 240, 0);
                        std::cout << "Foreground" << std::endl;
                        for (auto s = 0; s < 64; s++) {
                            auto at = nes.ppu.SprRam[4 * s + 2];
                            auto td = nes.ppu.SprRam[4 * s + 1];
                            auto sx = nes.ppu.SprRam[4 * s + 3];
                            auto sy = nes.ppu.SprRam[4 * s + 0];
                            for (auto yy = 0; yy < nes.ppu.SpriteHeight; yy++) {
                                for (auto xx = 0; xx < 8; xx++) {
                                    auto taddr = nes.ppu.SpriteTable + 16 * td + yy;
                                    auto b = nes.ppumap.GetByteAt(taddr);
                                    Uint8 v = (b >> (7 - xx)) & 0x01;
                                    b = nes.ppumap.GetByteAt(taddr + 8);
                                    v += ((b >> (7 - xx)) & 0x01) << 1;
                                    v += (at & 0x3) << 2;
                                    v += 0x10;

                                    auto x = sx + xx;
                                    auto y = sy + yy + 1;
                                    if ((0 <= x) && (x < 256) && (0 <= y) && (y < 240)) {
                                        auto ci = nes.ppu.PpuPalette.ReadAt(v);
                                        auto color = palette[ci];
                                        p[256 * y + x] = color;
                                    }
                                }
                            }
                        }

                        SDL::SetScale(4);
                        SDL::Show(256, 240, p.data());
                    }
                    else if (line[0] == 'f') {
                        step = 29781 * std::stoll(line.substr(1));
                    }
                    else if (line == "a") {
                        std::vector<Uint32> p(VIDEO_WIDTH * VIDEO_HEIGHT, 0);
                        std::cout << "PPU frame" << std::endl;
                        for (auto i = 0; i < VIDEO_SIZE; ++i) {
                            p[i] = palette[nes.ppu.Frame[i]];
                        }
                        SDL::SetScale(3);
                        SDL::Show(VIDEO_WIDTH, VIDEO_HEIGHT, p.data());
                    }
                    else if (line == "p1 up")     nes.ctrl.P1_Up = !nes.ctrl.P1_Up;
                    else if (line == "p1 down")   nes.ctrl.P1_Down = !nes.ctrl.P1_Down;
                    else if (line == "p1 left")   nes.ctrl.P1_Left = !nes.ctrl.P1_Left;
                    else if (line == "p1 right")  nes.ctrl.P1_Right = !nes.ctrl.P1_Right;
                    else if (line == "p1 select") nes.ctrl.P1_Select = !nes.ctrl.P1_Select;
                    else if (line == "p1 start")  nes.ctrl.P1_Start = !nes.ctrl.P1_Start;
                    else if (line == "p1 b")      nes.ctrl.P1_B = !nes.ctrl.P1_B;
                    else if (line == "p1 a")      nes.ctrl.P1_A = !nes.ctrl.P1_A;
                    else {
                        try {
                            step = std::stoll(line);
                        }
                        catch (const std::exception & e) {
                            break;
                        }
                    }
                }
                else {
                    //std::cout << std::endl;
                }

                if (step > 0) {
                    --step;
                    ++counter;
                    nes.StepOneCpuInstruction();
                }

            }
        }
        else if (IsSet(Options::Test)) {
            bool quit = false;
            while (!quit) {
                const auto pair = nes.Step();
                if (pair.first) {
                    char cmd;
                    replay >> cmd;
                    quit = replay.eof();

                    bool finished = quit;
                    while (!finished) {
                        replay >> cmd;
                        switch (cmd) {
                        case replay::FrameEnd: {
                            finished = true;
                            break;
                        }
                        case replay::Player_1: {
                            Byte p1;
                            replay >> p1;
                            replay::SetP1State(nes, p1);
                            break;
                        }
                        case replay::CheckFrame: {
                            std::cout << "Check frame" << std::endl;
                            Byte d;
                            bool success = true;
                            std::array<Uint32, VIDEO_SIZE> difference;
                            for (int i = 0; i < VIDEO_SIZE; ++i) {
                                replay >> d;
                                const bool isSame = (nes.ppu.Frame[i] == d);
                                success = success && isSame;
                                difference[i] = isSame ? Grey(32) : Grey(224);
                            }
                            if (!success) {
                                SDL::SetScale(3);
                                SDL::Show(VIDEO_WIDTH, VIDEO_HEIGHT, difference.data());
                                throw std::runtime_error("Frame check failed");
                            }
                            break;
                        }
                        case replay::Reset: {
                            nes.Reset();
                            break;
                        }
                        }
                    }
                }
            }
        }
        else {
            bool replayCheckFrame = false;
            bool replayReset = false;
            SDL::SetScale(3);

            std::array<Uint32, VIDEO_SIZE> pixels;
            pixels.fill(0);

            SDL_Window * win;
            SDL_Renderer * ren;
            SDL_Texture * tex;
            win = SDL_CreateWindow("Software Renderer",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                SDL::get()->Scale * VIDEO_WIDTH, SDL::get()->Scale * VIDEO_HEIGHT,
                SDL_WINDOW_SHOWN);
            ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
            SDL_RenderSetLogicalSize(ren, VIDEO_WIDTH, VIDEO_HEIGHT);
            tex = SDL_CreateTexture(ren,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_STREAMING,
                VIDEO_WIDTH, VIDEO_HEIGHT);

            SDL_AudioSpec RequestedSettings = {};
            RequestedSettings.freq = 48000; // Our sampling rate
            RequestedSettings.format = AUDIO_S16; // Use 16-bit amplitude values
            RequestedSettings.channels = 1; // Stereo samples
            RequestedSettings.samples = 4096; // Size, in samples, of audio buffer
            RequestedSettings.callback = &FillAudioDeviceBuffer; // Function called when sound device needs data

            SDL_AudioSpec ObtainedSettings = {};
            SDL_AudioDeviceID DeviceID = SDL_OpenAudioDevice(
                NULL, 0, &RequestedSettings, &ObtainedSettings, 0
            );

            // Start music playing
            SDL_PauseAudioDevice(DeviceID, 0);

            bool quit = false;
            std::vector<float> samples;
            while (!quit) {
                const auto pair = nes.Step();
                samples.push_back(pair.second);

                if (pair.first) {
                    PushFrameSamples(samples);
                    samples.clear();
                    SDL_Event e;
                    while (SDL_PollEvent(&e) > 0)
                    {
                        switch (e.type)
                        {
                        case SDL_QUIT:
                            quit = true;
                            break;
                        case SDL_KEYDOWN: {
                            quit = quit || (e.key.keysym.sym == SDLK_ESCAPE);
                            if (e.key.keysym.sym == SDLK_UP) nes.ctrl.P1_Up = true;
                            if (e.key.keysym.sym == SDLK_DOWN) nes.ctrl.P1_Down = true;
                            if (e.key.keysym.sym == SDLK_LEFT) nes.ctrl.P1_Left = true;
                            if (e.key.keysym.sym == SDLK_RIGHT) nes.ctrl.P1_Right = true;
                            if (e.key.keysym.sym == SDLK_o) nes.ctrl.P1_Select = true;
                            if (e.key.keysym.sym == SDLK_p) nes.ctrl.P1_Start = true;
                            if (e.key.keysym.sym == SDLK_s) nes.ctrl.P1_A = true;
                            if (e.key.keysym.sym == SDLK_q) nes.ctrl.P1_B = true;
                            if (e.key.keysym.sym == SDLK_F1) nes.apu.Pulse1Output = 1 - nes.apu.Pulse1Output;
                            if (e.key.keysym.sym == SDLK_F2) nes.apu.Pulse2Output = 1 - nes.apu.Pulse2Output;
                            if (e.key.keysym.sym == SDLK_F3) nes.apu.Triangle1Output = 1 - nes.apu.Triangle1Output;
                            if (e.key.keysym.sym == SDLK_F4) nes.apu.Noise1Output = 1 - nes.apu.Noise1Output;
                            if (e.key.keysym.sym == SDLK_F5) nes.apu.DMC1Output = 1 - nes.apu.DMC1Output;
                            if (e.key.keysym.sym == SDLK_F10) --frameSkip;
                            if (e.key.keysym.sym == SDLK_F11) ++frameSkip;
                            if (e.key.keysym.sym == SDLK_F12) showFps = !showFps;
                            if (e.key.keysym.sym == SDLK_SPACE) replayCheckFrame = true;
                            if (e.key.keysym.sym == SDLK_BACKSPACE) replayReset = true;
                            break;
                        }
                        case SDL_KEYUP: {
                            quit = quit || (e.key.keysym.sym == SDLK_ESCAPE);
                            if (e.key.keysym.sym == SDLK_UP) nes.ctrl.P1_Up = false;
                            if (e.key.keysym.sym == SDLK_DOWN) nes.ctrl.P1_Down = false;
                            if (e.key.keysym.sym == SDLK_LEFT) nes.ctrl.P1_Left = false;
                            if (e.key.keysym.sym == SDLK_RIGHT) nes.ctrl.P1_Right = false;
                            if (e.key.keysym.sym == SDLK_o) nes.ctrl.P1_Select = false;
                            if (e.key.keysym.sym == SDLK_p) nes.ctrl.P1_Start = false;
                            if (e.key.keysym.sym == SDLK_s) nes.ctrl.P1_A = false;
                            if (e.key.keysym.sym == SDLK_q) nes.ctrl.P1_B = false;
                            break;
                        }
                        }
                    }
                    if (IsSet(Options::Record)) {
                        record << char{ replay::FrameStart }
                            << char{ replay::Player_1 }
                            << replay::GetP1State(nes);
                        if (replayCheckFrame) {
                            record << char{ replay::CheckFrame };
                            for (int i = 0; i < VIDEO_SIZE; ++i) record << nes.ppu.Frame[i];
                            replayCheckFrame = false;
                        }
                        if (replayReset) record << char{ replay::Reset };
                        record << char{ replay::FrameEnd };
                    }
                    if (IsSet(Options::Replay)) {
                        char cmd;
                        Byte p1;
                        replay >> cmd;
                        SDL_assert(cmd == replay::FrameStart);
                        bool finished = false;
                        do {
                            replay >> cmd;
                            switch (cmd) {
                            case replay::FrameEnd: {
                                finished = true;
                                break;
                            }
                            case replay::Player_1: {
                                replay >> p1;
                                replay::SetP1State(nes, p1);
                                break;
                            }
                            case replay::CheckFrame: {
                                Byte d;
                                for (int i = 0; i < VIDEO_SIZE; ++i) replay >> d;
                                break;
                            }
                            case replay::Reset: {
                                nes.Reset();
                                break;
                            }
                            }
                        } while (!finished);
                    }
                    if (replayReset) {
                        nes.Reset();
                        replayReset = false;
                    }

                    const auto ticks = SDL_GetTicks();
                    if (fps.update(ticks)) {
                        if (showFps) std::cout << fps.fps << std::endl;
                    }

                    if (frameSkip <= 0) {
                        for (auto i = 0; i < VIDEO_SIZE; ++i) {
                            pixels[i] = palette[nes.ppu.Frame[i]];
                        }

                        auto skip = frameSkip;
                        while (skip <= 0) {
                            SDL_UpdateTexture(tex, NULL, pixels.data(), VIDEO_WIDTH * sizeof(Uint32));
                            SDL_RenderCopy(ren, tex, NULL, NULL);
                            SDL_RenderPresent(ren);
                            SDL_UpdateWindowSurface(win);
                            ++skip;
                        }
                    }
                    else {
                        for (auto i = 0; i < VIDEO_SIZE; ++i) {
                            pixels[i] = palette[nes.ppu.Frame[i]];
                        }

                        SDL_UpdateTexture(tex, NULL, pixels.data(), VIDEO_WIDTH * sizeof(Uint32));
                        SDL_RenderCopy(ren, tex, NULL, NULL);
                        SDL_RenderPresent(ren);
                        SDL_UpdateWindowSurface(win);
                    }
                }
            }

            SDL_CloseAudioDevice(DeviceID);

            SDL_DestroyTexture(tex);
            SDL_DestroyRenderer(ren);
            SDL_DestroyWindow(win);
        }
    }
    catch (const std::exception & e) {
        log("Exception: " + std::string(e.what()));
        throw e;
    }

    return 0;
}
