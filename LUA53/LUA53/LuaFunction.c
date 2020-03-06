#include "../libLua53/stackDump/stackDump.h"

int LuaCall(lua_State* L)
{
	stackDump(L);
	int n = lua_gettop(L);    /* 参数的个数 */
	lua_Number sum = 0.0;
	int i;
	for (i = 1; i <= n; i++) {
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "incorrect argument");
			lua_error(L);
		}
		sum += lua_tonumber(L, i);
	}
	lua_pushnumber(L, sum / n);        /* 第一个返回值 */
	lua_pushnumber(L, sum);         /* 第二个返回值 */
	stackDump(L);
	return 2;                   /* 返回值的个数 */
}

void LuaFunction()
{
	const char* str =
		"function luaFunction(count)\n"
		"	local n, t = LuaCall(count, 1, 3, 5, 7, 9)\n"
		"	print(n .. \" => \" .. t)\n"
		"end\n"
		"luaFunction(3)\n"
		"print(\"luaFunction\")";
		

	lua_State *L = luaL_newstate();
	lua_register(L, "LuaCall", LuaCall);
	luaL_dostring(L, str);
	lua_close(L);
}