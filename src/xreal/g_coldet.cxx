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
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_local.h"
#include "g_entity.h"

class g_areanode_c
{
public:
	void	clear()
	{
		axis		= 0;
		dist		= 0;
		
		children[0]	= NULL;
		children[1]	= NULL;
		
		trigger_entities.clear();
		solid_entities.clear();
	}

	int			axis;		// -1 = leaf node
	float			dist;
	g_areanode_c*		children[2];
	
	std::vector<g_entity_c*>	trigger_entities;
	std::vector<g_entity_c*>	solid_entities;
};


#define	AREA_DEPTH	4
#define	AREA_NODES	32

g_areanode_c	g_areanodes[AREA_NODES];
int		g_areanodes_num;


static aabb_c				g_area_bbox;
static std::vector<g_entity_c*>*	g_area_list;
static int				g_area_count;
static int				g_area_maxcount;


/*
===============
G_CreateAreaNode

Builds a uniformly subdivided tree for the given world size
===============
*/
static g_areanode_c*	G_CreateAreaNode_r(int depth, const aabb_c &aabb)
{
	g_areanode_c *anode = &g_areanodes[g_areanodes_num++];
	
	anode->trigger_entities.clear();
	anode->solid_entities.clear();
	
	if(depth == AREA_DEPTH)
	{
		anode->axis = -1;
		anode->children[0] = anode->children[1] = NULL;
		return anode;
	}
	
	vec3_c size = aabb._maxs - aabb._mins;
	if(size[0] > size[1])
		anode->axis = 0;
	else
		anode->axis = 1;
	
	anode->dist = 0.5 * (aabb._maxs[anode->axis] + aabb._mins[anode->axis]);
	
	aabb_c aabb1 = aabb;
	aabb_c aabb2 = aabb;
	
	aabb1._maxs[anode->axis] = aabb2._mins[anode->axis] = anode->dist;
	
	anode->children[0] = G_CreateAreaNode_r(depth+1, aabb2);
	anode->children[1] = G_CreateAreaNode_r(depth+1, aabb1);

	return anode;
}

void	G_ClearWorld(const std::string &map)
{
	trap_Com_Printf("------- G_ClearWorld(%s) -------\n", map.c_str());

	// load map
	unsigned	checksum;
	
#if defined(ODE)
	d_bsp_c* bsp = trap_CM_BeginRegistration(map, false, &checksum, g_ode_space_toplevel->getId());
	if(bsp)
		g_ode_bsp = bsp;
#else
	g_world_cmodel = trap_CM_BeginRegistration(map, false, &checksum);
#endif
	
	// tell client to load map
	trap_SV_SetConfigString(CS_MAPCHECKSUM, va("%i", checksum));
	
	for(int i=0; i<trap_CM_NumModels(); i++)
	{		
		cmodel_c *cmodel = trap_CM_GetModelByNum(i);
		trap_SV_SetConfigString(CS_MODELS+i, cmodel->getName());
	}

	// clear area nodes
	int		i;
	g_areanode_c*	node;
	
	for(i=0, node=g_areanodes; i<g_areanodes_num; i++, node++)
		node->clear();
	
	g_areanodes_num = 0;
	
	// build area nodes
	G_CreateAreaNode_r(0, g_world_cmodel->getAABB());
}


static void	G_UnlinkEntity_r(g_areanode_c *node, g_entity_c *ent)
{
	std::vector<g_entity_c*>::iterator ir;
	 
	ir = std::find(node->trigger_entities.begin(), node->trigger_entities.end(), ent);
	if(ir != node->trigger_entities.end())
		node->trigger_entities.erase(ir);
		
	ir = std::find(node->solid_entities.begin(), node->solid_entities.end(), ent);
	if(ir != node->solid_entities.end())
		node->solid_entities.erase(ir);
	
	if(node->axis == -1)
		return;		// terminal node

	// recurse down both sides
	G_UnlinkEntity_r(node->children[0], ent);
	G_UnlinkEntity_r(node->children[1], ent);
}

void	g_entity_c::unlink()
{
	if(!_r.islinked)
		return;		// not linked in anywhere
	
	G_UnlinkEntity_r(g_areanodes, this);
	
	_r.islinked = false;
	
//	if(_r.islinked)
//		trap_Com_Error(ERR_FATAL, "g_entity_c::unlink: entity still linked");
}

enum
{
	MAX_ENT_LEAFS		= 128,
	MAX_ENT_CLUSTERS	= 16
};

void	g_entity_c::link()
{
	g_areanode_c	*node;
	static std::vector<int>		leafs;
	static std::deque<int>		clusters;
	
	int			area;
	int			topnode;

	if(_r.islinked)
		unlink();	// unlink from old position
		
	if(this == g_world)
		return;		// don't add the world

	if(!_r.inuse)
		return;

	// set the size
	_r.size = _r.bbox._maxs - _r.bbox._mins;
	
	// encode the size into the entity_state for client prediction
	/* FIXME
	if(ent->_r.solid == SOLID_BBOX && !(ent->_r.svflags & SVF_CORPSE))
	{
		// assume that x/y are equal and symetric
		i = (int)(ent->_r.bbox._maxs[0]/8);
		X_clamp(i, 1, 31);
		
		// z is not symetric
		j = (int)((-ent->_r.bbox._mins[2])/8);
		X_clamp(j, 1, 31);
		
		// and z maxs can be negative...
		k = (int)((ent->_r.bbox._maxs[2]+32)/8);
		X_clamp(k, 1, 63);

		ent->_s.solid = (k<<10) | (j<<5) | i;
	}
	else if(ent->_r.solid == SOLID_BSP)
	{
		ent->_s.solid = 31;	// a solid_bbox will never create this value
	}
	else
		ent->_s.solid = 0;
	*/
	
	/*
	if(_r.solid == SOLID_BBOX && !(ent->_r.svflags & SVF_CORPSE))
	{
		ent->_s.vectors[0] = ent->_r.bbox._mins;
		ent->_s.vectors[1] = ent->_r.bbox._maxs;
	}
	else
	{
		ent->_s.vectors[0].clear();
		ent->_s.vectors[1].clear();	// a solid_bbox will never create this value
	}
	*/

	// set the abs box
	if(_r.solid == SOLID_BSP && (_s.quat != quat_identity))
	{	
		// expand for rotation
		vec_t radius = _r.bbox.radius();
		
		for(int i=0; i<3; i++)
		{
			_r.bbox_abs._mins[i] = _s.origin[i] - radius;
			_r.bbox_abs._maxs[i] = _s.origin[i] + radius;
		}
	}
	else
	{	
		// normal
		_r.bbox_abs._mins = _s.origin + _r.bbox._mins;	
		_r.bbox_abs._maxs = _s.origin + _r.bbox._maxs;
	}

	// because movement is clipped an epsilon away from an actual edge,
	// we must fully check even when bounding boxes don't quite touch
	_r.bbox_abs._mins[0] -= 1;
	_r.bbox_abs._mins[1] -= 1;
	_r.bbox_abs._mins[2] -= 1;
	
	_r.bbox_abs._maxs[0] += 1;
	_r.bbox_abs._maxs[1] += 1;
	_r.bbox_abs._maxs[2] += 1;

	// link to PVS leafs
	_r.area = 0;
	_r.area2 = 0;

	//get all leafs, including solids
	topnode = g_world_cmodel->boxLeafnums(_r.bbox_abs, leafs, MAX_ENT_LEAFS);
	
//	trap_Com_DPrintf("g_entity_c::link: touching %i leafs\n", leafs.size());
//	for(uint_t i=0; i<leafs.size(); i++)
//		trap_Com_DPrintf("%i ", i);
//	trap_Com_DPrintf("\n");

	clusters.resize(leafs.size());

	// set areas
	for(uint_t i=0; i<leafs.size(); i++)
	{
		clusters[i] = g_world_cmodel->leafCluster(leafs[i]);
		area = g_world_cmodel->leafArea(leafs[i]);
		
		if(area)
		{
			// doors may legally straggle two areas,
			// but nothing should evern need more than that
			if(_r.area && _r.area != area)
			{
				if(_r.area2 && _r.area2 != area)
					trap_Com_DPrintf("Object touching 3 areas at %s\n", _r.bbox_abs._mins.toString());
				
				_r.area2 = area;
			}
			else
			{
				_r.area = area;
			}
		}
	}

	if(leafs.size() >= MAX_ENT_LEAFS)
	{	
		// assume we missed some leafs, and mark by headnode
		_r.clusters.clear();
		_r.headnode = topnode;
	}
	else
	{
		//_r.clusters.resize(clusters.size());
		_r.clusters.clear();
		
		for(uint_t i=0; i<leafs.size(); i++)
		{
			if(clusters[i] == -1)
				continue;		// not a visible leaf
			
			uint_t j;
			for(j=0; j<i; j++)
				if(clusters[j] == clusters[i])
					break;
				
			if(j == i)
			{
				if(_r.clusters.size() == MAX_ENT_CLUSTERS)
				{	
					// assume we missed some leafs, and mark by headnode
					_r.clusters.clear();
					_r.headnode = topnode;
					break;
				}

				_r.clusters.push_back(clusters[i]);
			}
		}
	}

	// if first time, make sure old_origin is valid
	if(!_r.linkcount)
	{
		//FIXME
		//_s.origin2 = _s.origin;
	}
	_r.linkcount++;

	if(_r.solid == SOLID_NOT)
		return;

	// find the first node that the ent's box crosses
	node = g_areanodes;
	while(true)
	{
		if(node->axis == -1)
			break;
		
		if(_r.bbox_abs._mins[node->axis] > node->dist)
			node = node->children[0];
		
		else if(_r.bbox_abs._maxs[node->axis] < node->dist)
			node = node->children[1];
		
		else
			break;		// crosses the node
	}
	
	// link it in	
	if(_r.solid == SOLID_TRIGGER)
		node->trigger_entities.push_back(this);
	else
		node->solid_entities.push_back(this);

//	trap_Com_DPrintf("g_entity_c::link: linked entity %i\n", _s.getNumber());
		
	_r.islinked = true;
}


static void	G_AreaEdicts_r(g_areanode_c *node, area_type_e type)
{
	// touch linked edicts
	if(type == AREA_SOLID)
	{
		for(std::vector<g_entity_c*>::iterator ir = node->solid_entities.begin(); ir != node->solid_entities.end(); ++ir)
		{
			g_entity_c* check = *ir;

			if(check->_r.solid == SOLID_NOT)
				continue;		// deactivated
	
			if(!check->_r.bbox_abs.intersect(g_area_bbox))
				continue;		// not touching

			if(g_area_count == g_area_maxcount)
			{
				Com_Printf("G_AreaEdicts_r: MAXCOUNT %i %i\n", g_area_count, node->solid_entities.size());
				return;
			}

			(*g_area_list)[g_area_count++] = check;
		}
	}
	else
	{
		for(std::vector<g_entity_c*>::iterator ir = node->trigger_entities.begin(); ir != node->trigger_entities.end(); ++ir)
		{
			g_entity_c* check = *ir;

			if(check->_r.solid == SOLID_NOT)
				continue;		// deactivated
		
			if(!check->_r.bbox_abs.intersect(g_area_bbox))
				continue;		// not touching

			if(g_area_count == g_area_maxcount)
			{
				Com_Printf("G_AreaEdicts_r: MAXCOUNT %i %i\n", g_area_count, node->trigger_entities.size());
				return;
			}

			(*g_area_list)[g_area_count++] = check;
		}
	}

	if(node->axis == -1)
		return;		// terminal node

	// recurse down both sides
	if(g_area_bbox._maxs[node->axis] > node->dist)
		G_AreaEdicts_r(node->children[0], type);
	
	if(g_area_bbox._mins[node->axis] < node->dist)
		G_AreaEdicts_r(node->children[1], type);
}

int	G_AreaEdicts(const aabb_c &bbox, std::vector<g_entity_c*> &list, area_type_e type)
{
	g_area_bbox = bbox;
	g_area_list = &list;
	g_area_count = 0;
	g_area_maxcount = list.size();

	G_AreaEdicts_r(g_areanodes, type);

	return g_area_count;
}

void	G_SetAreaPortalState(g_entity_c *ent, bool open)
{
	//FIXME
#if 0
	std::vector<int>	leafs(MAX_TOTAL_ENT_LEAFS);
	int	areaportal = 0;
	
	// entity must touch at least two areas
	if(!ent->_r.area || !ent->_r.area2)
		return;
	
	// get all leafs, including solids
	CM_BoxLeafnums(ent->_r.bbox_abs, leafs, 0);
	
	if(leafs.empty())
		return;
	
	for(int i=0; i<(int)leafs.size(); i++)
	{
		if(!CM_LeafCluster(leafs[i]))
			continue;
		
		if(CM_LeafContents(leafs[i]) & X_CONT_AREAPORTAL)
		{
			areaportal = leafs[i];
			break;
		}
	}
	
	if(!areaportal)
		return;
	
	CM_SetAreaPortalState(ent->_r.area, ent->_r.area2, open);
#endif
}

/*
void	G_SetAreaPortalState(g_entity_c *ent, bool open)
{
	int	leafs[MAX_TOTAL_ENT_LEAFS];
	int	leafs_num;
	int	i;
	int	areaportal = 0;
	
	// entity must touch at least two areas
	if(!ent->_r.area || !ent->_r.area2)
		return;
	
	// get all leafs, including solids
	leafs_num = CM_BoxLeafnums(ent->_r.bbox, leafs, MAX_TOTAL_ENT_LEAFS, NULL);
	
	if(!leafs_num)
		return;
	
	for(i=0; i<leafs_num; i++)
	{
		if(!CM_LeafCluster(leafs[i]))
			continue;
		
		if(CM_LeafContents(leafs[i]) & CONTENTS_AREAPORTAL)
		{
			areaportal = leafs[i];
			break;
		}
	}
	
	if(!areaportal)
		return;
	

	CM_SetAreaPortalState(ent->_r.portal, open);
}
*/

int	G_PointContents(const vec3_c &p)
{
#if 0
	static std::vector<g_entity_c*>	touch(MAX_ENTITIES);
	g_entity_c*		hit = NULL;
	int			contents, c2=0;
	int			headnode;
	
	aabb_c		bbox;
	
	bbox._mins = p;
	bbox._maxs = p;

	// get base contents from world
	contents = trap_CM_PointContents(p, 0);

	// or in contents from all the other entities
	int num = G_AreaEdicts(bbox, touch, AREA_SOLID);

	for(int i=0; i<num; i++)
	{
		hit = touch[i];

		// might intersect, so do an exact clip
		headnode = G_HullForEntity(hit);

		if(hit->_r.solid != SOLID_BSP)
			hit->_s.quat.identity();	// boxes don't rotate
	
		c2 = trap_CM_TransformedPointContents(p, headnode, hit->_s.origin, hit->_s.quat);

		contents |= c2;
	}

	return contents;
#else
	return g_world_cmodel->pointContents(p);
#endif
}

class g_clip_c
{
public:
	g_clip_c();
	void		traceBounds();
	void		moveToEntities();

	aabb_c		_bbox_abs;	// enclose the test object along entire move
	aabb_c		_bbox;		// size of the moving object
	
	vec3_c		_start;
	vec3_c		_end;
	trace_t		_trace;
	g_entity_c*	_passedict;
	int		_contentmask;
};

g_clip_c::g_clip_c()
{
	_passedict = NULL;
	_contentmask = X_CONT_NONE;
}


void	g_clip_c::traceBounds()
{
	for(int i=0; i<3; i++)
	{
		if(_end[i] > _start[i])
		{
			_bbox_abs._mins[i] = _start[i] + _bbox._mins[i] - 1;
			_bbox_abs._maxs[i] = _end[i] + _bbox._maxs[i] + 1;
		}
		else
		{
			_bbox_abs._mins[i] = _end[i] + _bbox._mins[i] - 1;
			_bbox_abs._maxs[i] = _start[i] + _bbox._maxs[i] + 1;
		}
	}
}

/*
================
G_HullForEntity

Returns a headnode that can be used for testing or clipping an
object of mins/maxs size.
Offset is filled in to contain the adjustment that must be added to the
testing object's origin to get a point to use with the returned hull.
================
*/
cmodel_c*	G_HullForEntity(g_entity_c *ent)
{
	// decide which clipping hull to use, based on the size
	if(ent->_r.solid == SOLID_BSP)
	{	
		// explicit hulls in the BSP model
		cmodel_c* cmodel = trap_CM_RegisterModel(ent->_model);

		if(!cmodel)
			Com_Error(ERR_FATAL, "G_HullForEntity: MOVETYPE_PUSH with a non bsp model");

		return cmodel;
	}
	
	// create a temp hull from bounding box sizes
	return trap_CM_ModelForBox(ent->_r.bbox);
}


void	g_clip_c::moveToEntities()
{
	static std::vector<g_entity_c*>	touchlist(MAX_ENTITIES);
	g_entity_c*		touch;
	trace_t			trace;

	int num = G_AreaEdicts(_bbox_abs, touchlist, AREA_SOLID);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for(int i=0; i<num; i++)
	{
		touch = touchlist[i];
		
		if(touch->_r.solid == SOLID_NOT)
			continue;
		
		if(touch == _passedict)
			continue;
		
		if(_trace.allsolid)
			return;
		
		if(_passedict)
		{
		 	if(touch->_r.owner == _passedict)
				continue;	// don't clip against own missiles
			
			if(_passedict->_r.owner == touch)
				continue;	// don't clip against owner
		}

		if(!(_contentmask & X_CONT_CORPSE) && (touch->_r.svflags & SVF_CORPSE))
			continue;

		// might intersect, so do an exact clip
		//cmodel_c* box = G_HullForEntity(touch);
		
		if(touch->_r.solid != SOLID_BSP)	//FIXME
			touch->_s.quat.identity();	// boxes don't rotate

		trace = g_world_cmodel->traceOBB(_start, _end, _bbox, _contentmask, touch->_s.origin, touch->_s.quat);
		
		if(trace.allsolid || trace.startsolid || trace.fraction < _trace.fraction)
		{
			trace.ent = touch;
			
		 	if(_trace.startsolid)
			{
				_trace = trace;
				_trace.startsolid = true;
			}
			else
			{
				_trace = trace;
			}
		}
		else if(trace.startsolid)
		{
			_trace.startsolid = true;
		}
	}
}

/*
==================
G_Trace

Moves the given mins/maxs volume through the world from start to end.

Passedict and edicts owned by passedict are explicitly not checked.

==================
*/
trace_t	G_Trace(const vec3_c &start, const aabb_c &aabb, const vec3_c &end, g_entity_c *passedict, int contentmask)
{
	g_clip_c clip;

	// clip to world
	clip._trace = g_world_cmodel->traceAABB(start, end, aabb, contentmask);
//	clip._trace.ent = g_world;
//	clip._trace.fraction = 1.0;

//	if(clip._trace.fraction == 0.0)
//		trap_Com_Printf("G_Trace: blocked by world");

//	if(clip._trace.fraction == 1.0)
//		trap_Com_Printf("G_Trace: full move");
	
//	if(clip._trace.fraction == 0)
		return clip._trace;		// blocked by the world

/*
	clip._contentmask = contentmask;
	clip._start = start;
	clip._end = end;
	clip._bbox = aabb;
	clip._passedict = passedict;
	
	// create the bounding box of the entire move
	clip.traceBounds();

	// clip to other solid entities
	clip.moveToEntities();

	return clip._trace;
*/
}
