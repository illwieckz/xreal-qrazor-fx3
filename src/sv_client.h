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
#ifndef SV_CLIENT_H
#define SV_CLIENT_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
#include <deque>
#include <queue>

// shared -------------------------------------------------------------------
#include "shared/x_protocol.h"

// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "xreal/g_public.h"


enum sv_client_state_t
{
	CS_FREE,		// can be reused for a new connection
	CS_ZOMBIE,		// client has been disconnected, but don't reuse connection for a couple seconds
	CS_CONNECTED,		// has been assigned to a client_t, but not in game yet
	CS_SPAWNED		// client is fully in game
};

struct sv_client_frame_t
{
	int			areabytes;
	byte			areabits[MAX_BSP_AREAS/8];	// portalarea visibility bits
	player_state_t		ps;
	int			num_entities;
	int			first_entity;			// into the circular sv_packet_entities[]
	int			senttime;			// for ping calculations
};

#define	LATENCY_COUNTS	16
#define	RATE_MESSAGES	10

struct sv_client_c
{
public:
	sv_client_c(const netadr_t &adr, int challenge);
	
	~sv_client_c();
	
	void		extractUserInfo();
	void		printf(g_print_level_e level, const char *fmt, ...);
	
	bool		sendDatagram();
private:
	void		buildFrame();
//	void		fatPVS(const vec3_c &org);
	bool		cullEntity(sv_entity_c *ent, byte *bitvector);
	void		writePacketEntities(sv_client_frame_t *from, sv_client_frame_t *to, message_c &msg);
	void		writePlayerState(sv_client_frame_t *from, sv_client_frame_t *to, message_c &msg);
	void		writeFrame(message_c &msg);
public:
	void		calcPing();
	bool		rateDrop();
	void		executeMessage(message_c &msg);
	void		executeCommand(const char *s);
	void		think(const usercmd_t &cmd);
	
	void 		drop();
	void		kick();
	
private:
	// user commands
	void		new_uc();
	void		configStrings_uc();
	void		baseLines_uc();
	void		begin_uc();
	void		nextDownload_uc();
	void		beginDownload_uc();
	void		disconnect_uc();
	void		showServerInfo_uc();
	void		nextServer();
	void		nextServer_uc();
	
public:
	sv_client_state_t	getState() const		{return _state;}
	const char*		getUserInfo() const		{return _userinfo.toString();}
	const char*		getName() const			{return _name;}
	sv_entity_c*		getEntity() const		{return _entity;}
	int			getCommandTime() const		{return _command_time;}
	int			getPing() const			{return _ping;}
	message_c*		getDatagram()			{return &_datagram;}
	int			getLastMessageTime() const	{return _lastmessage_time;}
	int			getLastConnectTime() const	{return _lastconnect_time;}
	
	void			setState(sv_client_state_t s)	{_state = s;}
	void			setUserInfo(const info_c &userinfo)	{_userinfo = userinfo;}
	void			setEntity(sv_entity_c *ent)	{_entity = ent;}
	void			setLastMessageTime(int time)	{_lastmessage_time = time;}

	void			resetCommandTime()		{_command_time = 1800;}		// 1600 + some slop
private:
	sv_client_state_t	_state;
	
	info_c			_userinfo;				// name, etc
	
	// extracted from userinfo
	char			_name[32];				// high bits masked
	int			_rate;
	int			_messagelevel;				// for filtering printed messages
	
	sv_entity_c*		_entity;				// EDICT_NUM(clientnum+1)

	int			_lastframe;				// for delta compression
	usercmd_t		_lastcmd;				// for filling in big drops

	int			_command_time;				// every seconds this is reset, if user
										// commands exhaust it, assume time cheating

	int			_frame_latency[LATENCY_COUNTS];
	int			_ping;

	int			_message_size[RATE_MESSAGES];		// used to rate drop packets
	int			_surpress_count;			// number of messages rate supressed
	

	// The datagram is written to by sound calls, prints, temp ents, etc.
	// It can be harmlessly overflowed.
	message_c		_datagram;

	sv_client_frame_t	_frames[UPDATE_BACKUP];			// updates can be delta'd from here

	byte*			_download;				// file being downloaded
	int			_download_size;				// total bytes (can't use EOF because of paks)
	int			_download_count;			// bytes sent

	int			_lastmessage_time;			// sv.framenum when packet was last received
	int			_lastconnect_time;

	int			_challenge;				// challenge of this user, randomly generated
									// save it for checksumming

public:
	netchan_c		netchan;
};



sv_client_c*	SV_GetClientByNum(int num);
int		SV_GetNumForClient(sv_client_c *cl);


#endif // SV_CLIENT_H


