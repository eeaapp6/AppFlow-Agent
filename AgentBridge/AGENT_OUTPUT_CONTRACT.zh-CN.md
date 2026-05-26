# Agent 输出对接清单

本文档面向后续 Agent 界面和 Agent 工作流开发者，说明 Agent 端到端跑完一次 OpenFOAM 仿真后，需要交给 APPFlow 的最小产物和建议产物。

当前目标不是让 Agent 和 APPFlow 分步骤双向控制，而是先跑通：

```text
用户自然语言需求
  -> Agent 分析任务
  -> 生成 OpenFOAM case
  -> 网格划分
  -> 求解器配置
  -> 运行仿真
  -> 后处理可视化产物
  -> 写出 agent_manifest_v1.json
  -> APPFlow 读取 manifest 并导入结果
```

## 一句话要求

Agent 端至少要交付一个真实存在的 OpenFOAM case 目录，以及一个指向该目录的 `agent_manifest_v1.json`。

最小闭环只要求：

```text
agent_manifest_v1.json
case/
  system/
  constant/
  0/
```

如果已经有网格，则还应有：

```text
case/
  constant/
    polyMesh/
```

如果已经有 VTK 后处理结果，则还应有：

```text
case/
  VTK/
    *.vtk
```

## 推荐输出目录

推荐 Agent 使用下面的目录组织方式：

```text
agent_output/
  agent_manifest_v1.json
  case/
    system/
      controlDict
      fvSchemes
      fvSolution
    constant/
      transportProperties
      polyMesh/
    0/
      U
      p
  logs/
    blockMesh.log
    checkMesh.log
    solver.log
  summary/
    agent_report.md
```

目录名可以不同，但 manifest 中的路径必须指向真实文件或目录。APPFlow 不根据固定文件夹名猜测产物，优先相信 manifest。

## 最小 Manifest

这是 Agent 必须能输出的最小 JSON：

```json
{
  "version": 1,
  "workflow": {
    "id": "case_001",
    "status": "succeeded"
  },
  "solver": {
    "family": "openfoam"
  },
  "artifacts": {
    "case_dir": "D:/work/case_001/case"
  }
}
```

最小字段说明：

| 字段 | 是否必需 | 说明 |
| --- | --- | --- |
| `version` | 是 | 当前固定为 `1`。 |
| `workflow.id` | 是 | 本次 Agent 任务或算例 ID。 |
| `workflow.status` | 是 | 建议使用 `succeeded`、`failed`、`partial`。 |
| `solver.family` | 是 | APPFlow 当前只处理 `openfoam`。 |
| `artifacts.case_dir` | 是 | OpenFOAM case 根目录。 |

## 推荐 Manifest

真实联调时，建议 Agent 输出更多上下文，便于 APPFlow 展示和定位问题：

```json
{
  "version": 1,
  "workflow": {
    "id": "lid_driven_cavity_001",
    "status": "succeeded",
    "created_at": "2026-05-17T10:30:00+08:00"
  },
  "solver": {
    "family": "openfoam",
    "command": "icoFoam",
    "case_type": "incompressible"
  },
  "artifacts": {
    "case_dir": "D:/work/lid_driven_cavity_001/case",
    "mesh_dir": "D:/work/lid_driven_cavity_001/case/constant/polyMesh",
    "result_dir": "D:/work/lid_driven_cavity_001/case/VTK",
    "logs": [
      {
        "type": "blockMesh",
        "path": "D:/work/lid_driven_cavity_001/logs/blockMesh.log"
      },
      {
        "type": "solver",
        "path": "D:/work/lid_driven_cavity_001/logs/icoFoam.log"
      }
    ]
  },
  "case_summary": {
    "case_name": "2D lid driven cavity",
    "mesh_format": "openfoam_polyMesh",
    "fields": ["U", "p"],
    "boundaries": [
      {
        "name": "movingWall",
        "type": "wall"
      },
      {
        "name": "fixedWalls",
        "type": "wall"
      }
    ]
  },
  "appflow_hints": {
    "import_mesh": true,
    "show_logs": true,
    "show_results": true,
    "preferred_result_format": "vtk",
    "open_paraview": true,
    "export_vtk": true,
    "foam_to_vtk_backend": "docker",
    "docker_image": "foamagent",
    "docker_case_dir": "/case"
  }
}
```

## OpenFOAM Case 要求

Agent 生成的 OpenFOAM case 应至少能让 OpenFOAM 命令识别：

```text
case/
  system/
    controlDict
    fvSchemes
    fvSolution
  constant/
  0/
```

各目录作用：

| 目录或文件 | 作用 |
| --- | --- |
| `system/controlDict` | 控制起止时间、时间步、写出间隔、运行控制。 |
| `system/fvSchemes` | 控制离散格式。 |
| `system/fvSolution` | 控制线性求解器、残差、松弛因子等。 |
| `constant/transportProperties` | 保存物性参数，例如运动黏度。 |
| `constant/polyMesh` | OpenFOAM 网格目录，通常由 `blockMesh` 或 `snappyHexMesh` 生成。 |
| `0/U`、`0/p` | 初始场和边界条件。 |

如果 Agent 报告 `workflow.status = succeeded`，建议保证至少完成：

```text
blockMesh 或 snappyHexMesh 成功
checkMesh 建议成功
求解器命令成功
日志文件可读
```

## VTK 和 ParaView 要求

Agent 可以选择两种方式。

方式一：Agent 已经执行 `foamToVTK`

```json
{
  "artifacts": {
    "result_dir": "D:/work/case_001/case/VTK"
  },
  "appflow_hints": {
    "open_paraview": true,
    "export_vtk": false
  }
}
```

这种情况下，APPFlow 会扫描 `result_dir`，找到 `.vtk` 文件后打开 ParaView。

方式二：Agent 没有执行 `foamToVTK`

```json
{
  "artifacts": {
    "case_dir": "D:/work/case_001/case",
    "result_dir": "D:/work/case_001/case/VTK"
  },
  "appflow_hints": {
    "open_paraview": true,
    "export_vtk": true,
    "foam_to_vtk_backend": "docker",
    "docker_image": "foamagent",
    "docker_case_dir": "/case"
  }
}
```

这种情况下，APPFlow 会尝试通过配置的后端执行 `foamToVTK`。Windows 本机没有 OpenFOAM 时，推荐使用 Docker 后端。

## 求解器参数同步建议

当前 APPFlow v1 已经能读取和展示 `solver.command`、`solver.case_type`、`case_summary` 等摘要字段。现在也已经支持把部分 `solver_settings` 同步到 APPFlow 内部参数面板。

当前已支持的写入范围：

| Agent 字段 | APPFlow 当前写入位置 |
| --- | --- |
| `solver.command = simpleFoam` | 自动映射到 APPFlow `SIMPLE` 求解器。 |
| `solver.command = interFoam` | 自动映射到 APPFlow `Inter` 求解器。 |
| `solver_settings.time.delta_t` | 写入 `RunControl` 中与时间步相关的字段。 |
| `solver_settings.time.end_time` | 写入 `RunControl` 的仿真结束时间。 |
| `solver_settings.time.write_interval` | 写入 `RunControl` 的输出间隔。 |
| `solver_settings.physics.nu` | 写入 Transport Properties 中的运动粘度字段 `v [m2/s]`。 |

当前暂不支持的范围：

```text
icoFoam 没有直接映射到 APPFlow 内置求解器。
fvSchemes / fvSolution 暂不自动写入。
边界条件暂不自动写入 BoundaryManager。
复杂多相、湍流、热物性参数暂不自动写入。
```

为了避免 manifest 臃肿，建议分两层表达：

```text
摘要参数
  放在 manifest 中，便于 APPFlow 快速展示。

完整参数
  保留在 OpenFOAM case 文件中，例如 controlDict、fvSchemes、fvSolution、0/U、0/p。
  APPFlow 后续如果需要精确同步，应解析这些原始文件或由 Agent 额外输出结构化摘要。
```

可以添加一个可选字段 `solver_settings`，但第一版不要强制要求：

```json
{
  "solver_settings": {
    "time": {
      "start_time": 0,
      "end_time": 0.5,
      "delta_t": 0.005,
      "write_interval": 20
    },
    "physics": {
      "nu": 0.01
    },
    "numerics": {
      "source_files": [
        "system/controlDict",
        "system/fvSchemes",
        "system/fvSolution",
        "constant/transportProperties"
      ]
    }
  }
}
```

建议映射关系：

| Agent / OpenFOAM 来源 | 后续 APPFlow 可同步内容 |
| --- | --- |
| `system/controlDict` | 起始时间、结束时间、时间步、写出间隔。 |
| `system/fvSchemes` | 离散格式设置。 |
| `system/fvSolution` | 求解器、残差控制、松弛因子。 |
| `constant/transportProperties` | 黏度、密度等物性参数。 |
| `0/U`、`0/p` | 初始场和边界条件。 |
| `case_summary.boundaries` | 边界名称和粗类型展示。 |

推荐实现顺序：

```text
第一步
  APPFlow 展示 solver_settings 摘要，并写入少量稳定字段。当前已按这个方式实现。

第二步
  扩展更多 APPFlow 内部求解器参数映射，例如湍流模型、离散格式和残差控制。

第三步
  建立边界条件映射：patch 名称 -> APPFlow mesh boundary ID -> BoundaryManager。

第四步
  如果 Agent 后续支持分步骤运行，再考虑 APPFlow 修改参数后回传给 Agent。
```

## 边界条件同步建议

边界条件建议单独使用可选字段 `boundary_conditions`，不要塞进 `solver_settings`。原因是边界条件不是单纯的数值参数，它依赖 OpenFOAM 网格中的 patch 名称，也依赖 APPFlow 导入网格后生成的真实 boundary ID。

推荐结构：

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
    },
    {
      "patch": "fixedWalls",
      "appflow_type": "Wall",
      "fields": {
        "U": {
          "type": "No-Slip"
        },
        "p": {
          "type": "Zero Gradient"
        }
      }
    },
    {
      "patch": "frontAndBack",
      "appflow_type": "Empty",
      "fields": {
        "U": {
          "type": "Empty"
        },
        "p": {
          "type": "Empty"
        }
      }
    }
  ]
}
```

字段说明：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `patch` | string | OpenFOAM patch 名称，必须与导入网格后的 boundary 名称一致。 |
| `appflow_type` | string | APPFlow 粗边界类型，例如 `Wall`、`Velocity Inlet`、`Pressure Outlet`。 |
| `fields` | object | 按变量名组织的边界条件，例如 `U`、`p`、`T`。 |
| `fields.<name>.type` | string | APPFlow 或 OpenFOAM 边界类型显示名，例如 `No-Slip`、`Fixed Value`、`Zero Gradient`。 |
| `fields.<name>.value` | number / array | 可选，固定值边界需要提供。 |

第一版建议支持：

```text
Wall
Velocity Inlet
Pressure Outlet
Pressure Inlet
```

第一版建议只读或跳过：

```text
empty / frontAndBack
mappedWall
coupled
复杂热耦合、多相、旋转机械专用边界
```

APPFlow 后续真正写入边界条件时，建议按下面顺序做：

```text
导入 OpenFOAM mesh
  -> 获取 APPFlow 内部 mesh boundary 列表
  -> 用 boundary name 匹配 manifest 中的 patch
  -> 找到 regionMeshID 和 meshBoundaryID
  -> 调用 APPFlow 现有 BoundaryManager / PhysicsHandler 写入
  -> 写入失败时只报 warning，不影响 mesh 和结果导入
```

## 不建议 Agent 现在做的事

```text
不要把完整 OpenFOAM 文件内容全部塞进 manifest。
不要把长日志全文塞进 manifest，只写日志路径。
不要在 APPFlow manifest 中混入 PHengLEI 专用字段。
不要让 solver.family 写错，例如 OpenFOAM case 写成 phenglei。
不要让 result_dir 指向不存在的目录却同时设置 export_vtk=false。
```

manifest 应该是索引和摘要，不是完整数据库。

## APPFlow 当前可验证结果

导入 manifest 后，APPFlow 当前应能输出类似信息：

```text
Agent manifest loaded: ...
Workflow: ...
Status: ...
Solver family: openfoam
Solver command: ...
Case dir: ...
Mesh dir: ...
Result dir: ...
Case dir exists: yes
Mesh dir exists: yes
Result files: *.vtk: ...
Solver settings:
Time start_time: 0
Time end_time: 0.5
Time delta_t: 0.005
Time write_interval: 20
Physics nu: 0.01
OpenFOAM mesh import requested from: ...
ParaView open requested for: ...
```

如果是 PHengLEI manifest，当前 APPFlow 应明确提示不支持，并且不进入 OpenFOAM 导入和后处理逻辑。

## Agent 端交付前检查

Agent 端把产物交给 APPFlow 前，至少检查：

```text
agent_manifest_v1.json 是合法 JSON
version = 1
workflow.id 非空
workflow.status 非空
solver.family = openfoam
artifacts.case_dir 存在
如果填写 artifacts.mesh_dir，则该目录存在
如果填写 artifacts.result_dir 且 export_vtk=false，则该目录中应有可用结果
如果 export_vtk=true，则 case_dir 中应有 OpenFOAM 时间步目录或可转换结果
logs 中列出的文件路径可读
Windows 路径使用 D:/work/case 这种正斜杠格式更稳妥
```

## 与 appPHengLEI 的关系

APPFlow 当前只处理 OpenFOAM：

```text
solver.family = openfoam
```

后续 `appPHengLEI` 应处理：

```text
solver.family = phenglei
```

两边可以复用：

```text
version / workflow / solver / artifacts / case_summary / appflow_hints
日志预览思路
最小 manifest 校验思路
```

两边不应该强行复用：

```text
OpenFoamAgentAdapter
OpenFOAM 网格导入逻辑
foamToVTK / ParaView case.foam 逻辑
PHengLEI 专用 case 解释逻辑
```

这样后续迁移到 `appPHengLEI` 时，可以复用协议和通用解析层，但保持求解器适配逻辑清晰分离。
