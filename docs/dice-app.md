[简体中文](dice-app.zh_CN.md) · **English**

# Pocket Dice

This application replaces the hardware-test menu with a portrait, dark navy and
amber interface. It runs offline on the existing Passport BSP. The Flipper Zero
[DnD Dice](https://github.com/Ka3u6y6a/flipper-zero-dice) project is a behavioral
reference; its GPL source, graphics, and UI were not incorporated.

## Features and controls

- Coin, D4, D6, D8, D10, D12, D20, and D100. Dice produce inclusive values 1..N.
- One to six dice per throw, a 720 ms animation, individual values and a total.
  D6 results use pip faces. A coin is always one coin and displays heads or tails.
- Twelve recent throws, newest first, including each die and totals up to 600.
- Battery percentage at the top right; unavailable readings show `--%`.

| Page | UP / DOWN tap | OK tap | Hold OK | Hold DOWN |
| --- | --- | --- | --- | --- |
| Roll | Previous / next type, wrapping | Roll again | Quantity | History |
| Quantity | Increase / decrease, clamped to 1..6 | Return | Return | No action |
| History | Newer / older, clamped at ends | Return | Return | No action |

Holds use the BSP's 500 ms threshold. Double taps on UP/DOWN count as two
steps; double OK starts at most one roll. Input during animation is ignored.
Coin quantity remains one. Switching to a coin resets quantity to one.
The footer's second line describes **hold** actions for both keys.

After 30 seconds idle the backlight dims, after 90 seconds it turns off,
and after five minutes the device enters deep sleep. The first press after
screen-off only wakes the screen. Any function key wakes deep sleep; release
the wake key before the next action. The session is retained in RTC memory
through this application's deep sleep, but lost on power-off, reset, or flashing.
There are no NVS writes or network/audio services.

## Implementation and limits

`main/dice_model.c` owns the portable state machine. `main/dice_ui.c` creates
the new screens using ordinary LVGL objects; no demo screen or `ui_pixel`
shell is linked. `main/main.c` queues button events and owns the model.
All UI access from that task uses the BSP LVGL lock. Battery I2C reads and
peripheral shutdown run outside the lock. Screen changes have no timers or
asynchronous producers referencing deleted children.

Random words come from `esp_random()`. Rejection sampling removes modulo bias;
retries are bounded and a failed draw preserves the previous result/history.
With radio off, ESP-IDF does not guarantee a continuing true-entropy source.
This is a recreational dice tool, not certified gambling or cryptographic RNG.
No additional ADC entropy source is enabled over the BSP's keypad ADC.

The default 8 MB partition table and BSP are unchanged. The codec is initialized
and suspended at boot; Wi-Fi and Bluetooth are not started. Deep sleep reuses the
BSP's keypad handoff, battery/audio suspension, shared-bus and display shutdown.
Shutdown failures are logged; terminal handoff failures restart the device.
Physical sleep current and wake behavior require device measurements.

Chinese labels use a 16 px, 4 bpp Noto Sans SC subset with 20 px line height.
The inventory covers every fixed application string plus printable ASCII;
arbitrary external text is not supported. Font provenance and regeneration
are in the [assets guide](../assets/README.md#fonts).
Boot checks every glyph through `lv_font_get_glyph_dsc()`, rejecting placeholders.
Numeric result labels use Montserrat 48; die values use Montserrat 20.
The LVGL pool is 32 KiB and the application task stack is 6 KiB.

## Build and validation

Use ESP-IDF 5.5.3 and the pinned component lock:

```sh
./tools/validate.sh --static
./tools/validate.sh --firmware
./tools/validate.sh
```

The gate includes deterministic sampling/bounds, timing, history rollover,
error atomicity, brightness thresholds, font inventory and bitmap readback.
Uniform-mapping tests do not measure the physical RNG's entropy.

For headless rendering with the actual pinned LVGL and application code,
after Managed Components are downloaded:

```sh
cmake -S tests/dice_ui -B build/ui-test -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ui-test
cd build/ui-test
./dice_ui_test
```

This checks glyph lookup, label bounds, 900 page transitions and LVGL heap
integrity, and creates PPM screen captures. It is a software renderer, not a
photo or proof of the physical display. Windows can select Ninja and a native
GCC compiler explicitly. The repository shell gate needs Git Bash, a `python3`
command for the activated Python, a host C compiler and Windows actionlint.

The verified `build/FoloToy-AI-Passport-full.bin` is the merged firmware for
offset **0x0**. Its matching ELF/MAP and manifest are retained under
`build/firmware/<sha256>/`. Never flash the application-only binary at 0x0.
A merged flash may reset stored NVS/PHY data. No full-chip erase is required.
Flashing requires explicit confirmation for this exact image.

## Device acceptance still required

1. Confirm boot, all Chinese glyphs and punctuation, clipping, color and D6 pips.
2. Exercise all eight types, quantity limits, repeated and long presses,
   animation input suppression, history endpoints, and six D100 values.
3. Check battery availability/failure, 30/90-second backlight behavior, and
   five-minute deep sleep; verify wake does not generate an unintended roll.
4. Check RTC session retention across sleep and clearing on cold boot.
5. Observe free heap/largest block, runtime stability, temperature and idle current.

Build, host tests and device tests must be reported separately. No flashing,
commit, push, or publication is part of the build process.
