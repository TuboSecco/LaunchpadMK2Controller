// Background Windows app — no console window.
// System-tray icon (right-click → Exit) is the only UI.
//
// Startup sequence:
//   clearAll() before switchPage(0) to wipe any hardware-persisted LED state.
//
// Shutdown (tray exit, logoff, or DestroyWindow):
//   clearAll() → midi.close() → PostQuitMessage

#include <Windows.h>
#include <shellapi.h>
#include <cstring>
#include <string>

#include "MidiManager.h"
#include "LedRenderer.h"
#include "PageManager.h"
#include "ConfigLoader.h"
#include "LightshowEngine.h"

#pragma comment(lib, "Shell32.lib")

static constexpr UINT WM_TRAYICON = WM_USER + 1;
static constexpr UINT IDM_EXIT    = 1001;
static constexpr UINT IDI_ICON1   = 101;
static constexpr UINT TIMER_MIDI  = 1;
static constexpr UINT TIMER_MS    = 10; // ~100 Hz

static struct {
    MidiManager*    midi = nullptr;
    LedRenderer*    led  = nullptr;
    NOTIFYICONDATAA nid  = {};
    bool            done = false;
} g;


static void doCleanup(HWND hwnd) {
    if (g.done) return;
    g.done = true;

    KillTimer(hwnd, TIMER_MIDI);
    Shell_NotifyIconA(NIM_DELETE, &g.nid);

    if (g.led)  g.led->clearAll();
    if (g.midi) g.midi->close();
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {

    case WM_TIMER:
        if (g.midi) g.midi->poll();
        return 0;

    case WM_TRAYICON:
        if (LOWORD(lp) == WM_RBUTTONUP || LOWORD(lp) == WM_LBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            HMENU menu = CreatePopupMenu();
            AppendMenuA(menu, MF_STRING, IDM_EXIT, "Exit Launchpad Controller");
            TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
            DestroyMenu(menu);
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wp) == IDM_EXIT) DestroyWindow(hwnd);
        return 0;

    case WM_QUERYENDSESSION:
        doCleanup(hwnd);
        return TRUE;

    case WM_DESTROY:
        doCleanup(hwnd);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}


static int runApp(HINSTANCE hInstance) {
    WNDCLASSEXA wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = "LaunchpadCtrl";
    RegisterClassExA(&wc);

    HWND hwnd = CreateWindowExA(
        0, wc.lpszClassName, "Launchpad Controller",
        WS_OVERLAPPEDWINDOW,
        0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

    MidiManager midi;
    if (!midi.openInput("Launchpad") || !midi.openOutput("Launchpad")) {
        MessageBoxA(nullptr,
            "Could not open Launchpad.\nCheck USB connection and driver.",
            "Launchpad Controller", MB_ICONERROR);
        return 1;
    }
    g.midi = &midi;

    LedRenderer     led(midi);
    PageManager     pages(led);
    LightshowEngine lightshow(led);
    g.led = &led;

    if (!ConfigLoader::load("config.json", pages) || pages.pageCount() == 0) {
        MessageBoxA(nullptr,
            "Failed to load config.json (or no pages defined).",
            "Launchpad Controller", MB_ICONERROR);
        return 1;
    }

    pages.setPageChangeCallback([&lightshow](const std::string& name) {
        lightshow.onPageChange(name);
    });

    midi.setCallback([&pages](const MidiMessage& msg) {
        const bool isNoteOn = ((msg.status & 0xF0) == 0x90) && (msg.data2 > 0);
        const bool isCC     = ((msg.status & 0xF0) == 0xB0) && (msg.data2 > 0);
        if (isNoteOn)
            pages.handlePadPress(msg.data1);
        else if (isCC) {
            if      (msg.data1 == 104) pages.prevPage();
            else if (msg.data1 == 105) pages.nextPage();
        }
    });

    led.clearAll();
    pages.switchPage(0);

    g.nid.cbSize           = sizeof(NOTIFYICONDATAA);
    g.nid.hWnd             = hwnd;
    g.nid.uID              = 1;
    g.nid.uFlags           = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    g.nid.uCallbackMessage = WM_TRAYICON;
    g.nid.hIcon            = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_ICON1));
    strcpy_s(g.nid.szTip, "Launchpad Controller");
    Shell_NotifyIconA(NIM_ADD, &g.nid);

    SetTimer(hwnd, TIMER_MIDI, TIMER_MS, nullptr);

    MSG winMsg;
    while (GetMessageA(&winMsg, nullptr, 0, 0) > 0) {
        TranslateMessage(&winMsg);
        DispatchMessageA(&winMsg);
    }

    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    return runApp(hInstance);
}
