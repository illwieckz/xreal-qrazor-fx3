/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003 German Garcia
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
// xreal --------------------------------------------------------------------
#include "cg_local.h"



/*
==================
CG_PlayerModels_UpdateAnimationBuffer

Called just after parsing a new frame from the server
==================
*/
void	CG_UpdateAnimationBuffer()
{
	
	int			pnum;
	client_info_t		*ci;
	int			newanim;

	for(pnum=0; pnum<cg.frame.entities_num; pnum++)
	{
		entity_state_t& state = cg.entities_parse[(cg.frame.entities_parse_index+pnum)&(MAX_PARSE_ENTITIES-1)];

		if(state.getNumber() < MAX_CLIENTS+1)
		{
			if(state.index_model == 255 /*&& !(state->renderfx & RF_DEADBODY*/)
			{
				ci = &cg.clientinfo[state.getNumber() -1];

				//extract the different animations from s.frame
				/*
				newanim[PLAYER_BODY_PART_LOWER] = (int)(state->_s.frame & 0x3F);
				newanim[PLAYER_BODY_PART_UPPER] = (int)((state->_s.frame >> 6) & 0x3F);
				newanim[PLAYER_BODY_PART_HEAD] = (int)((state->_s.frame >> 12) & 0xF);
				*/
				
				newanim = state.frame;
												
				if(newanim)
				{	
					ci->anim.buffer.newanim = newanim;
					ci->anim.buffer.serverframe = cg.frame.serverframe;
				}
			}
		}
	}
}

void	CG_AnimToFrameRate(anim_info_t &anim)
{
	//TODO
}

/*
static void	CG_AnimToServerFrame(anim_info_t &anim)
{
	//
	// set backlerp
	//
	anim.backlerp = 1.0 - cg.frame_lerp;
		
	if(cg.frame.servertime < anim.frame_delay)
		return;
	else
		anim.frame_delay = cg.frame.servertime + 1;
	
	
	// advance frames
	anim.frame_old = anim.frame;
	anim.frame++;
		
	// looping
	if(anim.frame > anim.animations[anim.frame_current].last_frame)
		anim.frame = (anim.animations[anim.frame_current].last_frame - anim.animations[anim.frame_current].looping_frames);
			
	// new animation
	if(anim.buffer.newanim && (anim.buffer.updated != anim.buffer.serverframe))
	{
		
		if(anim.buffer.newanim < 0 || anim.buffer.newanim >= (int)anim.animations.size())
		{
			trap_Com_Error(ERR_DROP, "CG_AnimToServerFrame: bad sequence %i of %i", anim.buffer.newanim, anim.animations.size());
		}
			
		anim.frame = anim.animations[anim.buffer.newanim].first_frame;
		anim.frame_current = anim.buffer.newanim;
		anim.buffer.updated = anim.buffer.serverframe;
		anim.buffer.newanim = 0;
	}
}
*/

/*
static void	CG_AnimToFrame(anim_info_t &anim)
{
	CG_AnimToServerFrame(anim);
}
*/

/*
static void	CG_AnimToFrame(anim_info_t &anim)
{
	
}
*/

/*
static std::string	cg_tagnames[] = {"tag_torso", "tag_head", ""};

void	CG_AddPlayerEntities(r_entity_t &ent, const cg_entity_t *cent, int effects, int renderfx)
{
	client_info_t* ci = &cg.clientinfo[cent->current.getNumber()];
	
	r_entity_t ppm;
		
	
	//
	// setup frames
	//
	CG_AnimToFrame(ci->anim);
	
	ppm.backlerp = ci->anim.backlerp;
	ppm.frame = ci->anim.frame;
	ppm.frame_old = ci->anim.frame_old;
	
	
	//
	// move lower body to final position
	//
	ppm.origin	= ent.origin;
	ppm.origin2	= ent.origin2;
		
	//
	// add to the renderer
	//
	ppm.model	= ci->model;			// opaque type outside refresh
	ppm.custom_skin = ci->skin;
	ppm.quat	= ent.quat;
	ppm.flags	= ent.flags;
	ppm.scale	= ent.scale;
		
	
	// add entities to the renderer
	trap_R_AddEntity(ppm);
	
	//TODO
}
*/

/*
static void	CG_ParseAnimationCFG(client_info_t *ci, const std::string &model_name)
{
	std::string	filename;

	char *buffer = NULL;
	char *ptr;
	char *token;
	
	//int	offset;
	bool	nooffset;
	vec3_c	headoffset;
	
	int	anim = 0;
	int	frames_upper = 0;
	int	frames_lower = 0;
	std::vector<r_anim_sequence_t> upper_sequences;
	std::vector<r_anim_sequence_t> lower_sequences;
	std::vector<r_anim_sequence_t> head_sequences;
	
	//
	// load the animation.cfg
	//
	filename = "models/players/" + model_name + "/animation.cfg";
	trap_VFS_FLoad(filename, (void**)&buffer);
	if(!buffer)
	{
		trap_Com_Error(ERR_DROP, "CG_ParseAnimationCFG: bad config '%s\n", filename.c_str());
	}
		
	//
	// parse buffer
	//
	ptr = buffer;
	while(ptr)
	{
		token = Com_Parse(&ptr);
		
		if(!token)
		{
			break;
		}		
		else if(!isdigit(token[0]))
		{
			if(X_strcaseequal(token, "sex"))
			{
				// skip gender
				Com_Parse(&ptr);
			
				continue;
			}
		
			if(X_strcaseequal(token, "headoffset"))
			{
				// parse headoffset origin
			
				for(int i=0; i<3; i++)
				{	
					token = Com_Parse(&ptr);
			
					if(!token)
						break;
					else
						headoffset[i] = atof(token);
				}
														
				continue;
			}
		
			if(X_strcaseequal(token, "footsteps"))
			{
				// skip it
				Com_Parse(&ptr);
			
				continue;
			}
			
			if(X_strcaseequal(token, "nooffset"))
			{
				nooffset = true;
				
				continue;
			}
		}
		else
		{
			r_anim_sequence_t	a;
		
			// well ignore first token
			a.first_frame		= atoi(token);			// first frame
			a.frames_num		= Com_ParseInt(&ptr);		// frames num
			a.looping_frames	= Com_ParseInt(&ptr);		// looping frames
			a.frames_per_second	= Com_ParseInt(&ptr);		// frames per second
						
							
#if 0
			uii.Com_Printf("CG_ParseAnimationCFG: %i %i %i %i\n",	a.first_frame,
										a.frames_num,
										a.looping_frames,
										a.frames_per_second);
#endif
			
			if(anim <= PLAYER_ANIM_UPPER_DEATH3)	// upper and lower share this animation frames
			{
				upper_sequences.push_back(a);
				lower_sequences.push_back(a);
			}
			else if(anim <= PLAYER_ANIM_UPPER_STAND2)
			{
				upper_sequences.push_back(a);
			}
			else if(anim <= (PLAYER_ANIM_UPPER_STAND2 + (PLAYER_ANIM_LOWER_TURN - PLAYER_ANIM_LOWER_DEATH3)))
			{
				lower_sequences.push_back(a);
			}
			else if(anim <= (PLAYER_ANIM_UPPER_STAND2 + (PLAYER_ANIM_LOWER_TURN - PLAYER_ANIM_LOWER_DEATH3) + 1))
			{
				head_sequences.push_back(a);
			}

			
			anim++;
		}
	}
	
	trap_VFS_FFree(buffer);
	
	trap_Com_Printf("CG_ParseAnimationCFG: animation sequences %i %i %i\n",	upper_sequences.size(),
										lower_sequences.size(),
										head_sequences.size());

	ci->anim.sequences[PLAYER_BODY_PART_LOWER] = lower_sequences;
	ci->anim.sequences[PLAYER_BODY_PART_UPPER] = upper_sequences;
	ci->anim.sequences[PLAYER_BODY_PART_HEAD]  = head_sequences;
	
	// setup lower frames
	for(int i=0; i<=(PLAYER_ANIM_LOWER_TURN); i++)
	{
		if(i==0)
		{
			ci->anim.sequences[PLAYER_BODY_PART_LOWER][i].first_frame = 0;
			ci->anim.sequences[PLAYER_BODY_PART_LOWER][i].last_frame = 0;
		}
		else
		{
			ci->anim.sequences[PLAYER_BODY_PART_LOWER][i].first_frame = (frames_lower += ci->anim.sequences[PLAYER_BODY_PART_LOWER][i-1].frames_num);
			ci->anim.sequences[PLAYER_BODY_PART_LOWER][i].last_frame = ci->anim.sequences[PLAYER_BODY_PART_LOWER][i].first_frame + ci->anim.sequences[PLAYER_BODY_PART_LOWER][i].frames_num;
		}
	}

	
	// setup upper frames
	for(int i=0; i<=(PLAYER_ANIM_UPPER_STAND2); i++)
	{
		if(i==0)
		{
			ci->anim.sequences[PLAYER_BODY_PART_UPPER][i].first_frame = 0;
			ci->anim.sequences[PLAYER_BODY_PART_UPPER][i].last_frame = 0;
		}
		else
		{
			ci->anim.sequences[PLAYER_BODY_PART_UPPER][i].first_frame = (frames_upper += ci->anim.sequences[PLAYER_BODY_PART_UPPER][i-1].frames_num);
			ci->anim.sequences[PLAYER_BODY_PART_UPPER][i].last_frame = ci->anim.sequences[PLAYER_BODY_PART_UPPER][i].first_frame + ci->anim.sequences[PLAYER_BODY_PART_UPPER][i].frames_num;
		}
	}
	
	// setup head frames
	for(int i=0; i<=(PLAYER_ANIM_HEAD_IDLE); i++)
	{
		if(i==0)
		{
			ci->anim.sequences[PLAYER_BODY_PART_HEAD][i].first_frame = 0;
			ci->anim.sequences[PLAYER_BODY_PART_HEAD][i].last_frame = 0;
		}
		else
		{
			ci->anim.sequences[PLAYER_BODY_PART_HEAD][i].first_frame = (frames_upper += ci->anim.sequences[PLAYER_BODY_PART_HEAD][i-1].frames_num);
			ci->anim.sequences[PLAYER_BODY_PART_HEAD][i].last_frame = ci->anim.sequences[PLAYER_BODY_PART_HEAD][i].first_frame + ci->anim.sequences[PLAYER_BODY_PART_HEAD][i].frames_num;
		}
	}
	
}
*/

/*
static void	CG_LoadClientModelinfo(client_info_t *ci, const std::string model_name)
{
	std::string filename;
	
	//
	// load body model
	//
	ci->model = trap_R_RegisterModel(filename = "models/players/" + model_name + "/body.md5mesh");
	
	
}
*/

/*
static void	CG_LoadClientAnimationinfo(client_info_t *ci, const std::string model_name)
{
	//TODO
	
	r_animation_t	anim;
	
	anim.first_frame	= 0;
	anim.frames_num		= 300;
	anim.looping_frames	= 0;
	anim.frames_per_second	= 24;
	
		
	// load idle animation
	anim.model = ci->model;
	anim.sequence = trap_R_RegisterAnimation("models/players/" + model_name + "/idle.md5anim");
	
	// dummy sequence
	ci->anim.animations.push_back(anim);
}
*/

/*
static void	CG_LoadClientSkininfo(client_info_t *ci, const std::string model_name, const std::string skin_name)
{
	//
	// load body skin
	//
	ci->skin = trap_R_RegisterSkin("models/players/" + model_name + "/body_" + skin_name +  ".skin");
		
	//TODO
}
*/

void	CG_LoadClientinfo(client_info_t *ci, const std::string &s)
{
	std::string	model_name;
	std::string	skin_name;
	std::string	weapon_name;
	
	trap_Com_Printf("CG_LoadClientInfo: '%s'\n", s.c_str());

	ci->cinfo = s;

	// isolate the player's name
	std::string player_name = s.substr(0, s.find('\\'));
	
	trap_Com_Printf("CG_LoadClientInfo: name '%s'\n", player_name.c_str());
	
	ci->name = player_name;
	
	//ci->name[sizeof(ci->name)-1] = 0;
	//t = strstr(s.c_str(), "\\");
	/*
	if(t)
	{
		ci->name[t-s] = 0;
		s = t+1;
	}
	*/

	//if(cg_noskins->value || *s == 0)
	{
		model_name = DEFAULT_PLAYERMODEL;
		skin_name = DEFAULT_PLAYERSKIN;
		//Com_sprintf(ci->iconname, sizeof(ci->iconname), "players/male/grunt_i.pcx");
		
		//CG_LoadClientModelinfo(ci, model_name);
		//CG_LoadClientSkininfo(ci, model_name, skin_name);
		//CG_LoadClientAnimationinfo(ci, model_name);
		//CG_ParseAnimationCFG(ci, model_name);
		
		
	}
	/*
	else
	{
		// isolate the model name
		strcpy (model_name, s);
		t = strstr(model_name, "/");
		if (!t)
			t = strstr(model_name, "\\");
		if (!t)
			t = model_name;
		*t = 0;

		// isolate the skin name
		strcpy (skin_name, s + strlen(model_name) + 1);

		// model file
		Com_sprintf (model_filename, sizeof(model_filename), "players/%s/tris.md2", model_name);
		//Com_sprintf (model_filename, sizeof(model_filename), "players/%s/body.mds", model_name);
		ci->model = trap_R_RegisterModel (model_filename);
		if (!ci->model)
		{
			strcpy(model_name, "male");
			Com_sprintf (model_filename, sizeof(model_filename), "players/male/tris.md2");
			ci->model = trap_R_RegisterModel (model_filename);
		}

		// skin file
		Com_sprintf (skin_filename, sizeof(skin_filename), "players/%s/%s.pcx", model_name, skin_name);
		ci->skin = trap_R_RegisterShader(skin_filename);

		// if we don't have the skin and the model wasn't male,
		// see if the male has it (this is for CTF's skins)
 		if (!ci->skin && X_stricmp(model_name, "male"))
		{
			// change model to male
			strcpy(model_name, "male");
			Com_sprintf (model_filename, sizeof(model_filename), "players/male/tris.md2");
			ci->model = trap_R_RegisterModel (model_filename);

			// see if the skin exists for the male model
			Com_sprintf (skin_filename, sizeof(skin_filename), "players/%s/%s.pcx", model_name, skin_name);
			ci->skin = trap_R_RegisterShader(skin_filename);
		}

		// if we still don't have a skin, it means that the male model didn't have
		// it, so default to grunt
		if (!ci->skin) 
		{
			// see if the skin exists for the male model
			Com_sprintf (skin_filename, sizeof(skin_filename), "players/%s/grunt.pcx", model_name, skin_name);
			ci->skin = trap_R_RegisterShader(skin_filename);
		}

		// icon file
		Com_sprintf (ci->iconname, sizeof(ci->iconname), "/players/%s/%s_i.pcx", model_name, skin_name);
		ci->icon = trap_R_RegisterPic (ci->iconname);
	}
	*/
	
	//trap_Com_Printf("CG_LoadClientInfo: middle\n");

	// must have loaded all data types to be valud
	/*
	if(ci->skins[0] == -1 || ci->icon == -1 || ci->models[0] == -1 || ci->weaponmodel[0] == -1)
	{
		ci->skin = -1;
		ci->icon = -1;
		ci->model_lower = -1;
		ci->weaponmodel[0] = -1;
	}
	*/
	
	//trap_Com_Printf("CG_LoadClientInfo: end\n");
}



/*
================
CG_ParseClientinfo

Load the skin, icon, and model for a client
================
*/
void	CG_ParseClientinfo(int player)
{
	trap_Com_Printf("CG_ParseClientinfo: %i\n", player);
		
	if(!trap_CL_GetConfigString(player+CS_PLAYERSKINS)[0])
		return;

	CG_LoadClientinfo(&cg.clientinfo[player], trap_CL_GetConfigString(player+CS_PLAYERSKINS));
}

