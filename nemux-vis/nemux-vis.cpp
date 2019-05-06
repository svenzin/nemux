#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

#include "SDL.h"

#include "NesFile.h"
#include "Mapper_0.h"
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

    std::string GetAPU(const Apu & apu) {
        std::stringstream oss;
        oss << "APU Frame Counter " << endl
            << "        Mode " << ((apu.Pulse1.Frame.Mode == 0) ? 4 : 5) << " steps" << endl
            << "        Counter " << apu.Pulse1.Frame.Ticks << endl
            << "    Pulse 1 " << dec << apu.Pulse1.Enabled << endl
            << "        P " << apu.Pulse1.Period << " T " << apu.Pulse1.T << " D " << apu.Pulse1.Duty << endl
            << "        SE " << apu.Pulse1.SweepEnabled << " SP " << Word{ apu.Pulse1.SweepPeriod } << " ST " << Word{ apu.Pulse1.SweepT } << " SN " << apu.Pulse1.SweepNegate << " SA " << Word{ apu.Pulse1.SweepAmount } << endl
            << "        LC " << apu.Pulse1.Length << " Halt " << apu.Pulse1.Halt;
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

std::array<Uint32, 0x40> palette{
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
    Debug
};

static std::vector<float> samples;
void FillAudioDeviceBuffer(void* UserData, Uint8* DeviceBuffer, int Length)
{
    if (samples.size() == 0) {
        std::fill_n(DeviceBuffer, Length, Uint8(0));
        return;
    }

    Sint16* SampleBuffer = (Sint16*)DeviceBuffer;
    int SamplesToWrite = Length / 2;
    int step = 36;// 1789 / 48;
    int size = samples.size();
    for (int SampleIndex = 0;
        SampleIndex < SamplesToWrite;
        SampleIndex++)
    {
        float s = 0.0f;
        for (int i = 0; i < step; ++i) s += samples[(step * SampleIndex + i) % size];
        Sint16 SampleValue = 20000 * (s / float(step)) - 10000;
        *SampleBuffer++ = SampleValue;
    }
    samples.clear();
}

int main(int argc, char ** argv) {
    std::vector<std::string> positionals;
    std::set<Options> options;
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

    if (positionals.size() != 1) {
        error("Please specify a NES ROM file");
        return 1;
    }

    try {
        std::string filepath = positionals[0];
        log("Opening NES ROM at " + filepath + " ...");
        std::ifstream file(filepath, std::ios::binary);
        NesFile rom(file);
        log("Done.");

        Controllers ctrl;
        Mapper_000 mapper(rom);

        PpuMemoryMap<Palette> ppumap(nullptr, &mapper);
        Ppu ppu(&ppumap);

        Apu apu;

        CpuMemoryMap<Cpu, Ppu, Controllers, Apu> cpumap(nullptr, &apu, &ppu, &mapper, &ctrl);
        Cpu cpu("6502", &cpumap);
        cpumap.CPU = &cpu;

        cpu.Reset();

        if (IsSet(Options::Debug)) {
            bool quit = false;
            long long step = 0;
            std::size_t counter = 1;
            std::string line;
            while (!quit) {
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
                                auto ci = ppu.PpuPalette.ReadAt(4 * i + c);
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
                        const Byte * nt = &ppumap.Vram[0];
                        std::cout << "Nametable 0 @0x2000" << std::endl;
                        for (auto y = 0; y < 30; ++y) {
                            for (auto x = 0; x < 32; x++) {
                                const auto d = 32 * y + x;
                                p[64 * y + x] = Grey(ppumap.GetByteAt(0x2000 + d));
                                p[64 * y + x + 32] = Grey(ppumap.GetByteAt(0x2400 + d));
                                p[64 * y + x + 64 * 30] = Grey(ppumap.GetByteAt(0x2800 + d));
                                p[64 * y + x + 64 * 30 + 32] = Grey(ppumap.GetByteAt(0x2C00 + d));
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
                        const Byte * at = &ppumap.Vram[0x3C0];
                        std::cout << "Attribute Table 0 @0x23C0" << std::endl;
                        for (auto y = 0; y < 8; ++y) {
                            for (auto x = 0; x < 8; x++) {
                                const auto d = 16 * y + x;
                                p[16 * y + x] = Grey(ppumap.GetByteAt(0x23C0 + d));
                                p[16 * y + x + 8] = Grey(ppumap.GetByteAt(0x27C0 + d));
                                p[16 * y + x + 16 * 8] = Grey(ppumap.GetByteAt(0x2BC0 + d));
                                p[16 * y + x + 16 * 8 + 8] = Grey(ppumap.GetByteAt(0x2FC0 + d));
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
                                auto b = ppumap.GetByteAt(16 * td + yy);
                                Uint8 v = (b >> (7 - xx)) & 0x01;
                                b = ppumap.GetByteAt(16 * td + yy + 8);
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
                                auto td = ppumap.Vram[32 * ty + tx];
                                auto taddr = ppu.BackgroundTable + 16 * td + yy;
                                auto b = ppumap.GetByteAt(taddr);
                                Uint8 v = (b >> (7 - xx)) & 0x01;
                                b = ppumap.GetByteAt(taddr + 8);
                                v += ((b >> (7 - xx)) & 0x01) << 1;
                                auto atx = x / 32; auto aty = y / 32;
                                auto a = ppumap.Vram[0x3C0 + 8 * aty + atx];
                                v += ((a >> (2 * (x / 16 % 2) + 4 * (y / 16 % 2))) & 0x3) << 2;
                                auto ci = ppu.PpuPalette.ReadAt(v);
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
                            auto at = ppu.SprRam[4 * s + 2];
                            auto td = ppu.SprRam[4 * s + 1];
                            auto sx = ppu.SprRam[4 * s + 3];
                            auto sy = ppu.SprRam[4 * s + 0];
                            for (auto yy = 0; yy < ppu.SpriteHeight; yy++) {
                                for (auto xx = 0; xx < 8; xx++) {
                                    auto taddr = ppu.SpriteTable + 16 * td + yy;
                                    auto b = ppumap.GetByteAt(taddr);
                                    Uint8 v = (b >> (7 - xx)) & 0x01;
                                    b = ppumap.GetByteAt(taddr + 8);
                                    v += ((b >> (7 - xx)) & 0x01) << 1;
                                    v += (at & 0x3) << 2;
                                    v += 0x10;

                                    auto x = sx + xx;
                                    auto y = sy + yy + 1;
                                    if ((0 <= x) && (x < 256) && (0 <= y) && (y < 240)) {
                                        auto ci = ppu.PpuPalette.ReadAt(v);
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
                            p[i] = palette[ppu.Frame[i]];
                        }
                        SDL::SetScale(3);
                        SDL::Show(VIDEO_WIDTH, VIDEO_HEIGHT, p.data());
                    }
                    else if (line == "p1 up")     ctrl.P1_Up = !ctrl.P1_Up;
                    else if (line == "p1 down")   ctrl.P1_Down = !ctrl.P1_Down;
                    else if (line == "p1 left")   ctrl.P1_Left = !ctrl.P1_Left;
                    else if (line == "p1 right")  ctrl.P1_Right = !ctrl.P1_Right;
                    else if (line == "p1 select") ctrl.P1_Select = !ctrl.P1_Select;
                    else if (line == "p1 start")  ctrl.P1_Start = !ctrl.P1_Start;
                    else if (line == "p1 b")      ctrl.P1_B = !ctrl.P1_B;
                    else if (line == "p1 a")      ctrl.P1_A = !ctrl.P1_A;
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

                    cpu.Tick(); ppu.Tick(); ppu.Tick(); ppu.Tick(); apu.Tick();
                    while (cpu.CurrentTick < cpu.Ticks) {
                        cpu.Tick(); ppu.Tick(); ppu.Tick(); ppu.Tick(); apu.Tick();
                    }
                }

                if (step == 0) {
                    std::cout << dec << counter
                        << " " << setfill(' ') << setw(12) << std::left << debug::GetCPUInstruction(cpu) << std::endl
                        << " " << cpu.ToMiniString() << std::endl
                        << " " << debug::GetPlayer1(ctrl) << std::endl
                        << " " << debug::GetAPU(apu) << std::endl;
                }
            }
        }
        else {
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
            while (!quit) {
                const auto frame = ppu.FrameCount;
                cpu.Tick();
                ppu.Tick();
                ppu.Tick();
                ppu.Tick();
                const auto sample = apu.Tick();
                samples.push_back(sample);

                if (ppu.FrameCount != frame) {
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
                            if (e.key.keysym.sym == SDLK_UP) ctrl.P1_Up = true;
                            if (e.key.keysym.sym == SDLK_DOWN) ctrl.P1_Down = true;
                            if (e.key.keysym.sym == SDLK_LEFT) ctrl.P1_Left = true;
                            if (e.key.keysym.sym == SDLK_RIGHT) ctrl.P1_Right = true;
                            if (e.key.keysym.sym == SDLK_o) ctrl.P1_Select = true;
                            if (e.key.keysym.sym == SDLK_p) ctrl.P1_Start = true;
                            if (e.key.keysym.sym == SDLK_s) ctrl.P1_A = true;
                            if (e.key.keysym.sym == SDLK_q) ctrl.P1_B = true;
                            break;
                        }
                        case SDL_KEYUP: {
                            quit = quit || (e.key.keysym.sym == SDLK_ESCAPE);
                            if (e.key.keysym.sym == SDLK_UP) ctrl.P1_Up = false;
                            if (e.key.keysym.sym == SDLK_DOWN) ctrl.P1_Down = false;
                            if (e.key.keysym.sym == SDLK_LEFT) ctrl.P1_Left = false;
                            if (e.key.keysym.sym == SDLK_RIGHT) ctrl.P1_Right = false;
                            if (e.key.keysym.sym == SDLK_o) ctrl.P1_Select = false;
                            if (e.key.keysym.sym == SDLK_p) ctrl.P1_Start = false;
                            if (e.key.keysym.sym == SDLK_s) ctrl.P1_A = false;
                            if (e.key.keysym.sym == SDLK_q) ctrl.P1_B = false;
                            break;
                        }
                        }
                    }

                    for (auto i = 0; i < VIDEO_SIZE; ++i) {
                        pixels[i] = palette[ppu.Frame[i]];
                    }

                    SDL_UpdateTexture(tex, NULL, pixels.data(), VIDEO_WIDTH * sizeof(Uint32));
                    SDL_RenderCopy(ren, tex, NULL, NULL);
                    SDL_RenderPresent(ren);
                    SDL_UpdateWindowSurface(win);
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
