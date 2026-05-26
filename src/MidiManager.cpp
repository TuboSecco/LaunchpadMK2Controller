#include "MidiManager.h"
#include <algorithm>


std::vector<std::string> MidiManager::listInputs() {
    UINT n = midiInGetNumDevs();
    std::vector<std::string> result;
    result.reserve(n);
    for (UINT i = 0; i < n; ++i) {
        MIDIINCAPSA caps{};
        if (midiInGetDevCapsA(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
            result.emplace_back(caps.szPname);
    }
    return result;
}

std::vector<std::string> MidiManager::listOutputs() {
    UINT n = midiOutGetNumDevs();
    std::vector<std::string> result;
    result.reserve(n);
    for (UINT i = 0; i < n; ++i) {
        MIDIOUTCAPSA caps{};
        if (midiOutGetDevCapsA(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
            result.emplace_back(caps.szPname);
    }
    return result;
}


bool MidiManager::openInput(const std::string& nameFragment) {
    UINT n = midiInGetNumDevs();
    for (UINT i = 0; i < n; ++i) {
        MIDIINCAPSA caps{};
        if (midiInGetDevCapsA(i, &caps, sizeof(caps)) != MMSYSERR_NOERROR)
            continue;
        if (std::string(caps.szPname).find(nameFragment) == std::string::npos)
            continue;

        MMRESULT r = midiInOpen(
            &m_hIn, i,
            reinterpret_cast<DWORD_PTR>(winmmCallback),
            reinterpret_cast<DWORD_PTR>(this),
            CALLBACK_FUNCTION);

        if (r == MMSYSERR_NOERROR) {
            midiInStart(m_hIn);
            return true;
        }
        return false;
    }
    return false;
}

bool MidiManager::openOutput(const std::string& nameFragment) {
    UINT n = midiOutGetNumDevs();
    for (UINT i = 0; i < n; ++i) {
        MIDIOUTCAPSA caps{};
        if (midiOutGetDevCapsA(i, &caps, sizeof(caps)) != MMSYSERR_NOERROR)
            continue;
        if (std::string(caps.szPname).find(nameFragment) == std::string::npos)
            continue;

        MMRESULT r = midiOutOpen(&m_hOut, i, 0, 0, CALLBACK_NULL);
        if (r == MMSYSERR_NOERROR) return true;
        return false;
    }
    return false;
}

void MidiManager::close() {
    if (m_hIn) {
        midiInStop(m_hIn);
        midiInReset(m_hIn);
        midiInClose(m_hIn);
        m_hIn = nullptr;
    }
    if (m_hOut) {
        midiOutReset(m_hOut);
        midiOutClose(m_hOut);
        m_hOut = nullptr;
    }
}


void MidiManager::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!m_hOut) return;
    midiOutShortMsg(m_hOut, static_cast<DWORD>(
        (0x90 | (channel & 0x0F)) | (note << 8) | (velocity << 16)));
}

void MidiManager::sendCC(uint8_t channel, uint8_t cc, uint8_t value) {
    if (!m_hOut) return;
    midiOutShortMsg(m_hOut, static_cast<DWORD>(
        (0xB0 | (channel & 0x0F)) | (cc << 8) | (value << 16)));
}

void MidiManager::sendSysEx(const std::vector<uint8_t>& data) {
    if (!m_hOut || data.empty()) return;

    MIDIHDR hdr{};
    hdr.lpData         = reinterpret_cast<LPSTR>(const_cast<uint8_t*>(data.data()));
    hdr.dwBufferLength = static_cast<DWORD>(data.size());

    if (midiOutPrepareHeader(m_hOut, &hdr, sizeof(hdr)) != MMSYSERR_NOERROR) return;
    midiOutLongMsg(m_hOut, &hdr, sizeof(hdr));
    while (!(hdr.dwFlags & MHDR_DONE)) Sleep(0);
    midiOutUnprepareHeader(m_hOut, &hdr, sizeof(hdr));
}


void MidiManager::poll() {
    std::queue<MidiMessage> local;
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        std::swap(local, m_queue);
    }
    while (!local.empty()) {
        if (m_callback) m_callback(local.front());
        local.pop();
    }
}

void MidiManager::enqueue(MidiMessage msg) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.push(msg);
}


void CALLBACK MidiManager::winmmCallback(
    HMIDIIN, UINT wMsg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR)
{
    if (wMsg != MIM_DATA) return;

    MidiMessage msg;
    msg.status = static_cast<uint8_t>( param1        & 0xFF);
    msg.data1  = static_cast<uint8_t>((param1 >>  8) & 0xFF);
    msg.data2  = static_cast<uint8_t>((param1 >> 16) & 0xFF);

    reinterpret_cast<MidiManager*>(instance)->enqueue(msg);
}
