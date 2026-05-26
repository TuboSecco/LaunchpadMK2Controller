#pragma once
#include <string>
#include "PageManager.h"


class ConfigLoader {
public:
    // Parse config.json and register all pages with the PageManager.
    // Writes a starter config if the file does not exist.
    // Returns false on unrecoverable parse error.
    static bool load(const std::string& path, PageManager& pm);

private:
    static void writeDefaultConfig(const std::string& path);

    // Translate a key-name string ("CTRL", "F12", "A", …) to a VK code.
    // Returns 0 for unknown names. Must stay in sync with KEY_LIST in
    // launchpad-configurator.html.
    static WORD keyNameToVK(const std::string& name);
};
