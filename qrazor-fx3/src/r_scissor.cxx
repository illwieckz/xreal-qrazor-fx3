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

void	r_scissor_iface_a::updateScissor(const matrix_c &modelviewproj, const r_vrect_t &vrect, const cbbox_c &bbox)
{
	_mins[0] = 100000000;
	_mins[1] = 100000000;
	
	_maxs[0] =-100000000;
	_maxs[1] =-100000000;

#if 1
	vec3_c	v1, v2;
	
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
#else
	vec3_c vertexes[8];	// max x,y,z points in space
	
	vertexes[0].set(bbox._maxs[0], bbox._mins[1], bbox._mins[2]);
	vertexes[1].set(bbox._maxs[0], bbox._mins[1], bbox._maxs[2]);
	vertexes[2].set(bbox._mins[0], bbox._mins[1], bbox._maxs[2]);
	vertexes[3].set(bbox._mins[0], bbox._mins[1], bbox._mins[2]);
	
	vertexes[4].set(bbox._maxs[0], bbox._maxs[1], bbox._mins[2]);
	vertexes[5].set(bbox._maxs[0], bbox._maxs[1], bbox._maxs[2]);
	vertexes[6].set(bbox._mins[0], bbox._maxs[1], bbox._maxs[2]);
	vertexes[7].set(bbox._mins[0], bbox._maxs[1], bbox._mins[2]);
	
	for(int i=0; i<8; i++)
	{
		addVertex(modelviewproj, vrect, vertexes[i]);
	}
#endif
}

void	r_scissor_iface_a::setScissor(const r_vrect_t &vrect)
{
	_mins[0] = vrect.x;
	_mins[1] = vrect.y;
	
	_maxs[0] = vrect.x + vrect.width;
	_maxs[1] = vrect.y + vrect.height;
}

void	r_scissor_iface_a::addVertex(const matrix_c &mvp, const r_vrect_t &vrect, const vec3_c &v1)
{
	vec4_c	point, res;
	float	x, y;

	point[0] = v1[0];
	point[1] = v1[1];
	point[2] = v1[2];
	point[3] = 1.0f;
	
	res = mvp * point;
	
	if(res[3] <= 0.0)
		return;
	
	res[0] /= res[3];
	res[1] /= res[3];
	res[2] /= res[3];

//	x = (res[0]*(1/res[3])+1.0) * 0.5;
//	y = (res[1]*(1/res[3])+1.0) * 0.5;

//	x = x * vrect.width + vrect.x;
//	y = y * vrect.height + vrect.y;

	x = vrect.x + (1.0 + res[0]) * vrect.width * 0.5;
	y = vrect.y + (1.0 - res[1]) * vrect.height * 0.5;	//FIXME?

	if(x > _maxs[0])	_maxs[0] = x;
	if(x < _mins[0])	_mins[0] = x;
		
	if(y > _maxs[1])	_maxs[1] = y;
	if(y < _mins[1])	_mins[1] = y;
}

void	r_scissor_iface_a::addEdge(const matrix_c &mvp, const r_vrect_t &vrect, const vec3_c &v1, const vec3_c &v2)
{
	vec3_c		intersect;
	plane_side_e	side1, side2;

	// check edge against all frustum planes
	for(int i=0; i<FRUSTUM_PLANES; i++)
	{
		side1 = r_frustum[i].onSide(v1);
		side2 = r_frustum[i].onSide(v2);
		
		if(side1 == SIDE_BACK && side2 == SIDE_BACK)
			continue;	//edge behind plane
			
		if(side1 == SIDE_BACK || side2 == SIDE_BACK)
			intersect = r_frustum[i].intersect(v1, v2);
			
		if(side1 == SIDE_BACK)
			addVertex(mvp, vrect, intersect);
		else
			addVertex(mvp, vrect, v1);
			
		if(side2 == SIDE_BACK)
			addVertex(mvp, vrect, intersect);
		else
			addVertex(mvp, vrect, v2);
	}
}

