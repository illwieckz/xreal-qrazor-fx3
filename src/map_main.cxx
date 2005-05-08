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
// qrazor-fx ----------------------------------------------------------------
#include "map_local.h"

#include "common.h"
#include "cmd.h"


// xreal --------------------------------------------------------------------

int		cm_pointcontents;
int		cm_traces;
int		cm_brush_traces;

namespace map
{

entity_v			entities;
brush_v				brushes;
brushside_v			brushsides;
plane_v				planes;

int		c_structural = 0;
int		c_detail = 0;
int		c_areaportals = 0;


static void	BSPInfo_f()
{
	if(Cmd_Argc() != 2)
	{
		Com_Printf("usage : bspinfo <levelname>\n");
		return;
	}
	
	std::string filename = "maps/" + std::string(Cmd_Argv(1)) + ".bsp";
	
	LoadBSPFile(filename);
	
	PrintBSPFileSizes();
}

static void	ProcessModels()
{
	for(entity_i i = entities.begin(); i != entities.end(); ++i)
	{
		if((*i)->isWorldSpawn())
			(*i)->processWorldModel();
		else
			(*i)->processSubModel();
	}
}

static void	Map2BSP_f()
{
	if(Cmd_Argc() != 2)
	{
		Com_Printf("usage : map2bsp <levelname>\n");
		return;
	}
	
	std::string filename = "maps/" + std::string(Cmd_Argv(1)) + ".map";
	
	//TODO
	
	LoadMapFile(filename);
	
	ProcessModels();
}


static void	PlaneList_f()
{
	Com_Printf("------------------\n");

	for(plane_ci i = planes.begin(); i != planes.end(); ++i)
	{
		Com_Printf("%s\n", (*i)->toString());
	}
	
	Com_Printf("Total planes count: %i\n", planes.size());
}

} // namespace map

void	Map_Init()
{
	Com_Printf("------- Map_Init -------\n");

	Cmd_AddCommand("bspinfo",		map::BSPInfo_f);
	Cmd_AddCommand("map2bsp",		map::Map2BSP_f);
	
	Cmd_AddCommand("planelist",		map::PlaneList_f);
	
	Cmd_AddCommand("shaderlist",		map::ShaderList_f);
	Cmd_AddCommand("shadercachelist",	map::ShaderCacheList_f);
	Cmd_AddCommand("shadersearch",		map::ShaderSearch_f);
	
	map::InitShaders();
	
	//TODO
}

void	Map_Shutdown()
{
	Com_Printf("------- Map_Shutdown -------\n");
	
	map::ShutdownShaders();
	
	//TODO
}

void	Map_Frame(int msec)
{
	//DO NOTHING
}

