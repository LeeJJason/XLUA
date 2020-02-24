## XLUA 学习笔记
`
运行环境：  xlua, Unity 2019.2.6f1
`

### 1、目录结构
* **DOC**：文档说明，包含API、配置、教程。
* **Examples**：例子存放目录。
* **Src**：XLUA在UNITY中使用的源码。
* **Tutorial**：部分教程。

### 2、LuaEnv 启动
[LuaEnv](LuaEnv/README.md) 实例化后，Lua 环境开始运行，直到执行 Dispose。LuaEnv 在实例化时，执行了一系列初始化操作，构建整个环境。

### 3、例子详见
* [01_Helloworld](01_Helloworld/README.md)