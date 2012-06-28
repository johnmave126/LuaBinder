#include "rect.h"
#include "exrect.h"

int main (int argc, char* argv[])	{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	LuaClass::Init(L);
	LuaReg<Rect>::Register(L);
	LuaReg<exRect>::Register(L);
	luaL_dofile(L, "sample.lua");
	lua_close(L);
	return 0;
}