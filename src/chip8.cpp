#include "chip8.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace {
constexpr std::array<uint8_t, 80> kFont = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

[[noreturn]] void badOpcode(uint16_t op, uint16_t pc) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "unknown opcode %04X at %03X", op, pc);
    throw std::runtime_error(buf);
}
}  // namespace

Chip8::Chip8(Quirks q) : q_(q) { reset(); }

void Chip8::reset() {
    mem_.fill(0);
    v_.fill(0);
    stack_.fill(0);
    keys_.fill(false);
    fb_.fill(0);
    i_ = 0;
    pc_ = kRomStart;
    sp_ = delayTimer_ = soundTimer_ = 0;
    waitingKey_.reset();
    std::copy(kFont.begin(), kFont.end(), mem_.begin() + kFontBase);
}

void Chip8::load(const std::vector<uint8_t>& bytes) {
    if (bytes.size() > mem_.size() - kRomStart)
        throw std::runtime_error("ROM too large (max 3584 bytes)");
    std::copy(bytes.begin(), bytes.end(), mem_.begin() + kRomStart);
}

bool Chip8::loadRom(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)),
                               std::istreambuf_iterator<char>());
    load(data);
    return true;
}

void Chip8::tickTimers() {
    if (delayTimer_) --delayTimer_;
    if (soundTimer_) --soundTimer_;
}

void Chip8::drawSprite(uint8_t xr, uint8_t yr, uint8_t rows) {
    // Starting coordinate wraps; pixels running past an edge are clipped.
    const int px = v_[xr] % kScreenW;
    const int py = v_[yr] % kScreenH;
    uint8_t collision = 0;

    for (int r = 0; r < rows; ++r) {
        if (py + r >= kScreenH) break;
        const uint8_t bits = mem_[(i_ + r) & 0x0FFF];
        for (int c = 0; c < 8; ++c) {
            if (px + c >= kScreenW) break;
            if (bits & (0x80 >> c)) {
                uint8_t& pix = fb_[(py + r) * kScreenW + (px + c)];
                collision |= pix;
                pix ^= 1;
            }
        }
    }
    v_[0xF] = collision;
}

void Chip8::step() {
    const uint16_t at = pc_;
    const uint16_t op = (mem_[pc_ & 0x0FFF] << 8) | mem_[(pc_ + 1) & 0x0FFF];
    pc_ = (pc_ + 2) & 0x0FFF;

    const uint8_t  x   = (op >> 8) & 0xF;
    const uint8_t  y   = (op >> 4) & 0xF;
    const uint8_t  n   = op & 0xF;
    const uint8_t  kk  = op & 0xFF;
    const uint16_t nnn = op & 0x0FFF;

    switch (op >> 12) {
    case 0x0:
        if (op == 0x00E0)      fb_.fill(0);
        else if (op == 0x00EE) {
            if (sp_ == 0) throw std::runtime_error("stack underflow");
            pc_ = stack_[--sp_];
        } else badOpcode(op, at);
        break;

    case 0x1: pc_ = nnn; break;

    case 0x2:
        if (sp_ >= stack_.size()) throw std::runtime_error("stack overflow");
        stack_[sp_++] = pc_;
        pc_ = nnn;
        break;

    case 0x3: if (v_[x] == kk)    skip(); break;
    case 0x4: if (v_[x] != kk)    skip(); break;
    case 0x5: if (v_[x] == v_[y]) skip(); break;
    case 0x6: v_[x] = kk; break;
    case 0x7: v_[x] = static_cast<uint8_t>(v_[x] + kk); break;

    case 0x8:
        switch (n) {
        case 0x0: v_[x] = v_[y]; break;
        case 0x1: v_[x] |= v_[y]; if (q_.logicResetsVF) v_[0xF] = 0; break;
        case 0x2: v_[x] &= v_[y]; if (q_.logicResetsVF) v_[0xF] = 0; break;
        case 0x3: v_[x] ^= v_[y]; if (q_.logicResetsVF) v_[0xF] = 0; break;
        case 0x4: {
            const unsigned sum = v_[x] + v_[y];
            v_[x] = static_cast<uint8_t>(sum);
            v_[0xF] = sum > 0xFF;
        } break;
        case 0x5: {
            const uint8_t flag = v_[x] >= v_[y];
            v_[x] = static_cast<uint8_t>(v_[x] - v_[y]);
            v_[0xF] = flag;
        } break;
        case 0x6: {
            const uint8_t src  = q_.shiftUsesVy ? v_[y] : v_[x];
            const uint8_t flag = src & 1;
            v_[x] = src >> 1;
            v_[0xF] = flag;
        } break;
        case 0x7: {
            const uint8_t flag = v_[y] >= v_[x];
            v_[x] = static_cast<uint8_t>(v_[y] - v_[x]);
            v_[0xF] = flag;
        } break;
        case 0xE: {
            const uint8_t src  = q_.shiftUsesVy ? v_[y] : v_[x];
            const uint8_t flag = src >> 7;
            v_[x] = static_cast<uint8_t>(src << 1);
            v_[0xF] = flag;
        } break;
        default: badOpcode(op, at);
        }
        break;

    case 0x9: if (v_[x] != v_[y]) skip(); break;
    case 0xA: i_ = nnn; break;
    case 0xB: pc_ = (nnn + (q_.jumpUsesVx ? v_[x] : v_[0])) & 0x0FFF; break;
    case 0xC: v_[x] = static_cast<uint8_t>(rng_() & kk); break;
    case 0xD: drawSprite(x, y, n); break;

    case 0xE:
        if (kk == 0x9E)      { if (keys_[v_[x] & 0xF])  skip(); }
        else if (kk == 0xA1) { if (!keys_[v_[x] & 0xF]) skip(); }
        else badOpcode(op, at);
        break;

    case 0xF:
        switch (kk) {
        case 0x07: v_[x] = delayTimer_; break;
        case 0x0A:
            // Phase 1: latch the first key seen down. Phase 2: wait for its release.
            if (!waitingKey_) {
                for (uint8_t k = 0; k < 16; ++k)
                    if (keys_[k]) { waitingKey_ = k; break; }
            }
            if (!waitingKey_ || keys_[*waitingKey_]) {
                pc_ = (pc_ - 2) & 0x0FFF;   // repeat this instruction next step
            } else {
                v_[x] = *waitingKey_;
                waitingKey_.reset();
            }
            break;
        case 0x15: delayTimer_ = v_[x]; break;
        case 0x18: soundTimer_ = v_[x]; break;
        case 0x1E: i_ = static_cast<uint16_t>(i_ + v_[x]); break;
        case 0x29: i_ = kFontBase + (v_[x] & 0xF) * 5; break;
        case 0x33: {
            const uint8_t val = v_[x];
            mem_[(i_    ) & 0x0FFF] = val / 100;
            mem_[(i_ + 1) & 0x0FFF] = (val / 10) % 10;
            mem_[(i_ + 2) & 0x0FFF] = val % 10;
        } break;
        case 0x55:
            for (int k = 0; k <= x; ++k) mem_[(i_ + k) & 0x0FFF] = v_[k];
            if (q_.memoryIncrementsI) i_ = static_cast<uint16_t>(i_ + x + 1);
            break;
        case 0x65:
            for (int k = 0; k <= x; ++k) v_[k] = mem_[(i_ + k) & 0x0FFF];
            if (q_.memoryIncrementsI) i_ = static_cast<uint16_t>(i_ + x + 1);
            break;
        default: badOpcode(op, at);
        }
        break;
    }
}
