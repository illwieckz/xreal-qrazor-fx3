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
#include "r_local.h"
#include "r_lwo.h"


std::vector<r_model_c*>	r_models;

unsigned int		r_registration_sequence;







r_model_c::r_model_c(const std::string &name, byte *buffer, uint_t buffer_size, r_mod_type_t type)
{
	//ri.Com_Printf ("r_model_c::ctor: %s\n", name);
	
	_name		= name;
	_type		= type;
	_buffer		= buffer;
	_buffer_size	= buffer_size;
	
//	_aabb.clear();
	
	setRegistrationSequence();
}

r_model_c::~r_model_c()
{
	X_purge<std::vector<r_mesh_c*> >(_meshes);
	
	X_purge<std::vector<r_model_shader_c*> >(_shaders);
}



void	r_model_c::setupMeshes()
{
	_aabb.clear();

	for(std::vector<r_mesh_c*>::const_iterator ir = _meshes.begin(); ir != _meshes.end(); ++ir)
	{
		r_mesh_c*	mesh = *ir;
		
		if(!mesh)
			continue;
		
		mesh->calcTangentSpaces();
//		mesh->calcEdges();
		mesh->createBBoxFromVertexes();
		
		_aabb.mergeWith(mesh->bbox);
	}
}


void	R_ShutdownModels()
{
	X_purge<std::vector<r_model_c*> >(r_models);
	
	r_models.clear();
}



void 	R_Modellist_f()
{
	ri.Com_Printf("Loaded models:\n");
	
	for(std::vector<r_model_c*>::const_iterator ir = r_models.begin(); ir != r_models.end(); ++ir)
	{
		r_model_c *mod = *ir;
	
		ri.Com_Printf("%s\n", mod->getName());
	}
	
	ri.Com_Printf("Total resident: %i\n", r_models.size());
}



r_model_c*	R_FindModel(const std::string &name)
{
	for(std::vector<r_model_c*>::const_iterator ir = r_models.begin(); ir != r_models.end(); ++ir)
	{
		r_model_c *mod = *ir;
		
		if(mod->getName() == name)
		{
			mod->setRegistrationSequence();
			return mod;
		}
	}

	return NULL;
}


r_model_c*	R_GetModel(const std::string &name, bool crash)
{
	byte*		buf = NULL;
	int		buf_size;
	r_model_c*	mod = NULL;
	
	
	if(!name.length())
		ri.Com_Error(ERR_DROP, "R_GetModel: NULL name");

	
	//
	// search the currently loaded models
	//
	mod = R_FindModel(name);
	if(mod)
		return mod;
	
		
	//
	// load the file
	//
	ri.Com_DPrintf("loading rendering model '%s' ...\n", name.c_str());
	
	buf_size = ri.VFS_FLoad(name, (void**)&buf);
	if(!buf)
	{
		if(crash)
			ri.Com_Error(ERR_DROP, "R_GetModel: %s not found", name.c_str());
			
		return NULL;
	}

	//
	// create the right class object
	//
	if(X_strnequal((const char*)buf, MD2_IDENTSTRING, 4))
		mod = new r_md2_model_c(name, buf, buf_size);
		
	else if(X_strnequal((const char*)buf, MD3_IDENTSTRING, 4))
		mod = new r_md3_model_c(name, buf, buf_size);
		
//	else if(X_strnequal((const char*)buf, MDC_IDSTRING, 4))
//		mod = new r_mdc_model_c(name, buf);
	
//	else if(X_strnequal((const char*)buf, MDS_IDENTSTRING, 4))
//		mod = new r_mds_model_c(name, buf);

	else if(X_strnequal((const char*)buf, ASE_IDENTSTRING, strlen(ASE_IDENTSTRING)))
		mod = new r_ase_model_c(name, buf, buf_size);
		
	else if(X_strnequal((const char*)buf, "FORM", 4))
		mod = new r_lwo_model_c(name, buf, buf_size);
		
	else if(X_strnequal((const char*)buf, MD5_IDENTSTRING, 10))
		mod = new r_md5_model_c(name, buf, buf_size);
			
//	else if(X_strnequal((const char*)buf, BSP_IDENTSTRING, 4))
//		mod = new r_bsp_model_c(name, buf);
		
//	else if(X_strnequal((const char*)buf, PROC_IDENTSTRING,11))
//		mod = new r_proc_model_c(name, buf);
	
	else
	{
		ri.VFS_FFree(buf);
		ri.Com_Error(ERR_DROP,"R_GetModel: unknown fileid for %s", name.c_str());
		return NULL;
	}
		
	
	//
	// load it
	//			
	mod->load();
		
	mod->setupMeshes();
	mod->setupVBO();
	
	r_models.push_back(mod);

	ri.VFS_FFree(buf);

	return mod;
}







/*
================
R_BeginRegistration
Specifies the model that will be used as the world
================
*/
void 	R_BeginRegistration(const std::string &model)
{
	r_registration_sequence++;
	
	//ri.Com_Printf("R_BeginRegistration: %s\n", model.c_str());
		
	R_ShutdownModels();
	
	R_ShutdownTree();	

	R_LoadLightMapImages(model);
	R_InitTree(model);
}

r_model_c*	R_RegisterModel(const std::string &name)
{
	//ri.Com_Printf("R_RegisterModel: '%s'\n", name.c_str());
	
	return R_GetModel(name, false);
}


int	R_RegisterModelExp(const std::string &name)
{
	r_model_c *mod = R_RegisterModel(name);
	
	return R_GetNumForModel(mod);
}

int	R_GetNumForModel(r_model_c *mod)
{
	if(!mod)
	{
		//ri.Com_Error(ERR_DROP, "R_GetNumForModel: NULL parameter\n");
		return -1;
	}

	int i=0;
	for(std::vector<r_model_c*>::const_iterator ir = r_models.begin(); ir != r_models.end(); ir++, i++)
	{
		if(*ir == mod)
			return i;
	}
	
	//ri.Com_Error(ERR_DROP, "R_GetNumForModel: bad pointer\n");
	return -1;
}

r_model_c*	R_GetModelByNum(int num)
{
	if(num < 0 || num >= (int)r_models.size())
	{
		//ri.Com_Error(ERR_DROP, "R_GetModelByNum: bad number %i\n", num);
		return NULL;
	}

	return r_models[num];
}


static void	R_FreeUnusedModels()
{
	for(std::vector<r_model_c*>::iterator ir = r_models.begin(); ir != r_models.end(); ir++)
	{
		if(!*ir)
			continue;
		
		if((*ir)->getRegistrationSequence() != r_registration_sequence)
		{
			ri.Com_Printf("deleting model '%s' ...\n", (*ir)->getName());
				
			delete *ir;
			
			*ir = NULL;
		}
	}
}

void 	R_EndRegistration()
{
	R_FreeUnusedModels();

	R_FreeUnusedSkins();

	R_FreeUnusedShaders();
	
	R_FreeUnusedImages();
}




