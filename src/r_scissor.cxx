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

void	r_scissoriface_a::updateScissor(const matrix_c &modelviewproj, const r_vrect_t &vrect, const cbbox_c &bbox)
{
	vec3_c	v1, v2;

	_coords[0] = 100000000;
	_coords[1] = 100000000;
	_coords[2] =-100000000;
	_coords[3] =-100000000;

	v1.set(bbox._maxs[0], bbox._maxs[1], bbox._maxs[2]);
	v2.set(bbox._mins[0], bbox._maxs[1], bbox._maxs[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	v1.set(bbox._maxs[0], bbox._maxs[1], bbox._maxs[2]);
	v2.set(bbox._maxs[0], bbox._mins[1], bbox._maxs[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	v1.set(bbox._mins[0], bbox._mins[1], bbox._maxs[2]);
	v2.set(bbox._mins[0], bbox._maxs[1], bbox._maxs[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	v1.set(bbox._mins[0], bbox._mins[1], bbox._maxs[2]);
	v2.set(bbox._maxs[0], bbox._mins[1], bbox._maxs[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	//Bottom plane
	v1.set(bbox._maxs[0], bbox._maxs[1], bbox._mins[2]);
	v2.set(bbox._mins[0], bbox._maxs[1], bbox._mins[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	v1.set(bbox._maxs[0], bbox._maxs[1], bbox._mins[2]);
	v2.set(bbox._maxs[0], bbox._mins[1], bbox._mins[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	v1.set(bbox._mins[0], bbox._mins[1], bbox._mins[2]);
	v2.set(bbox._mins[0], bbox._maxs[1], bbox._mins[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	v1.set(bbox._mins[0], bbox._mins[1], bbox._mins[2]);
	v2.set(bbox._maxs[0], bbox._mins[1], bbox._mins[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	//Sides
	v1.set(bbox._mins[0], bbox._maxs[1], bbox._mins[2]);
	v2.set(bbox._mins[0], bbox._maxs[1], bbox._maxs[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	v1.set(bbox._maxs[0], bbox._maxs[1], bbox._mins[2]);
	v2.set(bbox._maxs[0], bbox._maxs[1], bbox._maxs[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	v1.set(bbox._mins[0], bbox._mins[1], bbox._mins[2]);
	v2.set(bbox._mins[0], bbox._mins[1], bbox._maxs[2]);
	addEdge(modelviewproj, vrect, v1, v2);

	v1.set(bbox._maxs[0], bbox._mins[1], bbox._mins[2]);
	v2.set(bbox._maxs[0], bbox._mins[1], bbox._maxs[2]);
	addEdge(modelviewproj, vrect, v1, v2);
}

void	r_scissoriface_a::addVertex(const matrix_c &mvp, const r_vrect_t &vrect, const vec3_c &v1)
{
	vec4_c	point, res;
	float	px, py;

	point[0] = v1[0];
	point[1] = v1[1];
	point[2] = v1[2];
	point[3] = 1.0f;
	
	res = mvp * point;

	px = (res[0]*(1/res[3])+1.0) * 0.5;
	py = (res[1]*(1/res[3])+1.0) * 0.5;

	px = px * vrect.width + vrect.x;
	py = py * vrect.height + vrect.y;

	if(px > _coords[2])	_coords[2] = px;
	if(px < _coords[0])	_coords[0] = px;
	
	if(py > _coords[3])	_coords[3] = py;
	if(py < _coords[1])	_coords[1] = py;
}

void	r_scissoriface_a::addEdge(const matrix_c &mvp, const r_vrect_t &vrect, const vec3_c &v1, const vec3_c &v2)
{
	vec3_c		intersect;
	plane_side_e	side1, side2;

	// check edge to frustrum near plane
	side1 = r_frustum[FRUSTUM_NEAR].onSide(v1);
	side2 = r_frustum[FRUSTUM_NEAR].onSide(v2);

	if(side1 == SIDE_BACK && side2 == SIDE_BACK)
		return; //edge behind near plane

	if(side1 == SIDE_BACK || side2 == SIDE_BACK)
		r_frustum[FRUSTUM_NEAR].intersect(v1, v2, intersect);

	if(side1 == SIDE_BACK)
		addVertex(mvp, vrect, intersect);
	else
		addVertex(mvp, vrect, v1);
		
	if(side2 == SIDE_BACK)
		addVertex(mvp, vrect, intersect);
	else
		addVertex(mvp, vrect, v2);
}

