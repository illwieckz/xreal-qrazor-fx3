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
r_mdc_model_c::r_mdc_model_c(const std::string &name, byte *buffer, int max_size)
:r_alias_model_c(name, buffer, max_size)
{
	//ri.Com_Printf ("r_alias_md3_model_c::ctor: %s\n", name);
}

r_mdc_model_c::~r_mdc_model_c()
{
	//DO NOTHING
}

void	r_mdc_model_c::load()
{
	int					i, j, k;
	int					version;
	float					alpha, beta;

	mdc_dheader_t				*pinmodel;
	mdc_dframe_t				*pinframe;
	mdc_dmesh_t				*pinmesh;
	mdc_dshader_t				*pinshader;
	mdc_dtexcoord_t				*pintexcoord;
	mdc_dvertex_t				*pinvertex;
	mdc_dvertex_comp_t			*pinvertex_comp;
	index_t					*pinindex;
	
	index_t					*poutindex;
	r_vertex_t				*poutvertex;	
	r_mesh_c				*poutmesh;
	r_model_shader_t			*poutshader;
	
	r_alias_frame_t				*poutframe;
	

	pinmodel = (mdc_dheader_t *)_buffer;
	version = LittleLong (pinmodel->version);
	
	if (version != MDC_ALIAS_VERSION)
		ri.Com_Error (ERR_DROP, "r_mdc_model_c::load: %s has wrong version number (%i should be %i)", getName(), version, MDC_ALIAS_VERSION);
	
	// byte swap the header fields and sanity check
	//for (i=0 ; i<sizeof(md2_dmdl_t)/4 ; i++)
	//	((int *)pheader)[i] = LittleLong (((int *)buffer)[i]);
	
	int frames_num = LittleLong (pinmodel->frames_num);
	int frames_comp_num = 0;
	int meshes_num = LittleLong (pinmodel->meshes_num);
	int shaders_num = 0;
	
		
	if (frames_num <= 0 || frames_num > MDC_MAX_FRAMES)
		ri.Com_Error (ERR_DROP, "r_mdc_model_c::load: model %s has invalid frames number %i", getName(), frames_num);
	
	if (meshes_num <= 0 || meshes_num > MDC_MAX_MESHES)
		ri.Com_Error (ERR_DROP, "r_mdc_model_c::load: model %s has invalid meshes number %i", getName(), meshes_num);
	
	
		

	//
	// load the frames
	//
	pinframe = (mdc_dframe_t*) ((byte*) pinmodel + LittleLong (pinmodel->frames_ofs));
	//poutframe = _frames = (r_alias_frame_t*) ri.Hunk_Alloc (_frames_num * sizeof (r_alias_frame_t));
	
	//_radius = 0;
	//_bbox.clear();
	
	for (i=0; i<frames_num; i++, pinframe++)
	{
		poutframe = (r_alias_frame_t*) ri.Hunk_Alloc(sizeof(r_alias_frame_t));
		
		for (j=0; j<3; j++)
		{
			poutframe->bbox._mins[j] = LittleFloat (pinframe->mins[j]);
			poutframe->bbox._maxs[j] = LittleFloat (pinframe->maxs[j]);
			
			poutframe->translate[j] = LittleFloat (pinframe->translate[j]);
		}
		
		
		poutframe->radius = LittleFloat (pinframe->radius);

		_frames.push_back(poutframe);
	}
	
	
	//
	// load the meshes
	//
	pinmesh = (mdc_dmesh_t*)((byte*)pinmodel + LittleLong(pinmodel->meshes_ofs));
	
	for(i=0; i<meshes_num; i++)
	{
		poutmesh = new r_alias_mesh_t();
		
		int vertexes_num = LittleLong(pinmesh->vertexes_num);
		poutmesh->indexes_num = LittleLong(pinmesh->triangles_num)*3;
		
		frames_num = LittleLong (pinmesh->frames_num);
		frames_comp_num = LittleLong (pinmesh->frames_comp_num);
		shaders_num = LittleLong (pinmesh->shaders_num);
		
		ri.Com_Printf("r_mdc_model_c::load: compressed frames %i\n", frames_comp_num);
		//ri.Com_Printf("r_mdc_model_c::load: vertexes %i\n", poutmesh->vertexes_num);
		
		
		if(vertexes_num <= 0 || vertexes_num > MDC_MAX_VERTEXES)
			ri.Com_Error(ERR_DROP, "r_mdc_model_c::load: mesh %s in model %s has invalid vertexes number %i", pinmesh->name, getName(), vertexes_num);
		
		if(poutmesh->indexes_num <= 0 || poutmesh->indexes_num > MDC_MAX_INDEXES)
			ri.Com_Error(ERR_DROP, "r_mdc_model_c::load: mesh %s in model %s has invalid indexes number %i", pinmesh->name, getName(), poutmesh->indexes_num);
		
		if(shaders_num <= 0 || shaders_num > MDC_MAX_SHADERS)
			ri.Com_Error(ERR_DROP, "r_mdc_model_c::load: mesh %s in model %s has invalid shaders number %i", pinmesh->name, getName(), shaders_num);
			
			
		//
		// load the shaders
		//
		pinshader = (mdc_dshader_t*) ((byte*) pinmesh + LittleLong (pinmesh->shaders_ofs));
			
		//for (j=0; j<shaders_num; j++, pinshader++)	// Tr3B - only grab first shader
		{
			poutshader = NULL;
			poutshader = (r_model_shader_t*) ri.Hunk_Alloc (sizeof (r_model_shader_t));
			
			Com_sprintf (poutshader->name, sizeof (poutshader->name), pinshader->name);
			
			_shaders.push_back(poutshader);
		}
		
		//
		// load the indexes
		//
		pinindex = (index_t*) ((byte*) pinmesh + LittleLong (pinmesh->indexes_ofs));
		poutindex = poutmesh->indexes = (index_t*) ri.Hunk_Alloc (poutmesh->indexes_num * sizeof (index_t));
	
		for (j=0; j<(poutmesh->indexes_num/3); j++, pinindex += 3, poutindex += 3)
		{
			poutindex[0] = (index_t) LittleLong (pinindex[0]);
			poutindex[1] = (index_t) LittleLong (pinindex[1]);
			poutindex[2] = (index_t) LittleLong (pinindex[2]);
		}
	
			
		//
		// load the vertexes, normals and texcoords
		//
		
		// load compressed vertexes
		pintexcoord = (mdc_dtexcoord_t*)((byte*)pinmesh + LittleLong(pinmesh->texcoords_ofs));
		pinvertex_comp = (mdc_dvertex_comp_t*)((byte*)pinmesh + LittleLong(pinmesh->vertexes_comp_ofs));
		
		//if(frames_comp_num)
		//	poutvertex = poutmesh->vertexes = (r_vertex_t*) ri.Hunk_Alloc(vertexes_num * (frames_num + frames_comp_num) * sizeof (r_vertex_t));
		//else
		//	poutvertex = poutmesh->vertexes = (r_vertex_t*) ri.Hunk_Alloc(poutmesh->vertexes_num * frames_num * sizeof (r_vertex_t));
		
		for(j=0; j<frames_comp_num; j++)
		{
			for (k=0; k<vertexes_num; k++, pinvertex_comp++, pintexcoord++)
			{
				poutvertex = (r_vertex_t*)ri.Hunk_Alloc(sizeof(r_vertex_t));
				
				int vertex = LittleLong(pinvertex_comp->vertex);
				
				// decompress vertex position
				poutvertex->position[0] = (float)((((vertex)&255) - 127) * MDC_VERTEX_COMP_SCALE);
				poutvertex->position[1] = (float)((((vertex>>8)&255) - 127) * MDC_VERTEX_COMP_SCALE);
				poutvertex->position[2] = (float)((((vertex>>16)&255) - 127) * MDC_VERTEX_COMP_SCALE);
				
				
				//alpha = (float) pinvertex->normal[0] * M_TWOPI / 255.0;
				//beta = (float) pinvertex->normal[1] * M_TWOPI / 255.0;
							
				//poutvertex->normal[0] = cos(beta) * sin(alpha);
				//poutvertex->normal[1] = sin(beta) * sin(alpha);
				//poutvertex->normal[2] = cos(alpha);
				
				poutvertex->normal.clear();
				
				poutvertex->texcoord[0] = LittleFloat(pintexcoord->st[0]);
				poutvertex->texcoord[1] = LittleFloat(pintexcoord->st[1]);
				
				poutmesh->vertexes.push_back(poutvertex);
				poutmesh->vertexes.push_back(new r_vertex_t());
			}
		}
		
		// load base vertexes
		//pintexcoord = (mdc_dtexcoord_t*) ((byte*) pinmesh + LittleLong (pinmesh->texcoords_ofs));
		pinvertex = (mdc_dvertex_t*) ((byte*)pinmesh + LittleLong (pinmesh->vertexes_ofs));
		//poutvertex = poutbasevertexes = (r_vertex_t*) ri.Hunk_Alloc (poutmesh->vertexes_num * frames_num * sizeof (r_vertex_t));
		
		for(j=0; j<frames_num; j++)
		{
			for(k=0; k<vertexes_num; k++, pinvertex++, pintexcoord++)
			{
				poutvertex = (r_vertex_t*)ri.Hunk_Alloc(sizeof(r_vertex_t));
				
				poutvertex->position[0] = (float)(LittleShort (pinvertex->vertex[0]) * MDC_VERTEX_SCALE);
				poutvertex->position[1] = (float)(LittleShort (pinvertex->vertex[1]) * MDC_VERTEX_SCALE);
				poutvertex->position[2] = (float)(LittleShort (pinvertex->vertex[2]) * MDC_VERTEX_SCALE);
				
				int normal = LittleShort (pinvertex->normal);
								
				alpha = ((float)((normal)&255) * M_TWOPI / 255.0);
				beta = ((float)((normal>>8)&255) * M_TWOPI / 255.0);
							
				poutvertex->normal[0] = cos(beta) * sin(alpha);
				poutvertex->normal[1] = sin(beta) * sin(alpha);
				poutvertex->normal[2] = cos(alpha);
				
				poutvertex->normal.normalize();
				
				poutvertex->texcoord[0] = LittleFloat (pintexcoord->st[0]);
				poutvertex->texcoord[1] = LittleFloat (pintexcoord->st[1]);
				
				poutmesh->vertexes.push_back(poutvertex);
				poutmesh->vertexes.push_back(new r_vertex_t());
			}
		}
		
				
		
			
		//
		// build triangle neighbours
		//
		poutmesh->neighbours_tr = (int*)ri.Hunk_Alloc(poutmesh->indexes_num * sizeof (int));
		RB_BuildTriangleNeighbours (poutmesh->neighbours_tr, poutmesh->indexes, poutmesh->indexes_num);
		
		
		//
		// put in list
		//
		_meshes.push_back(poutmesh);
		
		//
		// go to next mesh
		//
		pinmesh = (mdc_dmesh_t*)((byte*)pinmesh + LittleLong(pinmesh->end_ofs));
	}
}
#endif
