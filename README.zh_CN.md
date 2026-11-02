[English](README.md) · **简体中文**

# passport-pocket-dice

面向 FoloToy AI Passport 的离线口袋骰子应用，采用中文界面、实体按键和重新设计的
深蓝与琥珀色 UI，复用 AI Passport BSP。

## v0.2 功能

- 硬币、D4、D6、D8、D10、D12、D20、D100 和 D200。
- 每次投掷 1–6 粒骰子，带骰面动画、各粒点数及最高 1200 的总和。
- D4 和 D6 使用圆点骰面；单粒骰子及硬币在原动画行居中。
- 最近 12 次投掷记录及电量显示。
- 使用 `1粒 点数4 骰子` 等中文描述，字体子集覆盖 163 个字符。

## 按键操作

| 页面 | 上 / 下短按 | 确定短按 | 长按确定 | 长按下键 |
| --- | --- | --- | --- | --- |
| 投掷 | 上一种 / 下一种 | 投掷 | 修改数量 | 历史记录 |
| 数量 | 增加 / 减少，限制为 1–6 | 返回 | 返回 | 无操作 |
| 历史 | 更新 / 更旧 | 返回 | 返回 | 无操作 |

硬币数量固定为一。长按阈值为 500 毫秒；虽然底部提示已简写，查看历史仍需长按下键。
投掷动画期间忽略输入。

闲置 30 秒调暗背光，90 秒熄屏，5 分钟进入深睡。熄屏后的第一次按键仅唤醒屏幕。
历史通过 RTC 内存在本应用深睡前后保留，但关机、复位或刷机会清除。

## 固件与构建

硬件为 FoloToy AI Passport：ESP32-C3、8 MB Flash、240 x 320 屏幕、无 PSRAM。
使用 ESP-IDF 5.5.3 和仓库的 `dependencies.lock`：

```sh
./tools/validate.sh --static
./tools/validate.sh --firmware
```

将验证后的合并固件 **`passport-pocket-dice-full.bin` 从 `0x0` 写入**。
不能将应用单镜像写到该偏移。合并刷写可能重置 NVS/PHY 数据，无需整片擦除。

详见[应用说明](docs/dice-app.zh_CN.md)、[v0.2 发布说明](docs/releases/v0.2.zh_CN.md)
及[构建指南](docs/development/engineering/build-and-test.zh_CN.md)，其中包含
Windows 原生固件构建的替代方式。

## 验证情况

v0.2 固件已使用 ESP-IDF 5.5.3 构建，验证各镜像内容及对应 ELF，
刷写后通过数据哈希校验，并观察启动 25 秒，未发现崩溃标记。使用者随后反馈工作正常。
主机测试覆盖骰子模型、全部类型和数量、界面布局、900 次页面切换和中文字形覆盖。
Windows 下跳过了 21 项已有的文件系统/符号链接测试。

休眠唤醒时序、闲置电流和长期稳定性仍未专项验证。
完整 shell 门禁因 ESP-IDF 的 MSYS 限制中止；静态检查及原生固件构建、
合并和验证已分别通过。

## 致谢与许可证

- 基于 [FoloToy/ai-passport](https://github.com/FoloToy/ai-passport) 开发，
  保留原有 FoloToy 版权及 [MIT 许可证](LICENSE)。
- 功能设计受到
  [Ka3u6y6a 的 Flipper Zero DnD Dice](https://github.com/Ka3u6y6a/flipper-zero-dice)
  启发，原仓库采用 GPL-3.0。本应用独立实现逻辑和界面，未复制或移植该项目的源码、
  图标或动画资源。感谢原作者分享创意。
- Noto Sans SC 字体子集保留 [SIL Open Font License 1.1](assets/fonts/OFL.txt)。
  其他依赖保留各自许可证，详见[第三方声明](docs/third-party-notices.zh_CN.md)。

本项目由独立维护者维护，与参考项目作者及 Flipper Devices 无隶属关系，
也不代表获得其认可或背书。
