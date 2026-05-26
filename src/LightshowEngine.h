#pragma once
#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include "LedRenderer.h"
#include "Types.h"

// Radial Ember effect — runs on a dedicated thread.
// Active only when the current page name equals LP2::LIGHTSHOW_PAGE_NAME.
//
// Two concentric wave sources interfere to produce softly deforming rings:
//   Source A — fixed at the geometric centre (3.5, 3.5).
//   Source B — orbits A at ORBIT_RADIUS, advancing ORBIT_SPEED per frame.
//
//   raw        = sin(dA * WAVE_A_FREQ − t) * 0.70
//              + sin(dB * WAVE_B_FREQ − t * WAVE_B_SPEED) * 0.30
//   normalized = ((raw + 1) / 2) ^ GAMMA   →  [0, 1]
//   level      = clamp(floor(normalized * LEVEL_COUNT), 0, LEVEL_COUNT−1)
//
// GAMMA > 1 biases the palette toward the darker (red/orange) end.

class LightshowEngine {
public:
    explicit LightshowEngine(LedRenderer& led);
    ~LightshowEngine();

    void onPageChange(const std::string& pageName);

private:
    void run();
    void effectRadialEmber();

    static constexpr int ROWS = LP2::ROWS;
    static constexpr int COLS = 8;

    static constexpr float BASE_SPEED   = 0.025f;
    static constexpr float WAVE_A_FREQ  = 0.45f;
    static constexpr float WAVE_B_FREQ  = 0.38f;
    static constexpr float WAVE_B_SPEED = 0.75f;
    static constexpr float ORBIT_RADIUS = 1.8f;
    static constexpr float ORBIT_SPEED  = 0.007f;
    static constexpr float GAMMA        = 1.8f;

    static constexpr int TICK_MS = 50; // ~20 FPS

    static constexpr uint8_t LEVELS[] = {
        LP2::Color::RED,
        LP2::Color::ORANGE,
        LP2::Color::AMBER,
        LP2::Color::YELLOW,
    };
    static constexpr int LEVEL_COUNT = 4;

    LedRenderer&      m_led;
    std::atomic<bool> m_running{ true  };
    std::atomic<bool> m_active { false };
    std::thread       m_thread;
    int               m_frame = 0;
};
