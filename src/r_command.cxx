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
#include "r_local.h"

// xreal --------------------------------------------------------------------

r_command_t::r_command_t()
{
	_entity		= NULL;
	_entity_model	= NULL;
	_entity_mesh	= NULL;
	_entity_shader	= NULL;
	
	_light		= NULL;
	_light_shader	= NULL;
	_light_indexes	= NULL;
	
	_infokey	= -1;
	
	_distance	= 0;
}

r_command_t::r_command_t(	r_entity_c*		entity,
				r_model_c*		entity_model,
				r_mesh_c*		entity_mesh,
				r_shader_c*		entity_shader,
				r_light_c*		light,
				r_shader_c*		light_shader,
				std::vector<index_t>*	light_indexes,
				int			infokey,
				vec_t			distance)
{
	_entity		= entity;
	_entity_model	= entity_model;
	_entity_mesh	= entity_mesh;
	_entity_shader	= entity_shader;
	
	_light		= light;
	_light_shader	= light_shader;
	_light_indexes	= light_indexes;
	
	_infokey	= infokey;
	
	_distance	= 0;
}

