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

using std::hex;
using std::dec;
using std::setfill;
using std::setw;

#undef main

Uint32 RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    return (Uint32(r) << 24) + (Uint32(g) << 16) + (Uint32(b) << 8) + Uint32(a);
}

Uint32 RGB(Uint8 r, Uint8 g, Uint8 b) {
    return RGBA(r, g, b, SDL_ALPHA_OPAQUE);
}

Uint32 RGBA(float r, float g, float b, float a) {
    return RGBA(Uint8(255 * r), Uint8(255 * g), Uint8(255 * b), Uint8(255 * a));
}

Uint32 RGB(float r, float g, float b) {
    return RGB(Uint8(255 * r), Uint8(255 * g), Uint8(255 * b));
}

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
private:
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

int main(int argc, char ** argv) {
    std::vector<std::string> positionals;
    for (int i = 1; i < argc; ++i) {
        std::string param(argv[i]);
        if (param[0] == '-') {
            if (param == "-help") {
                usage();
                return 0;
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

        Mapper_000 mapper(rom);

        PpuMemoryMap<Palette> ppumap(nullptr, &mapper);
        Ppu ppu(&ppumap);

        CpuMemoryMap<Cpu, Ppu> cpumap(nullptr, &ppu, &mapper);
        Cpu cpu("6502", &cpumap);
        cpumap.CPU = &cpu;

        cpu.Reset();

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
                else if (line[0] == 'f') {
                    step = 29781 * std::stoll(line.substr(1));
                }
                else if (line == "bg") {
                    std::vector<Uint32> p(256 * 240, 0);
                    std::cout << "Background" << std::endl;
                    for (auto ty = 0; ty < 30; ++ty) {
                        for (auto tx = 0; tx < 32; tx++) {
                            auto td = ppumap.Vram[32 * ty + tx];
                            for (auto yy = 0; yy < 8; yy++) {
                                for (auto xx = 0; xx < 8; xx++) {
                                    auto x = 8 * tx + xx;
                                    auto y = 8 * ty + yy;
                                    auto b = ppumap.GetByteAt(ppu.BackgroundTable + 16 * td + yy);
                                    Uint8 v = (b >> (7 - xx)) & 0x01;
                                    b = ppumap.GetByteAt(ppu.BackgroundTable + 16 * td + yy + 8);
                                    v += ((b >> (7 - xx)) & 0x01) << 1;
                                    p[256 * y + x] = RGB(v / 3.0f, v / 3.0f, v / 3.0f);
                                }
                            }
                        }
                    }
                    SDL::SetScale(2);
                    SDL::Show(256, 240, p.data());
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
    catch (const std::exception & e) {
        log("Exception: " + std::string(e.what()));
        throw e;
    }

    return 0;
}
