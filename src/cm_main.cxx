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
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "common.h"
#include "cvar.h"
#include "vfs.h"
#include "files.h"

#include "cm.h"
#include "cm_md3.h"

// xreal --------------------------------------------------------------------

extern std::vector<cmodel_c*>		cm_models;


cmodel_c*	CM_RegisterModel(const std::string &name)
{
	if(name.empty())
		Com_Error(ERR_DROP, "CM_RegisterModel: empty name");

	//
	// search the currently loaded models
	//
	for(std::vector<cmodel_c*>::iterator ir = cm_models.begin(); ir != cm_models.end(); ir++)
	{
		if(X_strequal((*ir)->getName(), name.c_str()))
			return *ir;
	}

	Com_DPrintf("loading '%s' ...\n", name.c_str());

	//
	// load the file
	//
	byte 		*data = NULL;
	cmodel_c	*mod = NULL;
	
	VFS_FLoad(name, (void**)&data);
	if(!data)
	{
		Com_Printf("CM_RegisterModel: '%s' not found\n", name.c_str());	
		return NULL;
	}

	//
	// create the right class object
	//
	if(X_strnequal((const char*)data, MD3_IDENTSTRING, 4))
		mod = new cmodel_md3_c(name);
		
	//else if(X_strnequal((const char*)buf, MD5_IDENTSTRING, 10))
	//	mod = new cm_skel_md5_model_c(name, buf);
			
	//else if(X_strnequal((const char*)buf, BSP_IDENTSTRING, 4))
	//	mod = new cm_bsp_model_c(name, buf);
		
	//else if(X_strnequal((const char*)buf, PROC_IDENTSTRING,11))
	//	mod = new cm_proc_model_c(name, buf);
	
	else
	{
		Com_Printf("CM_RegisterModel: unknown fileid for '%s'\n", name.c_str());
		return NULL;
	}
		
	
	//
	// load it
	//			
	mod->load(data);
		
	cm_models.push_back(mod);

	VFS_FFree(data);

	return mod;	
}

void	CM_EndRegistration()
{
	// TODO free unused models
}

cmodel_c*	CM_GetModelByName(const std::string &name)
{
	if(!name.length())
		Com_Error(ERR_DROP, "CM_GetModelByName: empty name");
		
	//Com_DPrintf("CM_GetModelByName: '%s'\n", name.c_str());
	
	for(std::vector<cmodel_c*>::iterator ir = cm_models.begin(); ir != cm_models.end(); ir++)
	{
		if(X_strequal((*ir)->getName(), name.c_str()))
			return *ir;
	}
	
	Com_Printf("CM_GetModelByName: couldn't find '%s'\n", name.c_str());
	return NULL;
}

cmodel_c*	CM_GetModelByNum(int num)
{
	if(num < 0 || num >= (int)cm_models.size())
	{
		Com_Error(ERR_DROP, "CM_GetModelbyNum: bad number %i", num);
		return NULL;
	}
	
	return cm_models[num];
}



