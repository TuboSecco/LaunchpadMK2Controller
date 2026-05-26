#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

// Launchpad MK2 note layout (default session mode)
//
//  Top CC:  104 105 106 107 108 109 110 111
//           ---------------------------------
//  Row 8:   81  82  83  84  85  86  87  88 | 89
//  Row 7:   71  72  73  74  75  76  77  78 | 79
//  Row 6:   61  62  63  64  65  66  67  68 | 69
//  Row 5:   51  52  53  54  55  56  57  58 | 59
//  Row 4:   41  42  43  44  45  46  47  48 | 49
//  Row 3:   31  32  33  34  35  36  37  38 | 39
//  Row 2:   21  22  23  24  25  26  27  28 | 29
//  Row 1:   11  12  13  14  15  16  17  18 | 19
//
//  Grid pads : Note On ch1 (0x90), velocity = color index
//  Top row   : CC    ch1 (0xB0), CC# 104–111, value = color index

namespace LP2 {

    inline constexpr uint8_t Note(int row, int col) {
        return static_cast<uint8_t>(row * 10 + col);
    }

    namespace Color {
        constexpr uint8_t OFF     =  0;
        constexpr uint8_t WHITE   =  3;
        constexpr uint8_t RED     =  5;
        constexpr uint8_t RED_HI  =  7;
        constexpr uint8_t ORANGE  =  9;
        constexpr uint8_t AMBER   = 11;
        constexpr uint8_t YELLOW  = 13;
        constexpr uint8_t LIME    = 17;
        constexpr uint8_t GREEN   = 21;
        constexpr uint8_t TEAL    = 33;
        constexpr uint8_t CYAN    = 37;
        constexpr uint8_t SKY     = 41;
        constexpr uint8_t BLUE    = 45;
        constexpr uint8_t INDIGO  = 49;
        constexpr uint8_t PURPLE  = 53;
        constexpr uint8_t PINK    = 57;
    }

    inline uint8_t colorFromName(const std::string& name) {
        static const std::unordered_map<std::string, uint8_t> table = {
            { "OFF",    Color::OFF    }, { "WHITE",  Color::WHITE  },
            { "RED",    Color::RED    }, { "RED_HI", Color::RED_HI },
            { "ORANGE", Color::ORANGE }, { "AMBER",  Color::AMBER  },
            { "YELLOW", Color::YELLOW }, { "LIME",   Color::LIME   },
            { "GREEN",  Color::GREEN  }, { "TEAL",   Color::TEAL   },
            { "CYAN",   Color::CYAN   }, { "SKY",    Color::SKY    },
            { "BLUE",   Color::BLUE   }, { "INDIGO", Color::INDIGO },
            { "PURPLE", Color::PURPLE }, { "PINK",   Color::PINK   },
        };
        auto it = table.find(name);
        return it != table.end() ? it->second : Color::OFF;
    }

    constexpr int ROWS         = 8;
    constexpr int COLS         = 9;
    constexpr int TOP_FIRST_CC = 104;
    constexpr int TOP_LAST_CC  = 111;

    constexpr const char* LIGHTSHOW_PAGE_NAME = "LIGHTSHOW";

} // namespace LP2


struct MidiMessage {
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
};


struct PadConfig {
    uint8_t color = LP2::Color::OFF;
    std::function<void()> onPress;

    bool    isToggle = false;
    uint8_t colorOn  = LP2::Color::OFF;
    uint8_t colorOff = LP2::Color::OFF;
};


using PadMap = std::unordered_map<uint32_t, PadConfig>;
