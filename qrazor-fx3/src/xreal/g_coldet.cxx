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


static cbbox_c				g_area_bbox;
static std::vector<g_entity_c*>*	g_area_list;
static int				g_area_count;
static int				g_area_maxcount;

static int G_HullForEntity(g_entity_c *ent);


/*
===============
G_CreateAreaNode

Builds a uniformly subdivided tree for the given world size
===============
*/
static g_areanode_c*	G_CreateAreaNode(int depth, const cbbox_c &bbox)
{
	g_areanode_c	*anode;
	vec3_c		size;
	
	cbbox_c	bbox1, bbox2;

	anode = &g_areanodes[g_areanodes_num];
	g_areanodes_num++;
	
	anode->trigger_entities.clear();
	anode->solid_entities.clear();
	
	if(depth == AREA_DEPTH)
	{
		anode->axis = -1;
		anode->children[0] = anode->children[1] = NULL;
		return anode;
	}
	
	size = bbox._maxs - bbox._mins;
	if(size[0] > size[1])
		anode->axis = 0;
	else
		anode->axis = 1;
	
	anode->dist = 0.5 * (bbox._maxs[anode->axis] + bbox._mins[anode->axis]);
	
	bbox1 = bbox;
	bbox2 = bbox;
	
	bbox1._maxs[anode->axis] = bbox2._mins[anode->axis] = anode->dist;
	
	anode->children[0] = G_CreateAreaNode(depth+1, bbox2);
	anode->children[1] = G_CreateAreaNode(depth+1, bbox1);

	return anode;
}

void	G_ClearWorld(const std::string &map)
{
	unsigned	checksum;
	
	d_bsp_c* bsp = trap_CM_BeginRegistration(map, false, &checksum, g_ode_space_toplevel->getId());
	if(bsp)
		g_ode_bsp = bsp;
	
	trap_SV_SetConfigString(CS_MAPCHECKSUM, va("%i", checksum));
	
	for(int i=0; i<trap_CM_NumModels(); i++)
	{		
		cmodel_c *model = trap_CM_GetModelByNum(i);
		
		trap_SV_SetConfigString(CS_MODELS+i, model->getName());
	}

	int		i;
	g_areanode_c*	node;
	
	for(i=0, node=g_areanodes; i<g_areanodes_num; i++, node++)
		node->clear();
	
	g_areanodes_num = 0;
	
	cmodel_c* model = trap_CM_GetModelByNum(0); 
	
	G_CreateAreaNode(0, model->getBBox());
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

void	G_UnlinkEntity(g_entity_c *ent)
{
	if(!ent->_r.islinked)
		return;		// not linked in anywhere
	
	G_UnlinkEntity_r(g_areanodes, ent);
	
	ent->_r.islinked = false;
	
	//if(ent->_r.islinked)
	//	Com_Error(ERR_FATAL, "G_UnlinkEdict: entity still linked");
}

#define MAX_TOTAL_ENT_LEAFS		128
void	G_LinkEntity(g_entity_c *ent)
{
	g_areanode_c	*node;
	static std::deque<int>	leafs;
	static std::deque<int>	clusters;
	
	int			i, j, k=0;
	int			area;
	int			topnode;

	if(ent->_r.islinked)
		G_UnlinkEntity(ent);	// unlink from old position
		
	if(ent == g_world)
		return;		// don't add the world

	if(!ent->_r.inuse)
		return;

	// set the size
	ent->_r.size = ent->_r.bbox._maxs - ent->_r.bbox._mins;
	
	// encode the size into the entity_state for client prediction
	/*
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
	
	if(ent->_r.solid == SOLID_BBOX && !(ent->_r.svflags & SVF_CORPSE))
	{
		ent->_s.vectors[0] = ent->_r.bbox._mins;
		ent->_s.vectors[1] = ent->_r.bbox._maxs;
	}
	else
	{
		ent->_s.vectors[0].clear();
		ent->_s.vectors[1].clear();	// a solid_bbox will never create this value
	}

	// set the abs box
	if(ent->_r.solid == SOLID_BSP && (ent->_s.quat != quat_identity))
	{	
		// expand for rotation
		float		radius;
		
		radius = ent->_r.bbox.radius();
		
		for(i=0; i<3; i++)
		{
			ent->_r.bbox_abs._mins[i] = ent->_s.origin[i] - radius;
			ent->_r.bbox_abs._maxs[i] = ent->_s.origin[i] + radius;
		}
	}
	else
	{	
		// normal
		Vector3_Add(ent->_s.origin, ent->_r.bbox._mins, ent->_r.bbox_abs._mins);	
		Vector3_Add(ent->_s.origin, ent->_r.bbox._maxs, ent->_r.bbox_abs._maxs);
	}

	// because movement is clipped an epsilon away from an actual edge,
	// we must fully check even when bounding boxes don't quite touch
	ent->_r.bbox_abs._mins[0] -= 1;
	ent->_r.bbox_abs._mins[1] -= 1;
	ent->_r.bbox_abs._mins[2] -= 1;
	
	ent->_r.bbox_abs._maxs[0] += 1;
	ent->_r.bbox_abs._maxs[1] += 1;
	ent->_r.bbox_abs._maxs[2] += 1;

	// link to PVS leafs
	ent->_r.area = 0;
	ent->_r.area2 = 0;

	//get all leafs, including solids
	topnode = trap_CM_BoxLeafnums(ent->_r.bbox_abs, leafs, 0);
	
	clusters.resize(leafs.size());

	// set areas
	for(i=0; i<(int)leafs.size(); i++)
	{
		clusters[i] = trap_CM_LeafCluster(leafs[i]);
		area = trap_CM_LeafArea(leafs[i]);
		
		if(area)
		{
			// doors may legally straggle two areas,
			// but nothing should evern need more than that
			if(ent->_r.area && ent->_r.area != area)
			{
				if(ent->_r.area2 && ent->_r.area2 != area)
					trap_Com_DPrintf("Object touching 3 areas at %f %f %f\n", ent->_r.bbox_abs._mins[0], ent->_r.bbox_abs._mins[1], ent->_r.bbox_abs._mins[2]);
				
				ent->_r.area2 = area;
			}
			else
				ent->_r.area = area;
		}
	}

	if(leafs.size() >= MAX_TOTAL_ENT_LEAFS)
	{	
		// assume we missed some leafs, and mark by headnode
		ent->_r.clusters.clear();
		
		ent->_r.headnode = topnode;
	}
	else
	{
		ent->_r.clusters.resize(clusters.size());
		
		for(i=0, k; i<(int)leafs.size(); i++)
		{
			if(clusters[i] == -1)
				continue;		// not a visible leaf
			
			for(j=0; j<i; j++)
				if(clusters[j] == clusters[i])
					break;
				
			if(j == i)
			{
				/*
				if(ent->_r.clusters_num == MAX_ENT_CLUSTERS)
				{	
					// assume we missed some leafs, and mark by headnode
					ent->_r.clusters_num = -1;
					ent->_r.headnode = topnode;
					break;
				}
				*/

				ent->_r.clusters[k++] = clusters[i];
			}
		}
	}

	// if first time, make sure old_origin is valid
	if(!ent->_r.linkcount)
	{
		//FIXME
		ent->_s.origin2 = ent->_s.origin;
	}
	ent->_r.linkcount++;

	if(ent->_r.solid == SOLID_NOT)
		return;

	// find the first node that the ent's box crosses
	node = g_areanodes;
	while(1)
	{
		if(node->axis == -1)
			break;
		
		if(ent->_r.bbox_abs._mins[node->axis] > node->dist)
			node = node->children[0];
		
		else if(ent->_r.bbox_abs._maxs[node->axis] < node->dist)
			node = node->children[1];
		
		else
			break;		// crosses the node
	}
	
	// link it in	
	if(ent->_r.solid == SOLID_TRIGGER)
		node->trigger_entities.push_back(ent);
	else
		node->solid_entities.push_back(ent);
		
	ent->_r.islinked = true;

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

int	G_AreaEdicts(const cbbox_c &bbox, std::vector<g_entity_c*> &list, area_type_e type)
{
	g_area_bbox = bbox;
	g_area_list = &list;
	g_area_count = 0;
	g_area_maxcount = list.size();

	G_AreaEdicts_r(g_areanodes, type);

	return g_area_count;
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
	static std::vector<g_entity_c*>	touch(MAX_ENTITIES);
	g_entity_c*		hit = NULL;
	int			contents, c2=0;
	int			headnode;
	
	cbbox_c		bbox;
	
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
}

struct moveclip_t
{
	cbbox_c		bbox_abs;	// enclose the test object along entire move
	
	cbbox_c		bbox;		// size of the moving object
	
	vec3_c		start, end;
	trace_t		trace;
	g_entity_c*	passedict;
	int		contentmask;
};



/*
================
G_HullForEntity

Returns a headnode that can be used for testing or clipping an
object of mins/maxs size.
Offset is filled in to contain the adjustment that must be added to the
testing object's origin to get a point to use with the returned hull.
================
*/
int	G_HullForEntity(g_entity_c *ent)
{
	// decide which clipping hull to use, based on the size
	if(ent->_r.solid == SOLID_BSP)
	{	
		// explicit hulls in the BSP model
		cmodel_c* model = trap_CM_RegisterModel(ent->_model);

		if(!model)
			Com_Error(ERR_FATAL, "G_HullForEntity: MOVETYPE_PUSH with a non bsp model");

		return model->_headnode;
	}
	
	// create a temp hull from bounding box sizes
	return trap_CM_HeadnodeForBox(ent->_r.bbox);
}



void	G_ClipMoveToEntities(moveclip_t *clip)
{
	static std::vector<g_entity_c*>	touchlist(MAX_ENTITIES);
	g_entity_c*		touch;
	trace_t			trace;
	int			headnode;

	int num = G_AreaEdicts(clip->bbox_abs, touchlist, AREA_SOLID);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for(int i=0;i<num; i++)
	{
		touch = touchlist[i];
		
		if(touch->_r.solid == SOLID_NOT)
			continue;
		
		if(touch == clip->passedict)
			continue;
		
		if(clip->trace.allsolid)
			return;
		
		if(clip->passedict)
		{
		 	if(touch->_r.owner == clip->passedict)
				continue;	// don't clip against own missiles
			
			if(clip->passedict->_r.owner == touch)
				continue;	// don't clip against owner
		}

		if(!(clip->contentmask & X_CONT_CORPSE) && (touch->_r.svflags & SVF_CORPSE))
				continue;

		// might intersect, so do an exact clip
		headnode = G_HullForEntity(touch);
		
		if(touch->_r.solid != SOLID_BSP)	//FIXME
			touch->_s.quat.identity();	// boxes don't rotate

		trace = trap_CM_TransformedBoxTrace(clip->start, clip->end, clip->bbox, headnode,  clip->contentmask, touch->_s.origin, touch->_s.quat);
		
		if(trace.allsolid || trace.startsolid || trace.fraction < clip->trace.fraction)
		{
			trace.ent = touch;
			
		 	if(clip->trace.startsolid)
			{
				clip->trace = trace;
				clip->trace.startsolid = true;
			}
			else
				clip->trace = trace;
		}
		else if(trace.startsolid)
			clip->trace.startsolid = true;
	}
}


void	G_TraceBounds(const vec3_c &start, const cbbox_c &bbox, const vec3_c &end, cbbox_c &bbox_abs)
{
	for(int i=0; i<3; i++)
	{
		if(end[i] > start[i])
		{
			bbox_abs._mins[i] = start[i] + bbox._mins[i] - 1;
			bbox_abs._maxs[i] = end[i] + bbox._maxs[i] + 1;
		}
		else
		{
			bbox_abs._mins[i] = end[i] + bbox._mins[i] - 1;
			bbox_abs._maxs[i] = start[i] + bbox._maxs[i] + 1;
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
trace_t	G_Trace(const vec3_c &start, const cbbox_c &bbox, const vec3_c &end, g_entity_c *passedict, int contentmask)
{
	moveclip_t	clip;
	
	memset(&clip, 0, sizeof(moveclip_t));

	// clip to world
	clip.trace = trap_CM_BoxTrace(start, end, bbox, 0, contentmask);
	clip.trace.ent = g_world;
	
	//if(clip.trace.fraction == 0)
		return clip.trace;		// blocked by the world

	/*
	clip.contentmask = contentmask;
	clip.start = start;
	clip.end = end;
	clip.bbox = bbox;
	clip.passedict = passedict;
	
	// create the bounding box of the entire move
	G_TraceBounds(start, clip.bbox, end, clip.bbox_abs);

	// clip to other solid entities
	G_ClipMoveToEntities(&clip);

	return clip.trace;
	*/
}
