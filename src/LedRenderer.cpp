#include "LedRenderer.h"


LedRenderer::LedRenderer(MidiManager& midi) : m_midi(midi) {}


void LedRenderer::setPadInternal(uint8_t note, uint8_t color) {
    if (note >= LP2::TOP_FIRST_CC && note <= LP2::TOP_LAST_CC)
        m_midi.sendCC(LED_CHANNEL, note, color);
    else
        m_midi.sendNoteOn(LED_CHANNEL, note, color);
}


void LedRenderer::setPad(uint8_t note, uint8_t color) {
    std::lock_guard<std::mutex> lock(m_mutex);
    setPadInternal(note, color);
}

void LedRenderer::clearAll() {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (uint8_t row = 1; row <= LP2::ROWS; ++row)
        for (uint8_t col = 1; col <= LP2::COLS; ++col)
            setPadInternal(LP2::Note(row, col), LP2::Color::OFF);

    for (uint8_t cc = LP2::TOP_FIRST_CC; cc <= LP2::TOP_LAST_CC; ++cc)
        setPadInternal(cc, LP2::Color::OFF);
}

void LedRenderer::refreshAll(const std::unordered_map<uint32_t, uint8_t>& ledMap) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (uint8_t row = 1; row <= LP2::ROWS; ++row) {
        for (uint8_t col = 1; col <= LP2::COLS; ++col) {
            uint8_t note  = LP2::Note(row, col);
            auto    it    = ledMap.find(note);
            uint8_t color = (it != ledMap.end()) ? it->second : LP2::Color::OFF;
            setPadInternal(note, color);
        }
    }

    for (uint8_t cc = LP2::TOP_FIRST_CC; cc <= LP2::TOP_LAST_CC; ++cc) {
        auto    it    = ledMap.find(cc);
        uint8_t color = (it != ledMap.end()) ? it->second : LP2::Color::OFF;
        setPadInternal(cc, color);
    }
}
