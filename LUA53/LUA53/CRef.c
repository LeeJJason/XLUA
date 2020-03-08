#include "stackDump/stackDump.h"

void CRef()
{
	const char * str =
		"function foo()\n"
		"	print(\"hello world\")"
		"end";
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	luaL_dostring(L, str);


	lua_getglobal(L, "foo");
	stackDump(L);//1:  -1 > function

	// 存放函数到注册表中并返回引用
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	stackDump(L);//0

	// 从注册表中读取该函数并调用
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	stackDump(L);//1:  -1 > function

	//printf("stack size:%d,%d\n",lua_gettop(L),lua_type(L,-1));
	lua_pcall(L, 0, 0, 0);
	stackDump(L);//0
	printf("------------------------华丽的分割线------------\n");

	lua_getglobal(L, "foo");
	stackDump(L);//1:  -1 > function

	lua_setfield(L, LUA_REGISTRYINDEX, "sb");
	stackDump(L);//0


	lua_getfield(L, LUA_REGISTRYINDEX, "sb");
	lua_pcall(L, 0, 0, 0);
	stackDump(L);//0
	printf("------------------------又一次华丽的分割线------------\n");
	

	luaL_unref(L, LUA_REGISTRYINDEX, ref);
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	stackDump(L);//1:  -1 > nil

	lua_getfield(L, LUA_REGISTRYINDEX, "sb");
	lua_pcall(L, 0, 0, 0);
	lua_close(L);
}