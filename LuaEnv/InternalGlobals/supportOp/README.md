# InternalGlobals.supportOp

```cs
internal static volatile Dictionary<string, string> supportOp = new Dictionary<string, string>()
    {
        { "op_Addition", "__add" },
        { "op_Subtraction", "__sub" },
        { "op_Multiply", "__mul" },
        { "op_Division", "__div" },
        { "op_Equality", "__eq" },
        { "op_UnaryNegation", "__unm" },
        { "op_LessThan", "__lt" },
        { "op_LessThanOrEqual", "__le" },
        { "op_Modulus", "__mod" },
        { "op_BitwiseAnd", "__band" },
        { "op_BitwiseOr", "__bor" },
        { "op_ExclusiveOr", "__bxor" },
        { "op_OnesComplement", "__bnot" },
        { "op_LeftShift", "__shl" },
        { "op_RightShift", "__shr" },
    };
```

用处
```cs
static void makeReflectionWrap(RealStatePtr L, Type type, int cls_field, int cls_getter, int cls_setter, int obj_field, int obj_getter, int obj_setter, int obj_meta, out LuaCSFunction item_getter, out LuaCSFunction item_setter, BindingFlags access)
{
    ObjectTranslator translator = ObjectTranslatorPool.Instance.Find(L);
    BindingFlags flag = BindingFlags.DeclaredOnly | BindingFlags.Instance | BindingFlags.Static | access;
    FieldInfo[] fields = type.GetFields(flag);
    EventInfo[] all_events = type.GetEvents(flag | BindingFlags.Public | BindingFlags.NonPublic);

    LuaAPI.lua_checkstack(L, 2);

    for (int i = 0; i < fields.Length; ++i)
    {
        FieldInfo field = fields[i];
        string fieldName = field.Name;
        // skip hotfix inject field
        if (field.IsStatic && (field.Name.StartsWith("__Hotfix") || field.Name.StartsWith("_c__Hotfix")) && typeof(Delegate).IsAssignableFrom(field.FieldType))
        {
            continue;
        }
        if (all_events.Any(e => e.Name == fieldName))
        {
            fieldName = "&" + fieldName;
        }

        if (field.IsStatic && (field.IsInitOnly || field.IsLiteral))
        {
            LuaAPI.xlua_pushasciistring(L, fieldName);
            translator.PushAny(L, field.GetValue(null));
            LuaAPI.lua_rawset(L, cls_field);
        }
        else
        {
            LuaAPI.xlua_pushasciistring(L, fieldName);
            translator.PushFixCSFunction(L, genFieldGetter(type, field));
            LuaAPI.lua_rawset(L, field.IsStatic ? cls_getter : obj_getter);

            LuaAPI.xlua_pushasciistring(L, fieldName);
            translator.PushFixCSFunction(L, genFieldSetter(type, field));
            LuaAPI.lua_rawset(L, field.IsStatic ? cls_setter : obj_setter);
        }
    }

    EventInfo[] events = type.GetEvents(flag);
    for (int i = 0; i < events.Length; ++i)
    {
        EventInfo eventInfo = events[i];
        LuaAPI.xlua_pushasciistring(L, eventInfo.Name);
        translator.PushFixCSFunction(L, translator.methodWrapsCache.GetEventWrap(type, eventInfo.Name));
        bool is_static = (eventInfo.GetAddMethod(true) != null) ? eventInfo.GetAddMethod(true).IsStatic : eventInfo.GetRemoveMethod(true).IsStatic;
        LuaAPI.lua_rawset(L, is_static ? cls_field : obj_field);
    }

    List<PropertyInfo> items = new List<PropertyInfo>();
    PropertyInfo[] props = type.GetProperties(flag);
    for (int i = 0; i < props.Length; ++i)
    {
        PropertyInfo prop = props[i];
        if (prop.GetIndexParameters().Length > 0)
        {
            items.Add(prop);
        }
    }

    var item_array = items.ToArray();
    item_getter = item_array.Length > 0 ? genItemGetter(type, item_array) : null;
    item_setter = item_array.Length > 0 ? genItemSetter(type, item_array) : null;
    MethodInfo[] methods = type.GetMethods(flag);
    if (access == BindingFlags.NonPublic)
    {
        methods = type.GetMethods(flag | BindingFlags.Public).Join(methods, p => p.Name, q => q.Name, (p, q) => p).ToArray();
    }
    Dictionary<MethodKey, List<MemberInfo>> pending_methods = new Dictionary<MethodKey, List<MemberInfo>>();
    for (int i = 0; i < methods.Length; ++i)
    {
        MethodInfo method = methods[i];
        string method_name = method.Name;

        MethodKey method_key = new MethodKey { Name = method_name, IsStatic = method.IsStatic };
        List<MemberInfo> overloads;
        if (pending_methods.TryGetValue(method_key, out overloads))
        {
            overloads.Add(method);
            continue;
        }

        //indexer
        if (method.IsSpecialName && ((method.Name == "get_Item" && method.GetParameters().Length == 1) || (method.Name == "set_Item" && method.GetParameters().Length == 2)))
        {
            if (!method.GetParameters()[0].ParameterType.IsAssignableFrom(typeof(string)))
            {
                continue;
            }
        }

        if ((method_name.StartsWith("add_") || method_name.StartsWith("remove_")) && method.IsSpecialName)
        {
            continue;
        }

        if (method_name.StartsWith("op_") && method.IsSpecialName) // 操作符
        {
            if (InternalGlobals.supportOp.ContainsKey(method_name))
            {
                if (overloads == null)
                {
                    overloads = new List<MemberInfo>();
                    pending_methods.Add(method_key, overloads);
                }
                overloads.Add(method);
            }
            continue;
        }
        else if (method_name.StartsWith("get_") && method.IsSpecialName && method.GetParameters().Length != 1) // getter of property
        {
            string prop_name = method.Name.Substring(4);
            LuaAPI.xlua_pushasciistring(L, prop_name);
            translator.PushFixCSFunction(L, translator.methodWrapsCache._GenMethodWrap(method.DeclaringType, prop_name, new MethodBase[] { method }).Call);
            LuaAPI.lua_rawset(L, method.IsStatic ? cls_getter : obj_getter);
        }
        else if (method_name.StartsWith("set_") && method.IsSpecialName && method.GetParameters().Length != 2) // setter of property
        {
            string prop_name = method.Name.Substring(4);
            LuaAPI.xlua_pushasciistring(L, prop_name);
            translator.PushFixCSFunction(L, translator.methodWrapsCache._GenMethodWrap(method.DeclaringType, prop_name, new MethodBase[] { method }).Call);
            LuaAPI.lua_rawset(L, method.IsStatic ? cls_setter : obj_setter);
        }
        else if (method_name == ".ctor" && method.IsConstructor)
        {
            continue;
        }
        else
        {
            if (overloads == null)
            {
                overloads = new List<MemberInfo>();
                pending_methods.Add(method_key, overloads);
            }
            overloads.Add(method);
        }
    }


    IEnumerable<MethodInfo> extend_methods = GetExtensionMethodsOf(type);
    if (extend_methods != null)
    {
        foreach (var extend_method in extend_methods)
        {
            MethodKey method_key = new MethodKey { Name = extend_method.Name, IsStatic = false };
            List<MemberInfo> overloads;
            if (pending_methods.TryGetValue(method_key, out overloads))
            {
                overloads.Add(extend_method);
                continue;
            }
            else
            {
                overloads = new List<MemberInfo>() { extend_method };
                pending_methods.Add(method_key, overloads);
            }
        }
    }

    foreach (var kv in pending_methods)
    {
        if (kv.Key.Name.StartsWith("op_")) // 操作符
        {
            LuaAPI.xlua_pushasciistring(L, InternalGlobals.supportOp[kv.Key.Name]);
            translator.PushFixCSFunction(L,
                new LuaCSFunction(translator.methodWrapsCache._GenMethodWrap(type, kv.Key.Name, kv.Value.ToArray()).Call));
            LuaAPI.lua_rawset(L, obj_meta);
        }
        else
        {
            LuaAPI.xlua_pushasciistring(L, kv.Key.Name);
            translator.PushFixCSFunction(L,
                new LuaCSFunction(translator.methodWrapsCache._GenMethodWrap(type, kv.Key.Name, kv.Value.ToArray()).Call));
            LuaAPI.lua_rawset(L, kv.Key.IsStatic ? cls_field : obj_field);
        }
    }
}
```