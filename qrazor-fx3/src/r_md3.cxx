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


r_md3_model_c::r_md3_model_c(const std::string &name, byte *buffer, uint_t buffer_size)
:r_alias_model_c(name, buffer, buffer_size)
{
	//ri.Com_Printf ("r_md3_model_c::ctor: %s\n", name);
}

r_md3_model_c::~r_md3_model_c()
{
	//DO NOTHING
}

void	r_md3_model_c::load()
{
	int					i=0, j=0, k=0;
	int					version;
	float					alpha, beta;

	md3_dheader_t				*pinmodel;
	md3_dframe_t				*pinframe;
	md3_dmesh_t				*pinmesh;
	md3_dtag_t				*pintag;
	md3_dshader_t				*pinshader;
	md3_dtexcoord_t				*pintexcoord;
	md3_dvertex_t				*pinvertex;
	index_t					*pinindex;
	
	r_mesh_c				*poutmesh;
	r_model_shader_c			*poutshader;
	
	r_alias_mesh_frame_t			*poutmeshframe;
	r_alias_frame_t				*poutframe;
	r_alias_tag_t				*pouttag;
	

	pinmodel = (md3_dheader_t *)_buffer;
	version = LittleLong (pinmodel->version);
	
	if(version != MD3_VERSION)
		ri.Com_Error(ERR_DROP, "r_md3_model_c::load: %s has wrong version number (%i should be %i)", getName(), version, MD3_VERSION);

	
	int model_frames_num = LittleLong(pinmodel->frames_num);
	int meshes_num = LittleLong(pinmodel->meshes_num);
	int tags_num = LittleLong(pinmodel->tags_num);
	
		
	if(model_frames_num <= 0 || model_frames_num > MD3_MAX_FRAMES)
		ri.Com_Error(ERR_DROP, "r_md3_model_c::load: model %s has invalid frames number %i", getName(), model_frames_num);
	
	
	if(meshes_num <= 0 || meshes_num > MD3_MAX_MESHES)
		ri.Com_Error(ERR_DROP, "r_md3_model_c::load: model %s has invalid meshes number %i", getName(), meshes_num);
		
	if(tags_num < 0 || tags_num > MD3_MAX_TAGS)
		ri.Com_Error(ERR_DROP, "r_md3_model_c::load: model %s has invalid tags number %i", getName(), tags_num);
	
		

	//
	// load the frames
	//
	pinframe = (md3_dframe_t*)((byte*) pinmodel + LittleLong(pinmodel->frames_ofs));
	
	for(i=0; i<model_frames_num; i++, pinframe++)
	{
		poutframe = new r_alias_frame_t();
		
		for(j=0; j<3; j++)
		{
			poutframe->bbox._mins[j] = LittleFloat(pinframe->mins[j]);
			poutframe->bbox._maxs[j] = LittleFloat(pinframe->maxs[j]);
			
			poutframe->translate[j] = LittleFloat(pinframe->translate[j]);
		}
		
		
		poutframe->radius = LittleFloat (pinframe->radius);

		_frames.push_back(poutframe);
	}
	//ri.Com_Printf("r_md3_model_c::load: model %s has %i frames\n", getName(), _frames.size());
	
	
	//
	// load the tags
	//
	pintag = (md3_dtag_t*)((byte*) pinmodel + LittleLong(pinmodel->tags_ofs));
	
	for(i=0; i<model_frames_num; i++)
	{
		r_alias_frame_t *frame = _frames[i];
	
		for(j=0; j<tags_num; j++, pintag++)
		{
			pouttag = new r_alias_tag_t();
		
			matrix_c axis;
		
			for(k=0; k<3; k++)
			{
				pouttag->origin[k] = LittleFloat(pintag->origin[k]);
			
				axis[0][k] =  LittleFloat(pintag->axis[0][k]);
				axis[1][k] =  LittleFloat(pintag->axis[1][k]);
				axis[2][k] =  LittleFloat(pintag->axis[2][k]);
			}
			
			pouttag->quat.fromMatrix(axis);
			
			pouttag->name = pintag->name;
			
			//ri.Com_Printf("r_md3_model_c::load: model '%s' has tag '%s' at %s\n", getName(), pouttag->name, pouttag->orientation.origin.toString());
			
			frame->tags.push_back(pouttag);
		}
		
		//ri.Com_Printf("r_md3_model_c::load: frame %i has tags %i\n", i, frame->tags.size());
	}
	
	//
	// load the meshes
	//
	model_frames_num = 0;
	pinmesh = (md3_dmesh_t*)((byte*) pinmodel + LittleLong(pinmodel->meshes_ofs));
	
	for(i=0; i<meshes_num; i++)
	{
		poutmesh = new r_mesh_c();
		
		if(!X_strnequal((const char*)pinmesh->ident, MD3_IDENTSTRING, 4))
		{
			ri.Com_Error(ERR_DROP, "r_md3_model_c::load: mesh %s in model %s has wrong ident (%s should be %s)", 
						pinmesh->name, getName(), pinmesh->ident, MD3_IDENTSTRING);
		}
		
		int vertexes_num = LittleLong(pinmesh->vertexes_num);
		int indexes_num = LittleLong(pinmesh->triangles_num)*3;
		int shaders_num = LittleLong(pinmesh->shaders_num);
		int frames_num = LittleLong(pinmesh->frames_num);
		
		
		if(vertexes_num <= 0 || vertexes_num > MD3_MAX_VERTEXES)
			ri.Com_Error(ERR_DROP, "r_md3_model_c::load: mesh %s in model %s has invalid vertexes number %i", pinmesh->name, getName(), vertexes_num);
		
		if(indexes_num <= 0 || indexes_num > MD3_MAX_INDEXES)
			ri.Com_Error(ERR_DROP, "r_md3_model_c::load: mesh %s in model %s has invalid indexes number %i", pinmesh->name, getName(), indexes_num);
		
		if(shaders_num <= 0 || shaders_num > MD3_MAX_SHADERS)
			ri.Com_Error(ERR_DROP, "r_md3_model_c::load: mesh %s in model %s has invalid shaders number %i", pinmesh->name, getName(), shaders_num);
		
		
		//ri.Com_Printf("r_md3_model_c::load: mesh %s in model %s has frames number %i\n", pinmesh->name, getName(), frames_num);
		
		//
		// assign name for .skin support
		//
		poutmesh->name = pinmesh->name;
		
				
		//
		// load the shaders
		//
		pinshader = (md3_dshader_t*)((byte*)pinmesh + LittleLong(pinmesh->shaders_ofs));
			
		//for (j=0; j<shaders_num; j++, pinshader++)	// Tr3B - only grab first shader
		{
			//ri.Com_Printf("r_md3_model_c::load: needs shader '%s'\n", pinshader->name);
		
			poutshader = new r_model_shader_c(pinshader->name, R_RegisterShader(pinshader->name), X_SURF_NONE, X_CONT_NONE);
						
			_shaders.push_back(poutshader);
		}
		
		
		//
		// load the indexes
		//
		pinindex = (index_t*)((byte*)pinmesh + LittleLong(pinmesh->indexes_ofs));
		poutmesh->indexes = std::vector<index_t>(indexes_num);
	
		for(j=0; j<indexes_num; j++, pinindex++)
		{
			poutmesh->indexes[j] = (index_t) LittleLong(*pinindex);
		}
		//reverse(poutmesh->indexes.begin(), poutmesh->indexes.end());
	
	
		//
		// load the texcoords
		//
		pintexcoord = (md3_dtexcoord_t*)((byte*)pinmesh + LittleLong(pinmesh->texcoords_ofs));
		std::vector<vec2_c> texcoords;
		
		for(j=0; j<vertexes_num; j++, pintexcoord++)
		{
			vec2_c texcoord;
			
			texcoord[0] = LittleFloat(pintexcoord->st[0]);
			texcoord[1] = LittleFloat(pintexcoord->st[1]);
			
			texcoords.push_back(texcoord);
		}
		
				
		//
		// load the vertexes, normals and texcoords for each frame
		//
		pinvertex = (md3_dvertex_t*)((byte*)pinmesh + LittleLong(pinmesh->vertexes_ofs));
		
		for(j=0; j<frames_num; j++)
		{
			r_alias_frame_t* frame = _frames[j];
			
			poutmeshframe = new r_alias_mesh_frame_t();
			
			poutmeshframe->vertexes = std::vector<vec3_c>(vertexes_num);
			poutmeshframe->texcoords = std::vector<vec2_c>(vertexes_num);
			poutmeshframe->tangents = std::vector<vec3_c>(vertexes_num);
			poutmeshframe->binormals = std::vector<vec3_c>(vertexes_num);
			poutmeshframe->normals = std::vector<vec3_c>(vertexes_num);
		
			for(k=0; k<vertexes_num; k++, pinvertex++)
			{
				poutmeshframe->vertexes[k][0] = (float)LittleShort(pinvertex->vertex[0]) * MD3_VERTEX_SCALE;
				poutmeshframe->vertexes[k][1] = (float)LittleShort(pinvertex->vertex[1]) * MD3_VERTEX_SCALE;
				poutmeshframe->vertexes[k][2] = (float)LittleShort(pinvertex->vertex[2]) * MD3_VERTEX_SCALE;
				
				int normal = LittleShort(pinvertex->normal);
								
				alpha = ((float)((normal)&255) * M_TWOPI / 255.0);
				beta = ((float)((normal>>8)&255) * M_TWOPI / 255.0);
							
				poutmeshframe->normals[k][0] = cos(beta) * sin(alpha);
				poutmeshframe->normals[k][1] = sin(beta) * sin(alpha);
				poutmeshframe->normals[k][2] = cos(alpha);
				
				poutmeshframe->normals[k].normalize();
				
				poutmeshframe->texcoords[k][0] = texcoords[k][0];
				poutmeshframe->texcoords[k][1] = texcoords[k][1];
			}
			
			frame->meshframes.push_back(poutmeshframe);
		}
		
		texcoords.clear();
		
		
		//
		// create dummy vertexes which will be lerped later
		//
		poutmesh->fillVertexes(vertexes_num, false, true);
		
			
		//
		// build triangle neighbours
		//
		//poutmesh->neighbours = new int[poutmesh->indexes_num];
		//RB_BuildTriangleNeighbours(poutmesh->neighbours, poutmesh->indexes, poutmesh->indexes_num);
		
		
		//
		// put in list
		//
		_meshes.push_back(poutmesh);
		
		
		//ri.Com_Printf("r_md3_model_c::load: mesh %s in model %s has indexes number %i\n", pinmesh->name, getName(), poutmesh->indexes_num);
		
		//
		// go to next mesh
		//
		pinmesh = (md3_dmesh_t*)((byte*)pinmesh + LittleLong(pinmesh->end_ofs));
	}
	
#if 0
	ri.Com_Printf("r_md3_model_c::load: model %s has %i frames\n", getName(), _frames.size());
	ri.Com_Printf("r_md3_model_c::load: model %s has %i meshes\n", getName(), _meshes.size());
	ri.Com_Printf("r_md3_model_c::load: model %s has %i tags\n", getName(), tags_num);
#endif

}
