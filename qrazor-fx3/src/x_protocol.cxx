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


/// includes ===================================================================
// system -------------------------------------------------------------------
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "x_protocol.h"

// xreal --------------------------------------------------------------------

const int	svc_bitcount = bitCount(SVC_LAST);
const char*	svc_strings[] =
{
	"SVC_BAD",
	
	"SVC_MUZZLEFLASH",
	"SVC_TEMP_ENTITY",
	"SVC_LAYOUT",
	"SVC_INVENTORY",
	
	"SVC_NOP",
	"SVC_DISCONNECT",
	"SVC_RECONNECT",
	"SVC_SOUND",
	"SVC_PRINT",
	"SVC_STUFFTEXT",
	"SVC_SERVERDATA",
	"SVC_CONFIGSTRING",
	"SVC_SPAWNBASELINE",
	"SVC_CENTERPRINT",
	"SVC_DOWNLOAD",
	"SVC_AREABITS",
	"SVC_PLAYERINFO",
	"SVC_PACKETENTITIES",
	"SVC_DELTAPACKETENTITIES",
	"SVC_FRAME",
};

const int	clc_bitcount = 8;//bitCount(CLC_LAST);	FIXME
const char*	clc_strings[] =
{
	"CLC_BAD",
	"CLC_NOP",
	"CLC_MOVE",
	"CLC_USERINFO",
	"CLC_STRINGCMD",
};


const int	te_bitcount = bitCount(TE_LAST);
const char*	te_strings[] =
{
	"TE_GUNSHOT",
	"TE_BLOOD",
	"TE_BLASTER",
	"TE_RAILTRAIL",
	"TE_SHOTGUN",
	"TE_EXPLOSION1",
	"TE_EXPLOSION2",
	"TE_ROCKET_EXPLOSION",
	"TE_GRENADE_EXPLOSION",
	"TE_SPARKS",
	"TE_SPLASH",
	"TE_BUBBLETRAIL",
	"TE_SCREEN_SPARKS",
	"TE_SHIELD_SPARKS",
	"TE_BULLET_SPARKS",
	"TE_LASER_SPARKS",
	"TE_ROCKET_EXPLOSION_WATER",
	"TE_GRENADE_EXPLOSION_WATER",
	"TE_BFG_EXPLOSION",
	"TE_BFG_BIGEXPLOSION",
	"TE_BFG_LASER",
	"TE_CONTACT"
};


const float DEFAULT_SOUND_PACKET_VOLUME = 1.0f;
const float DEFAULT_SOUND_PACKET_ATTENUATION = 1.0f;



