/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================


/// includes ===================================================================
// system -------------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_local.h"
#include "g_world.h"
//#include "g_ode.h"


#if 0
	// cursor positioning
	xl <value>
	xr <value>
	yb <value>
	yt <value>
	xv <value>
	yv <value>

	// drawing
	statpic <name>
	pic <stat>
	num <fieldwidth> <stat>
	string <stat>

	// control
	if <stat>
	ifeq <stat> <value>
	ifbit <stat> <value>
	endif

#endif

char *single_statusbar = 
"yb	-32 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xv	262 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "
;



/*
char *dm_statusbar =
"yb	-32 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xv	246 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

//  frags
"xr	-96 "
"yt 2 "
"color 1 1 1 1 "
"num 3 14 "

// spectator
"if 17 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "
;
*/


char *dm_statusbar =
"yb	-40 "

// health
"	xv	35 "
"	color 1 1 1 1 "
"	pic 0 "
"	xv	55 "
"	hnum "
"	yb	-15 "
"	color 0 1 0 1 "
"	string HEALTH "

// ammo
//"if 2 "
"	xv	-50 "
"	yb	-40 "
"	color 1 1 1 1 "
"	pic 2 "
"	xv	-30 "
"	anum "
"	yb	-15 "
"	color 0 1 0 1 "
"	string AMMO "
//"endif "

// armor
//"if 4 "
"	xv	200 "
"	yb	-40 "
"	pic 4 "
"	xv	220 "
"	rnum "
"	yb	-15 "
"	color 0 1 0 1 "
"	string ARMOR "
//"endif "

// selected item
"if 6 "
"	xv	296 "
"	yb	-40 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	yb 	-128 "
"	pic 7 "
"	xv	26 "
"	yb	-128 "
"	stat_string 8 "
"	yb	-90 "
"endif "

// timer
"if 9 "
"	xv	246 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

//  frags
"	xv	290 "
"	yb	-70 "
"	pic 14 "
"	xv	310 "
"	yb	-40 "
"	fnum "
"	yb	-15 "
"	color 0 1 0 1 "
"	string FRAGS "


// spectator
"if 18 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 17 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 17 "
"endif "


// hud bar
"if 19 "
"	xv	-100 "
"	yb	-128 "
"	color 0.5 0.5 0.5 1 "
"	pic	19 "
"endif "

// weapon window 0
"if 20 "
	// window border
"	yb	-384 "
"	xr	-96 "
"	pic	20 "
	// weapon ammo
"	yb	-330 "
"	xr	-56 "
"	wnum 	21 "
	// weapon model
"	yb	-370 "
"	xr	-85 "
"	size	72 35 "
"	model	22 45 "
"endif "

/*
// weapon window 1
"if 22 "
"	yb	-256 "
"	xr	-96 "
"	pic	22 "
"	wnum 	23 "
"endif "

// weapon window 2
"if 24 "
"	yb	-256 "
"	xr	-96 "
"	pic	24 "
"	wnum 	25 "
"endif "

// weapon window 3
"if 26 "
"	yb	-256 "
"	xr	-96 "
"	pic	26 "
"	wnum 	27 "
"endif "

// weapon window 4
"if 28 "
"	yb	-256 "
"	xr	-96 "
"	pic	28 "
"	wnum 	29 "
"endif "

// weapon window 5
"if 30 "
"	yb	-256 "
"	xr	-96 "
"	pic	30 "
"	wnum 	31 "
"endif "


// selected weapon window
"if 33 "
"	yb	-256 "
"	xr	-96 "
"	pic	33 "
//"	anum "
"endif "
*/
;






/*
================================================================================
				WORLD ENTITY
================================================================================
*/


/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"	environment map name
"skyaxis"	vector axis for rotating sky
"skyrotate"	speed of rotation in degrees/second
"sounds"	music cd track number
"gravity"	800 is default gravity
"message"	text to print at user logon
*/
g_world_c::g_world_c()
{
	addField(g_field_c("sky", &_sky, F_STRING, FFL_SPAWNTEMP));
	addField(g_field_c("skyrotate", &_skyrotate, F_FLOAT, FFL_SPAWNTEMP));
	addField(g_field_c("skyaxis", &_skyaxis, F_VECTOR, FFL_SPAWNTEMP));
	
	addField(g_field_c("nextmap", &_nextmap, F_STRING, FFL_SPAWNTEMP));
	
	addField(g_field_c("lip", &_lip, F_INT, FFL_SPAWNTEMP));
	addField(g_field_c("distance", &_distance, F_INT, FFL_SPAWNTEMP));
	addField(g_field_c("height", &_height, F_INT, FFL_SPAWNTEMP));
	addField(g_field_c("noise", &_noise, F_STRING, FFL_SPAWNTEMP));
	addField(g_field_c("pausetime", &_pausetime, F_FLOAT, FFL_SPAWNTEMP));
	addField(g_field_c("item", &_item, F_STRING, FFL_SPAWNTEMP));
	
	addField(g_field_c("minyaw", &_minyaw, F_FLOAT, FFL_SPAWNTEMP));
	addField(g_field_c("maxyaw", &_maxyaw, F_FLOAT, FFL_SPAWNTEMP));
	addField(g_field_c("minpitch", &_minpitch, F_FLOAT, FFL_SPAWNTEMP));
	addField(g_field_c("maxpitch", &_maxpitch, F_FLOAT, FFL_SPAWNTEMP));
}

g_world_c::~g_world_c()
{
	//DO NOTHING
}

void	g_world_c::activate()
{
	g_entity_c *ent = this;
	
	ent->_movetype = MOVETYPE_PUSH;
	ent->_r.solid = SOLID_BSP;
	ent->_r.inuse = true;			// since the world doesn't use G_Spawn()
	ent->_s.index_model = 1;		// world model is always index 1

	// set configstrings for items
	G_SetItemNames();

	if(_nextmap.length())
		level.nextmap = _nextmap;

	// make some data visible to the server
	if(ent->_message.length() && ent->_message[0])
	{
		//trap_Com_Printf("g_world_c::activate: message '%s'\n", ent->_message.c_str());
		
		trap_SV_SetConfigString(CS_MAPMESSAGE, ent->_message);
		level.level_name = ent->_message;
	}
	else
		level.level_name = level.mapname;


	trap_SV_SetConfigString(CS_SKY, "cubemaps/dots");

	//trap_SV_Configstring (CS_CDTRACK, va("%i", ent->sounds) );

	trap_SV_SetConfigString(CS_MAXCLIENTS, va("%i", maxclients->getInteger()));

	
	// status bar program	//TODO use client side GUIs based on XUL scripting
	if(deathmatch->getValue())
		trap_SV_SetConfigString(CS_STATUSBAR, dm_statusbar);
	else
		trap_SV_SetConfigString(CS_STATUSBAR, "");

	/*
	if(_gravity)
	{
		trap_Cvar_SetValue("g_gravity", _gravity);
		
		vec3_c gravity(0, 0, (-9.81 * 32/10) * g_gravity->value);

		g_ode_world->setGravity(gravity);
	}
	*/	
		
	G_SetWorldModel(this, "*0");
//	G_SetWorldModel(this, "worldMap");

//	delete g_ode_bsp; 
//	g_ode_bsp = NULL;
}

void	SP_worldspawn(g_entity_c **entity)
{
	g_entity_c *ent = (g_entity_c*)g_entities[0];
	*entity = ent;
}

