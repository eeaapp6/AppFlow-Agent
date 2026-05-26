# AgentBridge 测试说明

这份文档记录第一阶段 AgentBridge 的手动验证方式。当前目标是验证端到端 Agent 产物能被 APPFlow 消费：

```text
Agent manifest -> APPFlow 导入网格 -> 检查结果 -> 导出 VTK -> 打开 ParaView
```

## 测试前准备

确认 APPFlow 是从已经继承环境变量的 VS2017 或终端启动的。至少需要：

```text
paraview.exe
docker.exe
```

如果使用 Docker 后端，还需要 Docker Desktop 已启动，并且 `test.json` 里的镜像名存在：

```json
"docker_image": "foamagent"
```

如果你的镜像不是 `foamagent`，把它改成 `docker images` 里显示的真实镜像名。

## 场景一：已有 VTK 结果

测试文件：

```text
D:/study/appflow/APPFlow_5936/AgentBridge/examples/agent_manifest_v1.json
```

前置条件：

```text
D:/work/demo_case_001/case/VTK
```

目录存在，并且里面有 `.vtk` 文件。

成功日志应包含：

```text
Agent manifest loaded: ...
Result files:
  *.vtk: ...
VTK result is ready: ...
OpenFOAM mesh import requested from: ...
ParaView open mode: openfoam_case
ParaView OpenFOAM case file: ...
ParaView open requested for OpenFOAM case: ...
```

这个场景验证的是：

```text
Agent 已经完成 foamToVTK -> APPFlow 按原生方式创建 case.foam -> ParaView 打开 OpenFOAM case
```

## 场景二：没有 VTK，使用 Docker 导出

测试文件：

```text
D:/study/appflow/APPFlow_5936/AgentBridge/examples/test.json
```

前置条件：

```text
D:/work/demo_case_002/case
```

目录下存在 OpenFOAM 时间步目录，例如：

```text
0.1
0.2
0.3
0.4
0.5
```

如果 `case/VTK` 已经存在，可以临时改名，确保这次测试会触发导出：

```powershell
Rename-Item D:\work\demo_case_002\case\VTK VTK_bak
```

`test.json` 的关键字段应为：

```json
"export_vtk": true,
"foam_to_vtk_backend": "docker",
"docker_image": "foamagent",
"docker_case_dir": "/case"
```

成功日志应包含：

```text
OpenFOAM time directories detected: ...
foamToVTK docker export requested from: D:/work/demo_case_002/case
foamToVTK backend: docker
Docker executable: ...
Docker image: foamagent
foamToVTK export finished.
VTK result dir after export: D:/work/demo_case_002/case/VTK
ParaView open mode: first_vtk
ParaView open requested for: ...
```

这个场景验证的是：

```text
Agent 没有生成 VTK -> APPFlow 通过 Docker 执行 foamToVTK -> 再打开 ParaView
```

## 常见问题

如果出现：

```text
ParaView open failed.
Tried ParaView executable: paraview.exe
```

说明 APPFlow 进程没有找到 ParaView。重新打开 VS2017 和 APPFlow，确保它们继承了新的 `PATH`。

如果出现：

```text
foamToVTK export failed. Check whether the selected backend is available in PATH.
```

说明 APPFlow 没找到当前后端需要的程序。`native` 后端需要 `foamToVTK`，`docker` 后端需要 `docker`。

如果 Docker 后端失败并出现：

```text
failed to connect to the docker API
```

说明 Docker Desktop 没有启动，或者当前用户无法访问 Docker daemon。

如果 Docker 后端提示找不到镜像，检查：

```powershell
docker images
```

然后把 `test.json` 里的 `docker_image` 改成真实镜像名。

如果导出成功但 APPFlow 没找到 VTK，检查 Docker 是否真的把 Windows case 目录挂载到了容器内：

```text
D:/work/demo_case_002/case:/case
```

导出完成后，Windows 下应出现：

```text
D:/work/demo_case_002/case/VTK
```

## 场景三：AgentChat 分步骤联动验收

这个场景验证 APPFlow 内嵌 AgentChat 与后端 Agent 服务的当前闭环：

```text
输入需求
  -> /foam/plan
  -> APPFlow 显示 next_action
  -> 用户输入“继续”
  -> APPFlow 自动调用 generate / validate / run / import_manifest
  -> 状态条更新到“已完成”
```

### 后端启动

如果使用 Docker 后端，先启动服务。示例结构如下，具体 API key 不应写入文档或提交：

```powershell
docker run -it --rm `
  --name simagent-openfoam `
  -p 8765:8765 `
  -e SIMAGENT_HOST=0.0.0.0 `
  -e SIMAGENT_RUN_PIPELINE_MODE=allrun_safe `
  -e ACTIVE_PROVIDER=deepseek `
  -e PROVIDER_API_KEY="你的 API Key" `
  -e PROVIDER_MODEL=deepseek-v4-flash `
  -e PROVIDER_BASE_URL=https://api.deepseek.com `
  -v "D:\study\appflow\APPFlow_5936\AgentBridge\chat_ui_backend:/workspace/chat_ui" `
  -v "D:\work\FoamAgentOutputs:/workspace/outputs" `
  leoyue123/foamagent:latest `
  bash -lc "cd /workspace/chat_ui && source /opt/openfoam10/etc/bashrc && python scripts/agent_service.py"
```

APPFlow AgentChat 中路径建议配置为：

```text
本机输出：D:/work/FoamAgentOutputs
后端输出：/workspace/outputs
```

这两个路径必须通过 Docker volume 指向同一批文件，否则 APPFlow 无法把后端返回的 manifest 路径映射成本机路径。

### 验收步骤

1. 打开 APPFlow。
2. 打开 Agent 聊天面板。
3. 确认后端状态显示可用，或者聊天区没有后端连接错误。
4. 输入一个 OpenFOAM 需求，例如：

```text
生成一个二维方腔流动 cavity 算例，使用 icoFoam，delta_t=0.005，end_time=0.5，nu=0.01。
```

5. 期望状态条变化：

```text
阶段：规划中
阶段：已规划
下一步：生成算例
```

6. 期望聊天区出现只读求解参数摘要：

```text
求解参数
求解器：icoFoam
时间：end_time=0.5, delta_t=0.005
物理：nu=0.01
```

如果用户需求没有明确给出某些参数，APPFlow 不应显示空字段。

7. 输入：

```text
继续
```

期望 APPFlow 自动调用 `/foam/generate`，并显示：

```text
阶段：生成中
阶段：已生成
下一步：校验算例
```

8. 再输入：

```text
继续
```

期望 APPFlow 自动调用 `/foam/validate`，并显示：

```text
阶段：校验中
阶段：已校验
下一步：运行 OpenFOAM
```

9. 再输入：

```text
继续
```

期望 APPFlow 自动调用 `/foam/run`，后端完成 OpenFOAM 运行，并返回 manifest：

```text
阶段：运行中
阶段：已运行
下一步：导入到 APPFlow
清单：已收到
```

10. 再输入：

```text
继续
```

期望 APPFlow 执行本地 `actionLoadAgentManifest`，导入 OpenFOAM 网格和结果：

```text
阶段：导入中
阶段：已完成
下一步：无
清单：已收到
```

11. 检查 APPFlow 消息窗口，应能看到类似：

```text
Agent manifest loaded: ...
OpenFOAM mesh import requested from: ...
read succeed ! ...
```

12. 在已有任务上下文中输入参数修改请求，例如：

```text
把 end_time 改成 1.0
```

当前期望行为：

```text
APPFlow 显示“检测到参数修改请求”
APPFlow 尝试调用后端 /foam/replan
后端返回 Parameter change request received
状态条重新显示：已规划
下一步重新显示：生成算例
不修改 APPFlow 内部求解器参数
不修改 OpenFOAM 字典
```

这是当前阶段的边界验证。`/foam/replan` 当前是最小占位实现，只打通 APPFlow 到后端的通道；它还不会真实解析参数或改写 OpenFOAM case。

### 需要重点观察的 UI 行为

| 位置 | 期望行为 |
| --- | --- |
| 状态条 | 显示任务 ID、阶段、下一步、清单状态 |
| 聊天区 | 显示 Agent 回复、工作流信息、只读求解参数、建议下一步 |
| Agent 回复 | 普通 Agent 回复应逐字显示，工作流/错误/工具消息仍直接完整显示；该项已验证成功 |
| 按钮栏 | 随阶段启用生成、校验、运行、导入按钮 |
| 输入框 | 输入“继续”时执行当前 next_action，而不是重新规划任务 |
| 参数修改请求 | 当前识别后尝试调用 `/foam/replan`，但不在 APPFlow 本地改参数或字典 |

### AgentChat 常见问题

如果输入“继续”后又开始重新规划，通常说明当前没有缓存到 `next_action.id`。检查后端响应里是否包含：

```json
"next_action": {
  "id": "generate"
}
```

如果状态条一直停在“已规划”，检查后端是否在 `/foam/generate` 后返回：

```json
"task": {
  "status": "generated"
}
```

如果运行成功但不能导入，检查：

```text
task.manifest_path 是否存在
output_dir 与 host_output_dir 是否能正确映射
agent_manifest_v1.json 是否在宿主机真实存在
```

如果后端返回了 manifest，但 APPFlow 显示“清单：未收到”，检查后端响应中的字段位置是否是：

```json
{
  "task": {
    "manifest_path": "..."
  }
}
```

## 阶段回归测试顺序

当前阶段建议每次改动后按下面顺序回归，尤其是 Agent 端更新 `/foam/replan` 或 case 生成逻辑之后。

### A. APPFlow 主链路

```text
1. 启动后端
2. 打开 APPFlow AgentChat
3. 输入 OpenFOAM 需求
4. 确认 /foam/plan 成功
5. 确认聊天区显示求解参数摘要
6. 确认 Agent 普通回复逐字显示
7. 连续输入“继续”
8. 依次完成 generate / validate / run / import_manifest
9. 确认状态条最终为“已完成”
```

这个测试保证现有端到端联动没有被破坏。

其中第 6 项已经完成手动验证。后续改动如果影响 `MessageList`、`AgentTypingRenderer`、`FoamAgentChatPanel` 或后端回复格式，需要重新检查逐字显示。

如果后续改动 `AgentChatIntent`，需要额外回归：

```text
输入“继续”仍然执行当前 next_action
输入“把 end_time 改成 1.0”仍然进入 /foam/replan
状态条仍然能正确显示 task id 和下一步
```

### B. 参数修改通道

```text
1. 在已有任务上下文中输入：把 end_time 改成 1.0
2. 确认 APPFlow 调用 /foam/replan
3. 当前占位后端应返回 Parameter change request received
4. 确认状态条回到“已规划”
5. 确认下一步是“生成算例”
6. 确认 APPFlow 没有本地修改 OpenFOAM 字典
```

这个测试保证方案 A 的通道是通的。

### C. Agent 端真实 replan 完成后的新增验收

当 Agent 端实现真实 `/foam/replan` 后，再额外检查：

```text
1. 输入：把 end_time 改成 1.0
2. APPFlow 聊天区应显示新的求解参数摘要
3. task_context.json 中 plan.requested_changes.end_time 应变成 1.0
4. 输入：继续
5. /foam/generate 应按新 plan 重写 case/system/controlDict
6. controlDict 中 endTime 应变成 1.0
7. validate / run 应仍然通过
```

这个测试只在 Agent 端完成真实参数解析和 case 重生成后执行。当前占位后端不要求通过 C。
