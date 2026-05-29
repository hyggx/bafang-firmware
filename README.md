# 八方固件 · Bafang Firmware

> 专为泉盛 UV-K6 V3 / UV-K1 / UV-K1(8) 打造的中文自定义固件

---

## ⚠️ 使用须知

**使用本固件的风险完全由您自行承担。**
本固件不提供任何形式的保证，不保证在您的设备上正常工作，极端情况下可能导致设备无法启动（变砖）。

> 🔴 **重要**：本固件仅适用于搭载 **PY32F071** 主控的机型（UV-K6 V3 / UV-K1 / UV-K1(8)）。
> **请勿将本固件刷入旧款 UV-K5 V1（DP32G030 主控），否则会直接变砖。**
> 如不确定自己的机型，请查看机器背面的型号标贴。

**强烈建议在刷机前先备份校准数据**，具体方法见[备份校准数据](#-备份和恢复校准数据)章节。

---

## 📻 支持机型

| 机型 | 主控芯片 | 是否支持 |
|------|---------|---------|
| 泉盛 UV-K5 V3 | PY32F071 | ✅ |
| 泉盛 UV-K6 V3 | PY32F071 | ✅ |
| 泉盛 UV-K1 | PY32F071 | ✅ |
| 泉盛 UV-K1(8) | PY32F071 | ✅ |
| 泉盛 UV-K5 V1 | DP32G030 | ❌ 不支持，会变砖 |
| 泉盛 UV-K6 V1 | DP32G030 | ❌ 不支持，会变砖 |

---

## ✨ 功能概览

> ✅ = Default 版本已启用　　❌ = Default 未启用（Full 版包含）　　⚙️ = 菜单可手动开启

| 类别 | 功能 | 说明 | Default |
|------|------|------|---------|
| **接收 / 发射** | 宽频接收 | 超出业余频段的宽频接收能力 | ✅ |
| | 四档发射功率 | Low ≈0.5 W / Mid ≈1 W / High ≈2 W / Turbo ≈5 W | ✅ |
| | 区域发射锁定 | FCC / CE / GB / CA / PMR / FRS / GMRS / MURS | ✅ |
| | 窄带 / 超窄带 | 12.5 kHz / 6.25 kHz 切换 | ✅ |
| | AM 接收修正 | AM Fix 自动增益修正 | ✅ |
| | CTCSS / DCS | 亚音 / 数字静噪扫描与编码 | ✅ |
| **信道管理** | 999 信道存储 | 手动输入或 CHIRP 写频 | ✅ |
| | 信道名编辑 | 支持英文 / 中文拼音输入，最多 10 显示格 | ✅ |
| | 24 扫描列表 | 每条信道可分配至指定列表或全局扫描 | ✅ |
| | 极速扫描 | ≈150 个频率 / 秒 | ✅ |
| | 频率排除 | 扫描时跳过已标记频率 | ✅ |
| | 开机状态恢复 | 自动恢复上次扫描 / 频谱 / FM 状态 | ✅ |
| | AirCopy | 通过无线电波无线克隆信道数据 | ❌ |
| **频谱分析仪** | 一键开启 | **F + 5** 启动，显示当前频率附近活动分布 | ✅ |
| | 信道名叠加 | 锁定活跃频率并显示信道名 | ✅ |
| | 参数保存 | 频谱参数可持久化 | ✅ |
| **FM 收音机** | FM 广播接收 | 76–108 MHz，最多存储 20 个电台 | ✅ |
| | 自动搜台 | 一键扫描全波段 | ✅ |
| **界面 / 显示** | S 表样式 | 经典 / 精简（符合 IARU 一区 R.1 建议） | ✅ |
| | 屏幕模式 | 单 VFO / 双 VFO / 交叉显示 | ✅ |
| | 背光控制 | 亮度调节 + 渐变淡出 | ✅ |
| | 屏幕对比度 | 0–15 可调 | ✅ |
| | 屏幕翻转 | 180° 旋转显示 | ✅ |
| | 状态栏 | 电量 / 电压、PTT 模式、RX/TX 计时器、USB 图标 | ✅ |
| | 音频示波器 | TX 时显示音频波形 | ✅ |
| | 截图导出 | 通过 USB 串口导出屏幕（配合 K5Viewer） | ❌ |
| **输入法** | 中文拼音 IME | 拼音联想候选字，配合 CJK 字体使用 | ✅ |
| **其他** | VOX | 声控发射 | ⚙️ |
| | DTMF | 拨号音发射 | ⚙️ |
| | 省电休眠 | 无操作自动深度休眠（1 min – 2 h 可调） | ✅ |
| | BEAM | 紧急信令发射 | ❌ |

### ⌨️ 快捷键

| 按键组合 | 功能 |
|---------|------|
| **F + ↑ / ↓** | 动态调整静噪等级 |
| **F + F1 / F2** | 动态调整步进 |
| **F + 5** | 开启频谱分析仪 |
| **F + 8** | 临时切换最大背光亮度 |
| **F + 9** | 恢复自动背光策略 |
| **长按 MENU**（扫描中） | 排除当前信道 |
| **长按 EXIT** | 恢复完整 VFO 状态 |

---

## 🚀 刷机指南

> 刷机不需要安装任何软件，用浏览器即可完成。

### 第一步：备份校准数据（强烈建议）

> 校准数据存储着对讲机出厂频率校准信息，丢失后将影响发射频率准确度，**请务必在刷机前备份**。

1. 将对讲机**正常开机**后，用 USB 数据线连接电脑
2. 用 **Chrome 或 Edge 浏览器**打开：[uvtools2 备份页面](https://armel.github.io/uvtools2/?mode=dump)
3. 点击 `Dump Calibration Data`，选择串口
4. 完成后点击 `Download calibration.dat` 保存到电脑
5. 建议用对讲机序列号重命名文件（序列号在电池仓背面标签上）

### 第二步：下载固件

前往 [Releases 页面](../../releases) 下载最新版本的 `.bin` 固件文件。

> **版本说明**
>
> | 版本 | 适合人群 | 说明 |
> |------|---------|------|
> | **Default** | 日常使用推荐 | 完整功能集：频谱、FM、VOX、音频调节、扫描加速等 |
> | **Full** | 进阶 / 测试 | 全功能打开（含游戏、截图、AirCopy、救援操作等） |
> | **Custom** | 开发者 | 继承 Default，可自行增删功能开关 |

### 第三步：进入 DFU 刷机模式

| 机型 | 进入方法 |
|------|---------|
| UV-K6 V3 | 关机状态下，按住 **PTT + F** 同时插入 USB 线 |
| UV-K1 / UV-K1(8) | 关机状态下，按住 **PTT** 同时插入 USB 线 |

进入成功后屏幕通常显示版本信息或空白。

### 第四步：刷入固件

1. 用 **Chrome 或 Edge 浏览器**打开：[uvtools2 刷机页面](https://armel.github.io/uvtools2/?mode=flash)
2. 选择下载好的 `.bin` 固件文件
3. 点击 `Flash Firmware`，选择串口
4. 等待进度条走完
5. 对讲机自动重启，加载新固件完成 ✅

---

## 💾 备份和恢复校准数据

### 备份

1. 对讲机**正常开机**，连接 USB 数据线
2. 打开 [uvtools2 备份页面](https://armel.github.io/uvtools2/?mode=dump)
3. 点击 `Dump Calibration Data` → 选串口 → 下载 `calibration.dat`

### 恢复

1. 对讲机**正常开机**，连接 USB 数据线
2. 打开 [uvtools2 恢复页面](https://armel.github.io/uvtools2/?mode=restore)
3. 选择之前保存的 `calibration.dat` 文件
4. 点击 `Restore Calibration Data`，等待完成

---

## 📖 菜单说明

以下为菜单分类速览。每项参数的取值范围与使用建议，请查阅 **[Wiki › 菜单参考](../../wiki/菜单参考)**。

| 类别 | 主要设置项 |
|------|----------|
| **接收设置** | 静噪等级、步进、带宽、AM Fix、RX 频率偏移 |
| **发射设置** | 发射功率、CTCSS / DCS 编码、区域发射锁定 |
| **信道管理** | 写入信道、删除信道、编辑信道名、扫描列表分配 |
| **扫描** | 扫描模式、暂留时间、恢复条件、频率排除 |
| **显示 / 界面** | 背光亮度 / 时长、对比度、S 表样式、屏幕翻转 |
| **省电 / 系统** | 休眠时间、按键音、VOX、DTMF |
| **FM 收音机** | 频率、电台存储、自动搜台 |

---

## ❓ 常见问题

**Q：刷机后对讲机无法开机？**
A：请先尝试恢复校准数据。若问题持续，请在 [Issues](../../issues) 页面反馈。

**Q：刷机后发射频率偏移？**
A：通常是校准数据未恢复导致。请重新恢复备份的 `calibration.dat`。

**Q：如何写频（编程信道）？**
A：可使用 Releases 页面附带的 CHIRP 驱动，或在菜单中手动输入信道。

**Q：我的 UV-K5 旧款能刷吗？**
A：**不能。** 旧款 UV-K5（V1）使用 DP32G030 主控，与本固件不兼容，刷入会变砖。

**Q：这个固件是否合法？**
A：固件本身是合法的开源软件。发射行为是否合法取决于您当地的无线电法规及您是否持有相应许可证。请遵守当地法律法规。

> 更多问题请查阅 **[Wiki › 常见问题](../../wiki/常见问题)**。

---

## 🔧 开发者：从源码编译

> 本章节面向开发者，普通用户无需阅读。

### 使用 Docker（推荐）

```bash
git clone git@github.com:hyggx/bafang-firmware.git
cd bafang-firmware

./compile-with-docker.sh Default   # 推荐日常版
./compile-with-docker.sh Full      # 全功能版
./compile-with-docker.sh Custom    # 自定义版
```

### 本机编译（需 `arm-none-eabi-gcc` + CMake）

```bash
cmake --preset Default
cmake --build --preset Default -j
```

输出文件位于 `build/<Preset>/`，包含 `.elf`、`.bin`、`.hex`。

**功能开关**：在 `CMakePresets.json` 的 `Custom` Preset 中修改 `cacheVariables`，修改后需清空 `build/Custom/` 目录并重新运行 `cmake --preset Custom`。

---

## 🈶 中文字体刷写

> 本固件内置中文菜单与拼音输入法（IME），需要额外将字体文件写入 SPI Flash 才能显示汉字。

### 环境要求

```bash
pip install pyserial
```

### 准备字体文件

| 文件 | 字形数 | 大小 | 适用场景 |
|------|--------|------|---------|
| `tools/cjk_font_menu.bin` | 232 | ≈ 6.4 KB | 仅菜单固定字符串（今日可刷，无需新固件） |
| `tools/cjk_font.bin` | 1149 | ≈ 31.4 KB | 完整 IME 拼音候选字（需配套固件） |

重新生成字体（需要 Python 3 + `fonttools`）：

```bash
# 仅菜单字体（约 6.4 KB，适合通过 EEPROM 协议写入）
python3 tools/gen_cjk_font.py gen \
    --bdf tools/wenquanyi_9pt.bdf \
    --subset App/l10n/strings_zh.c \
    --subset App/l10n/strings.h \
    --subset App/ui/menu.c \
    --subset App/ui/menu_sub_values_zh.c \
    --out tools/cjk_font_menu.bin

# 完整 IME 字体（约 31 KB，需 CMD_0535 直写路径）
python3 tools/gen_cjk_font.py gen \
    --bdf tools/wenquanyi_9pt.bdf \
    --subset App/l10n/strings_zh.c \
    --subset App/l10n/strings.h \
    --subset App/ui/menu.c \
    --subset App/ui/menu_sub_values_zh.c \
    --subset App/ime/pinyin_table.c \
    --out tools/cjk_font.bin
```

### 刷写字体

对讲机**正常开机**后连接 USB，运行：

```bash
# 菜单字体（≤ 12 KB，走 EEPROM 虚地址协议，兼容任意版本固件）
python3 tools/serialtool/flash_font.py \
    --port /dev/cu.usbserial-130 \
    --font tools/cjk_font_menu.bin

# 完整 IME 字体（37.5 KB，自动切换到 CMD_0535 直写 SPI Flash，需固件含 ENABLE_UART_SPI_WRITE）
python3 tools/serialtool/flash_font.py \
    --port /dev/cu.usbserial-130 \
    --font tools/cjk_font.bin
```

Windows 用户将 `/dev/cu.usbserial-130` 替换为 `COM3`（或实际端口号）。

脚本会自动完成写入后的逐块读回校验，打印 `Done. CJK font successfully written to SPI Flash.` 表示成功。

> **提示**：固件刷写完成后只需刷一次字体；字体数据存储在 SPI Flash，不受固件更新影响。

---

## 🙏 致谢

本固件站在众多开发者的肩膀上：

- [DualTachyon](https://github.com/DualTachyon) — 泉盛 UV-K5 开源固件的奠基人
- [Egzumer](https://github.com/egzumer) — 大量基础功能增强
- [OneOfEleven](https://github.com/OneOfEleven) — 诸多核心改进
- [armel (F4HWN)](https://github.com/armel) — UV-K1/K5 V3 PY32F071 适配
- [muzkr](https://github.com/muzkr) — PY32F071 驱动开发
- [fagci (Mikhail)](https://github.com/fagci) — 频谱分析仪

---

## 📄 许可证

Copyright 2023 Dual Tachyon  
Copyright 2025–2026 [hyggx](https://github.com/hyggx) and Bafang Firmware Contributors  
基于 [Apache License 2.0](LICENSE) 开源协议发布。

如果您基于本项目创建派生版本，鼓励您保持开源，与社区共享您的工作。