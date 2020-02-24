# ObjectTranslator

## 构造函数
```cs
public ObjectTranslator(LuaEnv luaenv,RealStatePtr L)
{
#if XLUA_GENERAL  || (UNITY_WSA && !UNITY_EDITOR)
    var dumb_field = typeof(ObjectTranslator).GetField("s_gen_reg_dumb_obj", BindingFlags.Static| BindingFlags.DeclaredOnly | BindingFlags.NonPublic);
    if (dumb_field != null)
    {
        dumb_field.GetValue(null);
    }
#endif
    assemblies = new List<Assembly>();

#if (UNITY_WSA && !ENABLE_IL2CPP) && !UNITY_EDITOR
    var assemblies_usorted = Utils.GetAssemblies();
#else
    assemblies.Add(Assembly.GetExecutingAssembly());
    var assemblies_usorted = AppDomain.CurrentDomain.GetAssemblies();
#endif
    addAssemblieByName(assemblies_usorted, "mscorlib,");
    addAssemblieByName(assemblies_usorted, "System,");
    addAssemblieByName(assemblies_usorted, "System.Core,");
    foreach (Assembly assembly in assemblies_usorted)
    {
        if (!assemblies.Contains(assembly))
        {
            assemblies.Add(assembly);
        }
    }

    this.luaEnv=luaenv;
    objectCasters = new ObjectCasters(this);
    objectCheckers = new ObjectCheckers(this);
    methodWrapsCache = new MethodWrapsCache(this, objectCheckers, objectCasters);
    metaFunctions=new StaticLuaCallbacks();

    importTypeFunction = new LuaCSFunction(StaticLuaCallbacks.ImportType);
    loadAssemblyFunction = new LuaCSFunction(StaticLuaCallbacks.LoadAssembly);
    castFunction = new LuaCSFunction(StaticLuaCallbacks.Cast);

    LuaAPI.lua_newtable(L);
    LuaAPI.lua_newtable(L);
    LuaAPI.xlua_pushasciistring(L, "__mode");
    LuaAPI.xlua_pushasciistring(L, "v");
    LuaAPI.lua_rawset(L, -3);
    LuaAPI.lua_setmetatable(L, -2);
    cacheRef = LuaAPI.luaL_ref(L, LuaIndexes.LUA_REGISTRYINDEX);

    initCSharpCallLua();
}
```

## 初始化程序集信息
1. 程序集缓存列表  
`internal List<Assembly> assemblies;`