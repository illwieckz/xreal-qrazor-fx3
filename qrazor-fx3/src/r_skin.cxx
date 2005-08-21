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
// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"




static std::vector<r_model_skin_c*>	r_skins;

void	R_InitSkins()
{
	ri.Com_Printf("------- R_InitSkins -------\n");

	//TODO ?
}


void	R_ShutdownSkins()
{
	ri.Com_Printf("------- R_ShutdownSkins -------\n");
	
	X_purge<std::vector<r_model_skin_c*> >(r_skins);
	
	r_skins.clear();
}


static r_model_skin_c*	R_GetFreeSkin(const std::string &name)
{
	r_model_skin_c* skin = new r_model_skin_c(name);
	
	std::vector<r_model_skin_c*>::iterator ir = find(r_skins.begin(), r_skins.end(), static_cast<r_model_skin_c*>(NULL));
	
	if(ir != r_skins.end())
		*ir = skin;
	else
		r_skins.push_back(skin);
	
	return skin;
}


r_model_skin_c*	R_LoadSkin(const std::string &name)
{
	char*		buf = NULL;
	char*		token;
	r_model_skin_c*	skin = NULL;
	std::map<std::string, std::string>	skins;
	
	Com_Printf("loading '%s' ...\n", name.c_str());
	
	//
	// load the file
	//
	ri.VFS_FLoad(name, (void**)&buf);
	if(!buf)
	{
		ri.Com_Error(ERR_DROP, "R_LoadSkin: bad skin file '%s'", name.c_str());
		return NULL;
	}
	
	while(buf)
	{
		token = Com_Parse(&buf);
		
		if(!token)
			break;
		
		if(!token[0])
			continue;
		
		// skip tag_ keywords
		if(!strncmp(token, "tag_", 4))
			continue;
		
		//ri.Com_Printf("R_LoadSkin: '%s'\n", token);
		
		std::string tok = token;
		std::string mesh = tok.substr(0, tok.find(','));
		std::string skin = tok.substr(tok.find(',')+1, tok.length());
		
		//ri.Com_Printf("R_LoadSkin: '%s' '%s'\n", mesh.c_str(), skin.c_str());
		
		skins.insert(make_pair(mesh, skin));
	}
	
	skin = R_GetFreeSkin(name);
	
	for(std::map<std::string, std::string>::const_iterator ir = skins.begin(); ir != skins.end(); ir++)
	{
		skin->addShader(ir->first, R_RegisterShader(ir->second));
	}
	
	return skin;
}


r_model_skin_c*	R_FindSkin(const std::string &name)
{
	if(!name.length())
	{	
		ri.Com_Error(ERR_DROP, "R_FindSkin: NULL name");
		return NULL;
	}
	
	//Com_StripExtension ((char*)name.c_str(), name_short);

	for(std::vector<r_model_skin_c*>::iterator ir = r_skins.begin(); ir != r_skins.end(); ir++)
	{
		r_model_skin_c*	skin = *ir;
		
		if(!skin)
			continue;
		
		if(X_strcaseequal(name.c_str(), skin->getName()))
		{
			skin->setRegistrationSequence();
			return skin;
		}
	}

	//
	// load the skin from disk
	//
	return R_LoadSkin(name);
}


int	R_GetNumForSkin(r_model_skin_c *skin)
{
	if(!skin)
	{
		//ri.Com_Error(ERR_DROP, "R_GetNumForSkin: NULL parameter\n");
		return -1;
	}

	for(unsigned int i=0; i<r_skins.size(); i++)
	{
		if(skin == r_skins[i])
			return i;
	}
	
	ri.Com_Error(ERR_DROP, "R_GetNumForSkin: bad pointer\n");
	return -1;
}


r_model_skin_c*	R_GetSkinByNum(int num)
{
	if(num < 0 || num >= (int)r_skins.size())
	{
		ri.Com_Error(ERR_DROP, "R_GetSkinByNum: bad number %i\n", num);
		return NULL;
	}

	return r_skins[num];
}


void	R_FreeUnusedSkins()
{
	for(std::vector<r_model_skin_c*>::iterator ir = r_skins.begin(); ir != r_skins.end(); ir++)
	{
		r_model_skin_c *skin = *ir;
		
		if(!skin)
			continue;
		
		if(skin->getRegistrationSequence() == r_registration_sequence)
		{
			skin->updateShaders();
			continue;		// used this sequence
		}
			
		
		// free it
		delete skin;
		*ir = NULL;
	}
}

int	R_RegisterSkinExp(const std::string &name)
{
	//ri.Com_Printf("R_RegisterSkinExp: '%s'\n", name.c_str());
	
	return R_GetNumForSkin(R_FindSkin(name));
}


void	R_SkinList_f()
{
	ri.Com_Printf("------------------\n");

	for(std::vector<r_model_skin_c*>::iterator ir = r_skins.begin(); ir != r_skins.end(); ir++)
	{
		r_model_skin_c*	skin = *ir;
		
		if(!skin)
			continue;
		
		ri.Com_Printf("%s\n", skin->getName());
	}
}



