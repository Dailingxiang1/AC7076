from pathlib import Path
from xml.sax.saxutils import escape
from reportlab.pdfgen import canvas
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.enums import TA_LEFT
from pypdf import PdfReader
import json

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'output/pdf/AC7076A3_工程编译下载快速上手.pdf'
OUT.parent.mkdir(parents=True, exist_ok=True)
pdfmetrics.registerFont(TTFont('CN', 'C:/Windows/Fonts/msyh.ttc', subfontIndex=0))
pdfmetrics.registerFont(TTFont('CNB', 'C:/Windows/Fonts/msyhbd.ttc', subfontIndex=0))
pdfmetrics.registerFont(TTFont('Mono', 'C:/Windows/Fonts/consola.ttf'))
NAVY=colors.HexColor('#142D47'); TEAL=colors.HexColor('#087F8C'); GRAY=colors.HexColor('#526478'); PALE=colors.HexColor('#EEF5F8')
styles = {
 'body':ParagraphStyle('body',fontName='CN',fontSize=10,leading=16,spaceAfter=8,textColor=NAVY,wordWrap='CJK'),
 'small':ParagraphStyle('small',fontName='CN',fontSize=8.3,leading=12.5,spaceAfter=5,textColor=GRAY,wordWrap='CJK'),
 'title':ParagraphStyle('title',fontName='CNB',fontSize=25,leading=35,spaceAfter=12,textColor=NAVY),
 'h1':ParagraphStyle('h1',fontName='CNB',fontSize=19,leading=27,spaceAfter=14,textColor=NAVY),
 'h2':ParagraphStyle('h2',fontName='CNB',fontSize=12,leading=18,spaceBefore=9,spaceAfter=7,textColor=TEAL),
 'cell':ParagraphStyle('cell',fontName='CN',fontSize=9,leading=14,wordWrap='CJK',textColor=NAVY),
 'head':ParagraphStyle('head',fontName='CNB',fontSize=9,leading=14,textColor=colors.white,wordWrap='CJK'),
 'code':ParagraphStyle('code',fontName='Mono',fontSize=8.5,leading=13,spaceAfter=0,textColor=NAVY,wordWrap='CJK'),
}
story=[]
def p(text,style='body'):
    story.append(Paragraph(text,styles[style]))
def h(text): p(text,'h2')
def page(n,title):
    if story: story.append(PageBreak())
    p(f'{n:02d} / 快速上手','small'); p(title,'h1')
def table(rows,widths):
    data=[[Paragraph(str(t),styles['head' if i==0 else 'cell']) for t in row] for i,row in enumerate(rows)]
    t=Table(data,colWidths=widths,hAlign='LEFT')
    t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),NAVY),('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),10),('RIGHTPADDING',(0,0),(-1,-1),10),('TOPPADDING',(0,0),(-1,-1),8),('BOTTOMPADDING',(0,0),(-1,-1),8),('ROWBACKGROUNDS',(0,1),(-1,-1),[PALE,colors.white]),('LINEBELOW',(0,0),(-1,0),.5,TEAL)]))
    story.append(t); story.append(Spacer(1,9))
def code(text):
    paras=[Paragraph(escape(line).replace(' ','&#160;') or '&#160;',styles['code']) for line in text.splitlines()]
    t=Table([[paras]],colWidths=[499]);t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,-1),PALE),('BOX',(0,0),(-1,-1),.4,colors.HexColor('#D6E3EA')),('LEFTPADDING',(0,0),(-1,-1),12),('TOPPADDING',(0,0),(-1,-1),10),('BOTTOMPADDING',(0,0),(-1,-1),10)]));story.append(t);story.append(Spacer(1,9))
def note(text):
    t=Table([[Paragraph(text,styles['body'])]],colWidths=[499]);t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,-1),colors.HexColor('#E7F4F0')),('LINEBEFORE',(0,0),(0,0),3,TEAL),('LEFTPADDING',(0,0),(-1,-1),12),('TOPPADDING',(0,0),(-1,-1),10),('BOTTOMPADDING',(0,0),(-1,-1),5)]));story.append(t);story.append(Spacer(1,9))

page(1,'AC7076A3 工程编译与下载')
p('先完成编译，再完成烧录，最后用日志确认启动。','h2')
p('对象：当前 AC707N3.2.1SDK 工作区。主工程属于 AC707N / BR35 平台，使用 pi32v2 R3 工具链；当前选择 AC707N 手表演示板。芯片具体封装、引脚编号和板上外设仍须以你的 AC7076A3 原理图及规格书为准。')
note('本机实测：主工程编译、链接成功，退出码 0，已生成 sdk.elf。存在 103 条链接阶段栈大小警告。本次跳过了打包和自动下载，尚未验证硬件烧录及开机。')
table([
 ['顺序','操作','完成标志'],
 ['1. 确认配置','核对 Flash、晶振、电源、屏幕和日志引脚；保留当前配置作基线。','配置与板子一致'],
 ['2. 编译主工程','打开 SDK/AC707N.cbp 的 Release，或使用第 4 页命令。','sdk.elf 更新；无编译/链接错误'],
 ['3. USB 下载','板子进入下载模式；运行生成的 cpu/br35/tools/download.bat。','下载器报告成功'],
 ['4. 正常启动','退出下载模式；重新上电，用 PB03 日志口观察。','看到启动日志并进入应用'],
 ['5. 再改功能','先加自己的启动标记，再逐项接入按键、屏幕、蓝牙。','修改可编译、可烧录、可复现']
],[65,295,139])
h('现在最该记住的三个位置')
p('主工程：<b>SDK/AC707N.cbp</b><br/>配置入口：<b>SDK/apps/watch/board/br35/board_config.h</b><br/>生成的下载入口：<b>SDK/cpu/br35/tools/download.bat</b>')
p('阅读顺序：工程结构第 2 页；板级参数第 3 页；编译第 4 页；下载第 5 页；启动与源码流程第 6 页；排错第 7 页；实测记录及资料第 8 页。','small')
p('整理日期：2026-09-09。工程元数据为基础版本 3.0.0、标准补丁 3.2.1，最近记录的可视化工具版本为 4.9.3。本文以本地源码、脚本及本次构建结果为主要依据。','small')

page(2,'工程结构：先从 SDK 入手')
p('以下路径均相对工程根目录。根目录有 project.jlproj；真正的主程序 Makefile 位于 SDK/ 下，命令行编译必须先进入 SDK。')
table([
 ['路径','职责与第一次上手的用法'],
 ['project.jlproj','杰理可视化工程入口，记录手表方案、AC707N、补丁版本和配置页面。已有匹配的 JLStudio 时可从这里打开。'],
 ['src/','板级、功能、按键、电源、LCD/UI、蓝牙、升级、音频等配置，以及音频流程。用于可视化工具编辑。'],
 ['output/','当前已有 cfg_tool.bin、stream.bin 等配置/音频流程产物；下载脚本默认也把最终固件复制到这里。本文 PDF 位于 output/pdf/。'],
 ['SDK/apps/watch/','主应用：app_main.c、模式切换、板级配置、UI 与业务逻辑。首次开发重点看这里。'],
 ['SDK/apps/common/','公共功能：按键、存储、蓝牙、音频、升级、UI、第三方协议等。'],
 ['SDK/cpu/br35/','芯片启动和平台代码、链接脚本、liba/ 静态库、tools/ 打包下载工具。'],
 ['SDK/interface/','系统、驱动、媒体、蓝牙等接口头文件，查 API 时使用。'],
 ['SDK/build/ 与 SDK/tools/','Makefile 辅助规则、动态源文件列表、工具程序。SDK/objs/ 是 Make 构建的中间文件目录。'],
 ['SensorHub/','独立子工程；其后处理能生成并复制 P11 固件到主 SDK 的 p11_code.bin。首次先使用主 SDK 已带的文件。'],
 ['loader/','BLE/EDR/NOR 等 OTA loader 和升级 UI 工程。首次 USB 下载主程序通常不需要重编这些子工程。'],
 ['cache/','可视化工具缓存与备份，不是主编译入口。']
],[141,358])
h('配置如何到达编译和固件')
p('可视化工程与配置 → sdk_config.h / sdk_config.c / 音频节点配置及 output/ 二进制资源 → app_config.h → board_config.h → 选中的板级配置 → 编译、打包。')
p('board_config.h 提醒 sdk_config.h 与可视化工具有关；当前生成文件也带有同步语法约束。长期修改建议从可视化配置或自己的板级代码落地，并核对生成结果。不要只改 src/ JSON 就认为头文件与资源已同步。','small')

page(3,'先核对这块板子的参数')
p('下表是本地当前配置，不是对 AC7076A3 硬件参数的推定。相同芯片系列也可能使用不同 Flash、屏幕和电源电路。')
table([
 ['项目','当前值','核对要点'],
 ['板型 / 平台','CONFIG_BOARD_JL707N_DEMO<br/>BR35 / pi32v2 R3','暂不切到 JL7074 或彩屏仓板型。'],
 ['Flash / RAM 扩展','Flash 0x800000 = 8 MiB<br/>PSRAM、外接 NOR、NAND 均关闭','以实际料号和板图核对；不能从 ELF 文件大小判断 Flash 占用。'],
 ['晶振 / 电源','24 MHz；PWR_DCDC15<br/>IOVDD 配置 3.2 V','确认晶振及 DCDC 外围匹配。'],
 ['应用日志','PB03；2,000,000 baud<br/>TCFG_DEBUG_UART_ENABLE=1','PB03 接 USB-TTL 的 RX，双方共地；串口器须支持此速率及合适的 IO 电平。'],
 ['下载 / VM','DOWNLOAD_MODEL=USB<br/>VM 32K，VM_OPT=1','保留默认下载模式；首次不要使用清空全部 Flash 的选项。'],
 ['显示 / 触摸','JLUI；ST77916；320×385<br/>CST816D 触摸','工程也带 320×386 UI 资源目录，屏幕与资源匹配需在硬件上验证。'],
 ['显示引脚','RESET PC03；CS PA07<br/>DC PA02；TE PC00<br/>背光 IO_LCD_PG','这是代码端口名，不是封装脚号；SPI 总线还需核对板级驱动配置。'],
 ['开机 / 电量','自动开机开启；ADKEY 开启<br/>电量检测开启；关机 3500 mV','低电或供电检测不符合预期可能影响开机；充电功能当前关闭。']
],[91,195,213])
h('复位配置有两个来源，先看实际生成结果')
p('当前生成 isd_config.ini 的 RESET 为 PB07_08_0，表示配置了 PB07 低有效、8 秒长按复位；来源是板级 global_build_cfg.h。sdk_config.h 中另一组 TCFG_LONG_PRESS_RESET_ENABLE 当前为 0，不能把它下面 PB01 的值当作已启用配置。')
p('首次下载前仍需确认：具体板型、实际 Flash、USB D+/D- 引出、电源方式、下载器版本、PB03 是否有测试点。本文没有板图，不能给出 AC7076A3 的物理管脚编号。','small')

page(4,'编译：本机已跑通的命令')
h('方式 A：PowerShell，先只验证编译与链接')
p('在当前工程根目录打开 PowerShell，执行以下命令。它使用 SDK 自带 make，并加入配套辅助工具路径；本次实测采用的就是此流程。')
code('Set-Location .\\SDK\n$env:PATH = "$((Get-Location).Path)\\tools\\utils;" +\n            "C:\\JL\\pi32\\bin;$env:PATH"\n.\\tools\\utils\\make.exe "RUN_POST_SCRIPT=echo BUILD_ONLY_SKIP_DOWNLOAD"')
p('成功末尾可看到 +POST-BUILD 和 BUILD_ONLY_SKIP_DOWNLOAD sdk。此命令覆盖后处理执行项，仍会生成 download.bat 和 isd_config.ini，但不会执行固件打包或访问下载器。普通 make 默认会接着下载。')
h('方式 B：Code::Blocks 图形界面')
p('1. 按杰理官方说明先安装 Code::Blocks，再安装杰理工具链 [W1、W2]。本机 C:/JL/pi32/bin/clang.exe 已可运行，C:/JL/ver.txt 为 2.5.2。')
p('2. 打开 SDK/AC707N.cbp，选择 Release。工程指定编译器标识为 pi32v2_r3_large_lto_compiler；需要杰理的 pi32v2 R3 large LTO 配置。')
p('3. 点击 Build，查看完整 Build log。工程的 Build options → Pre/post build steps 已配置生成脚本及自动下载；无硬件时，ELF 生成成功与随后下载等待应分开判断。图形界面路径本次仅核对工程配置，未实际操作验证。')
h('常用命令：均在 SDK 目录，先设置上面的 PATH')
code('.\\tools\\utils\\make.exe\n.\\tools\\utils\\make.exe VERBOSE=1\n.\\tools\\utils\\make.exe clean')
p('第一条为编译并执行下载；第二条显示详细命令；clean 用于清除 Make 的 ELF 和 objs/ 中间文件。切换板型或编译选项后再全量重编，首次不建议直接并行 make -j。')
h('编译过程与成功标准')
p('Makefile 预处理 build/genFileList.c → build/fileList.mk；随后 build/Makefile.mk 编译源文件、生成链接/下载配置并链接为 cpu/br35/tools/sdk.elf。检查退出码和 ELF 更新时间。sdk.map 用于查内存及符号；仅见到旧 ELF 不算本次成功。')
p('当前中文路径已完成本次命令行构建。若其他旧工具报路径错误，可把整套工程复制到短英文路径后再试；要连 src/、output/、loader/ 一起保留。','small')

page(5,'下载：沿当前 USB 流程操作')
h('1. 接线并进入下载模式')
p('按下载器标识连接 PC 端与目标板端：PC → 杰理 USB 强制升级工具 → 板上 USB 接口/下载测试点。确认供电、共地及 D+/D- 连接。具体芯片引脚必须查你的板图。PB03 是当前应用日志口，不能据此当作 USB 下载口。')
p('若使用杰理强制升级工具 4.0，可参考官方常规升级流程 [W3]：默认不拨低四位开关时，按工具按键触发复位和握手。其他版本以对应手册为准。若板子由电池持续供电，工具断电不一定能让芯片复位，还需按板子实际电路触发复位。')
p('进入下载模式后，查看工具指示灯及 Windows 设备管理器的设备枚举；不要仅凭 USB 串口出现就认为已进入芯片下载模式。')
h('2. 运行编译生成的下载入口')
p('完成第 4 页的编译后，在 SDK 目录的 PowerShell 执行：')
code('cmd /c cpu\\br35\\tools\\download.bat sdk')
p('或者双击 SDK/cpu/br35/tools/download.bat。当前配置会选择 download/watch/download_jlui.bat，它会打包 UI/配置资源并调用 isd_download.exe。不要直接挑 download_demo.bat、download_csc.bat，或绕过入口执行内部子脚本。')
h('3. 脚本会处理哪些文件')
table([
 ['文件 / 目录','用途'],
 ['sdk.elf → app.bin','先抽取各段并合并为应用二进制；app.bin 不等于带全部资源的完整固件。'],
 ['isd_config.ini','由 isd_config_rule.c 预处理生成；含芯片、USB 模式、SPI、复位、VM 等配置。'],
 ['ui_resource/、ui_upgrade/','界面、字体、表盘及升级界面资源，由 JLUI 子脚本打包。'],
 ['output/*.bin','配置及音频流程资源；配合 SDK 自带的 boot、loader、P11 等文件使用。'],
 ['jl_isd.fw / update.ufw','脚本声明的固件包与升级包输出；默认复制回工程根 output/。需以下载日志和实际文件为准。']
],[169,330])
p('下载脚本等待参数为 -wait 300。明确成功后退出强制下载模式，再正常上电。update.ufw 用于相应升级流程；首次空片下载按上述完整脚本执行。','small')
p('首次不运行 SDK/format_vm.bat，也不启用 FORMAT_ALL_ENABLE；这些选项会改变已有存储数据。修改下载配置应改源规则/板级宏，直接改生成的 ini 会被下一次编译覆盖。','small')

page(6,'启动验证与主要代码流程')
h('日志接线和串口设置')
p('板子 PB03 → USB-TTL RX；GND → GND。按板子 IO 电平选串口器，不把 5 V TTL 信号接到芯片 IO。应用日志单向观察时无需接串口器 TX。串口先按 2,000,000、8N1、无流控配置，确认串口器支持 2 Mbaud。')
p('波特率和 TX 引脚由 sdk_config.h 定义，debug_uart_config.c 引用。若电脑串口工具不支持此速率，可在配置中调整后重新生成、编译和下载，电脑端同步修改。')
h('当前主应用调用关系')
code('setup_arch()                  [cpu/br35/setup.c]\n  -> app_main()               [apps/watch/app_main.c]\n     -> task_create(app_task_loop, ..., "app_core")\n     -> os_start()\n        -> app_task_loop()\n           -> app_task_init()\n              -> app_version_check()\n              -> do_early_initcall()\n              -> board_init()\n              -> cfg_file_parse() / key_driver_init()\n              -> initcalls / dev_manager_init()\n              -> power / update / UI initialization\n           -> mode loop / message handling')
p('这是关键调用的简化顺序。board_init() 的有效实现位于选中板型的 board_ac707n_demo.c，负责电源、RTC、ADC 等初始化。应用后续按模式进入开机、蓝牙、RTC 等处理；同时 setup.c 中还有裸机入口，不要将 system_main() 当成本工程唯一应用入口。')
h('可观察的启动标记')
table([
 ['日志 / 现象','说明'],
 ['setup_arch + 编译日期时间','进入系统启动阶段，可区分新旧固件。'],
 ['UI FRAME INIT: JLUI','当前 UI 配置启用，进入主 app_main()。'],
 ['app_task_loop / Version','app_core 任务已开始，正在进行应用初始化。'],
 ['模式日志、按键或蓝牙响应','进一步证明应用在运行；具体表现取决于开机、电量、UI 与业务配置。']
],[200,299])
p('第一次闭环以“下载成功 + 新版本启动日志 + 可重复复位启动”为基准。屏幕不亮时先看日志进度，再查屏驱、背光和资源；如需增加自己的打印，放在 apps/watch/app_main.c 的 app_main() 起始处，例如 puts("AC7076A3 bringup v1\\n");。本次未修改业务源码。','small')

page(7,'遇到问题，先定位在哪一阶段')
table([
 ['症状','优先检查与处理'],
 ['找不到 clang / 编译器无效','检查 C:/JL/pi32/bin；Code::Blocks 必须具备 pi32v2_r3_large_lto_compiler。不要选 ARM GCC 或普通主机 GCC。'],
 ['make、mkdir_win、fixbat 不存在','进入 SDK，按第 4 页把 SDK/tools/utils 加到 PATH，使用该目录的 make.exe。'],
 ['sdk_config.h 缺失 / 配置不生效','检查 SDK/apps/watch/board/br35/ 生成文件。用匹配的可视化工具打开 project.jlproj，并确认配置生成同步完成。'],
 ['更换配置后表现像旧固件','先核对可视化导出和当前板型，再清理重编；检查 ELF、app.bin、固件输出时间与设备启动标记。'],
 ['已有 ELF，但没有 app.bin / UFW','本次 BUILD_ONLY 命令有意跳过后处理；运行生成的主 download.bat 才继续提取、资源打包和下载。'],
 ['下载脚本缺文件 / UI 打包失败','确认从完整工程操作，output/ 配置资源和 ui_resource/ 齐全；从主 download.bat 进入，查看最早的错误。'],
 ['长时间等待或找不到设备','检查目标是否真正进下载模式、USB 数据线及 D+/D-、工具接法、电源/复位。代码编译成功不能证明下载通道已连接。'],
 ['下载成功但没有串口日志','确认退出下载模式、正常供电；查 PB03、共地、2 Mbaud、串口器性能和 IO 电平。再查晶振、Flash 与电源配置。'],
 ['启动后停住、重启或关机','记录最后日志、异常信息及电源电压；当前启用电量检测且关机阈值 3.5 V。核对 UI/外设等待及任务栈，按实际板子调整。'],
 ['黑屏 / 显示异常','当前选择 ST77916 320×385 和 CST816D；核对屏幕料号、RESET/CS/DC/TE、总线和背光。UI 资源目录标称 320×386，需验证匹配。'],
 ['看到 stack size limit exceeded','本次链接有 103 条此类警告但返回成功。这是栈使用分析提示，不是已观察到的运行时溢出；结合调用链和各任务栈大小进一步评估。']
],[146,353])
h('排查原则')
p('保存完整日志，从第一个错误开始；每次只调整一项，重新完成编译、下载、启动验证。不要用“文件出现了”代替下载成功提示，也不要用“屏幕黑了”直接判定程序完全没运行。')

page(8,'本次验证记录与查阅入口')
table([
 ['项目','本次状态'],
 ['主工程编译、链接','已实测，make 返回 0；生成 sdk.elf（11,111,272 字节）及 sdk.map。ELF 含调试/链接信息，大小不是 Flash 实际占用。'],
 ['编译前配置生成','已生成 fileList.mk、链接相关文件、download.bat、isd_config.ini；确认最终分支是 download_jlui.bat，下载模式是 USB。'],
 ['告警','日志有 103 条 warning，均为链接器栈大小提示；未把这些警告等同于运行时测试通过。'],
 ['固件打包、USB 下载、开机','未执行。需要板子及匹配的下载器完成；本文不声明硬件已跑通。'],
 ['工作区改动','保留原有配置改动；未修改业务源码。本次构建新增中间文件/产物，并更新生成的 movable/section.txt。'],
 ['构建原始记录','tmp/pdfs/build_check.log（工程根目录内）。']
],[129,370])
h('本地依据：需要改哪里就查哪里')
p('• project.jlproj：方案、版本、配置页面与最近工具版本。<br/>• SDK/AC707N.cbp：Release、编译器、编译前/后命令。<br/>• SDK/Makefile；SDK/build/Makefile.mk；SDK/build/README：构建流程。<br/>• SDK/apps/watch/board/br35/board_config.h：板型选择。<br/>• 同目录 sdk_config.h、sdk_config.c：配置与应用日志参数。<br/>• 同目录 board_ac707n_demo/ 下的 *_cfg.h、*_global_build_cfg.h 及 .c：屏驱、电源、下载宏和板初始化。<br/>• SDK/cpu/br35/tools/download.c、isd_config_rule.c、download/watch/download_jlui.bat：打包与下载流程。<br/>• SDK/cpu/br35/setup.c；SDK/apps/watch/app_main.c：启动与主应用。','small')
h('官方资料：工具操作的补充依据')
links=[
 ('W1','安装 Code::Blocks','https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/dev_env/code_blocks.html'),
 ('W2','安装杰理工具链','https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/dev_env/jl_toolchain.html'),
 ('W3','升级与下载说明','https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/forced_upgrade/upgrade_and_download.html'),
]
for ref,label,url in links:
    p(f'[{ref}] <link href="{url}" color="#087F8C">{label}</link><br/>{url}','small')
p('以上官方页面于 2026-09-09 查阅，用于工具安装及一般强制升级操作。页面中的其他芯片示例不作为 AC7076A3 接线依据。具体硬件连接与电气参数仍以对应板图、芯片资料及下载器版本手册为准。','small')

def footer(c,doc):
    c.setStrokeColor(colors.HexColor('#D6E3EA'));c.line(48,44,547,44)
    c.setFont('CN',8);c.setFillColor(GRAY);c.drawString(48,29,'AC7076A3 · AC707N SDK 3.2.1 · 编译 / 下载 / 启动')
    c.drawRightString(547,29,f'{doc.page}')
    c.setFont('CN',8);c.drawString(48,808,'工程实践手册   /   基于当前工作区核对')

doc=SimpleDocTemplate(str(OUT),pagesize=(595.28,841.89),rightMargin=48,leftMargin=48,topMargin=56,bottomMargin=58,title='AC7076A3 工程编译下载快速上手',author='Codex',pageCompression=1)
doc.build(story,onFirstPage=footer,onLaterPages=footer)
r=PdfReader(OUT)
info={'path':str(OUT),'pages':len(r.pages),'page_chars':[len(x.extract_text()) for x in r.pages]}
(ROOT/'tmp/pdfs/pdf_qa.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(info,ensure_ascii=False))
