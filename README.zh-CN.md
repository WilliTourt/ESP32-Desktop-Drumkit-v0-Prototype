# ESP32 Desktop Drumkit v0 Prototype

## 项目简介

这是一个基于 ESP32 的电子鼓垫原型项目，支持 BLE MIDI 与串口 MIDI 输出。项目包含多路压电鼓垫/镲片采集，力度归一化，串口与 BLE 双模式输出，以及电池管理功能。  
本项目适合 DIY 音乐爱好者、电子鼓开发者、嵌入式学习者参考和二次开发。

---

## 功能特性

- **多Pad采集**：支持多路压电传感器（鼓垫、镲片），每Pad可独立设置阈值。
- **MIDI输出**：
  - **BLE MIDI**（支持手机、电脑无线连接）
  - **串口MIDI**（通过 CH340/USB 转串口与 PC 连接，Hairless MIDI 桥接软件支持）
- **力度归一化**：采集到的击打强度自动映射为标准 MIDI 力度（1~127）。
- **抗串扰算法**：每次采样只允许真实敲击的Pad触发（最大ADC值判断），其他Pad静默期避免误触发。
- **电池电压管理**：支持电池采样与低电量报警。
- **LED/Buzzer提示**：模式切换与状态反馈。
- **一键模式切换**：按钮切换 BLE MIDI / 串口MIDI。
- **外壳设计**：模块化设计，易于扩展。

---

## 硬件说明

- **主控芯片**：ESP32（推荐 ESP32-S3，原生USB可选）
- **压电片**：鼓垫推荐 ≥42mm，镲片推荐 27mm；每路信号线 *强烈强烈* 建议加屏蔽，并联0.1μF~1μF陶瓷电容
- **串口芯片**：CH340（或其他 USB-UART 转换器）
- **电池**：可选锂电池供电，带电池采样电路
- **其他**：LED、蜂鸣器、按键，外设接口见 `def.h`

---

## 软件说明

- **PlatformIO/Arduino 支持**，推荐 PlatformIO Arduino框架 环境
- **依赖库**：
  - `BLEMIDI_Transport` / `hardware/BLEMIDI_ESP32`
  - `OneButtonTiny`（按键检测）
  - 其他见源码头文件

---

## 使用方法

### 1. 硬件连接

- 按 `def.h` 配置各路压电片与 GPIO
- 串口 TX/RX 通过 CH340 与电脑相连，电脑识别为串口设备（如 COM3）
- BLE MIDI 支持手机或电脑蓝牙连接
- 按键用于模式切换

### 2. 编译与烧录

- 使用 PlatformIO，`pio run -t upload` 烧录
- 或用 Arduino IDE，选择正确板型与端口

### 3. MIDI输出说明

#### 串口MIDI

- 打开 [Hairless MIDI <-> Serial Bridge](https://projectgus.github.io/hairless-midiserial/) 或 [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html)
- 设置波特率为115200（与代码一致）
- 软件桥接后，PC上的DAW/音频软件可识别虚拟MIDI端口

#### BLE MIDI

- 连接蓝牙设备，选择“ESP32 Desktop DrumKit v1”
- DAW或手机MIDI应用可直接识别

### 4. 抗干扰/误触发说明

- 每次采样周期仅允许ADC值最大的Pad触发，其他Pad进入短暂静默期（软件防串扰）
- 鼓垫与镲片阈值可独立设置，见源码`Pad`对象构造参数
- 若出现极端误触发，可调整阈值、静默期或硬件分压/电容参数

---

## 源码结构

- `main.cpp`：主逻辑、Pad管理、MIDI输出与模式切换
- `pad.h` / `pad.cpp`：Pad对象定义、采样与触发判定
- `def.h`：所有硬件引脚定义、MIDI音色映射
- `OneButtonTiny.h` / `.cpp`：按键库
- `battery.h` / `.cpp`：电池管理
- 其他辅助文件

---

## 典型问题

1. **误触发/少触发/多Pad同时出音**
   - 检查硬件分压/电容/屏蔽
   - 调高鼓垫阈值，调低镲片阈值
   - 软件已采用最大ADC值触发+静默期防护

  *因为我也是第一次做这种乐器项目，实际的硬件结构没考虑好，忽略了压电片依赖的振动传递，导致鼓垫不灵敏。下一代v1会参考真实电子鼓的结构去设计。*

2. **串口MIDI无法被PC识别为MIDI设备**  
   - 用 Hairless MIDI 或 loopMIDI 进行串口-MIDI桥接

---

## 贡献/定制建议

- 按需修改`Pad`类构造参数，实现更精细的每Pad阈值调节
- 增加更多Pad支持，或扩展为双击/连击识别
- 如需原生USB MIDI（无需串口桥接），建议使用ESP32-S3并查阅TinyUSB库
- 欢迎PR/Issue交流

---

## 鸣谢

- 感谢所有开源库作者与电子鼓DIY社区分享经验
- 本项目受 [Victor2805/Homemade-electronic-drum-kit-with-arduino](https://github.com/Victor2805/Homemade-electronic-drum-kit-with-arduino) 启发
- 感谢我热爱打击乐器以及DAW经验丰富的表哥 @[Woodbreeze](https://github.com/WoodBreeze) ;)
---

**WilliTourt / 2025.9 原型发布**
