#include "PageManager.h"


PageManager::PageManager(LedRenderer& led) : m_led(led) {}


void PageManager::addPage(std::string name, PadMap padMap) {
    m_names.push_back(std::move(name));
    m_pages.push_back(std::move(padMap));
}


void PageManager::switchPage(int index) {
    if (m_pages.empty()) return;
    m_currentIdx = index % static_cast<int>(m_pages.size());
    refreshLEDs();
    if (m_onPageChange) m_onPageChange(m_names[m_currentIdx]);
}

void PageManager::nextPage() {
    if (m_pages.empty()) return;
    switchPage((m_currentIdx + 1) % static_cast<int>(m_pages.size()));
}

void PageManager::prevPage() {
    if (m_pages.empty()) return;
    switchPage((m_currentIdx - 1 + static_cast<int>(m_pages.size())) % static_cast<int>(m_pages.size()));
}


void PageManager::handlePadPress(uint8_t note) {
    if (m_pages.empty()) return;

    const PadMap& padMap = m_pages[m_currentIdx];
    auto it = padMap.find(note);
    if (it == padMap.end()) return;

    const PadConfig& pad = it->second;

    if (pad.isToggle) {
        bool& state = m_toggleStates[toggleKey(m_currentIdx, note)];
        state = !state;
        m_led.setPad(note, state ? pad.colorOn : pad.colorOff);
    }

    if (pad.onPress) pad.onPress();
}


void PageManager::refreshLEDs() {
    if (m_pages.empty()) return;

    const PadMap& current = m_pages[m_currentIdx];
    std::unordered_map<uint32_t, uint8_t> ledMap;
    ledMap.reserve(current.size());

    for (const auto& [note, pad] : current) {
        if (pad.isToggle) {
            auto it     = m_toggleStates.find(toggleKey(m_currentIdx, static_cast<uint8_t>(note)));
            bool state  = (it != m_toggleStates.end()) ? it->second : false;
            ledMap[note] = state ? pad.colorOn : pad.colorOff;
        } else {
            ledMap[note] = pad.color;
        }
    }

    m_led.refreshAll(ledMap);
}
