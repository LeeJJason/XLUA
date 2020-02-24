# LuaEnv 初始化过程

## 1、InternalGlobals
[InternalGlobals](InternalGlobals/README.md) 作为 XLUA 内部的全局缓存静态类。


## 2、ObjectTranslator
[ObjectTranslator](ObjectTranslator/README.md) 作为 C# 和 LUA 交互的桥梁。缓存C#中的一些信息作为LUA中的userdata。

## 2、DoString(init_xlua, "Init");


## 扩展
### 宏定义说明
|宏定义|说明|
|:----|:---|
|THREAD_SAFE|标记 LuaEnv 为线程安全状态，用于多线程运行。|
|HOTFIX_ENABLE|标记 LuaEnv 开启热更状态，具有 **THREAD_SAFE** 相同作用。|
|[NATIVE_LUA_PUSHSTRING](InternalGlobals/strBuff/README.md)|标记 XLUA 不使用 wrap 过的 pushstring 相关的函数。|
|[SINGLE_ENV](InternalGlobals/ObjectTranslatorPool/README.md)|标记当前应用只有一个 LuaEnv。|
|USE_UNI_LUA|使用 UniLua|