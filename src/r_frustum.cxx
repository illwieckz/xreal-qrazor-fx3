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



/*
=================
R_CullBBox

Returns true if the bounding box is completely outside the frustum
=================
*/
bool	R_CullBBox(const r_frustum_t frustum, const cbbox_c &bbox, int clipflags)
{
	if(!r_cull->getValue())
		return false;

	for(int i=0; i<FRUSTUM_PLANES; i++)
	{
		if(!(clipflags & (1<<i)))
			continue;
	
		if(frustum[i].onSide(bbox, true) == SIDE_BACK)
			return true;
	}
	
	return false;
}


/*
=================
R_CullBBox

Returns true if the bounding sphere is completely outside the frustum
=================
*/
bool	R_CullBSphere(const r_frustum_t frustum, const vec3_c &center, vec_t radius, int clipflags)
{
	if(!r_cull->getValue())
		return false;

	for(int i=0; i<FRUSTUM_PLANES; i++)
	{
		if(!(clipflags & (1<<i)))
			continue;
			
		if(frustum[i].onSide(center, radius) == SIDE_BACK)
			return true;
	}
	
	return false;
}


/*
=================
R_CullPoint

Returns true if the point is completely outside the frustum
=================
*/
bool	R_CullPoint(const r_frustum_t frustum, const vec3_c &origin, int clipflags)
{
	if(!r_cull->getValue())
		return false;

	for(int i=0; i<FRUSTUM_PLANES; i++)
	{
		if(!(clipflags & (1<<i)))
			continue;
			
		if(frustum[i].onSide(origin) == SIDE_BACK)
			return true;
	}
	
	return false;
}

