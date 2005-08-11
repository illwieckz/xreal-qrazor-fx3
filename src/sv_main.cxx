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
#include "sys.h"


netadr_t	master_adr[MAX_MASTERS];	// address of group servers

cvar_t	*sv_shownet;
cvar_t	*sv_paused;
cvar_t	*sv_timedemo;

cvar_t	*sv_enforcetime;

cvar_t	*timeout;				// seconds without any message
cvar_t	*zombietime;			// seconds to sink messages after disconnect

cvar_t	*rcon_password;			// password for remote server commands

cvar_t	*sv_allow_download;
cvar_t	*sv_allow_download_players;
cvar_t	*sv_allow_download_models;
cvar_t	*sv_allow_download_sounds;
cvar_t	*sv_allow_download_maps;

cvar_t	*sv_airaccelerate;

cvar_t	*sv_noreload;			// don't reload level state when reentering

cvar_t	*maxclients;			// FIXME: rename sv_maxclients
cvar_t	*sv_showclamp;

cvar_t	*hostname;
cvar_t	*public_server;			// should heartbeats be sent

cvar_t	*sv_reconnect_limit;	// minimum seconds between connect messages











/*
==============================================================================

CONNECTIONLESS COMMANDS

==============================================================================
*/

/*
===============
SV_StatusString

Builds the string that is sent as heartbeats and status replies
===============
*/
static const char*	SV_StatusString()
{
	char	player[1024];
	static char	status[MAX_PACKETLEN - 16];
	int		statusLength;
	int		playerLength;

	strcpy(status, Cvar_Serverinfo());
	strcat(status, "\n");
	statusLength = strlen(status);

	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
		
		if(cl->getState() == CS_CONNECTED || cl->getState() == CS_SPAWNED)
		{
			Com_sprintf(player, sizeof(player), "%i %i \"%s\"\n", cl->getEntity()->_r.ps.stats[STAT_FRAGS], cl->getPing(), cl->getName());
			
			playerLength = strlen(player);
			
			if(statusLength + playerLength >= (int)sizeof(status))
				break;		// can't hold any more
				
			strcpy(status + statusLength, player);
			statusLength += playerLength;
		}
	}

	return status;
}

/*
================
SVC_Status

Responds with all the info that qplug or qspy can see
================
*/
static void 	SVC_Status(const netadr_t &adr)
{
	Netchan_OutOfBandPrint(adr, "print\n%s", SV_StatusString());
}

static void 	SVC_Ack(const netadr_t &adr)
{
	Com_Printf("Ping acknowledge from %s\n", Sys_AdrToString(adr));
}

/*
================
SVC_Info

Responds with short info for broadcast scans
The second parameter should be the current protocol version number.
================
*/
static void 	SVC_Info(const netadr_t &adr)
{
	char	string[64];
	
	if(maxclients->getInteger() == 1)
		return;		// ignore in single player

	int version = atoi(Cmd_Argv(1));

	if(version != PROTOCOL_VERSION)
	{
		Com_sprintf(string, sizeof(string), "%s: wrong version\n", hostname->getString(), sizeof(string));
	}
	else
	{
		int count = 0;
		for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
		{
			sv_client_c *cl = *ir;
	
			if(!cl)
				continue;
				
			if(cl->getState() >= CS_CONNECTED)
				count++;
		}

		Com_sprintf(string, sizeof(string), "%16s %8s %2i/%2i\n", hostname->getString(), sv.name.c_str(), count, maxclients->getInteger());
	}

	Netchan_OutOfBandPrint(adr, "info\n%s", string);
}

/*
================
SVC_Ping

Just responds with an acknowledgement
================
*/
static void 	SVC_Ping(const netadr_t &adr)
{
	Netchan_OutOfBandPrint(adr, "ack");
}


/*
=================
SVC_GetChallenge

Returns a challenge number that can be used
in a subsequent client_connect command.
We do this to prevent denial of service attacks that
flood the server with invalid connection IPs.  With a
challenge, they must give a valid IP address.
=================
*/
static void 	SVC_GetChallenge(const netadr_t &adr)
{
	int		i;
	int		oldest;
	int		oldestTime;

	oldest = 0;
	oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	for(i=0; i < MAX_CHALLENGES; i++)
	{
		if(Sys_CompareBaseAdr(adr, svs.challenges[i].adr))
			break;
			
		if(svs.challenges[i].time < oldestTime)
		{
			oldestTime = svs.challenges[i].time;
			oldest = i;
		}
	}

	if(i == MAX_CHALLENGES)
	{
		// overwrite the oldest
		svs.challenges[oldest].challenge = rand() & 0x7fff;
		svs.challenges[oldest].adr = adr;
		svs.challenges[oldest].time = Sys_Milliseconds();
		i = oldest;
	}

	// send it back
	Netchan_OutOfBandPrint(adr, "challenge %i", svs.challenges[i].challenge);
}

/*
==================
SVC_DirectConnect

A connection request that did not come from the master
==================
*/
static void 	SVC_DirectConnect(const netadr_t &adr)
{
	info_c			userinfo;
	int			i;
	sv_entity_c		*ent;
	int			edictnum;
	int			version;
	int			qport;
	int			challenge;

	Com_DPrintf("SVC_DirectConnect()\n");

	version = atoi(Cmd_Argv(1));
	if(version != PROTOCOL_VERSION)
	{
		Netchan_OutOfBandPrint(adr, "print\nServer is version %4.2f.\n", VERSION);
		Com_DPrintf("    rejected connect from version %i\n", version);
		return;
	}

	qport = atoi(Cmd_Argv(2));

	challenge = atoi(Cmd_Argv(3));

	userinfo.fromString(Cmd_Argv(4));

	// force the IP key/value pair so the game can filter based on ip
	userinfo.setValueForKey("ip", Sys_AdrToString(adr));

	// attractloop servers are ONLY for local clients
	if(sv.attractloop)
	{
		if(!Sys_IsLocalAddress(adr))
		{
			Com_Printf("Remote connect in attract loop.  Ignored.\n");
			Netchan_OutOfBandPrint(adr, "print\nConnection refused.\n");
			return;
		}
	}

	// see if the challenge is valid
	if(!Sys_IsLocalAddress(adr))
	{
		for(i=0; i<MAX_CHALLENGES; i++)
		{
			if(Sys_CompareBaseAdr(adr, svs.challenges[i].adr))
			{
				if(challenge == svs.challenges[i].challenge)
					break;		// good
					
				Netchan_OutOfBandPrint(adr, "print\nBad challenge.\n");
				return;
			}
		}
		if(i == MAX_CHALLENGES)
		{
			Netchan_OutOfBandPrint(adr, "print\nNo challenge for address.\n");
			return;
		}
	}
	
	std::vector<sv_client_c*>::iterator ir;

	// if there is already a slot for this ip, reuse it
	for(ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
	
		if(cl->getState() == CS_FREE)
			continue;
			
		if(Sys_CompareBaseAdr(adr, cl->netchan.getRemoteAddress()) && (cl->netchan.getRemoteAddress().port == qport))
		{
			if(!Sys_IsLocalAddress(adr) && (svs.realtime - cl->getLastConnectTime()) < ((int)sv_reconnect_limit->getValue() * 1000))
			{
				Com_DPrintf("%s:reconnect rejected : too soon\n", Sys_AdrToString(adr));
				return;
			}
			
			Com_Printf("%s:reconnect\n", Sys_AdrToString(adr));
			
			goto svc_directconnect_got_slot;
		}
	}

	// find a client slot
	ir = find(svs.clients.begin(), svs.clients.end(), static_cast<sv_client_c*>(NULL));
	
	if(ir == svs.clients.end())
	{
		Netchan_OutOfBandPrint(adr, "print\nServer is full.\n");
		
		Com_DPrintf("Rejected a connection.\n");
		return;
	}


svc_directconnect_got_slot:

	// build a new connection
	// accept the new client
	// this is the only place a sv_client_c is ever initialized
	if(*ir)
	{
		delete *ir;
		*ir = NULL;
	}
		
	sv_client_c *newcl = *ir = new sv_client_c(adr, qport, challenge);
	
	edictnum = SV_GetNumForClient(newcl) + 1;
	ent = SV_GetEntityByNum(edictnum);

	newcl->setEntity(ent);
	
	//Com_DPrintf("SVC_DirectConnect: %i\n", ent->s.number);

	// get the game a chance to reject this connection or modify the userinfo
	if(!(ge->G_ClientConnect(ent, userinfo)))
	{
		if(*userinfo.getValueForKey("rejmsg")) 
			Netchan_OutOfBandPrint(adr, "print\n%s\nConnection refused.\n", userinfo.getValueForKey("rejmsg"));
		else
			Netchan_OutOfBandPrint(adr, "print\nConnection refused.\n");
			
		Com_DPrintf("Game rejected a connection.\n");
		
		// destroy client and clear slot
		delete *ir;
		*ir = NULL;
		
		return;
	}

	// parse some info from the info strings
	newcl->setUserInfo(userinfo);
	newcl->extractUserInfo();

	// send the connect packet to the client
	Netchan_OutOfBandPrint(adr, "client_connect");
}

bool 	Rcon_Validate()
{
	if(X_strequal(rcon_password->getString(), ""))
		return false;

	if(!X_strequal(Cmd_Argv(1), rcon_password->getString()))
		return false;

	return true;
}

/*
===============
SVC_RemoteCommand

A client issued an rcon command.
Shift down the remaining args
Redirect all printfs
===============
*/
void 	SVC_RemoteCommand(bitmessage_c &msg, const netadr_t &adr)
{
	/*
	int		i;
	char	remaining[1024];

	i = Rcon_Validate();

	if(i == 0)
		Com_Printf("Bad rcon from %s:\n%s\n", Sys_AdrToString(adr), &msg[4]);
	else
		Com_Printf("Rcon from %s:\n%s\n", Sys_AdrToString(adr), &msg[4]);

	Com_BeginRedirect(sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect, adr);

	if(!Rcon_Validate())
	{
		Com_Printf("Bad rcon_password.\n");
	}
	else
	{
		remaining[0] = 0;

		for(i=2 ; i<Cmd_Argc() ; i++)
		{
			strcat(remaining, Cmd_Argv(i) );
			strcat(remaining, " ");
		}

		Cmd_ExecuteString(remaining);
	}

	Com_EndRedirect();
	*/
}

/*
=================
SV_ConnectionlessPacket

A connectionless packet has four leading 0xff
characters to distinguish it from a game channel.
Clients that are in the game can still send
connectionless packets.
=================
*/
static void 	SV_ConnectionlessPacket(bitmessage_c &msg, const netadr_t &adr)
{
	const char	*string;
	const char	*cmd;

	msg.beginReading();
	msg.readLong();		// skip the -1 marker

	string = msg.readString();
	Cmd_TokenizeString(string);
	
//	if(X_strncaseequal(net_message->data + 4, "connect", 7))
//		Huff_DecompressPacket(buf, 12);

	cmd = Cmd_Argv(0);
	
	Com_DPrintf("connectionless packet '%s : '%s'\n", Sys_AdrToString(adr), string);

	if(!strcmp(cmd, "ping"))
	{
		SVC_Ping(adr);
	}
	else if(!strcmp(cmd, "ack"))
	{
		SVC_Ack(adr);
	}	
	else if(!strcmp(cmd,"status"))
	{
		SVC_Status(adr);
	}	
	else if(!strcmp(cmd,"info"))
	{
		SVC_Info(adr);
	}	
	else if(!strcmp(cmd,"getchallenge"))
	{
		SVC_GetChallenge(adr);
	}	
	else if(!strcmp(cmd,"connect"))
	{
		SVC_DirectConnect(adr);
	}	
//	else if(!strcmp(cmd, "rcon"))
//	{
//		SVC_RemoteCommand(msg, adr);
//	}
	else
	{
		Com_Printf("bad connectionless packet from '%s':\n'%s'\n", Sys_AdrToString(adr), string);
	}
}




/*
===================
SV_CalcPings

Updates the cl->ping variables
===================
*/
static void 	SV_CalcPings()
{
	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
		
		if(cl->getState() != CS_SPAWNED)
			continue;
			
		cl->calcPing();
	}
}


/*
===================
SV_GiveMsec

Every few frames, gives all clients an allotment of milliseconds
for their command moves.  If they exceed it, assume cheating.
===================
*/
static void 	SV_GiveMsec()
{
	if(sv.framenum & 15)
		return;

	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
		
		if(cl->getState() == CS_FREE)
			continue;
			
		cl->resetCommandTime();
	}
}


void 	SV_PacketEvent(bitmessage_c &msg, const netadr_t &adr)
{
//	Com_DPrintf("SV_PacketEvent()\n");

	// check for connectionless packet (0xffffffff) first
	if(msg.isConnectionless())
	{
		SV_ConnectionlessPacket(msg, adr);
		return;
	}

	// read the qport out of the message so we can fix up
	// stupid address translating routers
	msg.readBits(NETCHAN_PACKET_HEADER_BITS_SEQUENCE);
	msg.readBits(NETCHAN_PACKET_HEADER_BITS_RELIABLE);
	
	msg.readBits(NETCHAN_PACKET_HEADER_BITS_SEQUENCE_ACK);
	msg.readBits(NETCHAN_PACKET_HEADER_BITS_RELIABLE_ACK);
	
	msg.readBits(NETCHAN_PACKET_HEADER_BITS_UNCOMPRESSED_SIZE);
	msg.readBits(NETCHAN_PACKET_HEADER_BITS_COMPRESSED_SIZE);
	
	msg.readBits(NETCHAN_PACKET_HEADER_BITS_CHECKSUM);
	
	int qport = msg.readBits(NETCHAN_PACKET_HEADER_BITS_QPORT) & 0xffff;

	// check for packets from connected clients
	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ++ir)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
			
		if(cl->getState() == CS_FREE)
			continue;
			
		if(!Sys_CompareBaseAdr(adr, cl->netchan.getRemoteAddress()))
			continue;
			
		if(cl->netchan.getQPort() != qport)
			continue;

		if(cl->netchan.getRemoteAddress().port != adr.port)
		{
			Com_Printf("SV_PacketEvent: fixing up a translated port\n");
			cl->netchan.fixRemoteAddressPort(adr.port);
		}

		if(cl->netchan.process(msg))
		{	
			// this is a valid, sequenced packet, so process it
			if(cl->getState() != CS_ZOMBIE)
			{
				cl->executeMessage(msg);
			}
		}
		
		return;
	}
}

/*
void 	SV_ReadPackets()
{
	netadr_t	adr;
	message_c	msg;
	byte		msg_buffer[MAX_PACKETLEN];
	
	MSG_InitRaw(&msg, msg_buffer, sizeof(msg_buffer));

	while((msg.cursize = Sys_GetPacket(msg.data, msg.maxsize, adr)) != -1)
	{
		// check for connectionless packet (0xffffffff) first
		if(*(int*)msg.data == -1)
		{
			SV_ConnectionlessPacket(&msg, adr);
			continue;
		}

		// read the qport out of the message so we can fix up
		// stupid address translating routers
		int			i;
		sv_client_c		*cl;
		int			qport;
		
		MSG_BeginReading(&msg);
		MSG_ReadLong(&msg);		// sequence number
		MSG_ReadLong(&msg);		// sequence number
		qport = MSG_ReadShort(&msg) & 0xffff;

		// check for packets from connected clients
		for(i=0, cl=svs.clients; i<maxclients->getInteger(); i++, cl++)
		{
			if(cl->state == CS_FREE)
				continue;
				
			if(!Sys_CompareBaseAdr(adr, cl->netchan.getRemoteAddress()))
				continue;
				
			if(cl->netchan.getQPort() != qport)
				continue;
				
			if(cl->netchan.getRemoteAddress().port != adr.port)
			{
				Com_Error(ERR_FATAL, "SV_ReadPackets: bad translated port");
				//Com_Printf("SV_ReadPackets: fixing up a translated port\n");
				//cl->netchan.getRemoteAddress().port = adr.port;
			}

			if(cl->netchan.process(&msg))
			{	
				// this is a valid, sequenced packet, so process it
				if(cl->state != CS_ZOMBIE)
				{
					cl->lastmessage = svs.realtime;	// don't timeout
					SV_ExecuteClientMessage(cl, &msg);
				}
			}
			break;
		}
		
		if(i != maxclients->getInteger())
			continue;
	}
}
*/


/*
==================
SV_CheckTimeouts

If a packet has not been received from a client for timeout->value
seconds, drop the conneciton.  Server frames are used instead of
realtime to avoid dropping the local client while debugging.

When a client is normally dropped, the sv_client_c goes into a zombie state
for a few seconds to make sure any final reliable message gets resent
if necessary
==================
*/
static void 	SV_CheckTimeouts()
{
	int			droppoint;
	int			zombiepoint;

	droppoint = (int)(svs.realtime - 1000*timeout->getValue());
	zombiepoint = (int)(svs.realtime - 1000*zombietime->getValue());

	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
			
		// message times may be wrong across a changelevel
		if(cl->getLastMessageTime() > svs.realtime)
			cl->setLastMessageTime(svs.realtime);

		if(cl->getState() == CS_ZOMBIE && cl->getLastMessageTime() < zombiepoint)
		{
			cl->setState(CS_FREE);	// can now be reused
			continue;
		}
		
		if((cl->getState() == CS_CONNECTED || cl->getState() == CS_SPAWNED) && cl->getLastMessageTime() < droppoint)
		{
			SV_BroadcastPrintf(PRINT_HIGH, "'%s' timed out\n", cl->getName());
			cl->drop(); 
			cl->setState(CS_FREE);	// don't bother with zombie state
		}
	}
}

/*
================
SV_PrepWorldFrame

This has to be done before the world logic, because
player processing happens outside RunWorldFrame
================
*/
static void 	SV_PrepWorldFrame()
{
	for(std::vector<sv_entity_c*>::const_iterator ir = ge->entities->begin(); ir != ge->entities->end(); ir++)
	{
		sv_entity_c *ent = *ir;
		
		if(!ent)
			continue;
		
		// events only last for a single message
		ent->_s.event = 0;
	}
}


static void 	SV_RunGameFrame()
{
	if(com_speeds->getInteger())
		time_before_game = Sys_Milliseconds();

	// we always need to bump framenum, even if we
	// don't run the world, otherwise the delta
	// compression can get confused when a client
	// has the "current" frame
	sv.framenum++;
	
	sv.time = sv.framenum * FRAMETIME;

	// don't run if paused
	if(!sv_paused->getInteger() || maxclients->getInteger() > 1)
	{
		//for(int i=0; i<5; i++)
			ge->G_RunFrame();

		// never get more than one tic behind
		if((int)sv.time < svs.realtime)
		{
			if(sv_showclamp->getInteger())
				Com_Printf("sv highclamp\n");
				
			svs.realtime = sv.time;
		}
	}

	if(com_speeds->getInteger())
		time_after_game = Sys_Milliseconds();
}

void 	SV_Frame(int msec)
{
	time_before_game = time_after_game = 0;

	// if server is not active, do nothing
	if(!svs.initialized)
		return;

    	svs.realtime += msec;

	// keep the random time dependent
	rand();

	// check timeouts
	SV_CheckTimeouts();

	// get packets from clients
//	SV_ReadPackets();

	// move autonomous things around if enough time has passed
	if(!sv_timedemo->getInteger() && svs.realtime < (int)sv.time)
	{
		// never let the time get too far off
		if(sv.time - svs.realtime > FRAMETIME)
		{
			if(sv_showclamp->getInteger())
				Com_Printf("sv lowclamp\n");
				
			svs.realtime = sv.time - FRAMETIME;
		}
		Sys_SleepNet(sv.time - svs.realtime);
		return;
	}

	// update ping based on the last known frame from all clients
	SV_CalcPings();

	// give the clients some timeslices
	SV_GiveMsec();

	// let everything in the world think and move
	SV_RunGameFrame();

	// send messages back to the clients that had packets read this frame
	SV_SendClientMessages();

	// send a heartbeat to the master if needed
	Master_Heartbeat();

	// clear teleport flags, etc for next frame
	SV_PrepWorldFrame();
}



/*
================
Master_Heartbeat

Send a message to the master every few minutes to
let it know we are alive, and log information
================
*/
#define	HEARTBEAT_SECONDS	300
void 	Master_Heartbeat()
{
	const char		*string;
	int			i;

	// pgm post3.19 change, cvar pointer not validated before dereferencing
	if(!dedicated || !dedicated->getInteger())
		return;		// only dedicated servers send heartbeats

	// pgm post3.19 change, cvar pointer not validated before dereferencing
	if(!public_server || !public_server->getInteger())
		return;		// a private dedicated game

	// check for time wraparound
	if(svs.last_heartbeat > svs.realtime)
		svs.last_heartbeat = svs.realtime;

	if(svs.realtime - svs.last_heartbeat < HEARTBEAT_SECONDS*1000)
		return;		// not time to send yet

	svs.last_heartbeat = svs.realtime;

	// send the same string that we would give for a status OOB command
	string = SV_StatusString();

	// send to group master
	for(i=0; i<MAX_MASTERS; i++)
	{
		if(master_adr[i].port)
		{
			Com_Printf("Sending heartbeat to %s\n", Sys_AdrToString(master_adr[i]));
			Netchan_OutOfBandPrint(master_adr[i], "heartbeat\n%s", string);
		}
	}
}

/*
=================
Master_Shutdown

Informs all masters that this server is going down
=================
*/
void 	Master_Shutdown()
{
	// pgm post3.19 change, cvar pointer not validated before dereferencing
	if(!dedicated || !dedicated->getInteger())
		return;		// only dedicated servers send heartbeats

	// pgm post3.19 change, cvar pointer not validated before dereferencing
	if(!public_server || !public_server->getInteger())
		return;		// a private dedicated game

	// send to group master
	for(int i=0; i<MAX_MASTERS; i++)
	{
		if(master_adr[i].port)
		{
			if(i > 0)
				Com_Printf("Sending heartbeat to %s\n", Sys_AdrToString(master_adr[i]));
				
			Netchan_OutOfBandPrint(master_adr[i], "shutdown");
		}
	}
}




/*
===============
SV_Init

Only called at quake2.exe startup, not for each game
===============
*/
void 	SV_Init()
{
	Com_Printf ("------- SV_Init -------\n");
	
	SV_InitOperatorCommands();

	rcon_password = Cvar_Get("rcon_password", "", 0);
	Cvar_Get("skill", "1", 0);
	Cvar_Get("deathmatch", "1", CVAR_LATCH);
	Cvar_Get("coop", "0", CVAR_LATCH);
	Cvar_Get("dmflags", "0", CVAR_SERVERINFO);
	Cvar_Get("fraglimit", "0", CVAR_SERVERINFO);
	Cvar_Get("timelimit", "0", CVAR_SERVERINFO);
	Cvar_Get("cheats", "1", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO | CVAR_INIT);;
	
	maxclients			= Cvar_Get("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
	hostname			= Cvar_Get("hostname", "noname", CVAR_SERVERINFO | CVAR_ARCHIVE);
	timeout				= Cvar_Get("timeout", "125", 0);
	zombietime			= Cvar_Get("zombietime", "2", 0);
	sv_showclamp			= Cvar_Get("showclamp", "0", CVAR_NONE);
	sv_shownet			= Cvar_Get("sv_shownet", "0", CVAR_NONE);
	sv_paused			= Cvar_Get("paused", "0", 0);
	sv_timedemo			= Cvar_Get("timedemo", "0", 0);
	sv_enforcetime			= Cvar_Get("sv_enforcetime", "0", 0);
	
	sv_allow_download 		= Cvar_Get("allow_download", "1", CVAR_ARCHIVE | CVAR_SERVERINFO);
	sv_allow_download_players  	= Cvar_Get("allow_download_players", "0", CVAR_ARCHIVE | CVAR_SERVERINFO);
	sv_allow_download_models 	= Cvar_Get("allow_download_models", "1", CVAR_ARCHIVE | CVAR_SERVERINFO);
	sv_allow_download_sounds 	= Cvar_Get("allow_download_sounds", "1", CVAR_ARCHIVE | CVAR_SERVERINFO);
	sv_allow_download_maps		= Cvar_Get("allow_download_maps", "1", CVAR_ARCHIVE | CVAR_SERVERINFO);

	sv_noreload			= Cvar_Get("sv_noreload", "0", 0);

	sv_airaccelerate		= Cvar_Get("sv_airaccelerate", "0", CVAR_LATCH);

	public_server			= Cvar_Get("public", "0", 0);

	sv_reconnect_limit		= Cvar_Get("sv_reconnect_limit", "3", CVAR_ARCHIVE);
}

/*
==================
SV_FinalMessage

Used by SV_Shutdown to send a final message to all
connected clients before the server goes down.  The messages are sent immediately,
not just stuck on the outgoing message list, because the server is going
to totally exit after returning from this function.
==================
*/
static void 	SV_FinalMessage(const std::string &message, bool reconnect)
{
	bitmessage_c msg(MAX_PACKETLEN);
	
	msg.writeBits(SVC_PRINT, svc_bitcount);
	msg.writeByte(PRINT_HIGH);
	msg.writeString(message.c_str());

	if(reconnect)
		msg.writeBits(SVC_RECONNECT, svc_bitcount);
	else
		msg.writeBits(SVC_DISCONNECT, svc_bitcount);

	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ++ir)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
			
		if(cl->getState() >= CS_CONNECTED)
		{
			// send it twice
			// stagger the packets to crutch operating system limited buffers
			cl->netchan.transmit(msg);
			cl->netchan.transmit(msg);
		}
	}
}



/*
================
SV_Shutdown

Called when each game quits,
before Sys_Quit or Sys_Error
================
*/
void 	SV_Shutdown(char *finalmsg, bool reconnect)
{
	Com_Printf("------- SV_Shutdown -------\n");
	
	if(svs.clients.size())
		SV_FinalMessage(finalmsg, reconnect);

	Master_Shutdown();
	
	SV_ShutdownGameProgs();
		
	sv.clear();
	Com_SetServerState(sv.state);

	// free server static data
	X_purge<std::vector<sv_client_c*> >(svs.clients);
	
	if(svs.client_entities)
		Com_Free(svs.client_entities);
		
	svs.clear();
}

