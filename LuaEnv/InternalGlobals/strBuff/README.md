# InternalGlobals.strBuff

代码
```cs
#if NATIVE_LUA_PUSHSTRING
    [DllImport(LUADLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_pushstring(IntPtr L, string str);
#else
    public static void lua_pushstring(IntPtr L, string str) //业务使用
    {
        if (str == null)
        {
            lua_pushnil(L);
        }
        else
        {
#if !THREAD_SAFE && !HOTFIX_ENABLE
            if (Encoding.UTF8.GetByteCount(str) > InternalGlobals.strBuff.Length)
            {
                byte[] bytes = Encoding.UTF8.GetBytes(str);
                xlua_pushlstring(L, bytes, bytes.Length);
            }
            else
            {
                int bytes_len = Encoding.UTF8.GetBytes(str, 0, str.Length, InternalGlobals.strBuff, 0);
                xlua_pushlstring(L, InternalGlobals.strBuff, bytes_len);
            }
#else
            var bytes = Encoding.UTF8.GetBytes(str);
            xlua_pushlstring(L, bytes, bytes.Length);
#endif
        }
    }
#endif
```

```cs
[DllImport(LUADLL, CallingConvention = CallingConvention.Cdecl)]
public static extern void xlua_pushlstring(IntPtr L, byte[] str, int size);
public static void xlua_pushasciistring(IntPtr L, string str) // for inner use only
{
    if (str == null)
    {
        lua_pushnil(L);
    }
    else
    {
#if NATIVE_LUA_PUSHSTRING
        lua_pushstring(L, str);
#else
#if !THREAD_SAFE && !HOTFIX_ENABLE
        int str_len = str.Length;
        if (InternalGlobals.strBuff.Length < str_len)
        {
            InternalGlobals.strBuff = new byte[str_len];
        }

        int bytes_len = Encoding.UTF8.GetBytes(str, 0, str_len, InternalGlobals.strBuff, 0);
        xlua_pushlstring(L, InternalGlobals.strBuff, bytes_len);
#else
        var bytes = Encoding.UTF8.GetBytes(str);
        xlua_pushlstring(L, bytes, bytes.Length);
#endif
#endif
    }
}
```

* `xlua_pushasciistring` 是对 `lua_pushstring` 的尽一步优化。只用于 ascii 字符串。
* `NATIVE_LUA_PUSHSTRING` 宏用于标记，直接使用 LUA 的 pushstring 接口，效率更高。否则所有的字符串都会转换为 bytes 后使用 `xlua_pushlstring` 传入 LUA 中，减少GC。