/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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


//static r_vertex_t		r_poly_vertexes[MAX_POLY_VERTEXES];
//static index_t			r_poly_indexes[(MAX_POLY_VERTEXES-2)*3];

static r_entity_t		r_poly_entity;
//static r_mesh_t			r_poly_mesh;
//static r_mesh_buffer_t		r_poly_mesh_buffer;

void 	R_InitPolys()
{
	/*
	int		i;
	index_t*	index;
	
	// trifan indexes setup
	for(i=0, index = r_poly_indexes; i<(MAX_POLY_VERTEXES-2); i++, index += 3)
	{
		index[0] = 0;
		index[1] = i + 1;
		index[2] = i + 2;
	}

	// basic setup
	r_poly_mesh.vertexes = r_poly_vertexes;
	r_poly_mesh.indexes = r_poly_indexes;
	
	r_poly_mesh_buffer.entity = &r_poly_entity;
	r_poly_mesh_buffer.mesh = &r_poly_mesh;
	*/
}

/*
void 	R_DrawPoly(r_mesh_buffer_t *mbuffer)
{
	
	int		i;
	r_poly_t*	poly;
	
	poly = &r_polys[-mbuffer->infokey-1];
	
	for(i=0; poly->vertexes_num; i++)
	{
		r_poly_vertexes[i] = poly->vertexes[i];
	}
	
	r_poly_mesh.vertexes_num = poly->vertexes_num;
	r_poly_mesh.indexes_num = (poly->vertexes_num - 2) * 3;
	
	r_poly_mesh_buffer.entity = mbuffer->entity;
	r_poly_mesh_buffer.shader = mbuffer->shader;
	
	RB_MeshMatrix(matrix_identity);
	RB_PushMesh(&r_poly_mesh, false, false);
	RB_RenderMeshBuffer(&r_poly_mesh_buffer, false);
}
*/

void 	R_AddPolysToBuffer()
{
	/*
	if(!r_drawpolygons->value)
		return;

	r_current_entity = &r_poly_entity;
	r_current_model = NULL;


	for(int i=0; i<r_polys_num; i++)
	{
		r_poly_t *poly = &r_polys[i];
		
		R_AddMeshToBuffer(NULL, NULL, R_GetShaderByNum(poly->shader), -(i+1));
	}
	*/
}




