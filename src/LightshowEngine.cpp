#include "LightshowEngine.h"
#include <chrono>
#include <cmath>


constexpr uint8_t LightshowEngine::LEVELS[];


LightshowEngine::LightshowEngine(LedRenderer& led) : m_led(led) {
    m_thread = std::thread(&LightshowEngine::run, this);
}

LightshowEngine::~LightshowEngine() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

void LightshowEngine::onPageChange(const std::string& pageName) {
    m_active = (pageName == LP2::LIGHTSHOW_PAGE_NAME);
}


void LightshowEngine::run() {
    using namespace std::chrono_literals;
    while (m_running) {
        if (m_active) {
            effectRadialEmber();
            ++m_frame;
            std::this_thread::sleep_for(std::chrono::milliseconds(TICK_MS));
        } else {
            std::this_thread::sleep_for(20ms);
        }
    }
}


void LightshowEngine::effectRadialEmber() {
    const float t = static_cast<float>(m_frame) * BASE_SPEED;

    constexpr float ax = 3.5f;
    constexpr float ay = 3.5f;

    const float orbitAngle = static_cast<float>(m_frame) * ORBIT_SPEED;
    const float bx = ax + std::cos(orbitAngle) * ORBIT_RADIUS;
    const float by = ay + std::sin(orbitAngle) * ORBIT_RADIUS;

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            const float px = static_cast<float>(row);
            const float py = static_cast<float>(col);

            const float dA = std::sqrt((px - ax) * (px - ax) + (py - ay) * (py - ay));
            const float dB = std::sqrt((px - bx) * (px - bx) + (py - by) * (py - by));

            const float raw        = std::sin(dA * WAVE_A_FREQ - t) * 0.70f
                                   + std::sin(dB * WAVE_B_FREQ - t * WAVE_B_SPEED) * 0.30f;
            const float normalized = std::pow((raw + 1.0f) * 0.5f, GAMMA);

            int level = static_cast<int>(normalized * LEVEL_COUNT);
            level = (level >= LEVEL_COUNT) ? LEVEL_COUNT - 1 : (level < 0 ? 0 : level);

            m_led.setPad(LP2::Note(row + 1, col + 1), LEVELS[level]);
        }
    }
}
