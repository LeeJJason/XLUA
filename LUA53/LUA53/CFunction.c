#include "../libLua53/stackDump/stackDump.h"

void CFunction()
{
	const char* str = 
		"function Side(a, b, c, d)\n"
		"	return a, d\n"
		"end";

	lua_State *L = luaL_newstate();
	luaL_dostring(L, str);
	stackDump(L);
	
	int t = lua_getglobal(L, "Side");
	printf("Lua Type : %d\n", t);
	stackDump(L);

	lua_pushinteger(L, 2);
	lua_pushnil(L);
	lua_pushboolean(L, 1);
	lua_pushstring(L, "string value");
	stackDump(L);
	//函数、参数出栈，返回值入栈
	lua_call(L, 4, 2);
	stackDump(L);

	lua_close(L);
}