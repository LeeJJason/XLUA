# InternalGlobals

源码
```cs
internal partial class InternalGlobals
{
#if !THREAD_SAFE && !HOTFIX_ENABLE
    internal static byte[] strBuff = new byte[256];
#endif

    internal delegate bool TryArrayGet(Type type, RealStatePtr L, ObjectTranslator translator, object obj, int index);
    internal delegate bool TryArraySet(Type type, RealStatePtr L, ObjectTranslator translator, object obj, int array_idx, int obj_idx);
    internal static volatile TryArrayGet genTryArrayGetPtr = null;
    internal static volatile TryArraySet genTryArraySetPtr = null;

    internal static volatile ObjectTranslatorPool objectTranslatorPool = new ObjectTranslatorPool();

    internal static volatile int LUA_REGISTRYINDEX = -10000;

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

    internal static volatile Dictionary<Type, IEnumerable<MethodInfo>> extensionMethodMap = null;

#if GEN_CODE_MINIMIZE
    internal static volatile LuaDLL.CSharpWrapperCaller CSharpWrapperCallerPtr = new LuaDLL.CSharpWrapperCaller(StaticLuaCallbacks.CSharpWrapperCallerImpl);
#endif

    internal static volatile LuaCSFunction LazyReflectionWrap = new LuaCSFunction(Utils.LazyReflectionCall);
}
```

### 1、strBuff 使用分析
* [strBuff](strBuff/README.md) 用于在线程安全时，作为 C# 字符串到 LUA的 byte buffer。
* [objectTranslatorPool](ObjectTranslatorPool/README.md) 作为 ObjectTranslator 对应于每个lua 虚拟机的缓存池。