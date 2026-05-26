# Agent 端 `/foam/replan` 实现任务清单

本文档给 Agent / Foam-Agent 后端开发者使用，目标是把 APPFlow 已经预留好的参数修改入口真正接上。

当前 APPFlow 已经能做到：

```text
用户在 AgentChat 输入参数修改请求
  -> APPFlow 识别这是已有任务的参数修改
  -> APPFlow 调用 POST /foam/replan
  -> APPFlow 接收新的 task / plan / next_action
  -> APPFlow 刷新只读求解参数摘要
  -> 用户继续输入“继续”
  -> APPFlow 调用 /foam/generate
```

后端需要补齐的是：

```text
/foam/replan 真正理解参数修改
  -> 更新 task_context.json 里的 plan
  -> 后续 /foam/generate 根据新 plan 重写 OpenFOAM case
```

## 1. 接口目标

`/foam/replan` 用于处理已有任务上的参数修改请求。

它不是新建任务接口，也不是直接运行接口。

推荐职责划分：

```text
/foam/replan
  只负责理解用户修改意图、更新 plan、保存 task_context.json

/foam/generate
  根据更新后的 plan 生成或重写 OpenFOAM case 文件

/foam/validate
  检查重生成后的 case

/foam/run
  运行重生成后的 case
```

这样 APPFlow、Agent 和 OpenFOAM case 文件之间的责任边界比较清楚。

## 2. APPFlow 请求格式

APPFlow 当前会向后端发送：

```json
{
  "task_dir": "/workspace/outputs/task_20260518_001",
  "message": "把 end_time 改成 1.0",
  "mode": "replan",
  "output_dir": "/workspace/outputs",
  "host_output_dir": "D:/work/FoamAgentOutputs"
}
```

字段说明：

| 字段 | 含义 |
| --- | --- |
| `task_dir` | 后端可访问的任务目录，里面应有 `task_context.json` |
| `message` | 用户本次参数修改请求 |
| `mode` | 当前固定为 `replan` |
| `output_dir` | 后端输出根目录，Docker 场景通常是 `/workspace/outputs` |
| `host_output_dir` | APPFlow 主机侧输出根目录，用于路径映射 |

后端必须至少使用：

```text
task_dir
message
```

## 3. 后端返回格式

成功时建议返回：

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
      "write_interval": "1",
      "kinematic_viscosity": "0.01"
    }
  },
  "replan": {
    "status": "updated",
    "applied_changes": {
      "end_time": "1.0"
    },
    "unrecognized_changes": {}
  },
  "next_action": {
    "id": "generate",
    "label": "重新生成算例",
    "endpoint": "/foam/generate",
    "requires_confirmation": true
  }
}
```

APPFlow 当前重点读取：

```text
task.task_dir
task.status
task.manifest_path
plan.solver_name
plan.requested_changes
plan.parameters.requested_changes
next_action.id
next_action.label
next_action.endpoint
```

其中 `next_action.id` 必须返回：

```text
generate
```

这样用户输入“继续”后，APPFlow 会调用 `/foam/generate`。

## 4. 最小参数集合

第一版建议只支持这些参数：

| 用户参数 | plan 中建议字段 | OpenFOAM 目标文件 |
| --- | --- | --- |
| `end_time` | `requested_changes.end_time` | `case/system/controlDict` 的 `endTime` |
| `delta_t` | `requested_changes.delta_t` | `case/system/controlDict` 的 `deltaT` |
| `write_interval` | `requested_changes.write_interval` | `case/system/controlDict` 的 `writeInterval` |
| `nu` | `requested_changes.kinematic_viscosity` 或 `requested_changes.nu` | `case/constant/physicalProperties` 或对应物性文件 |

建议支持的用户表达：

```text
把 end_time 改成 1.0
修改 delta_t 为 0.001
把 write_interval 改成 10
把 nu 改成 0.005
set end_time to 1.0
set delta_t to 0.001
```

暂时不建议一开始支持复杂表达，例如：

```text
把时间步调小一点
让结果更稳定
提高雷诺数
```

这些可以后续交给 LLM 意图解析增强。

## 5. `task_context.json` 更新要求

`/foam/replan` 应读取：

```text
task_dir/task_context.json
```

然后更新其中的：

```text
status = "planned"
plan.requested_changes
plan.parameters.requested_changes
updated_at
```

如果当前 `plan.parameters.requested_changes` 存在，建议同步更新，方便不同读取路径都能拿到新值。

示例：

```json
{
  "status": "planned",
  "plan": {
    "solver_name": "icoFoam",
    "requested_changes": {
      "delta_t": "0.005",
      "end_time": "1.0",
      "kinematic_viscosity": "0.01"
    },
    "parameters": {
      "requested_changes": {
        "delta_t": "0.005",
        "end_time": "1.0",
        "kinematic_viscosity": "0.01"
      }
    }
  }
}
```

## 6. `/foam/generate` 的后续要求

`/foam/replan` 更新 plan 后，真正改 OpenFOAM case 的动作应发生在 `/foam/generate`。

也就是说：

```text
用户：把 end_time 改成 1.0
  -> /foam/replan 更新 plan
用户：继续
  -> /foam/generate 根据新 plan 重写 case/system/controlDict
```

验证时应能看到：

```text
case/system/controlDict
endTime 1.0;
```

如果修改 `delta_t`，应能看到：

```text
deltaT 0.001;
```

如果修改 `write_interval`，应能看到：

```text
writeInterval 10;
```

如果修改 `nu`，应能看到对应物性文件中的值更新。

## 7. 错误处理建议

如果缺少 `task_dir`：

```json
{
  "error": "task_dir must not be empty."
}
```

如果缺少 `message`：

```json
{
  "error": "Message must not be empty."
}
```

如果 `task_context.json` 不存在：

```json
{
  "error": "task_context.json not found: ..."
}
```

如果用户请求无法识别：

```json
{
  "reply": "No supported parameter changes were detected.",
  "task": {...},
  "plan": {...},
  "replan": {
    "status": "no_supported_changes",
    "applied_changes": {},
    "unrecognized_changes": {
      "message": "把结果调稳定一点"
    }
  },
  "next_action": {}
}
```

不建议在无法识别参数时直接重生成 case。

## 8. APPFlow 验收步骤

第一阶段验收：

```text
1. 在 APPFlow AgentChat 中创建一个 OpenFOAM 任务
2. 等 APPFlow 显示求解参数摘要
3. 输入：把 end_time 改成 1.0
4. APPFlow 调用 /foam/replan
5. 后端返回新的 plan
6. APPFlow 聊天区显示新的求解参数摘要
7. 状态条显示：已规划
8. 下一步显示：重新生成算例 或 生成算例
```

第二阶段验收：

```text
1. 在第一阶段基础上输入：继续
2. APPFlow 调用 /foam/generate
3. 打开生成的 case/system/controlDict
4. 确认 endTime 已经变成 1.0
5. 继续 validate / run
6. 确认修改后的 case 可以正常运行
```

## 9. 当前边界

APPFlow 不负责：

```text
解析复杂参数语义
修改 OpenFOAM 字典
判断哪些参数适合哪个求解器
重写 case 文件
```

Agent 后端负责：

```text
理解用户参数修改请求
更新 plan
生成或重生成 OpenFOAM case
保证 validate / run 使用的是更新后的 case
```

这就是当前方案 A 的核心边界。
