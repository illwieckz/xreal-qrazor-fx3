/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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
#include "cl_local.h"

#include "cmd.h"
#include "cvar.h"
#include "cm.h"
#include "vfs.h"
#include "sys.h"




cvar_t	*adr0;
cvar_t	*adr1;
cvar_t	*adr2;
cvar_t	*adr3;
cvar_t	*adr4;
cvar_t	*adr5;
cvar_t	*adr6;
cvar_t	*adr7;
cvar_t	*adr8;

cvar_t	*rcon_client_password;
cvar_t	*rcon_address;


cvar_t	*cl_timeout;
cvar_t	*cl_maxfps;

cvar_t	*cl_shownet;

cvar_t	*cl_timedemo;






//
// userinfo
//
static cvar_t	*info_password;
static cvar_t	*info_spectator;
static cvar_t	*name;
static cvar_t	*model;
static cvar_t	*skin;
static cvar_t	*rate;
static cvar_t	*fov;
static cvar_t	*msg;
static cvar_t	*hand;
static cvar_t	*gender;
static cvar_t	*gender_auto;


client_static_t	cls;
client_state_t	cl;




//static cvar_t *cl_allow_download;
//static cvar_t *cl_allow_download_players;
//static cvar_t *cl_allow_download_models;
//static cvar_t *cl_allow_download_sounds;
//static cvar_t *cl_allow_download_maps;

/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length
====================
*/
void	CL_WriteDemoMessage(bitmessage_c &msg)
{
#if 0
	//FIXME
	int		len, swlen;

	// the first eight bytes are just packet sequencing stuff
	len = msg.getCurSize() - 8;
	swlen = LittleLong(len);
	
	VFS_FWrite(&swlen, 4, cls.demo_stream);
	VFS_FWrite(&msg[8], len, cls.demo_stream);
#endif
}


/*
====================
CL_Stop_f

stop recording a demo
====================
*/
void	CL_Stop_f()
{
	int		len;

	if(!cls.demo_recording)
	{
		Com_Printf("Not recording a demo.\n");
		return;
	}

	// finish up
	len = -1;
	VFS_FWrite(&len, 4, cls.demo_stream);
	VFS_FClose(&cls.demo_stream);
	cls.demo_recording = false;
	Com_Printf("Stopped demo.\n");
}

/*
====================
CL_Record_f

record <demoname>

Begins recording a demo from the current position
====================
*/
void	CL_Record_f()
{
	/*
	char	name[MAX_OSPATH];
	char	buf_data[MAX_MSGLEN];
	message_c	buf;
	int		i;
	int		len;
	entity_state_c	*ent;
	entity_state_c	nullstate;

	if(Cmd_Argc() != 2)
	{
		Com_Printf("record <demoname>\n");
		return;
	}

	if(cls.demo_recording)
	{
		Com_Printf("Already recording.\n");
		return;
	}

	if(cls.state != CA_ACTIVE)
	{
		Com_Printf("You must be in a level to record.\n");
		return;
	}

	//
	// open the demo file
	//
	Com_sprintf (name, sizeof(name), "demos/%s.dm2", Cmd_Argv(1));

	Com_Printf ("recording to %s.\n", name);
	VFS_Mkdir (name);
	//cls.demofile = fopen (name, "wb");
	
	VFS_FOpenWrite(name, &cls.demo_stream);
	if(!cls.demo_stream)
	{
		Com_Printf("ERROR: couldn't open.\n");
		return;
	}
	cls.demo_recording = true;

	// don't start saving messages until a non-delta compressed message is received
	cls.demo_waiting = true;


	//
	// write out messages to hold the startup information
	//
	MSG_Init(&buf, (byte*)buf_data, sizeof(buf_data));

	// send the serverdata
	MSG_WriteByte(&buf, SVC_SERVERDATA);
	MSG_WriteLong(&buf, PROTOCOL_VERSION);
	MSG_WriteLong(&buf, 0x10000 + cl.servercount);
	MSG_WriteByte(&buf, 1);	// demos are always attract loops
	MSG_WriteString(&buf, cl.gamedir);
	MSG_WriteShort(&buf, cl.playernum);

	MSG_WriteString(&buf, cl.configstrings[CS_NAME]);

	// configstrings
	for(i=0; i<MAX_CONFIGSTRINGS; i++)
	{
		if(cl.configstrings[i][0])
		{
			if(buf.cursize + (int)strlen (cl.configstrings[i]) + 32 > buf.maxsize)
			{
				// write it out
				len = LittleLong(buf.cursize);
				VFS_FWrite(&len, 4, cls.demo_stream);
				VFS_FWrite(buf.data, buf.cursize, cls.demo_stream);
				buf.cursize = 0;
			}

			MSG_WriteByte(&buf, SVC_CONFIGSTRING);
			MSG_WriteShort(&buf, i);
			MSG_WriteString(&buf, cl.configstrings[i]);
		}

	}

	// baselines
	memset(&nullstate, 0, sizeof(nullstate));
	for(i=0; i<MAX_EDICTS ; i++)
	{
		ent = &cl_entities[i].baseline;
		
		if(!ent->modelindex)
			continue;

		if (buf.cursize + 64 > buf.maxsize)
		{	// write it out
			len = LittleLong (buf.cursize);
			VFS_FWrite (&len, 4, cls.demo_stream);
			VFS_FWrite (buf.data, buf.cursize, cls.demo_stream);
			buf.cursize = 0;
		}

		MSG_WriteByte(&buf, SVC_SPAWNBASELINE);		
		MSG_WriteDeltaEntity(&nullstate, &cl_entities[i].baseline, &buf, true, true);
	}

	MSG_WriteByte(&buf, SVC_STUFFTEXT);
	MSG_WriteString(&buf, "precache\n");

	// write it to the demo file

	len = LittleLong (buf.cursize);
	VFS_FWrite (&len, 4, cls.demo_stream);
	VFS_FWrite (buf.data, buf.cursize, cls.demo_stream);

	// the rest of the demo file will be individual frames
	*/
}



/*
===================
Cmd_ForwardToServer

adds the current command line as a CLC_STRINGCMD to the client message.
things like godmode, noclip, etc, are commands directed to the server,
so when they are typed in at the console, they will need to be forwarded.
===================
*/
void	Cmd_ForwardToServer()
{
	std::string cmd = Cmd_Argv(0);
	
	if(cls.state <= CA_CONNECTED || cmd[0] == '-' || cmd[0] == '+')
	{
		Com_Printf("Unknown command \"%s\"\n", cmd.c_str());
		return;
	}

	if(Cmd_Argc() > 1)
	{
		cmd += " ";
		cmd += Cmd_Args();
	}
	
	cls.netchan.message.writeByte(CLC_STRINGCMD);
	cls.netchan.message.writeString(cmd);
}

void	CL_ForwardToServer_f()
{
	if(cls.state != CA_CONNECTED && cls.state != CA_ACTIVE)
	{
		Com_Printf("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}
	
	// don't forward the first argument
	if(Cmd_Argc() > 1)
	{
		cls.netchan.message.writeByte(CLC_STRINGCMD);
		cls.netchan.message.writeString(Cmd_Args());
	}
}

void	CL_Setenv_f()
{
	int argc = Cmd_Argc();

	if(argc > 2)
	{
		char buffer[1000];
		int i;

		strcpy( buffer, Cmd_Argv(1) );
		strcat( buffer, "=" );

		for ( i = 2; i < argc; i++ )
		{
			strcat( buffer, Cmd_Argv( i ) );
			strcat( buffer, " " );
		}

		putenv( buffer );
	}
	else if ( argc == 2 )
	{
		char *env = getenv( Cmd_Argv(1) );

		if ( env )
		{
			Com_Printf( "%s=%s\n", Cmd_Argv(1), env );
		}
		else
		{
			Com_Printf( "%s undefined\n", Cmd_Argv(1), env );
		}
	}
}

void	CL_Quit_f()
{
	CL_Disconnect();
	Com_Shutdown();
}

/*
================
CL_Drop

Called after an ERR_DROP was thrown
================
*/
void	CL_Drop()
{
	if(cls.state == CA_UNINITIALIZED)
		return;
		
	if(cls.state == CA_DISCONNECTED)
		return;

	CL_Disconnect();

	// drop loading plaque unless this is the initial game start
	if(cls.disable_servercount != -1)
		SCR_EndLoadingPlaque();	// get rid of loading plaque
}


/*
=======================
CL_SendConnectPacket

We have gotten a challenge from the server, so try and
connect.
======================
*/
void	CLC_SendConnectPacket()
{
	Com_DPrintf("CLC_SendConnectPacket()\n");

	netadr_t	adr;
	int		port;

	if(!Sys_StringToAdr(cls.servername, adr))
	{
		Com_Printf("Bad server address\n");
		cls.connect_time = 0;
		return;
	}
	
	if(adr.port == 0)
		adr.port = BigShort(PORT_SERVER);

	port = Cvar_VariableInteger("qport");
	cvar_userinfo_modified = false;

	//Netchan_OutOfBandPrint(adr, "connect %i %i %i \"%s\"\n", PROTOCOL_VERSION, port, cls.challenge, Cvar_Userinfo());
	
	bitmessage_c msg(MAX_PACKETLEN*8);
	msg.writeLong(-1);	// -1 sequence means out of band
	msg.writeString(va("connect %i %i %i \"%s\"", PROTOCOL_VERSION, port, cls.challenge, Cvar_Userinfo()));
	
	//Huff_CompressPacket(&msg, 12);
	
	Sys_SendPacket(msg, adr);
}

/*
=================
CL_CheckForResend

Resend a connect message if the last one has timed out
=================
*/
void	CL_CheckForResend()
{
	netadr_t	adr;

	/*
	// if the local server is running and we aren't then connect
	if(cls.state == CA_DISCONNECTED && Com_ServerState())
	{
		cls.state = CA_CONNECTING;
		strncpy (cls.servername, "localhost", sizeof(cls.servername)-1);
		// we don't need a challenge on the localhost
		CL_SendConnectPacket();
		return;
//		cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
	}
	*/

	// resend if we haven't gotten a reply yet
	if(cls.state != CA_CONNECTING)
		return;

	if(cls.realtime - cls.connect_time < 3000)
		return;

	if(!Sys_StringToAdr(cls.servername, adr))
	{
		Com_Printf("Bad server address\n");
		cls.state = CA_DISCONNECTED;
		return;
	}
	
	if(adr.port == 0)
		adr.port = BigShort(PORT_SERVER);

	cls.connect_time = cls.realtime;	// for retransmit requests

	Com_Printf("Connecting to %s...\n", cls.servername);

	Netchan_OutOfBandPrint(adr, "getchallenge\n");
}


void	CL_Connect_f()
{
	const char	*server;

	if(Cmd_Argc() != 2)
	{
		Com_Printf("usage: connect <server>\n");
		return;	
	}
	
	if(Com_ServerState())
	{
		// if running a local server, kill it and reissue
		SV_Shutdown(va("Server quit\n", msg), false);
	}
	else
	{
		CL_Disconnect();
	}

	server = Cmd_Argv(1);
	
	CL_Disconnect();

	cls.state = CA_CONNECTING;
	strncpy(cls.servername, server, sizeof(cls.servername)-1);
	cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
}


/*
=====================
CL_Rcon_f

  Send the rest of the command line over as
  an unconnected command.
=====================
*/
void	CL_Rcon_f()
{
#if 0
	//TODO

	char	message[1024];
	int		i;
	netadr_t	to;

	if(!X_strequal(rcon_client_password->getString(), ""))
	{
		Com_Printf("You must set 'rcon_password' before\n"
					"issuing an rcon command.\n");
		return;
	}

	message[0] = (char)255;
	message[1] = (char)255;
	message[2] = (char)255;
	message[3] = (char)255;
	message[4] = 0;

	strcat(message, "rcon ");

	strcat(message, rcon_client_password->getString());
	strcat(message, " ");

	for(i=1; i<Cmd_Argc(); i++)
	{
		strcat(message, Cmd_Argv(i));
		strcat(message, " ");
	}

	if(cls.state >= CA_CONNECTED)
		to = cls.netchan.getRemoteAddress();
	else
	{
		if(!X_strequal(rcon_address->getString(), ""))
		{
			Com_Printf ("You must either be connected,\n"
						"or set the 'rcon_address' cvar\n"
						"to issue rcon commands\n");

			return;
		}
		
		Sys_StringToAdr(rcon_address->getString(), to);
		
		if(to.port == 0)
			to.port = BigShort (PORT_SERVER);
	}
	
	Sys_SendPacket(message, strlen(message)+1, to);
#endif
}


static void	CL_ClearClientState()
{
	// wipe the entire cl structure
	//memset(&cl, 0, sizeof(cl));
	
	cl.timeoutcount			= 0;

	cl.timedemo_frames		= 0;
	cl.timedemo_start		= 0;

	cl.refresh_prepped		= false;	// false if on new level or new ref dll
	cl.sound_prepped		= false;	// ambient sounds can start
	cl.force_refdef			= false;	// vid has changed, so we can't use a paused refdef
	
	cl.baselines			= std::vector<entity_state_t>(MAX_ENTITIES);
	
	for(int i=0; i<X_asz(cl.entities_parse); i++)
	{
		cl.entities_parse[i].clear();
	}
	cl.entities_parse_index		= 0;

//	cl.cmd.clear();
	cl.cmds_num			= 0;
	for(int i=0; i<CMD_BACKUP; i++)
	{
		cl.cmds[i].clear();
		
		cl.cmd_time[i] = 0;
	}


	cl.frame.clear();				// received from server
	cl.surpress_count		= 0;		// number of messages rate supressed
	for(int i=0; i<CMD_BACKUP; i++)
	{
		cl.frames[i].clear();
	}

	cl.viewangles.clear();

	cl.time				= 0;			// this is the time value that the client
								// is rendering at.  always <= cls.realtime

	cl.attractloop			= false;		// running the attract loop, any key will menu
	cl.servercount			= 0;			// server identification for prespawns
	cl.playernum			= 0;

	memset(cl.configstrings, 0, sizeof(cl.configstrings));
}

void	CL_ClearState()
{
	S_StopAllSounds();
	
	cge->CG_ClearState();

	CL_ClearClientState();
	
	cls.netchan.message.beginWriting();

}

/*
=====================
CL_Disconnect

Goes from a connected state to full screen console state
Sends a disconnect message to the server
This is also called on Com_Error, so it shouldn't cause any errors
=====================
*/
void	CL_Disconnect()
{
	if(cls.state == CA_DISCONNECTED)
		return;

	if(cl_timedemo && cl_timedemo->getValue())
	{
		int	time;
		
		time = Sys_Milliseconds () - cl.timedemo_start;
		if (time > 0)
			Com_Printf ("%i frames, %3.1f seconds: %3.1f fps\n", cl.timedemo_frames,
			time/1000.0, cl.timedemo_frames*1000.0 / time);
	}

	uie->M_ForceMenuOff();

	cls.connect_time = 0;

	if(cls.demo_recording)
		CL_Stop_f();

	// send a disconnect message to the server
	bitmessage_c msg(MAX_PACKETLEN*8);
	msg.writeByte(CLC_STRINGCMD);
	msg.writeString("disconnect");
	
	cls.netchan.transmit(msg);
	cls.netchan.transmit(msg);
	cls.netchan.transmit(msg);

	CL_ClearState();

	// stop download
	if(cls.download_stream) 
	{
		VFS_FClose(&cls.download_stream);
	}

	cls.state = CA_DISCONNECTED;
}

void	CL_Disconnect_f()
{
	Com_Error(ERR_DROP, "Disconnected from server");
}


/*
====================
CL_Packet_f

packet <destination> <contents>

Contents allows \n escape character
====================
*/
/*
void	CL_Packet_f()
{
	char	send[2048];
	int		i, l;
	char	*in, *out;
	netadr_t	adr;

	if(Cmd_Argc() != 3)
	{
		Com_Printf("packet <destination> <contents>\n");
		return;
	}

	if(!Sys_StringToAdr(Cmd_Argv(1), adr))
	{
		Com_Printf("Bad address\n");
		return;
	}
	if(!adr.port)
		adr.port = BigShort(PORT_SERVER);

	in = (char*)Cmd_Argv(2);
	out = send+4;
	send[0] = send[1] = send[2] = send[3] = (char)0xff;

	l = strlen (in);
	for (i=0 ; i<l ; i++)
	{
		if(in[i] == '\\' && in[i+1] == 'n')
		{
			*out++ = '\n';
			i++;
		}
		else
			*out++ = in[i];
	}
	*out = 0;

	Sys_SendPacket(send, out-send, adr);
}
*/

/*
=================
CL_Changing_f

Just sent as a hint to the client that they should
drop to full console
=================
*/
void	CL_Changing_f()
{
	//ZOID
	//if we are downloading, we don't change!  This so we don't suddenly stop downloading a map
	if(cls.download_stream)
		return;

	SCR_BeginLoadingPlaque();
	
	cls.state = CA_CONNECTED;	// not active anymore, but not disconnected
	Com_Printf("\nChanging map...\n");
}


/*
=================
CL_Reconnect_f

The server is changing levels
=================
*/
void	CL_Reconnect_f()
{
	//ZOID
	//if we are downloading, we don't change!  This so we don't suddenly stop downloading a map
	if(cls.download_stream)
		return;

	S_StopAllSounds();
	
	if(cls.state == CA_CONNECTED)
	{
		Com_Printf("reconnecting...\n");
		//cls.state = ca_connected;
		cls.netchan.message.writeByte(CLC_STRINGCMD);
		cls.netchan.message.writeString("new");
		return;
	}

	if(*cls.servername)
	{
		if(cls.state >= CA_CONNECTED)
		{
			CL_Disconnect();
			cls.connect_time = cls.realtime - 1500;
		}
		else
		{
			cls.connect_time = -99999; // fire immediately
		}

		cls.state = CA_CONNECTING;
		Com_Printf("reconnecting...\n");
	}
}

/*
=================
CL_ParseStatusMessage

Handle a reply from a ping
=================
*/
static void	CLC_ParseStatusMessage(bitmessage_c &msg, const netadr_t &adr)
{
	const char *s = msg.readString();

	Com_Printf("%s\n", s);
	
	uie->M_AddToServerList(adr, s);
}


void	CL_PingServers_f()
{
	int			i;
	netadr_t	adr;
	char		name[32];
	const char		*adrstring;
	cvar_t		*noudp;

	// send a broadcast packet
	Com_Printf("pinging broadcast...\n");

	noudp = Cvar_Get("noudp", "0", CVAR_INIT);
	if(!noudp->getValue())
	{
		adr.type = NA_BROADCAST;
		adr.port = BigShort(PORT_SERVER);
		Netchan_OutOfBandPrint(adr, va("info %i", PROTOCOL_VERSION));
	}

	// send a packet to each address book entry
	for(i=0 ; i<16 ; i++)
	{
		Com_sprintf(name, sizeof(name), "adr%i", i);
		adrstring = Cvar_VariableString (name);
		if(!adrstring || !adrstring[0])
			continue;

		Com_Printf("pinging %s...\n", adrstring);
		if(!Sys_StringToAdr(adrstring, adr))
		{
			Com_Printf("Bad address: %s\n", adrstring);
			continue;
		}
		
		if(!adr.port)
			adr.port = BigShort(PORT_SERVER);
			
		Netchan_OutOfBandPrint(adr, va("info %i", PROTOCOL_VERSION));
	}
}




/*
=================
CL_ConnectionlessPacket

Responses to broadcasts, etc
=================
*/
void	CL_ConnectionlessPacket(bitmessage_c &msg, const netadr_t &adr)
{
	const char	*string;
	const char	*cmd;
	
	msg.beginReading();
	msg.readLong();	// skip the -1

	string = msg.readString();

	Cmd_TokenizeString(string);

	cmd = Cmd_Argv(0);

	Com_DPrintf("connectionless packet '%s' : '%s'\n", Sys_AdrToString(adr), string);

	// server connection
	if(!strcmp(cmd, "client_connect"))
	{
		if(cls.state == CA_CONNECTED)
		{
			Com_Printf("Dup connect received.  Ignored.\n");
			return;
		}
		cls.netchan.setup(adr, cls.quake_port, true);
		cls.netchan.message.writeByte(CLC_STRINGCMD);
		cls.netchan.message.writeString("new");	
		cls.state = CA_CONNECTED;
		return;
	}

	// server responding to a status broadcast
	if(!strcmp(cmd, "info"))
	{
		CLC_ParseStatusMessage(msg, adr);
		return;
	}

	// remote command from gui front end
	if(!strcmp(cmd, "cmd"))
	{
		if(!Sys_IsLocalAddress(adr))
		{
			Com_Printf("Command packet from remote host.  Ignored.\n");
			return;
		}
		Sys_AppActivate();
		string = msg.readString();
		Cbuf_AddText(string);
		Cbuf_AddText("\n");
		return;
	}
	
	// print command from somewhere
	if(!strcmp(cmd, "print"))
	{
		string = msg.readString();
		Com_Printf("%s", string);
		return;
	}

	// ping from somewhere
	if(!strcmp(cmd, "ping"))
	{
		Netchan_OutOfBandPrint(adr, "ack");
		return;
	}

	// challenge from the server we are connecting to
	if(!strcmp(cmd, "challenge"))
	{
		cls.challenge = atoi(Cmd_Argv(1));
		CLC_SendConnectPacket();
		return;
	}

	// echo request from server
	if(!strcmp(cmd, "echo"))
	{
		Netchan_OutOfBandPrint(adr, "%s", Cmd_Argv(1));
		return;
	}

	Com_Printf("Unknown command.\n");
}

void	CL_PacketEvent(bitmessage_c &msg, const netadr_t &adr)
{
//	Com_DPrintf("CL_PacketEvent()\n");

	//
	// remote command packet
	//
	if(msg.isConnectionless())
	{
		CL_ConnectionlessPacket(msg, adr);
		return;
	}

	if(cls.state == CA_DISCONNECTED || cls.state == CA_CONNECTING)
		return;		// dump it if not connected

	if(msg.getCurSize() < NETCHAN_PACKET_HEADER_BITS)
	{
		Com_Printf("%s: runt packet\n", Sys_AdrToString(adr));
		return;
	}

	//
	// packet from server
	//
	if(!Sys_CompareAdr(adr, cls.netchan.getRemoteAddress()))
	{
		Com_DPrintf("%s:sequenced packet without connection\n", Sys_AdrToString(adr));
		return;
	}
	
	if(cls.netchan.process(msg))
	{
		// this is a valid, sequenced packet, so process it
		CL_ParseServerMessage(msg);
	}
}

static void	CL_CheckTimeout()
{
	if(cls.state >= CA_CONNECTED && cls.realtime - cls.netchan.getLastReceived() > cl_timeout->getValue()*1000)
	{
		if(++cl.timeoutcount > 5)	// timeoutcount saves debugger
		{
			Com_Printf("\nServer connection timed out.\n");
			CL_Disconnect();
			return;
		}
	}
	else
		cl.timeoutcount = 0;
}


/*
void	CL_ReadPackets()
{
	netadr_t	adr;
	sizebuf_t	msg;
	byte		msg_buffer[MAX_PACKETLEN];
	
	MSG_InitRaw(&msg, msg_buffer, sizeof(msg_buffer));

	while((msg.cursize = Sys_GetPacket(msg.data, msg.maxsize, adr)) != -1)
	{
		//Com_Printf ("packet\n");
		
		//
		// remote command packet
		//
		if(*(int*)msg.data == -1)
		{
			CL_ConnectionlessPacket(&msg, adr);
			continue;
		}

		if(cls.state == CA_DISCONNECTED || cls.state == CA_CONNECTING)
			continue;		// dump it if not connected

		if(msg.cursize < 8)
		{
			Com_Printf("%s: Runt packet\n", Sys_AdrToString(adr));
			continue;
		}

		//
		// packet from server
		//
		if(!Sys_CompareAdr(adr, cls.netchan.getRemoteAddress()))
		{
			Com_DPrintf("%s:sequenced packet without connection\n", Sys_AdrToString(adr));
			continue;
		}
		
		if(!cls.netchan.process(&msg))
			continue;		// wasn't accepted for some reason
			
		CL_ParseServerMessage(&msg);
	}


	//
	// check timeout
	//
	if(cls.state >= CA_CONNECTED && cls.realtime - cls.netchan.getLastReceived() > cl_timeout->getValue()*1000)
	{
		if(++cl.timeoutcount > 5)	// timeoutcount saves debugger
		{
			Com_Printf("\nServer connection timed out.\n");
			CL_Disconnect();
			return;
		}
	}
	else
		cl.timeoutcount = 0;
}
*/


void	CL_FixUpGender()
{
	char *p;
	char sk[80];

	if(gender_auto->getValue())
	{
		if(gender->isModified())
		{
			// was set directly, don't override the user
			gender->isModified(false);
			return;
		}

		strncpy(sk, skin->getString(), sizeof(sk) - 1);
		
		if((p = strchr(sk, '/')) != NULL)
			*p = 0;
		if(X_stricmp(sk, "male") == 0 || X_stricmp(sk, "cyborg") == 0)
			Cvar_Set("gender", "male");
		else if(X_stricmp(sk, "female") == 0 || X_stricmp(sk, "crackhor") == 0)
			Cvar_Set("gender", "female");
		else
			Cvar_Set("gender", "none");
			
		gender->isModified(false);
	}
}

void	CL_Userinfo_f()
{
	Com_Printf("User info settings:\n");
	Com_PrintInfo(Cvar_Userinfo());
}




static int precache_check; // for autodownload of precache items
static int precache_spawncount;
//static int precache_tex;
static int precache_model_skin;

static byte *precache_model; // used for skin checking in alias models

/*
#define PLAYER_MULT 5

// ENV_CNT is map load, ENV_CNT+1 is first env map
#define ENV_CNT (CS_PLAYERSKINS + MAX_CLIENTS * PLAYER_MULT)
#define TEXTURE_CNT (ENV_CNT+13)
*/

void	CL_RequestNextDownload()
{
	//unsigned	map_checksum;		// for detecting cheater maps
	//char fn[MAX_OSPATH];
	//md2_dheader_t *pheader;

	if(cls.state != CA_CONNECTED)
		return;

	/*	
	if (!allow_download->getValue() && precache_check < ENV_CNT)
		precache_check = ENV_CNT;

//ZOID
	if (precache_check == CS_MODELS) 
	{ // confirm map
		precache_check = CS_MODELS+2; // 0 isn't used
		if (allow_download_maps->getValue())
			if (!CL_CheckOrDownloadFile(cl.configstrings[CS_MODELS+1]))
				return; // started a download
	}
	
	
	if (precache_check >= CS_MODELS && precache_check < CS_MODELS+MAX_MODELS) 
	{
		if (allow_download_models->getValue()) 
		{
			while (precache_check < CS_MODELS+MAX_MODELS &&	cl.configstrings[precache_check][0]) 
			{
				if (cl.configstrings[precache_check][0] == '*' || cl.configstrings[precache_check][0] == '#') 
				{
					precache_check++;
					continue;
				}
				
				if (precache_model_skin == 0) 
				{
					if (!CL_CheckOrDownloadFile(cl.configstrings[precache_check])) 
					{
						precache_model_skin = 1;
						return; // started a download
					}
					
					precache_model_skin = 1;
				}

				// checking for skins in the model
				if (!precache_model) {

					VFS_FLoad (cl.configstrings[precache_check], (void **)&precache_model);
					if (!precache_model) {
						precache_model_skin = 0;
						precache_check++;
						continue; // couldn't load it
					}
					
					if (LittleLong(*(unsigned *)precache_model) != MD2_IDALIASHEADER) 
					{
						// not an alias model
						VFS_FFree(precache_model);
						precache_model = 0;
						precache_model_skin = 0;
						precache_check++;
						continue;
					}
					
					pheader = (md2_dheader_t *)precache_model;
					if (LittleLong (pheader->version) != MD2_ALIAS_VERSION) 
					{
						precache_check++;
						precache_model_skin = 0;
						continue; // couldn't load it
					}
				}

				pheader = (md2_dheader_t *)precache_model;

				while (precache_model_skin - 1 < LittleLong(pheader->skins_num)) {
					if (!CL_CheckOrDownloadFile((char *)precache_model +
						LittleLong(pheader->skins_ofs) + 
						(precache_model_skin - 1)*MD2_MAX_SKINNAME)) {
						precache_model_skin++;
						return; // started a download
					}
					precache_model_skin++;
				}
				if (precache_model) { 
					VFS_FFree(precache_model);
					precache_model = 0;
				}
				precache_model_skin = 0;
				precache_check++;
			}
		}
		precache_check = CS_SOUNDS;
	}
	if (precache_check >= CS_SOUNDS && precache_check < CS_SOUNDS+MAX_SOUNDS) { 
		if (allow_download_sounds->getValue()) {
			if (precache_check == CS_SOUNDS)
				precache_check++; // zero is blank
			while (precache_check < CS_SOUNDS+MAX_SOUNDS &&
				cl.configstrings[precache_check][0]) {
				if (cl.configstrings[precache_check][0] == '*') {
					precache_check++;
					continue;
				}
				Com_sprintf(fn, sizeof(fn), "sound/%s", cl.configstrings[precache_check++]);
				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}
		precache_check = CS_IMAGES;
	}
	if (precache_check >= CS_IMAGES && precache_check < CS_IMAGES+MAX_IMAGES) 
	{
		if (precache_check == CS_IMAGES)
			precache_check++; // zero is blank
			
		while (precache_check < CS_IMAGES+MAX_IMAGES &&
			cl.configstrings[precache_check][0]) 
		{
				//Com_sprintf(fn, sizeof(fn), "textures/pics/%s.pcx", cl.configstrings[precache_check++]);
				Com_sprintf(fn, sizeof(fn), "%s", cl.configstrings[precache_check++]);
				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
		}
		precache_check = CS_PLAYERSKINS;
	}
	
	// skins are special, since a player has three things to download:
	// model, weapon model and skin
	// so precache_check is now *3
	if (precache_check >= CS_PLAYERSKINS && precache_check < CS_PLAYERSKINS + MAX_CLIENTS * PLAYER_MULT) {
		if (allow_download_players->getValue()) {
			while (precache_check < CS_PLAYERSKINS + MAX_CLIENTS * PLAYER_MULT) {
				int i, n;
				char model[MAX_QPATH], skin[MAX_QPATH], *p;

				i = (precache_check - CS_PLAYERSKINS)/PLAYER_MULT;
				n = (precache_check - CS_PLAYERSKINS)%PLAYER_MULT;

				if (!cl.configstrings[CS_PLAYERSKINS+i][0]) {
					precache_check = CS_PLAYERSKINS + (i + 1) * PLAYER_MULT;
					continue;
				}

				if ((p = strchr(cl.configstrings[CS_PLAYERSKINS+i], '\\')) != NULL)
					p++;
				else
					p = cl.configstrings[CS_PLAYERSKINS+i];
				strcpy(model, p);
				p = strchr(model, '/');
				if (!p)
					p = strchr(model, '\\');
				if (p) {
					*p++ = 0;
					strcpy(skin, p);
				} else
					*skin = 0;

				switch (n) {
				case 0: // model
					Com_sprintf(fn, sizeof(fn), "players/%s/tris.md2", model);
					if (!CL_CheckOrDownloadFile(fn)) {
						precache_check = CS_PLAYERSKINS + i * PLAYER_MULT + 1;
						return; // started a download
					}
					n++;
					//FALL THROUGH

				case 1: // weapon model
					Com_sprintf(fn, sizeof(fn), "players/%s/weapon.md2", model);
					if (!CL_CheckOrDownloadFile(fn)) {
						precache_check = CS_PLAYERSKINS + i * PLAYER_MULT + 2;
						return; // started a download
					}
					n++;
					//FALL THROUGH/

				case 2: // weapon skin
					Com_sprintf(fn, sizeof(fn), "players/%s/weapon.pcx", model);
					if (!CL_CheckOrDownloadFile(fn)) {
						precache_check = CS_PLAYERSKINS + i * PLAYER_MULT + 3;
						return; // started a download
					}
					n++;
					//FALL THROUGH

				case 3: // skin
					Com_sprintf(fn, sizeof(fn), "players/%s/%s.pcx", model, skin);
					if (!CL_CheckOrDownloadFile(fn)) {
						precache_check = CS_PLAYERSKINS + i * PLAYER_MULT + 4;
						return; // started a download
					}
					n++;
					//FALL THROUGH

				case 4: // skin_i
					Com_sprintf(fn, sizeof(fn), "players/%s/%s_i.pcx", model, skin);
					if (!CL_CheckOrDownloadFile(fn)) {
						precache_check = CS_PLAYERSKINS + i * PLAYER_MULT + 5;
						return; // started a download
					}
					// move on to next model
					precache_check = CS_PLAYERSKINS + (i + 1) * PLAYER_MULT;
				}
			}
		}
		// precache phase completed
		precache_check = ENV_CNT;
	}
	*/
	
	//if(precache_check == ENV_CNT)
	{
		//precache_check = ENV_CNT + 1;

		//std::string mapname = "maps/" + std::string(cl.configstrings[CS_MAPNAME]);

		//CM_BeginRegistration(mapname, true, &map_checksum);

		//if((int)map_checksum != atoi(cl.configstrings[CS_MAPCHECKSUM]))
		//{
		//	Com_Error(ERR_DROP, "Local map version differs from server: %i != '%s'\n", map_checksum, cl.configstrings[CS_MAPCHECKSUM]);
		//	return;
		//}
	}

	/*
	if(precache_check > ENV_CNT && precache_check < TEXTURE_CNT) 
	{
		if (allow_download->getValue() && allow_download_maps->getValue())
		{
			while (precache_check < TEXTURE_CNT) 
			{
				//int n = precache_check++ - ENV_CNT - 1;
				
				
				//if (n & 1)
				//	Com_sprintf(fn, sizeof(fn), "env/%s%s.pcx", cl.configstrings[CS_SKY], env_suf[n/2]);
				//else
				//	Com_sprintf(fn, sizeof(fn), "env/%s%s.tga", cl.configstrings[CS_SKY], env_suf[n/2]);
				
				if(!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}
		precache_check = TEXTURE_CNT;
	}

	if (precache_check == TEXTURE_CNT) {
		precache_check = TEXTURE_CNT+1;
		precache_tex = 0;
	}
	*/

	// confirm existance of textures, download any that don't exist
	/*
	if (precache_check == TEXTURE_CNT+1)
	{
		// from qcommon/cmodel.c
		extern int			numtexinfo;
		extern mapsurface_t	map_surfaces[];

		if (allow_download->getValue() && allow_download_maps->getValue())
		{
			while (precache_tex < numtexinfo) {
				char fn[MAX_OSPATH];

				sprintf(fn, "textures/%s.wal", map_surfaces[precache_tex++].rname);
				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}
		precache_check = TEXTURE_CNT+999;
	}
	*/
	
	//Tr3B - HACK HACK HACK  TODO download textures
	//if (precache_check == TEXTURE_CNT+1) 
	//	precache_check = TEXTURE_CNT+999;
//ZOID


	//
	// continue 
	//
	cge->CG_PrepRefresh();

	cls.netchan.message.writeByte(CLC_STRINGCMD);
	cls.netchan.message.writeString(va("begin %i\n", precache_spawncount));
}

/*
=================
CL_Precache_f

The server will send this command right
before allowing the client into the server
=================
*/
void	CL_Precache_f()
{
	precache_check = CS_MODELS;
	precache_spawncount = atoi(Cmd_Argv(1));
	precache_model = 0;
	precache_model_skin = 0;

	CL_RequestNextDownload();
}


/*
===============
CL_WriteConfig

Writes key bindings and archived cvars to config.cfg
===============
*/
static void	CL_WriteConfig(const std::string &filename)
{
	VFILE*		v = NULL;

	if(cls.state == CA_UNINITIALIZED)
		return;
		
	VFS_FOpenWrite(filename , &v);
	if(!v)
	{
		Com_Printf("Couldn't begin writing %s\n", filename.c_str());
		return;
	}
	
	Com_Printf("writing %s...\n", filename.c_str());

	VFS_FPrintf(v, "// generated by QRazor FX III, do not modify\n");
	Key_WriteBindings(v);
	Cvar_WriteVars(v);
	VFS_FClose(&v);
}

/*
===============
CL_WriteConfig_f

Writes key bindings and archived cvars to config_of_choice.cfg
===============
*/
void	CL_WriteConfig_f()
{
	if(Cmd_Argc() != 2)
	{
		Com_Printf("writeconfig usage:  writeconfig <filename>\n");
		return;
	}
	
	CL_WriteConfig(Cmd_Argv(1));
}


void	CL_InitLocal()
{
	cls.state = CA_DISCONNECTED;
	cls.realtime = Sys_Milliseconds();

	CL_InitInput();

	adr0 = Cvar_Get("adr0", "", CVAR_ARCHIVE);
	adr1 = Cvar_Get("adr1", "", CVAR_ARCHIVE);
	adr2 = Cvar_Get("adr2", "", CVAR_ARCHIVE);
	adr3 = Cvar_Get("adr3", "", CVAR_ARCHIVE);
	adr4 = Cvar_Get("adr4", "", CVAR_ARCHIVE);
	adr5 = Cvar_Get("adr5", "", CVAR_ARCHIVE);
	adr6 = Cvar_Get("adr6", "", CVAR_ARCHIVE);
	adr7 = Cvar_Get("adr7", "", CVAR_ARCHIVE);
	adr8 = Cvar_Get("adr8", "", CVAR_ARCHIVE);
	
	

	//
	// register our variables
	//
	cl_maxfps		= Cvar_Get("cl_maxfps", "90", CVAR_ARCHIVE);

	cl_shownet		= Cvar_Get("cl_shownet", "0", 0);
	cl_timeout		= Cvar_Get("cl_timeout", "120", 0);
	cl_timedemo		= Cvar_Get("timedemo", "0", 0);

	rcon_client_password	= Cvar_Get("rcon_password", "", 0);
	rcon_address		= Cvar_Get("rcon_address", "", 0);


	//
	// userinfo
	//
	info_password		= Cvar_Get("password", "", CVAR_USERINFO);
	info_spectator		= Cvar_Get("spectator", "0", CVAR_USERINFO);
	name			= Cvar_Get("name", "unnamed", CVAR_USERINFO | CVAR_ARCHIVE);
	
	//std::string skin_name	= std::string(DEFAULT_PLAYERMODEL) + "/" + std::string(DEFAULT_PLAYERSKIN);
	model			= Cvar_Get("model", DEFAULT_PLAYERMODEL, CVAR_USERINFO | CVAR_ARCHIVE);
	skin			= Cvar_Get("skin", DEFAULT_PLAYERSKIN, CVAR_USERINFO | CVAR_ARCHIVE);
	
	rate			= Cvar_Get("rate", "25000", CVAR_USERINFO | CVAR_ARCHIVE);	// FIXME
	msg			= Cvar_Get("msg", "1", CVAR_USERINFO | CVAR_ARCHIVE);
	hand			= Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	fov			= Cvar_Get("fov", "90", CVAR_USERINFO | CVAR_ARCHIVE);
	gender			= Cvar_Get("gender", "male", CVAR_USERINFO | CVAR_ARCHIVE);
	gender_auto		= Cvar_Get ("gender_auto", "1", CVAR_ARCHIVE);
	gender->isModified(false); // clear this so we know when user sets it manually

	

	//
	// register our commands
	//
	Cmd_AddCommand("cmd",		CL_ForwardToServer_f);
	Cmd_AddCommand("pingservers",	CL_PingServers_f);
	Cmd_AddCommand("userinfo",	CL_Userinfo_f);
	Cmd_AddCommand("changing",	CL_Changing_f);
	Cmd_AddCommand("disconnect",	CL_Disconnect_f);
	Cmd_AddCommand("record",	CL_Record_f);
	Cmd_AddCommand("stop",		CL_Stop_f);
	Cmd_AddCommand("quit",		CL_Quit_f);
	Cmd_AddCommand("connect",	CL_Connect_f);
	Cmd_AddCommand("reconnect",	CL_Reconnect_f);
	Cmd_AddCommand("rcon",		CL_Rcon_f);
//	Cmd_AddCommand("packet",	CL_Packet_f);		// this is dangerous to leave in
	Cmd_AddCommand("setenv",	CL_Setenv_f );
	Cmd_AddCommand("precache",	CL_Precache_f);
	Cmd_AddCommand("download",	CL_Download_f);	
	Cmd_AddCommand("writeconfig",	CL_WriteConfig_f);
}



void	CL_Frame(int msec)
{
	static int	extratime;
	//static int  lasttimecalled;

	if(dedicated->getValue())
		return;

	extratime += msec;

	if(!cl_timedemo->getValue())
	{
		if(cls.state == CA_CONNECTED && extratime < 100)
			return;			// don't flood packets out while connecting
			
		if(extratime < 1000/cl_maxfps->getValue())
			return;			// framerate is too high
	}

	// let the mouse activate or deactivate
	IN_Frame();

	// decide the simulation time
	cls.realtime = Sys_Milliseconds();
	cls.frametime = extratime;
	cl.time += extratime;

	extratime = 0;
	
	if(cls.frametime > 5000)
	{
		cls.frametime = 5000;
	}
	
	/*
	if(msec > 5000)
	{
		// if in the debugger last frame, don't timeout
		cls.netchan.last_received = Sys_Milliseconds();
	}
	*/

	// fetch results from server
//	CL_ReadPackets();

	// check timeout
	CL_CheckTimeout();

	// get new key events
	Sys_PushEvents();

	// allow mice or other external controllers to add commands
	IN_Commands();

	// process console commands
	Cbuf_Execute();

	// send intentions now
	CL_SendCmd();

	// resend a connection request if necessary
	CL_CheckForResend();

	// allow rendering DLL change
//	VID_CheckChanges();
//	if(!cl.refresh_prepped && cls.state == CA_ACTIVE)
//		cge->CG_PrepRefresh();

	// update the screen
	if(com_speeds->getValue())
		time_before_ref = Sys_Milliseconds();
	
	SCR_UpdateScreen();
	
	if(com_speeds->getValue())
		time_after_ref = Sys_Milliseconds();
	
	// advance local effects for next frame
//	SCR_RunCinematic();	
	SCR_RunConsole();

	cls.framecount++;
}


void	CL_Init()
{
	if(dedicated->getValue())
		return;		// nothing running on the client
	
	Com_Printf("------- CL_Init -------\n");

	// all archived variables will now be loaded
	Con_Init();
	
	VID_Init();

	S_Init();
	
	SCR_Init();

	CL_InitUserInterface();
	
	CL_InitClientGame();
	
	cls.disable_screen = true;	// don't draw yet
	
	CL_InitLocal();
	
	IN_Init();
	
	//Cbuf_AddText("exec autoexec.cfg\n");
	VFS_ExecAutoexec();
	Cbuf_Execute();
}


/*
===============
CL_Shutdown

FIXME: this is a callback from Sys_Quit and Com_Error.  It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void	CL_Shutdown()
{
	static bool isdown = false;
	
	if(isdown)
	{
		printf("recursive shutdown\n");
		return;
	}
	else
		Com_Printf("------- CL_Shutdown -------\n");
		
	isdown = true;

	CL_WriteConfig("config.cfg");

	S_Shutdown();
	
	IN_Shutdown();
	
	VID_Shutdown();
}


