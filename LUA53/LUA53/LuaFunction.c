#include "../libLua53/stackDump/stackDump.h"

int LuaCall(lua_State* L)
{
	stackDump(L);
	int n = lua_gettop(L);    /* �����ĸ��� */
	lua_Number sum = 0.0;
	int i;
	for (i = 1; i <= n; i++) {
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "incorrect argument");
			lua_error(L);
		}
		sum += lua_tonumber(L, i);
	}
	lua_pushnumber(L, sum / n);        /* ��һ������ֵ */
	lua_pushnumber(L, sum);         /* �ڶ�������ֵ */
	stackDump(L);
	return 2;                   /* ����ֵ�ĸ��� */
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