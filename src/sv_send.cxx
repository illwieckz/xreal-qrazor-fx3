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

#include "cm.h"
#include "vfs.h"



/*
=============================================================================

Com_Printf redirection

=============================================================================
*/

char sv_outputbuf[SV_OUTPUTBUF_LENGTH];

void 	SV_FlushRedirect(char *outputbuf, const netadr_t &adr)
{
	Netchan_OutOfBandPrint(adr, "print\n%s", outputbuf);
}


/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/


/*
=================
SV_BroadcastPrintf

Sends text to all active clients
=================
*/
void 	SV_BroadcastPrintf(g_print_level_e level, const char *fmt, ...)
{
	va_list		argptr;
	char		string[2048];
	
	va_start(argptr,fmt);
	vsprintf(string, fmt, argptr);
	va_end(argptr);
	
	// echo to console
	if(dedicated->getInteger())
	{
		char	copy[1024];
		int		i;
		
		// mask off high bits
		for(i=0 ; i<1023 && string[i]; i++)
			copy[i] = string[i]&127;
		copy[i] = 0;
		Com_Printf("%s", copy);
	}

	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
			
		if(cl->getState() != CS_SPAWNED)
			continue;
		
		if(cl->getEntity()->_r.isbot)
			continue;
			
		cl->printf(level, "%s", string);
	}
}

/*
=================
SV_BroadcastCommand

Sends text to all active clients
=================
*/
void 	SV_BroadcastCommand(const char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	
	if(!sv.state)
		return;
		
	va_start(argptr,fmt);
	vsprintf(string, fmt,argptr);
	va_end(argptr);

	sv.multicast.writeBits(SVC_STUFFTEXT, svc_bitcount);
	sv.multicast.writeString(string);
	
	SV_Multicast(vec3_origin, MULTICAST_ALL_R);
}


/*
=================
SV_Multicast

Sends the contents of sv.multicast to a subset of the clients,
then clears sv.multicast.

MULTICAST_ALL	same as broadcast (origin can be NULL)
MULTICAST_PVS	send to clients potentially visible from org
MULTICAST_PHS	send to clients potentially hearable from org
=================
*/
void 	SV_Multicast(const vec3_c &origin, multicast_type_e to)
{
	//byte*		mask;
	//int		leafnum, cluster;
	bool		reliable = false;
	int		area1, area2;
	

	if(to != MULTICAST_ALL_R && to != MULTICAST_ALL)
	{
		//leafnum = CM_PointLeafnum(origin);
		//area1 = CM_LeafArea(leafnum);
		area1 = CM_PointAreanum(origin);
	}
	else
	{
		//leafnum = 0;	// just to avoid compiler warnings
		area1 = 0;
	}
	
	switch(to)
	{
		case MULTICAST_ALL_R:
			reliable = true;	// intentional fallthrough
		case MULTICAST_ALL:
			//leafnum = 0;
			//mask = NULL;
			break;

		case MULTICAST_PHS_R:
		case MULTICAST_PVS_R:
			reliable = true;	// intentional fallthrough
		case MULTICAST_PHS:
		case MULTICAST_PVS:
		
			//leafnum = CM_PointLeafnum(origin);
			//cluster = CM_LeafCluster(leafnum);
			//mask = CM_ClusterPVS(cluster);
			break;

		default:
			//mask = NULL;
			Com_Error(ERR_FATAL, "SV_Multicast: bad to:%i", to);
	}

	// send the data to all relevent clients
	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
			
		if(cl->getState() < CS_CONNECTED)
			continue;
			
		if(cl->getState() != CS_SPAWNED && !reliable)
			continue;

		//if(mask)
		{
		//	leafnum = CM_PointLeafnum(client->entity->_s.origin);
		//	cluster = CM_LeafCluster(leafnum);
		//	area2 = CM_LeafArea(leafnum);
			area2 = CM_PointAreanum(cl->getEntity()->_s.origin);
			
			if(!CM_AreasConnected(area1, area2))
				continue;
				
			//if(mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
			//	continue;
		}

		if(reliable)
			cl->netchan.message.writeMessage(sv.multicast);
		else
			cl->getDatagram().writeMessage(sv.multicast);
	}

	sv.multicast.beginWriting();
}


/*  
==================
SV_StartSound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

If cahnnel & 8, the sound will be sent to everyone, not just
things in the PHS.

FIXME: if entity isn't in PHS, they must be forced to be sent or
have the origin explicitly sent.

Channel 0 is an auto-allocate channel, the others override anything
already running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)

Timeofs can range from 0.0 to 0.1 to cause sounds to be started
later in the frame than they normally would.

If origin is NULL, the origin is determined from the entity origin
or the midpoint of the entity box for bmodels.
==================
*/  
void 	SV_StartSound(vec3_t origin, sv_entity_c *entity, int channel, int soundindex, float volume, float attenuation, float timeofs)
{       
	int			sendchan;
	int			flags;
	int			i;
	int			ent;
	vec3_t		origin_v;
	bool		use_phs;
	
	if(!entity)
		return;
		
	ent = SV_GetNumForEntity(entity);
	
	if(ent == -1)
		return;
		

	if(volume < 0 || volume > 1.0)
		Com_Error (ERR_FATAL, "SV_StartSound: volume = %f", volume);

	if(attenuation < 0 || attenuation > 4)
		Com_Error (ERR_FATAL, "SV_StartSound: attenuation = %f", attenuation);

//	if(channel < 0 || channel > 15)
//		Com_Error (ERR_FATAL, "SV_StartSound: channel = %i", channel);

	if(timeofs < 0 || timeofs > 0.255)
		Com_Error (ERR_FATAL, "SV_StartSound: timeofs = %f", timeofs);

	
	if(channel & 8)	// no PHS flag
	{
		use_phs = false;
		channel &= 7;
	}
	else
		use_phs = true;

	sendchan = (ent<<3) | (channel&7);

	flags = 0;
	
	if(volume != DEFAULT_SOUND_PACKET_VOLUME)
		flags |= SND_VOLUME;
	if(attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		flags |= SND_ATTENUATION;

	// the client doesn't know that bmodels have weird origins
	// the origin can also be explicitly set
	if((entity->_r.svflags & SVF_NOCLIENT)  || !entity->_r.networksync || (entity->_r.solid == SOLID_BSP) || origin)
		flags |= SND_POS;

	// always send the entity number for channel overrides
	flags |= SND_ENT;

	if(timeofs)
		flags |= SND_OFFSET;

	// use the entity origin unless it is a bmodel or explicitly specified
	if(!origin)
	{
		origin = origin_v;
		if(entity->_r.solid == SOLID_BSP)
		{
			for(i=0 ; i<3 ; i++)
				origin_v[i] = entity->_s.origin[i]+0.5*(entity->_r.bbox._mins[i]+entity->_r.bbox._maxs[i]);
		}
		else
		{
			Vector3_Copy (entity->_s.origin, origin_v);
		}
	}

	sv.multicast.writeBits(SVC_SOUND, svc_bitcount);
	sv.multicast.writeByte(flags);
	sv.multicast.writeByte(soundindex);

	if(flags & SND_VOLUME)
		sv.multicast.writeFloat(volume/**255*/);
		
	if(flags & SND_ATTENUATION)
		sv.multicast.writeFloat(attenuation/**64*/);
		
	if(flags & SND_OFFSET)
		sv.multicast.writeFloat(timeofs/**1000*/);

	if(flags & SND_ENT)
		sv.multicast.writeShort(sendchan);

	if(flags & SND_POS)
		sv.multicast.writeVec3(origin);

	// if the sound doesn't attenuate,send it to everyone
	// (global radio chatter, voiceovers, etc)
	if(attenuation == ATTN_NONE)
		use_phs = false;

	if(channel & CHAN_RELIABLE)
	{
		if(use_phs)
			SV_Multicast(origin, MULTICAST_PHS_R);
		else
			SV_Multicast(origin, MULTICAST_ALL_R);
	}
	else
	{
		if(use_phs)
			SV_Multicast(origin, MULTICAST_PHS);
		else
			SV_Multicast(origin, MULTICAST_ALL);
	}
}           


void 	SV_SendClientMessages()
{
	// send a message to each connected client
	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ++ir)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
			
		if(cl->getState() == CS_FREE)
			continue;
			
		// if the reliable message overflowed, drop the client
		if(cl->netchan.message.isOverFlowed())
		{
			cl->netchan.message.beginWriting();
			cl->getDatagram().beginWriting();
			
			SV_BroadcastPrintf(PRINT_HIGH, "'%s' overflowed\n", cl->getName());
			
			cl->drop();
		}

		if(cl->getState() == CS_SPAWNED)
		{
			// don't overrun bandwidth
			if(cl->rateDrop())
				continue;

			cl->sendDatagram();
		}
		else
		{
			// just update reliable	if needed
			if(cl->netchan.message.getCurSize() || Sys_Milliseconds() - cl->netchan.getLastSent() > 1000)
				cl->netchan.transmit(bitmessage_c());
		}
	}
}

