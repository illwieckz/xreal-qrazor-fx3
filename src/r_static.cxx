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


r_static_model_c::r_static_model_c(const std::string &name, byte *buffer, uint_t buffer_size)
:r_model_c(name, buffer, buffer_size, MOD_STATIC)
{
	_bbox.clear();
}

r_static_model_c::~r_static_model_c()
{
}
	
void	r_static_model_c::addModelToList(r_entity_c *ent)
{
	if(r_frustum.cull(ent->getShared().origin, _bbox.radius()))
	{
		c_entities--;
		return;
	}

	for(unsigned i=0; i<_meshes.size(); i++)
	{
		r_mesh_c*		mesh = _meshes[i];
		r_shader_c*		shader = NULL;
		
		if(ent->getShared().custom_shader != -1)
		{
			shader = R_GetShaderByNum(ent->getShared().custom_shader);
		}
		else if(ent->getShared().custom_skin != -1)
		{
			r_model_skin_c *skin = R_GetSkinByNum(ent->getShared().custom_skin);
			
			shader = skin->getShader(mesh->name);
			
		}
		else if(i >= 0 && i < _shaders.size())
		{
			shader = _shaders[i]->getShader();
		}
		else
		{
			//ri.Com_Error(ERR_DROP, "r_static_model_c::addModelToList: no way to create command");
			continue;
		}
		
		if(!r_showinvisible->getValue() && shader->hasFlags(SHADER_NODRAW))
			continue;
			
		if(r_envmap && shader->hasFlags(SHADER_NOENVMAP))
			continue;
		
		RB_AddCommand(ent, this, mesh, shader, NULL, NULL, -1, r_origin.distance(ent->getShared().origin));
		
		if(r_lighting->getInteger())
		{
			for(std::vector<std::vector<r_light_c> >::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
			{
				std::vector<r_light_c>& lights = *ir;
			
				for(std::vector<r_light_c>::iterator ir = lights.begin(); ir != lights.end(); ++ir)
				{
					r_light_c& light = *ir;
			
					if(!light.isVisible())
						continue;
			
					if(light.getShared().radius_bbox.intersect(ent->getShared().origin, mesh->bbox.radius()))
						RB_AddCommand(ent, this, mesh, shader, &light, NULL, -(i+1), 0);
				
				}
			}
		}
	}
}

void	r_static_model_c::draw(const r_command_t *cmd, r_render_type_e type)
{
	RB_SetupModelviewMatrix(cmd->getEntity()->getTransform());

	RB_RenderCommand(cmd, type);
}

