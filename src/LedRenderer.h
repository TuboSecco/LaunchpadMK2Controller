#pragma once
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include "MidiManager.h"
#include "Types.h"

// Thread-safe LED control for Launchpad MK2.
// Grid pads  → Note On ch1 (0x90), velocity = color index
// Top-row    → CC ch1 (0xB0), CC# 104–111, value = color index

class LedRenderer {
public:
    explicit LedRenderer(MidiManager& midi);

    void setPad(uint8_t note, uint8_t color);
    void clearAll();

    // Apply a full LED snapshot. Pads absent from ledMap are turned off.
    void refreshAll(const std::unordered_map<uint32_t, uint8_t>& ledMap);

private:
    MidiManager& m_midi;
    std::mutex   m_mutex;

    static constexpr uint8_t LED_CHANNEL = 0; // ch1, static palette

    void setPadInternal(uint8_t note, uint8_t color); // requires m_mutex held
};
