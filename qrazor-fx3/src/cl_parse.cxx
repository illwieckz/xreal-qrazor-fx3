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


/*
===============
CL_CheckOrDownloadFile

Returns true if the file exists, otherwise it attempts
to start a download from the server.
===============
*/
bool	CL_CheckOrDownloadFile(char *filename)
{
	
	if(strstr(filename, ".."))
	{
		Com_Printf ("Refusing to download a path with: %s\n", filename);
		return true;
	}

	if(VFS_FLoad(filename, NULL) != -1)
	{	
		// it exists, no need to download
		return true;
	}

	cls.download_name = filename;

	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	cls.download_tempname = Com_StripExtension(cls.download_name);
	cls.download_tempname += ".tmp";


	// check to see if we already have a tmp for this file, if so, try to resume
	// open the file if not opened yet
	std::string name = cls.download_tempname;

	VFILE 	*fp = NULL;
	
	VFS_FOpenWrite(name, &fp);
	
	if(fp) 
	{ 
		// it exists
		int len;
		//VFS_FSeek(fp, 0, VFS_SEEK_END);
		//len = VFS_FTell(fp);
		len = VFS_FSize (fp);

		cls.download_stream = fp;

		// give the server an offset to start the download
		Com_Printf("Resuming %s\n", cls.download_name.c_str());
		cls.netchan.message.writeBits(CLC_STRINGCMD, clc_bitcount);
		cls.netchan.message.writeString(va("download %s %i", cls.download_name.c_str(), len));
	} 
	else 
	{
		Com_Printf("Downloading %s\n", cls.download_name.c_str());
		cls.netchan.message.writeBits(CLC_STRINGCMD, clc_bitcount);
		cls.netchan.message.writeString(va("download %s", cls.download_name.c_str()));
	}

	cls.download_number++;

	return false;
}

/*
===============
CL_Download_f

Request a download from the server
===============
*/
void	CL_Download_f()
{
	char filename[MAX_OSPATH];

	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: download <filename>\n");
		return;
	}

	Com_sprintf(filename, sizeof(filename), "%s", Cmd_Argv(1));

	if(strstr(filename, ".."))
	{
		Com_Printf("Refusing to download a path with ..\n");
		return;
	}

	if(VFS_FLoad(filename, NULL) != -1)
	{	
		// it exists, no need to download
		Com_Printf("File already exists.\n");
		return;
	}

	cls.download_name = filename;
	Com_Printf("Downloading %s\n", cls.download_name.c_str());

	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	cls.download_tempname = Com_StripExtension(cls.download_name);
	cls.download_tempname += ".tmp";

	cls.netchan.message.writeBits(CLC_STRINGCMD, clc_bitcount);
	cls.netchan.message.writeString(va("download %s", cls.download_name.c_str()));

	cls.download_number++;
}



/*
=====================
CL_ParseDownload

A download message has been received from the server
=====================
*/
/*
static void	CL_ParseDownload(message_c &msg)
{
	int		size, percent;
	std::string	name;
	int		r;

	// read the data
	size = msg.readShort();
	percent = msg.readByte();
	if(size == -1)
	{
		Com_Printf("Server does not have this file.\n");
		if(cls.download_stream)
		{
			// if here, we tried to resume a file but the server said no
			VFS_FClose(&cls.download_stream);
		}
		CL_RequestNextDownload();
		return;
	}

	// open the file if not opened yet
	if(!cls.download_stream)
	{
		name = cls.download_tempname;

		VFS_Mkdir(name);

		VFS_FOpenWrite(name, &cls.download_stream);
		if(!cls.download_stream)
		{
			msg->readcount_bytes += size;
			Com_Printf("Failed to open %s\n", cls.download_tempname.c_str());
			CL_RequestNextDownload();
			return;
		}
	}

	VFS_FWrite(&msg[msg->getBytesReadCount()], size, cls.download_stream);
	msg->readcount_bytes += size;

	if(percent != 100)
	{
		// request next block
// change display routines by zoid
#if 0
		Com_Printf (".");
		if(10*(percent/10) != cls.download_percent)
		{
			cls.download_percent = 10*(percent/10);
			Com_Printf("%i%%", cls.download_percent);
		}
#endif
		cls.download_percent = percent;

		MSG_WriteBits(&cls.netchan.message, CLC_STRINGCMD, clc_bitcount);
		MSG_Print(&cls.netchan.message, "nextdl");
	}
	else
	{
		std::string	oldn;
		std::string	newn;

		VFS_FClose(&cls.download_stream);

		// rename the temp file to it's final name
		oldn = cls.download_tempname;
		newn = cls.download_name;
		
		//FIXME use VFS_Rename
		r = rename(oldn.c_str(), newn.c_str());
		if(r)
			Com_Printf("failed to rename.\n");
			
		cls.download_percent = 0;

		// get another file if needed

		CL_RequestNextDownload();
	}
}
*/

static void	CL_ParseServerData(bitmessage_c &msg)
{
	std::string	str;
	int		i;
	
	Com_DPrintf("Serverdata packet received.\n");

	//
	// wipe the client_state_t struct
	//
	CL_ClearState();
	cls.state = CA_CONNECTED;

	// parse protocol version number
	i = msg.readLong();
	cls.server_protocol = i;

	if(i != PROTOCOL_VERSION)
		Com_Error(ERR_DROP,"Server returned version %i, not %i", i, PROTOCOL_VERSION);

	cl.servercount = msg.readLong();
	cl.attractloop = msg.readByte();

	// game directory
	str = msg.readString();

	// set gamedir
	if(str.length())
		Cvar_Set("vfs_game", str);

	// parse player entity number
	cl.playernum = msg.readShort();

	// get the full level name
	str = msg.readString();

	if(cl.playernum == -1)
	{	
		// playing a cinematic or showing a pic, not a level
		//cge->CG_PlayCinematic((char*)str.c_str());
	}
	else
	{
		// seperate the printfs so the server message can have a color
		Com_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
		Com_Printf("%c%s\n", 2, str.c_str());

		// need to prep refresh at next oportunity
		cl.refresh_prepped = false;
	}
}



static void	CL_ParseConfigString(bitmessage_c &msg)
{
	int index = msg.readShort();
	if(index < 0 || index >= MAX_CONFIGSTRINGS)
		Com_Error(ERR_DROP, "configstring > MAX_CONFIGSTRINGS");
	
	const char *s = msg.readString();

	char	olds[MAX_QPATH];
	strncpy(olds, cl.configstrings[index], sizeof(olds));
	olds[sizeof(olds) - 1] = 0;

	strcpy(cl.configstrings[index], s);
	
	cge->CG_UpdateConfig(index, cl.configstrings[index]);
}


static void	CL_ParseBaseline(bitmessage_c &msg)
{
	entity_state_t	nullstate;

	uint_t newnum = msg.readBits(MAX_ENTITIES_BITS);
	
	entity_state_t *state = NULL;
	
	try
	{
		state = &cl.entities_baseline.at(newnum);
	}
	catch(...)
	{
		Com_Error(ERR_DROP, "CL_ParseBaseLine: entity number %i out of range", newnum);
	}
		
	msg.readDeltaEntity(&nullstate, state, newnum);
	
	cge->CG_AddEntity(newnum, state);
}

static void	CL_ParseStartSoundPacket(bitmessage_c &msg)
{
	vec3_c  pos_v;
	vec_t	*pos;
	int 	channel, ent;
	int 	sound_num;
	float 	volume;
	float 	attenuation;  
	int	flags;
	float	ofs;

	flags = msg.readByte();
	sound_num = msg.readByte();

	if(flags & SND_VOLUME)
		volume = msg.readFloat();// / 255.0;
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;
	
	if(flags & SND_ATTENUATION)
		attenuation = msg.readFloat();// / 64.0;
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;	

	if(flags & SND_OFFSET)
		ofs = msg.readFloat();// / 1000.0;
	else
		ofs = 0;

	if(flags & SND_ENT)
	{
		// entity reletive
		channel = msg.readShort(); 
		ent = channel>>3;
		if(ent > MAX_ENTITIES)
			Com_Error(ERR_DROP, "CL_ParseStartSoundPacket: ent = %i", ent);

		channel &= 7;
	}
	else
	{
		ent = 0;
		channel = 0;
	}

	if(flags & SND_POS)
	{
		// positioned in space
		msg.readVec3(pos_v);
 
		pos = pos_v;
	}
	else
	{
		// use entity number
		pos = NULL;
	}

	S_StartSound(pos, ent, channel, S_RegisterSound(cl.configstrings[CS_SOUNDS + sound_num]));//, volume, attenuation, ofs);
}       


static void	CL_ShowNet(bitmessage_c &msg, const char *s)
{
	if(cl_shownet->getInteger() >= 2)
		Com_Printf("     %5i : %s\n", msg.getReadCount(), s);
}


static void	CL_DeltaEntity(bitmessage_c &msg, frame_t *frame, int newnum, entity_state_t *state_old, bool changed)
{
	entity_state_t *state = &cl.entities[cl.entities_first % MAX_ENTITIES];
	
	if(changed)
	{
		msg.readDeltaEntity(state_old, state, newnum);
		
		if(state->getNumber() == 0)	// the entity present in oldframe is not in the current frame
		{
			if(cl_shownet->getInteger() == 3)
				Com_Printf("   remove: %i\n", state_old->getNumber());
					
//			cge->CG_RemoveEntity(state_old->getNumber(), state);
			cge->CG_RemoveEntity(newnum, state);
			return;
		}
	}
	else
	{
		*state = *state_old;
	}
	
	cl.entities_first++;
	frame->entities_num++;
	
	cge->CG_UpdateEntity(newnum, state, changed);
}


/*
==================
CL_ParsePacketEntities

An svc_packetentities has just been parsed, deal with the
rest of the data stream.
==================
*/
static void	CL_ParsePacketEntities(bitmessage_c &msg, frame_t *oldframe, frame_t *newframe)
{
	int			newnum;
	int			oldindex, oldnum;
	entity_state_t*		oldstate = NULL;

	newframe->entities_first = cl.entities_first;
	newframe->entities_num = 0;
	
	cge->CG_BeginFrame(cl.frame);

	// delta from the entities present in oldframe
	oldindex = 0;
	if(!oldframe)
	{
		oldnum = 99999;
	}
	else if(oldframe->entities_num <= 0)
	{
		oldnum = 99999;
	}
	else
	{
		oldstate = &cl.entities[(oldframe->entities_first + oldindex) % MAX_ENTITIES];
		oldnum = oldstate->getNumber();
	}

	while(true)
	{
		newnum = msg.readBits(MAX_ENTITIES_BITS);
		
		if(newnum < 0 || newnum >= MAX_ENTITIES)
			Com_Error(ERR_DROP,"CL_ParsePacketEntities: bad number %i", newnum);

		if(msg.getReadCount() > msg.getCurSize())
			Com_Error(ERR_DROP,"CL_ParsePacketEntities: end of message");

		if(!newnum)
			break;

		while(oldnum < newnum)
		{
			// one or more entities from the old packet are unchanged
			if(cl_shownet->getInteger() == 3)
				Com_Printf("   unchanged: %i\n", oldnum);
				
			CL_DeltaEntity(msg, newframe, oldnum, oldstate, false);
			
			oldindex++;

			if(oldindex >= oldframe->entities_num)
			{
				oldnum = 99999;
			}
			else
			{
				oldstate = &cl.entities[(oldframe->entities_first + oldindex) % MAX_ENTITIES];
				oldnum = oldstate->getNumber();
			}
		}
		
		if(oldnum == newnum)
		{
			// delta from previous state
			if(cl_shownet->getInteger() == 3)
				Com_Printf("   delta: %i\n", newnum);
				
			CL_DeltaEntity(msg, newframe, newnum, oldstate, true);

			oldindex++;

			if(oldindex >= oldframe->entities_num)
			{
				oldnum = 99999;
			}
			else
			{
				oldstate = &cl.entities[(oldframe->entities_first + oldindex) % MAX_ENTITIES];
				oldnum = oldstate->getNumber();
			}
			continue;
		}

		if(oldnum > newnum)
		{
			// delta from baseline
			if(cl_shownet->getInteger() == 3)
				Com_Printf("   baseline: %i\n", newnum);
				
			CL_DeltaEntity(msg, newframe, newnum, &cl.entities_baseline[newnum], true);
			continue;
		}

	}

	// any remaining entities in the old frame are copied over
	while(oldnum != 99999)
	{	
		// one or more entities from the old packet are unchanged
		if(cl_shownet->getInteger() == 3)
			Com_Printf("   unchanged: %i\n", oldnum);
			
		CL_DeltaEntity(msg, newframe, oldnum, oldstate, false);
		
		oldindex++;

		if(oldindex >= oldframe->entities_num)
			oldnum = 99999;
		else
		{
			oldstate = &cl.entities[(oldframe->entities_first + oldindex) % MAX_ENTITIES];
			oldnum = oldstate->getNumber();
		}
	}
	
	cge->CG_EndFrame(cl.frame.entities_num);
}

static void	CL_ParsePlayerstate(bitmessage_c &msg, frame_t *oldframe, frame_t *newframe)
{
	int			flags;
	player_state_t	*state;
	int			i;
	int			statbits;

	state = &newframe->playerstate;

	// clear to old value before delta parsing
	if (oldframe)
		*state = oldframe->playerstate;
	else
		state->clear();

	flags = msg.readShort();

	//
	// parse the pmove_state_t
	//
	if(flags & PS_M_TYPE)
		state->pmove.pm_type = (pm_type_e) msg.readByte();

	if(flags & PS_M_ORIGIN)
	{
		state->pmove.origin[0] = msg.readFloat();
		state->pmove.origin[1] = msg.readFloat();
		state->pmove.origin[2] = msg.readFloat();
	}

	if(flags & PS_M_VELOCITY)
	{
		state->pmove.velocity_linear[0] = msg.readFloat();
		state->pmove.velocity_linear[1] = msg.readFloat();
		state->pmove.velocity_linear[2] = msg.readFloat();
	}

	if(flags & PS_M_TIME)
		state->pmove.pm_time = msg.readByte();

	if(flags & PS_M_FLAGS)
		state->pmove.pm_flags = msg.readByte();

	if(flags & PS_M_GRAVITY)
		state->pmove.gravity = msg.readFloat();

	if(flags & PS_M_DELTA_ANGLES)
	{
		state->pmove.delta_angles[0] = msg.readFloat();
		state->pmove.delta_angles[1] = msg.readFloat();
		state->pmove.delta_angles[2] = msg.readFloat();
	}

	if(cl.attractloop)
		state->pmove.pm_type = PM_FREEZE;		// demo playback

	//
	// parse the rest of the player_state_t
	//
	if(flags & PS_VIEW_OFFSET)
	{
		state->view_offset[0] = msg.readFloat();// * 0.25;
		state->view_offset[1] = msg.readFloat();// * 0.25;
		state->view_offset[2] = msg.readFloat();// * 0.25;
	}

	if(flags & PS_VIEW_ANGLES)
	{		
		state->view_angles[0] = msg.readAngle();
		state->view_angles[1] = msg.readAngle();
		state->view_angles[2] = msg.readAngle();

	}

	if(flags & PS_KICK_ANGLES)
	{
		state->kick_angles[0] = msg.readFloat();// * 0.25;
		state->kick_angles[1] = msg.readFloat();// * 0.25;
		state->kick_angles[2] = msg.readFloat();// * 0.25;
	}

	if(flags & PS_WEAPON_MODEL_INDEX)
	{
		state->gun_model_index = msg.readByte();
	}

	if(flags & PS_WEAPON_ANIMATION_FRAME)
	{
		state->gun_anim_frame = msg.readShort();
		
		state->gun_offset[0] = msg.readFloat();//*0.25;
		state->gun_offset[1] = msg.readFloat();//*0.25;
		state->gun_offset[2] = msg.readFloat();//*0.25;
		
		state->gun_angles[0] = msg.readFloat();//*0.25;
		state->gun_angles[1] = msg.readFloat();//*0.25;
		state->gun_angles[2] = msg.readFloat();//*0.25;
	}
	
	if(flags & PS_WEAPON_ANIMATION_INDEX)
	{
		state->gun_anim_index = msg.readByte();
	}

	if(flags & PS_BLEND)
	{
		msg.readColor(state->blend);
	}

	if(flags & PS_FOV)
		state->fov = msg.readFloat();

	if(flags & PS_RDFLAGS)
		state->rdflags = msg.readByte();

	// parse stats
	statbits = msg.readLong();
	for(i=0; i<MAX_STATS; i++)
		if(statbits & (1<<i))
			state->stats[i] = msg.readShort();
}

static void	CL_ParseFrame(bitmessage_c &msg)
{
	int		cmd;
	frame_t*	old;

	cl.frame.clear();

	cl.frame.serverframe = msg.readLong();
	cl.frame.servertime = cl.frame.serverframe*100;
	cl.frame.deltaframe = msg.readLong();
	cl.surpress_count = msg.readByte();

	if(cl_shownet->getInteger() == 3)
		Com_Printf("   frame:%i  delta:%i\n", cl.frame.serverframe, cl.frame.deltaframe);

	// If the frame is delta compressed from data that we
	// no longer have available, we must suck up the rest of
	// the frame, but not use it, then ask for a non-compressed
	// message 
	if(cl.frame.deltaframe <= 0)
	{
		cl.frame.valid = true;		// uncompressed frame
		old = NULL;
		cls.demo_waiting = false;	// we can start recording now
	}
	else
	{
		old = &cl.frames[cl.frame.deltaframe & UPDATE_MASK];
		
		if(!old->valid)
		{
			// should never happen
			Com_Printf("Delta from invalid frame (not supposed to happen!).\n");
		}
		
		if(old->serverframe != cl.frame.deltaframe)
		{
			// The frame that the server did the delta from
			// is too old, so we can't reconstruct it properly.
			Com_Printf("Delta frame too old.\n");
		}
		else if(cl.entities_first - old->entities_first > MAX_ENTITIES - 128)
		{
			Com_Printf("Delta parse_entities too old.\n");
		}
		else
		{
			cl.frame.valid = true;	// valid delta parse
		}
	}
	

	//
	// clamp time 
	//
	X_clamp(cl.time, cl.frame.servertime - 100, cl.frame.servertime);


	//
	// read areabits
	//
	cmd = msg.readBits(svc_bitcount);
	
	CL_ShowNet(msg, svc_strings[cmd]);
	
	if(cmd != SVC_AREABITS)
		Com_Error(ERR_DROP, "CL_ParseFrame: not SVC_AREABITS");
		
	int areabits_num = msg.readByte();
	msg.readBits(areabits_num, cl.frame.areabits);
//	cl.frame.areabits.set();


	//
	// read playerinfo
	//
	cmd = msg.readBits(svc_bitcount);
	
	CL_ShowNet(msg, svc_strings[cmd]);
	
	if(cmd != SVC_PLAYERINFO)
		Com_Error(ERR_DROP, "CL_ParseFrame: not SVC_PLAYERINFO");
	
	CL_ParsePlayerstate(msg, old, &cl.frame);


	//
	// read packet entities
	//
	cmd = msg.readBits(svc_bitcount);
	
	CL_ShowNet(msg, svc_strings[cmd]);
	
	if(cmd != SVC_PACKETENTITIES)
		Com_Error(ERR_DROP, "CL_ParseFrame: not SVC_PACKETENTITIES");
	
	CL_ParsePacketEntities(msg, old, &cl.frame);
	
	
	//
	// save the frame off in the backup array for later delta comparisons
	//
	cl.frames[cl.frame.serverframe & UPDATE_MASK] = cl.frame;

	if(cl.frame.valid)
	{
		// getting a valid frame message ends the connection process
		if(cls.state != CA_ACTIVE)
		{
			cls.state = CA_ACTIVE;
			cl.force_refdef = true;
			
			if(cls.disable_servercount != cl.servercount && cl.refresh_prepped)
				SCR_EndLoadingPlaque();	// get rid of loading plaque
		}
		
		cl.sound_prepped = true;	// can start mixing ambient sounds
	}
}






void	CL_ParseServerMessage(bitmessage_c &msg)
{
	int		cmd;
	const char*	string;

	//
	// if recording demos, copy the message out
	//
	if(cl_shownet->getInteger() == 1)
	{
		Com_Printf("%i ", msg.getCurSize());
	}
	else if(cl_shownet->getInteger() >= 2)
	{
		Com_Printf("------------------\n");
	}


	//
	// parse the message
	//
	while(true)
	{
		if(msg.getReadCount() > msg.getCurSize())
		{
			Com_Error(ERR_DROP,"CL_ParseServerMessage: Bad server message");
			break;
		}
		
		if(msg.getReadCount() == msg.getCurSize())
		{
			CL_ShowNet(msg, "END OF MESSAGE");
			break;
		}

		cmd = msg.readBits(svc_bitcount);

		if(cl_shownet->getInteger() >= 2)
		{
			if(cmd < SVC_FIRST || cmd > SVC_LAST)
				Com_Printf("     %5i : BAD CMD %i\n", msg.getReadCount(), cmd);
			else
				CL_ShowNet(msg, svc_strings[cmd]);
		}
	
		//
		// other commands
		//
		switch(cmd)
		{
			default:
				Com_Error(ERR_DROP,"CL_ParseServerMessage: Illegible server message\n");
				break;
				
			case -1:
				//CL_ShowNet(msg, "END OF MESSAGE");
				break;
			
			case SVC_NOP:
				//Com_Printf("SVC_NOP\n");
				break;
			
			case SVC_DISCONNECT:
				Com_Error(ERR_DISCONNECT, "Server disconnected\n");
				break;

			case SVC_RECONNECT:
				Com_Printf("Server disconnected, reconnecting\n");
				if(cls.download_stream) 
				{
					VFS_FClose(&cls.download_stream);
				}
				cls.state = CA_CONNECTING;
				cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
				break;

			case SVC_PRINT:
				Con_ServerPrint(msg);
				break;
			
			case SVC_CENTERPRINT:
				cge->CG_CenterPrint(msg.readString());
				break;
			
			case SVC_STUFFTEXT:
				string = msg.readString();
				Com_DPrintf("stufftext: %s\n", string);
				Cbuf_AddText(string);
				break;
			
			case SVC_SERVERDATA:
				Cbuf_Execute();		// make sure any stuffed commands are done
				CL_ParseServerData(msg);
				break;
			
			case SVC_CONFIGSTRING:
				CL_ParseConfigString(msg);
				break;
			
			case SVC_SOUND:
				CL_ParseStartSoundPacket(msg);
				break;
			
			case SVC_SPAWNBASELINE:
				CL_ParseBaseline(msg);
				break;

			case SVC_TEMP_ENTITY:
				cge->CG_ParseTEnt(msg);
				break;

			case SVC_MUZZLEFLASH:
				cge->CG_ParseMuzzleFlash(msg);
				break;

			case SVC_DOWNLOAD:
				//CL_ParseDownload(msg);
				Com_Error(ERR_DROP, "CL_ParseServerMessage: SVC_DOWNLOAD requested");
				break;

			case SVC_INVENTORY:
				cge->CG_ParseInventory(msg);
				break;

			case SVC_LAYOUT:
				cge->CG_ParseLayout(msg);
				break;

			case SVC_PLAYERINFO:
			case SVC_PACKETENTITIES:
			case SVC_DELTAPACKETENTITIES:
				Com_Error(ERR_DROP, "Out of place frame data");
				break;
				
			case SVC_FRAME:
				CL_ParseFrame(msg);
				break;
		}
	}

	SCR_AddNetgraph();

	//
	// we don't know if it is ok to save a demo message until
	// after we have parsed the frame
	//
	if(cls.demo_recording && !cls.demo_waiting)
		CL_WriteDemoMessage(msg);

}


