# ObjectTranslatorPool

源码
```cs
public class ObjectTranslatorPool
{
#if !SINGLE_ENV
    private Dictionary<RealStatePtr, WeakReference> translators = new Dictionary<RealStatePtr, WeakReference>();
    RealStatePtr lastPtr = default(RealStatePtr);
#endif
    ObjectTranslator lastTranslator = default(ObjectTranslator);

    public static ObjectTranslatorPool Instance
    {
        get
        {
            return InternalGlobals.objectTranslatorPool;
        }
    }

#if UNITY_EDITOR || XLUA_GENERAL
    public static ObjectTranslator FindTranslator(RealStatePtr L)
    {
        return InternalGlobals.objectTranslatorPool.Find(L);
    }
#endif

    public ObjectTranslatorPool ()
    {
    }
    
    public void Add (RealStatePtr L, ObjectTranslator translator)
    {
#if THREAD_SAFE || HOTFIX_ENABLE
        lock (this)
#endif
        {
            lastTranslator = translator;
#if !SINGLE_ENV
            var ptr = LuaAPI.xlua_gl(L);
            lastPtr = ptr;
            translators.Add(ptr , new WeakReference(translator));
#endif
        }
    }

    public ObjectTranslator Find (RealStatePtr L)
    {
#if THREAD_SAFE || HOTFIX_ENABLE
        lock (this)
#endif
        {
#if SINGLE_ENV
            return lastTranslator;
#else
            var ptr = LuaAPI.xlua_gl(L);
            if (lastPtr == ptr) return lastTranslator;
            if (translators.ContainsKey(ptr))
            {
                lastPtr = ptr;
                lastTranslator = translators[ptr].Target as ObjectTranslator;
                return lastTranslator;
            }
            
            return null;
#endif
        }
    }
    
    public void Remove (RealStatePtr L)
    {
#if THREAD_SAFE || HOTFIX_ENABLE
        lock (this)
#endif
        {
#if SINGLE_ENV
            lastTranslator = default(ObjectTranslator);
#else
            var ptr = LuaAPI.xlua_gl(L);
            if (!translators.ContainsKey (ptr))
                return;
            
            if (lastPtr == ptr)
            {
                lastPtr = default(RealStatePtr);
                lastTranslator = default(ObjectTranslator);
            }

            translators.Remove(ptr);
#endif
        }
    }
}
```

主要使用 `SINGLE_ENV` 来优化是否为单个 LuaEnv，否者使用 `translators` 通过 `RealStatePtr` 作为 key 做映射。