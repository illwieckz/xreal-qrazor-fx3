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


r_surface_c::r_surface_c()
{
	_framecount	= 0;
	
	_sub		= -1;
	
	_mesh		= NULL;
	_shaderref	= NULL;
	
	_facetype	= BSPST_BAD;
	
	_lightmap	= -1;
}

#if 0
bool	r_surface_c::hasLight(r_light_c *light)
{
	std::map<r_light_c*, std::vector<index_t> >::iterator ir = _interactions.find(light);
	
	if(ir != _interactions.end())
		return true;
	
	/*
	for(std::vector<r_interaction_c>::const_iterator ir = _interactions.begin(); ir != _interactions.end(); ++ir)
	{							
		const r_interaction_c& ia = *ir;
		
		if(ia.getLight() == light)
			return true;
	}
	*/
	
	return false;
}

void	r_surface_c::addLight(r_light_c *light, r_entity_c *ent, bool clear)
{
	if(!_mesh)
		return;
	
	if(_mesh->isNotValid())
		return;
	
	if(clear)
	{
		std::map<r_light_c*, std::vector<index_t> >::iterator ir = _interactions.find(light);
	
		if(ir != _interactions.end())
			_interactions.erase(ir);
	}
	
	const matrix_c& transform = ent->getTransform();

	std::vector<index_t> indexes;

	for(unsigned int i=0; i<_mesh->indexes.size(); i += 3)
	{
		vec3_c v0 = _mesh->vertexes[_mesh->indexes[i+0]];
		vec3_c v1 = _mesh->vertexes[_mesh->indexes[i+1]];
		vec3_c v2 = _mesh->vertexes[_mesh->indexes[i+2]];
		
		if(ent != &r_world_entity)
		{
			v0.transform(transform);
			v1.transform(transform);
			v2.transform(transform);
		}
	
#if 0
		// triangle edges interaction test
		if(	_s.radius_bbox.intersect(v0, v1)	||
			_s.radius_bbox.intersect(v1, v2)	||
			_s.radius_bbox.intersect(v2, v0)
		)
		{
			indexes.push_back(_mesh->indexes[i+0]);
			indexes.push_back(_mesh->indexes[i+1]);
			indexes.push_back(_mesh->indexes[i+2]);
		}
		
#else
		// triangle planes interaction test
		cplane_c p;
		p.fromThreePointForm(v0, v1, v2);
		
		if(p.onSide(light->getShared().radius_bbox) == SIDE_CROSS)
		{
			indexes.push_back(_mesh->indexes[i+0]);
			indexes.push_back(_mesh->indexes[i+1]);
			indexes.push_back(_mesh->indexes[i+2]);
		}
#endif
	}
	
	if(indexes.size())
	{
		_interactions.insert(std::make_pair(light, indexes));
//		_interactions.push_back(r_interaction_c(this, light, indexes));
	}
}
#endif
