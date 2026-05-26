# APPFlow AgentChat 对接协议

本文档说明当前 APPFlow 内嵌 AgentChat 与后端 Agent 服务之间的最小对接协议。

目标不是定义完整 Agent 架构，而是保证下面这条链路稳定：

```text
用户在 APPFlow 聊天界面输入需求
  -> 后端 Agent 规划 OpenFOAM 任务
  -> APPFlow 显示当前任务状态和建议下一步
  -> 用户输入“继续”或点击按钮
  -> APPFlow 调用后端分步骤接口
  -> 后端生成 case / 校验 / 运行 / 写 manifest
  -> APPFlow 导入 manifest 并展示结果
```

## 当前接口入口

APPFlow 当前固定连接本机后端服务：

```text
http://127.0.0.1:8765
```

后端需要提供这些接口：

| 接口 | 方法 | 作用 |
| --- | --- | --- |
| `/health` | GET | APPFlow 检查后端是否可用 |
| `/foam/plan` | POST | 根据用户自然语言需求创建任务和仿真计划 |
| `/foam/generate` | POST | 根据已有 task 生成 OpenFOAM case 文件 |
| `/foam/validate` | POST | 校验 case 文件是否可运行 |
| `/foam/run` | POST | 运行 OpenFOAM 算例并写出 manifest |
| `/foam/replan` | POST | 接收已有任务上的参数修改请求，当前为最小占位实现 |

当前 APPFlow 侧不直接调用 `/chat` 做普通聊天。用户输入新需求时，优先走 `/foam/plan`。

## `/foam/plan`

APPFlow 发送：

```json
{
  "message": "生成一个二维方腔流动算例",
  "output_dir": "/workspace/outputs",
  "host_output_dir": "D:/work/FoamAgentOutputs"
}
```

字段说明：

| 字段 | 含义 |
| --- | --- |
| `message` | 用户自然语言仿真需求 |
| `output_dir` | 后端或 Docker 容器内输出根目录 |
| `host_output_dir` | APPFlow 主机侧可访问的输出根目录 |

后端返回：

```json
{
  "reply": "Plan is ready.\nCase: cavity (icoFoam)\nNext: Generate Case.",
  "task": {
    "task_id": "task_20260518_001",
    "status": "planned",
    "task_dir": "/workspace/outputs/task_20260518_001",
    "manifest_path": "/workspace/outputs/task_20260518_001/agent_manifest_v1.json"
  },
  "plan": {
    "solver_name": "icoFoam",
    "requested_changes": {
      "delta_t": "0.005",
      "end_time": "0.5",
      "kinematic_viscosity": "0.01"
    }
  },
  "next_action": {
    "id": "generate",
    "label": "生成算例",
    "endpoint": "/foam/generate",
    "requires_confirmation": true
  }
}
```

APPFlow 行为：

```text
保存 task.task_dir
显示 task_id
状态条显示：已规划
显示 next_action
启用“生成算例”按钮
如果 plan 中包含参数，显示只读求解参数摘要
如果用户输入“继续”，自动调用 /foam/generate
```

## 求解参数只读摘要

APPFlow 当前会在 `/foam/plan` 返回后，从响应中读取计划参数，并在聊天区追加一条工作流消息。

当前优先读取：

```text
plan.solver_name
plan.requested_changes
plan.parameters.requested_changes
```

推荐后端在 `/foam/plan` 响应中直接返回：

```json
{
  "plan": {
    "solver_name": "icoFoam",
    "requested_changes": {
      "delta_t": "0.005",
      "end_time": "0.5",
      "write_interval": "1",
      "kinematic_viscosity": "0.01"
    }
  }
}
```

APPFlow 当前显示效果：

```text
求解参数
求解器：icoFoam
时间：end_time=0.5, delta_t=0.005, write_interval=1
物理：nu=0.01
```

当前支持展示的参数键：

| 分类 | 支持键 |
| --- | --- |
| 时间 | `start_time`、`end_time`、`final_time`、`delta_t`、`time_step`、`write_interval` |
| 物理 | `kinematic_viscosity`、`viscosity`、`nu`、`density`、`rho`、`lid_velocity`、`inlet_velocity`、`outlet_pressure` |
| 数值 | `max_co`、`max_courant`、`max_delta_t`、`adjust_time_step` |

注意：

```text
AgentChat 面板会展示这些参数摘要。
manifest 导入阶段会把当前支持字段写入 APPFlow 原有求解器参数模型。
当前已支持 RunControl 的 delta_t / end_time / write_interval，以及 Transport Properties 的 nu。
当前不自动写入 fvSchemes、fvSolution、湍流模型和复杂物性参数。
不会修改 OpenFOAM 字典文件。
不会在 generate / validate / run 阶段重复显示同一组参数。
```

## 参数修改请求

当前 APPFlow 能在已有任务上下文中识别明显的参数修改请求，并尝试把请求交给后端 `/foam/replan`。

示例输入：

```text
把 end_time 改成 1.0
修改 delta_t 为 0.001
set nu to 0.005
```

当前 APPFlow 行为：

```text
显示“检测到参数修改请求”
调用后端 /foam/replan
后端最小占位实现返回 next_action.id = generate
不在 APPFlow 侧直接解析自然语言参数语义
不修改 OpenFOAM 字典文件
```

后续推荐采用方案 A：

```text
用户参数修改请求
  -> APPFlow 将当前 task_dir 和修改文本发给后端 Agent
  -> 后端 Agent 重新规划或重新生成 case
  -> 后端返回新的 plan / task / next_action
  -> APPFlow 刷新求解参数摘要，并在导入 manifest 时写入当前支持字段
```

当前 APPFlow 优先调用 `/foam/replan`。后续如果需要更细的语义，也可以再扩展 `/foam/modify`。建议请求结构：

```json
{
  "task_dir": "/workspace/outputs/task_20260518_001",
  "message": "把 end_time 改成 1.0",
  "mode": "replan"
}
```

建议响应结构：

```json
{
  "reply": "Parameters updated. Please regenerate the case.",
  "task": {
    "task_id": "task_20260518_001",
    "status": "planned",
    "task_dir": "/workspace/outputs/task_20260518_001",
    "manifest_path": "/workspace/outputs/task_20260518_001/agent_manifest_v1.json"
  },
  "plan": {
    "solver_name": "icoFoam",
    "requested_changes": {
      "delta_t": "0.005",
      "end_time": "1.0",
      "kinematic_viscosity": "0.01"
    }
  },
  "next_action": {
    "id": "generate",
    "label": "重新生成算例",
    "endpoint": "/foam/generate",
    "requires_confirmation": true
  }
}
```

这个方案的边界是：APPFlow 负责识别、转发、显示和确认；Agent 负责理解参数含义、更新计划、生成或重生成 OpenFOAM case。

当前后端副本中的 `/foam/replan` 只是最小占位实现：

```text
加载已有 task_context.json
记录请求已收到
把 task.status 重新置为 planned
返回原有 plan
返回 next_action.id = generate
不解析具体参数
不改写 OpenFOAM 字典
不删除或重写已经生成的 case 文件
```

## `/foam/generate`

APPFlow 发送：

```json
{
  "task_dir": "/workspace/outputs/task_20260518_001"
}
```

后端返回：

```json
{
  "reply": "Case files generated.\nNext: Validate Case.",
  "task": {
    "task_id": "task_20260518_001",
    "status": "generated",
    "task_dir": "/workspace/outputs/task_20260518_001",
    "manifest_path": "/workspace/outputs/task_20260518_001/agent_manifest_v1.json"
  },
  "generated_files": [],
  "reference_files": [],
  "next_action": {
    "id": "validate",
    "label": "校验算例",
    "endpoint": "/foam/validate",
    "requires_confirmation": true
  }
}
```

APPFlow 行为：

```text
状态条显示：已生成
显示 next_action
启用“校验算例”按钮
如果用户输入“继续”，自动调用 /foam/validate
```

## `/foam/validate`

APPFlow 发送：

```json
{
  "task_dir": "/workspace/outputs/task_20260518_001"
}
```

校验通过时，后端返回：

```json
{
  "reply": "Case validation passed.\nNext: Run Case.",
  "task": {
    "task_id": "task_20260518_001",
    "status": "validated",
    "task_dir": "/workspace/outputs/task_20260518_001",
    "manifest_path": "/workspace/outputs/task_20260518_001/agent_manifest_v1.json"
  },
  "validation": {
    "status": "validated",
    "missing_files": [],
    "dictionary_errors": []
  },
  "next_action": {
    "id": "run",
    "label": "运行 OpenFOAM",
    "endpoint": "/foam/run",
    "requires_confirmation": true
  }
}
```

校验失败时，后端可以返回：

```json
{
  "reply": "Case validation failed.",
  "task": {
    "task_id": "task_20260518_001",
    "status": "validation_failed",
    "task_dir": "/workspace/outputs/task_20260518_001",
    "manifest_path": "/workspace/outputs/task_20260518_001/agent_manifest_v1.json"
  },
  "validation": {
    "status": "validation_failed",
    "missing_files": ["case/system/controlDict"]
  },
  "next_action": {}
}
```

APPFlow 行为：

```text
校验通过：状态条显示“已校验”，启用“运行算例”
校验失败：显示错误信息，不自动进入运行
如果用户输入“继续”，只有存在 next_action.id = run 时才会自动运行
```

## `/foam/run`

APPFlow 发送：

```json
{
  "task_dir": "/workspace/outputs/task_20260518_001"
}
```

运行成功时，后端返回：

```json
{
  "reply": "OpenFOAM run completed.\nAppFlow manifest: /workspace/outputs/task_20260518_001/agent_manifest_v1.json",
  "task": {
    "task_id": "task_20260518_001",
    "status": "run_completed",
    "task_dir": "/workspace/outputs/task_20260518_001",
    "manifest_path": "/workspace/outputs/task_20260518_001/agent_manifest_v1.json"
  },
  "run": {
    "status": "run_completed"
  },
  "next_action": {
    "id": "import_manifest",
    "label": "导入到 APPFlow",
    "endpoint": "appflow://import_manifest",
    "requires_confirmation": true
  }
}
```

运行失败时，后端可以返回：

```json
{
  "reply": "OpenFOAM run failed.\nReason: OpenFOAM runtime is not configured.",
  "task": {
    "task_id": "task_20260518_001",
    "status": "run_blocked",
    "task_dir": "/workspace/outputs/task_20260518_001",
    "manifest_path": "/workspace/outputs/task_20260518_001/agent_manifest_v1.json"
  },
  "run": {
    "status": "run_blocked",
    "reason": "OpenFOAM runtime is not configured. WM_PROJECT_DIR is not set."
  },
  "next_action": {}
}
```

APPFlow 行为：

```text
运行成功：状态条显示“已运行”，下一步显示“导入到 APPFlow”
运行失败：显示错误信息，不自动导入
如果用户输入“继续”，只有存在 next_action.id = import_manifest 时才会导入
```

## `next_action` 结构

`next_action` 是 APPFlow 当前分步骤联动的核心字段。

```json
{
  "id": "generate",
  "label": "生成算例",
  "endpoint": "/foam/generate",
  "requires_confirmation": true
}
```

字段说明：

| 字段 | 类型 | 是否必需 | 含义 |
| --- | --- | --- | --- |
| `id` | string | 是 | APPFlow 用它决定执行哪个动作 |
| `label` | string | 建议 | 面向用户显示的中文动作名 |
| `endpoint` | string | 建议 | 后端接口或 APPFlow 本地动作 |
| `requires_confirmation` | boolean | 建议 | 是否需要用户确认后再执行 |

当前 APPFlow 支持的 `id`：

| `id` | APPFlow 行为 |
| --- | --- |
| `generate` | 调用 `/foam/generate` |
| `validate` | 调用 `/foam/validate` |
| `run` | 调用 `/foam/run` |
| `import_manifest` | 调用 APPFlow 本地 `actionLoadAgentManifest` |

注意：

```text
APPFlow 当前不会执行未知 id。
未知 id 会显示错误提示，而不是静默忽略。
```

## 用户“继续”的当前规则

当前 APPFlow 先用最小规则判断用户是否确认下一步。

支持的确认词包括：

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

如果当前存在 `next_action.id`，并且用户输入命中确认词，APPFlow 不会重新规划任务，而是执行当前 `next_action`。

后续可以把这层替换成 LLM 意图解析：

```text
用户输入
  -> 后端 intent 解析
  -> 返回 confirm_next_action / reject / modify / new_task
  -> APPFlow 继续复用 executeCurrentNextAction
```

## 路径映射规则

Docker 场景下通常存在两套路径：

```text
后端路径：/workspace/outputs/task_xxx
主机路径：D:/work/FoamAgentOutputs/task_xxx
```

APPFlow 向 `/foam/plan` 发送：

```json
{
  "output_dir": "/workspace/outputs",
  "host_output_dir": "D:/work/FoamAgentOutputs"
}
```

后端可以继续在 `task.manifest_path` 中返回后端路径：

```text
/workspace/outputs/task_xxx/agent_manifest_v1.json
```

APPFlow 会根据 `output_dir` 和 `host_output_dir` 做路径替换：

```text
/workspace/outputs/task_xxx/agent_manifest_v1.json
  -> D:/work/FoamAgentOutputs/task_xxx/agent_manifest_v1.json
```

因此要求：

```text
Docker volume 必须保证 /workspace/outputs 与 host_output_dir 指向同一批文件。
```

## Manifest 导入

`import_manifest` 不调用后端 HTTP 接口。

它是 APPFlow 本地动作：

```text
next_action.id = import_manifest
  -> APPFlow 使用当前 task.manifest_path
  -> 转换为主机路径
  -> 调用 actionLoadAgentManifest
  -> 读取 agent_manifest_v1.json
  -> 导入 OpenFOAM 网格
  -> 检查结果目录
  -> 按 appflow_hints 打开 ParaView 或执行 foamToVTK
```

导入成功后：

```text
状态条：已完成
下一步：无
清单：已收到
```

导入失败后：

```text
状态条：导入失败
下一步保留为：导入到 APPFlow
用户可以修复问题后再次输入“继续”重试
```

## APPFlow 当前状态条

APPFlow 内嵌 AgentChat 当前显示：

```text
任务：task_xxx
阶段：已规划 / 生成中 / 已生成 / 校验中 / 已校验 / 运行中 / 已运行 / 导入中 / 已完成
下一步：生成算例 / 校验算例 / 运行 OpenFOAM / 导入到 APPFlow / 无
清单：未收到 / 已收到
```

这部分完全由 APPFlow 前端根据后端返回字段更新。

## 当前边界

当前已经支持：

```text
OpenFOAM 分步骤联动
next_action 展示
用户输入“继续”触发下一步
manifest 导入 APPFlow
任务状态条展示
Docker 输出路径映射
求解参数只读摘要
参数修改请求识别和 /foam/replan 调用入口
Agent 普通回复前端逐字显示
```

当前暂不支持：

```text
后端 SSE / WebSocket 真流式输出
LLM 意图解析
APPFlow 本地执行参数修改
APPFlow 表单参数反向同步到 Agent
APPFlow 自动写入内部求解器参数模型
PHengLEI 实际运行适配
未知 next_action 自动执行
```

## 后端最小验收清单

Agent 端继续更新时，建议按下面顺序验收：

```text
1. /health 返回 200
2. /foam/plan 返回 task.task_dir、task.manifest_path、next_action.id=generate
3. /foam/plan 返回 plan.solver_name 和 plan.requested_changes 时，APPFlow 显示求解参数摘要
4. /foam/generate 返回 next_action.id=validate
5. /foam/validate 成功时返回 next_action.id=run
6. /foam/run 成功时返回 next_action.id=import_manifest
7. manifest_path 能被 APPFlow 映射到主机真实文件
8. agent_manifest_v1.json 能被 actionLoadAgentManifest 读取
9. APPFlow 最终状态显示“已完成”
10. 参数修改请求要求 APPFlow 能调用 /foam/replan，并在占位实现下回到 next_action.id=generate
```
