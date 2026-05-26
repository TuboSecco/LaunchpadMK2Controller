#pragma once
#include <Windows.h>
#include <shellapi.h>
#include <functional>
#include <initializer_list>
#include <string>
#include <vector>

#pragma comment(lib, "Shell32.lib")


namespace Actions {

inline void fireKeys(const std::vector<WORD>& keys) {
    if (keys.empty()) return;
    std::vector<INPUT> inputs;
    inputs.reserve(keys.size() * 2);
    for (WORD vk : keys) {
        INPUT in{}; in.type = INPUT_KEYBOARD; in.ki.wVk = vk;
        inputs.push_back(in);
    }
    for (auto it = keys.rbegin(); it != keys.rend(); ++it) {
        INPUT in{}; in.type = INPUT_KEYBOARD; in.ki.wVk = *it;
        in.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(in);
    }
    SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
}

// Compile-time key list (hardcoded pages)
inline std::function<void()> sendKeys(std::initializer_list<WORD> vkeys) {
    std::vector<WORD> keys(vkeys);
    return [keys]() { fireKeys(keys); };
}

// Runtime key list (ConfigLoader)
inline std::function<void()> sendKeysVec(std::vector<WORD> keys) {
    return [keys]() { fireKeys(keys); };
}

inline std::function<void()> launchApp(
    std::string path,
    std::string args       = "",
    std::string workingDir = "")
{
    return [path, args, workingDir]() {
        ShellExecuteA(nullptr, "open", path.c_str(),
            args.empty()       ? nullptr : args.c_str(),
            workingDir.empty() ? nullptr : workingDir.c_str(),
            SW_SHOWNORMAL);
    };
}

inline std::function<void()> compound(
    std::initializer_list<std::function<void()>> actions)
{
    std::vector<std::function<void()>> fns(actions);
    return [fns]() { for (auto& fn : fns) if (fn) fn(); };
}

inline std::function<void()> noop() { return []() {}; }

} // namespace Actions
