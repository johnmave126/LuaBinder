#ifndef Rect_h
#define Rect_h

#include "lua.hpp"
#include "luareg.h"
#include "rect.h"

class exRect:Rect	{
	private:
		int z;
	public:
		static const char className[];
		static const char super[];
		static LuaReg<exRect>::RegType methods[];
		static LuaReg<exRect>::RegType attribs_r[];
		static LuaReg<exRect>::RegType attribs_a[];
		
		exRect();
		exRect(lua_State *L);
		
		~exRect();
		
		int gz(lua_State *L);
		
		int sz(lua_State *L);
		
		int set(lua_State *L);
		int empty(lua_State *L);
		int xyz(lua_State *L);
		int S(lua_State *L);
};
#endif