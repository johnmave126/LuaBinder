#ifndef rect_h
#define rect_h

#include "lua.hpp"
#include "luareg.h"

class Rect	{
	private:
		int x, y, width, height;
	public:
		static const char className[];
		static const char super[];
		static LuaReg<Rect>::RegType methods[];
		static LuaReg<Rect>::RegType attribs_r[];
		static LuaReg<Rect>::RegType attribs_a[];
		
		Rect();
		Rect(lua_State *L);
		
		~Rect();
		
		int gx(lua_State *L);
		int gy(lua_State *L);
		int gwidth(lua_State *L);
		int gheight(lua_State *L);
		
		int sx(lua_State *L);
		int sy(lua_State *L);
		int swidth(lua_State *L);
		int sheight(lua_State *L);
		
		int set(lua_State *L);
		int empty(lua_State *L);
		
};
#endif