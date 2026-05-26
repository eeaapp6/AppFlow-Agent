# AgentBridge 当前工作总结

本文档记录 APPFlow 与 AgentChat / Foam-Agent 后端联动的当前状态。它用于后续回归测试、与 Agent 端继续对接、以及迁移到 `appPHengLEI` 时快速对齐。

## 当前阶段结论

当前已经从最初的“APPFlow 只读取 Agent 端到端产物”推进到“APPFlow 内嵌 AgentChat，并支持 OpenFOAM 分步骤联动”。

当前主链路是：

```text
用户在 APPFlow AgentChat 输入自然语言需求
  -> 后端 /foam/plan 生成计划
  -> APPFlow 显示任务状态和 next_action
  -> 用户输入“继续”或点击按钮
  -> APPFlow 调用 /foam/generate
  -> APPFlow 调用 /foam/validate
  -> APPFlow 调用 /foam/run
  -> 后端写出 agent_manifest_v1.json
  -> APPFlow 调用本地 actionLoadAgentManifest
  -> 导入 OpenFOAM 网格、检查结果、打开可视化
  -> 状态条显示已完成
```

这个闭环已经由用户完成手动验证。

## 已实现内容

### Manifest 导入

- 支持读取 `version = 1` 的 `agent_manifest_v1.json`。
- 支持最小 manifest，缺失的可选字段不会导致导入失败。
- 支持按行输出 AgentBridge 日志。
- 支持 `solver.family` 分发：
  - `openfoam`：进入 APPFlow 当前 OpenFOAM 适配逻辑。
  - `phenglei`：当前 APPFlow 明确提示暂不支持，不误走 OpenFOAM。
- 支持读取并展示 `solver_settings`。
- 支持把 `simpleFoam` 自动映射到 APPFlow `SIMPLE` 求解器。
- 支持把 `interFoam` 自动映射到 APPFlow `Inter` 求解器。
- 支持把 `solver_settings.time.delta_t/end_time/write_interval` 写入 APPFlow `RunControl`。
- 支持把 `solver_settings.physics.nu` 写入 APPFlow Transport Properties 的运动粘度字段。

### OpenFOAM 适配

- 支持从 manifest 中读取 `case_dir`、`mesh_dir`、`result_dir`。
- 支持复用 APPFlow 原有 OpenFOAM 网格导入入口。
- 支持扫描结果文件：
  - `*.vtk`
  - `*.vtp`
  - `*.vtu`
  - `*.foam`
- 支持检测 OpenFOAM 时间步目录，例如 `0.005`、`0.1`、`0.5`。
- 支持在没有 VTK 时提示或触发 `foamToVTK`。

### foamToVTK

- 支持本机 `foamToVTK`。
- 支持 Docker 后端执行 `foamToVTK`。
- 支持 Docker case 目录映射，例如：

```text
宿主机：D:/work/demo_case_002/case
容器内：/case
```

- 执行后会重新扫描 `case/VTK`，确认是否生成 VTK 文件。

### ParaView 打开结果

当前支持两种模式，由 `appflow_hints.paraview_open_mode` 控制：

| 模式 | 含义 | 适用情况 |
| --- | --- | --- |
| `first_vtk` | 直接打开扫描到的第一个 VTK 文件 | Agent 已生成 `case/VTK/*.vtk` |
| `openfoam_case` | 创建空 `case.foam`，用 ParaView 按 OpenFOAM case 打开 | case 根目录存在 OpenFOAM 时间步 |

`case.foam` 是 ParaView 识别 OpenFOAM case 的触发文件，空文件是正常行为。

### AgentChat UI 集成

APPFlow 已经集成从 `chat_ui` 迁移过来的 Qt 聊天界面核心能力。

当前入口关系：

```text
菜单 / ribbon action
  -> MainWindow::showAgentPanel()
  -> QDockWidget
  -> AgentPanelWidget
  -> FoamAgentChatPanel
```

当前 AgentChat 面板包含：

- 后端状态和设置入口。
- 本机输出目录与后端输出目录。
- 当前任务状态条。
- 操作按钮：打开目录、生成算例、校验算例、运行算例、导入到 APPFlow。
- 聊天消息列表。
- 用户输入框。
- Agent 回复前端逐字显示。

### 后端服务集成

APPFlow 侧已经支持：

- 检查 `http://127.0.0.1:8765/health`。
- 如果本机后端未启动，尝试启动 `AgentBridge/chat_ui_backend/scripts/agent_service.py`。
- 读取 APPFlow 设置中的模型服务配置：
  - `ACTIVE_PROVIDER`
  - `PROVIDER_API_KEY`
  - `PROVIDER_MODEL`
  - `PROVIDER_BASE_URL`
- 支持用户手动启动 Docker 后端，再由 APPFlow 连接。

Docker 场景推荐路径：

```text
APPFlow 本机输出：D:/work/FoamAgentOutputs
后端输出：/workspace/outputs
```

### 分步骤接口

当前后端副本已经支持：

| 接口 | 作用 |
| --- | --- |
| `/foam/plan` | 创建任务和计划 |
| `/foam/generate` | 生成 OpenFOAM case |
| `/foam/validate` | 校验 case |
| `/foam/run` | 运行 OpenFOAM 并写出 manifest |
| `/foam/replan` | 参数修改请求占位入口，当前只返回重新生成下一步 |

APPFlow 会读取响应中的：

```json
{
  "task": {
    "task_id": "...",
    "status": "...",
    "task_dir": "...",
    "manifest_path": "..."
  },
  "next_action": {
    "id": "...",
    "label": "...",
    "endpoint": "..."
  }
}
```

当前支持的 `next_action.id`：

| id | APPFlow 行为 |
| --- | --- |
| `generate` | 调用 `/foam/generate` |
| `validate` | 调用 `/foam/validate` |
| `run` | 调用 `/foam/run` |
| `import_manifest` | 调用本地 `actionLoadAgentManifest` |

当前 `/foam/replan` 是通道占位实现：

```text
接收 task_dir + message
加载 task_context.json
把 task.status 重新置为 planned
返回原有 plan
返回 next_action.id = generate
不解析具体参数
不改写 OpenFOAM case
```

### 求解参数只读摘要

APPFlow 当前会在 `/foam/plan` 返回后，从后端响应中读取计划参数并显示为聊天区工作流消息。

当前读取来源：

```text
plan.solver_name
plan.requested_changes
plan.parameters.requested_changes
```

当前展示示例：

```text
求解参数
求解器：icoFoam
时间：end_time=0.5, delta_t=0.005, write_interval=1
物理：nu=0.01
```

当前行为边界：

- 在 AgentChat 中展示求解参数摘要。
- 在导入 manifest 时，尝试把已支持字段写入 APPFlow 参数模型。
- 当前已支持 `RunControl` 的时间步、结束时间、输出间隔，以及 Transport Properties 的 `nu`。
- 当前不自动写入 `fvSchemes`、`fvSolution`、湍流模型和复杂物性参数。
- 如果后端没有提取到参数，不显示空的时间或物理字段。
- 用户已经完成验证，确认带参数需求可以显示摘要，原有“继续”链路不受影响。

### 边界条件对接状态

当前已经完成 APPFlow 边界条件相关代码阅读和 manifest 字段设计，但还没有自动写入 APPFlow `BoundaryManager`。

当前结论：

```text
Agent 可以先输出 boundary_conditions
  -> APPFlow 先做日志或只读预览
  -> 后续再做 patch 名称到 mesh boundary ID 的映射
  -> 最后调用 APPFlow 现有边界写入接口
```

暂不直接写入的原因：

- APPFlow 创建边界条件时依赖真实 `regionMeshID` 和 `meshBoundaryID`。
- Agent manifest 中通常只有 OpenFOAM patch 名称，例如 `movingWall`、`fixedWalls`。
- 必须先完成 mesh 导入，再按 boundary name 查找 APPFlow 内部 ID。
- `empty/frontAndBack`、热耦合、多相、旋转机械等边界需要单独判断，不能一开始强行通用映射。

当前建议是：先让 Agent 端按 `AGENT_OUTPUT_CONTRACT.zh-CN.md` 输出 `boundary_conditions`，APPFlow 下一阶段先做只读展示和匹配检查，再决定是否写入。

### 参数修改请求识别

APPFlow 当前能在已有任务上下文中识别明显的参数修改请求，例如：

```text
把 end_time 改成 1.0
修改 delta_t 为 0.001
set nu to 0.005
```

当前行为边界：

- 显示“检测到参数修改请求”。
- 尝试调用后端 `/foam/replan`。
- 当前后端最小占位实现会返回 `next_action.id = generate`。
- 不重新规划新任务。
- 不直接在 APPFlow 侧解析自然语言参数语义。
- 不修改 OpenFOAM 字典文件。

后续方向已经确定为方案 A：

```text
用户参数修改请求
  -> APPFlow 交给后端 Agent
  -> 后端重新规划或重新生成 case
  -> APPFlow 再显示新的求解参数摘要和 next_action
```

这样 APPFlow 继续保持“展示、确认、导入、可视化”的角色，参数理解和 case 文件更新交给 Agent 端完成。

### 用户“继续”执行下一步

APPFlow 当前用最小规则判断用户是否确认下一步。

支持确认词：

```text
继续
下一步
执行
可以
确认
好的
好
行
确定
开始
ok
yes
continue
next
go
```

如果当前存在 `next_action.id`，用户输入这些词时，APPFlow 不会重新规划任务，而是执行当前下一步。

这层后续可以替换为 LLM 意图解析，但 `executeCurrentNextAction()` 这类动作分发逻辑可以保留。

### 状态条

APPFlow AgentChat 当前新增了任务状态条，位于输出目录栏和按钮栏之间。

显示内容：

```text
任务：task_xxx
阶段：已规划 / 生成中 / 已生成 / 校验中 / 已校验 / 运行中 / 已运行 / 导入中 / 已完成
下一步：生成算例 / 校验算例 / 运行 OpenFOAM / 导入到 APPFlow / 无
清单：未收到 / 已收到
```

用户已经验证状态条能随流程变化。

### 导入完成状态闭环

当前导入成功后：

```text
阶段：已完成
下一步：无
清单：已收到
```

导入失败后：

```text
阶段：导入失败
下一步仍保留为“导入到 APPFlow”
```

这样用户可以修复路径或文件问题后继续重试。

### Agent 回复逐字显示

当前已实现方案 1：前端假流式显示。

```text
后端仍然一次性返回完整 reply
  -> APPFlow 创建一条空的 Agent 消息
  -> QTimer 按小段追加文本
  -> 用户看到逐字输出效果
```

当前边界：

- 不改变后端协议。
- 不影响 `task / plan / next_action / manifest_path` 的解析。
- 只作用于 Agent 普通回复。
- 工作流消息、错误消息、工具消息仍然直接显示完整文本。
- 用户已经验证该功能可用。

后续如果升级为真流式，只需要把文本来源从“完整 reply 拆分”替换为“SSE / WebSocket delta”，消息追加接口可以复用。

## 已验证示例

| 示例文件或场景 | 当前用途 | 验证状态 |
| --- | --- | --- |
| `examples/minimal_manifest_v1.json` | 最小 OpenFOAM manifest | 已验证可加载 |
| `examples/agent_manifest_v1.json` | 已有 VTK 结果的 OpenFOAM 示例 | 已验证可导入网格、识别 VTK、请求 ParaView |
| `examples/test.json` | Docker foamToVTK / 参数展示与部分写入示例 | 已验证 Docker foamToVTK 流程 |
| `examples/phenglei_minimal_manifest_v1.json` | PHengLEI 最小示例 | 已验证 APPFlow 明确提示不支持 PHengLEI |
| `examples/new.json` | 真实 Agent 运行结果改写后的本机路径示例 | 已验证真实 Agent 输出可被 APPFlow 读取 |
| AgentChat 分步骤链路 | plan -> generate -> validate -> run -> import | 已验证可通过“继续”推进到完成 |
| Agent 回复逐字显示 | 前端假流式显示普通 Agent 回复 | 已验证成功 |

## 真实 Agent 输出验收结果

已经使用真实 Agent 生成的一次 OpenFOAM cavity 算例完成 APPFlow 侧验收。

案例特征：

- 求解器：`icoFoam`
- 算例类型：二维 lid-driven cavity
- 网格：`case/constant/polyMesh`
- 运行结果：OpenFOAM 时间步目录 `0.005` 到 `0.5`
- 日志：`blockMesh.log`、`icoFoam.log`
- 后处理：Agent 未预生成 `case/VTK` 时，可由 APPFlow 通过 Docker `foamToVTK` 触发生成

已验证 APPFlow 侧行为：

- 能加载真实 Agent 输出格式的 manifest。
- 能识别 `solver.family = openfoam`。
- 能定位 `case_dir` 和 `mesh_dir`。
- 能检测 OpenFOAM 时间步结果。
- 能展示 `solver_settings`，并对当前支持字段执行 APPFlow 参数写入。
- 能预览 `blockMesh` 和 `icoFoam` 日志。
- 能复用原有 OpenFOAM 网格导入入口。
- 能通过 Docker `foamToVTK` 生成 VTK。
- 能按 `first_vtk` 或 `openfoam_case` 模式请求 ParaView。

## 当前代码结构

### AgentBridge 文档层

| 文件 | 用途 |
| --- | --- |
| `APPFlow_AgentChat_Integration.zh-CN.md` | AgentChat 与后端分步骤接口契约 |
| `AGENT_REPLAN_TASK.zh-CN.md` | 发给 Agent 端实现 `/foam/replan` 的任务清单 |
| `AGENT_OUTPUT_CONTRACT.zh-CN.md` | Agent 输出 manifest 契约 |
| `CHAT_UI_PATH_SETUP.zh-CN.md` | Host / Backend 输出路径配置 |
| `TESTING.zh-CN.md` | 手动验收流程 |
| `MIGRATION_TO_APP_PHENGLEI.zh-CN.md` | 迁移到 appPHengLEI 的复用清单 |

### APPFlow C++ 层

```text
GUIFrame/AgentChat
  AgentController
    负责 HTTP 调用、解析 task / manifest_path / next_action / 只读求解参数摘要、调用 /foam/replan

  FoamAgentChatPanel
    负责聊天面板组合、状态联动、按钮动作、继续执行下一步、参数修改请求拦截

  AgentChatIntent
    负责“继续”确认词、参数修改请求、next_action 标题、任务 ID 提取等轻量意图判断

  AgentTypingRenderer
    负责 Agent 普通回复的前端假流式逐字播放

  MessageList
    管理聊天消息气泡，支持 Agent 消息逐字追加

  WorkflowStatusBar
    显示任务、阶段、下一步、清单状态

  WorkflowActionBar
    显示打开目录、生成、校验、运行、导入按钮

  WorkflowWorkspaceBar
    管理本机输出路径和后端输出路径

  LocalAgentServiceManager
    检查或启动本地 Python 后端
```

```text
OperatorsModel
  OperatorsAgentManifest
    APPFlow manifest 导入入口

  AgentManifestData
    通用 manifest 解析和字段读取

  AgentLogPreview
    日志存在性检查和预览

  OpenFoamAgentAdapter
    OpenFOAM case、VTK、foamToVTK、ParaView 适配
```

```text
AgentBridge/chat_ui_backend/scripts
  agent_service.py
    APPFlow 当前联调用的后端服务副本
    已提供 /health、/foam/plan、/foam/generate、/foam/validate、/foam/run、/foam/replan
    /foam/replan 当前只是占位，不承担真实参数解析
```

## 阶段冻结清单

当前阶段建议作为一个稳定检查点冻结。冻结含义不是停止开发，而是后续改动都要先保证这些能力不退化。

### 已冻结主链路

```text
APPFlow AgentChat
  -> /foam/plan
  -> Agent 普通回复逐字显示
  -> 显示求解参数摘要
  -> next_action = generate
  -> 用户输入“继续”
  -> /foam/generate
  -> /foam/validate
  -> /foam/run
  -> next_action = import_manifest
  -> actionLoadAgentManifest
  -> APPFlow 导入网格和结果
```

### 已冻结参数修改通道

```text
用户输入：把 end_time 改成 1.0
  -> APPFlow 识别为参数修改请求
  -> APPFlow 调用 /foam/replan
  -> 后端占位返回 next_action.id = generate
  -> APPFlow 回到“已规划 / 下一步生成算例”
```

当前冻结边界：

- APPFlow 不解析复杂参数语义。
- APPFlow 不修改 OpenFOAM 字典。
- APPFlow 不写内部求解器参数模型。
- 后端占位 `/foam/replan` 不真实更新 plan。
- 真正的参数理解、plan 更新、case 重生成交给 Agent 端继续实现。

### 已冻结结构边界

当前 APPFlow AgentChat 侧已经完成两次低风险拆分：

```text
AgentTypingRenderer
  从 FoamAgentChatPanel 拆出逐字播放逻辑

AgentChatIntent
  从 FoamAgentChatPanel 拆出确认词和参数修改请求识别
```

短期不建议继续拆：

```text
AgentController
  目前继续保留 HTTP 请求和响应解析职责

OpenFoamAgentAdapter
  目前继续保留 OpenFOAM manifest / VTK / ParaView / foamToVTK 适配职责
```

原因是这两处都已经被多轮手动验证覆盖，继续拆分的收益低于回归风险。下一阶段应优先等待 Agent 端真实 `/foam/replan` 返回后做联调。

### 给 Agent 端的交接文件

Agent 端继续开发时，优先阅读：

```text
AgentBridge/AGENT_REPLAN_TASK.zh-CN.md
AgentBridge/APPFlow_AgentChat_Integration.zh-CN.md
AgentBridge/AGENT_OUTPUT_CONTRACT.zh-CN.md
```

其中 `AGENT_REPLAN_TASK.zh-CN.md` 是当前最直接的任务清单。

## 当前已知行为

- 如果结果只在 `case/VTK` 中，而 OpenFOAM case 根目录没有实际时间步结果，`openfoam_case` 模式可能看不到预期的 VTK 数据。
- 如果要直接看 `case/VTK/*.vtk`，应使用 `first_vtk` 模式。
- 如果本机没有 `foamToVTK`，需要使用 Docker 模式，或者由 Agent 端提前生成 VTK。
- 如果本机没有配置 ParaView 环境变量，APPFlow 会提示 ParaView 打开失败。
- 修改系统 PATH 后，需要重启 VS2017 和 APPFlow，才能继承新环境变量。
- 当前确认词是规则判断，不是 LLM 意图解析。
- 当前参数修改请求会尝试交给后端 `/foam/replan`，但 APPFlow 本地不执行真实修改。
- 当前 `/foam/replan` 后端实现只是占位，不解析具体参数、不改写 case。
- 当前 AgentChat 是分步骤 HTTP JSON 响应；Agent 回复已做前端假流式，但还不是真正后端流式输出。

## 暂缓内容

以下内容暂时不做，后续按优先级逐步补：

- 后端 SSE / WebSocket 真流式输出。
- LLM 意图解析替代写死确认词。
- 后端完整实现参数解析、计划更新、重新规划或重新生成 case。
- 将更多 `solver_settings` 字段写入 APPFlow 内部求解器参数模型。
- 从 OpenFOAM 字典反向解析参数到 APPFlow UI。
- APPFlow 内置 CFDPostAPP 风格的 VTK 展示。
- 一次打开完整 VTK 序列。
- `appPHengLEI` 中的 PHengLEI 求解器适配实现。

## 下一步建议

当前第一阶段联动已经闭环。建议后续优先做两件事：

1. 保持当前链路作为回归测试。

```text
plan -> generate -> validate -> run -> import_manifest -> 已完成
```

2. 等 Agent 端实现真实 `/foam/replan` 后，再做参数联动验收。

建议下一阶段保持 APPFlow 不直接改 OpenFOAM 字典，而是把参数修改请求交给后端：

```text
用户：把 end_time 改成 1.0
  -> APPFlow 发送当前 task_dir 和修改文本
  -> 后端 Agent 更新计划或重新生成 case
  -> 后端返回新的 plan / task / next_action
  -> APPFlow 刷新只读参数摘要
```

不要一开始就让 APPFlow 本地自动改内部参数或字典文件，避免破坏原有手动建模和求解流程。

Agent 端返回新实现后，APPFlow 侧优先验证：

```text
/foam/replan 是否更新 plan.requested_changes
APPFlow 是否显示新的求解参数摘要
输入“继续”后 /foam/generate 是否按新 plan 重写 case
validate / run 是否仍然能通过
```
