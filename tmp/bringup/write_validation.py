from pathlib import Path
import json
import hashlib
from datetime import datetime

root = Path(__file__).resolve().parents[2]
out = root / 'output/bringup'
names = ['Configured', 'AudioUsb', 'Original']
data = {n: json.loads((out / f'build-{n}.json').read_text(encoding='utf-8-sig')) for n in names}
last = json.loads((out / 'last-build.json').read_text(encoding='utf-8-sig'))
assert last['profile'] == 'Configured'
elf = root / 'SDK/cpu/br35/tools/sdk.elf'
digest = hashlib.sha256(elf.read_bytes()).hexdigest().upper()
assert digest == last['sha256']
assert b'AC7076A3 Board1 demo' in elf.read_bytes()
lines = [
    '# AC7076A3 demo 构建验证记录', '',
    f'记录生成：{datetime.now().astimezone().isoformat(timespec="seconds")}', '',
    '已完成编译与链接验证，未连接实板验证，也未执行打包、烧录或全片擦除。', '',
    '## 实测组合', '',
    '| 配置 | 说明 | 完成时间 | ELF 字节 | 警告行数 | 结果 |',
    '| --- | --- | --- | ---: | ---: | --- |',
]
description = {
    'Configured': '最终头文件默认值：基础外设，音频/USB CDC/LCD 关闭；Windows PowerShell 5.1',
    'AudioUsb': '基础 + MIC + Speaker + USB CDC；PowerShell 7.6.5',
    'Original': '总宏 0，恢复原工程入口、原配置；PowerShell 7.6.5',
}
for name in names:
    d = data[name]
    lines.append(f'| {name} | {description[name]} | {d["builtAt"]} | {d["bytes"]} | {d["warningLines"]} | make 退出码 0 |')
lines += [
    '',
    'Configured 与 AudioUsb 在最后一次启动流程修正后全量重编。Original 已在本轮全量编译通过；之后新增的时钟初始化入口与 demo 消息循环改动均位于总宏开启分支。',
    '此前 Basic 和 Audio 组合也编译通过；最后基础固件以 Configured 记录为准。', '',
    '警告来自 SDK 链接阶段的栈大小限制检查，未消除，不能把编译成功当成栈使用或硬件稳定性已经验证。原工程配置为 103 条同类警告。', '',
    '## 编译保护与集成检查', '',
    '- 在未确认模组参数时强制 `DEMO_LCD_ENABLE=1`，预处理按预期失败，提示先确认 JD9855 分辨率并提供初始化表。日志：`output/bringup/build-lcd-guard.log`。',
    '- JD9855 色彩测试函数即使 LCD 关闭也参与 C 接口类型检查；运行入口仍由宏关闭。没有使用占位表向屏幕发初始化命令。',
    '- `SDK/AC707N.cbp` XML 可解析，新增 8 个 bringup 源码/头文件条目均存在，Makefile 同时收录 4 个 C 文件。',
    '- 修复 SDK CDC 配置工具消费者的条件编译，避免纯 CDC 回显仍链接已关闭的 `cfg_tool_data_from_cdc`。',
    '- 独占 demo 单独初始化时钟管理互斥量，按需调用音频初始化；任务使用 `os_taskq_pend_timeout` 保留 SDK 回调分发。',
    '- 马达关闭使用优先级 1 的 usr timeout 回调；申请失败立即停止输出。',
    '- PDF 共 8 页，已渲染检查全部页面，并检查文字边界，无页面溢出。',
    '- Git 差异检查剩余空白告警位于原有生成配置 `sdk_config.h`、`jlstream_node_cfg.h`；没有为清理空白改写这些用户文件。', '',
    '## 当前留在工程中的固件', '',
    f'- 配置：`{last["profile"]}`，总宏 1。',
    '- 路径：`SDK/cpu/br35/tools/sdk.elf`。',
    f'- 大小：{last["bytes"]} 字节。',
    f'- SHA256：`{digest}`。',
    r'- 构建命令：`powershell.exe -NoProfile -File .\SDK\tools\build_ac7076a3_demo.ps1`。',
    '- `output/bringup/last-build.json` 和各 `build-*.json` 保存参数、时间及散列；日志保存于同目录。不同配置共用一个 ELF 路径，后一次编译会替换前一次产物。', '',
    '## 硬件待验收项', '',
    '- PA2（U1.26）日志接出可达性和实际 IOVDD 电平；当前 UART 为 115200、8N1。',
    '- LED、按键、马达、DA213B、VBAT/VPWR、音频、USB 枚举与回显按调试手册逐项验证。',
    '- 板载 CST816S 已配置何种固件/电极参数、实际地址与报点协议；本 demo 不刷写触摸固件。',
    '- JD9855 模组分辨率、初始化表、像素偏移、方向、QSPI 时序；尚未完成屏幕点亮。',
    '- 背光回路 R5 标记为 X，需确认实际装配与阻值。', '',
    '本轮未宣称已在 AC7076A3 实板跑通全部外设。', '',
]
(root / 'doc/AC7076A3_DEMO_VALIDATION.md').write_text('\n'.join(lines), encoding='utf-8')
print('Validation written; final ELF SHA256 verified:', digest)
