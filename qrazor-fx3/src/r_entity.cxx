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




r_entity_c::r_entity_c()
:r_animation_iface_a(-1)
{
	_s.clear();
	
	setupTransform();
}

r_entity_c::r_entity_c(const r_entity_t &shared)
:r_animation_iface_a(shared.animation)
{
	_s = shared;
	
	setupTransform();
	
	if(_s.flags & RF_STATIC)
	{
		if(!r_world_tree && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
			ri.Com_Error(ERR_DROP, "r_entity_c::ctor: NULL worldmodel");
	
#if 1
		//_area = r_world_tree->pointInArea(_s.origin);
#else
		r_bsptree_leaf_c* leaf = r_world_tree->pointInLeaf(_s.origin);
		if(leaf)
		{
			_cluster = leaf->cluster;
		}
		else
		{
			_cluster = -1;
		}
		
		
		r_model_c *model = R_GetModelByNum(_s.model);
			
		if(model)
		{
			model->updateBBox(this);
			r_world_tree->boxLeafs(model->getBBox(), _leafs);
			
			if(_leafs.size())
				ri.Com_DPrintf("entity touches %i BSP leaves\n", _leafs.size());
		}
#endif
	}
	else
	{
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
