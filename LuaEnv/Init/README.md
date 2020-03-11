# Init 的加载
```lua
local metatable = {}
local rawget = rawget
local setmetatable = setmetatable
local import_type = xlua.import_type
local import_generic_type = xlua.import_generic_type
local load_assembly = xlua.load_assembly

function metatable:__index(key) 
    local fqn = rawget(self,'.fqn')
    fqn = ((fqn and fqn .. '.') or '') .. key

    local obj = import_type(fqn)

    if obj == nil then
        -- It might be an assembly, so we load it too.
        obj = { ['.fqn'] = fqn }
        setmetatable(obj, metatable)
    elseif obj == true then
        return rawget(self, key)
    end

    -- Cache this lookup
    rawset(self, key, obj)
    return obj
end

function metatable:__newindex()
    error('No such type: ' .. rawget(self,'.fqn'), 2)
end

-- A non-type has been called; e.g. foo = System.Foo()
function metatable:__call(...)
    local n = select('#', ...)
    local fqn = rawget(self,'.fqn')
    if n > 0 then
        local gt = import_generic_type(fqn, ...)
        if gt then
            return rawget(CS, gt)
        end
    end
    error('No such type: ' .. fqn, 2)
end

CS = CS or {}
setmetatable(CS, metatable)

typeof = function(t) return t.UnderlyingSystemType end
cast = xlua.cast
if not setfenv or not getfenv then
    local function getfunction(level)
        local info = debug.getinfo(level + 1, 'f')
        return info and info.func
    end

    function setfenv(fn, env)
        if type(fn) == 'number' then fn = getfunction(fn + 1) end
        local i = 1
        while true do
        local name = debug.getupvalue(fn, i)
        if name == '_ENV' then
            debug.upvaluejoin(fn, i, (function()
            return env
            end), 1)
            break
        elseif not name then
            break
        end

        i = i + 1
        end

        return fn
    end

    function getfenv(fn)
        if type(fn) == 'number' then fn = getfunction(fn + 1) end
        local i = 1
        while true do
        local name, val = debug.getupvalue(fn, i)
        if name == '_ENV' then
            return val
        elseif not name then
            break
        end
        i = i + 1
        end
    end
end

xlua.hotfix = function(cs, field, func)
    if func == nil then func = false end
    local tbl = (type(field) == 'table') and field or {[field] = func}
    for k, v in pairs(tbl) do
        local cflag = ''
        if k == '.ctor' then
            cflag = '_c'
            k = 'ctor'
        end
        local f = type(v) == 'function' and v or nil
        xlua.access(cs, cflag .. '__Hotfix0_'..k, f) -- at least one
        pcall(function()
            for i = 1, 99 do
                xlua.access(cs, cflag .. '__Hotfix'..i..'_'..k, f)
            end
        end)
    end
    xlua.private_accessible(cs)
end
xlua.getmetatable = function(cs)
    return xlua.metatable_operation(cs)
end
xlua.setmetatable = function(cs, mt)
    return xlua.metatable_operation(cs, mt)
end
xlua.setclass = function(parent, name, impl)
    impl.UnderlyingSystemType = parent[name].UnderlyingSystemType
    rawset(parent, name, impl)
end

local base_mt = {
    __index = function(t, k)
        local csobj = t['__csobj']
        local func = csobj['<>xLuaBaseProxy_'..k]
        return function(_, ...)
                return func(csobj, ...)
        end
    end
}
base = function(csobj)
    return setmetatable({__csobj = csobj}, base_mt)
end
```


## CS 表的初始化
```lua
function metatable:__index(key) 
    --获取 .fqn 的值，并于当前key拼接构成一个更具体的 class full name
    local fqn = rawget(self,'.fqn')
    fqn = ((fqn and fqn .. '.') or '') .. key

    local obj = import_type(fqn)
    --若没有成功导入类，则可能还不是具体类，重新构建一个 fqn 表来做查找
    --找到后则通过 rawget 返回该类型
    if obj == nil then
        -- It might be an assembly, so we load it too.
        obj = { ['.fqn'] = fqn }
        setmetatable(obj, metatable)
    elseif obj == true then
        return rawget(self, key)
    end

    -- Cache this lookup
    rawset(self, key, obj)
    return obj
end

function metatable:__newindex()
    error('No such type: ' .. rawget(self,'.fqn'), 2)
end

-- A non-type has been called; e.g. foo = System.Foo()
function metatable:__call(...)
    local n = select('#', ...)
    local fqn = rawget(self,'.fqn')
    if n > 0 then
        local gt = import_generic_type(fqn, ...)
        if gt then
            return rawget(CS, gt)
        end
    end
    error('No such type: ' .. fqn, 2)
end

CS = CS or {}
setmetatable(CS, metatable)
```

## import_type 的实现
在 **ObjectTranslator** 中通过 **FindType** 查找具体的 Type，再通过 **GetTypeId** 将类型传入 CS 表中。
```cs
internal int getTypeId(RealStatePtr L, Type type, out bool is_first, LOGLEVEL log_level = LOGLEVEL.WARN)
{
    int type_id;
    is_first = false;
    //如果没有映射过则执行注册
    if (!typeIdMap.TryGetValue(type, out type_id)) // no reference
    {
        if (type.IsArray)
        {
            if (common_array_meta == -1) throw new Exception("Fatal Exception! Array Metatable not inited!");
            return common_array_meta;
        }
        if (typeof(MulticastDelegate).IsAssignableFrom(type))
        {
            if (common_delegate_meta == -1) throw new Exception("Fatal Exception! Delegate Metatable not inited!");
            TryDelayWrapLoader(L, type);
            return common_delegate_meta;
        }

        is_first = true;
        Type alias_type = null;
        aliasCfg.TryGetValue(type, out alias_type);
        LuaAPI.luaL_getmetatable(L, alias_type == null ? type.FullName : alias_type.FullName);

        //判断是否在 register 表中注册过
        if (LuaAPI.lua_isnil(L, -1)) //no meta yet, try to use reflection meta
        {
            LuaAPI.lua_pop(L, 1);

            //实际注册过程
            if (TryDelayWrapLoader(L, alias_type == null ? type : alias_type))
            {
                //获取当前注册状态
                LuaAPI.luaL_getmetatable(L, alias_type == null ? type.FullName : alias_type.FullName);
            }
            else
            {
                throw new Exception("Fatal: can not load metatable of type:" + type);
            }
        }

        //循环依赖，自身依赖自己的class，比如有个自身类型的静态readonly对象。
        if (typeIdMap.TryGetValue(type, out type_id))
        {
            LuaAPI.lua_pop(L, 1);
        }
        else
        {
            if (type.IsEnum())
            {
                LuaAPI.xlua_pushasciistring(L, "__band");
                LuaAPI.lua_pushstdcallcfunction(L, metaFunctions.EnumAndMeta);
                LuaAPI.lua_rawset(L, -3);
                LuaAPI.xlua_pushasciistring(L, "__bor");
                LuaAPI.lua_pushstdcallcfunction(L, metaFunctions.EnumOrMeta);
                LuaAPI.lua_rawset(L, -3);
            }
            if (typeof(IEnumerable).IsAssignableFrom(type))
            {
                LuaAPI.xlua_pushasciistring(L, "__pairs");
                LuaAPI.lua_getref(L, enumerable_pairs_func);
                LuaAPI.lua_rawset(L, -3);
            }
            LuaAPI.lua_pushvalue(L, -1);
            type_id = LuaAPI.luaL_ref(L, LuaIndexes.LUA_REGISTRYINDEX);
            LuaAPI.lua_pushnumber(L, type_id);
            LuaAPI.xlua_rawseti(L, -2, 1);
            LuaAPI.lua_pop(L, 1);

            if (type.IsValueType())
            {
                typeMap.Add(type_id, type);
            }

            typeIdMap.Add(type, type_id);
        }
    }
    return type_id;
}
```

## TryDelayWrapLoader 的实现
```cs
public bool TryDelayWrapLoader(RealStatePtr L, Type type)
{
    if (loaded_types.ContainsKey(type)) return true;
    loaded_types.Add(type, true);

    LuaAPI.luaL_newmetatable(L, type.FullName); //先建一个metatable，因为加载过程可能会需要用到
    LuaAPI.lua_pop(L, 1);

    Action<RealStatePtr> loader;
    int top = LuaAPI.lua_gettop(L);
    if (delayWrap.TryGetValue(type, out loader))
    {
        delayWrap.Remove(type);
        loader(L);
    }
    else
    {
#if !GEN_CODE_MINIMIZE && !ENABLE_IL2CPP && (UNITY_EDITOR || XLUA_GENERAL) && !FORCE_REFLECTION && !NET_STANDARD_2_0
        if (!DelegateBridge.Gen_Flag && !type.IsEnum() && !typeof(Delegate).IsAssignableFrom(type) && Utils.IsPublic(type))
        {
            Type wrap = ce.EmitTypeWrap(type);
            MethodInfo method = wrap.GetMethod("__Register", BindingFlags.Static | BindingFlags.Public);
            method.Invoke(null, new object[] { L });
        }
        else
        {
            Utils.ReflectionWrap(L, type, privateAccessibleFlags.Contains(type));
        }
#else
        Utils.ReflectionWrap(L, type, privateAccessibleFlags.Contains(type));
#endif
#if NOT_GEN_WARNING
        if (!typeof(Delegate).IsAssignableFrom(type))
        {
#if !XLUA_GENERAL
            UnityEngine.Debug.LogWarning(string.Format("{0} not gen, using reflection instead", type));
#else
            System.Console.WriteLine(string.Format("Warning: {0} not gen, using reflection instead", type));
#endif
        }
#endif
    }
    if (top != LuaAPI.lua_gettop(L))
    {
        throw new Exception("top change, before:" + top + ", after:" + LuaAPI.lua_gettop(L));
    }

    foreach (var nested_type in type.GetNestedTypes(BindingFlags.Public))
    {
        if (nested_type.IsGenericTypeDefinition())
        {
            continue;
        }
        GetTypeId(L, nested_type);
    }
    
    return true;
}

```

## ReflectionWrap
```cs
public static void ReflectionWrap(RealStatePtr L, Type type, bool privateAccessible)
{
    //确保栈有足够空间
    LuaAPI.lua_checkstack(L, 20);

    //用于最后的 System.Diagnostics.Debug.Assert，判断是否恢复堆栈
    int top_enter = LuaAPI.lua_gettop(L);
    ObjectTranslator translator = ObjectTranslatorPool.Instance.Find(L);
    //create obj meta table
    LuaAPI.luaL_getmetatable(L, type.FullName);
    if (LuaAPI.lua_isnil(L, -1))
    {
        LuaAPI.lua_pop(L, 1);
        LuaAPI.luaL_newmetatable(L, type.FullName);
    }

    // 每个 lua_gettop 都指定为某个特定的table的栈位置
    LuaAPI.lua_pushlightuserdata(L, LuaAPI.xlua_tag());
    LuaAPI.lua_pushnumber(L, 1);
    LuaAPI.lua_rawset(L, -3);
    int obj_meta = LuaAPI.lua_gettop(L);

    LuaAPI.lua_newtable(L);
    int cls_meta = LuaAPI.lua_gettop(L);

    LuaAPI.lua_newtable(L);
    int obj_field = LuaAPI.lua_gettop(L);
    LuaAPI.lua_newtable(L);
    int obj_getter = LuaAPI.lua_gettop(L);
    LuaAPI.lua_newtable(L);
    int obj_setter = LuaAPI.lua_gettop(L);
    LuaAPI.lua_newtable(L);
    int cls_field = LuaAPI.lua_gettop(L);
    //set cls_field to namespace
    SetCSTable(L, type, cls_field);
    //finish set cls_field to namespace
    LuaAPI.lua_newtable(L);
    int cls_getter = LuaAPI.lua_gettop(L);
    LuaAPI.lua_newtable(L);
    int cls_setter = LuaAPI.lua_gettop(L);

    LuaCSFunction item_getter;
    LuaCSFunction item_setter;
    makeReflectionWrap(L, type, cls_field, cls_getter, cls_setter, obj_field, obj_getter, obj_setter, obj_meta,
        out item_getter, out item_setter, privateAccessible ? (BindingFlags.Public | BindingFlags.NonPublic) : BindingFlags.Public);

    // init obj metatable
    LuaAPI.xlua_pushasciistring(L, "__gc");
    LuaAPI.lua_pushstdcallcfunction(L, translator.metaFunctions.GcMeta);
    LuaAPI.lua_rawset(L, obj_meta);

    LuaAPI.xlua_pushasciistring(L, "__tostring");
    LuaAPI.lua_pushstdcallcfunction(L, translator.metaFunctions.ToStringMeta);
    LuaAPI.lua_rawset(L, obj_meta);

    LuaAPI.xlua_pushasciistring(L, "__index");
    LuaAPI.lua_pushvalue(L, obj_field);
    LuaAPI.lua_pushvalue(L, obj_getter);
    translator.PushFixCSFunction(L, item_getter);
    translator.PushAny(L, type.BaseType());
    LuaAPI.xlua_pushasciistring(L, LuaIndexsFieldName);
    LuaAPI.lua_rawget(L, LuaIndexes.LUA_REGISTRYINDEX);
    LuaAPI.lua_pushnil(L);
    LuaAPI.gen_obj_indexer(L);
    //store in lua indexs function tables
    LuaAPI.xlua_pushasciistring(L, LuaIndexsFieldName);
    LuaAPI.lua_rawget(L, LuaIndexes.LUA_REGISTRYINDEX);
    translator.Push(L, type);
    LuaAPI.lua_pushvalue(L, -3);
    LuaAPI.lua_rawset(L, -3);
    LuaAPI.lua_pop(L, 1);
    LuaAPI.lua_rawset(L, obj_meta); // set __index

    LuaAPI.xlua_pushasciistring(L, "__newindex");
    LuaAPI.lua_pushvalue(L, obj_setter);
    translator.PushFixCSFunction(L, item_setter);
    translator.Push(L, type.BaseType());
    LuaAPI.xlua_pushasciistring(L, LuaNewIndexsFieldName);
    LuaAPI.lua_rawget(L, LuaIndexes.LUA_REGISTRYINDEX);
    LuaAPI.lua_pushnil(L);
    LuaAPI.gen_obj_newindexer(L);
    //store in lua newindexs function tables
    LuaAPI.xlua_pushasciistring(L, LuaNewIndexsFieldName);
    LuaAPI.lua_rawget(L, LuaIndexes.LUA_REGISTRYINDEX);
    translator.Push(L, type);
    LuaAPI.lua_pushvalue(L, -3);
    LuaAPI.lua_rawset(L, -3);
    LuaAPI.lua_pop(L, 1);
    LuaAPI.lua_rawset(L, obj_meta); // set __newindex
                                    //finish init obj metatable

    LuaAPI.xlua_pushasciistring(L, "UnderlyingSystemType");
    translator.PushAny(L, type);
    LuaAPI.lua_rawset(L, cls_field);

    if (type != null && type.IsEnum())
    {
        LuaAPI.xlua_pushasciistring(L, "__CastFrom");
        translator.PushFixCSFunction(L, genEnumCastFrom(type));
        LuaAPI.lua_rawset(L, cls_field);
    }

    //init class meta
    LuaAPI.xlua_pushasciistring(L, "__index");
    LuaAPI.lua_pushvalue(L, cls_getter);
    LuaAPI.lua_pushvalue(L, cls_field);
    translator.Push(L, type.BaseType());
    LuaAPI.xlua_pushasciistring(L, LuaClassIndexsFieldName);
    LuaAPI.lua_rawget(L, LuaIndexes.LUA_REGISTRYINDEX);
    LuaAPI.gen_cls_indexer(L);
    //store in lua indexs function tables
    LuaAPI.xlua_pushasciistring(L, LuaClassIndexsFieldName);
    LuaAPI.lua_rawget(L, LuaIndexes.LUA_REGISTRYINDEX);
    translator.Push(L, type);
    LuaAPI.lua_pushvalue(L, -3);
    LuaAPI.lua_rawset(L, -3);
    LuaAPI.lua_pop(L, 1);
    LuaAPI.lua_rawset(L, cls_meta); // set __index 

    LuaAPI.xlua_pushasciistring(L, "__newindex");
    LuaAPI.lua_pushvalue(L, cls_setter);
    translator.Push(L, type.BaseType());
    LuaAPI.xlua_pushasciistring(L, LuaClassNewIndexsFieldName);
    LuaAPI.lua_rawget(L, LuaIndexes.LUA_REGISTRYINDEX);
    LuaAPI.gen_cls_newindexer(L);
    //store in lua newindexs function tables
    LuaAPI.xlua_pushasciistring(L, LuaClassNewIndexsFieldName);
    LuaAPI.lua_rawget(L, LuaIndexes.LUA_REGISTRYINDEX);
    translator.Push(L, type);
    LuaAPI.lua_pushvalue(L, -3);
    LuaAPI.lua_rawset(L, -3);
    LuaAPI.lua_pop(L, 1);
    LuaAPI.lua_rawset(L, cls_meta); // set __newindex

    LuaCSFunction constructor = typeof(Delegate).IsAssignableFrom(type) ? translator.metaFunctions.DelegateCtor : translator.methodWrapsCache.GetConstructorWrap(type);
    if (constructor == null)
    {
        constructor = (RealStatePtr LL) =>
        {
            return LuaAPI.luaL_error(LL, "No constructor for " + type);
        };
    }

    LuaAPI.xlua_pushasciistring(L, "__call");
    translator.PushFixCSFunction(L, constructor);
    LuaAPI.lua_rawset(L, cls_meta);

    LuaAPI.lua_pushvalue(L, cls_meta);
    LuaAPI.lua_setmetatable(L, cls_field);

    LuaAPI.lua_pop(L, 8);

    System.Diagnostics.Debug.Assert(top_enter == LuaAPI.lua_gettop(L));
}
```

## **SetCSTable** 关联 **LUA** 的 **CS** 表
```cs
public static void LoadCSTable(RealStatePtr L, Type type)
{
    int oldTop = LuaAPI.lua_gettop(L);
    //在 LuaEnv 初始化时会将 CS 注册到注册表中
    LuaAPI.xlua_pushasciistring(L, LuaEnv.CSHARP_NAMESPACE);
    LuaAPI.lua_rawget(L, LuaIndexes.LUA_REGISTRYINDEX);

    //获取类路径 .
    List<string> path = getPathOfType(type);

    for (int i = 0; i < path.Count; ++i)
    {
        LuaAPI.xlua_pushasciistring(L, path[i]);
        if (0 != LuaAPI.xlua_pgettable(L, -2))
        {
            LuaAPI.lua_settop(L, oldTop);
            LuaAPI.lua_pushnil(L);
            return;
        }
        if (!LuaAPI.lua_istable(L, -1) && i < path.Count - 1)
        {
            LuaAPI.lua_settop(L, oldTop);
            LuaAPI.lua_pushnil(L);
            return;
        }
        LuaAPI.lua_remove(L, -2);
    }
}
```