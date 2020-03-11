# 03_UIEvent

## 代码
```cs
scriptEnv = luaEnv.NewTable();

// 为每个脚本设置一个独立的环境，可一定程度上防止脚本间全局变量、函数冲突
LuaTable meta = luaEnv.NewTable();
meta.Set("__index", luaEnv.Global);
scriptEnv.SetMetaTable(meta);
meta.Dispose();

//设置对象
scriptEnv.Set("self", this);
foreach (var injection in injections)
{
    scriptEnv.Set(injection.name, injection.value);
}

//为 luaScript 设置 Env，用于脚本运行设置
luaEnv.DoString(luaScript.text, "LuaTestScript", scriptEnv);

//C# 获取 Lua 函数的两种方式
Action luaAwake = scriptEnv.Get<Action>("awake");
scriptEnv.Get("start", out luaStart);
scriptEnv.Get("update", out luaUpdate);
scriptEnv.Get("ondestroy", out luaOnDestroy);

if (luaAwake != null)
{
    luaAwake();
}
```
```lua
local speed = 10
local lightCpnt = nil

function awake()
	print("lua awake...")
end

function start()
	print("lua start...")
	print("injected object", lightObject)
	lightCpnt= lightObject:GetComponent(typeof(CS.UnityEngine.Light))
end

function update()
	local r = CS.UnityEngine.Vector3.up * CS.UnityEngine.Time.deltaTime * speed
	self.transform:Rotate(r)
	lightCpnt.color = CS.UnityEngine.Color(CS.UnityEngine.Mathf.Sin(CS.UnityEngine.Time.time) / 2 + 0.5, 0, 0, 1)
end

function ondestroy()
    print("lua destroy")
end


```