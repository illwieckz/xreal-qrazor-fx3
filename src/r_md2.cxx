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
#include 	"r_local.h"

// xreal --------------------------------------------------------------------


r_md2_model_c::r_md2_model_c(const std::string &name, byte *buffer, uint_t buffer_size)
:r_alias_model_c(name, buffer, buffer_size)
{
	//ri.Com_Printf ("r_md2_model_c::ctor: %s\n", name);
}

r_md2_model_c::~r_md2_model_c()
{
	//DO NOTHING
}

void	r_md2_model_c::load()
{
	int					i, j;
	int					version;
	int					framesize;
	int					skin_width;
	int					skin_height;
	int					indexes_num;
	int					vertexes_num;
	double					isw, ish;
	vec3_t					scale;
	
	int					indremap[MD2_MAX_TRIANGLES*3];
	index_t					ptmpindex[MD2_MAX_TRIANGLES*3];
	index_t					ptmpindex_st[MD2_MAX_TRIANGLES*3];
	
	md2_dheader_t				*pinmodel;
	md2_dtexcoord_t				*pintexcoord;
	md2_dtriangle_t				*pintri;
	md2_dframe_t				*pinframe;
	
	r_mesh_c				*poutmesh;
	r_model_shader_c			*poutshader;
	r_alias_mesh_frame_t			*poutmeshframe;
	r_alias_frame_t				*poutframe;
	

	pinmodel = (md2_dheader_t *)_buffer;
	version = LittleLong (pinmodel->version);
	framesize = LittleLong (pinmodel->framesize);
	
	if(version != MD2_VERSION)
		ri.Com_Error(ERR_DROP, "r_md2_model_c::load: %s has wrong version number (%i should be %i)", getName(), version, MD2_VERSION);
	
	// byte swap the header fields and sanity check
	//for (i=0 ; i<sizeof(md2_dmdl_t)/4 ; i++)
	//	((int *)pheader)[i] = LittleLong (((int *)buffer)[i]);
	
	
	skin_width = LittleLong(pinmodel->skin_width);
	skin_height = LittleLong(pinmodel->skin_height);
	
	if(skin_width <= 0)
		ri.Com_Error(ERR_DROP, "r_md2_model_c::load: model %s has invalid skin width", getName());
	
	if(skin_height <= 0)
		ri.Com_Error(ERR_DROP, "r_md2_model_c::load: model %s has invalid skin height", getName());
	
	
	isw = 1.0f / (double) skin_width;
	ish = 1.0f / (double) skin_height;
	
	int frames_num = LittleLong(pinmodel->frames_num);
	int shaders_num = LittleLong(pinmodel->skins_num);
	
	if(frames_num <= 0 || frames_num > MD2_MAX_FRAMES)
		ri.Com_Error(ERR_DROP, "r_md2_model_c::load: model %s has invalid frames number %i", getName(), frames_num);
			
	if(shaders_num < 0 || shaders_num > MD2_MAX_SKINS)
		ri.Com_Error(ERR_DROP, "r_md2_model_c::load: model %s has invalid shaders number %i", getName(), shaders_num);
		
		
	poutmesh = new r_mesh_c();
	_meshes.push_back(poutmesh);
	
	vertexes_num = LittleLong (pinmodel->vertexes_num);
	indexes_num = LittleLong(pinmodel->triangles_num)*3;
										
	
	if(vertexes_num <= 0 || vertexes_num > MD2_MAX_VERTS)
		ri.Com_Error (ERR_DROP, "r_md2_model_c::load: model %s has invalid vertices number", getName(), vertexes_num);

	if(indexes_num <= 0 || indexes_num > MD2_MAX_INDEXES)
		ri.Com_Error (ERR_DROP, "r_md2_model_c::load: model %s has invalid indexes number", getName(), indexes_num);
	

		


	//
	// load triangle lists
	//
	pintri = (md2_dtriangle_t*)((byte *)pinmodel + LittleLong(pinmodel->triangles_ofs));
	poutmesh->indexes = std::vector<index_t>(indexes_num);
	
	for(i=0; i<((int)poutmesh->indexes.size()/3); i++, pintri++)
	{
		for(j=0; j<3; j++)
		{
			ptmpindex[i*3+j] = (index_t)LittleShort(pintri->index_xyz[j]);
			ptmpindex_st[i*3+j] = (index_t)LittleShort(pintri->index_st[j]);
		}
	}

	//
	// build list of unique vertexes
	//
	vertexes_num = 0;
	
	
	memset(indremap, -1, sizeof(indremap));
	
	for(i=0; i<indexes_num; i++)
	{
		if(indremap[i] != -1)
			continue;
			
		for(j=0; j<indexes_num; j++)
		{
			if(j == i)
				continue;
				
			if((ptmpindex[i] == ptmpindex[j]) && (ptmpindex_st[i] == ptmpindex_st[j]))
				indremap[j] = i;
		}
	}
	
	// count unique vertexes
	for(i=0; i<indexes_num; i++)
	{
		if(indremap[i] != -1)
			continue;
			
		poutmesh->indexes[i] = vertexes_num++;
		indremap[i] = i;
	}

	// create lerp vertices for the mesh
	poutmesh->fillVertexes(vertexes_num);
	
	
	// remap remaining indexes
	for(i=0; i<indexes_num; i++)
	{
		if(indremap[i] != i)
			poutmesh->indexes[i] = poutmesh->indexes[indremap[i]];
	}		

	//
	// load the frames
	//
	pintexcoord = (md2_dtexcoord_t*)((byte*)pinmodel + LittleLong(pinmodel->texcoords_ofs));
	
	
	
	for (i=0; i<frames_num; i++/*, poutvertex += vertexes_num*/)
	{
		pinframe = (md2_dframe_t *)((byte*)pinmodel + LittleLong(pinmodel->frames_ofs) + i * framesize);
		poutframe = new r_alias_frame_t();
		poutmeshframe = new r_alias_mesh_frame_t();
		
		for(j=0; j<3; j++)
		{
			scale[j] = LittleFloat(pinframe->scale[j]);
			poutframe->translate[j] = LittleFloat(pinframe->translate[j]);
		}
		
		poutmeshframe->vertexes = std::vector<vec3_c>(vertexes_num);
		poutmeshframe->texcoords = std::vector<vec2_c>(vertexes_num);
		poutmeshframe->tangents = std::vector<vec3_c>(vertexes_num);
		poutmeshframe->binormals = std::vector<vec3_c>(vertexes_num);
		poutmeshframe->normals = std::vector<vec3_c>(vertexes_num);
		
		for(j=0; j<indexes_num; j++)
		{
			// verts are all 8 bit, so no swapping needed
			poutmeshframe->vertexes[poutmesh->indexes[j]][0] = (float) pinframe->vertexes[ptmpindex[indremap[j]]].v[0] * scale[0];
			poutmeshframe->vertexes[poutmesh->indexes[j]][1] = (float) pinframe->vertexes[ptmpindex[indremap[j]]].v[1] * scale[1];
			poutmeshframe->vertexes[poutmesh->indexes[j]][2] = (float) pinframe->vertexes[ptmpindex[indremap[j]]].v[2] * scale[2];
			
			poutmeshframe->normals[poutmesh->indexes[j]] = bytedirs[pinframe->vertexes[ptmpindex[indremap[j]]].lightnormalindex];
			
			poutmeshframe->normals[poutmesh->indexes[j]].normalize();
			
			poutmeshframe->texcoords[poutmesh->indexes[j]][0] = (float)(((double)LittleShort(pintexcoord[ptmpindex_st[indremap[j]]].st[0]) + 0.5f) * isw);
			poutmeshframe->texcoords[poutmesh->indexes[j]][1] = (float)(((double)LittleShort(pintexcoord[ptmpindex_st[indremap[j]]].st[1]) + 0.5f) * ish);
		}
		
		poutframe->meshframes.push_back(poutmeshframe);
		
		poutframe->bbox._mins = poutframe->translate;
		Vector3_MA(poutframe->translate, 255, scale, poutframe->bbox._maxs);
		
		poutframe->radius = poutframe->bbox.radius();
		
		_frames.push_back(poutframe);
	}

	
	//
	// build triangle neighbours
	//
	//poutmesh->neighbours = std::vector<int>(poutmesh->indexes.size()); //new int[poutmesh->indexes_num];
	//RB_BuildTriangleNeighbours(poutmesh->neighbours, poutmesh->indexes, poutmesh->indexes_num);
	
	
	//
	// register all skins
	//
	//for(i=0; i<shaders_num; i++)
	{
		std::string shadername;
		
		for(j=0; j<MD2_MAX_SKINNAME; j++)
			shadername += ((const char*)pinmodel + LittleLong(pinmodel->skins_ofs))[j];
	
		poutshader = new r_model_shader_c(shadername, R_RegisterShader(shadername), X_SURF_NONE, X_CONT_NONE);
		
		_shaders.push_back(poutshader);
		
		ri.Com_Printf("r_md2_model_c::load: model %s has shader '%s'\n", getName(), poutshader->getName());
	}
	
	//ri.Com_Printf("r_md2_model_c::load: loaded %i of %i frames\n", _frames.size(), frames_num);
}
