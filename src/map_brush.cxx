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

namespace map
{

void	brushside_c::setShader(const std::string& s)
{
	_shader = FindShader(s);
}


void	brushside_c::translate(const vec3_c &v)
{
	_plane->translate(v);
	
	//vec_t newdist = s->getPlane()->_dist - map_planes[s.planenum]._normal.dotProduct(_origin);
	//map_planes[s.planenum]._dist = newdist;
	//s.planenum = FindFloatPlane(map_planes[s.planenum]);
	//s->texinfo = TexinfoForBrushTexture (&mapplanes[s->planenum], &side_brushtextures[s-brushsides], mapent->origin);
}




void	brush_c::translate(const vec3_c &v)
{
	for(brushside_i i = _sides.begin(); i != _sides.end(); ++i)
	{
		brushside_p s = *i;
		
		s->translate(v);
	}
}

void	brush_c::calcContents()
{
	for(brushside_ci i = _sides.begin(); i != _sides.end(); ++i)
	{
		const shader_p s = (*i)->getShader();
		
		addContentFlags(s->getContentFlags());
		addCompileFlags(s->getCompileFlags());
	}
	
	// prefer structural shapes over detail shapes if mixed compile flags
	if(hasCompileFlags(C_DETAIL | C_STRUCTURAL))
	{
		delCompileFlags(C_DETAIL);
	}
	
	// default all translucent shapes into detail shapes if not explicit structural
	if(hasCompileFlags(C_TRANSLUCENT) && !hasCompileFlags(C_STRUCTURAL))
	{
		addCompileFlags(C_DETAIL);
	}
	
	if(hasCompileFlags(C_DETAIL))
	{
		_detail = true;
		c_detail++;
	}
	else
	{
		_detail = false;
		c_structural++;
	}
	
	if(hasCompileFlags(C_TRANSLUCENT))
	{
		_opaque = false;
	}
	else
	{
		_opaque = true;
	}
	
	if(hasCompileFlags(C_AREAPORTAL))
	{
		c_areaportals++;
	}
}

bool	brush_c::createWindings()
{
	for(brushside_i i = _sides.begin(); i != _sides.end(); ++i)
	{
		const cplane_c* p = (*i)->getPlane();
		
		winding_p w = new winding_c(*p);
		
		for(brushside_i j = _sides.begin(); j != _sides.end() && w; ++j)
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

bool	brush_c::calcAABB()
{
	_aabb.clear();
	
	for(brushside_i i = _sides.begin(); i != _sides.end(); ++i)
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

} // namespace map
