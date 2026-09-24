# AC7076A3 demo 构建验证记录

最近更新：2026-09-24（北京时间）

## 360×360 JD9855 屏幕试点亮包（2026-09-24）

- 用户确认屏幕为 JD9855/QSPI 360×360 圆屏，背光 R5 已焊接；尚无模组型号与本模组专属初始化表。
- `Screen` 配置关闭 USB CDC、PA2 调试 UART及手表 UI；PB0 LED 继续每秒翻转。LCD 硬件接口在 UI 关闭时为 demo 单独保留，屏幕初始化和 1.5 秒纯色循环运行于 `lcd_demo` 任务。
- 初始化表是另一块 JD9855/QSPI 360×360 圆屏的试点亮参数，不能替代本模组厂商参数。颜色、偏移及稳定性仍待实板观察。
- 全量构建通过：ELF 5,168,908 字节（含调试信息），SHA256 `D89DE7EDBD94E27206FB15D0743E36760ED89E5055BE7F0817088E0AA9169274`；`app.bin` 387,808 字节。INI 无手表 UI 资源分区，DATA 止于 `0x1FE000`。
- 使用用户已确认的 `C:\JL\keys\AC707N.key` 对在线 `BR35 UBOOT1.00` 下载，工具显示 4M、`Download completed.` 和 OTP 参数写入成功，无 `ERROR:`。等待用户正常重启后回报 LED、背光和颜色轮播。
- 构建日志 `output/bringup/build-Screen.log`；下载日志 `output/bringup/download-Screen.log`。

## USB 虚拟 COM 历史尝试（2026-09-24，当前已暂停）

- 用户反馈此前基础固件已成功烧录，PB0 LED 能规律闪烁；这是实板运行的直接观察。USB 日志版已多次烧录，但 Windows 未枚举出虚拟 COM。
- `Basic` 现默认开启 `DEMO_USB_CDC_ENABLE=1`，把 SDK `printf/putbyte` 输出先放入 4096 字节有界缓冲，再由 demo 任务分批写入 USB CDC；不初始化 PA2 调试 UART。USB OTG 后台负责连接/重连 CDC，未打开串口时不让 USB 写入阻塞外设任务。
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

## 当前留在工程中的固件（JD9855 屏幕试点亮版）

- 配置：`Screen`，总宏 1，4 MiB，USB CDC 关闭，JD9855 屏幕试点亮，无原手表 UI。
- 路径：`SDK/cpu/br35/tools/sdk.elf`。
- 大小：5,168,908 字节（含调试信息）。
- SHA256：`D89DE7EDBD94E27206FB15D0743E36760ED89E5055BE7F0817088E0AA9169274`。
- 构建命令：`& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Screen`，其后用生成的 `download.bat` 与 `C:\JL\keys\AC707N.key` 下载；运行时屏幕待验收。
- `output/bringup/last-build.json` 和各 `build-*.json` 保存参数、时间及散列；日志保存于同目录。不同配置共用一个 ELF 路径，后一次编译会替换前一次产物。

## 硬件待验收项

- USB CDC 暂停；PA2 未引出，当前固件无串口日志输出。
- LED、按键、马达、DA213B、VBAT/VPWR、音频、USB 枚举与回显按调试手册逐项验证。
- 板载 CST816S 已配置何种固件/电极参数、实际地址与报点协议；本 demo 不刷写触摸固件。
- JD9855 分辨率已确认 360×360；本模组的初始化表、像素偏移、方向、QSPI 时序及实板显示仍待核对。
- 背光回路 R5 已确认焊接，阻值仍待核对。

本轮未宣称已在 AC7076A3 实板跑通全部外设。
