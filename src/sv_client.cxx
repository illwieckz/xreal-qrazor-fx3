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

// xreal --------------------------------------------------------------------


sv_client_c::sv_client_c(const netadr_t &adr, int qport, int challenge)
{
	netchan.setup(adr, qport, false);

	_state = CS_CONNECTED;
	
	_datagram = bitmessage_c(tobits(MAX_MSGLEN), true);
	
	_lastmessage_time = svs.realtime;	// don't timeout
	_lastconnect_time = svs.realtime;
	
	_challenge = challenge;
}
	
sv_client_c::~sv_client_c()
{
	//TODO
}


/*
=================
SV_UserinfoChanged

Pull specific info from a newly changed userinfo string
into a more C freindly form.
=================
*/
void	sv_client_c::extractUserInfo()
{
	const char	*val;
	int		i;

	// call prog code to allow overrides
	ge->G_ClientUserinfoChanged(_entity, _userinfo);
	
	// name for C code
	strncpy(_name, _userinfo.getValueForKey("name"), sizeof(_name)-1);
	
	// mask off high bit
	for(i=0; i<(int)sizeof(_name); i++)
		_name[i] &= 127;

	// rate command
	val = _userinfo.getValueForKey("rate");
	if(strlen(val))
	{
		i = atoi(val);
		_rate = i;
		
		if(_rate < 100)
			_rate = 100;
		
		if(_rate > 15000)
			_rate = 15000;
	}
	else
	{
		_rate = 5000;
	}

	// msg command
	val = _userinfo.getValueForKey("msg");
	if(strlen(val))
	{
		_messagelevel = atoi(val);
	}
}


/*
=================
SV_ClientPrintf

Sends text across to be displayed if the level passes
=================
*/
void	sv_client_c::printf(g_print_level_e level, const char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	
	if(level < _messagelevel)
		return;
	
	va_start(argptr,fmt);
	vsprintf(string, fmt,argptr);
	va_end(argptr);
	
	netchan.message.writeByte(SVC_PRINT);
	netchan.message.writeByte(level);
	netchan.message.writeString(string);
}



bool	sv_client_c::sendDatagram()
{
	buildFrame();

	bitmessage_c msg(tobits(MAX_MSGLEN), true);

	// send over all the relevant entity_state_t
	// and the player_state_t
	writeFrame(msg);

	// copy the accumulated multicast datagram
	// for this client out to the message
	// it is necessary for this to be after the WriteEntities
	// so that entity references will be current
	if(_datagram.isOverFlowed())
		Com_Error(ERR_WARNING, "sv_client_c::sendDatagram: datagram overflowed for '%s'\n", _name);
	else
		msg.writeMessage(_datagram);
	
	_datagram.beginWriting();

	if(msg.isOverFlowed())
	{
		// must have room left for the packet header
		Com_Error(ERR_WARNING, "sv_client_c::sendDatagram: msg overflowed for '%s'\n", _name);
		msg.beginWriting();
	}

	// send the datagram
	netchan.transmit(msg);

	// record the size for rate estimation
	_message_size[sv.framenum % RATE_MESSAGES] = msg.getCurSize() / 8;

	return true;
}

/*
=============
SV_BuildClientFrame

Decides which entities are going to be visible to the client, and
copies off the playerstat and areabits.
=============
*/
void	sv_client_c::buildFrame()
{
	vec3_c			org;
	int			area;
	sv_client_frame_t*	frame;
	entity_state_t*		state;	

	if(!_entity->_r.isclient)
	{
		Com_Printf("SV_BuildClientFrame: _entity->_r.isclient is false for entity %i\n", SV_GetNumForEntity(_entity));
		return;		// not in game yet
	}

	// this is the frame we are creating
	frame = &_frames[sv.framenum & UPDATE_MASK];

	frame->senttime = svs.realtime; // save it for ping calc later

	// find the client's PVS
	org = _entity->_r.ps.pmove.origin + _entity->_r.ps.view_offset;

	area = _entity->_r.area = CM_PointAreanum(org);
	
	//Com_Printf("SV_BuildClientFrame: client area %i\n", area);

	// calculate the visible areas
	frame->areabytes = CM_WriteAreaBits(frame->areabits, _entity->_r.area);

	// grab the current player_state_t
	frame->ps = _entity->_r.ps;

	// save client's PVS so it can be later compared with fatpvs
	//fatPVS(org);
	
	//memcpy(_clientpvs, _fatpvs, CM_ClusterSize());
	

	// build up the list of visible entities
	frame->num_entities = 0;
	frame->first_entity = svs.next_client_entities;

	//Com_Printf("SV_BuildClientFrame: edicts num: %i\n", ge->edicts->size());

	for(std::vector<sv_entity_c*>::const_iterator ir = ge->entities->begin() + 1; ir != ge->entities->end(); ir++)
	{	
		sv_entity_c *ent = *ir;
		
		// ignore free entity slots
		if(!ent)
			continue;

		// ignore ents without visible models
		if(ent->_r.svflags & SVF_NOCLIENT)
			continue;

		// ignore ents without visible models unless they have an effect
		if(
			!ent->_s.index_model &&
			!ent->_s.index_sound &&
			!ent->_s.index_light &&
			!ent->_s.effects &&
			!ent->_s.event
		)
			continue;

		// ignore if not touching a PV leaf
		if(ent != _entity)
		{
#if 0
			// check area
			if(!CM_AreasConnected(_entity->_r.area, ent->_r.area))
			{	
				// doors can legally straddle two areas, so
				// we may need to check another one
				if(!ent->_r.area2 || !CM_AreasConnected(_entity->_r.area, ent->_r.area2))
					continue;		// blocked by a door
			}

			byte*		bitvector;

			// FIXME: if an ent has a model and a sound, but isn't
			// in the PVS, only the PHS, clear the model
			if(ent->_s.index_sound)
			{
				bitvector = _fatpvs;	//clientphs;
			}
			else
				bitvector = _fatpvs;
				
			if(cullEntity(ent, bitvector))
				continue;
				
#endif
		}



		// add it to the circular client_entities array
		state = &svs.client_entities[svs.next_client_entities%svs.num_client_entities];
		
		*state = ent->_s;

		// don't mark players missiles as solid
		//if(ent->_r.owner == _entity)
		//	state->solid = 0;

		svs.next_client_entities++;
		frame->num_entities++;
	}
	
	//Com_Printf("SV_BuildClientFrame: entities num: %i\n", frame->num_entities);
}


/*
The client will interpolate the view position,
so we can't use a single PVS point
*/
/*
void	sv_client_c::fatPVS(const vec3_c &org)
{
	int		leafs[128];
	int		i, j, count;
	int		longs;
	byte	*src;

	cbbox_c	bbox;

	for(i=0; i<3; i++)
	{
		bbox._mins[i] = org[i] - 8;
		bbox._maxs[i] = org[i] + 8;
	}

	count = CM_BoxLeafnums(bbox, leafs, 128, NULL);
	
	if(count < 1)
		Com_Error(ERR_FATAL, "SV_FatPVS: count < 1");
		
	longs = CM_ClusterSize()>>2;

	// convert leafs to clusters
	for(i=0; i<count; i++)
		leafs[i] = CM_LeafCluster(leafs[i]);

	memcpy(_fatpvs, CM_ClusterPVS(leafs[0]), longs<<2);
	
	// or in all the other leaf bits
	for(i=1; i<count; i++)
	{
		for(j=0; j<i; j++)
		{
			if(leafs[i] == leafs[j])
				break;
		}
		
		if(j != i)
			continue;		// already have the cluster we want
			
		src = CM_ClusterPVS(leafs[i]);
		
		for(j=0; j<longs; j++)
			((long *)_fatpvs)[j] |= ((long *)src)[j];
	}
}
*/

/*
static void	SV_MergePVS(const vec3_c &org)
{
	int		leafs[128];
	int		i, j, count;
	int		longs;
	byte	*src;

	cbbox_c	bbox;

	for(i=0; i<3; i++)
	{
		bbox._mins[i] = org[i] - 1;
		bbox._maxs[i] = org[i] + 1;
	}

	count = CM_BoxLeafnums(bbox, leafs, 128, NULL);
	
	if(count < 1)
		Com_Error(ERR_FATAL, "SV_MergePVS: count < 1");
		
	longs = CM_ClusterSize()>>2;
	//longs = (CM_NumClusters()+31)>>5;

	// convert leafs to clusters
	for(i=0; i<count; i++)
		leafs[i] = CM_LeafCluster(leafs[i]);
	
	// or in all the other leaf bits
	for(i=1; i<count; i++)
	{
		for(j=0; j<i; j++)
		{
			if(leafs[i] == leafs[j])
				break;
		}
		
		if(j != i)
			continue;		// already have the cluster we want
			
		src = CM_ClusterPVS(leafs[i]);
		
		for(j=0; j<longs; j++)
			((long *)fatpvs)[j] |= ((long *)src)[j];
	}
}
*/


bool	sv_client_c::cullEntity(sv_entity_c *ent, byte *bitvector)
{
	/*
	ent->_r.headnode = 0;

	CM_BoxLeafnums(ent->_r.bbox, NULL, 0, &ent->_r.headnode);
	
	if(!CM_HeadnodeVisible(ent->_r.headnode, bitvector))
		return true;
	*/
	
	return false;
}

/*
=============
SV_EmitPacketEntities

Writes a delta update of an entity_state_t list to the message.
=============
*/
void	sv_client_c::writePacketEntities(sv_client_frame_t *from, sv_client_frame_t *to, bitmessage_c &msg)
{
	int		oldindex, newindex;
	int		oldnum, newnum;
	int		from_num_entities;

	entity_state_t	*oldent = NULL;
	entity_state_t	*newent = NULL;

	msg.writeByte(SVC_PACKETENTITIES);

	if(!from)
		from_num_entities = 0;
	else
		from_num_entities = from->num_entities;

	newindex = 0;
	oldindex = 0;
	while(newindex < to->num_entities || oldindex < from_num_entities)
	{
		if(newindex >= to->num_entities)
			newnum = 9999;
		else
		{
			newent = &svs.client_entities[(to->first_entity+newindex)%svs.num_client_entities];
			newnum = newent->getNumber();
		}

		if(oldindex >= from_num_entities)
			oldnum = 9999;
		else
		{
			oldent = &svs.client_entities[(from->first_entity+oldindex)%svs.num_client_entities];
			oldnum = oldent->getNumber();
		}

		if(newnum == oldnum)
		{	
			// delta update from old position
			// because the force parm is false, this will not result
			// in any bytes being emited if the entity has not changed at all
			// note that players are always 'newentities', this updates their oldorigin always
			// and prevents warping
			msg.writeDeltaEntity(oldent, newent, false);
			oldindex++;
			newindex++;
			continue;
		}

		if(newnum < oldnum)
		{	
			// this is a new entity, send it from the baseline
			msg.writeDeltaEntity(&sv.baselines[newnum], newent, true);
			newindex++;
			continue;
		}
				
		if(newnum > oldnum)
		{
			// the old entity isn't present in the new message
			msg.writeDeltaEntity(oldent, NULL, false);
			oldindex++;
			continue;
		}
	}

	msg.writeShort(0);	// end of packetentities
}


void	sv_client_c::writePlayerState(sv_client_frame_t *from, sv_client_frame_t *to, bitmessage_c &msg)
{
	int				i;
	int				pflags;
	player_state_t	*ps, *ops;
	player_state_t	dummy;
	int				statbits;

	ps = &to->ps;
	if(!from)
	{
		memset(&dummy, 0, sizeof(dummy));
		ops = &dummy;
	}
	else
		ops = &from->ps;

	//
	// determine what needs to be sent
	//
	pflags = 0;

	if(	ps->pmove.pm_type != ops->pmove.pm_type)
		pflags |= PS_M_TYPE;

	if(	ps->pmove.origin[0] != ops->pmove.origin[0]
		|| ps->pmove.origin[1] != ops->pmove.origin[1]
		|| ps->pmove.origin[2] != ops->pmove.origin[2] )
		pflags |= PS_M_ORIGIN;

	if(	ps->pmove.velocity_linear[0] != ops->pmove.velocity_linear[0]
		|| ps->pmove.velocity_linear[1] != ops->pmove.velocity_linear[1]
		|| ps->pmove.velocity_linear[2] != ops->pmove.velocity_linear[2] )
		pflags |= PS_M_VELOCITY;

	if(	ps->pmove.pm_time != ops->pmove.pm_time)
		pflags |= PS_M_TIME;

	if(	ps->pmove.pm_flags != ops->pmove.pm_flags)
		pflags |= PS_M_FLAGS;

	if(	ps->pmove.gravity != ops->pmove.gravity)
		pflags |= PS_M_GRAVITY;

	if(	ps->pmove.delta_angles[0] != ops->pmove.delta_angles[0]
		|| ps->pmove.delta_angles[1] != ops->pmove.delta_angles[1]
		|| ps->pmove.delta_angles[2] != ops->pmove.delta_angles[2] )
		pflags |= PS_M_DELTA_ANGLES;


	if(	ps->view_offset[0] != ops->view_offset[0]
		|| ps->view_offset[1] != ops->view_offset[1]
		|| ps->view_offset[2] != ops->view_offset[2] )
		pflags |= PS_VIEW_OFFSET;

	if(	ps->view_angles[0] != ops->view_angles[0]
		|| ps->view_angles[1] != ops->view_angles[1]
		|| ps->view_angles[2] != ops->view_angles[2] )
		pflags |= PS_VIEW_ANGLES;

	if(	ps->kick_angles[0] != ops->kick_angles[0]
		|| ps->kick_angles[1] != ops->kick_angles[1]
		|| ps->kick_angles[2] != ops->kick_angles[2] )
		pflags |= PS_KICK_ANGLES;

	if(	ps->blend[0] != ops->blend[0]
		|| ps->blend[1] != ops->blend[1]
		|| ps->blend[2] != ops->blend[2]
		|| ps->blend[3] != ops->blend[3] )
		pflags |= PS_BLEND;

	if(ps->fov != ops->fov)
		pflags |= PS_FOV;

	if(ps->rdflags != ops->rdflags)
		pflags |= PS_RDFLAGS;

	if(ps->gun_anim_frame != ops->gun_anim_frame)
		pflags |= PS_WEAPON_ANIMATION_FRAME;
	
	if(ps->gun_anim_index != ops->gun_anim_index)
		pflags |= PS_WEAPON_ANIMATION_INDEX;

	pflags |= PS_WEAPON_MODEL_INDEX;
	

	//
	// write it
	//
	msg.writeByte(SVC_PLAYERINFO);
	msg.writeShort(pflags);

	//
	// write the pmove_state_t
	//
	if(pflags & PS_M_TYPE)
		msg.writeByte(ps->pmove.pm_type);

	if(pflags & PS_M_ORIGIN)
	{
		msg.writeFloat(ps->pmove.origin[0]);
		msg.writeFloat(ps->pmove.origin[1]);
		msg.writeFloat(ps->pmove.origin[2]);
	}

	if(pflags & PS_M_VELOCITY)
	{
		msg.writeFloat(ps->pmove.velocity_linear[0]);
		msg.writeFloat(ps->pmove.velocity_linear[1]);
		msg.writeFloat(ps->pmove.velocity_linear[2]);
	}

	if(pflags & PS_M_TIME)
		msg.writeByte(ps->pmove.pm_time);

	if(pflags & PS_M_FLAGS)
		msg.writeByte(ps->pmove.pm_flags);

	if(pflags & PS_M_GRAVITY)
		msg.writeFloat(ps->pmove.gravity);

	if(pflags & PS_M_DELTA_ANGLES)
	{
		msg.writeFloat(ps->pmove.delta_angles[0]);
		msg.writeFloat(ps->pmove.delta_angles[1]);
		msg.writeFloat(ps->pmove.delta_angles[2]);
	}

	//
	// write the rest of the player_state_t
	//
	if(pflags & PS_VIEW_OFFSET)
	{
		msg.writeFloat(ps->view_offset[0]/**4*/);
		msg.writeFloat(ps->view_offset[1]/**4*/);
		msg.writeFloat(ps->view_offset[2]/**4*/);
	}

	if(pflags & PS_VIEW_ANGLES)
	{
		msg.writeAngle(ps->view_angles[0]);
		msg.writeAngle(ps->view_angles[1]);
		msg.writeAngle(ps->view_angles[2]);
	}

	if(pflags & PS_KICK_ANGLES)
	{
		msg.writeFloat(ps->kick_angles[0]/**4*/);
		msg.writeFloat(ps->kick_angles[1]/**4*/);
		msg.writeFloat(ps->kick_angles[2]/**4*/);
	}

	if(pflags & PS_WEAPON_MODEL_INDEX)
	{
		msg.writeByte(ps->gun_model_index);
	}

	if(pflags & PS_WEAPON_ANIMATION_FRAME)
	{
		msg.writeShort(ps->gun_anim_frame);
		
		msg.writeFloat(ps->gun_offset[0]/**4*/);
		msg.writeFloat(ps->gun_offset[1]/**4*/);
		msg.writeFloat(ps->gun_offset[2]/**4*/);
		msg.writeFloat(ps->gun_angles[0]/**4*/);
		msg.writeFloat(ps->gun_angles[1]/**4*/);
		msg.writeFloat(ps->gun_angles[2]/**4*/);
	}
	
	if(pflags & PS_WEAPON_ANIMATION_INDEX)
	{
		msg.writeByte(ps->gun_anim_index);
	}

	if(pflags & PS_BLEND)
	{
		msg.writeColor(ps->blend);
	}
	
	if(pflags & PS_FOV)
		msg.writeFloat(ps->fov);
		
	if(pflags & PS_RDFLAGS)
		msg.writeByte(ps->rdflags);

	// send stats
	statbits = 0;
	for(i=0; i<MAX_STATS; i++)
		if(ps->stats[i] != ops->stats[i])
			statbits |= 1<<i;
	msg.writeLong(statbits);
	for(i=0; i<MAX_STATS; i++)
		if(statbits & (1<<i))
			msg.writeShort(ps->stats[i]);
}


void	sv_client_c::writeFrame(bitmessage_c &msg)
{
	sv_client_frame_t		*frame, *oldframe;
	int					lastframe;

	//Com_Printf ("%i -> %i\n", _lastframe, sv.framenum);
	// this is the frame we are creating
	frame = &_frames[sv.framenum & UPDATE_MASK];

	if(_lastframe <= 0)
	{	
		// client is asking for a retransmit
		oldframe = NULL;
		lastframe = -1;
	}
	else if(sv.framenum - _lastframe >= (UPDATE_BACKUP - 3))
	{	
		// client hasn't gotten a good message through in a long time
		Com_DPrintf("%s: Delta request from out-of-date packet.\n", _name);
		oldframe = NULL;
		lastframe = -1;
	}
	else
	{	
		// we have a valid message to delta from
		oldframe = &_frames[_lastframe & UPDATE_MASK];
		lastframe = _lastframe;
		
		//if(oldframe->first_entity <= svs.)
		//	Com_DPrintf("%s: Delta request from out-of-date packet.\n", _name);
	}

	msg.writeByte(SVC_FRAME);
	msg.writeLong(sv.framenum);
	msg.writeLong(lastframe);	// what we are delta'ing from
	msg.writeByte(_surpress_count);	// rate dropped packets
	_surpress_count = 0;

	// send over the areabits
	msg.writeByte(frame->areabytes);
	msg.writeBytes(frame->areabits, frame->areabytes);

	// delta encode the playerstate
	writePlayerState(oldframe, frame, msg);

	// delta encode the entities
	writePacketEntities(oldframe, frame, msg);
}


void	sv_client_c::calcPing()
{
	if(!_entity)
		Com_Error(ERR_FATAL, "sv_client_c::calcPing: NULL _entity");
			
	if(_entity->_r.isbot)
		return;
			
			
#if 0
	if(cl->lastframe > 0)
		cl->frame_latency[sv.framenum&(LATENCY_COUNTS-1)] = sv.framenum - cl->lastframe + 1;
	else
		cl->frame_latency[sv.framenum&(LATENCY_COUNTS-1)] = 0;
#endif

	int total = 0;
	int count = 0;
	for(int j=0; j<LATENCY_COUNTS; j++)
	{
		if(_frame_latency[j] > 0)
		{
			count++;
			total += _frame_latency[j];
		}
	}
		
	if(!count)
		_ping = 0;
	else
#if 0
	_ping = total*100/count - 100;
#else
	_ping = total / count;
#endif

	
	if(!_entity->_r.isclient)
	{
		Com_Error(ERR_FATAL, "sv_client_c::calcPing: _entity->_r.isclient is false for entity %i", SV_GetNumForEntity(_entity));
		return;
	}

	// let the game dll know about the ping
	_entity->_r.ping = _ping;
}


/*
=======================
SV_RateDrop

Returns true if the client is over its current
bandwidth estimation and should not be sent another packet
=======================
*/
bool 	sv_client_c::rateDrop()
{
	int total = 0;

	for(int i=0; i<RATE_MESSAGES; i++)
	{
		total += _message_size[i];
	}

	if(total > _rate)
	{
		_surpress_count++;
		_message_size[sv.framenum % RATE_MESSAGES] = 0;
		return true;
	}

	return false;
}


/*
===================
SV_ExecuteClientMessage

The current net_message is parsed for the given client
===================
*/
void	sv_client_c::executeMessage(bitmessage_c &msg)
{
	usercmd_t	nullcmd;
	usercmd_t	oldest, oldcmd, newcmd;
	
	int		net_drop;
	int		stringCmdCount;
	
//	int		checksum;
//	int		checksum_offset;
//	int		checksum_calculated;
	
	bool		move_issued;
	int		lastframe;

	// only allow one move command
	move_issued = false;
	stringCmdCount = 0;
	
	_lastmessage_time = svs.realtime;	// don't timeout

	while(true)
	{
		if(msg.getReadCount() > msg.getCurSize())
		{
			Com_Printf("sv_client_c::executeMessage: bad readcount\n");
			drop();
			return;
		}
		
		if(msg.getReadCount() == msg.getCurSize())
		{
			if(sv_shownet->getInteger())
				Com_Printf("     %5i : END OF MESSAGE from '%s'\n", msg.getReadCount(), _name);
			break;
		}

		int cmd = msg.readByte();
		
		if(sv_shownet->getInteger())
		{
			if(cmd < CLC_FIRST || cmd > CLC_LAST)
				Com_Printf("     %5i : BAD CMD %i from '%s'\n", msg.getReadCount(), cmd, _name);
			else
				Com_Printf("     %5i : %s\n", msg.getReadCount(), clc_strings[cmd]);
		}
				
		switch(cmd)
		{
			default:
			{
				Com_Printf("sv_client_c::executeMessage: unknown client command\n");
				drop();
				return;
			}
			
			case -1:
			case CLC_EOM:
			{
				break;
			}
						
			case CLC_NOP:
			{
				break;
			}
			
			case CLC_USERINFO:
			{
				_userinfo.fromString(msg.readString());
				extractUserInfo();
				break;
			}
			
			case CLC_MOVE:
			{
				if(move_issued)
					return;		// someone is trying to cheat...

				move_issued = true;
				
				/*
				checksum = msg.readLong();
				checksum_offset = msg.getCurSize();
				*/
				
				lastframe = msg.readLong();
				if(lastframe != _lastframe)
				{
					_lastframe = lastframe;
					
					if(_lastframe > 0)
					{
						_frame_latency[_lastframe&(LATENCY_COUNTS-1)] = svs.realtime - _frames[_lastframe & UPDATE_MASK].senttime;
					}
				}
				
				nullcmd.clear();
				
				msg.readDeltaUsercmd(&nullcmd, &oldest);
				msg.readDeltaUsercmd(&oldest, &oldcmd);
				msg.readDeltaUsercmd(&oldcmd, &newcmd);

				if(_state != CS_SPAWNED)
				{
					_lastframe = -1;
					break;
				}
				
				// if the checksum fails, ignore the rest of the packet
				/*
				//calculatedChecksum = Com_BlockSequenceCRCByte(&msg[checksumIndex + 1], msg.getBytesReadCount() - checksumIndex - 1, netchan.getIncomingSequence());
				checksum_calculated = msg.calcCheckSum(checksum_offset);

				if(checksum_calculated != checksum)
				{
					Com_DPrintf("Failed command checksum for %s (%d != %d)/%d\n",	_name, 
													checksum_calculated, 
													checksum,
													netchan.getIncomingSequence());
					return;
				}
				*/

				if(!sv_paused->getInteger())
				{
					net_drop = netchan.getDropped();
					
					if(net_drop < 20)
					{
						//if(net_drop > 2)
						//	Com_Printf("drop %i\n", net_drop);
						
						while(net_drop > 2)
						{
							think(_lastcmd);

							net_drop--;
						}
					
						if(net_drop > 1)
							think(oldest);

						if(net_drop > 0)
							think(oldcmd);

					}
				
					think(newcmd);
				}
				
				_lastcmd = newcmd;
				break;
			}
				
			case CLC_STRINGCMD:
			{
				const char *s = msg.readString();

				// malicious users may try using too many string commands
				if(++stringCmdCount < 8)
					executeCommand(s);
				
				if(_state == CS_ZOMBIE)
					return;	// disconnect command
				break;
			}
		}
	}
}


/*
=====================
SV_DropClient

Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing.
=====================
*/
void 	sv_client_c::drop()
{
	//
	// add the disconnect
	//
	netchan.message.writeByte(SVC_DISCONNECT);


	//
	// disconnect from game
	//
	if(_state == CS_SPAWNED)
	{
		// call the prog function for removing a client
		// this will remove the body, among other things
		ge->G_ClientDisconnect(_entity);
	}


	//
	// stop download
	//
	if(_download)
	{
		VFS_FFree(_download);
		_download = NULL;
	}

	_state = CS_ZOMBIE;		// become free in a few seconds
	_name[0] = 0;
}

void	sv_client_c::kick()
{
	drop();
	_lastmessage_time = svs.realtime;	// min case there is a funny zombie
}





sv_client_c*	SV_GetClientByNum(int num)
{
	if(num < 0 || num >= (int)svs.clients.size())
	{
		Com_Error(ERR_FATAL, "SV_GetClientByNum: bad number %i", num);
		return NULL;
	}
	
	if(!svs.clients[num])
	{
		Com_Error(ERR_FATAL, "SV_GetClientByNum: NULL slot %i", num);
		return NULL;
	}

	return svs.clients[num];

}

int	SV_GetNumForClient(sv_client_c *cl)
{
	if(!cl)
	{
		Com_Error(ERR_FATAL, "SV_GetNumForClient: NULL parameter");
		return -1;
	}
	
	for(unsigned int i=0; svs.clients.size(); i++)
	{
		if(cl == svs.clients[i])
			return i;
	}
	
	Com_Error(ERR_FATAL, "SV_GetNumForClient: bad client pointer for '%s'", cl->getName());
	return -1;
}
