#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include "Types.h"
#include "LedRenderer.h"


class PageManager {
public:
    explicit PageManager(LedRenderer& led);

    // Called once per page during startup (by ConfigLoader)
    void addPage(std::string name, PadMap padMap);

    int                pageCount()    const { return static_cast<int>(m_names.size()); }
    int                currentIndex() const { return m_currentIdx; }
    const std::string& currentName()  const { return m_names[m_currentIdx]; }

    void switchPage(int index);   // clamps to valid range
    void nextPage();
    void prevPage();

    void handlePadPress(uint8_t note);
    void refreshLEDs();

    using PageChangeCallback = std::function<void(const std::string& newPageName)>;
    void setPageChangeCallback(PageChangeCallback cb) { m_onPageChange = std::move(cb); }

private:
    LedRenderer& m_led;
    int          m_currentIdx = 0;

    std::vector<std::string> m_names;
    std::vector<PadMap>      m_pages;

    // Key: (page_index << 8) | note — missing entry implies false/OFF
    std::unordered_map<uint32_t, bool> m_toggleStates;

    PageChangeCallback m_onPageChange;

    static uint32_t toggleKey(int pageIdx, uint8_t note) {
        return (static_cast<uint32_t>(pageIdx) << 8) | note;
    }
};
