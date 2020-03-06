#include "../libLua53/stackDump/stackDump.h"

void CTable()
{
	const char* str = "me = { name = \"zilongshanren\", age = 27}";

	lua_State *L = luaL_newstate();
	luaL_dostring(L, str);
	stackDump(L);

	//从Lua里面取得me这个table，并压入栈
	lua_getglobal(L, "me");
	if (!lua_istable(L, -1)) {
		printf("error! me is not a table");
	}
	stackDump(L);

	//往栈里面压入一个key:name
	lua_pushstring(L, "name");
	stackDump(L);

	//取得-2位置的table，然后把栈顶元素弹出，取出table[name]的值并压入栈
	lua_gettable(L, -2);
	stackDump(L);

	//输出栈顶的name
	printf("name = %s\n", lua_tostring(L, -1));
	
	//把栈顶元素弹出去
	lua_pop(L, 1);
	stackDump(L);

	//压入另一个key:age
	lua_pushstring(L, "age");
	stackDump(L);

	//取出-2位置的table,把table[age]的值压入栈
	lua_gettable(L, -2);
	stackDump(L);
	printf("age = %d\n", lua_tointeger(L, -1));

	lua_pushfstring(L, "name");
	lua_pushfstring(L, "settable");
	stackDump(L);

	lua_settable(L, -4);
	stackDump(L);


	lua_getfield(L, -2, "name");
	stackDump(L);

	lua_pushinteger(L, 99);
	stackDump(L);
	lua_setfield(L, -4, "age");
	stackDump(L);
	lua_getfield(L, -3, "age");
	stackDump(L);

	lua_close(L);
}