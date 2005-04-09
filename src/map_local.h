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
#ifndef MAP_LOCAL_H
#define MAP_LOCAL_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
#include <map>
// qrazor-fx ----------------------------------------------------------------
#include "files.h"
// xreal --------------------------------------------------------------------



class face_t;
class portal_t;


#define	MAX_POINTS_ON_WINDING	64

// you can define on_epsilon in the makefile as tighter
#ifndef	ON_EPSILON
#define	ON_EPSILON	0.1
#endif


class winding_c
{
public:
	winding_c(int points);
	winding_c(const vec3_c &normal, vec_t dist);
	~winding_c();
	
	void		calcPlane(cplane_c &plane) const;
	vec_t		calcArea() const;
	void		calcAABB(aabb_c &aabb) const;
	void		calcCenter(vec3_c &center) const;
	bool		isTiny() const;
	
	//! Reverse vertex order
	void		reverse();
	
	//! Splits winding by split plane
	void		clip(const cplane_c &split, winding_c **front, winding_c **back, vec_t epsilon = ON_EPSILON) const;
	//! Returns the fragment of in that is on the front side of the cliping plane.
	winding_c*	chop(const cplane_c &split) const;
	// frees the original if clipped
//	void		chopWindingInPlace(winding_c **w, vec3_t normal, vec_t dist, vec_t epsilon);

	void		removeColinearPoints();
	
	//! Check for errors in this winding and abort if any
	void		check() const;
	
	plane_side_e	onPlaneSide(const cplane_c &p) const	{return onPlaneSide(p._normal, p._dist);}
	plane_side_e	onPlaneSide(const vec3_c &normal, vec_t dist) const;
	
	//! Print to console
	void		toString() const;
	
//	winding_c*	CopyWinding(winding_c *w);

private:
	std::vector<vec3_c>	_p;
};

class map_shader_c
{
public:
	//TODO
};

class map_brush_side_c
{
public:
	inline map_brush_side_c()
	{
		_plane		= NULL;
		
		_tex_mat.identity();
		
		_winding	= NULL;
		
		_shader		= NULL;
		
		_visible	= false;
		_tested		= false;
		_bevel		= false;
		_culled		= false;
	}

	void			setPlane(cplane_c *p)		{_plane = p;}
	const cplane_c*		getPlane() const 		{return _plane;}
	
	void			setShader(const std::string& s)	{/*TODO load shader;*/}

private:
//	vec3_c			_plane_pts[3];
//	cplane_c		_plane_equation;
	cplane_c*		_plane;
	
	matrix_c		_tex_mat;
	
	winding_c*		_winding;
//	side_t*			_original;		// bspbrush_t sides will reference the mapbrush_t sides
	
	map_shader_c*		_shader;
	
	bool			_visible;		// choose visble planes first
	bool			_tested;		// this plane allready checked as a split
	bool			_bevel;			// don't ever use for bsp splitting
	bool			_culled;
};

class map_brush_c
{
public:
	inline map_brush_c(int entity_num)
	{
		_entity_num	= entity_num;
	}
	
	void			addSide(map_brush_side_c* side)	{_sides.push_back(side);}

	int			getEntityNum() const	{return _entity_num;}
	const aabb_c&		getAABB() const		{return _aabb;}
	
private:
	int			_entity_num;
	int			_brush_num;

	int			_contents;
	
	aabb_c			_aabb;

	std::vector<map_brush_side_c*>	_sides;
};

class map_patch_c
{
public:
	//TODO
};


class map_entity_c
{
	friend void	UnparseEntities();
public:
	void		addBrush(map_brush_c* b)	{_brushes.push_back(b);}
	const std::vector<map_brush_c*>&	getBrushes() const	{return _brushes;}

	void 		setKeyValue(const std::string &key, const std::string &value);
	// will return "" if not present
	const char*	getValueForKey(const std::string &key) const;
	vec_t		getFloatForKey(const std::string &key) const;
	void 		getVector3ForKey(const std::string &key, vec3_c &v) const;
	
	void		toString() const;

private:
	vec3_c					_origin;
	
	std::vector<map_brush_c*>		_brushes;
	std::vector<map_patch_c*>		_patches;
	
	std::map<std::string, std::string>	_epairs;

	// only valid for func_areaportals
	int					_areaportalnum;
	int					_portalareas[2];
};


#define	PLANENUM_LEAF		-1

#define	MAXEDGES		20

class face_t
{
public:
	face_t*			next;		// on node

	// the chain of faces off of a node can be merged or split,
	// but each face_t along the way will remain in the chain
	// until the entire tree is freed
	face_t*			merged;	// if set, this face isn't valid anymore
	face_t*			split[2];	// if set, this face isn't valid anymore

	portal_t*		portal;
	int			texinfo;
	int			planenum;
	int			contents;	// faces in different contents can't merge
	int			outputnumber;
	winding_c*		w;
	int			numpoints;
	bool			badstartvert;	// tjunctions cannot be fixed without a midpoint vertex
	int			vertexnums[MAXEDGES];
};



class bspbrush_t
{
public:
	bspbrush_t*		next;
	
	vec3_t			mins, maxs;
	int			side, testside;		// side of node during construction
	map_brush_c*		original;
	
	int			sides_num;
	map_brush_side_c	sides[6];			// variably sized
};



#define	MAX_NODE_BRUSHES	8
class node_t
{
public:
	// both leafs and nodes
	int			planenum;	// -1 = leaf node
	node_t*			parent;
	vec3_t			mins, maxs;	// valid after portalization
	bspbrush_t*		volume;		// one for each leaf/node

	// nodes only
	bool			detail_seperator;	// a detail brush caused the split
	map_brush_side_c*	side;			// the side that created the node
	node_t*			children[2];
	face_t*			faces;

	// leafs only
	bspbrush_t*		brushlist;	// fragments of all brushes in this leaf
	int			contents;	// OR of all brush contents
	int			occupied;	// 1 or greater can reach entity
	map_entity_c*		occupant;	// for leak file testing
	int			cluster;	// for portalfile writing
	int			area;		// for areaportals
	portal_t*		portals;	// also on nodes during construction
};

class portal_t
{
public:
	cplane_c		plane;
	node_t*			onnode;		// NULL = outside box
	node_t*			nodes[2];		// [0] = front side of plane
	portal_t*		next[2];
	winding_c*		winding;

	bool			sidefound;		// false if ->side hasn't been checked
	map_brush_side_c*	side;			// NULL = non-visible
	face_t*			face[2];		// output face in bsp file
};

class tree_t
{
public:
	node_t*			headnode;
	node_t			outside_node;
	vec3_t			mins, maxs;
};


extern std::vector<map_brush_c*>	map_brushes;
extern std::vector<map_brush_side_c*>	map_brushsides;
extern std::vector<cplane_c*>		map_planes;


//
// map_bspfile.cxx
//
extern std::vector<bsp_dentdata_t>	dentdata;

extern std::vector<bsp_dshader_t>	dshaders;

extern std::vector<bsp_dplane_t>	dplanes;

extern std::vector<bsp_dnode_t>		dnodes;

extern std::vector<bsp_dleaf_t>		dleafs;

extern std::vector<bsp_dleafsurface_t>	dleafsurfaces;

extern std::vector<bsp_dleafbrush_t>	dleafbrushes;

extern std::vector<bsp_dmodel_t>	dmodels;

extern std::vector<bsp_dbrush_t>	dbrushes;

extern std::vector<bsp_dbrushside_t>	dbrushsides;

extern std::vector<bsp_dvertex_t>	dvertexes;

extern std::vector<bsp_dindex_t>	dindexes;

extern std::vector<bsp_dsurface_t>	dsurfaces;

extern std::vector<bsp_dvisdata_t>	dvisdata;

//extern	int		dareas_num;
//extern	bsp_darea_t	dareas[MAX_BSP_AREAS];

//extern int			dareaportals_num;
//extern bsp_dareaportal_t	dareaportals[MAX_BSP_AREAPORTALS];




//extern byte			dpop[256];

//void	DecompressVis(byte *in, byte *decompressed);
//int	CompressVis(byte *vis, byte *dest);


void		LoadBSPFile(const std::string &filename);
void		WriteBSPFile(const std::string &filename);
void		PrintBSPFileSizes();

//
// map_entity.cxx
//
extern std::vector<map_entity_c>	entities;

//void		ParseEntities();
void		UnparseEntities();


//
// map_map.cxx
//
void		LoadMapFile(const std::string &filename);





#endif // MAP_LOCAL_H
