# Read Abuse Lab

面向在线阅读平台的“完读刷量”行为模拟与检测靶场。

这个项目用于课程/毕设里的授权安全研究：它复现阅读、翻页、章节完读、任务奖励等核心流程，并提供行为模拟器和风控检测器。模拟器默认只允许访问本地靶场地址，不包含任何真实平台接口、鉴权、签名、Cookie、设备指纹或绕过逻辑。

## 结构

```text
read_abuse_lab/
  demo_server.py      # 本地阅读平台靶场
  register_simulator.py # 注册滥用模拟器
  register_detector.py  # 注册风控检测器
  simulator.py        # 行为模拟器
  detector.py         # 风控检测器
  run_lab_test.py     # 一键演示
  lab_config.json     # 本地白名单和参数
  data/
    events.jsonl      # 行为日志，运行后生成
```

## 快速开始

启动本地靶场：

```powershell
python .\read_abuse_lab\demo_server.py
```

另开一个终端，生成正常阅读日志：

```powershell
python .\read_abuse_lab\simulator.py --mode normal --user alice --chapters 3
```

生成几种刷完读模拟日志：

```powershell
python .\read_abuse_lab\simulator.py --mode fixed_fast --user bot_fixed --chapters 6
python .\read_abuse_lab\simulator.py --mode jump_read --user bot_jump --chapters 6
python .\read_abuse_lab\simulator.py --mode batch --users 5 --chapters 4
```

生成批量注册滥用样本：

```powershell
python .\read_abuse_lab\register_simulator.py --mode bulk_same_device --count 10
python .\read_abuse_lab\register_simulator.py --mode burst --count 20 --workers 6
```

运行检测器：

```powershell
python .\read_abuse_lab\detector.py
python .\read_abuse_lab\register_detector.py
```

一键演示注册滥用与完读刷量检测：

```powershell
python .\read_abuse_lab\run_lab_test.py
```

## 模拟器与真实刷完读脚本的差异

这个项目故意做了以下安全修改：

- 只允许请求 `lab_config.json` 里的本地白名单地址，默认是 `http://127.0.0.1:8787`。
- 不读取、不生成、不复用真实平台 Cookie、Token、设备 ID、签名参数或加密参数。
- 不包含真实 App/Web 的接口路径、请求字段、反调试绕过、验证码绕过、风控绕过。
- 行为日志会显式标记 `is_bot_simulation` 和 `mode`，便于做防护实验，而不是隐藏自动化痕迹。
- 请求频率有本地限制，模拟的是“攻击样本生成”，不是生产平台刷量。
- 靶场接口是教学用 API：`/api/session/start`、`/api/read/page`、`/api/chapter/complete`，不是任何真实平台 API。
- 注册靶场接口是教学用 API：`/api/register`，不包含真实平台注册流程、短信验证码、滑块、人机校验、设备校验或绕过逻辑。

如果你们有授权测试域名，可以在保持这些安全边界的前提下，把靶场替换成内部测试环境，并保留白名单、日志标记和速率限制。

## 可写进论文的实验点

- 翻页间隔均值、方差和变异系数
- 单章完成耗时
- 章节完成间隔
- 跳页率、跳章率
- 多账号行为相似度
- 风险分数、召回率、误报率
- 注册后立刻刷完读的联动风险
- 同设备/同 IP/同邮箱域名的批量注册特征
