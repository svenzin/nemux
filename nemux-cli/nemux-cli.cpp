#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <exception>
#include <sstream>
#include <iomanip>

#include "NesFile.h"
#include "Mapper_0.h"
#include "Cpu.h"

void usage() {
    std::cout << "NeMux emulator command-line executable" << std::endl;
    std::cout << "Usage: nemux-cli [options] nesfile" << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "    nesfile    Path the the NES ROM file" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "    -nesfile_info    Prints information about the NES ROM file" << std::endl;
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
            } else if (param == "-nesfile_info") {
                options.insert(Options::NesFile_Info);
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
        else {
            Mapper_0 mapper(rom);
            Ppu ppu;
            CpuMemoryMap<Ppu> cpumap(&ppu, &mapper);
            Cpu cpu("6502", &cpumap);
//            cpu.Reset();
            cpu.PC = 0xC000;

            std::string line;
            std::cout << cpu.ToMiniString() << " ";
            while (std::getline(std::cin, line) && line.empty()) {
                cpu.Tick();
                while (cpu.CurrentTick < cpu.Ticks) {
                    cpu.Tick();
                }
                std::cout << cpu.ToMiniString() << " ";
            }
        }
    }
    catch (const std::exception & e) {
        log("Exception: " + std::string(e.what()));
        throw e;
    }

    return 0;
}
