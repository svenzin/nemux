#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <exception>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "NesFile.h"
#include "Mapper_0.h"
#include "Cpu.h"
#include "Ppu.h"

using std::hex;
using std::dec;
using std::setfill;
using std::setw;

void usage() {
    std::cout << "NeMux emulator command-line executable" << std::endl;
    std::cout << "Usage: nemux-cli [options] nesfile" << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "    nesfile    Path the the NES ROM file" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    -nesfile_info    Prints information about the NES ROM file" << std::endl;
    std::cout << "    -test            Run a test rom where the results are copmpared against an expected output" << std::endl;
    std::cout << "    -test_log LOGFILE         nestest-formatted log to test against" << std::endl;
    std::cout << "    -test_start_at ADDRESS    Address to start execution at" << std::endl;
    std::cout << "                              This will skip the reset sequence" << std::endl;
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
    NesFile_Info,
    Test, Test_Log, Test_StartAt
};

struct state_t {
    Byte A;
    Byte X;
    Byte Y;
    Byte SP;
    Word PC;
    Byte P;
};
state_t ParseLog(std::string line) {
    state_t state;
    state.PC = std::stoul(line.substr(0, 4), 0, 16);
    state.SP = std::stoul(line.substr(71, 2), 0, 16);
    state.A = std::stoul(line.substr(50, 2), 0, 16);
    state.X = std::stoul(line.substr(55, 2), 0, 16);
    state.Y = std::stoul(line.substr(60, 2), 0, 16);
    state.P = std::stoul(line.substr(65, 2), 0, 16);
    return state;
}

int main(int argc, char ** argv) {
    std::vector<std::string> positionals;
    std::set<Options> options;
    std::map<Options, std::string> values;
    for (int i = 1; i < argc; ++i) {
        std::string param(argv[i]);
        if (param[0] == '-') {
            if (param == "-help") {
                usage();
                return 0;
            } else if (param == "-nesfile_info") {
                options.insert(Options::NesFile_Info);
            } else if (param == "-test") {
                options.insert(Options::Test);
            } else if (param == "-test_log") {
                options.insert(Options::Test_Log);
                if (++i == argc) {
                    error("-test_log requires a parameter");
                    return 1;
                }
                std::string value(argv[i]);
                values[Options::Test_Log] = value;
            } else if (param == "-test_start_at") {
                options.insert(Options::Test_StartAt);
                if (++i == argc) {
                    error("-test_start_at requires a parameter");
                    return 1;
                }
                std::string value(argv[i]);
                values[Options::Test_StartAt] = value;
            } else {
                error("Unrecognized parameter: " + param);
                return 1;
            }
        } else {
            positionals.push_back(param);
        }
    }

    auto IsSet = [&options] (const Options & opt) { return options.count(opt) == 1; };

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

        if (IsSet(Options::NesFile_Info)) {
            const auto size_prg = 0x4000 * rom.Header.PrgRomPages;
            const auto size_chr = 0x2000 * rom.Header.ChrRomPages;

            std::ostringstream oss;
            oss << "NES ROM file information" << std::endl
                << "Header" << std::endl
                << "    NES 2 format  " << std::boolalpha << rom.Header.IsNES2Format << std::endl
                << "    PC-10 ROM     " << std::boolalpha << rom.Header.IsPlaychoice10 << std::endl
                << "    Versus system " << std::boolalpha << rom.Header.IsVsUnisystem << std::endl
                << "    Battery       " << std::boolalpha << rom.Header.HasBattery << std::endl
                << "    Trainer       " << std::boolalpha << rom.Header.HasTrainer << std::endl
                << "    Mirroring     " << rom.Header.ScreenMode << std::endl
                << "    Mapper        " << rom.Header.MapperNumber << std::endl
                << "    PRG-ROM       " << static_cast<int>(rom.Header.PrgRomPages) << " page(s)" << std::endl
                << "    CHR-ROM       " << static_cast<int>(rom.Header.ChrRomPages) << " page(s)" << std::endl;
                log(oss.str());
        }
        else if (IsSet(Options::Test)) {
            std::pair<bool, std::ifstream> logfile { false, std::ifstream() };
            if (IsSet(Options::Test_Log)) {
                logfile.first = true;
                logfile.second.open(values[Options::Test_Log]);
            }

            std::pair<bool, int> start_addr { false, 0 };
            if (IsSet(Options::Test_StartAt)) {
                start_addr.first = true;
                start_addr.second = std::stoi(values[Options::Test_StartAt], nullptr, 0);
            }
            
            Mapper_000 mapper(rom);
            
            PpuMemoryMap<Palette> ppumap(nullptr, &mapper);
            Ppu ppu(&ppumap);
            
            CpuMemoryMap<Cpu, Ppu> cpumap(nullptr, &ppu, &mapper);
            Cpu cpu("6502", &cpumap);
            cpumap.CPU = &cpu;
            
            if (start_addr.first) {
                cpu.PC = start_addr.second;
                cpu.B = 0;
            } else {
                cpu.Reset();
            }

            long long step = 0;
            std::size_t counter = 1;
            std::string line;
            std::getline(logfile.second, line);
            while (true) {
                if (step == 0) {
                    std::cout << "> ";
                    std::getline(std::cin, line);
                    if (line.empty()) step = 1;
                    else if (line == "r") step = -1;
                    else if (line == "nt") {
                        const Byte * nt = &ppumap.Vram[0];
                        std::cout << "Nametable 0 @0x2000" << std::endl;
                        for (auto y = 0; y < 30; ++y) {
                            for (auto x = 0; x < 32; x++) {
                                std::cout << hex << setfill('0') << setw(2) << Word{ *nt } << ' ';
                                ++nt;
                            }
                            std::cout << std::endl;
                        }
                    }
                    else if (line == "at") {
                        const Byte * at = &ppumap.Vram[0x3C0];
                        std::cout << "Attribute Table 0 @0x23C0" << std::endl;
                        for (auto y = 0; y < 8; ++y) {
                            for (auto x = 0; x < 8; x++) {
                                std::cout << hex << setfill('0') << setw(2) << Word{ *at } << ' ';
                                ++at;
                            }
                            std::cout << std::endl;
                        }
                    }
                    else {
                        try {
                            step = std::stoll(line);
                        }
                        catch (const std::exception & e) {
                            break;
                        }
                    }
                } else {
                    //std::cout << std::endl;
                }

                if (step > 0) {
                    --step;
                    ++counter;

                    cpu.Tick(); ppu.Tick(); ppu.Tick(); ppu.Tick();
                    while (cpu.CurrentTick < cpu.Ticks) {
                        cpu.Tick(); ppu.Tick(); ppu.Tick(); ppu.Tick();
                    }
                }

                if (step == 0) {
                    std::cout << dec << counter << " " << cpu.ToMiniString() << " ";
                    if (logfile.first) {
                        std::getline(logfile.second, line);
                        const auto state = ParseLog(line);
                        if (cpu.PC != state.PC ||
                            cpu.SP != state.SP ||
                            cpu.A != state.A ||
                            cpu.X != state.X ||
                            cpu.Y != state.Y ||
                            (cpu.GetStatus() & ~Mask<Brk>(1)) != (state.P & ~Mask<Brk>(1))) {
                            std::cout << "DIFF"
                                << " PC=$" << hex << setfill('0') << setw(4) << state.PC
                                << " S=$" << hex << setfill('0') << setw(2) << Word{ state.SP }
                                << " A=$" << hex << setfill('0') << setw(2) << Word{ state.A }
                                << " X=$" << hex << setfill('0') << setw(2) << Word{ state.X }
                                << " Y=$" << hex << setfill('0') << setw(2) << Word{ state.Y }
                                << " P=$" << hex << setfill('0') << setw(2) << Word{ state.P }
                            << " ";

                            if (step < 0) step = 0;
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception & e) {
        log("Exception: " + std::string(e.what()));
        throw e;
    }

    return 0;
}
