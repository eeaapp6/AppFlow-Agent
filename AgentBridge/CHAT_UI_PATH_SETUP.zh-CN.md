# Agent UI 路径配置说明

本文档说明 `chat_ui` 项目和 APPFlow 联动时，`Host Output`、`Backend Output`、Docker volume 和 `agent_manifest_v1.json` 之间的路径关系。

它解决的问题是：Agent 后端在 Docker 或其他后端环境中运行，而 APPFlow 在 Windows 上读取结果。两边看到的是同一批文件，但路径字符串不同。

## 一、核心原则

APPFlow 只读取 manifest 中的宿主机路径，也就是 Windows 能直接访问的路径。

因此，真实 Agent 写出的 `agent_manifest_v1.json` 中应该出现：

```text
D:/work/FoamAgentOutputs/task_xxx/case
D:/work/FoamAgentOutputs/task_xxx/case/constant/polyMesh
D:/work/FoamAgentOutputs/task_xxx/case/VTK
D:/work/FoamAgentOutputs/task_xxx/logs/icoFoam.log
```

不应该让 APPFlow 读取 Docker 内部路径，例如：

```text
/workspace/outputs/task_xxx/case
```

Docker 内部路径只能给 Python 后端和 OpenFOAM 使用。

## 二、推荐目录

建议在 Windows 上固定一个输出根目录，例如：

```text
D:/work/FoamAgentOutputs
```

每次 Agent 运行后生成：

```text
D:/work/FoamAgentOutputs/
  task_xxx/
    task_context.json
    agent_manifest_v1.json
    case/
      0/
      constant/
      system/
      0.005/
      0.01/
      ...
    logs/
      blockMesh.log
      icoFoam.log
```

APPFlow 导入时选择：

```text
D:/work/FoamAgentOutputs/task_xxx/agent_manifest_v1.json
```

## 三、Agent UI 中怎么填

在 `chat_ui` 界面中有两个路径：

| UI 字段 | 含义 | Docker 后端推荐值 |
| --- | --- | --- |
| `Host Output` | Windows / APPFlow 看到的输出目录 | `D:/work/FoamAgentOutputs` |
| `Backend Output` | Python 后端 / Docker 容器看到的输出目录 | `/workspace/outputs` |

两者必须指向同一个挂载目录，只是路径写法不同。

正确配置示例：

```text
Host Output:
D:/work/FoamAgentOutputs

Backend Output:
/workspace/outputs
```

## 四、Docker volume 怎么映射

启动 Docker 后端时，应把 Windows 输出目录挂载到容器的 Backend Output：

```powershell
docker run --rm -it `
  -v "D:/work/FoamAgentOutputs:/workspace/outputs" `
  -p 8765:8765 `
  <foamagent-image>
```

这样：

```text
Windows:
D:/work/FoamAgentOutputs/task_xxx

Docker:
/workspace/outputs/task_xxx
```

指向的是同一批文件。

## 五、当前代码链路

`chat_ui` 提交任务时会把两个路径发给 Python 后端：

```text
output_dir       -> Backend Output
host_output_dir  -> Host Output
```

后端创建任务时：

```text
task.task_dir         = Backend Output / task_xxx
task.host_output_root = Host Output
```

manifest 写出时，`artifacts` 中的路径优先使用 `host_output_root`，因此 APPFlow 看到的是 Windows 路径。

```text
artifacts.case_dir
  = Host Output / task_xxx / case

artifacts.mesh_dir
  = Host Output / task_xxx / case / constant / polyMesh

artifacts.result_dir
  = Host Output / task_xxx / case / VTK

artifacts.logs[*].path
  = Host Output / task_xxx / logs / xxx.log
```

## 六、容易出错的配置

### 1. Backend Output 跟随 Host Output

如果 `Backend Output` 没有单独设置，它可能会跟随 `Host Output`。

这种配置：

```text
Host Output:
D:/work/FoamAgentOutputs

Backend Output:
D:/work/FoamAgentOutputs
```

只适合本机 Python 后端。

如果后端在 Docker 中，这样会导致容器内找不到目录或路径不符合预期。Docker 后端应使用：

```text
Backend Output:
/workspace/outputs
```

### 2. manifest 写成容器路径

如果 APPFlow 日志出现：

```text
Case dir: /workspace/outputs/task_xxx/case
Case dir exists: no
```

说明 manifest 写成了 Docker 内部路径。应检查 Agent UI 是否传入了 `host_output_dir`。

### 3. manifest 写成旧机器路径

如果 APPFlow 日志出现：

```text
Case dir: C:/Users/xxx/Documents/FoamAgentOutputs/task_xxx/case
Case dir exists: no
```

但文件实际在：

```text
D:/work/FoamAgentOutputs/task_xxx/case
```

说明这份结果是从别的位置复制来的，manifest 里的绝对路径没有同步更新。

推荐做法不是让 APPFlow 猜路径，而是让 Agent UI 一开始就把 `Host Output` 配到 APPFlow 可读的位置。

## 七、APPFlow 验收日志

路径配置正确后，APPFlow 导入 manifest 时应看到：

```text
Agent manifest loaded: D:/work/FoamAgentOutputs/task_xxx/agent_manifest_v1.json
Case dir: D:/work/FoamAgentOutputs/task_xxx/case
Mesh dir: D:/work/FoamAgentOutputs/task_xxx/case/constant/polyMesh
Case dir exists: yes
Mesh dir exists: yes
```

如果 Agent 没有提前生成 VTK，但已经有 OpenFOAM 时间步结果，应看到：

```text
OpenFOAM time directories detected: ...
VTK export not detected. foamToVTK may be required.
foamToVTK export requested from: D:/work/FoamAgentOutputs/task_xxx/case
```

## 八、当前决策

当前不优先在 APPFlow 中做路径自动修正。

原因是 `chat_ui` 已经提供了 `Host Output` 和 `Backend Output` 两个配置项，只要配置正确，manifest 就能直接写出 APPFlow 可读路径。

后续如果真实使用中经常出现复制任务目录、换机器导入、路径失效等情况，再考虑在 APPFlow AgentBridge 中增加基于 manifest 所在目录的路径回退逻辑。
