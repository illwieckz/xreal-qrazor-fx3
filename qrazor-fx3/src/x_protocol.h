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



#define	PROTOCOL_VERSION	61

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

//==============================================

// plyer_state_t communication

#define	PS_M_TYPE				(1<<0)
#define	PS_M_ORIGIN				(1<<1)
#define	PS_M_VELOCITY				(1<<2)
#define	PS_M_TIME				(1<<3)
#define	PS_M_FLAGS				(1<<4)
#define	PS_M_GRAVITY				(1<<5)
#define	PS_M_DELTA_ANGLES			(1<<6)

#define	PS_VIEW_OFFSET				(1<<7)
#define	PS_VIEW_ANGLES				(1<<8)
#define	PS_KICK_ANGLES				(1<<9)
#define	PS_BLEND				(1<<10)
#define	PS_FOV					(1<<11)
#define	PS_WEAPON_MODEL_INDEX			(1<<12)
#define	PS_WEAPON_ANIMATION_FRAME		(1<<13)
#define PS_WEAPON_ANIMATION_INDEX		(1<<14)
#define	PS_RDFLAGS				(1<<15)

//==============================================

// usercmd_t communication

// ms and light always sent, the others are optional
#define	CM_ANGLE1				(1<<0)
#define	CM_ANGLE2				(1<<1)
#define	CM_ANGLE3				(1<<2)
#define	CM_FORWARD				(1<<3)
#define	CM_SIDE					(1<<4)
#define	CM_UP					(1<<5)
#define	CM_BUTTONS				(1<<6)

//==============================================

// a sound without an ent or pos will be a local only sound
#define	SND_VOLUME				(1<<0)		// a byte
#define	SND_ATTENUATION				(1<<1)		// a byte
#define	SND_POS					(1<<2)		// three coordinates
#define	SND_ENT					(1<<3)		// a short 0-2: channel, 3-12: entity
#define	SND_OFFSET				(1<<4)		// a byte, msec offset from frame start

#define DEFAULT_SOUND_PACKET_VOLUME		1.0
#define DEFAULT_SOUND_PACKET_ATTENUATION	1.0

//==============================================

// entity_state_t communication
enum
{
	// try to pack the common update flags into the first byte
	U_TYPE					= (1<<0),
	U_ORIGIN				= (1<<1),
	U_ORIGIN2				= (1<<2),
	U_QUATERNION				= (1<<3),
	U_QUATERNION2				= (1<<4),
	U_VELOCITY_LINEAR			= (1<<5),
	U_VELOCITY_ANGULAR			= (1<<6),
	U_INDEX_MODEL				= (1<<7),

	// second byte
	U_INDEX_SHADER				= (1<<8),
	U_INDEX_ANIMATION			= (1<<9),
	U_INDEX_SOUND				= (1<<10),
	U_INDEX_LIGHT				= (1<<11),
	U_FRAME					= (1<<12),
	U_EFFECTS				= (1<<13),			// autorotate, trails, etc
	U_RENDERFX				= (1<<14),			// fullbright, etc
	U_EVENT					= (1<<15),

	// third byte
	U_SHADERPARM0				= (1<<16),
	U_SHADERPARM1				= (1<<17),
	U_SHADERPARM2				= (1<<18),
	U_SHADERPARM3				= (1<<19),
	U_SHADERPARM4				= (1<<20),
	U_SHADERPARM5				= (1<<21),
	U_SHADERPARM6				= (1<<22),
	U_SHADERPARM7				= (1<<23),

	// fourth byte
	U_VECTOR0				= (1<<24),
	U_VECTOR1				= (1<<25),
	U_VECTOR2				= (1<<26),
	
	U_BITS					= 27
};

#endif // X_PROTOCOL_H


