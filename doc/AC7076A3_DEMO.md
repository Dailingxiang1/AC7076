# AC7076A3 Board1 外设调试 demo

依据 `SCH_Schematic1_2026-09-10.pdf` 的 Board1 / P1，以及当前 SDK 3.2.1。
这是直接集成进 `SDK/AC707N.cbp` 和 Makefile 的独立外设验证入口。
总宏为 1 时，`app_main()` 创建 demo 任务，替换手表的 `app_task_loop()`；
不启动原手表 UI、健康算法、蓝牙业务或自动触摸固件升级。
总宏为 0 时恢复原工程入口及原板型配置。

## 1. 当前屏幕点亮配置

唯一的用户开关文件：

`SDK/apps/watch/bringup/ac7076a3_demo_config.h`

当前先点屏：`DEMO_USB_CDC_ENABLE=0`、`DEMO_LCD_ENABLE=1`。
PA2 未引出，独立 demo 在 CDC 关闭时不初始化调试 UART，日志暂不输出。
以 PB0 LED 和屏幕颜色作为可见验收信号。

| 开关 | 默认 | 预期动作 / 验收 |
| --- | --- | --- |
| `AC7076A3_DEMO_ENABLE` | 1 | 总开关；1 为独占外设 demo，0 恢复原程序 |
| `DEMO_LED_ENABLE` | 1 | PB0 低有效，每秒翻转；观察 LED1 |
| `DEMO_KEY_ENABLE` | 1 | PB7 下拉按键，30 ms 消抖；打印 DOWN/UP |
| `DEMO_MOTOR_ENABLE` | 1 | 每次按下 PB7，MTPG 输出 25% PWM、150 ms 短振动 |
| `DEMO_I2C_SCAN_ENABLE` | 1 | 扫描 0x08~0x77 七位地址，打印 ACK；ACK 不等于功能正常 |
| `DEMO_TOUCH_ENABLE` | 1 | 板载 CST816S：复位、读 ID/版本、轮询手势/触点/坐标及 IRQ 电平 |
| `DEMO_ACCEL_ENABLE` | 1 | DA213B ID=0x13 后才写配置；±2g、62.5 Hz，打印 XYZ mg |
| `DEMO_POWER_ENABLE` | 1 | 每秒读 VBAT 与 USB VPWR 电压；只检测，不开启充电 |
| `DEMO_SPEAKER_ENABLE` | 0 | PB7 触发约 200 ms、1 kHz 低音量正弦波，随后停止 |
| `DEMO_MIC_ENABLE` | 0 | PA4 单端省电容 MIC、16 kHz，报告数据块计数、峰值及平均绝对值 |
| `DEMO_USB_CDC_ENABLE` | 0 | 暂停 USB CDC；不占用 PA2 |
| `DEMO_LCD_ENABLE` | 1 | JD9855 QSPI 360×360 圆屏，红/绿/蓝/白/黑循环；模组初始化参数待实板确认 |

不要直接改生成的 `sdk_config.h`。demo 的局部差异在
`ac7076a3_demo_overrides.h`，只在总宏启用时生效。
音频关闭时不会初始化音频模拟模块；LCD 关闭时不会执行屏幕初始化。
关闭触摸、加速度和扫描全部开关后，不初始化 I²C 总线。

## 2. 原理图对应关系

| 外设/信号 | MCU 网络 / 引脚 | 板上连接 |
| --- | --- | --- |
| USB 日志 | USB0 D-/D+ / U1.15、U1.16 | Type-C 数据线接电脑，正常固件枚举为 CDC 虚拟 COM |
| LED | PB0 / U1.8 | 3V3 → R6 → LED1 → PB0，低有效 |
| KEY | PB7 / U1.5 | SW1 按下接 GND；原 boot 的 8 秒长按复位仍保留 |
| MOTOR | MTPG / U1.27 | H4 马达负端，专用低侧开漏 PG；高阻关闭 |
| I²C SDA/SCL | PB1 / U1.7；PB2 / U1.6 | R4/R3 上拉，CST816S 与 DA213B 共用 |
| TP IRQ / RESET | PA5 / U1.24；PA6 / U1.23 | U2.20 / U2.17，触摸复位低有效 |
| LCD D0/D1/D2/D3 | PA8/PA9/PA10/PA11 | FPC2.4/.5/.6/.7 |
| LCD CLK / CS | PA12 / PA7 | FPC2.9 / FPC2.10 |
| LCD RESET / TE / RS | PC3 / PC0 / PB8 | FPC2.11 / FPC2.3 / FPC2.8 |
| LCD 背光 | LCDPG / U1.28 | 经 R5 到 LED_K；FPC2.15 的 LCD_A 接 3V3 |
| LCD 逻辑供电 | IOVDD/3V3 | 固定供电；不使用原演示板的 PC1/PC2 电源 GPIO |
| MIC | PA4 / U1.25 | H3 单端两线麦克风，负端接地 |
| Speaker | DACP / U1.30；DACN / U1.29 | H2 差分输出，负端不是地 |
| Battery / USB | AVDD/VBAT / U1.31；VPWR / U1.1 | H1 电池、Type-C 5V |
| USB D-/D+ | U1.15 / U1.16 | 经 USBLC6-2SC6 接 Type-C |

FPC 编号以图中器件引脚数字为准，而不是排针行号。
原理图 R5 数值为 `X`；用户已确认实板已焊接，阻值仍未测量。
DA213B 的 INT 未连接，所以本 demo 使用轮询，不假设存在加速度中断。

## 3. 编译与下载

在工程根目录 PowerShell 执行（默认只编译，**不会下载**）：

```powershell
& .\SDK\tools\build_ac7076a3_demo.ps1
```

脚本默认 `Configured`，按头文件当前值构建。其他可复现配置：

```powershell
# 基础恢复包：关闭屏幕、音频、USB CDC，保留 LED
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Basic
# 当前屏幕包：保留 LED，开启 JD9855，关闭 USB CDC
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Screen
# 基础 + 喇叭、麦克风
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Audio
# 基础 + 喇叭、麦克风 + USB CDC
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile AudioUsb
# 恢复原工程逻辑进行编译验证
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Original
```

如以后恢复虚拟 COM 调试，可选 `AudioUsb`；当前屏幕验证使用 `Screen`。

注意：`Original` 恢复的是原演示板参数，包括 PB03 日志及原屏驱，
不代表这些参数已适配你的新 PCB。`Screen` 和当前 `Configured` 开启 LCD；
Basic/Audio/AudioUsb 关闭 LCD。

脚本每次全量重编，因为普通 Make 不能自动追踪命令行 `-D` 的变化。
新增 `make build-only` 先串行生成共享配置，再并行编译，避开原 `all`
目标的配置生成与编译竞争。不要同时在同一目录运行多个配置的构建。
日志和最后构建清单位于 `output/bringup/`，主 ELF 仍是
`SDK/cpu/br35/tools/sdk.elf`；这不是可直接替代完整下载包的裸 bin。

完成接线、板子进入杰理 USB 下载模式后，才执行：

```powershell
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Screen -Download
```

若烧录器报 `Key mismatch`，说明这颗芯片已写入 KEY。须取得**首次烧录时匹配的
杰理 `.key` 文件**，放在无中文、空格的英文路径，再执行：

```powershell
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Screen -Download -KeyFile 'C:\JL\keys\AC707N.key'
```

上面的密钥路径是本板已成功使用、由用户确认的本机文件，不是工程自带密钥。
其他芯片不能随意套用这个 KEY；也不能通过全片擦除绕过芯片已写入的 KEY。

此时才调用原 SDK 生成的 `download.bat`。本板烧录器识别到的是 **4 MiB 内置 Flash**；
demo 编译时将 `CONFIG_FLASH_SIZE` 设为 0x400000，并关闭原手表 UI，
因此下载脚本自动选用不含 `mode.bin` / `watch.bin` 的无 UI 打包分支。
原演示工程的 8 MiB 资源布局无法直接烧到这块板上，不能只改一个容量数字。
脚本会检查生成的 INI 和下载脚本，若仍包含手表 UI 分区会停止烧录。
烧录器先按 `Update` 进入 BR35 UBOOT 模式，Windows 能看到 `BR35 UBOOT1.00`
时再运行上面的 `-Download` 命令。不要把生成的 `sdk.elf` 或单个 `app.bin`
直接复制到 U 盘；使用 SDK 的下载工具写入完整固件。若再次失败，先看
`output/bringup/download-Screen.log` 中第一条 `ERROR:`，并记录下载器识别的容量。
脚本发现根 `output/` 缺少 cfg_tool.bin / stream.bin 时，会从你移动到的
`doc/` 目录复制缺失文件，不覆盖已导出的新文件。`src/`、SDK 的 UI 资源及
boot/P11 文件应继续保留。以 `isd_download` 的成功提示和新启动日志为准；
SDK 的 batch 退出码不能单独证明下载成功。不自动格式化 VM 或全片。

## 4. 首板逐项验收

1. **基础上电**：PB0 LED 按约 1 秒间隔持续翻转。当前没有串口日志输出。
2. **按键/马达**：短按 PB7，看到 DOWN 和 UP，每次按下短振一次；
   连续按住不会反复触发，8 秒可能触发 boot 长按复位。
   马达 PWM 用专用 PG API，独立硬件定时回调关闭；不是对普通 GPIO 输出高电平。
3. **I²C**：应能看到 DA213B 0x27，触摸通常为 0x15；触摸睡眠时可能 NACK。
   SDA/SCL 被拉低时打印 BUS LOW 并跳过总线测试，先查供电和上拉。
4. **加速度**：ID 读到 0x13；翻转板子，XYZ 的符号随方向变化。
   静止时重力对应轴约 ±1000 mg，传感器安装方向决定具体轴。
   数据为左对齐有符号 14 位，当前量程每 g 为 4096 个计数。
5. **触摸**：确认 CST816S 内已有与你的 FPC1 电极匹配的固件/参数。
   报告原始坐标，先验证单击、滑动及手势，不先接手表 UI。
   本次不写触摸固件，不调用 SDK 的 CST816D 自动升级流程。
   若是空白芯片或定制电极算法，仍需海栎创/模组供应商提供对应固件和协议。
6. **电源**：VBAT 与万用表对照；BR35 的 `AD_CH_PMU_VBAT` 是 VBAT/2，
   代码用 SDK 的 `AD_CH_PMU_VBAT_DIV` 恢复，不能照其他平台示例乘 4。
   VPWR 使用 VPWR/4 通道，再乘 4；USB_5V 只是电压判断，不代表数据枚举成功。
7. **音频**：先打开 Audio 配置，H2 接匹配扬声器；按键发短音，
   听不到时核对差分接线、DAC 设置与负载。对 H3 说话/轻触，MIC 的
   blocks 应增长，peak/mean_abs 应变化；这些是幅值指标，不是声压级或 RMS。
   此代码按图中的 PA4 两线驻极体/省电容接法配置；换数字 MIC 需改硬件与驱动。
8. **屏幕**：使用 `Screen` 包。LED 应持续闪烁；约第二次心跳后启动 LCD 任务，
   背光亮起，红、绿、蓝、白、黑每 1.5 秒循环。若仅背光亮而无颜色，先查
   RESET、CS/CLK/D0~D3、模组专属初始化表；当前表是相同 IC/分辨率的试点亮值。
9. **再次断电上电**：确认日志、LED 和所启用的外设可重复运行。

## 5. JD9855 屏幕点亮进度

已接入新文件 `SDK/apps/watch/bringup/lcd_qspi_jd9855.c`：

- 注册独立 `jd9855` 屏驱，不复制 ST77916/JD9853 的模拟初始化表。
- 使用杰理 DBI/QSPI 专用接口，RGB565，命令 0x02、四线像素写 0x32。
- CS=PA7、CLK=PA12、D0~D3=PA8~PA11，RESET=PC3；读回默认 PA9。
- FPC2 逻辑电源固定在 3V3；背光使用 LCDPG 开漏 PWM。
- 首次不等待 TE，防止没接屏或尚未初始化时无限等待 TE。
- 红、绿、蓝、白、黑色循环已启用，屏幕任务独立于 PB0 LED 心跳。

用户已确认屏幕是 JD9855/QSPI、360×360 圆屏，R5 已焊接，但模组没有具体型号。
当前试点亮表来自另一块同 IC/分辨率圆屏，包含电源、GIP、Gamma、
sleep-out/display-on 和延时；**不能据此断言本模组已经点亮或颜色准确**。
`TCFG_UI_ENABLE=0` 时已仅为此 demo 保留 SDK LCD 硬件接口，继续保持 4 MiB
无表盘资源布局。若背光亮而画面黑或花，需要换本模组厂商的初始化参数。

如以后拿到模组厂商的完整初始化表、方向、像素偏移和 QSPI 时序，
应替换 `jd9855_panel_init.h` 的试点亮值。SDK 命令格式为：

```c
static const u8 jd9855_panel_init[] ALIGNED(4) = {
    /* 格式示例： */
    /* _BEGIN_, command, parameter0, parameter1, _END_, */
    /* _BEGIN_, REGFLAG_DELAY, 120, _END_, */
};
```

初次色彩测试不要使用原 320×386 手表图片来判断屏幕尺寸。
若色彩不对，检查 RGB/BGR、RGB565 字节次序及 MADCTL；偏移不对查窗口起点；
完全黑屏先查背光 R5/LCDPG、RESET、QSPI 波形和初始化表。

## 6. 代码入口与边界

- `ac7076a3_demo.c`：独占主任务、GPIO、I²C、CST816S、DA213B、VBAT/VPWR。
- `ac7076a3_demo_audio.c`：音频输出/输入幅值；中断里只采集统计，任务中打印。
- `ac7076a3_demo_usb.c`：USB CDC 日志缓冲、任务内发送、回显与超时提示。
- `apps/common/debug/debug_uart_config.c`：demo 启用 CDC 时将 `putbyte` 接到日志缓冲，不占用 PA2；原工程分支仍使用原调试 UART。
- `ac7076a3_demo_config.h`：用户开关和接线参数。
- `ac7076a3_demo_overrides.h`：覆盖演示板的冲突引脚、关闭原触摸升级/低功耗。
- `lcd_qspi_jd9855.c`、`jd9855_panel_init.h`：屏驱接入与独立模组初始化数据。
- `SDK/tools/build_ac7076a3_demo.ps1`：可复现配置及显式下载入口。

编译通过只能证明软件配置、接口及链接可用；硬件行为尚未实测。
Bluetooth/RF 的射频和协议测试继续使用 SDK 对应工具及模式，不由本独占
外设 demo 自动启动。真实充电参数、睡眠/唤醒、RTC 保时、整机功耗和量产
校准不属于这轮基础外设验证；不要把当前持续打印的 demo 用作低功耗基线。

## 7. 寄存器与实现依据

- DA213B：MiraMEMS 原厂数据手册 DS_da213B，2018-01 Rev0.2，
  I²C 地址、CHIPID、14 位数据、RANGE、ODR_AXIS、MODE_BW。
  [原厂手册镜像](https://semic-boutique.com/wp-content/uploads/2021/01/da213B.pdf)
- CST816S：Hynitron 原厂数据手册，
  [CST816S V1.3](https://www.espruino.com/datasheets/CST816S.pdf)；
  报点寄存器使用 CST816S 常见协议，参考
  [CST816S 驱动源码项目](https://github.com/fbiego/CST816S)，需用实装固件核对。
- QSPI 时序枚举和接口：本地 `SDK/interface/ui/cpu/br35/dbi.h`，
  `SDK/cpu/br35/ui_driver/lcd_drive/`。JD9855 模组初始化数据尚未提供。
- GPIO、power gate、GPADC、音频、USB CDC：复用本地 SDK 接口和示例，
  未用其他杰理系列的 API 名称或 ADC 分压倍数替换本工程定义。
- 烧录 KEY 不匹配：[杰理官方《为什么无法下载程序》7.6.3](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/faqs/why_failed_to_download.html)。

构建验证结果请见工程根目录下的 `doc/AC7076A3_DEMO_VALIDATION.md`。
