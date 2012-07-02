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
#ifndef luareg_h
#define luareg_h

#include "lua.hpp"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define UNGC_TABLE_NAME "ungc_object"

//Lua Additional Functions
int lua_setenv (lua_State *L, int index);
typedef int (*luareg_nfp)(lua_State *L);

//For C Classes and Functions
template <typename T>
class LuaReg	{
	typedef struct { T *pT; } userdataType;
	public:
		typedef int (T::*mfp)(lua_State *L);
		typedef struct	{
			const char *name;
			mfp mfunc;
		} RegType;
		
		typedef struct	{
			const char *name;
			luareg_nfp mfunc;
		} GlobalRegType;
		
		static void Register(lua_State *L)	{
			char buff[32];
			const char *sRand;
			
			lua_getglobal(L, "luaregRandString");
			sRand = luaL_checkstring(L, -1);
			lua_remove(L, -1);
			
			lua_newtable(L);
			int methods = lua_gettop(L);
			
			luaL_newmetatable(L, T::className);
			int metatable = lua_gettop(L);
			
			lua_pushvalue(L, methods);
			lua_setglobal(L, T::className);
				
			lua_pushliteral(L, "__isC");
			lua_pushboolean(L, true);
			lua_settable(L, metatable);
			
			lua_pushliteral(L, "__metatable");
			lua_pushvalue(L, methods);
			lua_settable(L, metatable);
			
			lua_pushliteral(L, "__name");
			lua_pushstring(L, T::className);
			lua_settable(L, metatable);
			
			lua_pushliteral(L, "__index");
			lua_pushcfunction(L, index_T);
			lua_settable(L, metatable);
			
			lua_pushliteral(L, "__newindex");
			lua_pushcfunction(L, newindex_T);
			lua_settable(L, metatable);
			
			lua_pushliteral(L, "__tostring");
			lua_pushcfunction(L, tostring_T);
			lua_settable(L, metatable);
			
			lua_pushliteral(L, "__gc");
			lua_pushcfunction(L, gc_T);
			lua_settable(L, metatable);
			
			lua_pushliteral(L, "__super");
			lua_pushstring(L, T::super);
			lua_settable(L, metatable);
			
			lua_pushliteral(L, "new");
			lua_pushcfunction(L, new_T);
			lua_settable(L, methods);
			
			lua_newtable(L);
			int mt = lua_gettop(L);
			
			lua_pushliteral(L, "__call");
			lua_pushcfunction(L, new_T);
			lua_settable(L, mt);
			
			lua_setmetatable(L, methods);
			
			for (GlobalRegType *l = T::global_methods; l->name; l++)	{
				lua_pushstring(L, l->name);
				lua_pushlightuserdata(L, (void*)l);
				lua_pushcclosure(L, thunk_g, 1);
				lua_settable(L, methods);
			}
			
			for (RegType *l = T::methods; l->name; l++)	{
				lua_pushstring(L, l->name);
				lua_pushlightuserdata(L, (void*)l);
				lua_pushcclosure(L, thunk_m, 1);
				lua_settable(L, methods);
			}
			
			for (RegType *l = T::attribs_r; l->name; l++)	{
				buff[0] = 'g';
				strcpy(buff + 1, sRand);
				strcpy(buff + 8, l->name);
				lua_pushstring(L, buff);
				lua_pushlightuserdata(L, (void*)l);
				lua_pushcclosure(L, thunk_m, 1);
				lua_settable(L, methods);
			}
			
			for (RegType *l = T::attribs_a; l->name; l++)	{
				buff[0] = 's';
				strcpy(buff + 1, sRand);
				strcpy(buff + 8, l->name);
				lua_pushstring(L, buff);
				lua_pushlightuserdata(L, (void*)l);
				lua_pushcclosure(L, thunk_m, 1);
				lua_settable(L, methods);
			}
			
			lua_pop(L, 2);
		}
		
	private:
		LuaReg();

		static int thunk_m(lua_State *L)	{
			T *obj = check(L, 1);
			lua_remove(L, 1);
			RegType *l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
			return (obj->*(l->mfunc))(L);
		}
		
		static int thunk_g(lua_State *L)	{
			lua_remove(L, 1);
			GlobalRegType *l = static_cast<GlobalRegType*>(lua_touserdata(L, lua_upvalueindex(1)));
			return (*(l->mfunc))(L);
		}
		
	public:
		static T *check(lua_State *L, int narg)	{
			if(!lua_getmetatable(L, 1))	{
				luaL_error(L, "Invalid Userdata");
				return NULL;
			}
			lua_pushliteral(L, "__name");
			lua_rawget(L, -2);
			const char *sC = luaL_checkstring(L, -1);
			lua_pop(L, 2);
			void *pUserData = luaL_checkudata(L, narg, sC);
			userdataType *ud = static_cast<userdataType*>(pUserData);
			if( !ud )
				lua_typerror(L, narg, sC);
			return ud->pT;
		}
		
		static int new_T(lua_State *L)	{
			lua_remove(L, 1);
			T *obj = new T(L);
			userdataType *ud =
				static_cast<userdataType*>(lua_newuserdata(L, sizeof(userdataType)));
			ud->pT = obj;
			luaL_getmetatable(L, T::className);
			lua_setmetatable(L, -2);
			return 1;
		}
		
	private:
		static int gc_T(lua_State *L)	{
			userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
			T *obj = ud->pT;
			delete(obj);
			return 0;
		}
		
		static int tostring_T(lua_State *L)	{
			char buff[32];
			userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
			T *obj = ud->pT;
			sprintf(buff, "%p", obj);
			lua_pushfstring(L, "%s (%s)", T::className, buff);
			return 1;
		}
		
		static int newindex_T(lua_State *L)	{
			char buff[32];
			const char *sC = T::className;
			const char *sRand;
			
			lua_getglobal(L, "luaregRandString");
			sRand = luaL_checkstring(L, -1);
			lua_remove(L, -1);

			buff[0] = 's';
			strcpy(buff + 1, sRand);
			const char *aName = luaL_checkstring(L, -2);
			strcpy(buff + 8, aName);
			lua_remove(L, -2);

			while(*sC)	{
				lua_getglobal(L, sC);
				lua_pushstring(L, buff);
				lua_rawget(L, -2);
				if(!lua_isnil(L, -1))
					break;
				else	{
					lua_pop(L, 2);
					luaL_getmetatable(L, sC);
					lua_pushliteral(L, "__super");
					lua_rawget(L, -2);
					sC = luaL_checkstring(L, -1);
					lua_pop(L, 2);
				}
			}
			if(*sC)	{
				lua_remove(L, -2);
				lua_insert(L, -3);
				lua_call(L, 2, 0);
			}
			else	{
				lua_pop(L, 2);
				luaL_error(L, "The class \"%s\" has no attribution \"%s\"", T::className, buff + 8);
			}
			return 1;
		}
		
		static int index_T(lua_State *L)	{
			const char *sC = T::className;
			const char *aName, *sRand;
			char buff[32];
			while(*sC)	{
				lua_getglobal(L, sC);
				lua_pushvalue(L, -2);
				lua_rawget(L, -2);
				if(!lua_isnil(L, -1))
					break;
				else	{
					lua_pop(L, 2);
					luaL_getmetatable(L, sC);
					lua_pushliteral(L, "__super");
					lua_rawget(L, -2);
					sC = luaL_checkstring(L, -1);
					lua_pop(L, 2);
				}
			}
			if(*sC)	{
				lua_insert(L, -3);
				lua_settop(L, -3);
			}
			else	{
				aName = luaL_checkstring(L, -1);
				
				lua_getglobal(L, "luaregRandString");
				sRand = luaL_checkstring(L, -1);
				lua_pop(L, 2);
				
				buff[0] = 'g';
				strcpy(buff + 1, sRand);
				strcpy(buff + 8, aName);
				
				sC = T::className;
				while(*sC)	{
					lua_getglobal(L, sC);
					lua_pushstring(L, buff);
					lua_rawget(L, -2);
					if(!lua_isnil(L, -1))
						break;
					else	{
						lua_pop(L, 2);
						luaL_getmetatable(L, sC);
						lua_pushliteral(L, "__super");
						lua_rawget(L, -2);
						sC = luaL_checkstring(L, -1);
						lua_pop(L, 2);
					}
				}
				if(*sC)	{
					lua_insert(L, -3);
					lua_settop(L, -2);
					lua_call(L, 1, 1);
				}
				else	{
					lua_pop(L, 1);
					luaL_error(L, "The class \"%s\" has no attribution \"%s\"", T::className, buff + 8);
				}
			}
			return 1;
		}
		
};

//For Lua Classes and Functions
class LuaClass	{
	private:
		static int Lua_Class_Begin;
		static const char *Lua_processing;
		static const char *Lua_super;
		static const char *lua_key;
		static const char *sRand;
		static lua_State *LVM;
	
	public:
		static void Init(lua_State *L);
		static const char *objname(lua_State *L, int narg);
		static void ungc_table(lua_State *L);
		static void remove_ungc(int ref);

	private:
		static int Class_Begin(lua_State *L);
		static int Class_Attr_a(lua_State *L);
		static int Class_Attr_r(lua_State *L);
		static int default_reader(lua_State *L);
		static int default_accessor(lua_State *L);
		static int Class_End(lua_State *L);
		static int nil_function(lua_State *L);
		static int Class_newO(lua_State *L);
		static int superC(lua_State *L);
		static int superL(lua_State *L);
		static int set_env(lua_State *L);
		static int search_env(lua_State *L);
		static int Object_index(lua_State *L);
		static int Object_newindex(lua_State *L);
		static int lfunction_handler(lua_State *L);
		static int cfunction_handler(lua_State *L);
		static int function_handle(lua_State *L);
		static int Object_dispose(lua_State *L);
		static int Object_disposed(lua_State *L);
		static int Object_gc(lua_State *L);
		static int Class_newindex(lua_State *L);
};

#endif