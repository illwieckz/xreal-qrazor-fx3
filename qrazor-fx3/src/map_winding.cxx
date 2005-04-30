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
// qrazor-fx ----------------------------------------------------------------
#include "map_local.h"

#include "common.h"

// xreal --------------------------------------------------------------------
// counters are only bumped when running single threaded,
// because they are an awefull coherence problem
int	c_active_windings;
int	c_peak_windings;
int	c_winding_allocs;
int	c_winding_points;
int	c_removed;


#define	BOGUS_RANGE	WORLD_SIZE

winding_c::winding_c(int points)
{
	if(points >= MAX_POINTS_ON_WINDING)
		Com_Error(ERR_FATAL, "winding_c::ctor: MAX_POINTS_ON_WINDING exceeded");

	//if(numthreads == 1)
	{
		c_winding_allocs++;
		c_winding_points += points;
		c_active_windings++;
		if(c_active_windings > c_peak_windings)
			c_peak_windings = c_active_windings;
	}
	
	_p = std::vector<vec3_c>(points, vec3_origin);
}

winding_c::winding_c(const vec3_c &normal, vec_t dist)
{
	initFromPlane(normal, dist);
}

winding_c::winding_c(const cplane_c &p)
{
	initFromPlane(p._normal, p._dist);
}

winding_c::~winding_c()
{
	//if(numthreads == 1)
		c_active_windings--;
}

void	winding_c::initFromPlane(const vec3_c &normal, vec_t dist)
{
//	Com_DPrintf("winding_c::initFromPlane: (%i %i %i %i)\n", (int)normal[0], (int)normal[1], (int)normal[2], (int)dist);

	// find the major axis
	vec_t max = -BOGUS_RANGE;
	int x = -1;
	for(int i=0; i<3; i++)
	{
		vec_t v = X_fabs(normal[i]);
		
		if(v > max)
		{
			x = i;
			max = v;
		}
	}
	
	if(x == -1)
		Com_Error(ERR_FATAL, "winding_c::ctor: no axis found");
		
	vec3_c vup;
	switch(x)
	{
		case 0:
		case 1:
			vup[2] = 1.0;
			break;
		case 2:
			vup[0] = 1.0;
			break;
	}

	vec_t dot = vup.dotProduct(normal);
	Vector3_MA(vup, -dot, normal, vup);
	vup.normalize();
		
	vec3_c org = normal * dist;
	
	vec3_c vright;
	vright.crossProduct(vup, normal);
	
	vup *= MAX_WORLD_COORD;
	vright *= MAX_WORLD_COORD;

	// project a really big axis aligned box onto the plane
	_p = std::vector<vec3_c>(4, vec3_origin);
	
	//if(numthreads == 1)
	{
		c_winding_allocs++;
		c_winding_points += 4;
		c_active_windings++;
		if(c_active_windings > c_peak_windings)
			c_peak_windings = c_active_windings;
	}
	
	_p[0] = (org - vright) + vup;
	_p[1] = (org + vright) + vup;
	_p[2] = (org + vright) - vup;
	_p[3] = (org - vright) - vup;
}

cplane_c	winding_c::calcPlane() const
{
	return cplane_c(_p[0], _p[1], _p[2]);
}

vec_t	winding_c::calcArea() const
{
	vec3_c	d1, d2, cross;
	vec_t	total = 0;

	for(uint_t i=2; i<_p.size(); i++)
	{
		d1 = _p[i-1] - _p[0];
		d2 = _p[i] - _p[0];
		cross.crossProduct(d1, d2);
		total += 0.5 * cross.length();
	}
	
	return total;
}

aabb_c	winding_c::calcAABB() const
{
	aabb_c aabb;
	aabb.clear();

	for(uint_t i=0; i<_p.size(); i++)
	{
		aabb.addPoint(_p[i]);
	}
	
	return aabb;
}

vec3_c	winding_c::calcCenter() const
{
	vec3_c center;
	
	for(uint_t i=0; i<_p.size(); i++)
		center += _p[i];

	vec_t scale = X_recip(_p.size());
	center *= scale;
	
	return center;
}

bool	winding_c::isTiny() const
{
	uint_t	i, cnt;
	vec3_c	edge;
	vec_t	len;

#ifdef PARANOID
	if(_p.empty())
		return true;
#endif

	cnt = 0;
	for(i=0; i<_p.size(); i++)
	{
		edge = _p[i] - _p[(i+1) % _p.size()];
		
		len = edge.length();
		if(len > 0.3)
		{
			if(++cnt == 3)
				return false;
		}
	}

	return true;
}


void	winding_c::reverse()
{
	/*
	winding_t *c = AllocWinding(w->numpoints);
	
	for(int i=0; i<w->numpoints; i++)
	{
		Vector3_Copy(w->p[w->numpoints-1-i], c->p[i]);
	}
	c->numpoints = w->numpoints;
	return c;
	*/
	
	std::reverse(_p.begin(), _p.end());
}


void	winding_c::clip(const cplane_c &split, winding_c **front, winding_c **back, vec_t epsilon) const
{
	vec_t		dists[MAX_POINTS_ON_WINDING+4];
	int		sides[MAX_POINTS_ON_WINDING+4];
	int		counts[3];
	vec_t		dot;
	uint_t		i, j;
	vec3_c		mid;
	winding_c	*f, *b;
	uint_t		maxpts;
	
	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	for(i=0; i<_p.size(); i++)
	{
		vec_t dist = dists[i] = split.distance(_p[i]);
		
		if(dist > epsilon)
		{
			sides[i] = SIDE_FRONT;
		}
		else if(dist < -epsilon)
		{
			sides[i] = SIDE_BACK;
		}
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];
	
	*front = *back = NULL;

	if(counts[SIDE_FRONT] <= 0)
	{
		*back = new winding_c(*this);
		return;
	}
	
	if(counts[SIDE_BACK] <= 0)
	{
		*front = new winding_c(*this);
		return;
	}

	maxpts = _p.size() + 4;	// cant use counts[0]+2 because of fp grouping errors

	*front = f = new winding_c();
	*back = b = new winding_c();
		
	for(i=0; i<_p.size(); i++)
	{
		const vec3_c& p1 = _p[i];
		
		if(sides[i] == SIDE_ON)
		{
			f->_p.push_back(p1);
			b->_p.push_back(p1);
			continue;
		}
	
		if(sides[i] == SIDE_FRONT)
		{
			f->_p.push_back(p1);
		}
		
		if(sides[i] == SIDE_BACK)
		{
			b->_p.push_back(p1);
		}

		if(sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
		// generate a split point
		const vec3_c& p2 = _p[(i+1) % _p.size()];
		
		dot = dists[i] / (dists[i] - dists[i+1]);
		for(j=0; j<3; j++)
		{
			// avoid round off error when possible
			if(split._normal[j] == 1)
			{
				mid[j] = split._dist;
			}
			else if(split._normal[j] == -1)
			{
				mid[j] = -split._dist;
			}
			else
			{
				mid[j] = p1[j] + dot*(p2[j] - p1[j]);
			}
		}
			
		f->_p.push_back(mid);
		b->_p.push_back(mid);
	}
	
	if(f->_p.size() > maxpts || b->_p.size() > maxpts)
		Com_Error(ERR_FATAL, "winding_c::clip: points exceeded estimate");
		
	if(f->_p.size() > MAX_POINTS_ON_WINDING || b->_p.size() > MAX_POINTS_ON_WINDING)
		Com_Error(ERR_FATAL, "winding_c::clip: MAX_POINTS_ON_WINDING exceeded");
}

winding_c*	winding_c::chop(const cplane_c &split) const
{
	winding_c	*f = NULL, *b = NULL;

	clip(split, &f, &b);
	if(b)
		delete b;
		
	return f;
}

bool	winding_c::chopInPlace(const cplane_c &split, vec_t epsilon)
{
	vec_t		dists[MAX_POINTS_ON_WINDING+4];
	int		sides[MAX_POINTS_ON_WINDING+4];
	int		counts[3];
	uint_t		i, j;
	vec3_c		mid;
	
//	Com_DPrintf("winding_c::chopInPlace: split plane %s\n", split.toString());
	
	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	for(i=0; i<_p.size(); i++)
	{
		vec_t dist = dists[i] = split._normal.dotProduct(_p[i]) - split._dist;
		
		if(dist > epsilon)
		{
			sides[i] = SIDE_FRONT;
		}
		else if(dist < -epsilon)
		{
			sides[i] = SIDE_BACK;
		}
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if(!counts[SIDE_FRONT])
	{
		return false;
	}
	
	if(!counts[SIDE_BACK])
	{
		return true;
	}

	uint_t maxpts = _p.size() + 4;	// cant use counts[0]+2 because of fp grouping errors
	std::vector<vec3_c> p;
		
	for(i=0; i<_p.size(); i++)
	{
		const vec3_c& p1 = _p[i];
		
		if(sides[i] == SIDE_ON)
		{
			p.push_back(p1);
			continue;
		}
	
		if(sides[i] == SIDE_FRONT)
		{
			p.push_back(p1);
		}

		if(sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
		// generate a split point
		const vec3_c& p2 = _p[(i+1) % _p.size()];
		
		vec_t dot = dists[i] / (dists[i] - dists[i+1]);
		for(j=0; j<3; j++)
		{
			// avoid round off error when possible
			if(split._normal[j] == 1)
			{
				mid[j] = split._dist;
			}
			else if(split._normal[j] == -1)
			{
				mid[j] = -split._dist;
			}
			else
			{
				mid[j] = p1[j] + dot*(p2[j] - p1[j]);
			}
		}
			
		p.push_back(mid);
	}
	
	if(p.size() > maxpts)
		Com_Error(ERR_FATAL, "winding_c::chopInPlace: points exceeded estimate");
		
	if(p.size() > MAX_POINTS_ON_WINDING)
		Com_Error(ERR_FATAL, "winding_c::chopInPlace: MAX_POINTS_ON_WINDING exceeded");
		
	
	_p = p;
	return true;
}


void	winding_c::removeColinearPoints()
{
	vec3_c	v1, v2;
	std::vector<vec3_c> p;
	
	for(uint_t i=0; i<_p.size(); i++)
	{
		uint_t j = (i+1) % _p.size();		// next index
		uint_t k = (i+_p.size()-1) % _p.size();	// previous index
		
		v1 = _p[j] - _p[i];
		v2 = _p[i] - _p[k];
		
		v1.normalize();
		v2.normalize();
		
		if(v1.dotProduct(v2) < 0.999)
		{
			p.push_back(_p[i]);
		}
	}

	if(p.size() == _p.size())
		return;

	//if(numthreads == 1)
		c_removed += _p.size() - p.size();
		
	_p = p;
}

void	winding_c::check() const
{
	uint_t	i, j;
	vec_t	d, edgedist;
	vec3_c	dir, edgenormal;
	vec_t	area;
	
//	vec3_c	facenormal;
//	vec_t	facedist;

	if(_p.size() < 3)
		Com_Error(ERR_FATAL, "winding_c::check: %i points", _p.size());
	
	area = calcArea();
	if(area < 1.0)
		Com_Error(ERR_FATAL, "winding_c::check: %f area", area);

	cplane_c plane = calcPlane();
	
	for(i=0; i<_p.size(); i++)
	{
		const vec3_c& p1 = _p[i];

		for(j=0; j<3; j++)
			if(p1[j] > BOGUS_RANGE || p1[j] < -BOGUS_RANGE)
				Com_Error(ERR_FATAL, "winding_c::check: BUGUS_RANGE: %f", p1[j]);

		j = i+1 == _p.size() ? 0 : i+1;
		
		// check the point is on the face plane
		d = plane._normal.dotProduct(p1) - plane._dist;
		if(d < -ON_EPSILON || d > ON_EPSILON)
			Com_Error(ERR_FATAL, "winding_c::check: point off plane");
	
		// check the edge isnt degenerate
		const vec3_c& p2 = _p[j];
		dir = p2 - p1;
		
		if(dir.length() < ON_EPSILON)
			Com_Error(ERR_FATAL, "winding_c::check: degenerate edge");
			
		edgenormal.crossProduct(plane._normal, dir);
		edgenormal.normalize();
		edgedist = p1.dotProduct(edgenormal);
		edgedist += ON_EPSILON;
		
		// all other points must be on front side
		for(j=0; j<_p.size(); j++)
		{
			if(j == i)
				continue;
				
			d = _p[j].dotProduct(edgenormal);
			
			if(d > edgedist)
				Com_Error(ERR_FATAL, "CheckWinding: non-convex");
		}
	}
}


plane_side_e	winding_c::onPlaneSide(const vec3_c &normal, vec_t dist) const
{
	bool	front = false;
	bool	back = false;

	for(uint_t i=0; i<_p.size(); i++)
	{
		vec_t d = normal.dotProduct(_p[i]) - dist;
		
		if(d < -ON_EPSILON)
		{
			if(front)
				return SIDE_CROSS;
				
			back = true;
		}
		else if(d > ON_EPSILON)
		{
			if(back)
				return SIDE_CROSS;
				
			front = true;
		}
	}

	if(back)
		return SIDE_BACK;
		
	if(front)
		return SIDE_FRONT;
		
	return SIDE_ON;
}


void	winding_c::print() const
{
	Com_Printf("------- winding pts %i -------\n", _p.size());
	for(uint_t i=0 ; i<_p.size(); i++)
		Com_Printf("(%5.1f, %5.1f, %5.1f)\n", _p[i][0], _p[i][1], _p[i][2]);
}


/*
winding_t*	CopyWinding(winding_t *w)
{
	int			size;
	winding_t	*c;

	c = AllocWinding(w->numpoints);
	size = (int)((winding_t *)0)->p[w->numpoints];
	memcpy(c, w, size);
	return c;
}
*/
