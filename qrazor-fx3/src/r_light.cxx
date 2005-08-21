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


r_light_c::r_light_c(const r_entity_t &shared)
{
	_s = shared;
	
	_origin = _s.origin + _s.origin2;
	
	setupTransform();
	setupProjection();
	setupAttenuation();
	setupFrustum();
	
	_needsupdate = true;
}

void 	r_light_c::setupTransform()
{
	_transform.setupTranslation(_s.origin);
	
	_transform.multiplyRotation(_s.quat);
}

void	r_light_c::setupAttenuation()
{		
	_attenuation.setupTranslation(0.5, 0.5, 0.5);			// bias
	_attenuation.multiplyScale(0.5, 0.5, 0.5);			// scale
	_attenuation.multiply(_projection);				// light projection (frustum)
	_attenuation.multiply(_transform.affineInverse());		// light view matrix
}

void	r_light_c::setupProjection()
{
	switch(_s.type)
	{
		case ET_LIGHT_OMNI:
		{
			_projection.setupScale(1.0/_s.radius[0], 1.0/_s.radius[1], 1.0/_s.radius[2]);
			break;
		}
		
		case ET_LIGHT_PROJ:
		{
			vec_t fov_x = 45;
			vec_t fov_y = CalcFOV(fov_x, 100, 100);
		
			double n = r_znear->getValue();
			double f = r_zfar->getValue();
		
			double r = n * tan(fov_x * M_PI / 360.0);
			double l = -r;
			
			double t = n * tan(fov_y * M_PI / 360.0);
			double b = -t;
			
			RB_Frustum(_projection, l, r, b, t, n, f);
			break;
		}
		
		default:
			ri.Com_Error(ERR_DROP, "r_light_c::setupProjection: bad entity type %i", _s.type);
	}
}

void	r_light_c::setupFrustum()
{
	//TODO
}

bool	r_light_c::hasSurface(int areanum, const r_surface_c *surf)
{
	if(areanum < 0 || areanum >= (int)_areasurfaces.size())
	{
		ri.Com_Error(ERR_DROP, "r_light_c::hasSurface: %i areanum\n", areanum);
		return false;
	}

	std::map<const r_surface_c*, std::vector<index_t> >::iterator ir = _areasurfaces[areanum].find(surf);
	
	if(ir != _areasurfaces[areanum].end())
		return true;
	
	return false;
}

void	r_light_c::addSurface(int areanum, const r_surface_c *surf, bool clear)
{
	r_mesh_c* mesh;
	
	if(!(mesh = surf->getMesh()))
		return;
	
	if(mesh->isNotValid())
		return;
		
	if(clear)
	{
		std::map<const r_surface_c*, std::vector<index_t> >::iterator ir = _areasurfaces[areanum].find(surf);
		
		if(ir != _areasurfaces[areanum].end())
			_areasurfaces[areanum].erase(ir);
	}
	
	if(_areasurfaces.empty())
	{
		ri.Com_Error(ERR_DROP, "r_light_c::addSurface: 0 surfaces\n");
		return;
	}
	
	if(areanum < 0 || areanum >= (int)_areasurfaces.size())
	{
		ri.Com_Error(ERR_DROP, "r_light_c::addSurface: %i areanum\n", areanum);
		return;
	}

	std::vector<index_t> indexes;

	for(unsigned int i=0; i<mesh->indexes.size(); i += 3)
	{
		const vec3_c &v0 = mesh->vertexes[mesh->indexes[i+0]];
		const vec3_c &v1 = mesh->vertexes[mesh->indexes[i+1]];
		const vec3_c &v2 = mesh->vertexes[mesh->indexes[i+2]];
	
#if 0
		// triangle visibility test
		cplane_c p;
		p.fromThreePointForm(v0, v1, v2);
		
		if(p.onSide(_s.origin) == SIDE_FRONT)
		{
			indexes.push_back(mesh->indexes[i+0]);
			indexes.push_back(mesh->indexes[i+1]);
			indexes.push_back(mesh->indexes[i+2]);
		}
#elif 0
		
		// triangle vertices interaction test		
		if(_s.radius_bbox.intersect(v0, v1, v2))
		{
			indexes.push_back(mesh->indexes[i+0]);
			indexes.push_back(mesh->indexes[i+1]);
			indexes.push_back(mesh->indexes[i+2]);
		}
#elif 0
		// triangle edges interaction test
		if(	_s.radius_bbox.intersect(v0, v1)	||
			_s.radius_bbox.intersect(v1, v2)	||
			_s.radius_bbox.intersect(v2, v0)
		)
		{
			indexes.push_back(mesh->indexes[i+0]);
			indexes.push_back(mesh->indexes[i+1]);
			indexes.push_back(mesh->indexes[i+2]);
		}
		
#elif 1
		// triangle planes interaction test
		cplane_c p;
		p.fromThreePointForm(v0, v1, v2);
		
		if(p.onSide(_s.radius_bbox) == SIDE_CROSS)
		{
			indexes.push_back(mesh->indexes[i+0]);
			indexes.push_back(mesh->indexes[i+1]);
			indexes.push_back(mesh->indexes[i+2]);
		}
#endif
	}
	
	if(!indexes.empty())
		_areasurfaces[areanum].insert(std::make_pair(surf, indexes));
}


