# AgentBridge

AgentBridge 定义外部 Agent 工作流与 APPFlow 之间的产物交接契约。

第一阶段仍然保持 Agent 端到端运行。Agent 完成完整仿真流程后，写出 `agent_manifest_v1.json`。APPFlow 读取这个 manifest，用它定位 Agent 生成的算例、网格、日志和结果。

## 第一阶段目标

```text
Agent 端到端仿真流程
  -> 写出 agent_manifest_v1.json
  -> APPFlow 读取 manifest
  -> APPFlow 导入网格、显示日志、打开结果
```

在这个阶段，APPFlow 不控制 Agent 的每一个中间步骤，只消费 Agent 工作流结束后的最终产物。

## 预期 Agent 输出

```text
agent_output/
  agent_manifest_v1.json
  case/
    system/
    constant/
    0/
  logs/
  results/
```

实际文件夹名称可以调整，但 `agent_manifest_v1.json` 必须指向真实存在的产物路径。

## 示例文件

```text
examples/minimal_manifest_v1.json
  只包含最小必需字段。用于验证 Agent 输出 manifest 的最小契约。

examples/agent_manifest_v1.json
  已有 VTK 结果的 OpenFOAM 示例。APPFlow 可以直接导入网格并打开 ParaView。

examples/test.json
  没有预生成 VTK 的 OpenFOAM 示例。APPFlow 可以先通过 Docker 执行 foamToVTK，再打开 ParaView。

examples/phenglei_minimal_manifest_v1.json
  PHengLEI 最小示例。当前 APPFlow 只支持 OpenFOAM，因此这个示例用于验证 APPFlow 会明确提示不处理 PHengLEI，而不是误走 OpenFOAM 逻辑。
```

后续迁移到 `appPHengLEI` 时，可参考：

```text
MIGRATION_TO_APP_PHENGLEI.zh-CN.md
```

Agent 端开发者对接 APPFlow 时，可参考：

```text
AGENT_OUTPUT_CONTRACT.zh-CN.md
```

真实 Agent 项目接入 APPFlow 前的最小验收步骤，可参考：

```text
REAL_AGENT_CHECKLIST.zh-CN.md
```

Agent UI 与 Docker 后端联动时的 Host / Backend 输出路径配置，可参考：

```text
CHAT_UI_PATH_SETUP.zh-CN.md
```

当前 APPFlow 侧已经实现和验证到哪一步，可参考：

```text
CURRENT_STATUS.zh-CN.md
```

## Manifest 结构

v1 manifest 使用五个顶层字段：

```json
{
  "version": 1,
  "workflow": {},
  "solver": {},
  "artifacts": {},
  "case_summary": {},
  "appflow_hints": {}
}
```

### 最小必需字段

这些字段是 APPFlow 加载 manifest 并定位算例所需的最小契约。其他字段都应理解为可选增强，而不是每次都必须输出。

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `version` | integer | Manifest 协议版本。当前版本为 `1`。 |
| `workflow.id` | string | Agent 工作流 ID 或算例 ID。 |
| `workflow.status` | string | 工作流结果：`succeeded`、`failed` 或 `partial`。 |
| `solver.family` | string | 求解器后端类型，例如 `openfoam`，后续可扩展为 `phenglei`。 |
| `artifacts.case_dir` | string | Agent 生成的求解器算例目录。 |

### 可选产物字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `artifacts.mesh_dir` | string | 网格目录。OpenFOAM 通常是 `case/constant/polyMesh`。 |
| `artifacts.result_dir` | string | 结果目录，例如 OpenFOAM 的 `case/VTK`。 |
| `artifacts.logs` | object array | APPFlow 可以显示的日志列表。 |

### 可选展示字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `workflow.created_at` | string | 用于界面显示和排序的时间戳。 |
| `solver.command` | string | 实际求解器命令，例如 `simpleFoam`。 |
| `solver.case_type` | string | 粗粒度算例类型，例如 `steady_incompressible`。 |
| `case_summary.case_name` | string | 面向用户显示的算例名称。 |
| `case_summary.mesh_format` | string | 网格格式，例如 `openfoam_polyMesh`，后续也可以是 PHengLEI 网格类型。 |
| `case_summary.fields` | string array | 主要场变量，例如 `U`、`p`、`T`。 |
| `case_summary.boundaries` | object array | 边界名称和粗粒度边界类型。 |

### 可选行为 hints

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `appflow_hints.import_mesh` | boolean | APPFlow 是否应尝试自动导入网格。 |
| `appflow_hints.show_logs` | boolean | APPFlow 读取 manifest 后是否应显示日志。 |
| `appflow_hints.show_results` | boolean | APPFlow 读取 manifest 后是否应显示结果。 |
| `appflow_hints.preferred_result_format` | string | 优先结果格式，例如 `vtk`。 |
| `appflow_hints.open_paraview` | boolean | APPFlow 是否在检测到 VTK 结果后打开 ParaView。 |
| `appflow_hints.paraview_open_mode` | string | ParaView 打开方式。`openfoam_case` 表示创建 `case.foam` 并按 APPFlow 原生方式打开 OpenFOAM case；`first_vtk` 表示打开第一个 `.vtk` 文件。 |
| `appflow_hints.export_vtk` | boolean | 当存在 OpenFOAM 时间步目录但缺少 VTK 结果时，APPFlow 是否执行 `foamToVTK -ascii -case <case_dir>`。 |
| `appflow_hints.foam_to_vtk_backend` | string | 可选 VTK 导出后端。`native` 表示本机 `foamToVTK`，`docker` 表示通过 Docker 中的 OpenFOAM 执行。 |
| `appflow_hints.docker_image` | string | 当 `foam_to_vtk_backend` 为 `docker` 时使用的 Docker 镜像，例如 `foamagent`。 |
| `appflow_hints.docker_case_dir` | string | OpenFOAM case 在容器内的挂载路径，默认是 `/case`。 |

## APPFlow 第一阶段行为

APPFlow 初始阶段支持这些动作：

```text
load_agent_manifest
import_mesh_if_exists
show_logs
show_result_if_exists
export_vtk_if_requested
refresh_tree_and_view
```

对于 OpenFOAM，`artifacts.mesh_dir` 已连接到 APPFlow 当前已有的 OpenFOAM 网格导入流程。对于 PHengLEI，同样复用这套 manifest 结构，只是由 PHengLEI 求解器适配器解释对应的路径和文件。

### APPFlow 原生 ParaView / VTK 路径

当前 APPFlow 原有后处理界面主要有三条路径：

```text
ParaView 按钮
  -> 在 case 目录下创建空文件 case.foam
  -> 启动 paraview --case <case_dir>/case.foam
  -> 由 ParaView 按 OpenFOAM case 读取时间步结果

Export 按钮
  -> 执行 foamToVTK -ascii -case <case_dir>
  -> 输出结果目录 case/VTK

Post 按钮
  -> 启动自研后处理程序 CFDPostAPP
  -> 参数为 -i <working_dir>/case/VTK
```

也就是说，APPFlow 原生 ParaView 入口更偏向打开 OpenFOAM case，而不是直接打开某一个 `.vtk` 文件。AgentBridge 现在支持两种 ParaView 打开模式：

```text
openfoam_case
  创建 <case_dir>/case.foam，并执行 paraview --case <case_dir>/case.foam。
  这个方式更贴近 APPFlow 原有 ParaView 按钮逻辑，推荐用于完整 OpenFOAM case。

first_vtk
  打开 result_dir 中第一个 .vtk 文件。这个方式作为兼容模式保留。
```

配置字段放在 `appflow_hints.paraview_open_mode`。在没有明确配置时，继续保持已验证的 `first_vtk` 行为，避免影响现有最小闭环。

### 当前应用边界

当前边界已经明确：

```text
APPFlow
  只处理 solver.family = openfoam
  遇到 phenglei 或其他 solver.family 时，只输出“不支持”提示
  不进入 OpenFoamAgentAdapter
  不尝试导入网格、导出 VTK 或打开 ParaView

appPHengLEI
  后续只处理 solver.family = phenglei
  复用同一套 manifest 协议
  复用 AgentManifestData / AgentLogPreview 等通用层
  使用独立的 PHengLEI adapter 处理 PHengLEI case、网格和结果
```

因此当前不在 APPFlow 中实现 PHengLEI 适配器，也不建议在 `appPHengLEI` 中实现 OpenFOAM 适配器。两个应用共享协议和通用解析层，但求解器适配逻辑分别留在各自应用中。

## 当前代码结构

当前 APPFlow 实现已经拆成几个小层，目的是让核心桥接逻辑后续可以迁移复用：

```text
OperatorsAgentManifest
  APPFlow 绑定层：菜单 action、文件选择、FITK 消息输出、APPFlow 原有网格导入算子。

AgentManifestData
  可复用 manifest 层：读取 JSON、校验 v1 必需字段、暴露 workflow / solver / artifacts / hints。

AgentLogPreview
  可复用日志层：检查日志文件是否存在，并追加短日志预览。

OpenFoamAgentAdapter
  OpenFOAM 适配层：检查结果文件、检测时间步目录、执行 native 或 Docker foamToVTK、打开 ParaView。
```

后续迁移到 `appPHengLEI` 时，优先复用 `AgentManifestData` 和 `AgentLogPreview`。每个应用保留自己的 UI 绑定层，PHengLEI 相关逻辑应新增独立的 PHengLEI 适配器，不要继续塞进 `OperatorsAgentManifest`。

### 当前层交互关系

用户在 APPFlow 中点击 `Import Agent Result` 后，当前调用关系是：

```text
OperatorsAgentManifest
  -> execGUI()
     -> 弹出 JSON 文件选择框
     -> 将选中的 manifest 路径保存到 operator 参数

OperatorsAgentManifest
  -> execProfession()
     -> AgentManifestData::loadAgentManifest()
        -> 读取 agent_manifest_v1.json
        -> 校验 version / workflow / solver / artifacts
        -> 返回 AgentManifestData

     -> 输出 workflow / solver / case / mesh 摘要信息

     -> OpenFoamAgentAdapter::processResults()
        -> 检查 result_dir 是否存在
        -> 扫描 VTK / VTP / VTU 等结果文件
        -> 如果没有 VTK，检测 OpenFOAM 时间步目录
        -> 如果 export_vtk=true，执行 native 或 Docker foamToVTK
        -> 返回 OpenFoamAgentResult

     -> AgentLogPreview::appendAgentLogPreviewMessages()
        -> 检查 manifest 中列出的日志是否存在
        -> 读取日志前 80 行或最多 8000 字符
        -> 追加日志预览到消息列表

     -> FITKOPERREPO actionImportOpenFoamMesh
        -> 复用 APPFlow 原有 OpenFOAM 网格导入流程
        -> 刷新网格、树节点和显示窗口

     -> OpenFoamAgentAdapter::openParaViewIfRequested()
        -> 如果 open_paraview=true
        -> 按 appflow_hints.paraview_open_mode 打开 ParaView
        -> openfoam_case 创建 case.foam 并按 APPFlow 原生方式打开 OpenFOAM case
        -> first_vtk 打开第一个 VTK 结果文件

     -> FITKMessageNormal()
        -> 将完整流程日志输出到 APPFlow 消息窗口
```

当前交互关系可以概括为：

```text
APPFlow UI 入口
  -> 通用 manifest 解析
  -> 求解器适配器处理结果
  -> 通用日志预览
  -> APPFlow 原有网格导入
  -> 外部后处理程序打开结果
```

## 后续扩展

当端到端集成跑通后，这个契约可以继续扩展为分步骤事件：

```text
plan_ready
mesh_ready
case_config_ready
run_started
run_log
run_finished
result_ready
```

同一套顶层结构后续应继续支持 OpenFOAM 和 PHengLEI。差异部分放到各自的求解器适配器中处理。
