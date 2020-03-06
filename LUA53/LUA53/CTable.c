#include "../libLua53/stackDump/stackDump.h"

void CTable()
{
	const char* str = "me = { name = \"zilongshanren\", age = 27}";

	lua_State *L = luaL_newstate();
	luaL_dostring(L, str);
	stackDump(L);

	//��Lua����ȡ��me���table����ѹ��ջ
	lua_getglobal(L, "me");
	if (!lua_istable(L, -1)) {
		printf("error! me is not a table");
	}
	stackDump(L);

	//��ջ����ѹ��һ��key:name
	lua_pushstring(L, "name");
	stackDump(L);

	//ȡ��-2λ�õ�table��Ȼ���ջ��Ԫ�ص�����ȡ��table[name]��ֵ��ѹ��ջ
	lua_gettable(L, -2);
	stackDump(L);

	//���ջ����name
	printf("name = %s\n", lua_tostring(L, -1));
	
	//��ջ��Ԫ�ص���ȥ
	lua_pop(L, 1);
	stackDump(L);

	//ѹ����һ��key:age
	lua_pushstring(L, "age");
	stackDump(L);

	//ȡ��-2λ�õ�table,��table[age]��ֵѹ��ջ
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