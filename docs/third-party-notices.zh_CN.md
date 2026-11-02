[English](third-party-notices.md) · **简体中文**

# 第三方声明

## 应用与板级支持

`passport-pocket-dice` 基于
[FoloToy/ai-passport](https://github.com/FoloToy/ai-passport) 开发，
保留原有 **Copyright (c) 2026 FoloToy** 声明及完整的 [MIT 许可证](../LICENSE)。
本应用自己的代码沿用该许可证；依赖和资源的许可证分别适用。

## 功能灵感

[Flipper Zero DnD Dice](https://github.com/Ka3u6y6a/flipper-zero-dice)
由 Ka3u6y6a 开发，为本应用提供功能灵感。参考仓库标明采用 GPL-3.0，详见其
[许可证文件](https://github.com/Ka3u6y6a/flipper-zero-dice/blob/main/LICENSE.md)。

维护者在准备 v0.2 发布前确认已完成来源核对。
本项目仅参考功能与交互思路，未复制或移植该项目的源码、图标或动画资源。
此致谢不改变原项目许可证，也不代表获得原作者背书。

## 字体

- Noto Sans SC Regular 来自
  [Noto CJK Sans2.004](https://github.com/notofonts/noto-cjk/tree/Sans2.004)。
  源字体元数据署名为 Adobe，2014–2021。原 OTF 及生成的 16 像素子集保留
  [SIL Open Font License 1.1](../assets/fonts/OFL.txt)。
  生成资源名为 `dice_font_16`，源字体及字形哈希记录在
  `assets/fonts/dice_font_16.json`。
- LVGL 内置字体中的 Montserrat 和 Font Awesome 字形保留其上游声明与字体许可证，
  不改为本应用的 MIT 许可证。

字体重建方式见[资源说明](../assets/README.zh_CN.md)。

## 固件依赖

版本以 [dependencies.lock](../dependencies.lock) 为准：

| 依赖 | 版本 | 主要许可证 |
| --- | --- | --- |
| ESP-IDF | 5.5.3 | Apache-2.0；内含组件另有声明 |
| LVGL | 9.5.0 | MIT；内含库及字体另有声明 |
| espressif/button | 4.2.0 | Apache-2.0 |
| espressif/cmake_utilities | 1.1.1 | Apache-2.0 |
| espressif/esp_codec_dev | 1.6.2 | Apache-2.0 |
| espressif/esp_lvgl_port | 2.9.0 | Apache-2.0 |

再分发组件时保留上游版权、许可证及 NOTICE 文件。
发布许可附件收集本地 ESP-IDF 和锁定 Managed Components 中的许可与声明文件，
并包含本项目许可证及字体声明。这是源目录中的许可文件集合，可能包含未链接到固件的
可选组件声明，不代表固件启用了这些功能。
