/*
 * Copyright (c) 2012 Jefferson Tan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "luareg.h"

int lua_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}

int lua_setenv (lua_State *L, int index) {
  const char *upv;
  if(!lua_isfunction(L, index)) {
    lua_typerror(L, index, "function");
    return 0;
  }
  if(!( lua_istable(L, -1) || ( lua_isfunction(L, -1) && !lua_iscfunction(L, -1)))) {
    lua_typerror(L, -1, "table or Lua closure");
    return 0;
  }
  if( !(upv = lua_getupvalue(L, index, 1)) ) {
    return 0;
  }
  lua_pop(L, 1);
  if(strcmp("_ENV", upv) != 0) {
    return 0;
  }
  if(lua_istable(L, -1))	{
    luaL_dostring(L, "local _ENV; return function() return x; end;");
	lua_pushvalue(L, -2);
	lua_setupvalue(L, -2, 1);
	lua_remove(L, -2);
  }
  lua_upvaluejoin(L, index, 1, -1, 1);
  return 1;
}

void LuaClass::Init(lua_State *L)	{
	char buff[32];
	int i;
	Lua_Class_Begin = false;

	srand((unsigned)time(NULL));

	for(i = 0; i < 7; i++)	{
		buff[i] = rand() % 52;
		if(buff[i] < 26)
			buff[i] += 'A';
		else
			buff[i] += 'a' - 26;
	}
	buff[i] = '\0';
	lua_pushstring(L, buff);
	sRand = luaL_checkstring(L, -1);
	lua_setglobal(L, "luaregRandString");
	
	lua_pushcfunction(L, Class_Begin);
	lua_setglobal(L, "Class_Begin");
	

	lua_pushcfunction(L, Class_Attr_a);
	lua_setglobal(L, "Class_Attr_a");


	lua_pushcfunction(L, Class_Attr_r);
	lua_setglobal(L, "Class_Attr_r");
	
	lua_pushcfunction(L, Class_End);
	lua_setglobal(L, "Class_End");
}

int LuaClass::Class_Begin(lua_State *L)	{
	char buff[32];
	Lua_super = "";
	
	if(Lua_Class_Begin)	{
		luaL_error(L, "Class_Begin and Class_End mismatch");
		return 0;
	}
	Lua_Class_Begin = true;
	
	lua_settop(L, 2);
	
	if(!lua_isnil(L, -1))
		Lua_super = luaL_checkstring(L, -1);
	if(lua_isnil(L, -2))	{
		lua_typerror(L, 1, "string");
		return 0;
	}
	lua_getglobal(L, Lua_super);
	if(!lua_istable(L, -1))	{
		luaL_error(L, "Class \"%s\" cannot be found", Lua_super);
		return 0;
	}
	
	Lua_processing = luaL_checkstring(L, -2);
	lua_pop(L, 2);
	
	lua_newtable(L);
	int methods = lua_gettop(L);
	
	luaL_newmetatable(L, Lua_processing);
	int metatable = lua_gettop(L);
	
	lua_pushvalue(L, methods);
	lua_setglobal(L, Lua_processing);
	
	lua_newtable(L);
	int tmeta = lua_gettop(L);
	lua_pushliteral(L, "__newindex");
	lua_pushcfunction(L, Class_newindex);
	lua_settable(L, tmeta);

	lua_setmetatable(L, methods);
	
	lua_pushliteral(L, "__key");
	strcpy(buff, sRand);
	strcpy(buff + 7, Lua_processing);
	lua_pushstring(L, buff);
	lua_key = lua_tostring(L, -1);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__name");
	lua_pushstring(L, Lua_processing);
	lua_settable(L, metatable);
	
	lua_pushliteral(L, "__super");
	lua_pushstring(L, Lua_super);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__index");
	lua_pushstring(L, Lua_processing);
	lua_pushcclosure(L, Object_index, 1);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__gc");
	lua_pushstring(L, Lua_processing);
	lua_pushcclosure(L, Object_gc, 1);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__newindex");
	lua_pushstring(L, Lua_processing);
	lua_pushcclosure(L, Object_newindex, 1);
	lua_settable(L, metatable);
	
	lua_pushlightuserdata(L, (void*)lua_key);
	lua_newtable(L);
	lua_settable(L, LUA_REGISTRYINDEX);
	
	lua_pop(L, 2);
	return 0;
}

int LuaClass::Class_Attr_a(lua_State *L)	{
	int args, cnt = 0;
	const char *attr;
	char buff[32];

	lua_settop(L, 1);
	if(!lua_istable(L, 1))	{
		lua_typerror(L, 1, "table");
		return 0;
	}
	lua_getglobal(L, Lua_processing);

	lua_pushnil(L);
	while(lua_next(L, 1) != 0) {
		cnt++;
		if(!lua_istable(L, -1))	{
			lua_pop(L, 1);
			luaL_error(L, "table wanted at #%d argument of Class_Attr_a", cnt);
			continue;
		}
		lua_len(L, -1);
		args = lua_tointeger(L, -1);
		lua_pop(L, 1);
		if(args == 0)	{
			lua_pop(L, 1);
			continue;
		}
		else if(args == 1)	{//default accessor
			lua_pushinteger(L, 1);
			lua_gettable(L, -2);
			if(!lua_isstring(L, -1))	{
				lua_settop(L, 3);
				luaL_error(L, "string wanted at #%d argument of Class_Attr_a", cnt);
				continue;
			}
			attr = luaL_checkstring(L, -1);
			buff[0] = 's';
			strcpy(buff + 1, sRand);
			strcpy(buff + 8, attr);
			lua_pushstring(L, buff);
			lua_insert(L, -2);
			lua_pushcclosure(L, default_accessor, 1);
			lua_rawset(L, 2);
			lua_settop(L, 3);
		}
		else	{//custom accessor
			lua_pushinteger(L, 1);
			lua_gettable(L, -2);
			if(!lua_isstring(L, -1))	{
				lua_settop(L, 3);
				luaL_error(L, "string wanted at #1 of #%d argument of Class_Attr_a", cnt);
				continue;
			}
			attr = luaL_checkstring(L, -1);
			buff[0] = 's';
			strcpy(buff + 1, sRand);
			strcpy(buff + 8, attr);
			lua_pushstring(L, buff);
			lua_pushinteger(L, 2);
			lua_gettable(L, -4);
			if(!lua_isstring(L, -1))	{
				lua_settop(L, 3);
				luaL_error(L, "string wanted at #2 of #%d argument of Class_Attr_a", cnt);
				continue;
			}
			lua_rawget(L, 2);
			if(lua_isnil(L, -1) || !lua_isfunction(L, -1))	{
				lua_settop(L, 3);
				luaL_error(L, "cannot find function at #2 of #%d argument of Class_Attr_a", cnt);
				continue;
			}
			lua_rawset(L, 2);
		}
	}
	return 0;
}

int LuaClass::Class_Attr_r(lua_State *L)	{
	int args, cnt = 0;
	const char *attr;
	char buff[32];

	lua_settop(L, 1);
	if(!lua_istable(L, 1))	{
		lua_typerror(L, 1, "table");
		return 0;
	}
	lua_getglobal(L, Lua_processing);

	lua_pushnil(L);
	while(lua_next(L, 1) != 0) {
		cnt++;
		if(!lua_istable(L, -1))	{
			lua_pop(L, 1);
			luaL_error(L, "table wanted at #%d argument of Class_Attr_r", cnt);
			continue;
		}
		lua_len(L, -1);
		args = lua_tointeger(L, -1);
		lua_pop(L, 1);
		if(args == 0)	{
			lua_pop(L, 1);
			continue;
		}
		else if(args == 1)	{//default reader
			lua_pushinteger(L, 1);
			lua_gettable(L, -2);
			if(!lua_isstring(L, -1))	{
				lua_settop(L, 3);
				luaL_error(L, "string wanted at #%d argument of Class_Attr_r", cnt);
				continue;
			}
			attr = luaL_checkstring(L, -1);
			buff[0] = 'g';
			strcpy(buff + 1, sRand);
			strcpy(buff + 8, attr);
			lua_pushstring(L, buff);
			lua_insert(L, -2);
			lua_pushcclosure(L, default_reader, 1);
			lua_rawset(L, 2);
			lua_settop(L, 3);
		}
		else	{//custom reader
			lua_pushinteger(L, 1);
			lua_gettable(L, -2);
			if(!lua_isstring(L, -1))	{
				lua_settop(L, 3);
				luaL_error(L, "string wanted at #1 of #%d argument of Class_Attr_r", cnt);
				continue;
			}
			attr = luaL_checkstring(L, -1);
			buff[0] = 'g';
			strcpy(buff + 1, sRand);
			strcpy(buff + 8, attr);
			lua_pushstring(L, buff);
			lua_pushinteger(L, 2);
			lua_gettable(L, -4);
			if(!lua_isstring(L, -1))	{
				lua_settop(L, 3);
				luaL_error(L, "string wanted at #2 of #%d argument of Class_Attr_r", cnt);
				continue;
			}
			lua_rawget(L, 2);
			if(lua_isnil(L, -1) || !lua_isfunction(L, -1))	{
				lua_settop(L, 3);
				luaL_error(L, "cannot find function at #2 of #%d argument of Class_Attr_r", cnt);
				continue;
			}
			lua_rawset(L, 2);
			lua_settop(L, 3);
		}
	}
	return 0;
}

int LuaClass::default_reader(lua_State *L)	{
	char *ud;
	char buff[32];
	ud = static_cast<char*>(lua_touserdata(L, 1));
	lua_getuservalue(L, 1);
	buff[0] = '_'; buff[1] = '_';
	strcpy(buff + 2, luaL_checkstring(L, lua_upvalueindex(1)));
	lua_pushstring(L, buff);
	lua_rawget(L, -2);
	lua_insert(L, 1);
	lua_settop(L, 1);
	return 1;
}

int LuaClass::default_accessor(lua_State *L)	{
	char *ud;
	char buff[32];
	ud = static_cast<char*>(lua_touserdata(L, 1));
	lua_insert(L, 1);
	lua_getuservalue(L, 2);
	lua_settop(L, 1);
	buff[0] = '_'; buff[1] = '_';
	strcpy(buff + 2, luaL_checkstring(L, lua_upvalueindex(1)));
	lua_pushstring(L, buff);
	lua_pushvalue(L, 1);
	lua_rawset(L, -3);
	lua_settop(L, 0);
	return 0;
}

int LuaClass::Class_End(lua_State *L)	{
	if(!Lua_Class_Begin)	{
		luaL_error(L, "Class_Begin and Class_End mismatch");
		return 0;
	}
	lua_getglobal(L, Lua_processing);

	lua_pushliteral(L, "__newindex");
	lua_pushcfunction(L, nil_function);
	lua_rawset(L, -3);

	Lua_Class_Begin = false;
	return 0;
}

int LuaClass::nil_function(lua_State *L)	{
	return 0;
}

int LuaClass::Class_newO(lua_State *L)	{
	char *ud;
	const char *name;
	char buff[32];
	name = luaL_checkstring(L, lua_upvalueindex(1));
	int env;
	int dn = 0;
	if(lua_istable(L, 1))	{ //Direct new
		dn = 1;
		lua_newtable(L);
		env = lua_gettop(L);
		lua_newtable(L);
		int emeta = lua_gettop(L);
		
		lua_pushliteral(L, "__index");
		lua_pushcfunction(L, search_env);
		lua_settable(L, emeta);
		
		lua_pushliteral(L, "__newindex");
		lua_pushcfunction(L, set_env);
		lua_settable(L, emeta);

		buff[0] = 'e';
		lua_pushstring(L, strcpy(buff + 1, sRand) - 1);
		luaL_loadstring(L, "");
		lua_pushvalue(L, env);
		lua_setenv(L, -2);
		lua_remove(L, -2);
		lua_rawset(L, env);
		
		lua_setmetatable(L, env);
		ud = static_cast<char*>(lua_newuserdata(L, sizeof(char)));
		*ud = 1;
		lua_pushvalue(L, env);
		lua_setuservalue(L, -2);
		
		luaL_getmetatable(L, name);
		lua_setmetatable(L, -2);
		
		lua_remove(L, 1);
		lua_insert(L, 1);
	}
	else	{//get env ready
		ud = static_cast<char*>(lua_touserdata(L, 1));
		lua_getuservalue(L, 1);
		env = lua_gettop(L);
	}
	
	luaL_getmetatable(L, name);		
	lua_pushliteral(L, "__new");
	lua_gettable(L, -2);
	lua_insert(L, 1);
	env++;
	
	lua_pop(L, 1);
	
	if(!lua_isnil(L, lua_upvalueindex(2)))	{
		lua_pushliteral(L, "super");
		if(lua_toboolean(L, lua_upvalueindex(3)))	{
			lua_pushvalue(L, env);
			lua_pushvalue(L, lua_upvalueindex(2));
			lua_pushcclosure(L, superC, 2);
		}
		else	{
			lua_pushvalue(L, 2);
			lua_pushvalue(L, lua_upvalueindex(2));
			lua_pushcclosure(L, superL, 2);
		}
		lua_rawset(L, env);
	}
	else	{
		lua_pushliteral(L, "super");
		lua_pushnil(L);
		lua_rawset(L, env);
	}

	//call the new
	buff[0] = 'e';
	lua_pushstring(L, strcpy(buff + 1, sRand) - 1);
	lua_rawget(L, env);
	lua_setenv(L, 1);
	lua_pop(L, 1);
	lua_insert(L, 1);
	lua_pushvalue(L, 3);
	lua_insert(L, 1);
	lua_call(L, lua_gettop(L) - 3, 0);

	lua_pushliteral(L, "super");
	lua_pushnil(L);
	lua_rawset(L, 2);
	lua_remove(L, 2);
	if(dn)	{
		return 1;
	}
	return 0;
}

int LuaClass::superC(lua_State *L)	{
	lua_pushnil(L);
	lua_insert(L, 1);
	lua_pushvalue(L, lua_upvalueindex(2));
	lua_insert(L, 1);
	lua_call(L, lua_gettop(L) - 1, 1);
	
	lua_getglobal(L, "luaregRandString");
	lua_pushvalue(L, 1);
	lua_rawset(L, lua_upvalueindex(1));
	return 0;
}

int LuaClass::superL(lua_State *L)	{
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_insert(L, 1);
	lua_pushvalue(L, lua_upvalueindex(2));
	lua_insert(L, 1);
	lua_call(L, lua_gettop(L) - 1, 0);
	return 0;
}

int LuaClass::set_env(lua_State *L)	{
	const char *key = luaL_checkstring(L, 2);
	int top;
	
	if(key[0] == '_' && key[1] == '_')	{//Attribute
		lua_getglobal(L, "luaregRandString");
		lua_rawget(L, 1);
		if(!lua_isnil(L, -1))	{//have super C object
			lua_getmetatable(L, -1);
			lua_pushliteral(L, "__newindex");
			lua_rawget(L, -2);
			lua_remove(L, -2);
			
			lua_pushvalue(L, -2);
			lua_pushstring(L, key + 2);
			lua_pushvalue(L, 3);

			top = lua_gettop(L);

			if(!lua_pcall(L, 3, 0, 0))	{
				return 0;
			}
			else
				lua_settop(L, top - 4);
		}
		//No super C object or cannot find attribute
		lua_pop(L, 1);
		lua_rawset(L, 1);
		return 0;
	}
	if(key[0] == '_')	{//Global
		lua_setglobal(L, key);
		return 0;
	}
	lua_rawset(L, 1); //upvalue
	return 0;
}

int LuaClass::search_env(lua_State *L)	{
	const char *key = luaL_checkstring(L, 2);
	char buff[32];
	int top;
	
	if(key[0] == '_' && key[1] == '_')	{//Attribute
		lua_getglobal(L, "luaregRandString");
		lua_rawget(L, 1);
		if(!lua_isnil(L, -1))	{//have super C object
			lua_getmetatable(L, -1);
			lua_pushliteral(L, "__index");
			lua_rawget(L, -2);
			lua_remove(L, -2);
			
			buff[0] = 'g';
			strcpy(buff + 1, sRand);
			strcpy(buff + 8, key + 2);
			
			lua_pushvalue(L, -2);
			lua_pushstring(L, buff);

			top = lua_gettop(L);

			if(!lua_pcall(L, 2, 1, 0))	{
				lua_pushvalue(L, -2);
				if(!lua_pcall(L, 1, 1, 0))	{
					lua_insert(L, 1);
					lua_settop(L, 1);
					return 1;
				}
				lua_settop(L, top - 3);
			}
			else
				lua_settop(L, top - 3);
		}
		//No super C object or cannot find attribute
		lua_rawget(L, 1);
		lua_insert(L, 1);
		lua_settop(L, 1);
		return 1;
	}
	if(key[0] == '_')	{//Global
		lua_getglobal(L, key);
		lua_insert(L, 1);
		lua_settop(L, 1);
		return 1;
	}
	lua_rawget(L, 1);
	if(lua_isnil(L, -1))	{
		lua_getglobal(L, key);
		lua_insert(L, 1);
		lua_settop(L, 1);
		return 1;
	}
	lua_insert(L, 1);
	lua_settop(L, 1);
	return 1;
}

int LuaClass::Object_index(lua_State *L)	{
	const char *key = luaL_checkstring(L, 2);
	const char *name;
	const char *sC, *super;
	char buff[32];
	char *ud;
	int top;
	ud = static_cast<char*>(lua_touserdata(L, 1));
	if(!*ud)	{
		if(!strcmp(key, "disposed"))	{
			lua_pushcfunction(L, Object_disposed);
			lua_insert(L, 1);
			lua_settop(L, 1);
			return 1;
		}
		luaL_error(L, "Object Disposed");
		return 0;
	}

	name = luaL_checkstring(L, lua_upvalueindex(1));
	sC = name;

	while(*sC)	{
		luaL_getmetatable(L, sC);
		lua_pushliteral(L, "__isC");
		lua_rawget(L, -2);
		if(lua_toboolean(L, -1))	{
			lua_pop(L, 2);
			break;
		}
		else	{
			lua_pop(L, 1);
			lua_pushliteral(L, "__super");
			lua_rawget(L, -2);
			super = luaL_checkstring(L, -1);
			lua_pop(L, 2);
		}

		lua_getglobal(L, sC);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		if(!lua_isnil(L, -1))	{
			lua_insert(L, 1);
			lua_settop(L, 1);
			return 1;
		}
		else {
			lua_pop(L, 2);
			sC = super;
		}
	}
	top = lua_gettop(L);

	lua_getuservalue(L, 1);
	lua_getglobal(L, "luaregRandString");
	lua_rawget(L, -2);

	if(!lua_isnil(L, -1)) {//have C object
		lua_getmetatable(L, -1);
		lua_pushliteral(L, "__index");
		lua_rawget(L, -2);
		lua_remove(L, -2);
		
		lua_pushvalue(L, -2);
		lua_pushvalue(L, 2);

		if(!lua_pcall(L, 2, 1, 0))	{
			lua_insert(L, 1);
			lua_settop(L, 1);
			return 1;
		}
		else
			lua_settop(L, top);
	}
	else {
		lua_pop(L, 2);
	}

	//Cannot find attribute in C Object or no C object
	//Try Attribute
	sC = name;
	buff[0] = 'g';
	strcpy(buff + 1, sRand);
	strcpy(buff + 8, key);
	while(*sC)	{
		luaL_getmetatable(L, sC);
		lua_pushliteral(L, "__isC");
		lua_rawget(L, -2);
		if(lua_toboolean(L, -1))	{
			lua_pop(L, 2);
			break;
		}
		else	{
			lua_pop(L, 1);
			lua_pushliteral(L, "__super");
			lua_rawget(L, -2);
			super = luaL_checkstring(L, -1);
			lua_pop(L, 2);
		}

		lua_getglobal(L, sC);
		lua_pushstring(L, buff);
		lua_rawget(L, -2);
		if(!lua_isnil(L, -1))	{
			lua_pushvalue(L, 1);
			lua_pushvalue(L, 2);
			lua_call(L, 2, 1);
			lua_insert(L, 1);
			lua_settop(L, 1);
			return 1;
		}
		else {
			lua_pop(L, 2);
			sC = super;
		}
	}
	luaL_error(L, "The class \"%s\" has no attribution or method \"%s\"", name, key);
	return 0;
}

int LuaClass::Object_newindex(lua_State *L)	{
	const char *key = luaL_checkstring(L, 2);
	const char *name;
	const char *sC, *super;
	char buff[32];
	char *ud;
	int top;
	ud = static_cast<char*>(lua_touserdata(L, 1));
	if(!*ud)	{
		luaL_error(L, "Object Disposed");
		return 0;
	}

	top = lua_gettop(L);
	lua_getuservalue(L, 1);
	lua_getglobal(L, "luaregRandString");
	lua_rawget(L, -2);

	if(!lua_isnil(L, -1)) {//have C object
		lua_getmetatable(L, -1);
		lua_pushliteral(L, "__newindex");
		lua_rawget(L, -2);
		lua_remove(L, -2);
		
		lua_pushvalue(L, -2);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);

		if(!lua_pcall(L, 3, 0, 0))	{
			return 0;
		}
		else
			lua_settop(L, top);
	}
	else {
		lua_pop(L, 2);
	}

	//Cannot find attribute in C Object or no C object
	//Try Attribute
	name = luaL_checkstring(L, lua_upvalueindex(1));
	sC = name;

	buff[0] = 's';
	strcpy(buff + 1, sRand);
	strcpy(buff + 8, key);
	while(*sC)	{
		luaL_getmetatable(L, sC);
		lua_pushliteral(L, "__isC");
		lua_rawget(L, -2);
		if(lua_toboolean(L, -1))	{
			lua_pop(L, 2);
			break;
		}
		else	{
			lua_pop(L, 1);
			lua_pushliteral(L, "__super");
			lua_rawget(L, -2);
			super = luaL_checkstring(L, -1);
			lua_pop(L, 2);
		}

		lua_getglobal(L, sC);
		lua_pushstring(L, buff);
		lua_rawget(L, -2);
		if(!lua_isnil(L, -1))	{
			lua_pushvalue(L, 1);
			lua_pushvalue(L, 2);
			lua_pushvalue(L, 3);
			lua_call(L, 3, 0);
			lua_settop(L, 0);
			return 0;
		}
		else {
			lua_pop(L, 2);
			sC = super;
		}
	}
	luaL_error(L, "The class \"%s\" has no attribution \"%s\"", name, key);
	return 0;
}

int LuaClass::lfunction_handler(lua_State *L)	{
	lua_pushvalue(L, lua_upvalueindex(2));
	lua_insert(L, 1);
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_insert(L, 1);
	lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
	return lua_gettop(L);
}

int LuaClass::cfunction_handler(lua_State *L)	{
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_insert(L, 1);
	lua_pushvalue(L, lua_upvalueindex(2));
	lua_insert(L, 1);
	lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
	return lua_gettop(L);
}

int LuaClass::function_handle(lua_State *L)	{
	char *ud;
	char buff[32];
	int env;

	ud = static_cast<char*>(lua_touserdata(L, 1));
	
	if(!*ud)	{
		luaL_error(L, "Object Disposed");
		return 0;
	}

	if(!lua_getmetatable(L, 1))	{
		luaL_error(L, "Invalid Object(use : instead of . when calling method)");
		return 0;
	}
	lua_pop(L, 1);
	lua_getuservalue(L, 1);

	lua_pushliteral(L, "super");
	lua_rawget(L, -2);
	lua_insert(L, 1);
	lua_pushliteral(L, "super");
	lua_insert(L, 1);
	lua_pushvalue(L, -1);
	lua_insert(L, 1);

	env = lua_gettop(L);

	if(!lua_isnil(L, lua_upvalueindex(2)))	{
		lua_pushliteral(L, "super");
		if(lua_toboolean(L, lua_upvalueindex(3)))	{
			lua_pushvalue(L, lua_upvalueindex(2));
			lua_getglobal(L, "luaregRandString");
			lua_rawget(L, env);
			lua_pushcclosure(L, cfunction_handler, 2);
		}
		else	{
			lua_pushvalue(L, lua_upvalueindex(2));
			lua_pushvalue(L, 4);
			lua_pushcclosure(L, lfunction_handler, 2);
		}
		lua_rawset(L, env);
	}
	else	{
		lua_pushliteral(L, "super");
		lua_pushnil(L);
		lua_rawset(L, env);
	}

	lua_pushvalue(L, lua_upvalueindex(1));
	buff[0] = 'e';
	lua_pushstring(L, strcpy(buff + 1, sRand) - 1);
	lua_rawget(L, env);
	lua_setenv(L, -2);
	lua_pop(L, 1);
	lua_remove(L, env);
	
	lua_insert(L, 4);
	lua_call(L, lua_gettop(L) - 4, LUA_MULTRET);

	lua_pushvalue(L, 1); lua_pushvalue(L, 2); lua_pushvalue(L, 3);
	lua_rawset(L, -3);
	lua_pop(L, 1);
	lua_remove(L, 1); lua_remove(L, 1); lua_remove(L, 1);
	return lua_gettop(L);
}

int LuaClass::Object_dispose(lua_State *L)	{
	char *ud;
	const char *super, *name;
	char buff[32];
	name = luaL_checkstring(L, lua_upvalueindex(1));
	int env;
	ud = static_cast<char*>(lua_touserdata(L, 1));
	if(!*ud)	{
		luaL_error(L, "Object already disposed");
		return 0;
	}
	lua_getuservalue(L, 1);
	env = lua_gettop(L);

	//dispose C Object
	lua_getglobal(L, "luaregRandString");
	lua_rawget(L, -2);
	if(!lua_isnil(L, -1))	{
		lua_getmetatable(L, -1);
		lua_pushliteral(L, "__index");
		lua_rawget(L, -2);
		lua_remove(L, -2);
		lua_pushvalue(L, -2);
		lua_pushliteral(L, "dispose");
		if(!lua_pcall(L, 2, 1, 0))	{
			lua_insert(L, -2);
			lua_call(L, 1, 0);
		}
		else	{
			lua_settop(L, 2);
		}
	}
	else	{
		lua_pop(L, 1);
	}

	lua_getglobal(L, "luaregRandString");
	lua_pushnil(L);
	lua_rawset(L, env);
	
	lua_insert(L, 1);
	while(*name)	{
		luaL_getmetatable(L, name);
		lua_pushliteral(L, "__super");
		lua_rawget(L, -2);
		super = luaL_checkstring(L, -1);
		lua_pop(L, 1);

		lua_pushliteral(L, "__isC");
		lua_rawget(L, -2);
		if(lua_toboolean(L, -1))
			break;
		lua_pop(L, 1);
	
		lua_pushliteral(L, "__dispose");
		lua_rawget(L, -2);
		if(lua_isnil(L, -1))	{
			lua_settop(L, 2);
			name = super;
			continue;
		}
		lua_insert(L, 3);
	
		lua_pop(L, 1);
	
		buff[0] = 'e';
		lua_pushstring(L, strcpy(buff + 1, sRand) - 1);
		lua_rawget(L, 1);
		lua_setenv(L, 3);
		lua_pop(L, 1);
		lua_pushvalue(L, 2);

		if(lua_pcall(L, 1, 0, 0))	{
			fputs(luaL_checkstring(L, -1), stdout);
			lua_pop(L, 1);
			return 0;
		}
		name = super;
	}
	lua_pushlightuserdata(L, (void*)ud);
	lua_pushnil(L);
	lua_settable(L, LUA_REGISTRYINDEX);
	*ud = 0;
	return 0;
}

int LuaClass::Object_disposed(lua_State *L)	{
	char *ud;
	ud = static_cast<char*>(lua_touserdata(L, 1));
	lua_settop(L, 0);
	if(*ud)
		lua_pushboolean(L, false);
	else
		lua_pushboolean(L, true);
	return 1;
}

int LuaClass::Object_gc(lua_State *L)	{
	char *ud = static_cast<char*>(lua_touserdata(L, 1));
	if(*ud)	{
		lua_pushstring(L, luaL_checkstring(L, lua_upvalueindex(1)));
		lua_pushcclosure(L, Object_dispose, 1);
		lua_insert(L, 1);
		lua_pcall(L, 1, 0, 0);
	}
	return 0;
}

int LuaClass::Class_newindex(lua_State *L)	{
	lua_settop(L, 3);
	const char *key = luaL_checkstring(L, 2);
	const char *super, *snext;
	int isC;
	
	if(lua_isfunction(L, 3)) {
		if(!strcmp(key, "new"))	{
			luaL_getmetatable(L, Lua_processing);
			lua_pushliteral(L, "__new");
			lua_pushvalue(L, 3);
			lua_settable(L, -3);
			lua_pop(L, 2);
			lua_pushstring(L, Lua_processing);

			if(*Lua_super)	{//have super Class
				luaL_getmetatable(L, Lua_super);
				lua_pushliteral(L, "__isC");
				lua_gettable(L, -2);

				if(lua_toboolean(L, -1))	{ // super is C Object
					lua_pop(L, 2);
					lua_getglobal(L, Lua_super);
					lua_pushliteral(L, "new");
					lua_gettable(L, -2);
					lua_remove(L, -2);
					lua_pushboolean(L, true);
				}
				else	{
					lua_pop(L, 2);
					lua_getglobal(L, Lua_super);
					lua_pushliteral(L, "new");
					lua_gettable(L, -2);
					lua_remove(L, -2);
					lua_pushboolean(L, false);
				}
			}
			else{
				lua_pushnil(L);
				lua_pushnil(L);
			}

			lua_pushcclosure(L, Class_newO, 3);
		}
		else if(!strcmp(key, "dispose")) {
			luaL_getmetatable(L, Lua_processing);
			lua_pushliteral(L, "__dispose");
			lua_pushvalue(L, 3);
			lua_settable(L, -3);
			lua_pop(L, 2);
			lua_pushliteral(L, "disposed");
			lua_pushcfunction(L, Object_disposed);
			lua_rawset(L, 1);
			lua_pushstring(L, Lua_processing);
			lua_pushcclosure(L, Object_dispose, 1);
		}
		else{
			super = Lua_super;
			while(*super)	{
				luaL_getmetatable(L, super);
				lua_pushliteral(L, "__isC");
				lua_rawget(L, -2);
				isC = lua_toboolean(L, -1);
				lua_pushliteral(L, "__super");
				lua_rawget(L, -3);
				snext = luaL_checkstring(L, -1);
				lua_pop(L, 3);

				lua_getglobal(L, super);
				lua_pushstring(L, key);
				lua_rawget(L, -2);
				lua_remove(L, -2);
				if(lua_isfunction(L, -1))	{
					lua_pushboolean(L, isC);
					break;
				}
				if(!lua_isnil(L, -1))	{
					lua_pop(L, 1);
					lua_pushnil(L);
					lua_pushnil(L);
					break;
				}
				lua_pop(L, 1);
				super = snext;
			}
			if(!*super)	{
				lua_pushnil(L);
				lua_pushnil(L);
			}

			lua_pushcclosure(L, function_handle, 3);
		}
	}
	lua_rawset(L, 1);
	return 0;
}

int LuaClass::Lua_Class_Begin;
const char *LuaClass::Lua_super;
const char *LuaClass::Lua_processing;
const char *LuaClass::lua_key;
const char *LuaClass::sRand;