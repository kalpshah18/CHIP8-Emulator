#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <random>
#include <string>
#include <vector>

constexpr int      kScreenW  = 64;
constexpr int      kScreenH  = 32;
constexpr uint16_t kRomStart = 0x200;
constexpr uint16_t kFontBase = 0x050;

// Behaviour differences between CHIP-8 variants. Defaults suit most
// modern ROMs; flip them to match older programs or a test ROM.
struct Quirks {
    bool shiftUsesVy      = false; // 8xy6/8xyE read Vy (original) vs Vx (CHIP-48/SCHIP)
    bool memoryIncrementsI = false; // Fx55/Fx65 leave I = I + x + 1 (original)
    bool logicResetsVF    = false; // 8xy1/8xy2/8xy3 zero VF (original)
    bool jumpUsesVx       = false; // Bnnn uses Vx where x = high nibble of nnn (SCHIP)
};

class Chip8 {
public:
    explicit Chip8(Quirks q = {});

    void reset();
    bool loadRom(const std::string& path);
    void load(const std::vector<uint8_t>& bytes);   // copy bytes to 0x200

    void step();         // execute one instruction
    void tickTimers();   // call exactly 60 times per second

    void setKey(uint8_t key, bool down) { keys_[key & 0xF] = down; }

    const std::array<uint8_t, kScreenW * kScreenH>& framebuffer() const { return fb_; }
    bool buzzing() const { return soundTimer_ > 0; }

    // Read-only state for tests and debugging.
    uint8_t  v(int i)   const { return v_[i & 0xF]; }
    uint16_t pc()       const { return pc_; }
    uint16_t index()    const { return i_; }
    uint8_t  delay()    const { return delayTimer_; }

private:
    void drawSprite(uint8_t x, uint8_t y, uint8_t rows);
    void skip() { pc_ = (pc_ + 2) & 0x0FFF; }

    Quirks q_;
    std::array<uint8_t, 4096>               mem_{};
    std::array<uint8_t, 16>                 v_{};
    std::array<uint16_t, 16>                stack_{};
    std::array<bool, 16>                    keys_{};
    std::array<uint8_t, kScreenW * kScreenH> fb_{};   // 0 = off, 1 = on
    uint16_t i_ = 0, pc_ = kRomStart;
    uint8_t  sp_ = 0, delayTimer_ = 0, soundTimer_ = 0;
    std::optional<uint8_t> waitingKey_;               // Fx0A state
    std::mt19937 rng_{std::random_device{}()};
};
