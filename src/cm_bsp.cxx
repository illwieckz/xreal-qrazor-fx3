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
#include <boost/crc.hpp>

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


struct cnode_t
{
	cplane_c*		plane;
	int			children[2];		// negative numbers are leafs
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

struct cshader_t
{
	uint_t			flags;
	uint_t			contents;
};

struct cbrushside_t
{
	cplane_c*		plane;
	cshader_t*		shader;
//	cshader_t*		shader;
};

struct cmesh_t
{
	std::vector<vec3_c>	vertexes;
		
	std::vector<vec3_c>	normals;
	
	std::vector<index_t>	indexes;
};

struct csurface_t
{
	int			face_type;
	
	int			vertexes_first;
	int			vertexes_num;
	
	int			shader_num;
	
	cplane_c		plane;			// BSPST_PLANAR only
	
	int			mesh_cp[2];		// BSPST_BEZIER only
	
	cmesh_t			mesh;
	
	int			checkcount;
};

struct cbrush_t
{
	int			contents;
	
	int			sides_first;
	int			sides_num;
	
	int			checkcount;		// to avoid repeated testings
};

struct cpatch_t
{
	cbbox_c			bbox_abs;
	
	int			brushes_num;
	cbrush_t*		brushes;
	
	cshader_t*		shader;
	
	int			checkcount;
};

/*
struct csurface_t
{
	int			shader_num;
	
	cmesh_t			mesh;
};
*/

struct carea_t
{
	carea_t()
	{
		memset(numareaportals, 0, sizeof(numareaportals));
	}
	
	int	numareaportals[MAX_CM_AREAS];
};




static std::string			cm_name;

static std::vector<cbrushside_t>	cm_brushsides;

static std::vector<cshader_t>		cm_shaders;

static std::vector<cplane_c>		cm_planes;

static std::vector<cnode_t>		cm_nodes;

static std::vector<cleaf_t>		cm_leafs;
	
static std::vector<int>			cm_leafbrushes;
static std::vector<int>			cm_leafsurfaces;

static std::vector<cbrush_t>		cm_brushes;

std::vector<cmodel_c*>			cm_models;


static std::vector<byte>		cm_pvs;
static int				cm_pvs_clusters_num;
static int				cm_pvs_clusters_size;
//static byte				cm_visibility[MAX_CM_VISIBILITY];
//static bsp_dvis_t*			cm_pvs = (bsp_dvis_t *)cm_visibility;

static byte				cm_nullcluster[MAX_CM_LEAFS/8];

static std::string			cm_entitystring;

static std::vector<carea_t>		cm_areas;

static std::vector<vec3_c>		cm_vertexes;
static std::vector<vec3_c>		cm_normals;
static std::vector<index_t>		cm_indexes;

static std::vector<csurface_t>		cm_surfaces;




int		cm_pointcontents;
int		cm_traces;
int		cm_brush_traces;
int		cm_mesh_traces;
int		cm_surf_traces;


static byte*	cm_base;

static cvar_t*	cm_noareas;
static cvar_t*	cm_use_brushes;
static cvar_t*	cm_use_patches;
static cvar_t*	cm_use_meshes;
static cvar_t*	cm_subdivisions;


static cplane_c*	box_planes;
static int		box_headnode;
//static cbrush_t*	box_brush;
//static cleaf_t*		box_leaf;


static std::deque<int>*	leaf_list;
static cbbox_c		leaf_bbox;
static int		leaf_topnode;

// box tracing
// 1/32 epsilon to keep floating point happy
#define	CLIP_EPSILON	(0.03125)

static int		cm_checkcount;

static vec3_c		trace_start;		// replace this by a ray
static vec3_c		trace_end;

static cbbox_c		trace_bbox;
static cbbox_c		trace_bbox_abs;

static vec3_c		trace_extents;

static trace_t		trace_trace;
static int		trace_contents;
static bool		trace_ispoint;		// optimized case


/*
// fragment clipping
#define	MAX_FRAGMENT_VERTEXES	128
static vec3_c		mark_origin;
static vec3_c		mark_normal;
static vec_t		mark_radius;

static int		mark_vertexes_num;
static int		mark_vertexes_max;
static vec3_c*		mark_vertexes;

static int		mark_fragments_num;
static int		mark_fragments_max;
static cfragment_t*	mark_fragments;

static cplane_c		mark_planes[6];

static int		mark_checkcount;
*/


static void	CM_FloodAreaConnections();


static void	CM_LoadVertexes(bsp_lump_t *l)
{
	bsp_dvertex_t	*in;
	int			i, j, count;
	
	Com_DPrintf("loading vertexes ...\n");
	
	in = (bsp_dvertex_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadVertexes: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadVertexes: BSP with no vertexes");
			
	cm_vertexes = std::vector<vec3_c>(count);
	
	for(i=0; i<count; i++, in++)
	{
		for(j=0; j<3; j++)
			cm_vertexes[i][j] = LittleFloat(in->position[j]);
	}
}

static void	CM_LoadNormals(bsp_lump_t *l)
{
	bsp_dvertex_t	*in;
	int			i, j, count;
	
	Com_DPrintf("loading normals ...\n");
	
	in = (bsp_dvertex_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadNormals: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadNormals: BSP with no vertexes");
			
	cm_normals = std::vector<vec3_c>(count);
	
	for(i=0; i<count; i++, in++)
	{
		for(j=0; j<3; j++)
			cm_normals[i][j] = LittleFloat(in->normal[j]);
		
		cm_normals[i].normalize();
	}
}


static void	CM_LoadIndexes(bsp_lump_t *l)
{
	int		*in;
	int			i, count;
	
	Com_DPrintf("loading indexes ...\n");
	
	in = (int*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadIndexes: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadIndexes: BSP with no indexes");
			
	cm_indexes = std::vector<index_t>(count);
	
	for(i=0; i<count; i++)
	{
		cm_indexes[i] = LittleLong(in[i]);
	}
}


static void	CM_CreateMesh(const bsp_dsurface_t *in, cmesh_t &mesh)
{
	int vertexes_first = LittleLong(in->vertexes_first);
	int vertexes_num = LittleLong(in->vertexes_num);
				
	mesh.vertexes = std::vector<vec3_c>(vertexes_num);
	mesh.normals = std::vector<vec3_c>(vertexes_num);
			
	for(int j=0; j<vertexes_num; j++)
	{
		mesh.vertexes[j] = cm_vertexes[vertexes_first + j];
		mesh.normals[j] = cm_normals[vertexes_first + j];
	}
				
	int indexes_num = LittleLong(in->indexes_num);
	int indexes_first = LittleLong(in->indexes_first);
				
	mesh.indexes = std::vector<index_t>(indexes_num);
	for(int j=0; j<indexes_num; j++)
	{
		mesh.indexes[j] = cm_indexes[indexes_first + j];
	}
}

static void	CM_CreateBezierMesh(const bsp_dsurface_t *in, cmesh_t &mesh)
{
	int		step[2];
	int 		size[2];
	int		flat[2];
	int		mesh_cp[2];
	int		i, p, u, v;
	
	int		vertexes_first;
	int		vertexes_num;
	
	int		subdivlevel;
	
	mesh_cp[0] = LittleLong(in->mesh_cp[0]);
	mesh_cp[1] = LittleLong(in->mesh_cp[1]);
	
	if(!mesh_cp[0] || !mesh_cp[1])
		return;
	
	subdivlevel = cm_subdivisions->getInteger();
	if(subdivlevel < 1)
		subdivlevel = 1;
	
	vertexes_first = LittleLong(in->vertexes_first);
	vertexes_num = LittleLong(in->vertexes_num);
	
	std::vector<vec4_c>	vertexes(vertexes_num);
	
	for(i=0; i<vertexes_num; i++)
	{
		cm_vertexes[vertexes_first +i].copyTo(vertexes[i]);
	}
	
	// find degree of subdivision
	Curve_GetFlatness(subdivlevel, &(vertexes[0]), mesh_cp, flat);
	
	// allocate space for mesh
	step[0] = (1 << flat[0]);		//step u
	step[1] = (1 << flat[1]);		//step v
	
	size[0] = (mesh_cp[0] / 2) * step[0] + 1;
	size[1] = (mesh_cp[1] / 2) * step[1] + 1;
	vertexes_num = size[0] * size[1];

	mesh.vertexes = std::vector<vec3_c>(vertexes_num);
	
	// allocate and fill index table
	int indexes_num = (size[0]-1) * (size[1]-1) * 6;
	mesh.indexes = std::vector<index_t>(indexes_num);
	
	for(v=0, i=0; v<size[1]-1; v++)
	{
		for(u=0; u<size[0]-1; u++, i+=6)
		{	
			mesh.indexes[i+0] = p = v * size[0] + u;
			mesh.indexes[i+1] = p + size[0];
			mesh.indexes[i+2] = p + 1;
			mesh.indexes[i+3] = p + 1;
			mesh.indexes[i+4] = p + size[0];
			mesh.indexes[i+5] = p + size[0] + 1;
		}
	}
	
	std::vector<vec4_c>	vertexes2(vertexes_num);
		
	// fill in
	Curve_EvalQuadricBezierPatch(&(vertexes[0]), mesh_cp, step, &(vertexes2[0]));
	
	for(i=0; i<(int)mesh.vertexes.size(); i++)
	{
		mesh.vertexes[i] = vertexes2[i];
	}
}

static void	CM_LoadSurfaces(bsp_lump_t *l, d_bsp_c *bsp)
{
	bsp_dsurface_t		*in;
	int			i, count;
	
	Com_DPrintf("loading surfaces ...\n");
	
	in = (bsp_dsurface_t*)(cm_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadSurfaces: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadSurfaces: BSP with no surfaces");

	cm_surfaces = std::vector<csurface_t>(count);
	
	for(i=0; i<count; i++, in++)
	{
		csurface_t & out = cm_surfaces[i];
		
		out.face_type = (bsp_surface_type_t)LittleLong(in->face_type);
		
		out.shader_num = LittleLong(in->shader_num);
		
		switch(out.face_type)
		{
			case BSPST_PLANAR:
			{
				 CM_CreateMesh(in, out.mesh);
			
				//vec3_c	origin;
				vec3_c	normal;
			
				for(int j=0; j<3; j++)
				{
					//origin[j] = LittleFloat(in->origin[j]);
					normal[j] = LittleFloat(in->normal[j]);
				}
				
				if(!normal.length())
				{
					//out.face_type = BSPST_PLANAR_NOCULL;
					out.face_type = BSPST_MESH;
				}
				break;
			}
			
			case BSPST_MESH:
			{
				CM_CreateMesh(in, out.mesh);
				break;
			}
			
			case BSPST_BEZIER:
			{
				CM_CreateBezierMesh(in, out.mesh);
				break;
			}
			
			case BSPST_BAD:
			case BSPST_FLARE:
			default:
				Com_Error(ERR_DROP, "CM_LoadSurfaces: bad surface type %i", out.face_type);
		}
		
		if(bsp)
			bsp->addSurface(out.face_type, out.shader_num, out.mesh.vertexes, out.mesh.indexes);
	}
}

static void	CM_LoadLeafSurfaces(bsp_lump_t *l, d_bsp_c *bsp)
{
	Com_DPrintf("loading leaf surfaces ...\n");
	
	int* in = (int*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadLeafSurfaces: funny lump size");
	int count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadLeafSurfaces: BSP with no faces");

	cm_leafsurfaces = std::vector<int>(count);
	
	for(int i=0; i<count; i++)
	{
		int j = LittleLong(in[i]);
		
		if(j<0 || j >= (int)cm_surfaces.size())
			Com_Error(ERR_DROP, "CM_LoadLeafSurfaces: bad surface number %i", j);
		
		cm_leafsurfaces[i] = j;
		
		if(bsp)
			bsp->addLeafSurface(j);
	}
}

static void	CM_LoadModels(bsp_lump_t *l, d_bsp_c *bsp)
{
	bsp_dmodel_t		*in;
	//cleaf_t		*leaf;
	//int			*leafbrush;
	
	Com_DPrintf("loading models ...\n");

	in = (bsp_dmodel_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadModels: funny lump size");
	int count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadModels: BSP with no models");


	for(int i=0; i<count; i++, in++)
	{
		cmodel_c *model = new cmodel_c(va("*%i", i), NULL, 0);
			
		int	modelsurfaces_first = LittleLong(in->modelsurfaces_first);
		int	modelsurfaces_num = LittleLong(in->modelsurfaces_num);
		
#if 1
		// create a big polygons list for every model using all surfaces for it
		int	vertexes_counter = 0;
		
		for(int j=0; j<modelsurfaces_num; j++)
		{
			csurface_t &surf = cm_surfaces[modelsurfaces_first + j];
	
			cshader_t &shader = cm_shaders[surf.shader_num];			
			
			if(shader.flags & (X_SURF_NOIMPACT | X_SURF_NONSOLID))
				continue;
			
			for(std::vector<vec3_c>::const_iterator ir = surf.mesh.vertexes.begin(); ir != surf.mesh.vertexes.end(); ir++)
			{
				model->vertexes.push_back(*ir);
			}
			
			
			for(std::vector<index_t>::const_iterator ir = surf.mesh.indexes.begin(); ir != surf.mesh.indexes.end(); ir++)
			{
				model->indexes.push_back(vertexes_counter + *ir);
			}
			
			
			vertexes_counter += surf.mesh.vertexes.size();
		}
		
		reverse(model->indexes.begin(), model->indexes.end());
#else		
		for(int j=0; j<modelsurfaces_num; j++)
		{
			csurface_t &surf = cm_surfaces[modelsurfaces_first + j];
			cshader_t &shader = cm_shaders[surf.shader_num];
			
			if(surf.mesh.vertexes.empty() || surf.mesh.indexes.empty())
				continue;
		
			model->surfaces.push_back(csurface_c(surf.mesh.vertexes, surf.mesh.indexes, shader.flags, shader.contents));
		}
#endif
		
		/*
		if(!i)
		{
			out->headnode = 0;
		}
		else
		{
			out->headnode = -1 - cm_leafs_num;
		
			leaf = &cm_leafs[cm_leafs_num++];
			leaf->leafbrushes_num = LittleLong(in->brushes_num);
			leaf->leafbrushes_first = cm_leafbrushes_num;
			leaf->contents = 0;
			
			
			for(j=0, leafbrush = &cm_leafbrushes[cm_leafbrushes_num]; j<leaf->leafbrushes_num; j++, leafbrush++)
			{
				*leafbrush = LittleLong(in->brushes_first) + j;
				leaf->contents |= cm_brushes[*leafbrush].contents;
			}
			
			cm_leafbrushes_num += leaf->leafbrushes_num;
		}
		*/

		for(int j=0; j<3; j++)
		{	
			// spread the mins / maxs by a pixel
			model->_bbox._mins[j] = LittleFloat(in->mins[j]);
			model->_bbox._maxs[j] = LittleFloat(in->maxs[j]);
		}
		
		cm_models.push_back(model);
	}
	
	// set bsp size for AABB and assign it to the model
	if(bsp)
		bsp->setLengths(cm_models[0]->getBBox().size());
}

static void	CM_LoadShaders(bsp_lump_t *l)
{
	bsp_dshader_t		*in;
	int			i, count;
	
	
	Com_DPrintf("loading shaders ...\n");

	in = (bsp_dshader_t*)(cm_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadShaders: funny lump size");
	count = l->filelen / sizeof(*in);
	
	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadShaders: BSP with no shaders");

	cm_shaders = std::vector<cshader_t>(count);

	for(i=0; i<count; i++, in++)
	{
		cm_shaders[i].flags = LittleLong(in->flags);
		cm_shaders[i].contents = LittleLong(in->contents);
	}
}


static void	CM_LoadNodes(bsp_lump_t *l, d_bsp_c *bsp)
{
	bsp_dnode_t	*in;	
	
	int			i, j, count;
	
	Com_DPrintf("loading nodes ...\n");
	
	in = (bsp_dnode_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadNodes: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadNodes: BSP has no nodes");
	
	cm_nodes = std::vector<cnode_t>(count);
	
	for(i=0; i<count; i++,in++)
	{
		cnode_t &out = cm_nodes[i];
	
		out.plane = &cm_planes[LittleLong(in->plane_num)];
		
		for(j=0; j<2; j++)
		{
			out.children[j] = LittleLong(in->children[j]);
		}
		
		if(bsp)
			bsp->addNode(LittleLong(in->plane_num), out.children[0], out.children[1]);
	}

}

static void	CM_LoadBrushes(bsp_lump_t *l, d_bsp_c *bsp)
{
	Com_DPrintf("loading brushes ...\n");
	
	bsp_dbrush_t *in = (bsp_dbrush_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadBrushes: funny lump size");
	int count = l->filelen / sizeof(*in);

	cm_brushes = std::vector<cbrush_t>(count);

	for(int i=0; i<count; i++, in++)
	{
		cbrush_t &out = cm_brushes[i];
		
		int shader_num = LittleLong(in->shader_num);
		
		out.contents = cm_shaders[shader_num].contents;
		out.sides_first = LittleLong(in->sides_first);
		out.sides_num = LittleLong(in->sides_num);
		
		//if(bsp)
		//	bsp->addBrush(out.sides_first, out.sides_num);
	}
}

static void	CM_LoadLeafs(bsp_lump_t *l, d_bsp_c *bsp)
{
	bsp_dleaf_t 	*in;
	//cbrush_t		*brush;
	int			count;
	
	Com_DPrintf("loading leafs ...\n");
	
	in = (bsp_dleaf_t*)(cm_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadLeafs: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadLeafs: BSP with no leafs");

	cm_leafs = std::vector<cleaf_t>(count);	

	for(int i=0; i<count; i++, in++)
	{
		cleaf_t &out = cm_leafs[i];
		
		out.contents = 0;
		out.cluster = LittleLong(in->cluster);
		out.area = LittleLong(in->area);
		
		out.leafsurfaces_first = LittleLong(in->leafsurfaces_first);
		out.leafsurfaces_num = LittleLong(in->leafsurfaces_num);
		
		out.leafbrushes_first = LittleLong(in->leafbrushes_first);
		out.leafbrushes_num = LittleLong(in->leafbrushes_num);
		
		//out->leafpatches_first
		//out->leafpatches_num

		/*
		for(j=0; j<out->leafbrushes_num; j++)
		{
			brush = &cm_brushes[cm_leafbrushes[out->leafbrushes_first + j]];
			out->contents |= brush->contents;
		}
		*/
		
		if(out.area >= (int)cm_areas.size())
			cm_areas.push_back(carea_t());
		
		if(bsp)
			bsp->addLeaf(out.leafsurfaces_first, out.leafsurfaces_num, out.leafbrushes_first, out.leafbrushes_num, out.cluster, out.area);
	}
}

static void	CM_LoadPlanes(bsp_lump_t *l, d_bsp_c* bsp)
{
	int			i, j;
	bsp_dplane_t 	*in;
	int			count;
	vec3_c	normal;
	float	dist;
	
	
	Com_DPrintf("loading planes ...\n");
	
	in = (bsp_dplane_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadPlanes: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadPlanes: BSP with no planes");
	
	// need to save space for box planes
//	if(count > MAX_BSP_PLANES)
//		Com_Error (ERR_DROP, "CM_LoadPlanes: BSP has too many planes");

	cm_planes = std::vector<cplane_c>(count);

	for(i=0; i<count; i++, in++)
	{		
		for(j=0; j<3; j++)
		{
			normal[j] = LittleFloat(in->normal[j]);
		}
		
		dist = LittleFloat(in->dist);
		
		cm_planes[i].set(normal, dist);
		
		if(bsp)
			bsp->addPlane(cm_planes[i]);
	}
}

static void	CM_LoadLeafBrushes(bsp_lump_t *l, d_bsp_c* bsp)
{
	Com_DPrintf("loading leafbrushes ...\n");
	
	int *in = (int*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadLeafBrushes: funny lump size");
	int count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadLeafBrushes: BSP with no leafbrushes");
	
	cm_leafbrushes = std::vector<int>(count);
	
	for(int i=0; i<count; i++, in++)
	{
		int& out = cm_leafbrushes[i];
		
		out = LittleLong(*in);
		
		//if(bsp)
		//	bsp->addLeafBrush(out);
	}
}

static void	CM_LoadBrushSides(bsp_lump_t *l, d_bsp_c* bsp)
{
	Com_DPrintf("loading brushsides ...\n");
	
	bsp_dbrushside_t *in = (bsp_dbrushside_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadBrushSides: funny lump size");
	int count = l->filelen / sizeof(*in);

	cm_brushsides = std::vector<cbrushside_t>(count);

	for(int i=0; i<count; i++, in++)
	{
		cbrushside_t& out = cm_brushsides[i];
	
		out.plane = &cm_planes[LittleLong(in->plane_num)];
		
		uint_t shader_num = LittleLong(in->shader_num);
		
		if(shader_num >= cm_shaders.size())
			Com_Error(ERR_DROP, "CM_LoadBrushSides: Bad brushside shadernum");
		
		out.shader = &cm_shaders[shader_num];
		
		//if(bsp)
		//	bsp->addBrushSide(LittleLong(in->plane_num));
	}
}

static void	CM_LoadVisibility(bsp_lump_t *l)
{
	Com_DPrintf("loading visibility ...\n");
	
	int pvs_size = l->filelen - BSP_PVS_HEADERSIZE;
	
	Com_DPrintf("PVS data size: %i\n", pvs_size);
	
	if(pvs_size <= 0)
	{
		cm_pvs.clear();
		return;
	}
	
	cm_pvs = std::vector<byte>(pvs_size, 0);
	
	for(int i=0; i<pvs_size; i++)
	{
		cm_pvs[i] = cm_base[l->fileofs + BSP_PVS_HEADERSIZE + i];
	}
	
	cm_pvs_clusters_num  = LittleLong(((int*)((byte*)cm_base + l->fileofs))[0]);
	cm_pvs_clusters_size = LittleLong(((int*)((byte*)cm_base + l->fileofs))[1]);
	

	/*
	memcpy(cm_visibility, ((byte*)cm_base + l->fileofs), l->filelen);

	cm_pvs->clusters_num = LittleLong(cm_pvs->clusters_num);
	cm_pvs->cluster_size = LittleLong(cm_pvs->cluster_size);
	*/
}


static void	CM_LoadEntityString(bsp_lump_t *l)
{
	Com_DPrintf("loading entities ...\n");
	
	cm_entitystring.clear();
	
	for(int i=0; i<l->filelen; i++)
	{
		cm_entitystring += cm_base[l->fileofs + i];
	}
	
	cm_entitystring = X_strlwr(cm_entitystring);
}

/*
===================
CM_InitBoxHull

Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structure.
===================
*/
static void	CM_InitBoxHull()
{
#if 0
	for(int i=0; i<6; i++)
	{
		cm_nodes.push_back(cnode_t());
		cm_brushsides.push_back(cbrushside_t());	
		
		cm_planes.push_back(cplane_c());
		cm_planes.push_back(cplane_c());
	}
		
	cm_brushes.push_back(cbrush_t());
	cm_leafs.push_back(cleaf_t());
	cm_leafbrushes.push_back(cm_brushes.size());
	
	/*
	if(	cm_nodes_num >= MAX_CM_NODES+6 || 
		cm_brushes_num >= MAX_CM_BRUSHES+1 || 
		cm_leafbrushes_num >= MAX_CM_LEAFBRUSHES+1 || 
		cm_brushsides_num >= MAX_CM_BRUSHSIDES+6 || 
		cm_planes_num > MAX_CM_PLANES+12
	)
		Com_Error (ERR_DROP, "CM_InitBoxHull: Not enough room for box tree");
	*/
	
	box_headnode = cm_nodes.size()-6;
	box_planes = &cm_planes[cm_planes.size() -12];
	
	box_brush = &cm_brushes[cm_brushes.size() -1];
	box_brush->sides_num = 6;
	box_brush->sides_first = cm_brushsides.size() -6;
	box_brush->contents = X_CONT_BODY;

	box_leaf = &cm_leafs[cm_leafs.size() -1];
	box_leaf->contents = X_CONT_BODY;
	box_leaf->leafbrushes_first = cm_leafbrushes.size() -1;
	box_leaf->leafbrushes_num = 1;
	
	for(int i=0; i<6; i++)
	{
		int side = i&1;

		// brush sides
		cbrushside_t* s = &cm_brushsides[(cm_brushsides.size() -(1+6)) + i];
		s->plane = &cm_planes[((cm_planes.size() -(1+12)) + i*2 + side)];
		s->shader = NULL;

		// nodes
		cnode_t* c = &cm_nodes[box_headnode + i];
		c->plane = &cm_planes[((cm_planes.size() -(1+12)) + i*2)];
		c->children[side] = -1 - (cm_leafs.size() -1);
		
		if(i != 5)
			c->children[side^1] = box_headnode + i + 1;
		else
			c->children[side^1] = -1 - (cm_leafs.size() -1);
		
		// planes
		cplane_c* p = &box_planes[i*2];
		p->_type = (plane_type_e)(i>>1);
		p->_normal.clear();
		p->_normal[i>>1] = 1;

		p = &box_planes[i*2+1];
		p->_type = (plane_type_e)(3 + (i>>1));
		p->_normal.clear();
		p->_normal[i>>1] = -1;
	}
#endif
}


d_bsp_c*	CM_BeginRegistration(const std::string &name, bool clientload, unsigned *checksum, dSpaceID space)
{
	unsigned*		buf;
	int			i;
	bsp_dheader_t		header;
	int			length;
	std::string		full_name = "maps/" + name + ".bsp";
	static unsigned		last_checksum;

	cm_noareas	= Cvar_Get("cm_noareas", "1", CVAR_NONE);
	cm_use_brushes	= Cvar_Get("cm_use_brushes", "1", CVAR_NONE);
	cm_use_patches	= Cvar_Get("cm_use_patches", "0", CVAR_NONE);
	cm_use_meshes	= Cvar_Get("cm_use_meshes", "0", CVAR_NONE);
	cm_subdivisions	= Cvar_Get("cm_subdivisions", "0", CVAR_NONE);
	
	if(X_strequal(cm_name.c_str(), name.c_str()) && (clientload || !Cvar_VariableValue ("flushmap")))
	{
		*checksum = last_checksum;
		
		if(!clientload)
		{
			CM_FloodAreaConnections();
		}
		return NULL;		// still have the right version
	}

	// free old stuff
	X_purge<std::vector<cmodel_c*> >(cm_models);
	cm_models.clear();
	
	cm_planes.clear();
	cm_nodes.clear();
	cm_leafs.clear();
	
	cm_pvs.clear();
	cm_entitystring = "";
	cm_areas.clear();
		
	cm_vertexes.clear();
	cm_indexes.clear();

	// load the file
	length = VFS_FLoad(full_name, (void **)&buf);
	if (!buf)
		Com_Error(ERR_DROP, "CM_BeginRegistration: Couldn't load %s", full_name.c_str());
		
	boost::crc_32_type crc;
	crc.process_bytes(buf, length);
	last_checksum = LittleLong(crc.checksum());
	*checksum = last_checksum;

	header = *(bsp_dheader_t *)buf;
	for(i=0; i<(int)sizeof(bsp_dheader_t)/4; i++)
		((int *)&header)[i] = LittleLong(((int *)&header)[i]);

	if(header.version != BSP_VERSION)
		Com_Error(ERR_DROP, "CM_BeginRegistration: %s has wrong version number (%i should be %i)", full_name.c_str(), header.version, BSP_VERSION);

	cm_base = (byte*)buf;

	Com_DPrintf("CM_BeginRegistration: loading %s into heap ...\n", full_name.c_str());
	
	// create ODE BSP collision geom
	d_bsp_c* bsp = NULL;
	if(space != 0)
		bsp = new d_bsp_c(space);

	
	CM_LoadShaders(&header.lumps[BSP_LUMP_SHADERS]);
	CM_LoadPlanes(&header.lumps[BSP_LUMP_PLANES], bsp);
	CM_LoadLeafBrushes(&header.lumps[BSP_LUMP_LEAFBRUSHES], bsp);
	CM_LoadBrushes(&header.lumps[BSP_LUMP_BRUSHES], bsp);
	CM_LoadBrushSides(&header.lumps[BSP_LUMP_BRUSHSIDES], bsp);
	CM_LoadVertexes(&header.lumps[BSP_LUMP_VERTEXES]);
	CM_LoadNormals(&header.lumps[BSP_LUMP_VERTEXES]);
	CM_LoadIndexes(&header.lumps[BSP_LUMP_INDEXES]);
	CM_LoadSurfaces(&header.lumps[BSP_LUMP_SURFACES], bsp);
	CM_LoadLeafSurfaces(&header.lumps[BSP_LUMP_LEAFSURFACES], bsp);
	CM_LoadLeafs(&header.lumps[BSP_LUMP_LEAFS], bsp);
	CM_LoadNodes(&header.lumps[BSP_LUMP_NODES], bsp);
	CM_LoadModels(&header.lumps[BSP_LUMP_MODELS], bsp);
	CM_LoadVisibility(&header.lumps[BSP_LUMP_VISIBILITY]);
	CM_LoadEntityString(&header.lumps[BSP_LUMP_ENTITIES]);

	VFS_FFree(buf);

	CM_FloodAreaConnections();
	
	memset(cm_nullcluster, 255, sizeof(cm_nullcluster));
	
	cm_name = full_name;
	
	CM_InitBoxHull();
	
	return bsp;
}



int	CM_ClusterSize()
{
	return cm_pvs.size() ? cm_pvs_clusters_size : (MAX_CM_LEAFS / 8);
}

int	CM_NumClusters()
{
	return cm_pvs_clusters_num;
}


int	CM_NumModels()
{
	return cm_models.size();
}


const char*	CM_EntityString()
{
	return cm_entitystring.c_str();
}


int	CM_LeafContents(int leafnum)
{
	if(leafnum < 0 || leafnum >= (int)cm_leafs.size())
		Com_Error(ERR_DROP, "CM_LeafContents: out of range %i", leafnum);
	
	return cm_leafs[leafnum].contents;
}

int	CM_LeafCluster(int leafnum)
{
	if(leafnum < 0 || leafnum >= (int)cm_leafs.size())
		Com_Error(ERR_DROP, "CM_LeafCluster: out of range %i", leafnum);
	
	return cm_leafs[leafnum].cluster;
}


int	CM_LeafArea(int leafnum)
{
	if(leafnum < 0 || leafnum >= (int)cm_leafs.size())
		Com_Error(ERR_DROP, "CM_LeafArea: out of range %i", leafnum);
	
	return cm_leafs[leafnum].area;
}


/*
===================
CM_HeadnodeForBox

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
===================
*/
int	CM_HeadnodeForBox(const cbbox_c & bbox)
{
	box_planes[0]._dist	=  bbox._maxs[0];
	box_planes[1]._dist	= -bbox._maxs[0];
	box_planes[2]._dist	=  bbox._mins[0];
	box_planes[3]._dist	= -bbox._mins[0];
	
	box_planes[4]._dist	=  bbox._maxs[1];
	box_planes[5]._dist	= -bbox._maxs[1];
	box_planes[6]._dist	=  bbox._mins[1];
	box_planes[7]._dist	= -bbox._mins[1];
	
	box_planes[8]._dist	=  bbox._maxs[2];
	box_planes[9]._dist	= -bbox._maxs[2];
	box_planes[10]._dist	=  bbox._mins[2];
	box_planes[11]._dist	= -bbox._mins[2];

	return box_headnode;
}

static int	CM_PointLeafnum_r(const vec3_c &p, int num)
{
	float		d;
	cnode_t		*node;
	cplane_c	*plane;

	while(num >= 0)
	{
		node = &cm_nodes[num];
		plane = node->plane;
		
		d = plane->distance(p);
		
		if(d >= 0)
			num = node->children[0];
		else
			num = node->children[1];
	}

	cm_pointcontents++;		// optimize counter

	return -1 - num;
}

int	CM_PointLeafnum(const vec3_c &p)
{
	if(!cm_planes.size())
		return 0;		// sound may call this without map loaded
	
	return CM_PointLeafnum_r(p, 0);
}



/*
=============
CM_BoxLeafnums

Fills in a list of all the leafs touched
=============
*/
static void	CM_BoxLeafnums_r(int nodenum)
{
	while(nodenum >= 0)
	{
		const cnode_t& node = cm_nodes[nodenum];
		
		plane_side_e s = node.plane->onSide(leaf_bbox);
		
		if(s == SIDE_FRONT)
			nodenum = node.children[0];
		
		else if(s == SIDE_BACK)
			nodenum = node.children[1];
		
		else
		{	// go down both
			if(leaf_topnode == -1)
				leaf_topnode = nodenum;
			
			CM_BoxLeafnums_r(node.children[0]);
			
			nodenum = node.children[1];
		}
	}
	
	leaf_list->push_back(-1 - nodenum);
}

int	CM_PointAreanum(const vec3_c &p)
{
	if(!cm_nodes.size())
		return 0;		// sound may call this without map loaded
	
	int leafnum = CM_PointLeafnum(p);
	int areanum = CM_LeafArea(leafnum);
	
	return areanum;
}

int	CM_BoxLeafnums(const cbbox_c &bbox, std::deque<int> &list, int headnode)
{
	leaf_list = &list;
	leaf_bbox = bbox;
	leaf_topnode = -1;

	if(headnode < 0 || headnode >= (int)cm_nodes.size())
		CM_BoxLeafnums_r(0);
	else
		CM_BoxLeafnums_r(headnode);

	return leaf_topnode;
}

int	CM_PointContents(const vec3_c &p, int headnode)
{
	int	contents = 0;
	
	if(!cm_nodes.size())	// map not loaded
		return 0;

	const cleaf_t& leaf = cm_leafs[CM_PointLeafnum_r(p, headnode)];
	
	for(int i=0; i<leaf.leafbrushes_num; i++)
	{
		const cbrush_t& brush = cm_brushes[cm_leafbrushes[leaf.leafbrushes_first + i]];
		
		int j;
		
		for(j=0; j<brush.sides_num; j++)
		{
			const cbrushside_t& brushside = cm_brushsides[brush.sides_first + j];
			
			if(brushside.plane->distance(p) > 0)
				break;
		}
		
		if(j == brush.sides_num)
			contents |= brush.contents;
	}
	
	return contents;
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
	
	if(!cm_nodes.size())	//map not loaded
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

void	CM_ClipBoxToBrush(const cbbox_c &bbox, const vec3_c &p1, const vec3_c &p2, trace_t &trace, const cbrush_t &brush)
{
	int			i, j;
	cplane_c	*plane, *clipplane;
	float		enterfrac, leavefrac;
	vec3_c		ofs, ofs_ext;
	float		d1, d2;
	bool		getout, startout;
	float		f;
	cbrushside_t	*side, *leadside;

	enterfrac = -1;
	leavefrac = 1;
	clipplane = NULL;

	if(!brush.sides_num)
		return;

	cm_brush_traces++;

	getout = false;
	startout = false;
	leadside = NULL;

	for(i=0; i<brush.sides_num; i++)
	{
		side = &cm_brushsides[brush.sides_first + i];
		plane = side->plane;

		// FIXME: special case for axial

		if(!trace_ispoint)
		{	// general box case

			// push the plane out apropriately for mins/maxs

			// FIXME: use signbits into 8 way lookup for each mins/maxs
			for(j=0; j<3; j++)
			{
				if(plane->_normal[j] < 0)
					ofs[j] = bbox._maxs[j];
				else
					ofs[j] = bbox._mins[j];
			}
					
			ofs_ext = ofs + p1; 
			d1 = plane->distance(ofs_ext);
			
			ofs_ext = ofs + p2;
			d2 = plane->distance(ofs_ext);	
		}
		else
		{	// special point case
			d1 = plane->distance(p1);
			d2 = plane->distance(p2);
		}
		
		
		if(d2 > 0)
			getout = true;	// endpoint is not in solid
		
		if(d1 > 0)
			startout = true;

		// if completely in front of face, no intersection
		if(d1 > 0 && d2 >= d1)
			return;

		if(d1 <= 0 && d2 <= 0)
			continue;

		// crosses face
		if(d1 > d2)
		{	
			// enter
			f = (d1-CLIP_EPSILON) / (d1-d2);
			
			if(f > enterfrac)
			{
				enterfrac = f;
				clipplane = plane;
				leadside = side;
			}
		}
		else
		{	
			// leave
			f = (d1+CLIP_EPSILON) / (d1-d2);
			
			if(f < leavefrac)
				leavefrac = f;
		}
	}

	if(!startout)
	{	
		// original point was inside brush
		trace.startsolid = true;
		
		if(!getout)
			trace.allsolid = true;
		
		return;
	}
	
	if(enterfrac < leavefrac)
	{
		if(enterfrac > -1 && enterfrac < trace.fraction)
		{
			if(enterfrac < 0)
				enterfrac = 0;
			
			trace.fraction = enterfrac;
			trace.plane = *clipplane;
			//trace->surface = leadside->shader;
			trace.pos_flags = leadside->shader->flags;
			trace.pos_contents = leadside->shader->contents;
			trace.neg_contents = brush.contents;
		}
	}
}

void	CM_TestBoxInBrush(const cbbox_c &bbox, const vec3_c &p1, trace_t &trace, const cbrush_t &brush)
{
	int			i, j;
	cplane_c	*plane;
	vec3_c		ofs;
	float		d1;
	cbrushside_t	*side;

	if(!brush.sides_num)
		return;

	for(i=0; i<brush.sides_num; i++)
	{
		side = &cm_brushsides[brush.sides_first + i];
		plane = side->plane;

		// general box case

		// push the plane out apropriately for mins/maxs

		// FIXME: use signbits into 8 way lookup for each mins/maxs
		for(j=0; j<3; j++)
		{
			if(plane->_normal[j] < 0)
				ofs[j] = bbox._maxs[j];
			else
				ofs[j] = bbox._mins[j];
		}
		
		ofs += p1;
		
		d1 = plane->distance(ofs);

		// if completely in front of face, no intersection
		if(d1 > 0)
			return;
	}

	// inside this brush
	trace.startsolid = trace.allsolid = true;
	trace.fraction = 0;
	trace.pos_flags = 0;
	trace.pos_contents = 0;
	trace.neg_contents = brush.contents;
}

/*
void	CM_TestBoxInMesh(const cbbox_c &bbox, const vec3_c &p1, trace_t *trace, cmesh_t *mesh, cshader_t *shader)
{
	int			i;
	cplane_c	*plane;
	vec3_c		ofs;
	float		d1;
	
	index_t*	index;
	vec3_c		v0, v1, v2;
	vec3_c		n0, n1, n2;
	vec3_c		normal;
	vec_t		dist;
	cplane_c	p;

	if(!mesh->vertexes_num || mesh->indexes_num)
		return;

	for(i=0, index=mesh->indexes; i<(mesh->indexes_num / 3); i++, index += 3)
	{
		v0 = mesh->vertexes[index[0]];
		v1 = mesh->vertexes[index[1]];
		v2 = mesh->vertexes[index[2]];
		
		n0 = mesh->normals[index[0]];
		n1 = mesh->normals[index[1]];
		n2 = mesh->normals[index[2]];
		
		normal[0] = n0[0] + n1[0] + n2[0];
		normal[1] = n0[1] + n1[1] + n2[1];
		normal[2] = n0[2] + n1[2] + n2[2];
		
		normal.normalize();
		
		dist = normal.dotProduct(v0);
		
		p.set(normal, dist);
		
		plane = &p;

		// general box case

		// push the plane out apropriately for mins/maxs

		// FIXME: use signbits into 8 way lookup for each mins/maxs
		for(int j=0; j<3; j++)
		{
			if(plane->_normal[j] < 0)
				ofs[j] = bbox._maxs[j];
			else
				ofs[j] = bbox._mins[j];
		}
		
		ofs += p1;
		
		d1 = plane->diff(p1);

		// if completely in front of face, no intersection
		if(d1 > 0.01)
			return;
	}

	// inside this brush
	trace->startsolid = trace->allsolid = true;
	trace->fraction = 0;
	trace->contents = shader->contents;
}
*/

static void	CM_TraceToLeaf(int leafnum)
{
	const cleaf_t& leaf = cm_leafs[leafnum];
	
//	if(!(leaf.contents & trace_contents))
//		return;
	
//	if(cm_use_brushes->getInteger())
	{
		// trace line against all brushes in the leaf
		for(int i=0; i<leaf.leafbrushes_num; i++)
		{
			cbrush_t& brush = cm_brushes[cm_leafbrushes[leaf.leafbrushes_first + i]];
		
			if(brush.checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
		
			brush.checkcount = cm_checkcount;

//			if(!(brush.contents & trace_contents))
//				continue;
		
			CM_ClipBoxToBrush(trace_bbox, trace_start, trace_end, trace_trace, brush);
		
			if(!trace_trace.fraction)
				return;
		}
	}
	
	/*	
	if(cm_use_patches->integer)
	{
		// trace line against all patches in the leaf
		for(int i=0; i<leaf->leafpatches_num; i++)
		{
			cpatch_t *patch = &cm_patches[cm_leafpatches[leaf->leafpatches_first + i]];
		
			if(patch->checkcount == cm_checkcount)
				continue;	// already checked this patch in another leaf
			
			patch->checkcount = cm_checkcount;

			if(!(patch->shader->contents & trace_contents))
				continue;
		
			if(!patch->bbox_abs.intersect(trace_bbox_abs))
				continue;
		
			for(int j=0; j<patch->brushes_num; j++)
			{
				CM_ClipBoxToPatch(trace_bbox, trace_start, trace_end, &trace_trace, &patch->brushes[j]);
			
				if(!trace_trace.fraction)
					return;
			}
		}
	}
	
	if(cm_use_meshes->integer)
	{
		// trace line against all surfaces in the leaf
		for(int i=0; i<leaf->leafsurfaces_num; i++)
		{
			csurface_t *surf = &cm_surfaces[cm_leafsurfaces[leaf->leafsurfaces_first + i]];
		
			if(surf->checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
		
			surf->checkcount = cm_checkcount;
		
			cshader_t *shader = &cm_shaders[surf->shader_num];
		
			if(!(shader->contents & trace_contents))
				continue;
		
			CM_ClipBoxToMesh(trace_bbox, trace_start, trace_end, &trace_trace, surf->mesh, shader);
		
			if(!trace_trace.fraction)
				return;
		}
	}
	*/
}


static void	CM_TestInLeaf(int leafnum)
{
	const cleaf_t& leaf = cm_leafs[leafnum];
	
	if(!(leaf.contents & trace_contents))
		return;
	
	if(cm_use_brushes->getInteger())
	{
		// trace line against all brushes in the leaf
		for(int i=0; i<leaf.leafbrushes_num; i++)
		{
			cbrush_t& brush = cm_brushes[cm_leafbrushes[leaf.leafbrushes_first + i]];
		
			if(brush.checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
		
			brush.checkcount = cm_checkcount;

			if(!(brush.contents & trace_contents))
				continue;
		
			CM_TestBoxInBrush(trace_bbox, trace_start, trace_trace, brush);
		
			if(!trace_trace.fraction)
				return;
		}
	}
	
	/*
	if(cm_use_patches->integer)
	{
		// trace line against all patches in the leaf
		for(int i=0; i<leaf->leafpatches_num; i++)
		{
			cpatch_t *patch = &cm_patches[cm_leafpatches[leaf->leafpatches_first + i]];
		
			if(patch->checkcount == cm_checkcount)
				continue;	// already checked this patch in another leaf
		
			patch->checkcount = cm_checkcount;

			if(!(patch->shader->contents & trace_contents))
				continue;
		
			if(!patch->bbox_abs.intersect(trace_bbox_abs))
				continue;
		
			for(int j=0; j<patch->brushes_num; j++)
			{
				CM_TestBoxInBrush(trace_bbox, trace_start, &trace_trace, &patch->brushes[j]);
			
				if(!trace_trace.fraction)
					return;
			}
		}
	}
	
	if(cm_use_meshes->integer)
	{
		// trace line against all surfaces in the leaf
		for(int i=0; i<leaf->leafsurfaces_num; i++)
		{
			csurface_t *surf = &cm_surfaces[cm_leafsurfaces[leaf->leafsurfaces_first + i]];
		
			if(surf->checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
			
			surf->checkcount = cm_checkcount;
		
			cshader_t *shader = &cm_shaders[surf->shader_num];
		
			if(!(shader->contents & trace_contents))
				continue;
		
			CM_TestBoxInMesh(trace_bbox, trace_start, &trace_trace, surf->mesh, shader);
		
			if(!trace_trace.fraction)
				return;
		}
	}
	*/
}

static void	CM_HullCheck_r(int num, float p1f, float p2f, const vec3_c &p1, const vec3_c &p2)
{
	cnode_t		*node;
	cplane_c	*plane;
	float		t1, t2, offset;
	float		frac, frac2;
	float		idist;
	int		i;
	vec3_t		mid;
	int		side;
	float		midf;

	if(trace_trace.fraction <= p1f)
		return;		// already hit something nearer

	// if < 0, we are in a leaf node
	if(num < 0)
	{
		CM_TraceToLeaf(-1 - num);
		return;
	}

	//
	// find the point distances to the seperating plane
	// and the offset for the size of the box
	//
	node = &cm_nodes[num];
	plane = node->plane;

	if(plane->getType() < 3)
	{
		t1 = p1[plane->getType()] - plane->_dist;
		t2 = p2[plane->getType()] - plane->_dist;
		
		offset = trace_extents[plane->getType()];
	}
	else
	{
		t1 = plane->_normal.dotProduct(p1) - plane->_dist;
		t2 = plane->_normal.dotProduct(p2) - plane->_dist;
		
		if(trace_ispoint)
			offset = 0;
		else
		{
			offset = 	fabs(trace_extents[0]*plane->_normal[0]) +
					fabs(trace_extents[1]*plane->_normal[1]) +
					fabs(trace_extents[2]*plane->_normal[2]);
		}
	}


	// see which sides we need to consider
	if(t1 >= offset && t2 >= offset)
	{
		CM_HullCheck_r(node->children[0], p1f, p2f, p1, p2);
		return;
	}
	
	if(t1 < -offset && t2 < -offset)
	{
		CM_HullCheck_r(node->children[1], p1f, p2f, p1, p2);
		return;
	}

	// put the crosspoint CLIP_EPSILON pixels on the near side
	if(t1 < t2)
	{
		idist = 1.0/(t1-t2);
		side = 1;
		frac2 = (t1 + offset + CLIP_EPSILON)*idist;
		frac  = (t1 - offset + CLIP_EPSILON)*idist;
	}
	else if(t1 > t2)
	{
		idist = 1.0/(t1-t2);
		side = 0;
		frac2 = (t1 - offset - CLIP_EPSILON)*idist;
		frac  = (t1 + offset + CLIP_EPSILON)*idist;
	}
	else
	{
		side = 0;
		frac = 1;
		frac2 = 0;
	}

	// move up to the node
	X_clamp(frac, 0, 1);
		
	midf = p1f + (p2f - p1f)*frac;
	
	for(i=0; i<3; i++)
		mid[i] = p1[i] + frac*(p2[i] - p1[i]);

	CM_HullCheck_r(node->children[side], p1f, midf, p1, mid);


	// go past the node
	X_clamp(frac2, 0, 1);
		
	midf = p1f + (p2f - p1f)*frac2;
	
	for(i=0; i<3; i++)
		mid[i] = p1[i] + frac2*(p2[i] - p1[i]);

	CM_HullCheck_r(node->children[side^1], midf, p2f, mid, p2);
}



trace_t	CM_BoxTrace(const vec3_c &start, const vec3_c &end, const cbbox_c &bbox, int headnode, int brushmask)
{
	vec3_c	p;

	cm_checkcount++;		// for multi-check avoidance

	cm_traces++;			// for statistics, may be zeroed

	//
	// fill in a default trace
	//
	memset (&trace_trace, 0, sizeof(trace_trace));
	trace_trace.fraction = 1;
	trace_trace.surface = NULL;//&cm_nullshader;

	if(cm_nodes.empty())	// map not loaded
		return trace_trace;

	trace_contents = brushmask;
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
	if(start == end)
	{
		std::deque<int> leafs;

		cbbox_c		c;

		c._mins = start + bbox._mins;
		c._maxs = start + bbox._maxs;
		
		for(int i=0; i<3; i++)
		{
			c._mins[i] -= 1;
			c._maxs[i] += 1;
		}

		CM_BoxLeafnums(c, leafs, headnode);
		
		for(std::deque<int>::const_iterator ir = leafs.begin(); ir != leafs.end(); ++ir)
		{
			CM_TestInLeaf(*ir);
			
			if(trace_trace.allsolid)
				break;
		}
		
		trace_trace.pos = start;
		
		return trace_trace;
	}

	//
	// check for point special case
	//
	if(bbox.isZero())
	{
		trace_ispoint = true;
		trace_extents.clear();
	}
	else
	{
		trace_ispoint = false;
		trace_extents[0] = -bbox._mins[0] > bbox._maxs[0] ? -bbox._mins[0] : bbox._maxs[0];
		trace_extents[1] = -bbox._mins[1] > bbox._maxs[1] ? -bbox._mins[1] : bbox._maxs[1];
		trace_extents[2] = -bbox._mins[2] > bbox._maxs[2] ? -bbox._mins[2] : bbox._maxs[2];
	}

	//
	// general sweeping through world
	//
	CM_HullCheck_r(headnode, 0, 1, start, end);

	if(trace_trace.fraction == 1)
	{
		trace_trace.pos = end;
	}
	else
	{
		trace_trace.pos = start + ((end - start) * trace_trace.fraction);
	}
	
	return trace_trace;
}



/*
==================
CM_TransformedBoxTrace

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
trace_t	CM_TransformedBoxTrace(const vec3_c &start, const vec3_c &end,
						const cbbox_c &bbox,
						int headnode, int brushmask, 
						const vec3_c &origin, const quaternion_c &quat)
{
	trace_t		trace;
	vec3_c		start_l(false), end_l(false);
	bool	rotated;

	// subtract origin offset
	start_l = start - origin;
	end_l = end - origin;

	// rotate start and end into the models frame of reference
	if(headnode != box_headnode && (quat != quat_identity))
		rotated = true;
	else
		rotated = false;

	if(rotated)
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
	if(cluster >= 0 && cm_pvs.size())
		return &(cm_pvs[cluster * cm_pvs_clusters_size]);
	else
		return cm_nullcluster;
}

static void	CM_FloodAreaConnections()
{
	// area 0 is the void and not considered
	for(uint_t i=1; i<cm_areas.size(); i++)
	{
		for(uint_t j=1; j<cm_areas.size(); j++)
			cm_areas[i].numareaportals[j] = (j == i);
	}

}


int	CM_GetClosestAreaPortal(const vec3_c &p)
{
	return -1;
}

bool	CM_GetAreaPortalState(int portal)
{
	return false;
}

void	CM_SetAreaPortalState(int portal, bool open)
{
/*
	if(area1 > cm_areas_num || area2 > cm_areas_num)
		Com_Error (ERR_DROP, "CM_SetAreaPortalState: areas out of range");

	if(open)
	{
		cm_areas[area1].numareaportals[area2]++;
		cm_areas[area2].numareaportals[area1]++;
	}
	else
	{
		cm_areas[area1].numareaportals[area2]--;
		cm_areas[area2].numareaportals[area1]--;
	}
*/
}

bool	CM_AreasConnected(int area1, int area2)
{
	if(cm_noareas->getValue())
		return true;
	
	if(area1 > (int)cm_areas.size() || area2 > (int)cm_areas.size())
		Com_Error (ERR_DROP, "CM_AreasConnected: areas out of range");

	if(cm_areas[area1].numareaportals[area2])
		return true;
	
	// area 0 is not used
	for(uint_t i=1; i<cm_areas.size(); i++)
	{
		if(cm_areas[i].numareaportals[area1] && cm_areas[i].numareaportals[area2])
			return true;
	}
	
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
void	CM_WriteAreaBits(boost::dynamic_bitset<byte> &bits, int area)
{
	if(cm_noareas->getValue() || area <= 0)
	{	
		// for debugging, send everything
		bits = boost::dynamic_bitset<byte>(cm_areas.size());
		bits.set();
	}
	else
	{
		bits = boost::dynamic_bitset<byte>(cm_areas.size());
		bits.reset();

		// area 0 is the void and should not be visible
		for(uint_t i=1; i<cm_areas.size(); i++)
		{
			if((int)i == area)//|| CM_AreasConnected(i, area))
				bits[i] = true;
		}
	}
}


void	CM_MergeAreaBits(byte *buffer, int area)
{
	// area 0 is not used
	for(uint_t i=0; i<cm_areas.size(); i++)
	{
		if(CM_AreasConnected(i, area) || (int)i == area)
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
}



/*
static void	CM_ClipFragmentToSurface(csurface_t *surf)
{
	//TODO
}

static void	CM_ClipFragmentToLeaf(int leafnum)
{
	cleaf_t		*leaf;
	//cbrush_t	*brush;
	//cpatch_t	*patch;
	csurface_t	*surf;
	cshader_t	*shader;

	leaf = &cm_leafs[leafnum];
	
	// clip fragment against all surfaces in the leaf
	
	for(int i=0; i<leaf->leafsurfaces_num; i++)
	{
		surf = &cm_surfaces[cm_leafsurfaces[leaf->leafsurfaces_first + i]];
		
		if(surf->checkcount == mark_checkcount)
			continue;	// already checked this brush in another leaf
		
		surf->checkcount = mark_checkcount;
		
		shader = &cm_shaders[surf->shader_num];
		
		if(shader->contents & (SURF_NOIMPACT | SURF_NOMARKS))
			continue;
		
		CM_ClipFragmentToSurface(surf);
	}
	

}


static void	CM_MarkFragments_r(int num)
{
	if(mark_vertexes_num >= mark_vertexes_max || mark_fragments_num >= mark_fragments_max)
		return;

	// if < 0, we are in a leaf node
	if(num < 0)
	{
		CM_ClipFragmentToLeaf(-1 - num);
		return;
	}

	cnode_t *node = &cm_nodes[num];
	
	float dist = node->plane->distance(mark_origin);
	
	if(dist > mark_radius)
	{
		CM_MarkFragments_r(node->children[0]);
		return;
	}
	
	if(dist < -mark_radius)
	{
		CM_MarkFragments_r(node->children[1]);
		return;
	}
	
	CM_MarkFragments_r(node->children[0]);
	CM_MarkFragments_r(node->children[1]);
}
*/

/*
int	CM_MarkFragments(const vec3_c &origin, const matrix_c axis, float radius, int vertexes_max, vec3_c *vertexes, int fragments_max, cfragment_t *fragments)
{
	if(!cm_nodes.size())	// map not loaded
		return 0;

	mark_checkcount++;
	
	// initialize fragments
	mark_vertexes_num = 0;
	mark_vertexes_max = vertexes_max;
	mark_vertexes = vertexes;
	
	mark_fragments_num = 0;
	mark_fragments_max = fragments_max;
	mark_fragments = fragments;
	
	mark_origin = origin;
	mark_normal = axis[0];
	mark_radius = radius;
	
	// calculate clipping planes
	for(int i=0; i<3; i++)
	{
		vec3_c normal	= (vec_t*)axis[i];
		vec_t dist	= normal.dotProduct(mark_origin) - radius;
		
		mark_planes[i*2].set(normal, dist);
		
		mark_planes[i*2+1] = mark_planes[i*2];
		mark_planes[i*2+1].negate();
	}
	
	// clip against world geometry
	CM_MarkFragments_r(0);
	
	return mark_fragments_num;
	
	return 0;
}
*/


