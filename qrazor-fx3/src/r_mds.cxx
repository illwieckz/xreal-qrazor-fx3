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
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include 	"r_local.h"

// xreal --------------------------------------------------------------------


#if 0
r_mds_model_c::r_mds_model_c(const std::string &name, byte *buffer)
:r_skel_model_c(name, buffer)
{
	//ri.Com_Printf("r_mds_model_c::ctor: %s\n", name.c_str());
}

r_mds_model_c::~r_mds_model_c()
{
	//DO NOTHING
}

void	r_mds_model_c::load()
{
	int					i, j, k;
	int					version;

	mds_dheader_t				*pinmodel;
	//mds_dframe_t				*pinframe;
	//mds_dcompressed_bone_frame_t		*pincompressedboneframe;
	mds_dmesh_t				*pinmesh;
	mds_dbone_t				*pinbone;
	//mds_dtag_t				*pintag;
	mds_dvertex_t				*pinvertex;
	mds_dweight_t				*pinweight;
	index_t					*pinindex;
	
	r_mesh_c				*poutmesh;
	r_model_shader_t			*poutshader;
	
	r_skel_vertex_t				*poutvertex;
	r_skel_weight_t				*poutweight;
	r_skel_bone_t				*poutbone;
	//r_skel_frame_t			*poutframe;
	//r_skel_bone_frame_t			*poutboneframe;
	//r_skel_tag_t				*pouttag;
	
	

	pinmodel = (mds_dheader_t *)_buffer;
	version = LittleLong(pinmodel->version);
	
	if(version != MDS_VERSION)
		ri.Com_Error(ERR_DROP, "r_mds_model_c::load: %s has wrong version number (%i should be %i)", getName(), version, MDS_VERSION);
	

	
	int frames_num	= LittleLong(pinmodel->frames_num);
	int meshes_num	= LittleLong(pinmodel->meshes_num);
	int bones_num 	= LittleLong(pinmodel->bones_num);
	int tags_num	= LittleLong(pinmodel->tags_num);
	//int shaders_num	= 0;
	
		
	if(frames_num <= 0 || frames_num > MDS_MAX_FRAMES)
		ri.Com_Error(ERR_DROP, "r_alias_mds_model_c::load: model %s has invalid frames number %i", getName(), frames_num);
	
	if(meshes_num <= 0 || meshes_num > MDS_MAX_MESHES)
		ri.Com_Error(ERR_DROP, "r_alias_mds_model_c::load: model %s has invalid meshes number %i", getName(), meshes_num);
		
	if(bones_num <= 0 || bones_num > MDS_MAX_BONES)
		ri.Com_Error(ERR_DROP, "r_alias_mds_model_c::load: model %s has invalid bones number %i", getName(), bones_num);
		
	if(tags_num < 0 || tags_num > MDS_MAX_TAGS)
		ri.Com_Error(ERR_DROP, "r_alias_mds_model_c::load: model %s has invalid tags number %i", getName(), tags_num);
	
	
	//
	// load the frames
	//
	/*	
	int frames_ofs = 0;
	int bones_ofs = 0;
	for(i=0; i<frames_num; i++)
	{
		frames_ofs = i*sizeof(mds_dframe_t);
	
		pinframe = (mds_dframe_t*)((byte*)pinmodel + (LittleLong(pinmodel->frames_ofs) + (frames_ofs+bones_ofs)));
	
		poutframe = new r_skel_frame_t();
		
		// load generic frame information
		for(j=0; j<3; j++)
		{
			poutframe->bbox._mins[j] = LittleFloat(pinframe->mins[j]);
			poutframe->bbox._maxs[j] = LittleFloat(pinframe->maxs[j]);
			
			poutframe->translate[j] = LittleFloat(pinframe->translate[j]);
			
			poutframe->parent_offset[j] = LittleFloat(pinframe->parent_offset[j]);
		}
		
		//ri.Com_Printf("r_mds_model_c::load: frame %i has local origin %s\n", i, poutframe->translate.toString());
		//ri.Com_Printf("r_mds_model_c::load: frame %i has parent ofs   %s\n", i, poutframe->parent_offset.toString());
		
		poutframe->radius = LittleFloat(pinframe->radius);
		
		//_radius = max(_radius, poutframe->radius);
		
		//_bbox.addPoint(poutframe->bbox._mins);
		//_bbox.addPoint(poutframe->bbox._maxs);
		
		//pinframe++
		
		pincompressedboneframe = (mds_dcompressed_bone_frame_t*)((byte*)pinmodel + (LittleLong(pinmodel->frames_ofs) + ((i+1)*sizeof(mds_dframe_t) + bones_ofs)));
		
		// load bone poses
		for(j=0; j<bones_num; j++, pincompressedboneframe++)
		{
			poutboneframe = new r_skel_bone_frame_t();
		
			// get rotation
			vec3_c	angles_rot;
			for(k=0; k<3; k++)
				angles_rot[k] = (float)LittleShort(pincompressedboneframe->angles[k]) * (360.0/65536.0);
				
			// create rotation quaternion
			poutboneframe->rot_quat.fromAngles(angles_rot);
			
			// get offset rotation
			vec3_c	angles_ofs;
			angles_ofs[0] = (float)LittleShort(pincompressedboneframe->angles_ofs[0]) * (360.0/65536.0);
			angles_ofs[1] = (float)LittleShort(pincompressedboneframe->angles_ofs[1]) * (360.0/65536.0);
			
			// create offset quaternion
			poutboneframe->ofs_quat.fromAngles(angles_ofs);
			
			
						
			poutframe->boneframes.push_back(poutboneframe);
			
			bones_ofs += sizeof(mds_dcompressed_bone_frame_t);
		}

		_frames.push_back(poutframe);
	}
	*/
	
	
	//
	// load the bones
	//
	pinbone = (mds_dbone_t*)((byte*)pinmodel + LittleLong(pinmodel->bones_ofs));
	
	for(i=0; i<bones_num; i++, pinbone++)
	{
		poutbone = new r_skel_bone_t();
		
		poutbone->name = pinbone->name;
		poutbone->parent_index = LittleLong(pinbone->parent_index);
		//poutbone->torso_weight = LittleFloat(pinbone->torso_weight);
		//poutbone->parent_dist = LittleFloat(pinbone->parent_dist);
		//poutbone->flags = LittleLong(pinbone->flags);
		
		//ri.Com_Printf("r_mds_model_c::load: model '%s' has bone '%s'\n", getName(), poutbone->name);
		
		_bones.push_back(poutbone);
	}
	
	
	//
	// load the tags
	//
	/*
	pintag = (mds_dtag_t*)((byte*)pinmodel + LittleLong(pinmodel->tags_ofs));
		
	for(j=0; j<tags_num; j++, pintag++)
	{
		pouttag = new r_skel_tag_t();
		
		pouttag->name = pintag->name;
		pouttag->torso_weight = LittleFloat(pintag->torso_weight);
		pouttag->bone_index = LittleLong(pintag->bone_index);
			
		//ri.Com_Printf("r_mds_model_c::load: model '%s' has tag '%s'\n", getName(), pouttag->name);
			
		_tags.push_back(pouttag);
	}
	*/
	
	//
	// load the meshes
	//
	pinmesh = (mds_dmesh_t*)((byte*)pinmodel + LittleLong(pinmodel->meshes_ofs));
	
	//for(i=0; i<meshes_num; i++)
	{
		poutmesh = new r_mesh_c();
		
		
		int vertexes_num = LittleLong(pinmesh->vertexes_num);
		int indexes_num = LittleLong(pinmesh->triangles_num)*3;
		//int	bones_num = LittleLong(pinmesh->bones_num);
	
		
		if(vertexes_num <= 0 || vertexes_num > MDS_MAX_VERTEXES)
			ri.Com_Error(ERR_DROP, "r_mds_model_c::load: mesh '%s' in model %s has invalid vertexes number %i", pinmesh->name, getName(), vertexes_num);
		
		if(indexes_num <= 0 || indexes_num > MDS_MAX_INDEXES)
			ri.Com_Error(ERR_DROP, "r_mds_model_c::load: mesh '%s' in model %s has invalid indexes number %i", pinmesh->name, getName(), indexes_num);
			
		
		//
		// assign name for .skin support
		//
		poutmesh->name = pinmesh->name;
		
		ri.Com_Printf("r_mds_model_c::load: mesh %s\n", pinmesh->name);
					
			
		//
		// load the shader
		//
		poutshader = new r_model_shader_t(pinmesh->shader, R_RegisterShader(pinmesh->shader));
			
		_shaders.push_back(poutshader);
		
		
		//
		// load the indexes
		//
		pinindex = (index_t*)((byte*)pinmesh + LittleLong(pinmesh->triangles_ofs));
		poutmesh->indexes = std::vector<index_t>(indexes_num);
	
		for(j=0; j<indexes_num; j++, pinindex++)
		{
			poutmesh->indexes[j] = (index_t) LittleLong(*pinindex);
		}
			
			
		//
		// load the vertexes, normals and texcoords
		//
		
		int vertexes_ofs = 0;
		int weights_ofs = 0;
		for(j=0; j<vertexes_num; j++, pinvertex++)
		{
			vertexes_ofs = j*sizeof(mds_dvertex_t);
			pinvertex = (mds_dvertex_t*)((byte*)pinmesh + (LittleLong(pinmesh->vertexes_ofs) + (vertexes_ofs+weights_ofs)));
		
			poutvertex = new r_skel_vertex_t();
				
			poutvertex->normal[0] = LittleFloat(pinvertex->normal[0]);
			poutvertex->normal[1] = LittleFloat(pinvertex->normal[1]);
			poutvertex->normal[2] = LittleFloat(pinvertex->normal[2]);
			
			poutvertex->normal.normalize();
			
			//ri.Com_Printf("r_mds_model_c::load: mesh %s has vertex normal %i %s\n", pinmesh->name, j, poutvertex->normal.toString());
				
			poutvertex->texcoord[0] = LittleFloat(pinvertex->st[0]);
			poutvertex->texcoord[1] = LittleFloat(pinvertex->st[1]);
			
			//ri.Com_Printf("r_mds_model_c::load: mesh %s has vertex texcoord %i %s\n", pinmesh->name, j, poutvertex->texcoord.toString());
			
			//poutvertex->fixed_parent = LittleLong(pinvertex->fixed_parent);
			//poutvertex->fixed_dist = LittleFloat(pinvertex->fixed_dist);
	
			int weights_num = LittleLong(pinvertex->weights_num);
			
			//ri.Com_Printf("r_mds_model_c::load: mesh %s has vertex %i with %i weights\n", pinmesh->name, j, weights_num);
	
			pinweight = (mds_dweight_t*)((byte*)pinmesh + (LittleLong(pinmesh->vertexes_ofs) + ((j+1)*sizeof(mds_dvertex_t) + weights_ofs)));
			
			// load vertex weights
			for(k=0; k<weights_num; k++, pinweight++)
			{
				poutweight = new r_skel_weight_t();
				
				poutweight->bone_index = LittleLong(pinweight->bone_index);
				poutweight->weight = LittleFloat(pinweight->weight);
				
 				poutweight->position[0] = LittleFloat(pinweight->origin[0]);
				poutweight->position[1] = LittleFloat(pinweight->origin[1]);
				poutweight->position[2] = LittleFloat(pinweight->origin[2]);
				
				//ri.Com_Printf("r_mds_model_c::load: mesh %s has vertex %i weight %i bone_index %i\n", pinmesh->name, j, k, poutvertexweight->bone_index);
		
				poutvertex->weights.push_back(poutweight);
			
				weights_ofs += sizeof(mds_dweight_t);
			}
			
			poutmesh->vertexes.push_back(poutvertex);
		}
		
			
		//
		// build triangle neighbours
		//
		//poutmesh->neighbours = new int[poutmesh->indexes_num];
		//RB_BuildTriangleNeighbours(poutmesh->neighbours, poutmesh->indexes, poutmesh->indexes_num);
		
		//
		// put in list
		//
		_meshes.push_back(poutmesh);
		
		//ri.Com_Printf("r_mds_model_c::load: mesh %s in model %s has vertexes number %i\n", pinmesh->name, getName(), poutmesh->vertexes.size());
		
		//
		// go to next mesh
		//
		pinmesh = (mds_dmesh_t*)((byte*)pinmesh + LittleLong(pinmesh->end_ofs));
	}
	
#if 0
	ri.Com_Printf("r_mds_model_c::load: model '%s' has %i frames\n", getName(), _frames.size());
	ri.Com_Printf("r_mds_model_c::load: model '%s' has %i bones\n", getName(), _bones.size());
	ri.Com_Printf("r_mds_model_c::load: model '%s' has %i meshes\n", getName(), _meshes.size());
	ri.Com_Printf("r_mds_model_c::load: model '%s' has %i tags\n", getName(), _tags.size());
#endif
}
#endif
