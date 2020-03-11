# LuaEnv 初始化过程

## 1、InternalGlobals
[InternalGlobals](InternalGlobals/README.md) 作为 XLUA 内部的全局缓存静态类。


## 2、ObjectTranslator
[ObjectTranslator](ObjectTranslator/README.md) 作为 C# 和 LUA 交互的桥梁。缓存C#中的一些信息作为LUA中的userdata。

## 3、DoString(init_xlua, "Init");
[Init](Init/README.md) 初始化Lua到C#的一些信息。

## 4、代码分析
```cs
public LuaEnv()
{
    //库版本检测
    if (LuaAPI.xlua_get_lib_version() != LIB_VERSION_EXPECT)
    {
        throw new InvalidProgramException("wrong lib version expect:"
            + LIB_VERSION_EXPECT + " but got:" + LuaAPI.xlua_get_lib_version());
    }

#if THREAD_SAFE || HOTFIX_ENABLE
    lock(luaEnvLock)
    {
#endif
        //初始化 LUA_REGISTRYINDEX
        LuaIndexes.LUA_REGISTRYINDEX = LuaAPI.xlua_get_registry_index();
#if GEN_CODE_MINIMIZE
        LuaAPI.xlua_set_csharp_wrapper_caller(InternalGlobals.CSharpWrapperCallerPtr);
#endif
        // Create State
        rawL = LuaAPI.luaL_newstate();

        //Init Base Libs
        LuaAPI.luaopen_xlua(rawL);
        LuaAPI.luaopen_i64lib(rawL);

        translator = new ObjectTranslator(this, rawL);
        translator.createFunctionMetatable(rawL);
        translator.OpenLib(rawL);
        ObjectTranslatorPool.Instance.Add(rawL, translator);

        LuaAPI.lua_atpanic(rawL, StaticLuaCallbacks.Panic);

#if !XLUA_GENERAL
        LuaAPI.lua_pushstdcallcfunction(rawL, StaticLuaCallbacks.Print);
        if (0 != LuaAPI.xlua_setglobal(rawL, "print"))
        {
            throw new Exception("call xlua_setglobal fail!");
        }
#endif

        //template engine lib register
        TemplateEngine.LuaTemplate.OpenLib(rawL);

        //LUA 的加载方式
        AddSearcher(StaticLuaCallbacks.LoadBuiltinLib, 2); // just after the preload searcher
        AddSearcher(StaticLuaCallbacks.LoadFromCustomLoaders, 3);
#if !XLUA_GENERAL
        AddSearcher(StaticLuaCallbacks.LoadFromResource, 4);
        AddSearcher(StaticLuaCallbacks.LoadFromStreamingAssetsPath, -1);
#endif
        //Init 初始化
        DoString(init_xlua, "Init");
        init_xlua = null;

#if !UNITY_SWITCH || UNITY_EDITOR
        AddBuildin("socket.core", StaticLuaCallbacks.LoadSocketCore);
        AddBuildin("socket", StaticLuaCallbacks.LoadSocketCore);
#endif

        AddBuildin("CS", StaticLuaCallbacks.LoadCS);

        //创建一个 table1 __index对应于 StaticLuaCallbacks.MetaFuncIndex
        LuaAPI.lua_newtable(rawL); //metatable of indexs and newindexs functions
        LuaAPI.xlua_pushasciistring(rawL, "__index");
        LuaAPI.lua_pushstdcallcfunction(rawL, StaticLuaCallbacks.MetaFuncIndex);
        LuaAPI.lua_rawset(rawL, -3);
        //1 table1

        //新建一个 table2，并将table1设置为他的 metatable
        LuaAPI.xlua_pushasciistring(rawL, Utils.LuaIndexsFieldName);
        LuaAPI.lua_newtable(rawL);
        LuaAPI.lua_pushvalue(rawL, -3);
        LuaAPI.lua_setmetatable(rawL, -2);
        LuaAPI.lua_rawset(rawL, LuaIndexes.LUA_REGISTRYINDEX);
        //结果 LUA_REGISTRYINDEX.LuaIndexsFieldName = table2
        //1 table1

        //新建一个 table3，并将table1设置为他的 metatable
        LuaAPI.xlua_pushasciistring(rawL, Utils.LuaNewIndexsFieldName);
        LuaAPI.lua_newtable(rawL);
        LuaAPI.lua_pushvalue(rawL, -3);
        LuaAPI.lua_setmetatable(rawL, -2);
        LuaAPI.lua_rawset(rawL, LuaIndexes.LUA_REGISTRYINDEX);
        //结果 LUA_REGISTRYINDEX.LuaNewIndexsFieldName = table3
        //1 table1

        //新建一个 table4，并将table1设置为他的 metatable
        LuaAPI.xlua_pushasciistring(rawL, Utils.LuaClassIndexsFieldName);
        LuaAPI.lua_newtable(rawL);
        LuaAPI.lua_pushvalue(rawL, -3);
        LuaAPI.lua_setmetatable(rawL, -2);
        LuaAPI.lua_rawset(rawL, LuaIndexes.LUA_REGISTRYINDEX);
        //结果 LUA_REGISTRYINDEX.LuaClassIndexsFieldName = table4
        //1 table1

        //新建一个 table5，并将table1设置为他的 metatable
        LuaAPI.xlua_pushasciistring(rawL, Utils.LuaClassNewIndexsFieldName);
        LuaAPI.lua_newtable(rawL);
        LuaAPI.lua_pushvalue(rawL, -3);
        LuaAPI.lua_setmetatable(rawL, -2);
        LuaAPI.lua_rawset(rawL, LuaIndexes.LUA_REGISTRYINDEX);
        //结果 LUA_REGISTRYINDEX.LuaClassNewIndexsFieldName = table5
        //1 table1

        LuaAPI.lua_pop(rawL, 1); // pop metatable of indexs and newindexs functions
        //0

        LuaAPI.xlua_pushasciistring(rawL, MAIN_SHREAD);
        LuaAPI.lua_pushthread(rawL);
        LuaAPI.lua_rawset(rawL, LuaIndexes.LUA_REGISTRYINDEX);
        //设置 LUA_REGISTRYINDEX.MAIN_SHREAD = rawL

        LuaAPI.xlua_pushasciistring(rawL, CSHARP_NAMESPACE);
        if (0 != LuaAPI.xlua_getglobal(rawL, "CS"))
        {
            throw new Exception("get CS fail!");
        }
        LuaAPI.lua_rawset(rawL, LuaIndexes.LUA_REGISTRYINDEX);
        //设置 LUA_REGISTRYINDEX.CSHARP_NAMESPACE = CS

#if !XLUA_GENERAL && (!UNITY_WSA || UNITY_EDITOR)
        translator.Alias(typeof(Type), "System.MonoType");
#endif

        if (0 != LuaAPI.xlua_getglobal(rawL, "_G"))
        {
            throw new Exception("get _G fail!");
        }
        translator.Get(rawL, -1, out _G);
        LuaAPI.lua_pop(rawL, 1);

        errorFuncRef = LuaAPI.get_error_func_ref(rawL);

        if (initers != null)
        {
            for (int i = 0; i < initers.Count; i++)
            {
                initers[i](this, translator);
            }
        }

        translator.CreateArrayMetatable(rawL);
        translator.CreateDelegateMetatable(rawL);
        translator.CreateEnumerablePairs(rawL);
#if THREAD_SAFE || HOTFIX_ENABLE
    }
#endif
}
```

## 扩展
### 宏定义说明
|宏定义|说明|
|:----|:---|
|THREAD_SAFE|标记 LuaEnv 为线程安全状态，用于多线程运行。|
|HOTFIX_ENABLE|标记 LuaEnv 开启热更状态，具有 **THREAD_SAFE** 相同作用。|
|[NATIVE_LUA_PUSHSTRING](InternalGlobals/strBuff/README.md)|标记 XLUA 不使用 wrap 过的 pushstring 相关的函数。|
|[SINGLE_ENV](InternalGlobals/ObjectTranslatorPool/README.md)|标记当前应用只有一个 LuaEnv。|
|USE_UNI_LUA|使用 UniLua|