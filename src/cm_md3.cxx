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
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "cm_md3.h"

#include "common.h"
#include "vfs.h"
#include "files.h"


cmodel_md3_c::cmodel_md3_c(const std::string &name, byte *buffer, uint_t buffer_size)
:cmodel_c(name, buffer, buffer_size)
{
}

void	cmodel_md3_c::load()
{
	int					i=0, j=0, k=0;
	int					version;
	//float					alpha, beta;

	md3_dheader_t				*pinmodel;
	md3_dmesh_t				*pinmesh;
	md3_dvertex_t				*pinvertex;
	index_t					*pinindex;
	
	pinmodel = (md3_dheader_t*)_buffer;
	version = LittleLong(pinmodel->version);
	
	if(version != MD3_VERSION)
		Com_Error(ERR_DROP, "cmodel_md3_c::load: %s has wrong version number (%i should be %i)", getName(), version, MD3_VERSION);


	int meshes_num = LittleLong(pinmodel->meshes_num);
	
	if(meshes_num <= 0 || meshes_num > MD3_MAX_MESHES)
		Com_Error(ERR_DROP, "cmodel_md3_c::load: model %s has invalid meshes number %i", getName(), meshes_num);
			
		
	//
	// load the meshes
	//
	pinmesh = (md3_dmesh_t*)((byte*) pinmodel + LittleLong(pinmodel->meshes_ofs));
	
	//meshes_num = 1;
	int vertexes_counter = 0;
	for(i=0; i<meshes_num; i++)
	{
		if(!X_strnequal((const char*)pinmesh->ident, MD3_IDENTSTRING, 4))
		{
			Com_Error(ERR_DROP, "cmodel_md3_c::load: mesh %s in model %s has wrong ident (%s should be %s)", 
						pinmesh->name, getName(), pinmesh->ident, MD3_IDENTSTRING);
		}
		
		int vertexes_num = LittleLong(pinmesh->vertexes_num);
		int indexes_num = LittleLong(pinmesh->triangles_num)*3;
		
		if(vertexes_num <= 0 || vertexes_num > MD3_MAX_VERTEXES)
			Com_Error(ERR_DROP, "cmodel_md3_c::load: mesh %s in model %s has invalid vertexes number %i", pinmesh->name, getName(), vertexes_num);
		
		if(indexes_num <= 0 || indexes_num > MD3_MAX_INDEXES)
			Com_Error(ERR_DROP, "cmodel_md3_c::load: mesh %s in model %s has invalid indexes number %i", pinmesh->name, getName(), indexes_num);
		
		
				
		//
		// load the indexes
		//
		pinindex = (index_t*)((byte*)pinmesh + LittleLong(pinmesh->indexes_ofs));
	
		for(j=0; j<indexes_num; j++, pinindex++)
		{
			indexes.push_back(vertexes_counter + (index_t)LittleLong(*pinindex));
		}
	
					
		//
		// load the vertexes, normals and texcoords for each frame
		//
		pinvertex = (md3_dvertex_t*)((byte*)pinmesh + LittleLong(pinmesh->vertexes_ofs));
		
		for(k=0; k<vertexes_num; k++, pinvertex++)
		{
			vec3_c vertex;
			
			vertex[0] = ((float)LittleShort(pinvertex->vertex[0]) * MD3_VERTEX_SCALE);
			vertex[1] = ((float)LittleShort(pinvertex->vertex[1]) * MD3_VERTEX_SCALE);
			vertex[2] = ((float)LittleShort(pinvertex->vertex[2]) * MD3_VERTEX_SCALE);
			
			vertexes.push_back(vertex);
		}
		
		vertexes_counter += vertexes_num;
		
		//Com_Printf("cmodel_md3_c::load: mesh '%s' in model '%s' has indexes number %i\n", pinmesh->name, name.c_str(), poutmesh->indexes_num);
		
		//
		// go to next mesh
		//
		pinmesh = (md3_dmesh_t*)((byte*)pinmesh + LittleLong(pinmesh->end_ofs));
	}
	
	_aabb.clear();
	for(std::vector<vec3_c>::const_iterator ir = vertexes.begin(); ir != vertexes.end(); ++ir)
	{
		_aabb.addPoint(*ir);
	}
	
	// reverse polygon indices to flip normals for ODE
	reverse(indexes.begin(), indexes.end());
}

