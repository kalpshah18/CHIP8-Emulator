#pragma once
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "chip8.hpp"

class Platform {
public:
    Platform(const char* title, int scale) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
            throw std::runtime_error(SDL_GetError());

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");   // nearest-neighbour
        window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   kScreenW * scale, kScreenH * scale, SDL_WINDOW_RESIZABLE);
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!window_ || !renderer_) throw std::runtime_error(SDL_GetError());

        SDL_RenderSetLogicalSize(renderer_, kScreenW, kScreenH);
        SDL_RenderSetIntegerScale(renderer_, SDL_TRUE);
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_STREAMING, kScreenW, kScreenH);

        SDL_AudioSpec want{}, have{};
        want.freq = 48000;
        want.format = AUDIO_S16SYS;
        want.channels = 1;
        want.samples = 512;
        want.callback = &Platform::audioCallback;
        want.userdata = this;
        audio_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
        if (audio_) SDL_PauseAudioDevice(audio_, 0);   // silent failure is acceptable
    }

    ~Platform() {
        if (audio_)    SDL_CloseAudioDevice(audio_);
        if (texture_)  SDL_DestroyTexture(texture_);
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_)   SDL_DestroyWindow(window_);
        SDL_Quit();
    }

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;

    // Returns false when the user asked to quit.
    bool pump(Chip8& cpu) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) return false;
        }
        const Uint8* state = SDL_GetKeyboardState(nullptr);
        for (int k = 0; k < 16; ++k) cpu.setKey(k, state[kKeyMap[k]] != 0);
        return true;
    }

    void present(const std::array<uint8_t, kScreenW * kScreenH>& fb) {
        std::vector<uint32_t> pixels(fb.size());
        for (size_t i = 0; i < fb.size(); ++i)
            pixels[i] = fb[i] ? 0xFFFFFFFFu : 0xFF000000u;   // ARGB
        SDL_UpdateTexture(texture_, nullptr, pixels.data(), kScreenW * sizeof(uint32_t));
        SDL_RenderClear(renderer_);
        SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
        SDL_RenderPresent(renderer_);
    }

    void beep(bool on) { beep_.store(on, std::memory_order_relaxed); }

private:
    // Index = CHIP-8 key value, element = physical key (layout-independent scancodes).
    //   CHIP-8 keypad      Keyboard
    //   1 2 3 C            1 2 3 4
    //   4 5 6 D            Q W E R
    //   7 8 9 E            A S D F
    //   A 0 B F            Z X C V
    static constexpr SDL_Scancode kKeyMap[16] = {
        SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
        SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A,
        SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C,
        SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V};

    static void audioCallback(void* user, Uint8* stream, int len) {
        auto* self = static_cast<Platform*>(user);
        auto* out = reinterpret_cast<int16_t*>(stream);
        const int count = len / static_cast<int>(sizeof(int16_t));
        const bool on = self->beep_.load(std::memory_order_relaxed);
        for (int i = 0; i < count; ++i) {
            out[i] = on ? (self->phase_ < 0.5 ? 2500 : -2500) : 0;   // square wave
            self->phase_ += 440.0 / 48000.0;
            if (self->phase_ >= 1.0) self->phase_ -= 1.0;
        }
    }

    SDL_Window*   window_   = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture*  texture_  = nullptr;
    SDL_AudioDeviceID audio_ = 0;
    std::atomic<bool> beep_{false};
    double phase_ = 0.0;
};
