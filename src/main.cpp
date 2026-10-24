#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>

#include "chip8.hpp"
#include "platform.hpp"

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog << " <rom> [--scale N] [--ips N]\n"
              << "          [--shift-vy] [--inc-i] [--reset-vf] [--jump-vx]\n";
}

int main(int argc, char** argv) {
    if (argc < 2) { usage(argv[0]); return 1; }

    std::string romPath;
    int scale = 12, ips = 700;
    Quirks q;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--scale" && i + 1 < argc)      scale = std::max(1, std::atoi(argv[++i]));
        else if (a == "--ips" && i + 1 < argc)   ips   = std::max(60, std::atoi(argv[++i]));
        else if (a == "--shift-vy")              q.shiftUsesVy = true;
        else if (a == "--inc-i")                 q.memoryIncrementsI = true;
        else if (a == "--reset-vf")              q.logicResetsVF = true;
        else if (a == "--jump-vx")               q.jumpUsesVx = true;
        else if (a[0] != '-')                    romPath = a;
        else { usage(argv[0]); return 1; }
    }
    if (romPath.empty()) { usage(argv[0]); return 1; }

    try {
        Chip8 cpu(q);
        if (!cpu.loadRom(romPath)) {
            std::cerr << "cannot open ROM: " << romPath << "\n";
            return 1;
        }
        Platform platform("CHIP-8", scale);

        constexpr double kFrameMs = 1000.0 / 60.0;
        const int perFrame = std::max(1, ips / 60);
        const Uint64 freq = SDL_GetPerformanceFrequency();
        Uint64 last = SDL_GetPerformanceCounter();
        double acc = 0.0;

        while (platform.pump(cpu)) {
            const Uint64 now = SDL_GetPerformanceCounter();
            acc += (now - last) * 1000.0 / static_cast<double>(freq);
            last = now;
            acc = std::min(acc, 250.0);   // drop backlog after a stall (window drag, suspend)

            while (acc >= kFrameMs) {
                for (int i = 0; i < perFrame; ++i) cpu.step();
                cpu.tickTimers();
                acc -= kFrameMs;
            }

            platform.present(cpu.framebuffer());
            platform.beep(cpu.buzzing());
            SDL_Delay(1);
        }
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
