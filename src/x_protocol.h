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



#define	PROTOCOL_VERSION	60

#define	UPDATE_BACKUP		32	// copies of entity_state_t to keep buffered, must be power of two
#define	UPDATE_MASK		(UPDATE_BACKUP-1)

#define TEXTCMD_BACKUP		64	// size of reliable text commands buffer, must be power of two
#define TEXTCMD_MASK		(TEXTCMD_BACKUP-1)

#define MAX_PACKET_USERCMDS	32	// max number of usercmds client can send in one packet


enum net_port_t
{
	PORT_MASTER	= 27900,
	PORT_CLIENT	= 27901,
	PORT_SERVER	= 27910
};



//==================
// the svc_strings[] array in cl_parse.c should mirror this
//==================

//
// server to client
//
enum
{
	SVC_BAD,

	// these should be known to the game dll
	SVC_MUZZLEFLASH,
	SVC_TEMP_ENTITY,
	SVC_LAYOUT,
	SVC_INVENTORY,

	// the rest should be private to the client and server and not used in the game dll
	SVC_NOP,
	SVC_DISCONNECT,
	SVC_RECONNECT,
	SVC_SOUND,				// <see code>
	SVC_PRINT,				// [byte] id [string] null terminated string
	SVC_STUFFTEXT,				// [string] stuffed into client's console buffer, should be \n terminated
	SVC_SERVERDATA,				// [long] protocol ...
	SVC_CONFIGSTRING,			// [short] [string]
	SVC_SPAWNBASELINE,		
	SVC_CENTERPRINT,			// [string] to put in center of the screen
	SVC_DOWNLOAD,				// [short] size [size bytes]
	SVC_PLAYERINFO,				// variable
	SVC_PACKETENTITIES,			// [...]
	SVC_DELTAPACKETENTITIES,		// [...]
	SVC_FRAME
};


//
// client to server
//
enum
{
	CLC_BAD,
	CLC_NOP, 		
	CLC_MOVE,			// [[usercmd_t]
	CLC_USERINFO,			// [[userinfo string]
	CLC_STRINGCMD			// [string] message
};

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

// try to pack the common update flags into the first byte
#define U_REMOVE				(1<<0)
#define	U_TYPE					(1<<1)
#define	U_ORIGIN				(1<<2)
#define	U_ORIGIN2				(1<<3)
#define	U_QUATERNION				(1<<4)
#define U_QUATERNION2				(1<<5)
#define U_VELOCITY_LINEAR			(1<<6)
#define U_VELOCITY_ANGULAR			(1<<7)

// second byte
#define	U_INDEX_MODEL				(1<<8)
#define U_INDEX_SHADER				(1<<9)
#define U_INDEX_ANIMATION			(1<<10)
#define	U_INDEX_SOUND				(1<<11)
#define U_INDEX_LIGHT				(1<<12)
#define U_FRAME					(1<<13)
#define	U_EFFECTS				(1<<14)			// autorotate, trails, etc
#define U_RENDERFX				(1<<15)			// fullbright, etc

// third byte
#define	U_EVENT					(1<<16)
#define U_SHADERPARM0				(1<<17)
#define U_SHADERPARM1				(1<<18)
#define U_SHADERPARM2				(1<<19)
#define U_SHADERPARM3				(1<<20)
#define U_SHADERPARM4				(1<<21)
#define U_SHADERPARM5				(1<<22)
#define U_SHADERPARM6				(1<<23)

// fourth byty
#define U_SHADERPARM7				(1<<24)
#define U_VECTOR0				(1<<25)
#define U_VECTOR1				(1<<26)
#define U_VECTOR2				(1<<27)


#endif // X_PROTOCOL_H


