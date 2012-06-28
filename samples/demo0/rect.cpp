#include "rect.h"

Rect::Rect()	{
	x = 0; y = 0; width = 0; height = 0;
}

Rect::Rect(lua_State *L)	{
	int i;
	lua_settop(L, 4);
	for(i = 1; i <= 4; i++)	{
		if(!lua_isnumber(L, i))	{
			lua_typerror(L, i, "number");
			return;
		}
	}
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	width = luaL_checkinteger(L, 3);
	height = luaL_checkinteger(L, 4);
}

Rect::~Rect()	{}

int Rect::gx(lua_State *L)	{
	lua_pushnumber(L, x);
	return 1;
}

int Rect::gy(lua_State *L)	{
	lua_pushnumber(L, y);
	return 1;
}

int Rect::gwidth(lua_State *L)	{
	lua_pushnumber(L, width);
	return 1;
}

int Rect::gheight(lua_State *L)	{
	lua_pushnumber(L, height);
	return 1;
}

int Rect::sx(lua_State *L)	{
	x = luaL_checkinteger(L, 1);
	return 0;
}

int Rect::sy(lua_State *L)	{
	y = luaL_checkinteger(L, 1);
	return 0;
}

int Rect::swidth(lua_State *L)	{
	width = luaL_checkinteger(L, 1);
	return 0;
}

int Rect::sheight(lua_State *L)	{
	height = luaL_checkinteger(L, 1);
	return 0;
}

int Rect::set(lua_State *L)	{
	int i;
	lua_settop(L, 4);
	for(i = 1; i <= 4; i++)	{
		if(!lua_isnumber(L, i))	{
			lua_typerror(L, i, "number");
			return 0;
		}
	}
	x = luaL_checkinteger(L, 1);
	y = luaL_checkinteger(L, 2);
	width = luaL_checkinteger(L, 3);
	height = luaL_checkinteger(L, 4);
	return 0;
}

int Rect::empty(lua_State *L)	{
	x = 0; y = 0; width = 0; height = 0;
	return 0;
}

const char Rect::className[] = "Rect";
const char Rect::super[] = "";
LuaReg<Rect>::RegType Rect::methods[] = {
	{"set", &Rect::set},
	{"empty", &Rect::empty},
	{0, 0}
};

LuaReg<Rect>::RegType Rect::attribs_r[] = {
	{"x", &Rect::gx},
	{"y", &Rect::gy},
	{"width", &Rect::gwidth},
	{"height", &Rect::gheight},
	{0, 0}
};

LuaReg<Rect>::RegType Rect::attribs_a[] = {
	{"x", &Rect::sx},
	{"y", &Rect::sy},
	{"width", &Rect::swidth},
	{"height", &Rect::sheight},
	{0, 0}
};