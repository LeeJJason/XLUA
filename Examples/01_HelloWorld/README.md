# 01_Helloworld

## 代码
```cs
LuaEnv luaenv = new LuaEnv();
luaenv.DoString("CS.UnityEngine.Debug.Log('hello world')");
luaenv.Dispose();
```
主要通过 **CS** 表的 **import_type** 功能导入类。