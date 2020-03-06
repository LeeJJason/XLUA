#include "stackDump/stackDump.h"


void CStack()
{
	lua_State *L = luaL_newstate();
	//1 依次压入几个变量 true，10，nil，"hello"   
	printf("1:\n");
	lua_pushboolean(L, 1);
	lua_pushnumber(L, 10);
	lua_pushnil(L);
	lua_pushstring(L, "hello");
	stackDump(L);//true 10 nil hello   
	//2  
	printf("2:\n");
	lua_pushvalue(L, -4);
	stackDump(L);//true 10 nil hello true   
	//3  用栈顶更换第三个  
	printf("3:\n");
	lua_replace(L, 3);
	stackDump(L);//true 10 true hello   
	//4 设置栈变量为6个  
	printf("4:\n");
	lua_settop(L, 6);
	stackDump(L);//true 10 true hello nil nil   
	//5 移除从栈顶往下数的第三个  
	printf("5:\n");
	lua_remove(L, -3);
	stackDump(L);//true 10 true nil nil   
	//6 设置-5位置的变量作为栈顶变量  
	printf("6:\n");
	lua_settop(L, -5);
	stackDump(L);//true  
	//7  
	lua_close(L);
}