#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <string>
#include <iomanip>

#include "SDL.h"

#include "NesFile.h"
#include "Mapper_0.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Controllers.h"

using std::hex;
using std::dec;
using std::setfill;
using std::setw;

#undef main

Uint32 RGBA(int r, int g, int b, int a) {
    return (Uint32(r) << 24) + (Uint32(g) << 16) + (Uint32(b) << 8) + Uint32(a);
}

Uint32 RGB(int r, int g, int b) {
    return RGBA(r, g, b, SDL_ALPHA_OPAQUE);
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
            SDL_UpdateTexture(tex, NULL, pixels, sx * sizeof(Uint32));
            SDL_RenderCopy(ren, tex, NULL, NULL);
            SDL_RenderPresent(ren);
            while (!Quit) Pump();
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

                SDL_UpdateWindowSurface(win);
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
        SDL_Init(SDL_INIT_VIDEO);
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

        CpuMemoryMap<Cpu, Ppu, Controllers> cpumap(nullptr, &ppu, &mapper, &ctrl);
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
                        std::vector<Uint32> p(30 * 32);
                        const Byte * nt = &ppumap.Vram[0];
                        std::cout << "Nametable 0 @0x2000" << std::endl;
                        for (auto y = 0; y < 30; ++y) {
                            for (auto x = 0; x < 32; x++) {
                                p[32 * y + x] = RGB(*nt, *nt, *nt);
                                std::cout << hex << setfill('0') << setw(2) << Word{ *nt } << ' ';
                                ++nt;
                            }
                            std::cout << std::endl;
                        }
                        SDL::SetScale(8);
                        SDL::Show(32, 30, p.data());
                    }
                    else if (line == "at") {
                        std::vector<Uint32> p(8 * 8);
                        const Byte * at = &ppumap.Vram[0x3C0];
                        std::cout << "Attribute Table 0 @0x23C0" << std::endl;
                        for (auto y = 0; y < 8; ++y) {
                            for (auto x = 0; x < 8; x++) {
                                p[8 * y + x] = RGB(*at, *at, *at);
                                std::cout << hex << setfill('0') << setw(2) << Word{ *at } << ' ';
                                ++at;
                            }
                            std::cout << std::endl;
                        }
                        SDL::SetScale(32);
                        SDL::Show(8, 8, p.data());
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
                            for (auto yy = 0; yy < 8; yy++) {
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

                    cpu.Tick(); ppu.Tick(); ppu.Tick(); ppu.Tick();
                    //while (cpu.CurrentTick < cpu.Ticks) {
                    //    cpu.Tick(); ppu.Tick(); ppu.Tick(); ppu.Tick();
                    //}
                }

                if (step == 0) {
                    std::cout << dec << counter << " " << cpu.ToMiniString() << " ";
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
                ren = SDL_CreateRenderer(win, -1, 0);
                SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
                SDL_RenderSetLogicalSize(ren, VIDEO_WIDTH, VIDEO_HEIGHT);
                tex = SDL_CreateTexture(ren,
                    SDL_PIXELFORMAT_RGBA8888,
                    SDL_TEXTUREACCESS_STREAMING,
                    VIDEO_WIDTH, VIDEO_HEIGHT);

            bool quit = false;
            while (!quit) {
                const auto frame = ppu.FrameCount;
                cpu.Tick();
                ppu.Tick();
                ppu.Tick();
                ppu.Tick();

                if (ppu.FrameCount != frame) {
                    for (auto i = 0; i < VIDEO_SIZE; ++i) {
                        pixels[i] = palette[ppu.Frame[i]];
                    }

                    SDL_UpdateTexture(tex, NULL, pixels.data(), VIDEO_WIDTH * sizeof(Uint32));
                    SDL_RenderCopy(ren, tex, NULL, NULL);
                    SDL_RenderPresent(ren);
                    SDL_UpdateWindowSurface(win);

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
                            if (e.key.keysym.sym == SDLK_q) ctrl.P1_A = true;
                            if (e.key.keysym.sym == SDLK_s) ctrl.P1_B = true;
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
                            if (e.key.keysym.sym == SDLK_q) ctrl.P1_A = false;
                            if (e.key.keysym.sym == SDLK_s) ctrl.P1_B = false;
                            break;
                        }
                        }
                    }
                }
            }

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
