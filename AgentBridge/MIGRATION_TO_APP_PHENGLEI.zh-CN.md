# AgentBridge 迁移到 appPHengLEI 清单

这份清单用于后续把 APPFlow 中已经跑通的 AgentBridge 思路迁移到 `appPHengLEI`。当前建议不是整体复制 APPFlow 代码，而是按层复用。

## 迁移目标

目标是让 `appPHengLEI` 后续也能消费外部 Agent 输出的 manifest：

```text
外部 Agent 完成 PHengLEI 或其他求解器流程
  -> 写出 agent_manifest_v1.json
  -> appPHengLEI 读取 manifest
  -> appPHengLEI 定位 case / mesh / logs / results
  -> appPHengLEI 调用自己的导入、显示、后处理逻辑
```

## 应用边界

当前采用单应用单求解器边界：

```text
APPFlow
  只支持 OpenFOAM
  只处理 solver.family = openfoam
  遇到 solver.family = phenglei 时只提示不支持
  不在 APPFlow 内实现 PHengLEI adapter

appPHengLEI
  只支持 PHengLEI
  只处理 solver.family = phenglei
  不在 appPHengLEI 内实现 OpenFOAM adapter
```

两者共享的是：

```text
manifest 协议
AgentManifestData
AgentLogPreview
部分文档和 examples
```

两者不共享的是：

```text
UI 入口
消息输出绑定
求解器专属 adapter
网格导入和结果显示流程
```

这样可以避免把 APPFlow 做成过度复杂的多求解器程序，也避免把 appPHengLEI 变成 OpenFOAM/PHengLEI 混合程序。

## 优先复用的文件

这些文件基本不绑定 APPFlow UI，可以优先迁移：

```text
OperatorsModel/AgentManifestData.h
OperatorsModel/AgentManifestData.cpp
```

用途：

```text
读取 agent_manifest_v1.json
校验 v1 最小必需字段
暴露 workflow / solver / artifacts / case_summary / appflow_hints / logs
```

这些文件也建议复用，但需要确认 appPHengLEI 是否仍然使用 Qt：

```text
OperatorsModel/AgentLogPreview.h
OperatorsModel/AgentLogPreview.cpp
```

用途：

```text
检查日志路径是否存在
读取日志前 80 行或最多 8000 字符
把日志摘要追加到 QStringList messages
```

协议文档和示例也应复制过去：

```text
AgentBridge/README.md
AgentBridge/README.zh-CN.md
AgentBridge/TESTING.zh-CN.md
AgentBridge/examples/minimal_manifest_v1.json
AgentBridge/examples/agent_manifest_v1.json
AgentBridge/examples/test.json
```

## 需要重写的文件

这些文件绑定 APPFlow，不建议直接复用：

```text
OperatorsModel/OperatorsAgentManifest.h
OperatorsModel/OperatorsAgentManifest.cpp
```

原因：

```text
依赖 Core::FITKActionOperator
依赖 FITKOPERREPO
依赖 APPFlow 菜单 action
依赖 APPFlow 原有 OpenFOAM 网格导入 operator
```

在 `appPHengLEI` 中应改成自己的入口，例如：

```text
菜单按钮
工具栏按钮
命令行入口
项目导入向导
```

然后在入口内部调用：

```cpp
AgentManifestData data;
QString errorMessage;
loadAgentManifest(fileName, data, errorMessage);
```

## 需要按 appPHengLEI 改写的文件

这个文件目前是 APPFlow 消息绑定工具：

```text
OperatorsModel/AgentBridgeMessage.h
OperatorsModel/AgentBridgeMessage.cpp
```

当前能力：

```text
缺失字段显示为 [not provided]
把 QStringList messages 逐行输出到 APPFlow 的 FITKMessageNormal
```

其中 `agentValueOrNotProvided()` 可以复用；`outputAgentNormalMessages()` 需要根据 `appPHengLEI` 的消息系统改写。

如果 appPHengLEI 也使用同一套 FITK 消息系统，可以直接复用。否则需要替换成：

```text
appPHengLEI 的日志窗口输出
状态栏输出
控制台输出
QTextEdit append
```

## OpenFOAM 适配器的处理方式

当前文件：

```text
OperatorsModel/OpenFoamAgentAdapter.h
OperatorsModel/OpenFoamAgentAdapter.cpp
```

它主要服务 APPFlow + OpenFOAM 场景：

```text
检查 OpenFOAM result_dir
检测 OpenFOAM 时间步目录
执行 native / Docker foamToVTK
打开 ParaView
```

如果 appPHengLEI 仍然需要支持 OpenFOAM 结果，可以迁移这部分；如果 appPHengLEI 只面向 PHengLEI，则不需要直接复用，但可以参考它的结构。

建议后续新增：

```text
PHengLEIAgentAdapter.h
PHengLEIAgentAdapter.cpp
```

职责类似：

```text
读取 solver.family = phenglei
解释 PHengLEI case_dir / mesh_dir / result_dir
检查 PHengLEI 网格和结果文件是否存在
调用 appPHengLEI 自己的导入流程
必要时调用 PHengLEI 后处理或结果转换程序
把可显示结果交给 appPHengLEI UI
```

## 建议迁移后的结构

建议 `appPHengLEI` 中也保留类似分层：

```text
AgentBridge/
  AgentManifestData
  AgentLogPreview
  AgentBridgeMessage 或 appPHengLEIMessage
  PHengLEIAgentAdapter
  AppPHengLEIAgentImportOperator
```

调用关系：

```text
appPHengLEI UI 入口
  -> loadAgentManifest()
  -> 输出 workflow / solver / case 摘要
  -> PHengLEIAgentAdapter::processResults()
  -> appendAgentLogPreviewMessages()
  -> appPHengLEI 自己的网格或结果导入流程
  -> appPHengLEI 自己的可视化窗口
```

## manifest 字段复用建议

PHengLEI 不需要另起一套顶层 manifest。继续复用 v1 顶层结构：

```json
{
  "version": 1,
  "workflow": {},
  "solver": {
    "family": "phenglei"
  },
  "artifacts": {},
  "case_summary": {},
  "appflow_hints": {}
}
```

建议先保留最小必需字段：

```text
version
workflow.id
workflow.status
solver.family
artifacts.case_dir
```

PHengLEI 相关差异放到可选字段或后续扩展字段中，例如：

```text
case_summary.mesh_format
artifacts.mesh_dir
artifacts.result_dir
appflow_hints.open_app_phenglei
appflow_hints.app_phenglei_exe
```

具体字段不要现在过度设计，应等阅读 `appPHengLEI` 项目后再确定。

## 迁移前需要确认的问题

等 `appPHengLEI` 项目加入工作区后，先确认这些点：

```text
1. appPHengLEI 使用 qmake、CMake，还是其他构建系统？
2. appPHengLEI 是否使用 Qt？
3. appPHengLEI 是否使用 FITK 框架？
4. appPHengLEI 的主入口 main.cpp 在哪里？
5. appPHengLEI 是否已有菜单、工具栏或导入 action？
6. PHengLEI case 目录结构是什么？
7. PHengLEI 网格文件格式是什么？
8. PHengLEI 结果文件格式是什么？
9. appPHengLEI 当前如何显示网格和结果？
10. 是否支持命令行打开 case/result？
```

这些答案会决定：

```text
AgentManifestData 是否能直接复用
AgentLogPreview 是否能直接复用
AgentBridgeMessage 是否要改写
PHengLEIAgentAdapter 的职责边界
manifest 中是否需要新增 PHengLEI 专属 hints
```

## 推荐迁移顺序

建议不要一次性搬完整套代码。推荐顺序：

```text
1. 复制 AgentBridge 文档和 examples。
2. 迁移 AgentManifestData，并在 appPHengLEI 中跑通 minimal_manifest_v1.json。
3. 迁移 AgentLogPreview，验证日志显示。
4. 根据 appPHengLEI 消息系统实现 AgentBridgeMessage。
5. 新建 PHengLEIAgentAdapter，只做路径检查和消息输出。
6. 接入 appPHengLEI 自己的网格导入或结果显示流程。
7. 最后再考虑自动后处理、结果转换和分步骤 Agent 事件。
```

当前最重要的原则：

```text
先跑通最小 manifest
再接日志
再接 PHengLEI 结果
最后再做复杂自动化
```
