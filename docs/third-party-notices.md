[简体中文](third-party-notices.zh_CN.md) · **English**

# Third-party notices

## Application and board support

`passport-pocket-dice` is based on
[FoloToy/ai-passport](https://github.com/FoloToy/ai-passport).
Its existing notice, **Copyright (c) 2026 FoloToy**, and complete
[MIT license](../LICENSE) are retained. This application continues to use that
license for its own code; dependency and asset licenses are separate.

## Functional inspiration

[Flipper Zero DnD Dice](https://github.com/Ka3u6y6a/flipper-zero-dice)
by Ka3u6y6a inspired the dice application. The reference repository identifies
GPL-3.0 as its license; see its
[license file](https://github.com/Ka3u6y6a/flipper-zero-dice/blob/main/LICENSE.md).

The maintainer confirmed the provenance review before preparing v0.2.
Only functionality and interaction ideas were referenced. No source code,
icons or animation assets from that project were copied or ported into this
application. This acknowledgement does not relicense the original project or
claim its author's endorsement.

## Fonts

- Noto Sans SC Regular, from
  [Noto CJK Sans2.004](https://github.com/notofonts/noto-cjk/tree/Sans2.004).
  The source font metadata credits Adobe, 2014–2021. The original OTF and
  generated 16 px subset remain under
  [SIL Open Font License 1.1](../assets/fonts/OFL.txt).
  The generated asset is named `dice_font_16`; source and glyph hashes are
  recorded in `assets/fonts/dice_font_16.json`.
- The Montserrat and Font Awesome glyphs in LVGL's built-in fonts retain their
  upstream notices and font licenses. These are not relicensed under the
  application's MIT license.

See the [asset guide](../assets/README.md) for font regeneration.

## Firmware dependencies

Versions follow [dependencies.lock](../dependencies.lock):

| Dependency | Version | Main license |
| --- | --- | --- |
| ESP-IDF | 5.5.3 | Apache-2.0; bundled components have additional notices |
| LVGL | 9.5.0 | MIT; bundled libraries and fonts have separate notices |
| espressif/button | 4.2.0 | Apache-2.0 |
| espressif/cmake_utilities | 1.1.1 | Apache-2.0 |
| espressif/esp_codec_dev | 1.6.2 | Apache-2.0 |
| espressif/esp_lvgl_port | 2.9.0 | Apache-2.0 |

Keep upstream copyright, license and NOTICE files with redistributed components.
The release license attachment collects license/notice files from the local
ESP-IDF and pinned Managed Components, as well as this project's license and
font notice. That source-tree collection may also contain notices for optional
components that are not linked into the firmware; it is not a list of enabled
features.
