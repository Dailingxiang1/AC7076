# AC7076A3 demo 构建验证记录

最近更新：2026-09-28（北京时间）

## BOE1.8-N81 厂家初始化序列版（2026-09-28）

- 用户提供 `doc/BOE1.8-N81_JD9855_360x360_initial_code_V1.0_QSPI_G2.2_20250606（6代线）(1).c` 并确认采用厂家序列，已整表替换 `jd9855_panel_init.h`。
- 逐执行事件核对通过：50 条命令、全部参数、4 段延时（10/10/120/20 ms）与厂家调用流完整一致；页 0 `C8` 为 32 字节；包含 `4D/4E/4F=00 → 4C=01 → 10 ms → 4C=00`。独立复核也通过。
- 完整 700 字节 SDK 命令表已在链接后的 `sdk.elf` 中找到；核对记录：`output/bringup/jd9855-vendor-sequence-check.json`。
- 保持 360×360、offset=0、RGB565/QSPI SUBMODE1、PC3 硬复位、五色每 3 秒切换；厂家初始化窗口末值 `0168` 原样转换，测试绘制区域仍为 `0..359`。
- 背光恢复固定 80%（关闭此前 20%/80% 分段诊断）；USB CDC 与 PA2 调试 UART 关闭，PB0 LED 继续每 1 秒翻转。
- `Screen` 全量编译/链接退出码 0：ELF 5,168,792 字节（含调试信息）；SHA256 `800520B7E84FCC9EAD9417CDC0CF6A3AF5F3C359090767EEC76A5ACD533AACD9`。构建日志保留 SDK 既有 14 条链接栈大小警告。
- 生成的 INI 无 `MODE_FILE/FATFSI_FILE`，DATA 区仍为 `0x1D6000 + 0x28000`；使用 4 MiB Demo 布局。
- 2026-09-28 14:53（北京时间）用户进入 BR35 UBOOT 后，使用之前确认正确的 `C:\JL\keys\AC707N.key` 烧录上述 ELF。下载器显示 `Device online`、`Flash Capacity: 4M`、`Download completed.`、OTP 参数写入成功，退出码 0，无 `ERROR:`。
- 本次 `app.bin` 为 394,528 字节，SHA256 `492FED8FFF41C4B5B88F5E6A2D7BE1182F827E8162BCF3E51DF54E2A84C061CF`。下载日志与记录：`output/bringup/download-Screen-vendor-init.log`、`download-Screen-vendor-init.json`。
- 当前状态：厂家序列版已烧录成功，等待退出 Update、重新上电后的 LED 与五色画面观察；实板显示尚未验收。

## 360×360 JD9855 屏幕试点亮包（2026-09-24）

- 用户确认屏幕为 JD9855/QSPI 360×360 圆屏，背光 R5 已焊接；尚无模组型号与本模组专属初始化表。
- `Screen` 配置关闭 USB CDC、PA2 调试 UART及手表 UI；PB0 LED 继续每秒翻转。LCD 硬件接口在 UI 关闭时为 demo 单独保留，屏幕初始化运行于 `lcd_demo` 任务。
- 初始化表是另一块 JD9855/QSPI 360×360 圆屏的试点亮参数，不能替代本模组厂商参数。颜色、偏移及稳定性仍待实板观察。
- 首版五色循环全量构建通过：ELF 5,168,908 字节（含调试信息），SHA256 `D89DE7EDBD94E27206FB15D0743E36760ED89E5055BE7F0817088E0AA9169274`；`app.bin` 387,808 字节。INI 无手表 UI 资源分区，DATA 止于 `0x1FE000`。已用匹配 KEY 烧录。
- 实板观察：PB0 LED 正常；L1 已补焊；屏幕可见五种颜色变化，但用户报告画面颜色异常、不纯且持续闪烁，尚未点亮合格。
- 已将 `DEMO_LCD_COLOR_CYCLE_ENABLE` 默认设为 0：只写一次 360×360 白屏，随后不再发送像素数据。此版全量构建通过：ELF 5,168,212 字节，SHA256 `B1F8B57A90626C90147AC7B4B169E32F58863D908C8EFD3699E02FD149F70AB9`；`app.bin` 394,368 字节。
- 静态白屏版使用用户确认的 `C:\JL\keys\AC707N.key` 下载，工具显示在线设备、4M、`Download completed.`、OTP 参数写入成功，无 `ERROR:`。用户回报背光亮，但没有可识别画面。
- 已新增 `DEMO_LCD_INTERNAL_RAM_FILL=1`，只发 JD9855 0x4D/0x4E/0x4F 设置填充颜色与 0x4C 启用内部显存填充。此测试不发四线像素流，可区分命令/初始化与像素数据路径。同时明确把 PB8/RS 保持高电平，避免 QSPI 模式下悬空。
- 2026-09-25 将背光低电平占空比设为 80%，内部填充改为每 2 秒白/黑交替，以验证是否仅为显示反相。全量构建通过：ELF 5,169,464 字节，SHA256 `1AA1DAD71FBB0A91982695FC62A4EEE8C1923877BB0CA517395309415501020B`。用户进入 BR35 UBOOT 后使用匹配 KEY 烧录，工具显示 4M、`Download completed.` 和 OTP 参数写入成功，无 `ERROR:`；等待重启后实板观察。
- 实板回报：PB0 LED 持续闪烁、背光亮，但内部黑白交替填充完全没有可见变化；第一版主控五色像素填充曾产生五种变化。因此不能将当前故障简单解释为颜色反相，也不能仅凭内部填充无效断言命令线损坏。
- 恢复主控五色像素路径，撤回 PB8 固定高电平，DBI 目标帧率从 10 降为 5，每色停留从 1.5 秒延长为 3 秒，背光保持 80%。全量构建通过：ELF 5,168,772 字节，SHA256 `BC445B41D77292D095AC3665AA56A04F99055E93D080CFECA6B390F5CCBD3E04`。使用匹配 KEY 烧录，工具显示 4M、`Download completed.` 和 OTP 参数写入成功，无 `ERROR:`；待实板观察颜色和闪烁。
- 用户回报低速五色版也没有可见画面变化。板端 PB0 LED 仍正常；用户提出短距离 QSPI 线未等长。短距离线差暂不能认定为故障原因，需优先测 FPC VCC_3V、RESET 并查线序/连续性。
- 新诊断版将帧率恢复为第一版的 10，五色每 3 秒切换，背光每 15 秒在 20%/80% 间切换，以隔离高背光负载；已全量构建通过：ELF 5,169,452 字节，SHA256 `F4EA24911CB642DBA529348AAB4DE2DCF941C30E6CDDDEF363A165DEA179CB1E`，待烧录。
- 本次日志 `output/bringup/download-Screen-slow-color-80.log`。
- 本次下载日志 `output/bringup/download-Screen-bw-80.log`。
- 构建日志 `output/bringup/build-Screen.log`；下载日志 `output/bringup/download-Screen.log`、`output/bringup/download-Screen-static-white.log`。

## USB 虚拟 COM 历史尝试（2026-09-24，当前已暂停）

- 用户反馈此前基础固件已成功烧录，PB0 LED 能规律闪烁；这是实板运行的直接观察。USB 日志版已多次烧录，但 Windows 未枚举出虚拟 COM。
- 历史 CDC 测试版曾把 SDK `printf/putbyte` 输出先放入 4096 字节有界缓冲，再由 demo 任务分批写入 USB CDC；不初始化 PA2 调试 UART。当前 `Basic` 与 `Screen` 均默认关闭 CDC。
- 初版烧录后用户反馈 LED 闪烁、数据线正常，但 Windows 没有新的 USB/COM 设备。于是补充了 demo 启动时主动调用 `usb_device_mode(0, CDC_CLASS)`，保留 OTG 后台重连路径，并再次全量构建。
- 修正版全量构建退出码 0，ELF 为 5,173,468 字节（含调试信息），SHA256 为 `F83B6C54131DA5DA1CAA4E367F261A24F5D132A4D7B10856A1367B3C5DF4E56E`；抽取的 `app.bin` 为 387,744 字节。静态核对 DATA 分区仍止于 `0x1FE000`，没有原手表 UI 分区。
- 后续直接在 app_core 启动 CDC 导致 LED 停止闪烁；改为 SDK USB 任务异步启动后 LED 恢复，但 Windows 仍无新 COM。重新插拔 USB 线后 LED 只亮一下就灭。用户决定先暂停 USB，改做屏幕点亮。
- 构建与下载日志：`output/bringup/build-Basic.log`、`output/bringup/download-Basic-USB-retry.log`。

## 4 MiB 板烧录复核（2026-09-24）

- `Basic` 与 `AudioUsb` 均已全量编译和链接通过。`Basic` ELF 为 5,093,180 字节，抽取的 `app.bin` 为 378,208 字节；`AudioUsb` ELF 为 5,180,616 字节。ELF 含调试信息，不能按其文件长度估计烧录占用。
- `AC7076A3_DEMO_ENABLE=1` 时，Flash 配置为 `0x400000`，关闭原手表 UI。生成的 `isd_config.ini` 无 `MODE_FILE` / `FATFSI_FILE`，DATA 分区止于 `0x1FE000`；生成的批处理进入无 UI 打包分支。
- 2026-09-24 实际调用 `Basic -Download`。烧录工具显示 `Device online`、`Chip Version: C`、`Flash Capacity: 4M`，原来的 8M 容量错误没有再出现。随后报 `ERROR: Key mismatch!!!`，提示这颗芯片已写入 KEY，而命令未提供匹配的 `-key` 文件。**本次没有烧录成功，不能称实板 demo 已跑通。**
- 工程内未找到 `.key` 文件，`KEY_FILE_PATH` 未设置。[杰理官方排障文档](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/faqs/why_failed_to_download.html)说明非空片必须使用与芯片匹配的 KEY；需向首次烧录方、芯片代理商或杰理索取。不能通过全片擦除改变已烧入的 KEY。
- 下载 batch 在设备写入失败后仍继续生成并复制 FW/UFW；本次从根 `output/` 清除了失败运行生成的 `jl_isd.fw`、`jl_isd.bin`、`update.ufw`，以免误用。详细错误在 `output/bringup/download-Basic.log`。
- 更新后的构建脚本支持 `-KeyFile`，只接受供应商给的匹配 `.key` 文件路径；下一次仍以烧录工具的成功结果和新固件启动日志为硬件验收依据。

以下为 2026-09-10 的历史编译记录，当时尚未发现板载 4 MiB 容量和 KEY 限制。

## 实测组合

| 配置 | 说明 | 完成时间 | ELF 字节 | 警告行数 | 结果 |
| --- | --- | --- | ---: | ---: | --- |
| Configured | 最终头文件默认值：基础外设，音频/USB CDC/LCD 关闭；Windows PowerShell 5.1 | 2026-09-10T09:16:17.1235300+08:00 | 8287876 | 71 | make 退出码 0 |
| AudioUsb | 基础 + MIC + Speaker + USB CDC；PowerShell 7.6.5 | 2026-09-10T09:13:30.2578867+08:00 | 8372436 | 71 | make 退出码 0 |
| Original | 总宏 0，恢复原工程入口、原配置；PowerShell 7.6.5 | 2026-09-10T09:05:56.7738448+08:00 | 11112000 | 103 | make 退出码 0 |

Configured 与 AudioUsb 在最后一次启动流程修正后全量重编。Original 已在本轮全量编译通过；之后新增的时钟初始化入口与 demo 消息循环改动均位于总宏开启分支。
此前 Basic 和 Audio 组合也编译通过；最后基础固件以 Configured 记录为准。

警告来自 SDK 链接阶段的栈大小限制检查，未消除，不能把编译成功当成栈使用或硬件稳定性已经验证。原工程配置为 103 条同类警告。

## 编译保护与集成检查

- 在未确认模组参数时强制 `DEMO_LCD_ENABLE=1`，预处理按预期失败，提示先确认 JD9855 分辨率并提供初始化表。日志：`output/bringup/build-lcd-guard.log`。
- JD9855 色彩测试函数即使 LCD 关闭也参与 C 接口类型检查；运行入口仍由宏关闭。没有使用占位表向屏幕发初始化命令。
- `SDK/AC707N.cbp` XML 可解析，新增 8 个 bringup 源码/头文件条目均存在，Makefile 同时收录 4 个 C 文件。
- 修复 SDK CDC 配置工具消费者的条件编译，避免纯 CDC 回显仍链接已关闭的 `cfg_tool_data_from_cdc`。
- 独占 demo 单独初始化时钟管理互斥量，按需调用音频初始化；任务使用 `os_taskq_pend_timeout` 保留 SDK 回调分发。
- 马达关闭使用优先级 1 的 usr timeout 回调；申请失败立即停止输出。
- PDF 共 8 页，已渲染检查全部页面，并检查文字边界，无页面溢出。
- Git 差异检查剩余空白告警位于原有生成配置 `sdk_config.h`、`jlstream_node_cfg.h`；没有为清理空白改写这些用户文件。

## 已验收的屏幕固件（JD9855 厂家初始化序列版）

- 配置：`Screen`，总宏 1，4 MiB，USB CDC 关闭，BOE1.8-N81 厂家初始化表，无原手表 UI；背光固定 80%。
- 路径：`SDK/cpu/br35/tools/sdk.elf`。
- 大小：5,168,792 字节（含调试信息）。
- SHA256：`800520B7E84FCC9EAD9417CDC0CF6A3AF5F3C359090767EEC76A5ACD533AACD9`。
- 构建命令：`& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Screen`；厂家序列版已构建并于 2026-09-28 14:53 烧录成功，用户随后确认屏幕和 LED 正常。该 ELF 已备份至 `output/bringup/screen-vendor-known-good/sdk.elf`。
- `output/bringup/last-build.json` 和各 `build-*.json` 保存参数、时间及散列；日志保存于同目录。不同配置共用一个 ELF 路径，后一次编译会替换前一次产物。

## 硬件待验收项

- USB CDC 暂停；PA2 未引出，当前固件无串口日志输出。
- LED、按键、马达、DA213B、VBAT/VPWR、音频、USB 枚举与回显按调试手册逐项验证。
- 板载 CST816S 已配置何种固件/电极参数、实际地址与报点协议；本 demo 不刷写触摸固件。
- JD9855 分辨率已确认 360×360，厂家初始化表已整表移植；2026-09-28 用户确认实板屏幕正常。
- 背光回路 R5 已确认焊接，阻值仍待核对。

本轮未宣称已在 AC7076A3 实板跑通全部外设。

## DA213B 专项测试版（2026-09-28）

- 新增 `Accel` 配置：LED + 厂家 JD9855 初始化 + DA213B 屏幕读数；触摸、总线扫描、按键、马达、电压检测、音频和 USB CDC 关闭。
- PB1/PB2 软件 I²C，50 kHz，七位地址 0x27。读取 ID=0x13 才进行配置，并读回校验；±2g、62.5 Hz，20 ms 轮询，250 ms 刷屏。
- 屏幕显示 ID、初始化/读取状态、XYZ mg、成功读取计数和数据读取错误计数。失败不显示旧读数；缺失/配置失败每 3 秒重试。
- 快照采用互斥量保护，LCD 任务不操作 I²C；RGB565 条带使用独立对齐缓冲区，DMA 完成后才复用，逐条设置显示窗口。
- 360×360 圆屏字库布局已离线渲染检查，预览位于 `output/bringup/accel-screen-preview.png`（示例数据，不是实板读数）。
- 最终全量构建成功，14 条原有 SDK 链接栈警告，无新增编译错误。ELF 5,173,384 字节；SHA256：`A78864F555D5BDA95F1F74E24D3F898CF907B30384517576E81A4220F9B2079C`。
- 当前 ELF 位于 `SDK/cpu/br35/tools/sdk.elf`；构建参数/日志见 `output/bringup/build-Accel.json`、`build-Accel.log`。DA213B 功能仍须以重启后实际 ID、计数和翻转响应验收。
- Accel 已使用已确认的 KEY 烧录成功：下载器识别 Flash Capacity: 4M，报告 Download completed 和 OTP_CFG 写入成功，无 ERROR。见 output/bringup/download-Accel.log、download-Accel.json；等待实板加速度读数验收。

## 触摸识别与 I2C 扫描版（2026-09-28）

- 用户报告 DA213B 测试版显示 NO DEVICE；功能未验收通过，不能据此确定焊接故障。
- 新增 I2cScan 配置：LED、JD9855 厂家初始化表、80% 背光、触摸复位/ID 读取与 I2C 扫描；USB CDC、DA213B 配置写入及其他外设业务关闭。
- 七位地址 0x08～0x77，112 个常规地址，每个分别测试读/写 ACK；未知器件不写寄存器/配置。读探测接收一字节后 NACK/STOP。
- 每轮先复位触摸并读取 0x15 的 A7～A9。屏幕显示总线电平、最近触摸 ID、IRQ、已完成轮数和累计 ACK 地址，12 个地址/页，每 4 秒翻页。
- 扫描每次处理一个地址，不阻塞整个扫描周期；总线拉低时暂停并重试初始化。扫描/触摸均由原 demo 任务独占，LCD 用互斥快照。
- 界面已离线渲染核对，output/bringup/i2cscan-screen-preview.png 为示例数据，不是实板扫描结果。
- 全量构建及最终改动重新链接均成功；屏幕扫描列表移至 LCD 任务独占静态缓冲区，消除了新增 LCD 栈警告。最终仅保留 SDK 原有 14 条链接栈警告。
- 当前 ELF：SDK/cpu/br35/tools/sdk.elf；字节：5174356；SHA256：661318D4C04C40C28B346C52C3003E5BC0288B95D1836ACA518730477C415A3E。构建记录：output/bringup/build-I2cScan.json，最终日志：build-I2cScan-final.log。尚未烧录、未取得实板扫描结果。
- I2cScan 已烧录成功：下载器识别 4M Flash，报告 Download completed 和 OTP_CFG 写入成功，无 ERROR；日志和记录见 output/bringup/download-I2cScan.log、download-I2cScan.json。等待重新上电后的实板扫描结果。

### I2cScan 实板反馈与待定位项

- 用户反馈 SDA=1、SCL=0、TP:NO ID。此结果不证明触摸或 DA213B 单独损坏；两者共用时钟线，持续低电平首先阻断通信。
- 已核对 soft_iic 的 SCL_H 和 STOP：通过 PORT_INPUT_PULLUP_10K 释放时钟线；demo bus_init 也每次先将 PB2/PB1 设为上拉输入再读取电平，低电平时不扫描。
- 等待用户量取 PB2/SCL 对地电压、R3 上拉电源端电压，以及断电后 SCL 对地电阻；需测量才能区分短路、上拉/供电、外设拉低或引脚配置问题。
- 额外发现 SDK/cpu/components/iic_soft.c 的 iic_get_delay 固定返回 0，板级注释也说明软件 master_frequency 未使用。因此先前 50 kHz 是配置值而不是已落实/实测的时序。当前烧录固件未修改 SDK 软件 I2C 延时，待总线电平问题定位后修正并验证。
- 发现软件 IIC 初始化后再次 soft_iic_init 会返回已占用错误；电平恢复后的重试初始化路径亦需修正。该问题不能单独解释已释放 PB2 后仍读到 SCL=0。
- 已修正 demo 的已初始化总线重试路径：电平恢复时复用已初始化的软件 IIC，避免反复初始化被 SDK 判为占用；同时纠正日志中未经验证的 50 kHz 描述。重编链接通过，14 条 SDK 原有栈警告。最新 ELF SHA256：7C9C020B6664D31BABC179868EC23A39457C4B13373E02F8794630A07B46366D，日志 build-I2cScan-recovery.log。此修正版尚未烧录；板上仍为此前扫描版。该修改不声称解决硬件 SCL 低电平或软件时序问题。

### PB2 数字输入诊断准备

- 用户明确 3.3 V 在上拉电阻处测得，未测 MCU PB2 引脚本体；电阻哪一端仍待确认。因此不能将电源端电压等同于 SCL 电平。
- 已核对 gpio_set_mode 的已烧录库反汇编：PORT_INPUT_PULLUP_10K 路径设置输入方向、DIE=1、DIEH=1；gpio_read 转到 gpio_hw_read，后者通过 gpio2reg 取 IN 寄存器，并按 gpio 的低 4 位选位。PB2 定义为 18，位号为 2；没有发现明显的 API/编号偏移。
- 新增 I2cPins 诊断配置，PB1/PB2 全程不发送 I2C，屏幕同时显示 API 返回值、JL_PORTB->IN、DIR、DIE、DIEH 两个引脚的位。只配置上拉输入，不用推挽强行拉高总线。
- 该诊断不宣称已经解决 GPIO 电压/读值差异；须对照实际 MCU 引脚电压及新页面才能定位。
- I2cPins 全量编译链接成功，14 条 SDK 原有栈警告；ELF 5167580 字节，SHA256：A0E0830E819C6AA67876B841B3ED89BC49440B07B5CD3053723C4F1EFE2A0A38。构建日志/参数见 output/bringup/build-I2cPins.log、build-I2cPins.json。链接符号检查中发现的软件 I2C 事务/初始化及触摸探测函数数量：0。尚未烧录。
- I2cPins 已烧录成功：4M Flash，Download completed，OTP_CFG 写入成功，无 ERROR。日志/记录：output/bringup/download-I2cPins.log、download-I2cPins.json。等待重启后的 API/RAW/DIR/DIE/DIEH 和 MCU 引脚电压对照。

### I2cPins 实板验收与 GPIO I2C 版

- 用户确认 I2cPins 的 API、RAW、DIR、DIE、DIEH 均为 1 1，静态上拉输入读高测试通过。持续 PB2 接地或 GPIO 完全无法读高的判断目前不受该结果支持；通信中异常仍未定位。
- 已备份此诊断 ELF 和构建记录至 output/bringup/i2cpins-known-good/，原 SDK 软件扫描的下载记录保留于 output/bringup/history/download-I2cScan-sdk-soft.*。
- 新 I2cScan 通过 DEMO_I2C_GPIO_ENABLE=1 选择 demo 内的 GPIO I2C 后端，未修改 SDK 全局软件 I2C 驱动；其余配置默认仍为 0。
- GPIO 后端只拉低或释放：每个低/高阶段调用 10 us 延时；释放 SCL 后轮询实际输入，高电平阶段才继续，等待预算 5000 us。实际速率/延时精度需波形实测，未宣称精确 50 kHz。
- ACK 在第九个时钟采样；读字节中间 ACK、最后 NACK；寄存器读使用重复 START；未知器件扫描无寄存器写入。每个事务检查 STOP 后两线高，错误路径释放 MCU 的 SDA/SCL 输出。
- 显示的 FAULT 为累计最近总线异常码：0 尚未记录，1 SCL 等待超时，2 START 时 SDA 低，3 STOP 后总线未回高。普通地址 NACK 不视为总线异常，ACK 列表仍为累计结果。
- GPIO 后端修复是否有效仍需重新烧录观察，当前未取得触摸 ID 或地址响应验收。
- GPIO I2cScan 全量编译成功，14 条 SDK 原有栈警告，ELF 5176060 字节、SHA256：3A2AB27C56E19ADDADA656EC5A7828C7A038B790A2802A77F531320AA71D1581。链接符号包含 gpio_i2c_start/stop/tx/rx/scl_high；旧 soft_iic_init/start/stop/tx_byte 未出现在符号检查结果中。
- GPIO I2cScan 已烧录成功：4M Flash，Download completed，OTP_CFG 写入成功，无 ERROR；日志/记录为 output/bringup/download-I2cScan-gpio.log、download-I2cScan-gpio.json。尚待重启后通信验收。

### I2cDiag 编译、烧录与待验收（2026-09-28）

- GPIO I2cScan 用户反馈 FAULT=1、ACK=0；未取得触摸 ID。上拉电阻测量端及 MCU PB2 实际电压仍未确认。
- 新增 I2cDiag 配置与 DEMO_I2C_EDGE_TEST 开关：通信前测试 SCL 拉低/释放；捕获首次故障的阶段、位、地址、API/IN/DIR/DIE/DIEH、模式 API 返回值，后续扫描轮询冻结。LED 1 秒间隔、厂家 LCD 初始化及 80% 背光保持原配置。
- 全量构建成功，14 条原有 SDK 栈警告。ELF 5181508 字节；SHA256：2D3BF6F2D48DE4C8B6DB85C2D8CC26C884EA4B5407E13C72AA2AA8DAA331223C。构建记录：output/bringup/build-I2cDiag.json、build-I2cDiag.log。
- 两页已离线渲染检查，预览 output/bringup/i2cdiag-page1-preview.png、i2cdiag-page2-preview.png 采用示例数据，不是实板读数。
- 用户确认进入下载模式后，已烧录对应散列的 ELF：下载器识别 4M Flash，Download completed，OTP_CFG 写入成功，无 ERROR；output/bringup/download-I2cDiag.log、download-I2cDiag.json 保存记录。
- 尚待重启后的两个页面反馈。触摸/I2C 功能未验收通过，尚未确定 SCL 超时原因。

### I2cDiag 实板照片反馈

- 两页照片归档：output/bringup/i2cdiag-hardware-edges.jpg、i2cdiag-hardware-fault.jpg。
- 切换页：PRE=11、LOW=10、UP=11、LATE=11、CFG ERR=0、EDGE PASS。慢速切换与恢复读高通过；不能替代示波器上的通信波形验证。
- 故障页：F=1、A=6A、PH=3、BIT=255、API=10、RAW=10、DIR=11、DIE=11、H=11、RET=0。
- 按当前代码，表示扫描七位地址 0x6A 时，在发送字节后第九个时钟（从机 ACK 阶段）释放 SCL 等待超时。0x6A 是尝试的地址，不是已识别的器件地址；255 是非数据位阶段标记。
- 首次故障采样时两线方向均为输入，数字输入使能，SCL 配置 API 返回成功，但 API 和原始 IN 均读 SCL=0。不能仅凭照片确定物理脚电压、谁拉低时钟或是否存在 GPIO 复用问题。
- 下一步需核对故障冻结后 R3 的 SCL 信号端及 MCU PB2 电压；如有示波器/逻辑分析仪，捕获重启后的 PB1/PB2 通信波形，比较释放 SCL 与第九个时钟。触摸功能仍未通过验收。

### 背光降至 40% 的电源对比（2026-09-28）

- 用户实测标称 3.3V 电源为 2.8V，具体测量节点尚待确认。背光宏 DEMO_LCD_BACKLIGHT_DUTY 从 8000 改为 4000（PWM 参数满量程 10000），关闭亮度交替；保留当前 I2cDiag、厂家 LCD 初始化、LED 1 秒间隔和 USB CDC 禁用配置。
- 工程 sdk_config.h 的工作 IOVDD 档位为 VDDIOM_VOL_32V；电压档位配置不等于实测值，尚不能判断 2.8V 是否由负载、输入或供电路径引起。本轮未修改电源档位。
- I2cDiag 全量构建成功，14 条原有 SDK 栈警告，ELF 5181472 字节，SHA256：0BDB29AB4D5E30B4C05FBF52ED772FA16A6CAD7E5B05F365C48537D6D99499FE。
- 用户确认进入下载模式后烧录成功：4M Flash、Download completed、OTP_CFG 写入成功，无 ERROR。日志 output/bringup/download-I2cDiag-40.log，记录 download-I2cDiag-40.json。原 80% 版构建/下载及照片反馈记录归档 output/bringup/history/i2cdiag-80/。
- 待重新上电后在同一测量点比较电压，核对电源节点及故障页面。触摸/I2C 尚未验收。

### 10% 背光 I2cScan 实板测试（2026-09-29）

- 背光 PWM 改为 1000/10000（10%），使用 I2cScan 配置重新检查响应地址。LED 保持 1 秒间隔，USB CDC 禁用，LCD 厂家初始化表未改变。
- 全量构建成功，14 条原有 SDK 栈警告。ELF 5179720 字节，SHA256：028F9BD821C8A4B0E0F250523EFB8067589232AB4BDA64B5D6F0EDDE673B718C；ELF 和构建记录备份至 output/bringup/i2cscan-10/。
- 烧录成功：4M Flash、Download completed、OTP_CFG 写入成功，无 ERROR。记录 output/bringup/download-I2cScan-10.json、download-I2cScan-10.log。
- 用户反馈降至 10% 后仍无器件响应；未提供具体 ACK/FAULT、地址、电压。按用户要求转入蓝牙测试，触摸和 DA213B 未验收通过。
- 电源判断补充：sdk_config.h 的 3.2V 为工作档位配置，而当前独立 demo 跳过完整应用的 board_power_init/产品 initcall，不能认为实板已经应用该档位，也不能据此认定 2.8V 必然是负载压降。本轮仅修改背光，未改电源配置或初始化。

### Bluetooth 测试版构建与待验收（2026-09-29）

- 新增 Bluetooth 构建配置、DEMO_BLE_ENABLE 开关及 ac7076a3_demo_ble.c；独立 BLE 测试采用 SDK TRANS_DATA 的 GATT 服务/回显，关闭经典蓝牙、CTKD/桥接、I2C 和其他外设业务，保留 10% 背光、JD9855 厂家初始化和 1 秒 LED。
- 初始化名称/MAC、射频功率及 PLL 参数，启动 btstack；独立 app_core 处理 BT_STATUS_INIT_OK，避免丢弃初始化事件。SDK 状态/接收回调更新互斥快照，LCD 只读取快照，广播请求不冒充手机扫描验收。
- 全量编译及界面名称修正后重新链接成功。ELF 6656648 字节，SHA256：0C1DDFF7548114F4490A46C185BF276164E63A3436DA26287348A594BC4BF99F。记录 output/bringup/build-Bluetooth.json，最终日志 build-Bluetooth-final.log。
- 链接有 28 条 SDK 库栈警告，包含本次引入蓝牙栈后的 SDK 警告；未出现新的 demo 函数栈警告或编译错误。这些警告不能视为实板运行验收，仍需观察初始化和连接稳定性。
- ELF 符号确认 btstack_init、SDK TRANS_DATA 的 ble_profile_init/bt_ble_init 已链接；GPIO I2C 发送函数未出现在本版符号检查结果中。output/bringup/Bluetooth-check.lst 保存检查反汇编。
- 请求用户重新进入下载模式；此记录写入时尚未烧录，也未取得手机扫描、连接或回显结果。

- Bluetooth 已按用户确认烧录成功：4M Flash、Download completed、OTP_CFG 写入成功，无 ERROR；对应 ELF SHA256 为 0C1DDFF7548114F4490A46C185BF276164E63A3436DA26287348A594BC4BF99F。下载记录 output/bringup/download-Bluetooth.json、download-Bluetooth.log。等待重启后手机扫描/连接/AE01-AE02 回显，未宣称蓝牙实板通过。

### 蓝牙版异常与回退请求（2026-09-29）

- 用户报告蓝牙版屏幕不显示，随后明确表示使用蓝牙会死机，要求停止蓝牙测试，回退到 I2C 设备扫描 + 屏幕显示。
- BLE 扫描/连接/回显均未验收通过；不能凭屏幕无画面认定排线接触不良，亦未定位蓝牙启动挂起原因。
- 正在以 I2cScan 配置全量重编，DEMO_BLE_ENABLE 默认为 0；保持 10% 背光、1 秒 LED、GPIO I2C 扫描及厂家 JD9855 初始化。等待进入下载模式后烧录，未继续蓝牙测试。

- I2cScan 回退版已全量构建并烧录成功：ELF 5179960 字节、14 条原有 SDK 栈警告，SHA256：5CA016CADAD9C095D381C49CF65F874569BEC8DD7A76361E6D6D4337347A4DBD。下载器识别 4M、Download completed、OTP_CFG 写入成功，无 ERROR；记录 output/bringup/download-I2cScan-rollback.json、download-I2cScan-rollback.log。
- 符号检查确认 btstack_init 与独立 demo 的 BLE 启动/状态/显示代码未链接。SDK 注册表仍保留 bt_ble_init 符号，但当前 demo 不启动蓝牙栈，也不处理该注册表的蓝牙事件；不能将符号存在等同于蓝牙运行。
- 待退出 Update 重新上电确认屏幕和 LED 恢复。I2C 器件响应仍待验收，蓝牙问题暂停定位。

- 回退后用户反馈 LED 已恢复，但屏幕背光亮、没有文字；屏幕恢复未验收。背光工作不能证明 LCD 数据/时钟/复位连接或初始化成功。准备写回 output/bringup/i2cscan-10/sdk.elf 原始备份（SHA256 028F9BD821C8A4B0E0F250523EFB8067589232AB4BDA64B5D6F0EDDE673B718C），保持 10% 背光、扫描和蓝牙禁用，用于对比本次重新编译版本。等待用户进入下载模式。

- 原始 10% I2cScan 备份已写回并烧录成功：SHA256 028F9BD821C8A4B0E0F250523EFB8067589232AB4BDA64B5D6F0EDDE673B718C，下载器识别 4M、Download completed、OTP_CFG 写入成功，无 ERROR。下载记录 output/bringup/download-I2cScan-original10.json、download-I2cScan-original10.log。当前 SDK/cpu/br35/tools/sdk.elf 为备份恢复的旧版本，last-build.json 已标记 restoredFrom/restoredAt；build-I2cScan.json 仍为上次重新编译的记录，两者散列不同。等待彻底断电重启后的屏幕对照结果，尚未认定排线故障。

### 换屏恢复与 3.0V/地址范围核对（2026-09-29）

- 用户换另一块屏后显示正常，原始 10% 扫描固件的 LCD 恢复通过；未定位旧屏、旧排线或接触的具体原因。I2C 仍无器件响应，MCU IOVDD 实测约 3.0V。
- 本地 DA213B 数据手册表 3：VDD 1.62～3.6V，VDD_IO 1.62V～VDD，I2C VIH 最小为 0.7×VDD_IO。若器件两路实际供电均为 3.0V，该电压在规定范围内；MCU IOVDD 测量不能替代器件电源脚测量，也不能证明供电纹波/瞬态正常。
- 表 7/8 确认 DA213B 七位地址 0x27，线上写/读地址字节 0x4E/0x4F。demo 按 (addr<<1)|read 生成线上字节，0x08～0x77 扫描已包含 DA213B 0x27 和配置的触摸 0x15。触摸实际器件型号/供电范围仍以厂家资料为准。
- 常规 I2C 七位地址 0x00～0x7F，两端保留段通常跳过；0x00～0xFF 是地址字节包含 R/W 位的表示，不是 256 个七位设备地址。资料：https://www.nxp.com/docs/en/user-guide/UM10204.pdf。
- 前次 FAULT=1 为释放 SCL 超时，扩展扫描范围不能修复时钟异常。已请求当前 SDA/SCL/FAULT/ACK/RUN，尚未认定焊接、供电或软件根因。

- 换屏后当前 I2cScan 实板值：SDA=1、SCL=0、FAULT=1、ACK=0、RUN=0。时钟释放超时且当前读低，一轮扫描未完成；ACK=0 不能视为总线正常完成扫描后的无设备结论。
- 下一步请求测 R3 接 PB2/SCL 的信号端、MCU PB2 本体对地电压，区分真实低电平与配置/读值/节点连通问题。已有 MCU IOVDD=3.0V 测量不能替代 SCL 信号测量；当前没有据此判定外设损坏。

### 电压反馈更正与 I2cSoft 对照版（2026-09-29）

- 用户更正之前的读数：MCU PB2/SCL 实测 0V，PB1/SDA 3V，与软件一致。此前“PB2 实测高但 GPIO 读低”的推断撤回；R3 信号端更正后的电压尚未单独确认。
- 按用户要求继续试软件 IIC。原 I2cScan 已是 GPIO 模拟 I2C；新增 I2cSoft 配置，保持输入采样开启，OUT 常为 0，时钟/数据仅切 DIR，并放慢到每阶段请求 50us。初始化及方向设置 API 错误中止事务并释放输出。
- 保留 10% 背光、LED 1 秒间隔、厂家屏幕初始化，禁用蓝牙/USB CDC；增加实时 GPIO 状态页与原地址扫描页交替，以对照实际线路和 MCU 输出状态。
- 此记录写入时正在编译，尚未烧录/实板验证；不宣称已修复真实 SCL 低电平。

- I2cSoft 已全量构建并烧录成功：ELF 5182004 字节、14 条原有 SDK 栈警告；SHA256 A6F13329E8FD1E4A1EEE65C8F3C54E1AEEC1FD3ABACCF9CB6E882EE5BF08E52F。下载器识别 4M，Download completed，OTP_CFG 写入成功，无 ERROR。记录 output/bringup/build-I2cSoft.json 与 download-I2cSoft.json/log。等待退出 Update 重新上电，反馈扫描页和 LIVE GPIO 页；尚未验证 SCL 释放或器件应答。

- I2cSoft 实板反馈：SDA=1/SCL=0/FAULT=1/ACK=0/RUN=0；API=10、RAW=10、DIR=11、DIE=11、DIEH=11，OUT 未反馈。慢速软件方向切换后仍 SCL 低；GPIO 方向已为输入，输入缓冲开启，不能将 ACK=0 解释成完成扫描后的无设备。尚未排除引脚复用/外部拉低/上拉供电或连通性问题。下一步测 R3 两端对地电压及断电后 SCL 对地电阻，以定位真实低电平来源。
