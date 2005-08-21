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
#include "r_backend.h"


static r_entity_c	r_draw_entity;
static r_mesh_c		r_draw_mesh;

void 	R_InitDraw()
{
	r_draw_mesh.fillVertexes(4);

	r_draw_mesh.indexes = r_quad_indexes;
}




void	R_DrawStretchPic(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, r_shader_c *shader)
{
	if(!r_draw2d->getValue())
		return;

	if(!shader)
		return;
				
	// lower left
	r_draw_mesh.vertexes[0][0] = x;
	r_draw_mesh.vertexes[0][1] = y;
	
	r_draw_mesh.texcoords[0][0] = s1;
	r_draw_mesh.texcoords[0][1] = t1;
	
	
	// lower right
	r_draw_mesh.vertexes[1][0] = x+w;
	r_draw_mesh.vertexes[1][1] = y;
	
	r_draw_mesh.texcoords[1][0] = s2;
	r_draw_mesh.texcoords[1][1] = t1;
	
	
	// upper right
	r_draw_mesh.vertexes[2][0] = x+w;
	r_draw_mesh.vertexes[2][1] = y+h;
	
	r_draw_mesh.texcoords[2][0] = s2;
	r_draw_mesh.texcoords[2][1] = t2;
	
	
	// upper left
	r_draw_mesh.vertexes[3][0] = x;
	r_draw_mesh.vertexes[3][1] = y+h;
	
	r_draw_mesh.texcoords[3][0] = s1;
	r_draw_mesh.texcoords[3][1] = t2;
	
	// set color
	r_draw_entity.setColor(color);
	
	// build cmd
	r_command_t	cmd(	&r_draw_entity,
				NULL,
				&r_draw_mesh,
				shader,
				
				NULL,
				NULL,
				
				NULL,
				
				0);
	
	RB_EnableShader_generic();
	RB_RenderCommand(&cmd, RENDER_TYPE_GENERIC);
	RB_DisableShader_generic();
}

void 	R_DrawStretchPicExp(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, int shadernum)
{
	R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, color, R_GetShaderByNum(shadernum));
}


void 	R_DrawPic(int x, int y, int w, int h, const vec4_c &color, r_shader_c *shader)
{
	R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, color, shader);
}

void 	R_DrawPicExp(int x, int y, int w, int h, const vec4_c &color, int shadernum)
{
	R_DrawPic(x, y, w, h, color, R_GetShaderByNum(shadernum));
}


void 	R_DrawFill(int x, int y, int w, int h, const vec4_c &color)
{
	xglColor4f(color[0], color[1], color[2], color[3]);
	
	xglBegin(GL_QUADS);
	xglVertex3f(x, y, 0.0);
	xglVertex3f(x+w, y, 0.0);
	xglVertex3f(x+w, y+h, 0.0);
	xglVertex3f(x, y+h, 0.0);
	xglEnd();
	
	xglColor4fv(color_white);
}
