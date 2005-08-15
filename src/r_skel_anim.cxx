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



