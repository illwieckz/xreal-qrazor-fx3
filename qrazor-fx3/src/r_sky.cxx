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





//static index_t		sky_dome_indexes[SKY_ELEM_LEN];
//static r_mesh_buffer_t	sky_dome_mesh_buffer;
//static r_surface_t*		sky_warp;


static vec5_t	sky_vecs[24] = 
{
	{  1,  1,  1, 1, 0 }, {  1,  1, -1, 1, 1 }, { -1,  1, -1, 0, 1 }, { -1,  1,  1, 0, 0 },
	{ -1,  1,  1, 1, 0 }, { -1,  1, -1, 1, 1 }, { -1, -1, -1, 0, 1 }, { -1, -1,  1, 0, 0 },
	{ -1, -1,  1, 1, 0 }, { -1, -1, -1, 1, 1 }, {  1, -1, -1, 0, 1 }, {  1, -1,  1, 0, 0 },
	{  1, -1,  1, 1, 0 }, {  1, -1, -1, 1, 1 }, {  1,  1, -1, 0, 1 }, {  1,  1,  1, 0, 0 },
	{  1, -1,  1, 1, 0 }, {  1,  1,  1, 1, 1 }, { -1,  1,  1, 0, 1 }, { -1, -1,  1, 0, 0 },
	{  1,  1, -1, 1, 0 }, {  1, -1, -1, 1, 1 }, { -1, -1, -1, 0, 1 }, { -1,  1, -1, 0, 0 }
};



static void R_MakeSkyVec(vec5_t sky_vec)
{
	vec3_c		v;
	vec2_c		st;

	v.set(sky_vec[0], sky_vec[1], sky_vec[2]);
	v *= 4096.0 * 2;
	
	st.set(sky_vec[3] * (254.0/256.0) + (1.0f/256.f),
		sky_vec[4] * (254.0/256.0) + (1.0f/256.f));

	xglTexCoord2fv(st);
	xglVertex3fv((float*)((vec_t*)v));
}


void	R_DrawFastSkyBox()
{
	xglBegin(GL_QUADS);
	for(int i=0; i<6; i++)
	{
		R_MakeSkyVec (sky_vecs[i*4+0]);
		R_MakeSkyVec (sky_vecs[i*4+1]);
		R_MakeSkyVec (sky_vecs[i*4+2]);
		R_MakeSkyVec (sky_vecs[i*4+3]);
	}
	xglEnd ();
}


void 	R_InitSkyBox()
{
	//TODO
}


/*
void	R_CreateSkyDome (r_shader_t *shader, float sky_height)
{
	int		i;
	r_sky_dome_t*		sky_dome;
	r_mesh_t*		mesh;
	r_mesh_buffer_t*	mbuffer = &sky_dome_mesh_buffer;
	
	//sortkey;
	//mbuffer->dlights.clear();
	//mbuffer->entity
	//mbuffer->mesh
	mbuffer->shader = shader;
	
	sky_dome = shader->sky_dome;
	
	for(i=0, mesh = sky_dome->meshes; i<5; i++, mesh++)
	{
		mesh->vertexes_num = SKY_POINTS_LEN; 
		mesh->vertexes = (r_vertex_t*) ri.Z_Malloc (sizeof (r_vertex_t) * SKY_POINTS_LEN);
		
	
		mesh->indexes_num = SKY_ELEM_LEN;
		mesh->indexes = sky_dome_indexes;
	}

	R_GenSkyBox (sky_dome, sky_height);
}
*/

void 	R_ClearSkyBox()
{
	//TODO
	
	/*
	for (int i=0 ; i<6 ; i++)
	{
		r_current_list->sky_mins[0][i] = r_current_list->sky_mins[1][i] =  9999;
		r_current_list->sky_maxs[0][i] = r_current_list->sky_maxs[1][i] = -9999;
	}
	*/
}

void		R_DrawSkyDome()
{
	//TODO
}

void	R_DrawSky()
{
	if(!r_drawsky->getValue())
		return;
		
	if(r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	RB_DrawSkyBox();
}


void 	R_AddSkySurface(r_surface_c *surf)
{
	//TODO
	
#if 0
	int			i;
	vec3_t		verts[MAX_CLIP_VERTS];
	//glpoly_t	*p;

	// calculate vertex values for sky box
	for (i=0 ; i<p->numverts ; i++)
	{
		Vector3_Subtract (p->verts[i], r_origin, verts[i]);
	}
		R_ClipSkyPolygon (p->numverts, verts[0], 0);
#endif	
}


void 	R_SetSky(const std::string &name)
{
	r_image_c *sky = R_FindImage(name, IMAGE_NONE, IMAGE_UPLOAD_CUBEMAP);
	
	if(sky)
		r_img_cubemap_sky = sky;
}


