# 01_Helloworld

## 代码
```cs
LuaEnv luaenv = new LuaEnv();
luaenv.DoString("CS.UnityEngine.Debug.Log('hello world')");
luaenv.Dispose();
```