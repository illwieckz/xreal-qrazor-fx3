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
#include "vfs.h"


/*
============================================================

USER STRINGCMD EXECUTION

sv_client and sv_player will be valid.
============================================================
*/


/*
================
SV_New_f

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
void	sv_client_c::new_uc()
{
	const char		*gamedir;
	int			playernum;

	Com_DPrintf("sv_client_c::new_uc() from '%s'\n", _name);

	if(_state != CS_CONNECTED)
	{
		Com_Printf("New not valid -- already spawned\n");
		return;
	}

	//
	// serverdata needs to go over for all types of servers
	// to make sure the protocol is right, and to set the gamedir
	//
	gamedir = Cvar_VariableString("vfs_game");

	// send the serverdata
	netchan.message.writeBits(SVC_SERVERDATA, svc_bitcount);
	netchan.message.writeLong(PROTOCOL_VERSION);
	netchan.message.writeLong(svs.spawncount);
	netchan.message.writeByte(sv.attractloop);
	netchan.message.writeString(gamedir);

	playernum = SV_GetNumForClient(this);
		
	netchan.message.writeShort(playernum);

	// send full levelname
	netchan.message.writeString(sv.configstrings[CS_MAPNAME]);


	//
	// game server
	// 
	if(sv.state == SS_GAME)
	{
		// set up the entity for the client
		sv_entity_c *ent = SV_GetEntityByNum(playernum+1);
		_entity = ent;
		_lastcmd.clear();

		// begin fetching configstrings
		netchan.message.writeBits(SVC_STUFFTEXT, svc_bitcount);
		netchan.message.writeString(va("cmd configstrings %i 0\n", svs.spawncount));
	}
}

void 	sv_client_c::configStrings_uc()
{
	int			start;

	Com_DPrintf("sv_client_c::configStrings_uc() from '%s'\n", _name);

	if(_state != CS_CONNECTED)
	{
		Com_Printf("configstrings not valid -- already spawned\n");
		return;
	}

	// handle the case of a level changing while a client was connecting
	if(atoi(Cmd_Argv(1)) != svs.spawncount)
	{
		Com_Printf("sv_client_c::configStrings_uc from different level\n");
		new_uc();
		return;
	}
	
	start = atoi(Cmd_Argv(2));

	// write a packet full of data
	while(netchan.message.getCurSize() < (MAX_PACKETLEN/2)*8 && start < MAX_CONFIGSTRINGS)
	{
		if(sv.configstrings[start][0])
		{
			netchan.message.writeBits(SVC_CONFIGSTRING, svc_bitcount);
			netchan.message.writeShort(start);
			netchan.message.writeString(sv.configstrings[start]);
		}
		start++;
	}

	// send next command
	if(start == MAX_CONFIGSTRINGS)
	{
		netchan.message.writeBits(SVC_STUFFTEXT, svc_bitcount);
		netchan.message.writeString(va("cmd baselines %i 0\n", svs.spawncount));
	}
	else
	{
		netchan.message.writeBits(SVC_STUFFTEXT, svc_bitcount);
		netchan.message.writeString(va("cmd configstrings %i %i\n", svs.spawncount, start));
	}
}

void	sv_client_c::baseLines_uc()
{
	Com_DPrintf("sv_client_c::baseLines_uc() from '%s'\n", _name);

	if(_state != CS_CONNECTED)
	{
		Com_Printf("baselines not valid -- already spawned\n");
		return;
	}
	
	// handle the case of a level changing while a client was connecting
	if(atoi(Cmd_Argv(1)) != svs.spawncount )
	{
		Com_Printf("sv_client_c::baseLines_uc from different level\n");
		new_uc();
		return;
	}
	
	int start = atoi(Cmd_Argv(2));

	// write a packet full of data
	while(netchan.message.getCurSize() < (MAX_PACKETLEN/2)*8 && start < MAX_ENTITIES)
	{
		const entity_state_t& base = sv.baselines[start];
		
		if(	base.index_model ||
			base.index_animation ||
			base.index_sound ||
			base.index_light ||
			base.effects
		)
		{
			netchan.message.writeBits(SVC_SPAWNBASELINE, svc_bitcount);
			netchan.message.writeDeltaEntity(&null_entity_state, &base, true);
		}
		
		start++;
	}

	// send next command
	if(start == MAX_ENTITIES)
	{
		netchan.message.writeBits(SVC_STUFFTEXT, svc_bitcount);
		netchan.message.writeString(va("precache %i\n", svs.spawncount));
	}
	else
	{
		netchan.message.writeBits(SVC_STUFFTEXT, svc_bitcount);
		netchan.message.writeString(va("cmd baselines %i %i\n", svs.spawncount, start));
	}
}

void	sv_client_c::begin_uc()
{
	Com_DPrintf("sv_client_c::begin_uc() from '%s'\n", _name);

	// handle the case of a level changing while a client was connecting
	if(atoi(Cmd_Argv(1)) != svs.spawncount )
	{
		Com_Printf("sv_client_c::begin_uc from different level\n");
		new_uc();
		return;
	}

	_state = CS_SPAWNED;
	
	// call the game begin function
	ge->G_ClientBegin(_entity);

	Cbuf_InsertFromDefer();
}


void 	sv_client_c::nextDownload_uc()
{
	int		r;
	int		percent;
	int		size;

	if (!_download)
		return;

	r = _download_size - _download_count;
	if (r > 1024)
		r = 1024;

	netchan.message.writeBits(SVC_DOWNLOAD, svc_bitcount);
	netchan.message.writeShort(r);

	_download_count += r;
	size = _download_size;
	if (!size)
		size = 1;
	percent = _download_count*100/size;
	netchan.message.writeByte( percent);
	netchan.message.writeBytes(_download + _download_count - r, r);

	if (_download_count != _download_size)
		return;

	VFS_FFree (_download);
	_download = NULL;
}

void	sv_client_c::beginDownload_uc()
{
	const char	*name;
	int offset = 0;

	name = Cmd_Argv(1);

	if (Cmd_Argc() > 2)
		offset = atoi(Cmd_Argv(2)); // downloaded offset

	// hacked by zoid to allow more conrol over download
	// first off, no .. or global allow check
	if (strstr (name, "..") || !sv_allow_download->getInteger()
		
		// leading dot is no good
		|| *name == '.' 
		
		// leading slash bad as well, must be in subdir
		|| *name == '/'
		
		// next up, skin check
		|| (strncmp(name, "players/", 6) == 0 && !sv_allow_download_players->getInteger())
		
		// now models
		|| (strncmp(name, "models/", 6) == 0 && !sv_allow_download_models->getInteger())
		
		// now sounds
		|| (strncmp(name, "sound/", 6) == 0 && !sv_allow_download_sounds->getInteger())
		
		// now maps (note special case for maps, must not be in pak)
		|| (strncmp(name, "maps/", 6) == 0 && !sv_allow_download_maps->getInteger())
		
		// MUST be in a subdirectory	
		|| !strstr(name, "/") )	
	{	
		// don't allow anything with .. path
		netchan.message.writeBits(SVC_DOWNLOAD, svc_bitcount);
		netchan.message.writeShort(-1);
		netchan.message.writeByte(0);
		return;
	}


	if(_download)
		VFS_FFree(_download);

	_download_size = VFS_FLoad(name, (void **)&_download);
	_download_count = offset;

	if (offset > _download_size)
		_download_count = _download_size;


	if (!_download
		// special check for maps, if it came from a pak file, don't allow
		// download  ZOID
		
		// FIXME
		/*|| (strncmp(name, "maps/", 5) == 0 && VFS_IsPak(_download)*/)
	{
		Com_DPrintf ("Couldn't download %s to %s\n", name, _name);
		
		if (_download)
		{
			VFS_FFree (_download);
			_download = NULL;
		}

		netchan.message.writeBits(SVC_DOWNLOAD, svc_bitcount);
		netchan.message.writeShort(-1);
		netchan.message.writeByte(0);
		return;
	}

	nextDownload_uc();
	
	Com_DPrintf("Downloading '%s' to '%s'\n", name, _name);
}



/*
=================
SV_Disconnect_f

The client is going to disconnect, so remove the connection immediately
=================
*/
void 	sv_client_c::disconnect_uc()
{
	drop();	
}


/*
==================
SV_ShowServerinfo_f

Dumps the serverinfo info string
==================
*/
void 	sv_client_c::showServerInfo_uc()
{
	Com_PrintInfo(Cvar_Serverinfo());
}


void 	sv_client_c::nextServer()
{
	const char	*v;

	//ZOID, ss_pic can be nextserver'd in coop mode
	if(sv.state == SS_GAME)
		return;		// can't nextserver while playing a normal game

	svs.spawncount++;	// make sure another doesn't sneak in
	v = Cvar_VariableString("nextserver");
	if(!v[0])
		Cbuf_AddText("killserver\n");
	else
	{
		Cbuf_AddText(v);
		Cbuf_AddText("\n");
	}
	
	Cvar_Set("nextserver","");
}

/*
==================
SV_Nextserver_f

A cinematic has completed or been aborted by a client, so move
to the next server,
==================
*/
void 	sv_client_c::nextServer_uc()
{
	if(atoi(Cmd_Argv(1)) != svs.spawncount)
	{
		Com_DPrintf("sv_client_c::nextServer() from wrong level, from '%s'\n", _name);
		return;		// leftover from last server
	}

	Com_DPrintf("sv_client_c::nextServer_uc() from '%s'\n", _name);

	nextServer();
}

/*
static ucmd_t ucmds[] =
{
	// auto issued
	{"new",			SV_New_uc},
	{"configstrings",	SV_Configstrings_uc},
	{"baselines",		SV_Baselines_uc},
	{"begin",		SV_Begin_uc},

	{"nextserver",		SV_Nextserver_uc},

	{"disconnect",		SV_Disconnect_uc},

	// issued by hand at client consoles	
	{"info",		SV_ShowServerinfo_uc},

	{"download",		SV_BeginDownload_uc},
	{"nextdl",		SV_NextDownload_uc},

	{NULL, NULL}
};
*/

void 	sv_client_c::executeCommand(const char *s)
{
	Cmd_TokenizeString(s);
	
	const char* cmd = Cmd_Argv(0);
	
	if(X_strequal(cmd, "new"))
		new_uc();
	
	else if(X_strequal(cmd, "configstrings"))
		configStrings_uc();
		
	else if(X_strequal(cmd, "baselines"))
		baseLines_uc();
	
	else if(X_strequal(cmd, "begin"))
		begin_uc();
	
	else if(X_strequal(cmd, "nextserver"))
		nextServer_uc();
		
	else if(X_strequal(cmd, "disconnect"))
		disconnect_uc();
		
	else if(X_strequal(cmd, "info"))
		showServerInfo_uc();
		
	else if(X_strequal(cmd, "download"))
		beginDownload_uc();
		
	else if(X_strequal(cmd, "nextdl"))
		nextDownload_uc();
	
	else if(sv.state == SS_GAME)
		ge->G_ClientCommand(_entity);
}



void	sv_client_c::think(const usercmd_t &cmd)
{
	_command_time -= cmd.msec;

	if(_command_time < 0 && sv_time_enforce->getInteger())
	{
		Com_DPrintf("SV_ClientThink: command time underflow from '%s'\n", _name);
		return;
	}

	ge->G_ClientThink(_entity, cmd);
}


