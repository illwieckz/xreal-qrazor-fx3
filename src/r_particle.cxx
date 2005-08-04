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


static r_entity_c		r_part_entity;
static r_mesh_c			r_part_mesh;

void 	R_InitParticles()
{
	r_part_mesh.fillVertexes(4, false, false);
	
	r_part_mesh.indexes = r_quad_indexes;
	
	// specific setup
	r_part_mesh.texcoords[0][0] = 0;
	r_part_mesh.texcoords[0][1] = 0;
	
	r_part_mesh.texcoords[1][0] = 1;
	r_part_mesh.texcoords[1][1] = 0;
	
	r_part_mesh.texcoords[2][0] = 1;
	r_part_mesh.texcoords[2][1] = 1;
	
	r_part_mesh.texcoords[3][0] = 0;
	r_part_mesh.texcoords[3][1] = 1;
}



void	R_DrawParticles()
{
	const r_particle_t*	p;
	int			i;
	vec3_c			up, right;
	vec3_c			corner;

	if(!r_drawparticles->getInteger())
		return;

	if(!r_particles_num)
		return;

	Vector3_Scale(r_up, 1.5, up);
	Vector3_Scale(r_right, 1.5, right);

	RB_SetupModelviewMatrix(matrix_identity);
	
//	xglDepthMask(GL_FALSE);

	for(i=0, p = r_particles; i<r_particles_num; i++, p++)
	{
		r_part_entity.setColor(p->color);
		
		corner[0] = p->origin[0] - 0.5f * (up[0] + right[0]);
		corner[1] = p->origin[1] - 0.5f * (up[1] + right[1]);
		corner[2] = p->origin[2] - 0.5f * (up[2] + right[2]);

		r_part_mesh.vertexes[0].set(corner[0]+up[0]+right[0], corner[1]+up[1]+right[1], corner[2]+up[2]+right[2]);
		r_part_mesh.vertexes[1].set(corner[0]+up[0]         , corner[1]+up[1]         , corner[2]+up[2]         );
		r_part_mesh.vertexes[2].set(corner[0]               , corner[1]               , corner[2]               );
		r_part_mesh.vertexes[3].set(corner[0]+      right[0], corner[1]      +right[1], corner[2]      +right[2]);
		
		// build cmd
		r_command_t	cmd(	&r_part_entity,
					NULL,
					&r_part_mesh,
					R_GetShaderByNum(p->shader),
				
					NULL,
					NULL,
					
					NULL,
				
					0,
					
					r_origin.distance(p->origin));


		RB_RenderCommand(&cmd, RENDER_TYPE_DEFAULT);
	}
	
//	xglDepthMask(GL_TRUE);
}



