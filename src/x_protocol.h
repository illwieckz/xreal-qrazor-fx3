/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
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
#ifndef X_PROTOCOL_H
#define X_PROTOCOL_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"



#define	PROTOCOL_VERSION	62

#define	UPDATE_BACKUP		32	// copies of entity_state_t to keep buffered, must be power of two
#define	UPDATE_MASK		(UPDATE_BACKUP-1)

#define TEXTCMD_BACKUP		64	// size of reliable text commands buffer, must be power of two
#define TEXTCMD_MASK		(TEXTCMD_BACKUP-1)

#define MAX_PACKET_USERCMDS	32	// max number of usercmds client can send in one packet


enum net_port_t
{
	PORT_MASTER	= 27900,
	PORT_CLIENT	= 27901,
	PORT_SERVER	= 27910,
	PORT_ANY	= -1
};


//
// server to client
//
enum
{
	SVC_FIRST,
	
	SVC_BAD				= SVC_FIRST, 
	
	// only the following should be known to the game dll, 
	// the rest should be private to the client and server and not used in the game dll
	SVC_MUZZLEFLASH,
	SVC_TEMP_ENTITY,
	SVC_LAYOUT,
	SVC_INVENTORY,
	
	SVC_NOP,
	SVC_DISCONNECT,
	SVC_RECONNECT,
	SVC_SOUND,					// <see code>
	SVC_PRINT,					// [byte] id [string] null terminated string
	SVC_STUFFTEXT,					// [string] stuffed into client's console buffer, should be \n terminated
	SVC_SERVERDATA,					// [long] protocol ...
	SVC_CONFIGSTRING,				// [short] [string]
	SVC_SPAWNBASELINE,		
	SVC_CENTERPRINT,				// [string] to put in center of the screen
	SVC_DOWNLOAD,					// [short] size [size bytes]
	SVC_AREABITS,
	SVC_PLAYERINFO,					// variable
	SVC_PACKETENTITIES,				// [...]
	SVC_DELTAPACKETENTITIES,			// [...]
	SVC_FRAME,
	
	SVC_LAST			= SVC_FRAME,
};

extern const int	svc_bitcount;
extern const char*	svc_strings[];


//
// client to server
//
enum
{
	CLC_FIRST,

	CLC_BAD				= CLC_FIRST,
	CLC_NOP, 		
	CLC_MOVE,					// [[usercmd_t]
	CLC_USERINFO,					// [[userinfo string]
	CLC_STRINGCMD,					// [string] message
	
	CLC_LAST			= CLC_STRINGCMD
};

extern const int	clc_bitcount;
extern const char*	clc_strings[];



// temp entity events
//
// Temp entity events are for things that happen
// at a location seperate from any existing entity.
// Temporary entity messages are explicitly constructed
// and broadcast.
enum
{
	TE_FIRST,

	TE_GUNSHOT				= TE_FIRST,
	TE_BLOOD,
	TE_BLASTER,
	TE_RAILTRAIL,
	TE_SHOTGUN,
	TE_EXPLOSION1,
	TE_EXPLOSION2,
	TE_ROCKET_EXPLOSION,
	TE_GRENADE_EXPLOSION,
	TE_SPARKS,
	TE_SPLASH,
	TE_BUBBLETRAIL,
	TE_SCREEN_SPARKS,
	TE_SHIELD_SPARKS,
	TE_BULLET_SPARKS,
	TE_LASER_SPARKS,
	TE_ROCKET_EXPLOSION_WATER,
	TE_GRENADE_EXPLOSION_WATER,
	TE_BFG_EXPLOSION,
	TE_BFG_BIGEXPLOSION,
	TE_BFG_LASER,
	TE_CONTACT,
	
	TE_LAST					= TE_CONTACT
};

extern const int	te_bitcount;
extern const char*	te_strings[];


// a sound without an ent or pos will be a local only sound
enum
{
	SND_NONE				= 0,
	SND_VOLUME				= (1<<0),		// a byte
	SND_ATTENUATION				= (1<<1),		// a byte
	SND_POS					= (1<<2),		// three coordinates
	SND_ENT					= (1<<3),		// a short 0-2: channel, 3-12: entity
	SND_OFFSET				= (1<<4),		// a byte, msec offset from frame start
};
	
extern const float DEFAULT_SOUND_PACKET_VOLUME;
extern const float DEFAULT_SOUND_PACKET_ATTENUATION;


#endif // X_PROTOCOL_H


