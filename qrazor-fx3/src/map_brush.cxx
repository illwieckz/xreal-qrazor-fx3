/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
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
#include "map_local.h"

// xreal --------------------------------------------------------------------


void	map_brushside_c::setShader(const std::string& s)
{
	_shader = Map_FindShader(s);
}


void	map_brushside_c::translate(const vec3_c &v)
{
	_plane->translate(v);
	
	//vec_t newdist = s->getPlane()->_dist - map_planes[s.planenum]._normal.dotProduct(_origin);
	//map_planes[s.planenum]._dist = newdist;
	//s.planenum = FindFloatPlane(map_planes[s.planenum]);
	//s->texinfo = TexinfoForBrushTexture (&mapplanes[s->planenum], &side_brushtextures[s-brushsides], mapent->origin);
}




void	map_brush_c::translate(const vec3_c &v)
{
	for(map_brushside_i i = _sides.begin(); i != _sides.end(); ++i)
	{
		map_brushside_p s = *i;
		
		s->translate(v);
	}
}

bool	map_brush_c::createWindings()
{
	for(map_brushside_i i = _sides.begin(); i != _sides.end(); ++i)
	{
		const cplane_c* p = (*i)->getPlane();
		
		winding_p w = new winding_c(*p);
		
		for(map_brushside_i j = _sides.begin(); j != _sides.end() && w; ++j)
		{
			if(i == j)
				continue;
			
			if((*j)->isBevel())
				continue;
				
			p = (*j)->getPlane();
			
			// flip the plane, because we want to keep the back side
			const cplane_c pneg = -(*p);
			
			if(w)
			{
				if(!w->chopInPlace(pneg, 0)) //CLIP_EPSILON);
				{
					delete w;
					w = NULL;
				}
			}
		}
		
		if(w)
		{
			//w->print();
		}
		else
		{
			Com_DPrintf("map_brush_c::createWindings: killed winding\n");
		}
		
		(*i)->setWinding(w);
	}
	
	return calcAABB();
}

bool	map_brush_c::calcAABB()
{
	_aabb.clear();
	
	for(map_brushside_i i = _sides.begin(); i != _sides.end(); ++i)
	{
		winding_p w = (*i)->getWinding();
		
		if(!w)
			continue;
	
		_aabb.mergeWith(w->calcAABB());
	}
	
	for(int i=0; i<3; i++)
	{
		if(_aabb._mins[i] < MIN_WORLD_COORD || _aabb._maxs[i] > MAX_WORLD_COORD || _aabb._mins[i] >= _aabb._maxs[i] )
			return false;
	}
	
	return true;
}


