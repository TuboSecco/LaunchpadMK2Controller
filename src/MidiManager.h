#pragma once
#include <Windows.h>
#include <mmsystem.h>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include "Types.h"

#pragma comment(lib, "winmm.lib")


class MidiManager {
public:
    using Callback = std::function<void(const MidiMessage&)>;

    MidiManager()  = default;
    ~MidiManager() { close(); }

    static std::vector<std::string> listInputs();
    static std::vector<std::string> listOutputs();

    // Partial name match against device names (e.g. "Launchpad")
    bool openInput (const std::string& nameFragment);
    bool openOutput(const std::string& nameFragment);
    void close();

    bool isInputOpen()  const { return m_hIn  != nullptr; }
    bool isOutputOpen() const { return m_hOut != nullptr; }

    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendCC    (uint8_t channel, uint8_t cc,   uint8_t value);
    void sendSysEx (const std::vector<uint8_t>& data);

    void setCallback(Callback cb) { m_callback = std::move(cb); }

    // Drain the queue and fire the callback for each message. Call at ~100 Hz.
    void poll();

private:
    HMIDIIN  m_hIn  = nullptr;
    HMIDIOUT m_hOut = nullptr;

    Callback                m_callback;
    std::mutex              m_queueMutex;
    std::queue<MidiMessage> m_queue;

    // WinMM delivers input on its own thread; only enqueue here.
    static void CALLBACK winmmCallback(
        HMIDIIN, UINT wMsg,
        DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2);

    void enqueue(MidiMessage msg);
};
