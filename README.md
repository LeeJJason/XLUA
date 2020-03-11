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
* [01_Helloworld](Examples/01_Helloworld/README.md): 快速入门的例子。
* [02_U3DScripting](Examples/02_U3DScripting/README.md): 展示怎么用lua来写MonoBehaviour。
* [03_UIEvent](Examples/03_UIEvent/README.md): 展示怎么用lua来写UI逻辑。
* [04_LuaObjectOrented](Examples/04_LuaObjectOrented/README.md): 展示lua面向对象和C#的配合。
* [05_NoGc](Examples/05_NoGc/README.md): 展示怎么去避免值类型的GC。
* [06_Coroutine](Examples/06_Coroutine/README.md): 展示lua协程怎么和Unity协程相配合。
* [07_AsyncTest](Examples/07_AsyncTest/README.md): 展示怎么用lua协程来把异步逻辑同步化。
* [08_Hotfix](Examples/08_Hotfix/README.md): 热补丁的示例（需要开启热补丁特性，如何开启请看指南）。
* [09_GenericMethod](Examples/09_GenericMethod/README.md): 泛化函数支持的演示。
* [10_SignatureLoader](Examples/10_SignatureLoader/README.md): 展示如何读取经数字签名的lua脚本，参见数字签名的文档介绍。
* [11_RawObject](Examples/11_RawObject/README.md): 当C#参数是object时，如何把一个lua number指定以boxing后的int传递过去。
* [12_ReImplementInLua](Examples/12_ReImplementInLua/README.md): 展示如何将复杂值类型改为lua实现。


### 3、文档
[常见问题解答](docs/faq.md)：常见问题都总结在这里，初使用大多数问题都可以在这里找到答案。
[(必看)XLua教程](docs/XLua教程.md)：教程，其配套代码这里。
[(必看)XLua的配置](docs/configure.md)：介绍如何配置xLua。
[热补丁操作指南](docs/hotfix.md)：介绍如何使用热补丁特性。
[XLua增加删除第三方lua库](docs/XLua增加删除第三方lua库.md)：如何增删第三方lua扩展库。
[XLua API](docs/XLua_API.md)：API文档。
[生成引擎二次开发指南](docs/custom_generate.md)：介绍如何做生成引擎的二次开发。

