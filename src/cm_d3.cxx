/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2002 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2002, 2003  contributors of the XreaL project
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
#include "common.h"
#include "cvar.h"
#include "vfs.h"
#include "files.h"

#include "cm.h"
#include "cm_md3.h"



#define	MAX_CM_AREAS		(MAX_BSP_AREAS)
#define	MAX_CM_LEAFS		(MAX_BSP_LEAFS)
#define	MAX_CM_VISIBILITY	(MAX_BSP_VISIBILITY)


struct cedge_t
{
	int			v[2];
	int			unknown[2];
};

struct cnode_t
{
	cplane_c		plane;
	int			children[2];		// negative numbers are areas
};

struct cpolygon_t
{
	std::vector<int>	edges;			// surface egdes
	cplane_c		plane;
	vec3_c			u;
	vec3_c			v;
	std::string		shader_name;
};

struct cleaf_t
{
	int			contents;
	int			cluster;
	int			area;
	
	int			leafsurfaces_first;
	int			leafsurfaces_num;
	
	int			leafbrushes_first;
	int			leafbrushes_num;
	
	int			leafpatches_first;
	int			leafpatches_num;
};

struct cmesh_t
{
	//std::vector<vec3_c*>	vertexes;
	int			vertexes_num;
	vec3_c*			vertexes;
	
	int			normals_num;
	vec3_c*			normals;
	
	//std::vector<index_t*>	indexes;
	int			indexes_num;
	index_t*		indexes;
};

struct csurface_t
{
	int			face_type;
	
	int			vertexes_first;
	int			vertexes_num;
	
	int			shader_num;
	
	cplane_c		plane;			// BSPST_PLANAR only
	
	int			mesh_cp[2];		// BSPST_BEZIER only
	
	cmesh_t*		mesh;
	
	int			checkcount;
};


struct cbrushside_t
{
	cplane_c		plane;
	//cshader_t*		shader;
};

struct cbrush_t
{
	int			contents;
		
	std::vector<cbrushside_t>	sides;
	cbbox_c			bbox;
	int			unknown;
	
	int			checkcount;		// to avoid repeated testings
};

/*
struct cpatch_t
{
	cbbox_c			bbox_abs;
	
	cbrush_t*		brushes;
	
	cshader_t*		shader;
	
	int			checkcount;
};
*/

struct careaportal_t
{
	bool			open;

	int			areas[2];
	
	cbbox_c			bbox;
	
	std::vector<vec3_c>	points;
};

struct carea_t
{
	//int			floodvalid;

	std::vector<careaportal_t*>	areaportals;
};


struct cmodel_extended_t : public cmodel_c
{
	cmodel_extended_t(const std::string &name)
	:cmodel_c(name, NULL, 0)
	{
	}

	//std::vector<vec3_c>	vertexes;
	
	std::vector<cedge_t>	edges;
	std::vector<cnode_t>	nodes;
	std::vector<cpolygon_t>	polygons;
	std::vector<cbrush_t>	brushes;
};




static std::string			cm_name;

std::vector<cmodel_c*>		cm_models;

static std::string			cm_entitystring;

static std::vector<cnode_t>		cm_nodes;

static std::vector<carea_t>		cm_areas;

static std::vector<careaportal_t*>	cm_areaportals;

//static int				cm_visibility_num = 0;
//static byte				cm_visibility[MAX_CM_VISIBILITY];
//static bsp_dvis_t*			cm_pvs = (bsp_dvis_t *)cm_visibility;

static byte				cm_nullcluster[MAX_CM_LEAFS/8];

//static int				cm_floodvalid = 0;

int		cm_pointcontents;
int		cm_traces;
int		cm_brush_traces;
int		cm_mesh_traces;
int		cm_surf_traces;


static cvar_t*	cm_noareas;


/*
static int		leaf_count;
static int		leaf_maxcount;
static int*		leaf_list;
static cbbox_c		leaf_bbox;
static int		leaf_topnode;
*/

// box tracing
// 1/32 epsilon to keep floating point happy
//#define	DIST_EPSILON	(0.03125)

static vec3_c		trace_start;		// replace this by a ray
static vec3_c		trace_end;

static cbbox_c		trace_bbox;
static cbbox_c		trace_bbox_abs;

static vec3_c		trace_extents;

static trace_t		trace_trace;
//static int		trace_contents;


static void	CM_FloodAreaConnections();




/*
================================================================================
				.CM SPECIFIC
================================================================================
*/


static void	CM_LoadVertexes(cmodel_extended_t *model, char **buf_p)
{
	char *		token;
	int		count;
	
	token = Com_Parse(buf_p);
	if(std::string(token) != "vertices")
		Com_Error(ERR_DROP, "CM_LoadVertexes: model '%s' has bad vertices entry '%s'", model->getName(), token);
		
	Com_Parse(buf_p);	// skip '{'
	
	token = Com_Parse(buf_p);
	count = atoi(token);
	//if(count < 1)
	///	Com_Error(ERR_DROP, "CM_LoadVertexes: CM with no vertexes");
				
	Com_DPrintf("loading vertexes ... ");
	
	for(int i=0; i<count; i++)
	{
		vec3_c	v;
		
		Com_Parse(buf_p);	// skip '('
		
		v[0] = Com_ParseFloat(buf_p);
		v[1] = Com_ParseFloat(buf_p);
		v[2] = Com_ParseFloat(buf_p);
	
		Com_Parse(buf_p);	// skip ')'
		
		model->vertexes.push_back(v);
		
		//Com_Printf("CM_LoadVertexes: %s\n", v.toString());
	}
	
	Com_Parse(buf_p);	// skip '}'
	
	Com_Printf("%i\n", model->vertexes.size());
}



static void	CM_LoadEdges(cmodel_extended_t *model, char **buf_p)
{
	char *		token;
	int		count;
	
	token = Com_Parse(buf_p);
	if(std::string(token) != "edges")
		Com_Error(ERR_DROP, "CM_LoadEdges: model '%s' has bad edges entry '%s'", model->getName(), token);
		
	Com_Parse(buf_p);	// skip '{'
	
	token = Com_Parse(buf_p);
	count = atoi(token);
	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadEdges: CM with no edges");
				
	Com_DPrintf("loading edges ... ");
	
	for(int i=0; i<count; i++)
	{
		cedge_t	edge;
		
		Com_Parse(buf_p);	// skip '('
		
		edge.v[0] = Com_ParseInt(buf_p);
		edge.v[1] = Com_ParseInt(buf_p);
	
		Com_Parse(buf_p);	// skip ')'
		
		edge.unknown[0] = Com_ParseInt(buf_p);
		edge.unknown[1] = Com_ParseInt(buf_p);
		
		//Com_Printf("edge: %i %i %i %i\n", edge.v[0], edge.v[1], edge.unknown[0], edge.unknown[1]);
		
		model->edges.push_back(edge);
		
		
	}
	
	//Com_Printf("CM_LoadEdges: %s\n", model->edges[count-1].v.toString());
	
	Com_Parse(buf_p);	// skip '}'
	
	Com_Printf("%i\n", model->edges.size());
}


static void	CM_LoadNodes(cmodel_extended_t *model, char **buf_p)
{
	char *		token;
	
	token = Com_Parse(buf_p);
	if(std::string(token) != "nodes")
		Com_Error(ERR_DROP, "CM_LoadNodes: model '%s' has bad nodes entry '%s'", model->getName(), token);
		
	Com_Parse(buf_p);	// skip '{'
	
	Com_DPrintf("loading nodes ... ");
	
	while(true)//token && token[0] != '}')
	{
		token = Com_Parse(buf_p);
		
		if(token[0] == '}')
			break;
		
		else if(token[0] == '(')
		{
			cnode_t	n;
					
			n.children[0] = Com_ParseInt(buf_p);
			n.children[1] = Com_ParseInt(buf_p);
			
			Com_Parse(buf_p);	// skip ')'
		
			model->nodes.push_back(n);
		}
	}
	
	Com_Printf("%i\n", model->nodes.size());
}


static void	CM_LoadPolygons(cmodel_extended_t *model, char **buf_p)
{
	char *		token;
	
	token = Com_Parse(buf_p);
	if(std::string(token) != "polygons")
		Com_Error(ERR_DROP, "CM_LoadPolygons: model '%s' has bad nodes entry '%s'", model->getName(), token);
		
	Com_Parse(buf_p);	// skip '{'
	
	Com_DPrintf("loading polygons ... ");
	
	while(true)
	{
		token = Com_Parse(buf_p);
		
		if(token[0] == '}')
			break;
		
		else if(isdigit(token[0]))
		{
			cpolygon_t	p;
			
			// parse edges
			int edges_num = atoi(token);

			Com_Parse(buf_p);	// skip '('
			
			for(int i=0; i<edges_num; i++)
			{
				p.edges.push_back(Com_ParseInt(buf_p));
			}
			
			//Com_Parse(buf_p);	// skip ')'
			//Com_Printf("%i %i %i %i\n", p.edges[0], p.edges[1], p.edges[2], p.edges[3]);
			
			
			// parse plane equation
			Com_Parse(buf_p);	// skip '('
			
			vec3_c normal;
			for(int i=0; i<3; i++)
			{
				normal[i] = Com_ParseFloat(buf_p);
			}						
			
			Com_Parse(buf_p);	// skip ')'
			
			vec_t dist = Com_ParseFloat(buf_p);
			
			p.plane.set(normal, dist);
			
			
			// parse u vector
			Com_Parse(buf_p);	// skip '('
			
			for(int i=0; i<3; i++)
			{
				p.u[i] = Com_ParseFloat(buf_p);
			}
			
			Com_Parse(buf_p);	// skip ')'
			//Com_Printf("CM_LoadPolygons: %s\n", p.u.toString());
			
			
			// parse v vector
			Com_Parse(buf_p);	// skip '('
			
			for(int i=0; i<3; i++)
			{
				p.v[i] = Com_ParseFloat(buf_p);
			}
			
			Com_Parse(buf_p);	// skip ')'
			//Com_Printf("CM_LoadPolygons: %s\n", p.v.toString());
			
			
			// parse shader name
			p.shader_name = Com_Parse(buf_p);
		
		
			model->polygons.push_back(p);
		}
	}
	
	Com_Printf("%i\n", model->polygons.size());
}


static void	CM_LoadBrushes(cmodel_extended_t *model, char **buf_p)
{
	char *		token;
	
	token = Com_Parse(buf_p);
	if(std::string(token) != "brushes")
		Com_Error(ERR_DROP, "CM_LoadBrushes: model '%s' has bad brushes entry '%s'", model->getName(), token);
		
	Com_Parse(buf_p);	// skip '{'
	
	Com_DPrintf("loading brushes ... ");
	
	while(true)
	{
		token = Com_Parse(buf_p);
		
		if(token[0] == '}')
			break;
		
		else if(isdigit(token[0]))
		{
			cbrush_t	b;
			
			// parse sides
			int sides_num = atoi(token);

			Com_Parse(buf_p);	// skip '{'
			
			for(int i=0; i<sides_num; i++)
			{
				cbrushside_t s;
			
				// parse plane equation
				Com_Parse(buf_p, true);		// skip '('
			
				vec3_c normal;
				for(int j=0; j<3; j++)
				{
					normal[j] = Com_ParseFloat(buf_p, false);
				}
					
				Com_Parse(buf_p, false);	// skip ')'
				
				vec_t dist = Com_ParseFloat(buf_p, false);
				
				s.plane.set(normal, dist);
				
				b.sides.push_back(s);
			}
			
			Com_Parse(buf_p, true);	// skip '}'
			
			// parse bbox
			b.bbox._mins = Com_ParseVec3(buf_p);
			b.bbox._maxs = Com_ParseVec3(buf_p);
			
			// parse misc
			b.unknown = Com_ParseInt(buf_p);
		
			model->brushes.push_back(b);
		}
	}
	
	Com_Printf("%i\n", model->brushes.size());
}


static void	CM_CreateIndexes(cmodel_extended_t *model)
{
	for(std::vector<cpolygon_t>::iterator ir = model->polygons.begin(); ir != model->polygons.end(); ir++)
	{
		cpolygon_t& poly = *ir;
		
		int indexes[3];
		
		if(poly.edges[0] > 0)
		{
			indexes[0] = model->edges[poly.edges[0]].v[0];
			indexes[1] = model->edges[poly.edges[0]].v[1];
		}
		else
		{
			indexes[0] = model->edges[-poly.edges[0]].v[1];
			indexes[1] = model->edges[-poly.edges[0]].v[0];
		}
		
		for(std::vector<int>::iterator ir = (poly.edges.begin() + 1); ir != (poly.edges.end() - 1); ir++)
		{
			int& edge = *ir;
			
			if(edge > 0)
			{
				indexes[2] = model->edges[edge].v[1];
			}
			else
			{
				indexes[2] = model->edges[-edge].v[0];
			}
			
			//Com_Printf("indexes: %i %i %i\n", indexes[0], indexes[1], indexes[2]);
			
			model->indexes.push_back(indexes[0]);
			model->indexes.push_back(indexes[1]);
			model->indexes.push_back(indexes[2]);
			
			indexes[1] = indexes[2];
		}
	}
	
	//reverse(model->indexes.begin(), model->indexes.end());
}



/*
================================================================================
				.PROC SPECIFIC
================================================================================
*/

static void	CM_LoadProcNodes(char **buf_p)
{
	Com_Printf("loading proc nodes ... \n");
	
	Com_Parse(buf_p);	// skip '{'
	
	int count = Com_ParseInt(buf_p);
	
	cm_nodes = std::vector<cnode_t>(count);
	
	for(unsigned int i=0; i<cm_nodes.size(); i++)
	{
		cnode_t& node = cm_nodes[i];
	
		// parse plane equation
		Com_Parse(buf_p, true);		// skip '('
		
		vec3_c normal;
		for(int i=0; i<3; i++)
		{
			normal[i] = Com_ParseFloat(buf_p, false);
		}						
		
		vec_t dist = -Com_ParseFloat(buf_p, false);
			
		Com_Parse(buf_p, false);	// skip ')'
			
		node.plane.set(normal, dist); 
		
		// parse children
		node.children[SIDE_FRONT] = Com_ParseInt(buf_p, false);
		node.children[SIDE_BACK] = Com_ParseInt(buf_p, false);
		
		//Com_Printf("node: %s ", node.plane.toString());
		//Com_Printf("%i %i\n", node.children[0], node.children[1]);
	}
	
	Com_Parse(buf_p, true);	// skip '}'
	
	//Com_Printf("%i\n", cm_nodes.size());
}

static void	CM_LoadProcInterAreaPortals(char **buf_p)
{
	Com_Printf("loading inter area portals ... \n");

	Com_Parse(buf_p, true);	// skip '{'
	
	int areas_num = Com_ParseInt(buf_p, false);
	int areaportals_num = Com_ParseInt(buf_p, false);
	
	cm_areas = std::vector<carea_t>(areas_num);
	
	for(int i=0; i<areaportals_num; i++)
	{
		careaportal_t* ap = new careaportal_t;
		
		int points_num = Com_ParseInt(buf_p, true);
		
		ap->areas[SIDE_FRONT] = Com_ParseInt(buf_p, false);
		ap->areas[SIDE_BACK] = Com_ParseInt(buf_p, false);
		
		cm_areas[ap->areas[SIDE_FRONT]].areaportals.push_back(ap);
		cm_areas[ap->areas[SIDE_BACK]].areaportals.push_back(ap);
		
		ap->bbox.clear();
	
		// parse points
		for(int j=0; j<points_num; j++)
		{
			vec3_c p = Com_ParseVec3(buf_p);
		
			ap->bbox.addPoint(p);
		
			ap->points.push_back(p);
		}
		
		//Com_Printf("iap: %s ", ap..toString());
		//Com_Printf("%i %i %i \n", points_num, ap.areas[SIDE_FRONT], ap.areas[SIDE_BACK]);
		
		cm_areaportals.push_back(ap);
	}
	
	Com_Parse(buf_p, true);	// skip '}'
}

static void	CM_SkipProcSurface(char **buf_p)
{
	while(buf_p)
	{
		char *token = Com_Parse(buf_p);
		
		//if(!token[0])
		//	break;
		
		if(X_strequal(token, "}"))
			break;
	}
}

static void	CM_SkipProcModel(char **buf_p)
{
	Com_Parse(buf_p, false);	// skip '{'
	
	//token = Com_Parse(buf_p, false);
	//Com_Printf("found model '%s'\n", token);
	
	while(buf_p)
	{
		char *token = Com_Parse(buf_p);
		
		//if(!token[0])
		//	break;
		
		if(X_strequal(token, "}"))
			break;
		
		if(X_strequal(token, "{"))
			CM_SkipProcSurface(buf_p);
	}
}

static void 	CM_LoadProc(const std::string &name)
{
	char*			buf;
	char*			buf_p;
	char*			token = NULL;
	int			length;
	std::string		full_name;


	//
	// load the file
	//
	full_name = "maps/" + name + ".proc";
	length = VFS_FLoad(full_name, (void **)&buf);
	if(!buf)
		Com_Error(ERR_DROP, "CM_LoadProc: Couldn't load %s", full_name.c_str());
	
	buf_p = buf;
	
	std::string ident = Com_Parse(&buf_p);
	if(ident != (std::string(PROC_IDENTSTRING + std::string(PROC_VERSION))))
		Com_Error(ERR_DROP, "CM_LoadProc: %s has wrong ident ('%s' should be '%s%s')", full_name.c_str(), ident.c_str(), PROC_IDENTSTRING, PROC_VERSION);


	Com_DPrintf("loading '%s' ...\n", full_name.c_str());
	
	while(buf_p)
	{
		token = Com_Parse(&buf_p, true);
		
		//if(!token[0])
		//	break;
		
		//Com_Printf("token '%s'\n", token);
		
		/*	
		if(!X_stricmp(token, "model"))
		{
			Com_Parse(&buf_p);	// skip '{'
			
			std::string modelname = Com_Parse(&buf_p);
			
			r_proc_model_c *model = new r_proc_model_c(modelname);
		
			model->load(&buf_p);
			
			model->setupMeshes();
			
			//if(X_strnequal(model->getName(), "_area", 5))
			//	r_world_tree->_areamodels.push_back(model);
			//else
			r_models.push_back(model);
		}
		*/
		
		if(X_strequal(token, "model"))
		{			
			CM_SkipProcModel(&buf_p);
			continue;
		}
		else if(X_strequal(token, "interAreaPortals"))
		{
			CM_LoadProcInterAreaPortals(&buf_p);
			continue;
		}
		else if(X_strequal(token, "nodes"))
		{
			CM_LoadProcNodes(&buf_p);
			continue;
		}
		
		//Com_Printf("token '%s'\n", token);
	}
	
	
	VFS_FFree(buf);
}


/*
================================================================================
				.MAP SPECIFIC
================================================================================
*/

static void	CM_SkipPrimitive(char **data_p)
{
	char *token = Com_Parse(data_p);
	
	if(X_strequal(token, "brushDef") ||X_strequal(token, "brushDef3") || X_strequal(token, "patchDef2") || X_strequal(token, "patchDef3"))
	{
		while(true)
		{
			char *token = Com_Parse(data_p);
		
			if(token[0] == '}')
				break;
		}
	}
	else
	{
		Com_Error(ERR_DROP, "CM_SkipPrimitive: unknown primitive '%s'\n", token);
	}
	
	Com_Parse(data_p);	// skip '}'
}

static void	CM_ParseEntity(char **data_p)
{
	cm_entitystring += "{\n";
	
	while(true)
	{
		char *token = Com_Parse(data_p);
		
		if(token[0] == '}')
			break;
			
		if(token[0] == '{')
		{
			CM_SkipPrimitive(data_p);
			continue;
		}
		
		cm_entitystring += '\"';
		cm_entitystring += token;	// key
		cm_entitystring += '\"';
		cm_entitystring += ' ';
		
		token = Com_Parse(data_p, false);
		
		cm_entitystring += '\"';
		cm_entitystring += token;	//value
		cm_entitystring += '\"';
		cm_entitystring += '\n';
	}
	
	cm_entitystring += "}\n";
	
	cm_entitystring = X_strlwr(cm_entitystring);
}

void	CM_LoadMap(const std::string &name)
{
	char*			buf;
	char*			buf_p;
	char*			token = NULL;
	int			length;
	std::string		full_name;


	//
	// free old stuff
	//
	cm_entitystring = "";
	
	if(!name.length())
	{
		Com_Error(ERR_FATAL, "CM_LoadMap: empty name");
		return;			// cinematic servers won't have anything at all
	}

	//
	// load the file
	//
	full_name = "maps/" + name + ".map";
	length = VFS_FLoad(full_name, (void **)&buf);
	if (!buf)
		Com_Error(ERR_DROP, "CM_LoadMap: Couldn't load %s", full_name.c_str());
	
	buf_p = buf;
	
#if 1
	std::string ident = Com_Parse(&buf_p);
	if(ident != MAP_IDENTSTRING)
		Com_Error(ERR_DROP, "CM_LoadMap: %s has wrong ident ('%s' should be '%s')", full_name.c_str(), ident.c_str(), MAP_IDENTSTRING);
	
	
	token = Com_Parse(&buf_p);
	int version = atoi(token);
	if(version != MAP_VERSION)
		Com_Error(ERR_DROP, "CM_LoadMap: %s has wrong version number (%i should be %i)", full_name.c_str(), version, MAP_VERSION);
#endif

	Com_DPrintf("loading '%s' ...\n", full_name.c_str());
	
	//Com_Parse(&buf_p);	// skip unknown
	
	while(true)
	{
		token = Com_Parse(&buf_p);
		
		if(!token || !token[0])
			break;
			
		// parse entity
		if(token[0] == '{')
		{
			CM_ParseEntity(&buf_p);
			continue;
		}
	}
	
	//Com_Printf("%s", cm_entitystring.c_str());
	
	VFS_FFree(buf);
}

const char*	CM_EntityString()
{
	return cm_entitystring.c_str();
}


d_bsp_c*	CM_BeginRegistration(const std::string &name, bool clientload, unsigned *checksum, dSpaceID space)
{
	char*			buf;
	char*			buf_p;
	char*			token = NULL;
	int			length;
	std::string		full_name;
	static unsigned		last_checksum;

	cm_noareas	= Cvar_Get("cm_noareas", "1", CVAR_NONE);

	if(!X_stricmp(cm_name.c_str(), name.c_str()) && (clientload || !Cvar_VariableValue("flushmap")))
	{
		*checksum = last_checksum;
		
		if(!clientload)
		{
			CM_FloodAreaConnections();
		}
		
		return NULL;		// still have the right version
	}

	//
	// free old stuff
	//
	for(std::vector<cmodel_c*>::iterator ir = cm_models.begin(); ir != cm_models.end(); ir++)
	{
		delete *ir;
	}
	cm_models.clear();
	cm_nodes.clear();

	
	//
	// load the file
	//
	full_name = "maps/" + name + ".cm";
	length = VFS_FLoad(full_name, (void **)&buf);
	if (!buf)
		Com_Error(ERR_DROP, "CM_BeginRegistration: Couldn't load %s", full_name.c_str());

	last_checksum = LittleLong(Com_BlockChecksum(buf, length));
	*checksum = last_checksum;
	
	buf_p = buf;
	
	std::string ident = Com_Parse(&buf_p);
	if(ident != CM_IDENTSTRING)
		Com_Error(ERR_DROP, "CM_BeginRegistration: %s has wrong ident ('%s' should be '%s')", full_name.c_str(), ident.c_str(), CM_IDENTSTRING);
	
	
	token = Com_Parse(&buf_p);
	int version = atoi(token);
	if(version != CM_VERSION)
		Com_Error(ERR_DROP, "CM_BeginRegistration: %s has wrong version number (%i should be %i)", full_name.c_str(), version, CM_VERSION);

	Com_DPrintf("loading '%s' ...\n", full_name.c_str());
	
	Com_Parse(&buf_p);	// skip unknown
	
	while(true)
	{
		token = Com_Parse(&buf_p);
		
		if(!token[0])
			break;
			
		if(!X_stricmp(token, "collisionModel"))
		{
			token = Com_Parse(&buf_p);
		
			cmodel_extended_t *model = new cmodel_extended_t(token);
			
			Com_Parse(&buf_p);	// skip '{'
			
			CM_LoadVertexes(model, &buf_p);
			CM_LoadEdges(model, &buf_p);
			CM_LoadNodes(model, &buf_p);
			CM_LoadPolygons(model, &buf_p);
			CM_LoadBrushes(model, &buf_p);
			
			CM_CreateIndexes(model);
			
			token = Com_Parse(&buf_p);
			
			if(token[0] != '}')
			{
				Com_Error(ERR_DROP, "CM_BeginRegistration: model '%s' has bad end %s", model->getName(), token);
				break;
			}
			else
			{
				cm_models.push_back(model);
				continue;
			}
		}
			
		Com_Printf("CM_BeginRegistration: %s\n", token);
	}
	
	VFS_FFree(buf);
	
	CM_LoadProc(name);
	CM_LoadMap(name);

	CM_FloodAreaConnections();
		
	memset(cm_nullcluster, 255, sizeof(cm_nullcluster));
	
	cm_name = name;
	
	return NULL;
}


int	CM_ClusterSize()
{
	return (MAX_CM_LEAFS / 8);
}

int	CM_NumClusters()
{
	return 0;
}



int	CM_NumModels()
{
	return cm_models.size();
}

int	CM_LeafContents(int leafnum)
{
#if 0
	if(leafnum < 0 || leafnum >= cm_leafs_num)
		Com_Error(ERR_DROP, "CM_LeafContents: out of range %i", leafnum);
	
	return cm_leafs[leafnum].contents;
#else
	return 0;
#endif
}

int	CM_LeafCluster(int leafnum)
{
#if 0
	if(leafnum < 0 || leafnum >= cm_leafs_num)
		Com_Error (ERR_DROP, "CM_LeafCluster: out of range %i", leafnum);
	
	return cm_leafs[leafnum].cluster;
#else
	return 0;
#endif
}

int	CM_LeafArea(int leafnum)
{
#if 0
	if(leafnum < 0 || leafnum >= cm_leafs_num)
		Com_Error (ERR_DROP, "CM_LeafArea: out of range %i", leafnum);
	
	return cm_leafs[leafnum].area;
#else
	return 0;
#endif
}


int	CM_HeadnodeForBox(const cbbox_c & bbox)
{
	//TODO
	return cm_nodes.size();
}


static int	CM_PointAreanum_r(const vec3_c &p, int num)
{
	if(num < 0 || num > (int)cm_nodes.size())
	{
		Com_Error(ERR_DROP, "CM_PointAreanum: bad num %i", num);
	}

	cnode_t& node = cm_nodes[num];
		
	vec_t d = node.plane.distance(p);
		
	if(d >= 0)
		num = node.children[SIDE_FRONT];
	else
		num = node.children[SIDE_BACK];

	cm_pointcontents++;		// optimize counter
	
	if(num < 0)
		return -1 -num;
			
	if(num == 0)
		return -1;
	
	return CM_PointAreanum_r(p, num);
}


int	CM_PointAreanum(const vec3_c &p)
{
	if(!cm_nodes.size())
		return 0;		// sound may call this without map loaded
		
	return CM_PointAreanum_r(p, 0);
}

int	CM_BoxLeafnums(const cbbox_c &bbox, std::deque<int> &list, int headnode)
{
	//FIXME
	return 0;
}


int	CM_PointContents(const vec3_c &p, int headnode)
{
#if 0
	int		i, j;
	int		contents;
	
	cleaf_t			*leaf;
	cplane_c		*plane;
	cbrush_t		*brush;
	cbrushside_t	*brushside;

	if(!cm_nodes_num)	// map not loaded
		return 0;

	i = CM_PointLeafnum_r(p, headnode);
	leaf = &cm_leafs[i];
	
	contents = 0;
	
	for(i=0; i<leaf->leafbrushes_num; i++)
	{
		brush = &cm_brushes[cm_leafbrushes[leaf->leafbrushes_first + i]];
		
		for(j=0; j<brush->sides_num; j++)
		{
			brushside = &cm_brushsides[brush->sides_first + j];
			plane = brushside->plane;
			
			if(plane->diff(p) > 0)
				break;
		}
		
		if(j == brush->sides_num)
			contents |= brush->contents;
	}
	
	return contents;
#else
	return 0;
#endif
}

/*
==================
CM_TransformedPointContents

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
int	CM_TransformedPointContents(const vec3_c &p, int headnode, const vec3_c &origin, const quaternion_c &quat)
{
	vec3_c		p_l;
	
	if(!cm_models.size())	//map not loaded
		return 0;
	
	// subtract origin offset
	p_l = p - origin;

	// rotate start and end into the models frame of reference
	/*
	if(headnode != box_headnode && (angles[0] || angles[1] || angles[2]) )
	{
		p_l.rotate(angles);
	}
	*/
	
	return CM_PointContents(p_l, headnode);
}


trace_t	CM_BoxTrace(const vec3_c &start, const vec3_c &end, const cbbox_c &bbox, int headnode, int brushmask)
{
	vec3_c	p;

//	cm_checkcount++;		// for multi-check avoidance

	cm_traces++;			// for statistics, may be zeroed

	//
	// fill in a default trace
	//
	memset (&trace_trace, 0, sizeof(trace_trace));
	trace_trace.fraction = 1;
	trace_trace.surface = NULL;//&cm_nullshader;

	if(cm_nodes.empty())	// map not loaded
		return trace_trace;

//	trace_contents = brushmask;
	trace_start = start;
	trace_end = end;
	
	trace_bbox = bbox;
	
	//
	// build a bounding box of the entire move
	//
	trace_bbox_abs.clear();
	
	p = start + trace_bbox._mins;
	trace_bbox_abs.addPoint(p);
	
	p = start + trace_bbox._maxs;
	trace_bbox_abs.addPoint(p);
	
	p = end + trace_bbox._mins;
	trace_bbox_abs.addPoint(p);
	
	p = end + trace_bbox._maxs;
	trace_bbox_abs.addPoint(p);
	
	//
	// check for position test special case
	//
	
	//TODO

	//
	// check for point special case
	//
	
	//TODO

	//
	// general sweeping through world
	//
	/*
	CM_HullCheck_r(headnode, 0, 1, start, end);

	if(trace_trace.fraction == 1)
	{
		trace_trace.pos = end;
	}
	else
	*/
	{
		trace_trace.pos = start + ((end - start) * trace_trace.fraction);
	}
	
	return trace_trace;
}

trace_t	CM_TransformedBoxTrace(const vec3_c &start, const vec3_c &end,
						const cbbox_c &bbox,
						int headnode, int brushmask, 
						const vec3_c &origin, const quaternion_c &quat)
{
	trace_t		trace;
	vec3_c		start_l(false), end_l(false);
	bool	rotated = true;

	// subtract origin offset
	start_l = start - origin;
	end_l = end - origin;

	// rotate start and end into the models frame of reference
//	if(headnode != box_headnode && (quat != quat_identity))
//		rotated = true;
//	else
//		rotated = false;

//	if(rotated)
	{
		start_l.rotate(quat);
		end_l.rotate(quat);
	}

	// sweep the box through the model
	trace = CM_BoxTrace(start_l, end_l, bbox, headnode, brushmask);

	if(rotated && trace.fraction != 1.0)
	{
		// FIXME: figure out how to do this with existing angles
		//a = angles;
		//a.negate();
		//trace.plane.rotate(a);
		quaternion_c q = quat;
		q.inverse();
		trace.plane.rotate(q);
	}
	
	trace.pos = start + ((end - start) * trace_trace.fraction);
	
	return trace;
}


byte*	CM_ClusterPVS(int cluster)
{
	//if(cluster != -1 && cm_pvs->clusters_num)
	//	return (byte*) cm_pvs->visdata + cluster * cm_pvs->cluster_size;
	
	return cm_nullcluster;
}


/*
static void	CM_FloodArea_r(carea_t &area, int floodnum)
{

	if(area.floodvalid == floodvalid)
	{
		if (area->floodnum == floodnum)
			return;
			
		Com_Error(ERR_DROP, "FloodArea_r: reflooded");
	}

	//area->floodnum = floodnum;
	//area->floodvalid = floodvalid;
	careaportal_t *ap = &map_areaportals[area->firstareaportal];
	
	for(int i=0; i<area->numareaportals ; i++, p++)
	{
		if (portalopen[p->portalnum])
			FloodArea_r (&map_areas[p->otherarea], floodnum);
	}
}
*/

static void	CM_FloodAreaConnections()
{
	/*
	// area 0 is not used
	for(unsigned i=1; i<cm_areas.size(); i++)
	{
		for(unsigned int j=1; j<cm_areas.size(); j++)
			cm_areas[i].numareaportals[j] = (j == i);
	}
	*/
	/*
	cm_floodvalid++;
	
	int floodnum = 0;
	
	for(std::vector<carea_t>::iterator ir = cm_areas.begin(); ir != cm_areas.end(); ir++)
	{
		carea_t& area = *ir;
		
		if(area.floodvalid == cm_floodvalid)
			continue;
			
		floodnum++;
		
		CM_FloodArea_r(area, floodnum);
	}
	*/
	
	for(std::vector<careaportal_t*>::iterator ir = cm_areaportals.begin(); ir != cm_areaportals.end(); ir++)
	{
		(*ir)->open = true;
	}
}

int	CM_GetClosestAreaPortal(const vec3_c &p)
{
	int	best_iap = -1;
	vec_t	best_dist = 99999;
	vec_t	dist;

	for(unsigned int i=0; i< cm_areaportals.size(); i++)
	{
		careaportal_t* iap = cm_areaportals[i];
		
		vec3_c v = iap->bbox.origin();
		
		dist = p.distance(v);
		
		if(dist < best_dist && dist < 72)
		{
			best_iap = i;
			best_dist = dist;
		}
	}
	
	return best_iap;
}

bool	CM_GetAreaPortalState(int portal)
{
	if(portal < 0 || portal >= (int)cm_areaportals.size())
	{
		//Com_Error(ERR_DROP, "CM_GetAreaPortalState: bad number %i", portal);
		return true;
	}
	
	return cm_areaportals[portal]->open;
}

void	CM_SetAreaPortalState(int portal, bool open)
{
	if(portal < 0 || portal >= (int)cm_areaportals.size())
	{
		//Com_Error(ERR_DROP, "CM_SetAreaPortalState: bad number %i", portal);
		return;
	}
	
	cm_areaportals[portal]->open = open;
}

bool	CM_AreasConnected(int area1, int area2)
{
	if(cm_noareas->getValue())
		return true;

/*	
	if(area1 > (int)cm_areas.size() || area2 > (int)cm_areas.size())
		Com_Error (ERR_DROP, "CM_SetAreaPortalState: areas out of range");

	if(cm_areas[area1].numareaportals[area2] )
		return true;
	
	// area 0 is not used
	for(unsigned int i=1; i<cm_areas.size(); i++)
	{
		if(cm_areas[i].numareaportals[area1] && cm_areas[i].numareaportals[area2])
			return true;
	}
*/	
	return false;
}


/*
=================
CM_WriteAreaBits

Writes a length byte followed by a bit vector of all the areas
that area in the same flood as the area parameter

This is used by the client refreshes to cull visibility
=================
*/
int	CM_WriteAreaBits(byte *buffer, int area)
{
	int bytes = (cm_areas.size()+7)>>3;

	if(cm_noareas->getValue())
	{	
		// for debugging, send everything
		memset(buffer, 255, bytes);
	}
	/*
	else
	{
		memset(buffer, 0, bytes);

		// area 0 is not used
		for(int i=1; i<(int)cm_areas.size(); i++)
		{
			if(!area || CM_AreasConnected(i, area) || i == area)
				buffer[i>>3] |= 1<<(i&7);
		}
	}
	*/

	return bytes;
}


void	CM_MergeAreaBits(byte *buffer, int area)
{
	// area 0 is not used
	for(int i=1; i<(int)cm_areas.size(); i++)
	{
		if(CM_AreasConnected(i, area) || i == area)
			buffer[i>>3] |= 1<<(i&7);
	}
}


void	CM_WritePortalState(VFILE *stream)
{
	//TODO
	//VFS_FWrite (portalopen, sizeof(portalopen), v);
}

void	CM_ReadPortalState(VFILE *stream)
{
	//TODO
	//VFS_FRead (portalopen, sizeof(portalopen), v);
	//CM_FloodAreaConnections ();
}

/*
=============
CM_HeadnodeVisible

Returns true if any leaf under headnode has a cluster that
is potentially visible
=============
*/
bool	CM_HeadnodeVisible(int nodenum, byte *visbits)
{
#if 0
	int		leafnum;
	int		cluster;
	cnode_t	*node;

	if(nodenum < 0)
	{
		leafnum = -1 - nodenum;
		cluster = cm_leafs[leafnum].cluster;
		
		if(cluster == -1)
			return false;
		
		if(visbits[cluster>>3] & (1<<(cluster&7)))
			return true;
		
		return false;
	}

	node = &cm_nodes[nodenum];
	
	if(CM_HeadnodeVisible(node->children[0], visbits))
		return true;
	
	return CM_HeadnodeVisible(node->children[1], visbits);
#else
	return true;
#endif
}




