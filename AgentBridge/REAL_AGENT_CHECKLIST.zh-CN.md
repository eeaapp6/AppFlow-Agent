# 真实 Agent 对接验收清单

本文档用于真实 Agent 项目接入 APPFlow 前的最小验收。它关注“Agent 输出是否能被 APPFlow 消费”，不要求 Agent 和 APPFlow 立即实现分步骤双向控制。

当前目标是先跑通最小闭环：

```text
真实 Agent 端到端运行
  -> 生成 OpenFOAM case
  -> 写出 agent_manifest_v1.json
  -> APPFlow 读取 manifest
  -> APPFlow 导入网格
  -> APPFlow 检查日志和结果
  -> APPFlow 打开 ParaView
```

## 一、Agent 必须交付什么

真实 Agent 至少需要交付：

```text
agent_output/
  agent_manifest_v1.json
  case/
    system/
    constant/
    0/
```

如果已经完成网格生成，建议同时交付：

```text
case/
  constant/
    polyMesh/
```

如果 Agent 已经完成后处理，建议同时交付：

```text
case/
  VTK/
    *.vtk
```

如果 Agent 没有执行 `foamToVTK`，可以只交付 OpenFOAM 原始结果时间步目录，例如：

```text
case/
  0/
  0.1/
  0.2/
  0.5/
```

这种情况下，APPFlow 可以按配置尝试调用本机或 Docker 中的 `foamToVTK`。

## 二、最小 manifest

真实 Agent 第一次对接时，先只输出最小 manifest。这样可以先验证 APPFlow 能不能定位 case，而不是一开始就调试大量参数字段。

```json
{
  "version": 1,
  "workflow": {
    "id": "real_agent_case_001",
    "status": "succeeded"
  },
  "solver": {
    "family": "openfoam"
  },
  "artifacts": {
    "case_dir": "D:/work/real_agent_case_001/case"
  }
}
```

最小验收通过后，再逐步补充 `mesh_dir`、`result_dir`、`logs`、`solver_settings` 和 `appflow_hints`。

## 三、有 VTK 结果时怎么写

如果 Agent 已经生成了 `case/VTK/*.vtk`，推荐使用 `first_vtk` 模式：

```json
{
  "artifacts": {
    "case_dir": "D:/work/real_agent_case_001/case",
    "mesh_dir": "D:/work/real_agent_case_001/case/constant/polyMesh",
    "result_dir": "D:/work/real_agent_case_001/case/VTK"
  },
  "appflow_hints": {
    "import_mesh": true,
    "open_paraview": true,
    "paraview_open_mode": "first_vtk"
  }
}
```

这种方式最直接：APPFlow 扫描 VTK 文件，然后请求 ParaView 打开第一个 VTK 文件。

## 四、没有 VTK 结果时怎么写

如果 Agent 只生成了 OpenFOAM 原始结果，没有执行 `foamToVTK`，可以让 APPFlow 尝试后处理。

本机有 OpenFOAM 环境时：

```json
{
  "artifacts": {
    "case_dir": "D:/work/real_agent_case_001/case",
    "mesh_dir": "D:/work/real_agent_case_001/case/constant/polyMesh",
    "result_dir": "D:/work/real_agent_case_001/case/VTK"
  },
  "appflow_hints": {
    "import_mesh": true,
    "run_foam_to_vtk": true,
    "foam_to_vtk_backend": "native",
    "open_paraview": true,
    "paraview_open_mode": "first_vtk"
  }
}
```

Windows 本机没有 OpenFOAM，但 Docker 中有 OpenFOAM 时：

```json
{
  "artifacts": {
    "case_dir": "D:/work/real_agent_case_001/case",
    "mesh_dir": "D:/work/real_agent_case_001/case/constant/polyMesh",
    "result_dir": "D:/work/real_agent_case_001/case/VTK"
  },
  "appflow_hints": {
    "import_mesh": true,
    "run_foam_to_vtk": true,
    "foam_to_vtk_backend": "docker",
    "docker_image": "leoyue123/foamagent:latest",
    "docker_case_dir": "/case",
    "open_paraview": true,
    "paraview_open_mode": "first_vtk"
  }
}
```

注意：`docker_case_dir` 是容器内部路径，不是 Windows 路径。

## 五、日志怎么写

Agent 建议把关键运行日志写入 `artifacts.logs` 数组：

```json
{
  "artifacts": {
    "logs": [
      {
        "type": "blockMesh",
        "name": "blockMesh",
        "path": "D:/work/real_agent_case_001/logs/blockMesh.log"
      },
      {
        "type": "icoFoam",
        "name": "icoFoam",
        "path": "D:/work/real_agent_case_001/logs/icoFoam.log"
      }
    ]
  }
}
```

APPFlow 会检查日志文件是否存在，并预览前若干行。这样可以快速判断错误属于网格生成、求解器运行还是后处理阶段。

## 六、solver_settings 怎么写

`solver_settings` 当前已经不是纯只读字段。APPFlow 会先展示摘要，并在 manifest 导入时尝试把少量稳定字段写入内部参数模型。

当前已支持写入：

| Agent 字段 | APPFlow 写入位置 |
| --- | --- |
| `solver.command = simpleFoam` | APPFlow `SIMPLE` 求解器。 |
| `solver.command = interFoam` | APPFlow `Inter` 求解器。 |
| `solver_settings.time.delta_t` | `RunControl` 时间步相关字段。 |
| `solver_settings.time.end_time` | `RunControl` 仿真结束时间。 |
| `solver_settings.time.write_interval` | `RunControl` 输出间隔。 |
| `solver_settings.physics.nu` | Transport Properties 运动粘度 `v [m2/s]`。 |

建议真实 Agent 先输出少量关键字段：

```json
{
  "solver_settings": {
    "time": {
      "start_time": 0,
      "end_time": 0.5,
      "delta_t": 0.005
    },
    "physics": {
      "nu": 0.01
    },
    "numerics": {
      "solver": "icoFoam"
    }
  }
}
```

不要一开始把所有 OpenFOAM 字典都塞进 manifest。大型字典文件仍然应该保留在 `case/system`、`case/constant` 和 `case/0` 中，manifest 只写 APPFlow 需要快速展示和定位的摘要信息。

## 七、boundary_conditions 怎么写

边界条件建议作为独立可选字段输出，不要放进 `solver_settings`。

最小建议格式：

```json
{
  "boundary_conditions": [
    {
      "patch": "movingWall",
      "appflow_type": "Wall",
      "fields": {
        "U": {
          "type": "Fixed Value",
          "value": [1.0, 0.0, 0.0]
        },
        "p": {
          "type": "Zero Gradient"
        }
      }
    }
  ]
}
```

真实 Agent 输出时需要注意：

- `patch` 必须和 OpenFOAM `constant/polyMesh/boundary` 中的 patch 名称一致。
- `appflow_type` 先使用粗类型，例如 `Wall`、`Velocity Inlet`、`Pressure Outlet`。
- `fields` 中只放 APPFlow 当前需要展示或后续可映射的变量，例如 `U`、`p`。
- `empty/frontAndBack` 第一阶段可以输出，但 APPFlow 暂时只读或跳过，不强制自动写入。

APPFlow 当前还没有自动写入 `BoundaryManager`。后续要先做 patch 名称匹配，再拿到真实 mesh boundary ID，最后调用 APPFlow 现有边界接口。

## 八、APPFlow 侧验收顺序

真实 Agent 第一次接入时，建议按下面顺序验收。

### 1. 验证 manifest 能加载

预期日志：

```text
Agent manifest loaded: ...
Workflow: ...
Status: succeeded
Solver family: openfoam
Case dir: ...
```

如果这里失败，优先检查：

- JSON 是否合法。
- `version` 是否为 `1`。
- `solver.family` 是否为 `openfoam`。
- `artifacts.case_dir` 是否写对。

### 2. 验证 case 目录存在

预期日志：

```text
Case dir exists: yes
```

如果是 `no`，先不要调试网格、日志和 ParaView，先修正路径。

### 3. 验证网格导入

预期日志：

```text
OpenFOAM mesh import requested from: ...
read succeed ! .../constant/
```

如果网格导入失败，优先检查：

- `case/constant/polyMesh` 是否存在。
- `points`、`faces`、`owner`、`neighbour`、`boundary` 是否完整。
- `case_dir` 是否指向 OpenFOAM case 根目录，而不是 `constant` 或 `polyMesh`。

### 4. 验证结果识别

有 VTK 时，预期日志：

```text
Result dir exists: yes
Result files:
 *.vtk: ...
VTK result is ready: ...
```

没有 VTK 但有 OpenFOAM 时间步时，预期日志：

```text
OpenFOAM time directories detected: ...
VTK export not detected. foamToVTK may be required.
```

### 5. 验证 ParaView 打开

`first_vtk` 模式预期日志：

```text
ParaView open mode: first_vtk
ParaView open requested for: .../*.vtk
```

`openfoam_case` 模式预期日志：

```text
ParaView open mode: openfoam_case
ParaView OpenFOAM case file: .../case.foam
ParaView open requested for OpenFOAM case: .../case.foam
```

如果 ParaView 没打开，优先检查：

- `paraview.exe` 是否在 PATH 中。
- 修改环境变量后是否重启了 VS2017。
- manifest 中是否设置了 `open_paraview: true`。

## 九、通过标准

真实 Agent 对接的第一阶段通过标准：

- APPFlow 能成功读取真实 Agent 输出的 manifest。
- APPFlow 能识别 `solver.family = openfoam`。
- APPFlow 能定位真实存在的 `case_dir`。
- APPFlow 能导入 OpenFOAM 网格。
- APPFlow 能检查日志文件是否存在。
- APPFlow 能识别已有 VTK，或者能提示需要 `foamToVTK`。
- 在配置正确时，APPFlow 能请求 ParaView 打开结果。

只要这些通过，就说明第一阶段“端到端 Agent 产物接入 APPFlow”的主流程已经跑通。

## 十、已验证案例

当前已经验证过一次真实 Agent 运行结果接入 APPFlow。

使用的 manifest：

```text
APPFlow_5936/AgentBridge/examples/new.json
```

对应算例：

```text
D:/work/demo_case_003/case
```

真实 Agent 原始任务：

```text
task_20260517_074014_ba62d388
```

该案例特征：

- `solver.family = openfoam`
- `solver.command = icoFoam`
- `case_summary.case_name = cavity`
- `case/constant/polyMesh` 存在且可导入
- `case` 根目录包含 `0.005` 到 `0.5` 的 OpenFOAM 时间步结果
- `case/VTK` 初始不存在
- `artifacts.logs` 中包含 `blockMesh.log` 和 `icoFoam.log`
- `solver_settings` 中包含时间步、结束时间、输出间隔、运动粘度和求解器名称
- `appflow_hints.run_foam_to_vtk = true`
- `appflow_hints.foam_to_vtk_backend = docker`
- `appflow_hints.paraview_open_mode = first_vtk`

已验证 APPFlow 行为：

```text
Agent manifest loaded
Case dir exists: yes
Mesh dir exists: yes
OpenFOAM time directories detected
VTK export not detected. foamToVTK may be required.
Solver settings 显示正常
blockMesh / icoFoam 日志预览正常
OpenFOAM mesh import requested
read succeed
Docker foamToVTK 可被 run_foam_to_vtk 触发
```

这个案例可以作为后续回归测试样例。只要以后改 Agent 输出、APPFlow 适配器、foamToVTK 或 ParaView 打开逻辑，都应优先用这个案例重新验证。

## 十一、暂时不要做什么

真实 Agent 第一次接入时，暂时不要同时做下面这些事：

- 不要一开始就做 Agent 分步骤事件同步。
- 不要一开始就要求 APPFlow 反向控制 Agent 每个步骤。
- 不要一开始就把全部 OpenFOAM 字典内容展开到 manifest。
- 不要一开始就把全部 `solver_settings` 字段都强行写入 APPFlow 内部参数模型。
- 不要一开始就把 PHengLEI 和 OpenFOAM 混在同一个 APPFlow 验收流程里。

先保证最小闭环稳定，再逐步扩展。
