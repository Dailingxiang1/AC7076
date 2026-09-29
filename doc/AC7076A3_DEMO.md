# AC7076A3 Board1 外设调试 demo

依据 `SCH_Schematic1_2026-09-10.pdf` 的 Board1 / P1，以及当前 SDK 3.2.1。
这是直接集成进 `SDK/AC707N.cbp` 和 Makefile 的独立外设验证入口。
总宏为 1 时，`app_main()` 创建 demo 任务，替换手表的 `app_task_loop()`；
不启动原手表 UI、健康算法、蓝牙业务或自动触摸固件升级。
总宏为 0 时恢复原工程入口及原板型配置。

## 1. 当前屏幕点亮配置

唯一的用户开关文件：

`SDK/apps/watch/bringup/ac7076a3_demo_config.h`

当前先点屏：`DEMO_USB_CDC_ENABLE=0`、`DEMO_LCD_ENABLE=1`、
`DEMO_LCD_INTERNAL_RAM_FILL=0`、`DEMO_LCD_COLOR_CYCLE_ENABLE=1`。
当前使用厂家 BOE1.8-N81 的完整 JD9855/QSPI 初始化序列，五色像素填充每色
停留 3 秒，DBI 目标帧率为 10；背光固定 80%，关闭此前的 20%/80% 分段诊断。
PA2 未引出，独立 demo 在 CDC 关闭时不初始化调试 UART，日志暂不输出。
以 PB0 LED 和屏幕颜色作为可见验收信号。

| 开关 | 默认 | 预期动作 / 验收 |
| --- | --- | --- |
| `AC7076A3_DEMO_ENABLE` | 1 | 总开关；1 为独占外设 demo，0 恢复原程序 |
| `DEMO_LED_ENABLE` | 1 | PB0 低有效，每秒翻转；观察 LED1 |
| `DEMO_KEY_ENABLE` | 1 | PB7 下拉按键，30 ms 消抖；打印 DOWN/UP |
| `DEMO_MOTOR_ENABLE` | 1 | 每次按下 PB7，MTPG 输出 25% PWM、150 ms 短振动 |
| `DEMO_I2C_SCAN_ENABLE` | 1 | 扫描 0x08~0x77 七位地址，打印 ACK；ACK 不等于功能正常 |
| `DEMO_I2C_SCREEN_ENABLE` | 0 | `I2cScan` 配置启用，屏幕累计显示读/写 ACK 地址和触摸 ID |
| `DEMO_I2C_PIN_TEST` | 0 | `I2cPins` 配置启用，不发送 I2C 事务，只检查 PB1/PB2 输入及寄存器 |
| `DEMO_TOUCH_ENABLE` | 1 | 板载 CST816S：复位、读 ID/版本、轮询手势/触点/坐标及 IRQ 电平 |
| `DEMO_ACCEL_ENABLE` | 1 | DA213B ID=0x13 后才写配置；±2g、62.5 Hz，打印 XYZ mg |
| `DEMO_ACCEL_SCREEN_ENABLE` | 0 | `Accel` 配置设为 1，在屏幕显示 ID、状态、XYZ mg、样本数和读取错误数 |
| `DEMO_ACCEL_POLL_MS` | 20 | DA213B 轮询间隔；允许 20～1000 ms |
| `DEMO_POWER_ENABLE` | 1 | 每秒读 VBAT 与 USB VPWR 电压；只检测，不开启充电 |
| `DEMO_SPEAKER_ENABLE` | 0 | PB7 触发约 200 ms、1 kHz 低音量正弦波，随后停止 |
| `DEMO_MIC_ENABLE` | 0 | PA4 单端省电容 MIC、16 kHz，报告数据块计数、峰值及平均绝对值 |
| `DEMO_USB_CDC_ENABLE` | 0 | 暂停 USB CDC；不占用 PA2 |
| `DEMO_LCD_ENABLE` | 1 | JD9855 QSPI 360×360 圆屏；当前慢速五色像素填充 |

不要直接改生成的 `sdk_config.h`。demo 的局部差异在
`ac7076a3_demo_overrides.h`，只在总宏启用时生效。
音频关闭时不会初始化音频模拟模块；LCD 关闭时不会执行屏幕初始化。
关闭触摸、加速度和扫描全部开关后，不初始化 I²C 总线。

## 2. 原理图对应关系

### DA213B 专项验收（Accel 配置）

2026-09-28 用户已确认厂家初始化序列版的屏幕和 LED 正常。
`Accel` 保留该初始化表、80% 背光和 PB0 每秒翻转；关闭其他传感器测试。
PB1/PB2 软件 I²C 为 50 kHz，使用七位地址 `0x27`；先读 `0x01`，
只有 ID=`0x13` 才设置 ±2g、62.5 Hz、正常工作且关闭自动睡眠。
每 20 ms 连续读 `0x02`～`0x07` 的六个字节，按有符号 14 位换算成 mg；
屏幕每 250 ms 刷新，不依赖 PA2 或 USB CDC。配置写入均读回校验。

1. 下载 `Accel` 后退出 Update 并重新上电，屏幕应显示 `DA213B MG`、`ID:13 ADDR:27` 和绿色 `READY`。
2. `N` 是成功读取计数，应持续增加；它不是芯片数据就绪计数。`ERR` 是数据读取失败累计数，正常应保持 0。
3. 静置时 XYZ 应稳定，重力主要所在轴的绝对值约 1000 mg；其余轴接近 0。板子安装方向决定轴和正负号，不能只以 Z 必须为正判断。
4. 慢慢翻面，重力主要所在轴应改变符号；向不同方向倾斜时，重力应在三个轴之间重新分配。轻晃时读数应有明显变化。
5. `NO DEVICE`：地址无响应，每 3 秒重试。`BAD ID`：显示实际 ID，不写配置。`CONFIG ERR`：配置读回失败。
   `BUS LOW`：检查 PB1/PB2、R3/R4 和供电。`READ ERROR` 或 `STALE DATA` 时 XYZ 显示横杠，不显示旧值。

芯片参数依据 [MiraMEMS DA213B 数据手册 Rev0.2](https://semic-boutique.com/wp-content/uploads/2021/01/da213B.pdf)。
屏幕文字使用固件内的小字库，无需手表 UI 资源。设置 `DEMO_ACCEL_SCREEN_ENABLE=0` 或编译 `Screen` 可回到五色测试。

```powershell
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Accel -Download -KeyFile 'C:\JL\keys\AC707N.key'
```

已验收的屏幕固件备份：`output/bringup/screen-vendor-known-good/sdk.elf`；
该备份是含调试信息的 ELF，供原 SDK 下载批处理打包，不能直接拖入烧录器 U 盘。

| 外设/信号 | MCU 网络 / 引脚 | 板上连接 |
| --- | --- | --- |
| USB 日志 | USB0 D-/D+ / U1.15、U1.16 | Type-C 数据线接电脑；CDC 调试当前暂停 |
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
# DA213B 专项：屏幕显示加速度，保留 LED，关闭触摸/扫描/马达/音频/CDC
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Accel
# I2C 扫描 + 触摸识别，屏幕显示结果
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile I2cScan
# PB1/PB2 输入诊断：不发送 I2C，比较 GPIO API 和原始寄存器
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile I2cPins
# 基础 + 喇叭、麦克风
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Audio
# 基础 + 喇叭、麦克风 + USB CDC
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile AudioUsb
# 恢复原工程逻辑进行编译验证
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Original
```

如以后恢复虚拟 COM 调试，可选 `AudioUsb`；五色验证使用 `Screen`，加速度验证使用 `Accel`。

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
   通过主控 QSPI 像素路径按红、绿、蓝、白、黑顺序每 3 秒填充一次。
   先观察五色是否明显、颜色是否纯正、持续闪烁是否减轻。
   若仅背光亮而无画面，先查 RESET、CS/CLK/D0~D3、模组专属初始化表；
   当前表已按用户提供的 BOE1.8-N81 厂家序列完整转换。
9. **再次断电上电**：确认日志、LED 和所启用的外设可重复运行。

## 5. JD9855 屏幕点亮进度

已接入新文件 `SDK/apps/watch/bringup/lcd_qspi_jd9855.c`：

- 注册独立 `jd9855` 屏驱，不复制 ST77916/JD9853 的模拟初始化表。
- 使用杰理 DBI/QSPI 专用接口，RGB565，命令 0x02、四线像素写 0x32。
- CS=PA7、CLK=PA12、D0~D3=PA8~PA11，RESET=PC3；读回默认 PA9。
  FPC 的 RS/DCX 接 PB8；当前 `DEMO_LCD_RS_HOLD_HIGH=0`，保留现有 SDK QSPI 配置。
- FPC2 逻辑电源固定在 3V3；背光使用 LCDPG 开漏 PWM。
- 首次不等待 TE，防止没接屏或尚未初始化时无限等待 TE。
- 屏幕任务独立于 PB0 LED 心跳。曾启用红/绿/蓝/白/黑循环，实板能看到
  五次颜色变化，但用户报告颜色异常、画面不纯且持续闪烁。
- 前一版 `DEMO_LCD_COLOR_CYCLE_ENABLE=0`，主控仅写一次白屏；用户回报
  背光亮但没有可识别画面。
- JD9855 内部黑白交替填充版实板无可见变化，但第一版主控像素路径能看到
  五种变化。内部填充命令不适合继续作为唯一判据，现已关闭该宏。
- 上一版 `DEMO_LCD_FPS=5`、80% 背光、每色停留 3 秒，实板没有可见颜色变化。
- 此前背光分段诊断版帧率恢复为 10，每色停留 3 秒，背光每 15 秒在
  20%/80% 之间切换。
- 2026-09-28 用户提供 BOE1.8-N81 厂家初始化文件，已整表转换到
  `jd9855_panel_init.h`。50 条命令、全部参数和 4 段延时按原顺序保留，
  页 0 Gamma 为完整 32 字节，包含厂家 RAM 清零步骤。
- 当前 `DEMO_LCD_COLOR_CYCLE_ENABLE=1`、`DEMO_LCD_RS_HOLD_HIGH=0`、
  `DEMO_LCD_BRIGHTNESS_PHASE_TEST=0`，背光固定 80%；PB0 每秒翻转，USB CDC 关闭。

用户已确认屏幕是 JD9855/QSPI、360×360 圆屏，R5 已焊接；最新厂家文件标注
BOE1.8-N81 / V1.0 / QSPI / G2.2 / 20250606。当前表采用厂家全部命令与延时；
**编译与静态核对不代表实板显示已经正确**。
`TCFG_UI_ENABLE=0` 时已仅为此 demo 保留 SDK LCD 硬件接口，继续保持 4 MiB
无表盘资源布局。厂家未给出硬复位波形与 `SSD_CMD/SSD_PAR` 底层实现，
本轮保留现有 PC3 复位、RGB565 和 QSPI 传输配置，实板画面仍需验证。

厂家原始文件保留在 `doc/BOE1.8-N81_JD9855_360x360_initial_code_V1.0_QSPI_G2.2_20250606（6代线）(1).c`。
厂家初始化窗口末值 `0x0168` 原样保留；Demo 分辨率继续为 360×360，
测试绘制区域仍为 `0..359`，不据初始化末值改成 361。SDK 命令格式为：

```c
static const u8 jd9855_panel_init[] ALIGNED(4) = {
    /* 格式示例： */
    /* _BEGIN_, command, parameter0, parameter1, _END_, */
    /* _BEGIN_, REGFLAG_DELAY, 120, _END_, */
};
```

初次色彩测试不要使用原 320×386 手表图片来判断屏幕尺寸。
如果白屏出现彩色杂点，单纯 RGB/BGR 顺序不足以解释，应查 QSPI 波形、
数据线及初始化参数；若白屏周期性忽明忽暗，应查 3V3、背光 R5/LCDPG
及模组电源。偏移不对查窗口起点；完全黑屏先查背光、RESET、QSPI 波形。

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
   `SDK/cpu/br35/ui_driver/lcd_drive/`。JD9855 初始化数据来自上述 BOE1.8-N81 厂家文件。
- GPIO、power gate、GPADC、音频、USB CDC：复用本地 SDK 接口和示例，
  未用其他杰理系列的 API 名称或 ADC 分压倍数替换本工程定义。
- 烧录 KEY 不匹配：[杰理官方《为什么无法下载程序》7.6.3](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/faqs/why_failed_to_download.html)。

构建验证结果请见工程根目录下的 `doc/AC7076A3_DEMO_VALIDATION.md`。

## I2cScan：扫描总线并识别触摸（2026-09-28）

DA213B 测试版实板显示 `NO DEVICE`；只确认读 ID 通信失败，尚不能确定焊接原因。
新配置 `I2cScan` 保留 LED、80% 背光和厂家屏幕初始化，关闭 DA213B 配置写入、USB CDC、马达和音频。

扫描 PB1 SDA / PB2 SCL 的 112 个常规七位地址 `0x08～0x77`，每个地址分别探测读和写 ACK。
I2C 地址字节的最低位是读写位，255 个地址字节并不代表 255 个不同设备；保留地址不作为普通设备探测。
依据 [NXP UM10204 I2C 总线规范](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)。
未知地址的写探测只发送地址，不发送寄存器或配置数据；读探测接收一字节后 NACK 和 STOP。
软件驱动 ACK=1 的约定和 STOP 释放 busy 状态已核对 SDK 实现。

每约 20 ms 测试一个地址，完成一轮后间隔 3 秒重新扫描。
每轮先通过 PA6 复位触摸，再从 `0x15` 的 `0xA7～0xA9` 读取 ID、项目号和固件版本；不烧录触摸固件。
扫描期间不运行原触摸业务，目的是先验证通信。总线被拉低时暂停扫描并重试初始化，避免把 SDA 常低误认为所有地址都 ACK。

屏幕字段：

- `SDA/SCL`：事务外的总线电平，正常空闲应均为 1。
- `TP:xx FW:xx`：最近一次复位后成功读取的触摸 ID/固件版本；失败显示 `TP: NO ID`。ID 数值本身仍须结合触摸型号和固件确认。
- `RUN`：已完成的扫描轮数；`IRQ`：PA5 当前电平，单靠 IRQ 不能证明触摸通信正常。
- 地址后缀 `W` 为写地址 ACK，`R` 为读地址 ACK，`B` 为两者都曾 ACK；例如 `15B` 是七位地址 `0x15`，不是 `0x15B`。
- 地址累计保留到重启，每页显示 12 个，超过一页时每 4 秒翻页。累计 ACK 不表示设备此刻仍在线。
- `ACK`：曾响应的不同地址数量；`P`：页号/总页数。
- 无响应时显示 `NO ACK`，并显示当前探测地址 `AT`、事务启动错误计数 `ERR`；正常 NACK 不计为启动错误。

退出 Update 重新上电后观察至少两轮；轻触屏幕尝试唤醒，然后记录 `TP`、`SDA/SCL`、`ACK` 和地址列表。
发现 `0x15` 只说明该地址有响应，读取 ID 及后续报点验证才有助于确认触摸功能。
若所有地址均无响应，优先检查共用 I2C 总线、R3/R4、触摸复位和供电；仍需测量才能定位。

```powershell
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile I2cScan -Download -KeyFile 'C:\JL\keys\AC707N.key'
```

> 实板补充：用户报告 SDA=1、SCL=0、TP:NO ID，目前须先测量 SCL 电平和上拉供电。
> SDK 的软件 I2C 实现实际忽略 master_frequency，iic_get_delay 返回 0；本文之前所述 50 kHz 仅是配置值，不能视为实测或已实现的速率。延时与总线恢复路径待后续修正验证。
> 已修复电平恢复后重复初始化导致已占用错误的重试路径，并通过编译；该修正版尚未烧录，实际波特率仍待修正和实测。

## I2cPins：输入读取诊断

用户随后测得上拉电阻处 SDA/SCL 对地均为 3.3 V；尚未确认测量的是电源端还是信号端，也未确认 MCU 引脚处的电压。
电源端是固定供电，不能用于判定 SDA/SCL；信号端与 MCU 引脚之间也需确认连通。
当前不能据此认定总线短路，亦不能认定 GPIO 软件错误。

`I2cPins` 使用同一屏幕/LED配置，把 PB1/PB2 保持为上拉输入，不初始化软件 I2C、不读触摸 ID、不发送 START/地址/STOP。
每秒重新配置上拉输入并等待 10 ms，再采集状态；屏幕显示两列，左为 PB1/SDA，右为 PB2/SCL：

- `API`：gpio_read 的返回值，保留负数错误，不把错误当成高电平。
- `RAW`：JL_PORTB->IN 的 PB1/PB2 原始位。
- `DIR`：方向位，1 为输入；`DIE`、`DIEH`：数字输入相关使能位。
- `INPUT UP` 表示配置 API 未返回错误；不等于芯片脚电压或器件功能已经验证。

空闲输入且引脚均确认为 3.3 V 时，预期 API、RAW、DIR、DIE、DIEH 都为 `1 1`。
若 API/RAW 不同，继续查库的读取映射；若都为 0 且输入已启用，核对 MCU 引脚电压、焊点/走线连通和 GPIO 复用/芯片输入状态。
硬件电压须实测；屏幕的数字读值无法替代万用表或示波器。

## GPIO I2C 扫描版（I2cScan 最新配置）

I2cPins 静态输入测试已实板确认：API/RAW/DIR/DIE/DIEH 均为 1 1。
最新 I2cScan 用 `DEMO_I2C_GPIO_ENABLE=1` 选择独立 GPIO I2C 实现；其他配置默认使用原 SDK 后端。
SDK 的未使用频率参数问题不再影响本扫描路径。低/高阶段各请求 10 us 延时，实际速率须实测。
SCL 释放后等待实际高电平；等待预算 5000 us，失败停止事务并释放两线。
寄存器读的最后字节 NACK；事务末尾生成 STOP 并检查两线回高。保持原触摸复位和 ID 读取顺序。
无响应页面新增 FAULT：0 尚无总线异常记录，1 SCL 超时，2 START 时 SDA 低，3 STOP 后未回高。
该码累计保留最近一次异常到重启，不等于设备 ID。普通地址不 ACK 不计入 FAULT。
下一步观察 SDA/SCL 是否保持 1 1，以及 TP、ACK 地址列表和 FAULT；触摸仍未验收。

## I2cDiag：SCL 切换与首次故障记录

GPIO 扫描版实板反馈 FAULT=1、ACK=0，尚不能确定硬件或软件原因。I2cDiag 先在 SDA 释放状态下测试 SCL 拉低与释放，不发送 START 或地址；随后尝试触摸识别和扫描。记录首次故障的现场，后续扫描轮询停止，LED 和屏幕继续运行。

```powershell
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile I2cDiag -Download -KeyFile 'C:\JL\keys\AC707N.key'
```

两个页面每 4 秒切换；每组两位的左位为 PB1/SDA，右位为 PB2/SCL。

- 页面 1：PRE 为初始输入，LOW 为 SCL 拉低后的读取，UP 为释放后约 10 ms，LATE 为再等待约 100 ms；预期依次为 11、10、11、11，CFG ERR=0。LOW 是数字输入寄存器读值，输出模式下输入使能可能变化，不能替代物理电压测量。
- 页面 2：F 故障码；A 七位地址（十六进制）；PH 阶段；BIT 位号，255 表示不在数据位阶段；API/RAW 为 GPIO API 与 IN 寄存器读值；DIR 为方向；DIE/H 为 DIE/DIEH；RET 为最近 SCL 模式配置返回值。
- F：1 释放 SCL 超时，2 START 时 SDA 低，3 STOP 后未回高，4 切换测试失败，5 SCL 配置 API 返回错误。
- PH：1 START，2 发送数据位，3 从机 ACK，4 接收数据位，5 主机 ACK/NACK，6 STOP，7 引脚切换测试。切换测试没有从机地址，A=00 不代表发现地址 0。

未出现故障时第二页显示正常扫描状态。首次故障现场保留至重启；正常地址 NACK 不作为故障。须结合两个页面及 MCU 引脚实际电压定位，不能仅凭 ACK=0 判断器件焊接损坏。

当前背光参数（电源对比测试）：DEMO_LCD_BACKLIGHT_DUTY=4000，即 40% PWM 占空比；I2cDiag 其他诊断参数保持原配置。降亮度后的电压及触摸结果仍待实测。

## Bluetooth：BLE 扫描、连接与回显

最新背光 PWM 为 1000/10000（10%）。Bluetooth 配置通过 DEMO_BLE_ENABLE=1 启用，关闭 I2C、触摸、DA213B、按键、马达、音频、USB CDC，保留 LED 每秒切换与 JD9855 厂家初始化。先验证 BLE，经典蓝牙音频/SPP 尚未测试。

```powershell
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile Bluetooth -Download -KeyFile 'C:\JL\keys\AC707N.key'
```

独立 demo 在 LED 和 LCD 启动后初始化 SDK 蓝牙栈，处理 app_core 收到的 BT_STATUS_INIT_OK，再调用 SDK TRANS_DATA 的 bt_ble_init 请求广播。未运行原手表应用、提示音、回连或产品 initcall。读取已有蓝牙 MAC；无有效 MAC 时仅在 RAM 中生成随机地址，不写入 VM/OTP，重启可能变化。名称在 RAM 中设为 AC7076A3，SDK 添加 (BLE) 后缀，手机看到 AC7076A3(BLE)。

屏幕字段：WAIT START/WAIT INIT 为启动阶段；INIT ERROR 为初始化返回负值；INIT TIMEOUT 为超过 15 秒未收到初始化完成事件；ADV REQUEST 仅表示 SDK 发出了广播请求，须用手机实际扫描确认；CONNECTED 为链路连接；NOTIFY ON 为 SDK 通知状态；MAC 两行显示地址；RX/BYTES 统计 AE01 收到的包数/字节数；RET 为栈初始化返回值，ST 为 SDK BLE 状态码。

测试步骤：

1. 用手机 BLE 扫描工具搜索 AC7076A3(BLE)，记录能否扫描及信号强度，核对屏幕 MAC。
2. 连接，检查屏幕变为 CONNECTED；发现服务 0xAE30。
3. 开启特征 0xAE02 的通知，然后向 0xAE01 写入十六进制 01 02 03 04（Write Without Response）。SDK 自带回显应在 AE02 返回相同字节，屏幕 RX 增加 1、BYTES 增加 4。
4. 断开后重新扫描并连接，检查能否恢复广播。

扫描到广播、连接成功、数据回显分别验收；编译通过或显示 ADV REQUEST 不等于无线链路实测通过。

当前使用配置：I2cScan（I²C 地址扫描 + LCD），背光 10%，DEMO_BLE_ENABLE=0。蓝牙版用户反馈死机/无画面，暂停使用 Bluetooth 配置，等待单独定位；当前回退不继续蓝牙测试。

## I2cSoft：慢速、方向切换的软件 I2C

I2cScan 原本已经是 GPIO 软件 I2C；I2cSoft 改用另一种 GPIO 实现作对照，并不使用硬件 IIC 控制器。DEMO_I2C_DIR_ONLY=1，初始化时将 PB1/PB2 OUT 设为 0、数字输入 DIE/DIEH 开启。通信时只调用 gpio_hw_set_direction：0 输出低，1 输入释放。仅操作指定引脚，保留 PB0 LED/PB8 LCD。两线不输出高电平，由上拉电阻拉高。

```powershell
& .\SDK\tools\build_ac7076a3_demo.ps1 -Profile I2cSoft -Download -KeyFile 'C:\JL\keys\AC707N.key'
```

每个低/高阶段请求 50us 延时，实际频率须测量，不宣称精确 10kHz。SCL 释放后仍检查是否实际回高，预算 5000us；失败释放两线并停止该事务，普通地址 NACK 不作为总线故障。保持 0x08～0x77 地址范围、触摸复位/ID、每秒 LED、10% 背光、厂家 LCD 初始化；蓝牙/USB CDC 禁用。

屏幕每 4 秒切换扫描页与 LIVE GPIO 页。后者显示 API、RAW、DIR、DIE/H、OUT、FAULT，左位 PB1/SDA，右位 PB2/SCL；均为事务外的实时快照，不是首次故障冻结现场。OUT 预期 0 0；DIR=1 1 表示 MCU 两线均设为输入释放。若 PB2 实测仍为 0V，且 DIR=1、DIE/H=1，则需进一步查外设、线路或其他复用是否拉低，软件不能把真实低电平当高电平继续通信。FAULT=0 也可能只是总线初始低、尚未启动任何事务，须结合 BUS LOW/ACK/RUN 判断。
