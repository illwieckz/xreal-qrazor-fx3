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



r_entity_c::r_entity_c() :
r_model_iface_a(-1),
r_animation_iface_a(-1)
{
	_s.clear();
	
	setupTransform();
}

r_entity_c::r_entity_c(const r_entity_t &shared) :
r_model_iface_a(shared.model),
r_animation_iface_a(shared.animation)
{
	_s = shared;
	
	setupTransform();
	
	_subs = std::vector<r_entity_sub_c>(getModel()->getMeshesNum());
	
	//ri.Com_DPrintf("r_entity_c::ctor: %i sub entities\n", _subs.size());
	
	if(isStatic())
	{
		//ri.Com_DPrintf("r_entity_c::ctor: static\n");
	
		if(!r_world_tree && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
			ri.Com_Error(ERR_DROP, "r_entity_c::ctor: NULL worldmodel");
	
		r_bsptree_leaf_c* leaf = r_world_tree->pointInLeaf(_s.origin);
		if(leaf)
		{
			_cluster = leaf->cluster;
		}
		else
		{
			_cluster = -1;
		}
		
		if(getModel())
		{
			_aabb = getModel()->createAABB(this);
			
			//ri.Com_DPrintf("r_entity_c::ctor: aabb %s\n", _aabb.toString());
			
			_aabb.rotate(_s.quat);
			
			_aabb._mins += _s.origin;
			_aabb._maxs += _s.origin;
			
			// run this AABB through the BSP and collect all touched leaves
			r_world_tree->boxLeafs(_aabb, _leafs);
			
			//if(_leafs.size())
			//	ri.Com_DPrintf("entity touches %i BSP leaves\n", _leafs.size());
			
			
			// touch static lights
			int count = 0;
			for(std::vector<r_light_c*>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
			{
				r_light_c* light = *ir;
				
				if(!light)
					continue;
					
				if(!(light->getShared().flags & RF_STATIC))
					continue;
					
				if(!_aabb.intersect(light->getShared().radius_aabb))
					continue;
					
				count += getModel()->precacheLight(this, light);
			}
		
			//ri.Com_DPrintf("entity has %i precached static lights interactions\n", count);
		}
		else
		{
			//ri.Com_DPrintf("r_entity_c::ctor: model not found\n");
		}
	}
	else
	{
		//ri.Com_DPrintf("r_entity_c::ctor: dynamic\n");
		
		if(getModel())
		{
			_aabb = getModel()->createAABB(this);
			
			_aabb.rotate(_s.quat);
			
			_aabb._mins += _s.origin;
			_aabb._maxs += _s.origin;
		}
	
		_cluster = -1;
	}
}

void 	r_entity_c::setupTransform()
{
	_transform.setupTranslation(_s.origin);
	
	_transform.multiplyRotation(_s.quat);
	
	_transform.multiplyScale(_s.scale);
}


// Tr3B - possible easy solution for left handed weapon rendering
void 	r_entity_c::setupTransformLeftHanded()
{
	_transform.setupTranslation(_s.origin);
	
	_transform.multiplyRotation(_s.quat);
	
//	_transform.multiplyScale(_s.scale, -_s.scale, _s.scale);

	_transform.multiplyScale(1.0, -1.0, 1.0);
}



// Tr3B - used for autosprites, deform flare
void	r_entity_c::setupTransformToViewer()
{
	_transform.setupTranslation(_s.origin);
	
	vec3_c		angles;
	vec3_c		dir;
		
	dir = _s.origin - r_origin;
	Vector3_ToAngles(dir, angles);
	
	_transform.multiplyRotation(angles);
	
	_transform.multiplyScale(_s.scale);
}


const r_entity_sub_c&	r_entity_c::getSubEntity(int num) const
{
	//std::assert(num > 0 && < _sub.size());
	
	const r_entity_sub_c* sub = NULL;
	
	try
	{
		 sub = &_subs.at(num);
	}
	catch(...)
	{
		ri.Com_Error(ERR_DROP, "r_entity_c::getSubEntity: exception thrown while retrieving %i", num);
	}
	
	return *sub;
}

void	r_entity_c::addInteractionToSubEntity(int num, r_interaction_c* ia)
{
	r_entity_sub_c* sub = NULL;
	
	try
	{
		 sub = &_subs.at(num);
	}
	catch(...)
	{
		ri.Com_Error(ERR_DROP, "r_entity_c::addInteractionToSubEntity: exception thrown while retrieving %i", num);
	}
	
	sub->addInteraction(ia);
}


void	R_InitEntities()
{
	if(!r_entities.empty())
		R_ShutdownEntities();

	r_entities = std::vector<r_entity_c*>(MAX_ENTITIES, NULL);
}

void	R_ShutdownEntities()
{
	X_purge<std::vector<r_entity_c*> >(r_entities);
	r_entities.clear();
}



int	R_GetNumForEntity(r_entity_c *ent)
{
	if(!ent)
	{
		ri.Com_Error(ERR_DROP, "R_GetNumForEntity: NULL parameter\n");
		return -1;
	}

	for(uint_t i=0; i<r_entities.size(); i++)
	{
		if(ent == r_entities[i])
			return i;
	}
	
	ri.Com_Error(ERR_DROP, "R_GetNumForEntity: bad pointer\n");
	return -1;
}


r_entity_c*	R_GetEntityByNum(int num)
{
	r_entity_c* ent = NULL;
	
	try
	{
		ent = r_entities.at(num);
	}
	catch(...)
	{
		ri.Com_Error(ERR_DROP, "R_GetEntityByNum: bad number %i, have %i entity slots\n", num, r_entities.size());
		//ent = NULL;
	}

	return ent;
}

