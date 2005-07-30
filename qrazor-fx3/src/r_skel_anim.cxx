/// ============================================================================
/*
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
#include <map>
#include <algorithm>
#include <functional>

// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"


static std::vector<r_skel_animation_c*>	r_animations;


r_skel_animation_c::r_skel_animation_c(const std::string &name, byte *buffer, uint_t buffer_size)
{
	_name = name;
	setRegistrationCount();
	
	_buffer = buffer;
	_buffer_size = buffer_size;
	
	std::vector<r_skel_animation_c*>::iterator ir = find(r_animations.begin(), r_animations.end(), static_cast<r_skel_animation_c*>(NULL));
	
	if(ir != r_animations.end())
		*ir = this;
	else
		r_animations.push_back(this);
}

r_skel_animation_c::~r_skel_animation_c()
{
	X_purge<std::vector<r_skel_channel_t*> >(_channels);
	_channels.clear();
}

/*
void	r_skel_animation_c::loadChannels(char **data_p)
{
	Com_Parse(data_p);	// skip "numchannels"
	int channels_num = Com_ParseInt(data_p);
			
	if(channels_num <= 0)// || channels_num > MD5_MAX_CHANNELS)
	{
		ri.Com_Error(ERR_DROP, "r_skel_animation_c::loadChannels: animation '%s' has invalid channels number %i", _name.c_str(), channels_num);
		return;
	}
	
	for(int i=0; i<channels_num; i++)
	{
		r_skel_channel_t *channel = new r_skel_channel_t();
	
		Com_Parse(data_p, true);	// skip "channel"
		Com_Parse(data_p, false);	// skip channel number
		Com_Parse(data_p, false);	// skip '{'
		
		Com_Parse(data_p, true);	// skip "joint"
		channel->joint = Com_Parse(data_p, false);
						
		Com_Parse(data_p, true);	// skip "attribute"
		char* attr = Com_Parse(data_p, false);
		
		if(X_strequal(attr, "x"))
			channel->attribute = CHANNEL_ATTRIB_X;
			
		else if(X_strequal(attr, "y"))
			channel->attribute = CHANNEL_ATTRIB_Y;
			
		else if(X_strequal(attr, "z"))
			channel->attribute = CHANNEL_ATTRIB_Z;
		
		else if(X_strequal(attr, "pitch"))
			channel->attribute = CHANNEL_ATTRIB_PITCH;
			
		else if(X_strequal(attr, "yaw"))
			channel->attribute = CHANNEL_ATTRIB_YAW;
		
		else if(X_strequal(attr, "roll"))
			channel->attribute = CHANNEL_ATTRIB_ROLL;
		
		else
		{
			ri.Com_Error(ERR_DROP, "r_skel_animation_c::loadChannels: channel %i has bad attribute '%s'", i, attr);
			return;
		}
		
		Com_Parse(data_p, true);	// skip "starttime"
		channel->time_start = Com_ParseFloat(data_p, false);
		
		Com_Parse(data_p, true);	// skip "endtime"
		channel->time_end = Com_ParseFloat(data_p, false);
		
		Com_Parse(data_p, true);	// skip "framerate"
		channel->time_fps = Com_ParseFloat(data_p, false);
		
		Com_Parse(data_p, true);	// skip "strings"
		Com_Parse(data_p, false);	// skip strings number
		
		Com_Parse(data_p, true);	// skip "range"
		channel->range[0] = Com_ParseInt(data_p, false);
		channel->range[1] = Com_ParseInt(data_p, false);
		
		Com_Parse(data_p, true);	// skip "keys"
		int keys_num = Com_ParseInt(data_p, false);
		//ri.Com_Printf("channel %i has keys num %i\n", i, keys_num);
		
		channel->keys = std::vector<float>(keys_num);
		for(int j=0; j<keys_num; j++)
		{
			channel->keys[j] = Com_ParseFloat(data_p, true);
		}
		
		char *bracket = Com_Parse(data_p);	// skip '}'
		if(!X_strequal(bracket, "}"))
		{
			ri.Com_Error(ERR_DROP, "r_skel_animation_c::loadChannels: found '%s' instead of '}' in channel %i", bracket, i);
			return;
		}
		
		_channels.push_back(channel);
	}
}
*/


void	R_InitAnimations()
{
	ri.Com_Printf("------- R_InitAnimations -------\n");

	//TODO ?
}


void	R_ShutdownAnimations()
{
	ri.Com_Printf("------- R_ShutdownAnimations -------\n");
	
	X_purge<std::vector<r_skel_animation_c*> >(r_animations);
	r_animations.clear();
}


r_skel_animation_c*	R_LoadAnimation(const std::string &name)
{
	byte*	buffer = NULL;
	uint_t	buffer_size;
	
	Com_Printf("loading '%s' ...\n", name.c_str());

	//
	// load the file
	//
	buffer_size = ri.VFS_FLoad(name, (void **)&buffer);
	if(!buffer)
	{
		ri.Com_Error(ERR_DROP, "r_skel_md5_model_c::registerAnimation: couldn't load '%s'", name.c_str());
		return NULL;
	}
	
	r_skel_animation_c *anim = NULL;
	
	if(X_strnequal((const char*)buffer, MD5_IDENTSTRING, 10))
	{
		anim = new r_md5_animation_c(name, buffer, buffer_size);
	}
	else
	{
		ri.VFS_FFree(buffer);
		ri.Com_Error(ERR_DROP, "R_LoadAnimation: unknown fileid for %s", name.c_str());
		return NULL;
	}
	
	anim->load();

	ri.VFS_FFree(buffer);
	
	//TODO update public animation info
	
//	ri.Com_Printf("r_skel_md5_model_c::registerAnimation: anim '%s' has %i channels\n", name.c_str(), anim->_channels.size());
	
	return anim;
}





r_skel_animation_c*	R_FindAnimation(const std::string &name)
{
	if(!name.length())
	{	
		ri.Com_Error(ERR_DROP, "R_FindAnimation: empty name");
		return NULL;
	}
	
	for(std::vector<r_skel_animation_c*>::iterator ir = r_animations.begin(); ir != r_animations.end(); ir++)
	{
		r_skel_animation_c *anim = *ir;
		
		if(!anim)
			continue;
		
		if(X_strcaseequal(name.c_str(), anim->getName()))
		{
			anim->setRegistrationCount();
			return anim;
		}
	}

	return R_LoadAnimation(name);
}


int	R_GetNumForAnimation(r_skel_animation_c *anim)
{
	if(!anim)
	{
		//ri.Com_Error(ERR_DROP, "R_GetNumForSkin: NULL parameter\n");
		return -1;
	}

	for(unsigned int i=0; i<r_animations.size(); i++)
	{
		if(anim == r_animations[i])
			return i;
	}
	
	ri.Com_Error(ERR_DROP, "R_GetNumForAnimation: bad pointer\n");
	return -1;
}


r_skel_animation_c*	R_GetAnimationByNum(int num)
{
	if(num < 0 || num >= (int)r_animations.size())
	{
		//ri.Com_Error(ERR_DROP, "R_GetAnimationByNum: bad number %i\n", num);
		return NULL;
	}

	return r_animations[num];
}


void	R_FreeUnusedAnimations()
{
	for(std::vector<r_skel_animation_c*>::iterator ir = r_animations.begin(); ir != r_animations.end(); ir++)
	{
		r_skel_animation_c *anim = *ir;
		
		if(!anim)
			continue;
		
		if(anim->isRegistered())
			continue;		// used this sequence
				
		delete anim;
		*ir = NULL;
	}
}

int	R_RegisterAnimationExp(const std::string &name)
{
	//ri.Com_Printf("R_RegisterSkinExp: '%s'\n", name.c_str());
	
	return R_GetNumForAnimation(R_FindAnimation(name));
}


void	R_AnimationList_f()
{
	ri.Com_Printf("------------------\n");

	for(std::vector<r_skel_animation_c*>::iterator ir = r_animations.begin(); ir != r_animations.end(); ir++)
	{
		r_skel_animation_c* anim = *ir;
		
		if(!anim)
			continue;
		
		ri.Com_Printf("'%s'\n", anim->getName());
	}
}



