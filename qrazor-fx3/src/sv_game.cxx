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
#include "sv_local.h"

#include "cmd.h"
#include "cvar.h"
#include "cm.h"
#include "vfs.h"

game_export_t	*ge;

static void*	ge_handle = NULL;

/*
===============
SV_Unicast

Sends the contents of the mutlicast buffer to a single client
===============
*/
static void 	SV_Unicast(sv_entity_c *ent, bool reliable)
{
	int		p;
	sv_client_c	*client;

	if(!ent)
		return;

	p = SV_GetNumForEntity(ent);

	if(p < 1 || p > maxclients->getInteger())
		return;

	client = svs.clients[p-1];

	if(reliable)
		client->netchan.message.write(&sv.multicast[0], sv.multicast.getCurSize());
	else
		client->getDatagram()->write(&sv.multicast[0], sv.multicast.getCurSize());

	sv.multicast.clear();
}





/*
===============
SV_CPrintf

Print to a single client
===============
*/
static void 	SV_CPrintf(sv_entity_c *ent, g_print_level_e level, const char *fmt, ...)
{
	char		msg[1024];
	va_list		argptr;
	uint_t		n = 0;

	if(ent)
	{
		n = SV_GetNumForEntity(ent);
		
		if(n < 1 || n > svs.clients.size())
			Com_Error(ERR_DROP, "SV_CPrintf to a non-client");
			
		if(!svs.clients[n-1])
			Com_Error(ERR_DROP, "SV_CPrintf to a NULL client");
	}

	va_start(argptr,fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	if(ent && !ent->_r.isbot)
		svs.clients[n-1]->printf(level, "%s", msg);
	else
		Com_Printf("%s", msg);
}


/*
===============
SV_CenterPrintf

centerprint to a single client
===============
*/
static void 	SV_CenterPrintf(sv_entity_c *ent, const char *fmt, ...)
{
	char		msg[1024];
	va_list		argptr;
	uint_t		n;
	
	if(ent && ent->_r.isbot)
		return;
	
	n = SV_GetNumForEntity(ent);
	if(n < 1 || n > svs.clients.size())
		return;	// Com_Error (ERR_DROP, "centerprintf to a non-client");

	va_start(argptr,fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	sv.multicast.writeByte(SVC_CENTERPRINT);
	sv.multicast.writeString(msg);
	
	SV_Unicast(ent, true);
}




void 	SV_SetConfigString(int index, const std::string &val)
{
	if(index < 0 || index >= MAX_CONFIGSTRINGS)
		Com_Error(ERR_DROP, "SV_Configstring: bad index %i\n", index);

	//if (!val.length())
	//	val = "";

	// change the string in sv
	strcpy(sv.configstrings[index], val.c_str());
	
	if(sv.state != SS_LOADING)
	{	
		// send the update to everyone
		sv.multicast.clear();
		sv.multicast.writeByte(SVC_CONFIGSTRING);
		sv.multicast.writeShort(index);
		sv.multicast.writeString(val.c_str());

		SV_Multicast(vec3_origin, MULTICAST_ALL_R);
	}
}



static void 	SV_WriteByte(int c) 		{sv.multicast.writeByte(c);}
static void 	SV_WriteShort(int c) 		{sv.multicast.writeShort(c);}
static void 	SV_WriteLong(int c) 		{sv.multicast.writeLong(c);}
static void 	SV_WriteFloat(float f) 		{sv.multicast.writeFloat(f);}
static void 	SV_WriteString(const char *s) 	{sv.multicast.writeString(s);}
static void 	SV_WritePos(const vec3_c &pos) 	{sv.multicast.writeVector3(pos);}
static void 	SV_WriteDir(const vec3_c &dir) 	{sv.multicast.writeDir(dir);}
static void 	SV_WriteAngle(float f)		{sv.multicast.writeAngle(f);}
static void 	SV_WriteColor(vec4_t color) 	{sv.multicast.writeColor(color);}


/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
/*
static bool 	SV_InPVS(const vec3_c &p1, const vec3_c &p2)
{
	//int		leafnum;
	//int		cluster;
	int		area1, area2;
	//byte	*mask;

	//leafnum = CM_PointLeafnum(p1);
	//cluster = CM_LeafCluster(leafnum);
	//area1 = CM_LeafArea(leafnum);
	//mask = CM_ClusterPVS(cluster);
	area1 = CM_PointAreanum(p1);

	//leafnum = CM_PointLeafnum(p2);
	//cluster = CM_LeafCluster(leafnum);
	//area2 = CM_LeafArea(leafnum);
	area2 = CM_PointAreanum(p2);
	
	//if(mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
	//	return false;
	
	if(!CM_AreasConnected(area1, area2))
		return false;		// a door blocks sight
	
	return true;
}
*/


/*
===============
SV_ShutdownGameProgs

Called when either the entire server is being killed, or
it is changing to a different game directory.
===============
*/
void 	SV_ShutdownGameProgs()
{
	if(!ge)
		return;
		
	ge->G_ShutdownGame();
	
	Sys_UnloadAPI(&ge_handle);
}

/*
===============
SV_InitGameProgs

Init the game subsystem for a new map
===============
*/


void 	SV_InitGameProgs()
{
	game_import_t	import;

	// unload anything we have now
	if(ge)
		SV_ShutdownGameProgs();


	// load a new game dll
	import.Com_Printf		= Com_Printf;
	import.Com_DPrintf		= Com_DPrintf;
	import.Com_Error		= Com_Error;
	
	import.Cbuf_AddText		= Cbuf_AddText;
	
	import.Cmd_AddCommand		= Cmd_AddCommand;
	import.Cmd_RemoveCommand	= Cmd_RemoveCommand;
	import.Cmd_Argc			= Cmd_Argc;
	import.Cmd_Argv			= Cmd_Argv;
	import.Cmd_Args			= Cmd_Args;

	import.Cvar_Get 		= Cvar_Get;
	import.Cvar_Set 		= Cvar_Set;
	import.Cvar_ForceSet 		= Cvar_ForceSet;
	import.Cvar_SetValue		= Cvar_SetValue;
	
	import.SV_BPrintf		= SV_BroadcastPrintf;
	import.SV_CPrintf		= SV_CPrintf;
	import.SV_CenterPrintf		= SV_CenterPrintf;
	import.SV_StartSound		= SV_StartSound;
	
	import.SV_SetConfigString	= SV_SetConfigString;
	
	import.SV_ModelIndex 		= SV_ModelIndex;
	import.SV_ShaderIndex 		= SV_ShaderIndex;
	import.SV_AnimationIndex 	= SV_AnimationIndex;
	import.SV_SoundIndex 		= SV_SoundIndex;
	import.SV_LightIndex		= SV_LightIndex;
	
	import.CM_BeginRegistration	= CM_BeginRegistration;
	import.CM_RegisterModel		= CM_RegisterModel;
	import.CM_RegisterAnimation	= CM_RegisterAnimation;
	import.CM_GetModelByNum		= CM_GetModelByNum;
	import.CM_LeafContents		= CM_LeafContents;
	import.CM_LeafCluster		= CM_LeafCluster;
	import.CM_LeafArea		= CM_LeafArea;
	import.CM_NumModels		= CM_NumModels;
	import.CM_HeadnodeForBox	= CM_HeadnodeForBox;
	import.CM_PointContents		= CM_PointContents;
	import.CM_TransformedPointContents	= CM_TransformedPointContents;
	import.CM_BoxTrace		= CM_BoxTrace;
	import.CM_TransformedBoxTrace	= CM_TransformedBoxTrace;
	import.CM_PointLeafnum		= CM_PointLeafnum;
	import.CM_PointAreanum		= CM_PointAreanum;
	import.CM_BoxLeafnums		= CM_BoxLeafnums;
	import.CM_GetClosestAreaPortal	= CM_GetClosestAreaPortal;
	import.CM_GetAreaPortalState 	= CM_GetAreaPortalState;
	import.CM_SetAreaPortalState 	= CM_SetAreaPortalState;
	import.CM_AreasConnected 	= CM_AreasConnected;
			
	import.SV_Multicast 		= SV_Multicast;
	import.SV_Unicast 		= SV_Unicast;
	
	import.SV_WriteByte 		= SV_WriteByte;
	import.SV_WriteShort 		= SV_WriteShort;
	import.SV_WriteLong 		= SV_WriteLong;
	import.SV_WriteFloat 		= SV_WriteFloat;
	import.SV_WriteString 		= SV_WriteString;
	import.SV_WritePosition 	= SV_WritePos;
	import.SV_WriteDir 		= SV_WriteDir;
	import.SV_WriteAngle 		= SV_WriteAngle;
	import.SV_WriteColor		= SV_WriteColor;

        import.VFS_FOpenRead		= VFS_FOpenRead;
        import.VFS_FOpenWrite		= VFS_FOpenWrite;
        import.VFS_FClose		= VFS_FClose;

	import.VFS_FLoad		= VFS_FLoad;
	import.VFS_FSave		= VFS_FSave;
	import.VFS_FFree		= VFS_FFree;
	
	import.VFS_FRead		= VFS_FRead;
	import.VFS_FWrite		= VFS_FWrite;
	
	

	ge = (game_export_t*) Sys_GetAPI("game", "GetGameAPI", &import, &ge_handle);

	if(!ge)
		Com_Error(ERR_DROP, "failed to load game DLL");
	
	if(ge->apiversion != GAME_API_VERSION)
		Com_Error(ERR_DROP, "game is version %i, not %i", ge->apiversion, GAME_API_VERSION);

	ge->G_InitGame();
}

