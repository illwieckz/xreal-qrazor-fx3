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
#ifndef CLIENT_H
#define CLIENT_H

/// includes ===================================================================
// system -------------------------------------------------------------------
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// shared -------------------------------------------------------------------
#include "x_protocol.h"

// qrazor-fx ----------------------------------------------------------------
#include "r_public.h"
#include "s_public.h"

#include "vid.h"
#include "input.h"

#include "cl_keys.h"
#include "cl_console.h"
#include "cl_screen.h"

#include "net_chan.h"
#include "files.h"

// xreal --------------------------------------------------------------------
#include "xreal/cg_public.h"
#include "xreal/ui_public.h"




enum download_type_e
{
	DL_NONE,
	DL_MODEL,
	DL_SOUND,
	DL_SKIN,
	DL_SINGLE
};


//
// the client_state_t structure is wiped completely at every
// server map change
//
struct client_state_t
{
	int		timeoutcount;

	int		timedemo_frames;
	int		timedemo_start;

	bool		refresh_prepped;	// false if on new level or new ref dll
	bool		sound_prepped;		// ambient sounds can start
	bool		force_refdef;		// vid has changed, so we can't use a paused refdef
	
	std::vector<entity_state_t>	baselines;
	entity_state_t	entities_parse[MAX_PARSE_ENTITIES];
	int		entities_parse_index;

	int		cmds_num;
	usercmd_t	cmds[CMD_BACKUP];	// each mesage will send several old cmds
	int		cmd_time[CMD_BACKUP];	// time sent, for calculating pings

	frame_t		frame;				// received from server
	int		surpress_count;		// number of messages rate supressed
	frame_t		frames[UPDATE_BACKUP];

	// the client maintains its own idea of view angles, which are
	// sent to the server each frame.  It is cleared to 0 upon entering each level.
	// the server sends a delta each frame which is added to the locally
	// tracked view angles to account for standing on rotating objects,
	// and teleport direction changes
	vec3_c		viewangles;

	int		time;			// this is the time value that the client
								// is rendering at.  always <= cls.realtime

	
	//
	// server state information
	//
	bool		attractloop;		// running the attract loop, any key will menu
	int		servercount;		// server identification for prespawns
	int		playernum;

	char		configstrings[MAX_CONFIGSTRINGS][MAX_QPATH];
};


//
// the client_static_t structure is persistant through an arbitrary number
// of server connections
//
struct client_static_t
{
	connection_state_t	state;
	keydest_t		key_dest;

	int			realtime;			// always increasing, no clamping, etc
	int			frametime;			// milliseconds since last frame
	int			framecount;

	// screen rendering information
	float			disable_screen;			// showing loading plaque between levels
								// or changing rendering dlls
								// if time gets > 30 seconds ahead, break it
	int			disable_servercount;		// when we receive a frame and cl.servercount
								// > cls.disable_servercount, clear disable_screen

	// connection information
	char			servername[MAX_OSPATH];		// name of server from original connect
	netadr_t		serveraddress;
	float			connect_time;			// for connection retransmits

	int			quake_port;			// a 16 bit value that allows quake servers
								// to work around address translating routers
	netchan_c		netchan;
	int			server_protocol;		// in case we are doing some kind of version hack

	int			challenge;			// from the server to use for connecting

	VFILE*			download_stream;		// file transfer from server
	std::string		download_tempname;
	std::string		download_name;
	int			download_number;
	download_type_e		download_type;
	int			download_percent;

	// demo recording info must be here, so it isn't cleared on level change
	bool			demo_recording;
	bool			demo_waiting;			// don't record until a non-delta message is received
	VFILE*			demo_stream;
};



extern client_state_t		cl;
extern client_static_t		cls;



//
// cvars
//
extern	cvar_t	*cl_shownet;
extern	cvar_t	*cl_timedemo;



bool	CL_CheckOrDownloadFile(char *filename);



//
// cl_cgame.c -- client game module handling
//
extern	cg_export_t	*cge;

void	CL_InitClientGame();
void	CL_ShutdownClientGame();




//
// cl_main
//
extern	ref_export_t	re;		// interface to refresh .dll


void 	CL_FixUpGender();
void 	CL_Disconnect();
void 	CL_GetChallengePacket();
void 	CL_PingServers_f();
void 	CL_RequestNextDownload();
void	CL_Quit_f();

//
// cl_netchan
//
void	CL_InitNetchan();
void	CL_SendPacket();


//
// cl_input
//
struct kbutton_t
{
	int		down[2];		// key nums holding it down
	int		state;
};

extern 	kbutton_t 	in_strafe;
extern 	kbutton_t 	in_speed;

void 	CL_InitInput();
void	CL_MouseEvent(int dx, int dy);
void 	CL_SendCmd();

void 	CL_ClearState();

int  	CL_ReadFromServer();
void 	CL_WriteToServer(usercmd_t *cmd);

void 	IN_CenterView();




//
// cl_demo.c
//
void	CL_WriteDemoMessage(bitmessage_c &msg);
void	CL_Stop_f();
void	CL_Record_f();


//
// cl_parse.c
//
void	CL_ParseServerMessage(bitmessage_c &msg);
void	CL_Download_f();


//
// cl_ui.c
//
extern ui_export_t	*uie;

void	CL_InitUserInterface();
void	CL_ShutdownUserInterface();


#endif // CL_LOCAL_H


