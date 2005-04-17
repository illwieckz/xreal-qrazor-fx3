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


std::vector<map_entity_c*>	map_entities;
std::vector<map_brush_c*>	map_brushes;
std::vector<map_brush_side_c*>	map_brushsides;
std::vector<cplane_c*>		map_planes;


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

static void	BSP_f()
{
	if(Cmd_Argc() != 2)
	{
		Com_Printf("usage : bsp <levelname>\n");
		return;
	}
	
	std::string filename = "maps/" + std::string(Cmd_Argv(1)) + ".map";
	
	//TODO
	
	LoadMapFile(filename);
	//SetModelNumbers();
}


void	Map_Init()
{
	Com_Printf("------- Map_Init -------\n");

	Cmd_AddCommand("bspinfo",		BSPInfo_f);
	Cmd_AddCommand("bsp",			BSP_f);
	
	Cmd_AddCommand("shaderlist",		Map_ShaderList_f);
	Cmd_AddCommand("shadercachelist",	Map_ShaderCacheList_f);
	Cmd_AddCommand("shadersearch",		Map_ShaderSearch_f);
	
	Map_InitShaders();
	
	//TODO
}

void	Map_Shutdown()
{
	Com_Printf("------- Map_Shutdown -------\n");
	
	Map_ShutdownShaders();
	
	//TODO
}

void	Map_Frame(int msec)
{
	//DO NOTHING
}

