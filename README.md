[简体中文](README.zh_CN.md) · **English**

# passport-pocket-dice

An offline pocket dice application for the FoloToy AI Passport, with a Chinese
interface, physical-button controls and a redesigned navy-and-amber UI.
The application reuses the AI Passport BSP.

## Version 0.2

- Coin, D4, D6, D8, D10, D12, D20, D100 and D200.
- One to six dice per throw, animated faces, individual values and totals up to 1200.
- Pip faces for D4 and D6; a single die or coin is centered on its animation row.
- Twelve recent throws and a battery indicator.
- Chinese quantity/face-count captions and a 163-character font subset.

## Controls

| Page | UP / DOWN tap | OK tap | Hold OK | Hold DOWN |
| --- | --- | --- | --- | --- |
| Roll | Previous / next type | Roll | Change quantity | History |
| Quantity | Increase / decrease, 1..6 | Return | Return | No action |
| History | Newer / older | Return | Return | No action |

A coin always has quantity one. Holds use a 500 ms threshold; opening history
still requires holding DOWN despite the shortened footer. Input during a roll
is ignored.

The backlight dims after 30 seconds idle and turns off after 90 seconds; deep
sleep starts after five minutes. The first press after screen-off only wakes
the screen. History survives this application's deep sleep through RTC memory,
but does not survive power-off, reset or flashing.

## Firmware and building

Hardware: FoloToy AI Passport, ESP32-C3, 8 MB Flash, 240 x 320 display, no PSRAM.
Use ESP-IDF 5.5.3 and the tracked `dependencies.lock`:

```sh
./tools/validate.sh --static
./tools/validate.sh --firmware
```

Flash the verified merged **`passport-pocket-dice-full.bin` at `0x0`**.
Do not use the application-only binary at that offset. Merged flashing can
reset NVS/PHY data; no whole-chip erase is required.

See the [application guide](docs/dice-app.md), [v0.2 release notes](docs/releases/v0.2.md),
and [build guide](docs/development/engineering/build-and-test.md), including
the native Windows firmware-build fallback.

## Validation

The v0.2 firmware was built with ESP-IDF 5.5.3, verified against its component
images and matching ELF, flashed with write-hash verification, and observed for
25 seconds without crash markers. The owner subsequently reported normal
operation. Host tests cover the dice model, all types/quantities, UI layout,
900 page transitions and Chinese glyph coverage. On Windows, 21 existing
filesystem/symlink cases were skipped.

Sleep/wake timing, idle current and long-term stability remain unverified.
The combined shell gate was blocked by ESP-IDF's MSYS restriction; static
checks and native firmware build/merge/verification passed separately.

## Acknowledgements and licensing

- Based on [FoloToy/ai-passport](https://github.com/FoloToy/ai-passport).
  The existing FoloToy copyright and [MIT license](LICENSE) are retained.
- Inspired by [Ka3u6y6a's Flipper Zero DnD Dice](https://github.com/Ka3u6y6a/flipper-zero-dice),
  whose repository uses GPL-3.0. This application independently implements its
  logic and UI; no source code, icons or animation assets from that project
  were copied or ported. Thanks to its author for sharing the idea.
- The Noto Sans SC font subset retains the
  [SIL Open Font License 1.1](assets/fonts/OFL.txt). Other dependencies retain
  their own licenses; see [third-party notices](docs/third-party-notices.md).

This independently maintained application is not affiliated with or endorsed by
the reference project's author or Flipper Devices.
