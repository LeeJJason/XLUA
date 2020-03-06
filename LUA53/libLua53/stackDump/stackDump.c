#pragma once

#include "stackDump.h"

int stackDump(lua_State *L)
{
	printf("StackDump >>> \n");
	int i;
	int top = lua_gettop(L);
	for (i = 1; i <= top; i++)
	{
		printf("\t%4d:%4d > ", i, i - top - 1);
		int t = lua_type(L, i);
		switch (t)
		{
		case LUA_TSTRING:
			printf("%s", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			printf(lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
			printf("%g", lua_tonumber(L, i));
			break;
		default:
			printf("%s", lua_typename(L, t));
			break;
		}
		printf("\n");
	}
	return 0;
}