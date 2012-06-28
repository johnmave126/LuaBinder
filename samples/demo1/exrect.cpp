#include "exrect.h"

exRect::exRect()	{
	x = 0; y = 0; width = 0; height = 0; z = 0;
}

exRect::exRect(lua_State *L)	{
	int i;
	lua_settop(L, 5);
	for(i = 1; i <= 5; i++)	{
		if(!lua_isnumber(L, i))	{
			lua_typerror(L, i, "number");
			return;
		}
	}
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	width = luaL_checkinteger(L, 3);
	height = luaL_checkinteger(L, 4);
	z = luaL_checkinteger(L, 5);
}

exRect::~exRect()	{}

int exRect::gz(lua_State *L)	{
	lua_pushnumber(L, z);
	return 1;
}

int exRect::sz(lua_State *L)	{
	z = luaL_checkinteger(L, 1);
	return 0;
}

int exRect::set(lua_State *L)	{
	int i;
	lua_settop(L, 5);
	for(i = 1; i <= 5; i++)	{
		if(!lua_isnumber(L, i))	{
			lua_typerror(L, i, "number");
			return 0;
		}
	}
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	width = luaL_checkinteger(L, 3);
	height = luaL_checkinteger(L, 4);
	z = luaL_checkinteger(L, 5);
	return 0;
}

int exRect::empty(lua_State *L)	{
	x = 0; y = 0; width = 0; height = 0; z = 0;
	return 0;
}

int exRect::S(lua_State *L)	{
	lua_pushnumber(L, width * height);
	return 1;
}

int exRect::xyz(lua_State *L)	{
	lua_pushnumber(L, x * y * z);
	return 1;
}

const char exRect::className[] = "exRect";
const char exRect::super[] = "Rect";
LuaReg<exRect>::RegType exRect::methods[] = {
	{"set", &exRect::set},
	{"empty", &exRect::empty},
	{0, 0}
};

LuaReg<exRect>::RegType exRect::attribs_r[] = {
	{"z", &exRect::gz},
	{"S", &exRect::S},
	{"xyz", &exRect::xyz},
	{0, 0}
};

LuaReg<exRect>::RegType exRect::attribs_a[] = {
	{"z", &exRect::sz},
	{0, 0}
};