/*
===========================================================================
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// lua_entity.c -- entity library for Lua


#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "g_local.h"

typedef struct
{
	gentity_t      *e;
} lua_Entity;

static lua_Entity *entity_Get(lua_State * L)
{
	void           *ud;

//	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
//	ud = lua_touserdata(L, 1);
	ud = luaL_checkudata(L, 1, "game.entity");
	luaL_argcheck(L, ud != NULL, 1, "`entity' expected");
	return (lua_Entity *) ud;
}


static int entity_New(lua_State * L)
{
	lua_Entity     *lent;

	lent = lua_newuserdata(L, sizeof(lua_Entity));
//	lua_pushlightuserdata(L, ent);

	luaL_getmetatable(L, "game.entity");
	lua_setmetatable(L, -2);
	
	lent->e = G_Spawn();

	return 1;
}

static int entity_GetNumber(lua_State * L)
{
	lua_Entity     *lent;

	lent = entity_Get(L);
	lua_pushnumber(L, lent->e - g_entities);
//	lua_pushnumber(L, lent->e->s.number);

	return 1;
}

static int entity_GetClassName(lua_State * L)
{
	lua_Entity     *lent;

	lent = entity_Get(L);
	lua_pushstring(L, lent->e->classname);

	return 1;
}

static int entity_SetClassName(lua_State * L)
{
	lua_Entity     *lent;
//	char           *classname;

	lent = entity_Get(L);
	lent->e->classname = (char *) luaL_checkstring(L, 2);
	
//	lent->e->classname = classname;

	return 1;
}

static int entity_GC(lua_State * L)
{
	G_Printf("lua says bye to entity = %p\n", entity_Get(L));
	
	return 0;
}

static int entity_ToString(lua_State * L)
{
	lua_Entity     *lent;
	gentity_t      *gent;
	char            buf[MAX_STRING_CHARS];
	
	lent = entity_Get(L);
	gent = lent->e;
	Com_sprintf(buf, sizeof(buf), "entity: class=%s name=%s id=%i pointer=%p\n", gent->classname, gent->targetname, gent - g_entities, gent);
	lua_pushstring(L, buf);
	
	return 1;
}


static const luaL_reg entity_ctor[] = {
	{"new", entity_New},
	{NULL, NULL}
};

static const luaL_reg entity_meta[] = {
	{"__gc", entity_GC},
	{"__tostring", entity_ToString},
	{"GetNumber", entity_GetNumber},
	{"GetClassName", entity_GetClassName},
	{"SetClassName", entity_SetClassName},
	{NULL, NULL}
};

int luaopen_entity(lua_State * L)
{
	luaL_newmetatable(L, "game.entity");
	
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  // pushes the metatable
	lua_settable(L, -3);  // metatable.__index = metatable
	
	luaL_register(L, NULL, entity_meta);
	luaL_register(L, "entity", entity_ctor);

	return 1;
}
