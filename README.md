# Launchpad MK2 — Macro Controller

A lightweight Windows utility that turns your Novation Launchpad MK2 into a fully configurable macro pad. Each button can launch applications, send keyboard shortcuts, or trigger toggles — all defined in a simple JSON file you edit visually with the included configurator.

---

## Requirements

- Windows 10 or later
- Novation Launchpad MK2 connected via USB
- Official Launchpad MK2 driver installed (the device must appear as `"Launchpad MK2"` in your MIDI device list)

---

## Getting Started

1. Connect your Launchpad MK2 via USB.
2. Run `LaunchpadController.exe`.  
   On first launch, a default `config.json` is created automatically in the same folder as the executable.
3. The application runs silently in the background — no console window. A tray icon appears in the bottom-right system tray to confirm it is running.
4. To quit, right-click the tray icon and select **"Exit Launchpad Controller"**.

---

## Configuring Your Pads

Open `launchpad-configurator.html` in any browser — no internet required. This is a standalone visual editor where you can:

- Create and reorder pages
- Click any pad to assign a color, label, and action
- Export your setup as `config.json`

Once exported, place `config.json` in the same folder as the `.exe` and restart the application. The controller always reads `config.json` at startup.

To update your layout, edit it in the configurator, export again, overwrite the existing `config.json`, and restart the `.exe`.

---

## Pad Actions

| Action | Description |
|--------|-------------|
| `keys` | Send a keyboard shortcut. Examples: `Ctrl+Z`, `Win+Shift+S`, `F5`, media keys, etc. |
| `launch` | Open an application or file. Examples: `notepad.exe`, `C:\Tools\myapp.exe` |
| `toggle` | Two-state button. Fires a key and switches between two colors (ON / OFF). Useful for mute, recording arm, etc. |

---

## Pages

You can define as many pages as you like. Navigate between them using the arrow buttons at the top of the Launchpad (`CC 104` / `CC 105`).

### Lightshow Page

The page named `LIGHTSHOW` is a dedicated idle state. When active, the grid runs a slow, ambient ember-glow animation — warm colors drifting across the pads at low brightness. It is designed to be visually calm and to signal clearly that no macros are mapped to the current page, so you won't accidentally trigger shortcuts while away from your desk or during a break.

To use it, add a page called `LIGHTSHOW` to your `config.json` (the default config includes one). Navigate to it whenever you want a safe idle state, and navigate away to resume normal operation on any other page.

> The animation runs on a background thread and has negligible CPU impact.

---

## File Layout

```
LaunchpadController.exe       Main application
config.json                   Your pad configuration (auto-created if missing)
launchpad-configurator.html   Visual editor (open in any browser)
```

---

## Notes

- If the Launchpad is not detected at startup, a message box will appear. Check that the USB cable is connected and the driver is installed correctly.
- `config.json` is plain text — you can edit it manually if you prefer.
- The controller runs at ~100 Hz input polling, which is more than enough for responsive macro triggering.

---

*Developed by Giovanni Ponte*
