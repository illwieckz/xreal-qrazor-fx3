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
#include "cmap.h"


#define	MAX_CM_MODELS			(MAX_BSP_MODELS)
#define	MAX_CM_BRUSHES			(MAX_BSP_BRUSHES<<1)
#define	MAX_CM_ENTSTRING		(MAX_BSP_ENTSTRING)
#define MAX_CM_SHADERS			(MAX_BSP_SHADERS)

#define	MAX_CM_AREAS			(MAX_BSP_AREAS)
#define	MAX_CM_PLANES			(MAX_BSP_PLANES<<2)
#define	MAX_CM_NODES			(MAX_BSP_NODES)
#define	MAX_CM_BRUSHSIDES		(MAX_BSP_BRUSHSIDES<<1)
#define	MAX_CM_LEAFS			(MAX_BSP_LEAFS)
#define	MAX_CM_VERTEXES			(MAX_BSP_VERTEXES)
#define	MAX_CM_SURFACES			(MAX_BSP_SURFACES)
#define	MAX_CM_LEAFSURFACES		(MAX_BSP_LEAFSURFACES)
#define	MAX_CM_LEAFBRUSHES 		(MAX_BSP_LEAFBRUSHES)
#define	MAX_CM_INDEXES			(MAX_BSP_INDEXES)
#define	MAX_CM_VISIBILITY		(MAX_BSP_VISIBILITY)
#define	MAX_CM_HEARABILITY		(MAX_BSP_HEARABILITY)

#define MAX_CM_PATCHES			(0x10000)
#define MAX_CM_PATCH_VERTEXES		(4096)
#define MAX_CM_PATCH_SUBDIVLEVEL	(15)





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

struct cbrushside_t
{
	cplane_c*		plane;
	cshader_t*		shader;
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

struct carea_t
{
	int			numareaportals[MAX_CM_AREAS];
};


static int			cm_checkcount;

static std::string		cm_name;

static int			cm_brushsides_num;
static cbrushside_t		cm_brushsides[MAX_CM_BRUSHSIDES+6];		// extra for box hull

static int			cm_shaders_num;
static cshader_t		cm_shaders[MAX_CM_SHADERS];

static int			cm_planes_num;
static cplane_c			cm_planes[MAX_CM_PLANES+12];			// extra for box hull

static int			cm_nodes_num;
static cnode_t			cm_nodes[MAX_CM_NODES+6];				// extra for box hull

static int			cm_leafs_num = 1;	// allow leaf funcs to be called without a map
static cleaf_t			cm_leafs[MAX_CM_LEAFS];
static int			cm_leaf_empty;
	
static int			cm_leafbrushes_num;
static int			cm_leafbrushes[MAX_CM_LEAFBRUSHES+1];	// extra for box hull

static int			cm_models_num;
static cmodel_t			cm_models[MAX_CM_MODELS];

static int			cm_brushes_num;
static cbrush_t			cm_brushes[MAX_CM_BRUSHES+1];			// extra for box hull

// PHS
//static int			cm_numhearability;	//same as cm_numvisibility
//static byte			cm_hearability[MAX_CM_HEARABILITY];
//static bsp_dvis_t*		cm_phs = (bsp_dvis_t *)cm_hearability;

// PVS
static int			cm_visibility_num;
static byte			cm_visibility[MAX_CM_VISIBILITY];
static bsp_dvis_t*		cm_pvs = (bsp_dvis_t *)cm_visibility;

static int			cm_entitychars_num;
static char			cm_entitystring[MAX_CM_ENTSTRING];

static int			cm_areas_num = 1;
static carea_t			cm_areas[MAX_CM_AREAS];

// NULL
static byte			cm_nullcluster[MAX_CM_LEAFS/8];
static cshader_t		cm_nullshader;

static int			cm_patches_num;
static cpatch_t			cm_patches[MAX_CM_PATCHES];

static int			cm_leafpatches_num;
static int			cm_leafpatches[MAX_CM_LEAFSURFACES];

static int			cm_vertexes_num;
static vec3_c			cm_vertexes[MAX_CM_VERTEXES];

static int			cm_normals_num;
static vec3_c			cm_normals[MAX_CM_VERTEXES];

static int			cm_indexes_num;
static index_t			cm_indexes[MAX_CM_INDEXES];

static int			cm_surfaces_num;
static csurface_t		cm_surfaces[MAX_CM_SURFACES];

//static int			cm_meshes_num;
//static cmesh_t		cm_meshes[MAX_CM_SURFACES];

static int			cm_leafsurfaces_num;
static int			cm_leafsurfaces[MAX_CM_LEAFSURFACES];




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
static cbrush_t*	box_brush;
static cleaf_t*		box_leaf;


static int		leaf_count;
static int		leaf_maxcount;
static int*		leaf_list;
static cbbox_c		leaf_bbox;
static int		leaf_topnode;


// box tracing
// 1/32 epsilon to keep floating point happy
#define	CLIP_EPSILON	(0.03125)

static vec3_c		trace_start;		// replace this by a ray
static vec3_c		trace_end;

static cbbox_c		trace_bbox;
static cbbox_c		trace_bbox_abs;

static vec3_c		trace_extents;

static trace_t		trace_trace;
static int		trace_contents;
static bool		trace_ispoint;		// optimized case

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



static void	CM_InitBoxHull();
static void	CM_FloodAreaConnections();


static void	CM_LoadVertexes(bsp_lump_t *l)
{
	bsp_dvertex_t	*in;
	vec3_c		*out;
	int			i, j, count;
	
	Com_DPrintf("loading vertexes ...\n");
	
	in = (bsp_dvertex_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadVertexes: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadVertexes: BSP with no vertexes");
	
	if(count > MAX_BSP_VERTEXES)
		Com_Error(ERR_DROP, "CM_LoadVertexes: BSP has too many vertexes");

	cm_vertexes_num = count;
	out = cm_vertexes;
	
	for(i=0; i<count; i++, in++)
	{
		for(j=0; j<3; j++)
			out[i][j] = LittleFloat(in->position[j]);
	}
}

static void	CM_LoadNormals(bsp_lump_t *l)
{
	bsp_dvertex_t	*in;
	vec3_c		*out;
	int			i, j, count;
	
	Com_DPrintf("loading normals ...\n");
	
	in = (bsp_dvertex_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadNormals: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadNormals: BSP with no vertexes");
	
	if(count > MAX_BSP_VERTEXES)
		Com_Error(ERR_DROP, "CM_LoadNormals: BSP has too many vertexes");

	cm_normals_num = count;
	out = cm_normals;
	
	for(i=0; i<count; i++, in++)
	{
		for(j=0; j<3; j++)
			out[i][j] = LittleFloat(in->normal[j]);
	}
}


static void	CM_LoadIndexes(bsp_lump_t *l)
{
	int		*in;
	index_t		*out;
	int		i, count;
	
	Com_DPrintf("loading indexes ...\n");
	
	in = (int*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadIndexes: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadIndexes: BSP with no indexes");
	
	if(count > MAX_BSP_INDEXES)
		Com_Error(ERR_DROP, "CM_LoadIndexes: BSP has too many vertexes");

	cm_indexes_num = count;
	out = cm_indexes;
	
	for(i=0; i<count; i++)
	{
		out[i] = LittleLong(in[i]);
	}
}



/*
cmesh_t*	CM_CreateBezierMesh(bsp_dsurface_t *in)
{
	int	step[2];
	int 	size[2];
	int	flat[2];
	int	mesh_cp[2];
	int			i, p, u, v;
	
	// input data
	vec4_t		vertexes[MAX_CM_VERTEXES];
	//vec4_t		normals[MAX_ARRAY_VERTEXES];
		
	// output data
	vec4_t		vertexes2[MAX_CM_VERTEXES];
	//vec4_t		normals2[MAX_ARRAY_VERTEXES];
	
	
	int		vertexes_first;
	int		vertexes_num;
	
	index_t*	indexes;
	
	cmesh_t*	mesh;
	
	int		subdivlevel;

	
	mesh_cp[0] = LittleLong(in->mesh_cp[0]);
	mesh_cp[1] = LittleLong(in->mesh_cp[1]);
	
	if(!mesh_cp[0] || !mesh_cp[1])
		return NULL;
	
	subdivlevel = cm_subdivisions->integer;
	if(subdivlevel < 1)
		subdivlevel = 1;
	
	vertexes_first = LittleLong(in->vertexes_first);
	vertexes_num = LittleLong(in->vertexes_num);
	
	for(i=0; i<vertexes_num; i++)
	{
		cm_vertexes[vertexes_first +i].copyTo(vertexes[i]);
	}
	
	// find degree of subdivision
	Curve_GetFlatness(subdivlevel, vertexes, mesh_cp, flat);
	
	
	// allocate space for mesh
	step[0] = (1 << flat[0]);		//step u
	step[1] = (1 << flat[1]);		//step v
	
	size[0] = (mesh_cp[0] / 2) * step[0] + 1;
	size[1] = (mesh_cp[1] / 2) * step[1] + 1;
	vertexes_num = size[0] * size[1];
	
	if(size[0] * size[1] > MAX_CM_VERTEXES)
		return NULL;
	
	//mesh = &cm_meshes[cm_meshes_num++];
	mesh = new cmesh_t;
	
	// allocate vertexes
	mesh->vertexes_num = vertexes_num;
	mesh->vertexes = new vec3_c[mesh->vertexes_num];
	
	// allocate and fill index table
	mesh->indexes_num = (size[0]-1) * (size[1]-1) * 6;
	mesh->indexes = new index_t[mesh->indexes_num];
	
	indexes = mesh->indexes;
	for(v=0, i=0; v<size[1]-1; v++)
	{
		for(u=0; u<size[0]-1; u++, i+=6, indexes += 6)
		{	
			indexes[0] = p = v * size[0] + u;
			indexes[1] = p + size[0];
			indexes[2] = p + 1;
			indexes[3] = p + 1;
			indexes[4] = p + size[0];
			indexes[5] = p + size[0] + 1;
		}
	}	
	
	// fill in
	Curve_EvalQuadricBezierPatch(vertexes, mesh_cp, step, vertexes2);
		
	for(i=0; i<mesh->vertexes_num; i++)
	{
		mesh->vertexes[i] = vertexes2[i];
	}
	
	return mesh;
}
*/


static void	CM_LoadSurfaces(bsp_lump_t *l)
{
	bsp_dsurface_t		*in;
	csurface_t		*out;
	int			i, count;
	
	Com_DPrintf("loading surfaces ...\n");
	
	in = (bsp_dsurface_t*)(cm_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadSurfaces: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadSurfaces: BSP with no surfaces");
	
	if(count > MAX_BSP_SURFACES)
		Com_Error(ERR_DROP, "CM_LoadSurfaces: BSP has too many surfaces");

	cm_surfaces_num = count;
	out = cm_surfaces;
	
	for(i=0; i<count; i++, in++, out++)
	{
		out->face_type = LittleLong(in->face_type);
	
		out->vertexes_num = LittleLong(in->vertexes_num);
		out->vertexes_first = LittleLong(in->vertexes_first);
	
		out->shader_num = LittleLong(in->shader_num);
		
		out->mesh_cp[0] = LittleLong(in->mesh_cp[0]);
		out->mesh_cp[1] = LittleLong(in->mesh_cp[1]);
																		
		out->mesh = NULL;
		
		
		//
		// create curved surface if needed
		//
		if(out->face_type == BSPST_BEZIER)
		{
			//out->mesh = CM_CreateBezierMesh(in);
		}
		else if(out->face_type != BSPST_FLARE)
		{
			out->mesh = new cmesh_t;
			
			out->mesh->vertexes_num = LittleLong(in->vertexes_num);
			out->mesh->vertexes = cm_vertexes + LittleLong(in->vertexes_first);
			
			out->mesh->normals_num = LittleLong(in->vertexes_num);
			out->mesh->normals = cm_normals + LittleLong(in->vertexes_first);
			
			out->mesh->indexes_num = LittleLong(in->indexes_num);
			out->mesh->indexes = cm_indexes + LittleLong(in->indexes_first);
		}
		else
		{
			//Tr3B don't support flares
			continue;
		}
				
		/*										
		shadernum = LittleLong(in->shader_num);
		
		if(shadernum < 0 || shadernum >= (int)_shaders.size())
			ri.Com_Error(ERR_DROP, "r_bsp_model_c::loadSurfaces: shadernum out of range %i\n", shadernum);
		
		
		shader = _shaders[shadernum];
		
		if(!shader->shader)
		{
			shader->shader = (r_shader_c*)R_LoadShader(shader->name, SHADER_3D);	
		}
		
		out->shaderref = shader;
		
		
		//
		// create bounding box on the fly
		//	
		out->mesh->bbox.clear();
			
		for(std::vector<r_vertex_t*>::const_iterator ir = out->mesh->vertexes.begin(); ir != out->mesh->vertexes.end(); ir++)
		{
			out->mesh->bbox.addPoint((*ir)->position);
		}
		
		//out->mesh->origin = out->mesh->bbox.origin();
		//out->mesh->radius = out->mesh->bbox.radius();
		*/
		
		//
		// setup planar surface
		//
		if(out->face_type == BSPST_PLANAR)
		{
			vec3_c	origin;
			vec3_c	normal;
			float	dist;
			
			for(int j=0; j<3; j++)
			{
				origin[j] = LittleFloat(in->origin[j]);
				normal[j] = LittleFloat(in->normal[j]);
			}
				
			normal.normalize();
	
			dist = normal.dotProduct(origin);
			
			out->plane.set(normal, dist);
		}
	}
}


static void	CM_LoadLeafSurfaces(bsp_lump_t *l)
{
	int				*in;
	int				*out;
	int			i, j, count;
	
	Com_DPrintf("loading leaf surfaces ...\n");
	
	in = (int*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadLeafSurfaces: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadLeafSurfaces: BSP with no faces");
	
	if(count > MAX_BSP_SURFACES)
		Com_Error(ERR_DROP, "CM_LoadLeafSurfaces: BSP has too many faces");

	cm_leafsurfaces_num = count;
	out = cm_leafsurfaces;
	
	for(i=0; i<count; i++)
	{
		j = LittleLong(in[i]);
		
		if(j<0 || j>=cm_surfaces_num)
			Com_Error(ERR_DROP, "CM_LoadLeafSurfaces: bad surface number");
		
		out[i] = j;
	}
}


static void	CM_LoadSubmodels(bsp_lump_t *l)
{
	bsp_dmodel_t		*in;
	cmodel_t		*out;
	cleaf_t			*leaf;
	int			*leafbrush;
	
	int			i, j, count;
	
	Com_DPrintf("loading submodels ...\n");

	in = (bsp_dmodel_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadSubmodels: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadSubmodels: BSP with no models");
	
	if(count > MAX_BSP_MODELS)
		Com_Error(ERR_DROP, "CM_LoadSubmodels: BSP has too many models");

	cm_models_num = count;

	for(i=0; i<count; i++, in++, out++)
	{
		out = &cm_models[i];
		
		out->name = va("*%i", i);
		
		Com_DPrintf("loading '%s'\n", out->name.c_str());
		
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

		for(j=0; j<3; j++)
		{	
			// spread the mins / maxs by a pixel
			out->bbox._mins[j] = LittleFloat (in->mins[j]) - 1;
			out->bbox._maxs[j] = LittleFloat (in->maxs[j]) + 1;
		}
	}
}

static void	CM_LoadShaders(bsp_lump_t *l)
{
	bsp_dshader_t		*in;
	cshader_t		*out;
	int			i, count;
	
	
	Com_DPrintf("loading shaders ...\n");

	in = (bsp_dshader_t*)(cm_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadShaders: funny lump size");
	count = l->filelen / sizeof(*in);
	
	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadShaders: BSP with no shaders");
	
	if(count > MAX_BSP_SHADERS)
		Com_Error(ERR_DROP, "CM_LoadShaders: BSP has too many surfaces");

	cm_shaders_num = count;
	out = cm_shaders;

	for(i=0; i<count; i++, in++, out++)
	{
		out->flags = LittleLong(in->flags);
		out->contents = LittleLong(in->contents);
	}
}


static void	CM_LoadNodes(bsp_lump_t *l)
{
	bsp_dnode_t	*in;
	int			child;
	cnode_t		*out;
	
	int			i, j, count;
	
	Com_DPrintf("loading nodes ...\n");
	
	in = (bsp_dnode_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadNodes: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadNodes: BSP has no nodes");
	
	if(count > MAX_BSP_NODES)
		Com_Error (ERR_DROP, "CM_LoadNodes: BSP has too many nodes");

	cm_nodes_num = count;
	out = cm_nodes;
	
	for(i=0; i<count; i++, out++, in++)
	{
		out->plane = cm_planes + LittleLong(in->plane_num);
		
		for(j=0; j<2; j++)
		{
			child = LittleLong(in->children[j]);
			out->children[j] = child;
		}
	}

}

static void	CM_LoadBrushes(bsp_lump_t *l)
{
	bsp_dbrush_t	*in;
	cbrush_t		*out;
	int			i, count;
	int			shader_num;
	
	Com_DPrintf("loading brushes ...\n");
	
	in = (bsp_dbrush_t*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadBrushes: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count > MAX_BSP_BRUSHES)
		Com_Error(ERR_DROP, "CM_LoadBrushes: BSP has too many brushes");

	cm_brushes_num = count;
	out = cm_brushes;

	for(i=0; i<count; i++, out++, in++)
	{			
		shader_num = LittleLong(in->shader_num);
		out->contents = cm_shaders[shader_num].contents;
		out->sides_first = LittleLong(in->sides_first);
		out->sides_num = LittleLong(in->sides_num);
	}
}



static void	CM_LoadLeafs(bsp_lump_t *l)
{
	bsp_dleaf_t 	*in;
	cleaf_t			*out;
	cbrush_t		*brush;
	int			i, j;
	int			count;
	
	Com_DPrintf("loading leafs ...\n");
	
	in = (bsp_dleaf_t*)(cm_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadLeafs: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadLeafs: BSP with no leafs");
	
	// need to save space for box planes
	if(count > MAX_BSP_PLANES)
		Com_Error(ERR_DROP, "CM_LoadLeafs: BSP has too many planes");

	
	cm_leafs_num = count;
	out = cm_leafs;	
	
	cm_leaf_empty = -1;

	for(i=0; i<count; i++, in++, out++)
	{
		out->contents = 0;
		out->cluster = LittleLong(in->cluster);
		out->area = LittleLong(in->area) + 1;
		
		out->leafsurfaces_first = LittleLong(in->leafsurfaces_first);
		out->leafsurfaces_num = LittleLong(in->leafsurfaces_num);
		
		out->leafbrushes_first = LittleLong(in->leafbrushes_first);
		out->leafbrushes_num = LittleLong(in->leafbrushes_num);
		
		//out->leafpatches_first
		//out->leafpatches_num

		for(j=0; j<out->leafbrushes_num; j++)
		{
			brush = &cm_brushes[cm_leafbrushes[out->leafbrushes_first + j]];
			out->contents |= brush->contents;
		}
		
		if(out->area >= cm_areas_num)
			cm_areas_num = out->area + 1;
		
		if(!out->contents)
			cm_leaf_empty = i;
	}

	if(cm_leaf_empty == -1)
	{
		if(cm_leafs_num >= MAX_BSP_LEAFS-1)
			Com_Error(ERR_DROP, "CM_LoadLeafs: BSP does not have an empty leaf");
		
		out->contents = 0;
		out->cluster = -1;
		out->area = -1;
		
		//out->leafface_first
		//out->leaffaces_num;
		
		out->leafbrushes_first = 0;
		out->leafbrushes_num = 0;
		
		//out->leafpatch_first
		//out->leafpatches_num
		
		Com_DPrintf("CM_LoadLeafs: Forcing an empty leaf %i\n", cm_leafs_num);
		
		cm_leaf_empty = cm_leafs_num++;
	}
}

static void	CM_LoadPlanes(bsp_lump_t *l)
{
	int			i, j;
	bsp_dplane_t 	*in;
	cplane_c		*out;
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
	if(count > MAX_BSP_PLANES)
		Com_Error (ERR_DROP, "CM_LoadPlanes: BSP has too many planes");

	cm_planes_num = count;
	out = cm_planes;	

	for(i=0; i<count; i++, in++, out++)
	{		
		for(j=0; j<3; j++)
		{
			normal[j] = LittleFloat(in->normal[j]);
		}
		
		dist = LittleFloat(in->dist);
		
		out->set(normal, dist);
	}
}

static void	CM_LoadLeafBrushes(bsp_lump_t *l)
{
	int			i;
	int			*out;
	int 			*in;
	int			count;
	
	Com_DPrintf("loading leafbrushes ...\n");
	
	in = (int*)(cm_base + l->fileofs);
	if(l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadLeafBrushes: funny lump size");
	count = l->filelen / sizeof(*in);

	if(count < 1)
		Com_Error(ERR_DROP, "CM_LoadLeafBrushes: BSP with no leafbrushes");
	
	if(count > MAX_BSP_LEAFBRUSHES)
		Com_Error(ERR_DROP, "CM_LoadLeafBrushes: BSP has too many leafbrushes");

	
	cm_leafbrushes_num = count;
	out = cm_leafbrushes;
	
	for(i=0; i<count; i++, in++, out++)
		*out = LittleLong(*in);
}

static void	CM_LoadBrushSides(bsp_lump_t *l)
{
	int			i;
	cbrushside_t		*out;
	bsp_dbrushside_t 	*in;
	int			count;
	int			shader_num;
	
	Com_DPrintf("loading brushsides ...\n");
	
	in = (bsp_dbrushside_t*)(cm_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error(ERR_DROP, "CM_LoadBrushSides: funny lump size");
	count = l->filelen / sizeof(*in);

	if (count > MAX_BSP_BRUSHSIDES)
		Com_Error(ERR_DROP, "CM_LoadBrushSides: BSP has too many brushsides");

	out = cm_brushsides;	
	cm_brushsides_num = count;

	for(i=0; i<count; i++, in++, out++)
	{
		out->plane = cm_planes + LittleLong(in->plane_num);
		
		shader_num = LittleLong(in->shader_num);
		
		if(shader_num >= cm_shaders_num)
			Com_Error(ERR_DROP, "CM_LoadBrushSides: Bad brushside shadernum");
		
		out->shader = &cm_shaders[shader_num];
	}
}



static void	CM_LoadVisibility(bsp_lump_t *l)
{
	Com_DPrintf("loading visibility ...\n");
	
	cm_visibility_num = l->filelen;
	
	if(l->filelen > MAX_BSP_VISIBILITY)
		Com_Error(ERR_DROP, "CM_LoadVisibility: BSP has too large visibility lump");

	memcpy(cm_visibility, ((byte*)cm_base + l->fileofs), l->filelen);

	cm_pvs->clusters_num = LittleLong(cm_pvs->clusters_num);
	cm_pvs->cluster_size = LittleLong(cm_pvs->cluster_size);
}


static void	CM_LoadEntityString(bsp_lump_t *l)
{
	Com_DPrintf("loading entities ...\n");
	
	cm_entitychars_num = l->filelen;
	
	if(l->filelen > MAX_BSP_ENTSTRING)
		Com_Error (ERR_DROP, "CM_LoadEntityString: BSP has too large entity lump");

	memcpy(cm_entitystring, cm_base + l->fileofs, l->filelen);
}


static void	CM_CreateBrush(cbrush_t *brush, vec3_t *vertexes, cshader_t *shader)
{
	int			i, j, k;
	vec3_c			v1, v2;
	cbbox_c			bbox;
	
	cbrushside_t*	side;
	
	int				sign;
	bool			skip[20];
	cplane_c*		plane;
	cplane_c		plane_main;
	
	cplane_c		planes[20];
	int			planes_num = 0;
	
	//Com_DPrintf("creating brush ...\n");
	
	// calc bounds
	bbox.clear();
	for(i=0; i<3; i++)
		bbox.addPoint(vec3_c(vertexes[i]));
	
	plane_main.fromThreePointForm(vertexes[0], vertexes[1], vertexes[2]);
	
	// front plane
	plane = &planes[planes_num++];
	*plane = plane_main;
	
	// back plane
	plane = &planes[planes_num++];
	*plane = plane_main;
	plane->negate();
	
	// axial planes
	for(i=0; i<3; i++)
	{
		for(sign = -1; sign <= 1; sign += 2)
		{
			plane = &planes[planes_num++];
			
			plane->_normal.clear();
			
			plane->_normal[i] = sign;
			plane->_dist = sign > 0 ? bbox._maxs[i] : -bbox._mins[i];
			
			//plane->categorize();
		}
	}
	
	// edge planes
	for(i=0; i<3; i++)
	{
		vec3_c		normal;
		
		Vector3_Copy(vertexes[i], v1);
		Vector3_Copy(vertexes[(i+1)%3], v2);
		
		for(k=0; k<3; k++)
		{
			normal[k] = 0;
			normal[(k+1)%3] =   v1[(k+2)%3] - v2[(k+2)%3];
			normal[(k+2)%3] = -(v1[(k+1)%3] - v2[(k+1)%3]);
			
			if(normal.isZero())
				continue;
			
			plane = &planes[planes_num++];
			
			normal.normalize();
			plane->set(normal, normal.dotProduct(v1));
			
			if(Vector3_DotProduct(vertexes[(i+2)%3], normal) - plane->_dist > 0)
			{
				// invert
				plane->negate();
			}
		}
	}
	
	
	// set plane->type and mark duplicate planes for removal
	for(i=0; i<planes_num; i++)
	{
		skip[i] = false;
		
		for(j=i+1; j<planes_num; j++)
		{
			if((planes[j]._dist == planes[i]._dist) && (planes[j]._normal == planes[i]._normal))
			{
				skip[i] = true;
				break;
			}
		}
	}
	
	
	brush->sides_num = 0;
	brush->sides_first = cm_brushsides_num;
	
	for(k=0; k<2; k++)
	{
		for(i=0; i<planes_num; i++)
		{
			if(skip[i])
				continue;
			
			if(!k && planes[i]._type >= 3)
				continue;
			
			skip[i] = true;
			
			if(cm_planes_num >= MAX_CM_PLANES)
				Com_Error(ERR_DROP, "CM_CreateBrush: cm_numplanes >= MAX_CM_PLANES");
			
			plane = &cm_planes[cm_planes_num++];
			*plane = planes[i];
			
			
			if(cm_brushsides_num >= MAX_CM_BRUSHSIDES)
				Com_Error(ERR_DROP, "CM_CreateBrush: cm_numbrushsides >= MAX_CM_BRUSHSIDES");
			
			side = &cm_brushsides[cm_brushsides_num++];
			side->plane = plane;
			
			if(plane->_normal.dotProduct(plane_main._normal) >= 0)
				side->shader = shader;
			else
				side->shader = NULL;	//don't clip against this side
			
			brush->sides_num++;
		}
	}
}


static void	CM_CreatePatch(cpatch_t *patch, int vertexes_num, vec3_c *vertexes, int *mesh_cp)
{
	int			step[2];
	int 			size[2];
	int			flat[2];
	
	int			i, u, v;
	vec4_t		points[MAX_CM_PATCH_VERTEXES];
	vec4_t		points2[MAX_CM_PATCH_VERTEXES];
	
	//float*		vertex;
	vec3_t		tvertexes[2];
	vec3_t		tvertexes2[2];
	
	cbrush_t*	brush;
	cplane_c	plane;
	
	
	// find degree of subdivision in the u and v directions
	for(i=0; i<vertexes_num; i++)
	{
		points[i][0] = vertexes[i][0];
		points[i][1] = vertexes[i][1];
		points[i][2] = vertexes[i][2];
		points[i][3] = 0;
	}
	
	Curve_GetFlatness(MAX_CM_PATCH_SUBDIVLEVEL, points, mesh_cp, flat);
		
	// fill in sparse mesh control points
	step[0] = (1 << flat[0]);		//step u
	step[1] = (1 << flat[1]);		//step v
	
	size[0] = (mesh_cp[0] / 2) * step[0] + 1;
	size[1] = (mesh_cp[1] / 2) * step[1] + 1;
	
	if(size[0] * size[1] > MAX_CM_PATCH_VERTEXES)
		return;
	
	
	// fill in 
	Curve_EvalQuadricBezierPatch(points, mesh_cp, step, points2);
	
	patch->brushes = brush = cm_brushes + cm_brushes_num;
	patch->brushes_num = 0;
	
	patch->bbox_abs.clear();
	
	
	
	// create a set of brushes
	for(v=0; v<size[1]-1; v++)
	{
		for(u=0; u<size[0]-1; u++)
		{
			if(cm_brushes_num >= MAX_CM_BRUSHES)
				Com_Error(ERR_DROP, "CM_CreatePatch: too many patch brushes");
			
			i = v * size[0] + u;
			
			Vector3_Copy(points2[i], tvertexes[0]);
			Vector3_Copy(points2[i + size[0]], tvertexes[1]);
			Vector3_Copy(points2[i + 1], tvertexes[2]);
			Vector3_Copy(points2[i + size[0] + 1], tvertexes[3]);
			
			for(i=0; i<4; i++)
				patch->bbox_abs.addPoint(tvertexes[i]);
			
			plane.fromThreePointForm(tvertexes[0], tvertexes[1], tvertexes[2]);
			
			
			
			// create two brushes
			CM_CreateBrush(brush, tvertexes, patch->shader);
			brush->contents = patch->shader->contents;
			brush++;
			cm_brushes_num++;
			patch->brushes_num++;
				
				
			Vector3_Copy(tvertexes[2], tvertexes2[0]);
			Vector3_Copy(tvertexes[1], tvertexes2[1]);
			Vector3_Copy(tvertexes[3], tvertexes2[2]);
				
			CM_CreateBrush(brush, tvertexes2, patch->shader);
			brush->contents = patch->shader->contents;
			brush++;
			cm_brushes_num++;
			patch->brushes_num++;
		}
	}
}

static void	CM_CreatePatchesForLeafs()
{
	int			i, j, k;
	cleaf_t		*leaf;
	csurface_t	*face;
	cshader_t	*shader;
	cpatch_t	*patch;
	
	int		checkout[MAX_CM_SURFACES];
	
	Com_DPrintf("creating patches for leafs ...\n");
	
	memset(checkout, -1, sizeof(checkout));
	
	for(i=0, leaf = cm_leafs; i<cm_leafs_num; i++, leaf++)
	{
		leaf->leafpatches_num = 0;
		leaf->leafpatches_first = cm_leafpatches_num;
		
		for (j=0; j<leaf->leafsurfaces_num; j++)
		{
			k = cm_leafsurfaces[leaf->leafsurfaces_first + j];
			face = &cm_surfaces[k];
			
			if(face->face_type != BSPST_BEZIER || face->vertexes_num <= 0)
				continue;
			
			if(face->mesh_cp[0] <= 0 || face->mesh_cp[1] <= 0)
				continue;
			
			if(face->shader_num < 0 || face->shader_num >= cm_shaders_num)
				continue;
			
			shader = &cm_shaders[face->shader_num];
			
			if(!shader->contents || (shader->flags & SURF_NONSOLID))
				continue;
			
			if(cm_leafsurfaces_num >= MAX_CM_LEAFSURFACES)
				Com_Error(ERR_DROP, "CM_CreatePatchesForLeafs: BSP has too many faces");
			
			if(checkout[k] != -1)	//patch was already built
			{
				cm_leafpatches[cm_leafpatches_num] = checkout[k];
				patch = &cm_patches[checkout[k]];
			}
			else
			{
				if(cm_patches_num >= MAX_CM_PATCHES)
					Com_Error(ERR_DROP, "CM_CreatePatchesForLeafs: BSP has too many patches");
				
				patch = &cm_patches[cm_patches_num];
				patch->shader = shader;
				cm_leafpatches[cm_leafpatches_num] = cm_patches_num;
				checkout[k] = cm_patches_num++;
				
				CM_CreatePatch(patch, face->vertexes_num, cm_vertexes + face->vertexes_first, face->mesh_cp);
			}
			
			leaf->contents |= patch->shader->contents;
			leaf->leafpatches_num++;
			
			cm_leafpatches_num++;
		}
	}
	
	Com_DPrintf("created patches\n");
}


static void	CM_CalcPHS()
{
#if 0
	int         clusterbytes, clusterwords;
	int         i, j, k, l, index;
	int         bitbyte;
	unsigned int *dest, *src;
	byte       *scan;
	int         count, vcount;

	Com_Printf("building PHS...\n");
	
	clusterwords = cm_pvs->cluster_size / sizeof(long);
	clusterbytes = cm_pvs->cluster_size;

	memset(cm_phs, 0, MAX_CM_HEARABILITY);
	
	cm_phs->cluster_size = cm_pvs->cluster_size;
	cm_phs->clusters_num = cm_pvs->clusters_num;
	
	vcount = 0;
	for(i=0; i < cm_phs->clusters_num; i++)
	{
		scan = CM_ClusterPVS(i);
				
		for(j=0; j<cm_leafs_num; j++) 
		{
			if(scan[j >> 3] & (1 << (j & 7))) 
			{
				vcount++;
			}
		}
	}

	count = 0;
	scan = (byte*) cm_pvs->visdata;
	dest = (unsigned int*) cm_phs + 8;
	
	for(i=0; i<cm_pvs->clusters_num; i++, dest += clusterwords, scan += clusterbytes) 
	{
		memcpy(dest, scan, clusterbytes);
		
		for(j = 0; j < clusterbytes; j++) 
		{
			bitbyte = scan[j];
			
			if(!bitbyte)
				continue;
			
			for(k = 0; k < 8; k++) 
			{
				if(!(bitbyte & (1 << k)))
					continue;
				
				// or this pvs cluster into the phs
				index = ((j << 3) + k);
				
				if(index >= cm_leafs_num)
					continue;
				
				src = (unsigned int*)((byte*) cm_pvs->visdata) + index*clusterwords;
				
				for(l = 0; l < clusterwords; l++)
					dest[l] |= src[l];
			}
		}
		
		for(j=0; j<cm_phs->clusters_num; j++)
			if(((byte *) dest)[j >> 3] & (1 << (j & 7)))
				count++;
	}

	Com_Printf("average leafs visible / hearable / total: %i / %i / %i\n", vcount/cm_phs->clusters_num, 
										count/cm_phs->clusters_num,
										cm_phs->clusters_num);
#endif
}


void	CM_LoadMap(const std::string &name, bool clientload, unsigned *checksum)
{
	unsigned*		buf;
	int			i;
	bsp_dheader_t		header;
	int			length;
	std::string		full_name = name + ".bsp";
	static unsigned		last_checksum;

	cm_noareas	= Cvar_Get("cm_noareas", "0", CVAR_NONE);
	cm_use_brushes	= Cvar_Get("cm_use_brushes", "1", CVAR_ARCHIVE);
	cm_use_patches	= Cvar_Get("cm_use_patches", "1", CVAR_ARCHIVE);
	cm_use_meshes	= Cvar_Get("cm_use_meshes", "0", CVAR_ARCHIVE);
	cm_subdivisions	= Cvar_Get("cm_subdivisions", "0", CVAR_NONE);

	if(X_strequal(cm_name.c_str(), name.c_str()) && (clientload || !Cvar_VariableValue ("flushmap")))
	{
		*checksum = last_checksum;
		
		if(!clientload)
		{
			CM_FloodAreaConnections();
		}
		return;		// still have the right version
	}

	// free old stuff
	cm_planes_num = 0;
	cm_nodes_num = 0;
	cm_leafs_num = 0;
	cm_models_num = 0;
	cm_visibility_num = 0;
	cm_entitychars_num = 0;
	cm_patches_num = 0;
	cm_leafpatches_num = 0;
	cm_entitystring[0] = 0;
	cm_name[0] = 0;
	
	/*
	for(i=0; i<cm_surfaces_num; i++)
	{
		csurface_t *surf = &cm_surfaces[i];
		
		if(surf->mesh)
		{
			if(surf->face_type == BSPST_BEZIER)
			{
				//delete [] surf->mesh->vertexes;
				//delete [] surf->mesh->indexes;
			}
		
			delete surf->mesh;
			surf->mesh = NULL;
		}
	}
	*/
	
	cm_vertexes_num = 0;
	cm_indexes_num = 0;
	cm_surfaces_num = 0;


	//
	// load the file
	//
	length = VFS_FLoad(full_name, (void **)&buf);
	if (!buf)
		Com_Error(ERR_DROP, "CM_LoadMap: Couldn't load %s", full_name.c_str());

	last_checksum = LittleLong(Com_BlockChecksum (buf, length));
	*checksum = last_checksum;

	header = *(bsp_dheader_t *)buf;
	for(i=0; i<(int)sizeof(bsp_dheader_t)/4; i++)
		((int *)&header)[i] = LittleLong(((int *)&header)[i]);

	if(header.version != BSP_VERSION)
		Com_Error(ERR_DROP, "CM_LoadMap: %s has wrong version number (%i should be %i)", full_name.c_str(), header.version, BSP_VERSION);

	cm_base = (byte*)buf;

	Com_DPrintf("CM_LoadMap: loading %s into heap ...\n", full_name.c_str());
	
	// load into heap
	CM_LoadShaders(&header.lumps[BSP_LUMP_SHADERS]);
	CM_LoadPlanes(&header.lumps[BSP_LUMP_PLANES]);
	CM_LoadLeafBrushes(&header.lumps[BSP_LUMP_LEAFBRUSHES]);
	CM_LoadBrushes(&header.lumps[BSP_LUMP_BRUSHES]);
	CM_LoadBrushSides(&header.lumps[BSP_LUMP_BRUSHSIDES]);
	CM_LoadVertexes(&header.lumps[BSP_LUMP_VERTEXES]);
	CM_LoadNormals(&header.lumps[BSP_LUMP_VERTEXES]);
	CM_LoadIndexes(&header.lumps[BSP_LUMP_INDEXES]);
	CM_LoadSurfaces(&header.lumps[BSP_LUMP_SURFACES]);
	CM_LoadLeafSurfaces(&header.lumps[BSP_LUMP_LEAFSURFACES]);
	CM_LoadLeafs(&header.lumps[BSP_LUMP_LEAFS]);
	CM_LoadNodes(&header.lumps[BSP_LUMP_NODES]);
	CM_LoadSubmodels(&header.lumps[BSP_LUMP_MODELS]);
	CM_LoadVisibility(&header.lumps[BSP_LUMP_VISIBILITY]);
	CM_LoadEntityString(&header.lumps[BSP_LUMP_ENTITIES]);

	VFS_FFree(buf);

	CM_CreatePatchesForLeafs();
	CM_InitBoxHull();
	CM_FloodAreaConnections();
	CM_CalcPHS();
	
	
	memset(cm_nullcluster, 255, sizeof(cm_nullcluster));
	
	cm_name = full_name;
}


cmodel_t*	CM_GetModelByName(const std::string &name)
{
	int		i;
	cmodel_t*	model;

	if(!name.length())
		Com_Error(ERR_DROP, "CM_GetModelByName: empty name");
		
	//Com_DPrintf("CM_GetModelByName: '%s'\n", name.c_str());
	
	//for(std::vector<cmodel_t*>::iterator ir = cm_models.begin(); ir != cm_models.end(); ir++)
	for(i=0, model=cm_models; i<cm_models_num; i++, model++)
	{
		if(X_strequal(model->name.c_str(), name.c_str()))
			return model;
	}
	
	Com_Printf("CM_GetModelByName: couldn't find '%s'\n", name.c_str());
	return NULL;
}

cmodel_t*	CM_GetModelByNum(int num)
{
	if(num < 0 || num >= cm_models_num)
	{
		Com_Error(ERR_DROP, "CM_GetModelbyNum: bad number %i", num);
		return NULL;
	}
	
	return &cm_models[num];
}



int	CM_ClusterSize()
{
	return cm_pvs->cluster_size;// ? cm_pvs->cluster_size : (MAX_CM_LEAFS / 8);
}

int	CM_NumClusters()
{
	return cm_pvs->clusters_num;
}


int	CM_NumModels()
{
	return cm_models_num;
}

const char*	CM_EntityString()
{
	return cm_entitystring;
}


int	CM_LeafContents(int leafnum)
{
	if(leafnum < 0 || leafnum >= cm_leafs_num)
		Com_Error (ERR_DROP, "CM_LeafContents: out of range %i", leafnum);
	
	return cm_leafs[leafnum].contents;
}

int	CM_LeafCluster(int leafnum)
{
	if(leafnum < 0 || leafnum >= cm_leafs_num)
		Com_Error (ERR_DROP, "CM_LeafCluster: out of range %i", leafnum);
	
	return cm_leafs[leafnum].cluster;
}


int	CM_LeafArea(int leafnum)
{
	if(leafnum < 0 || leafnum >= cm_leafs_num)
		Com_Error (ERR_DROP, "CM_LeafArea: out of range %i", leafnum);
	
	return cm_leafs[leafnum].area;
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
	int			i;
	int			side;
	cnode_t		*c;
	cplane_c	*p;
	cbrushside_t	*s;

	box_headnode = cm_nodes_num;
	box_planes = &cm_planes[cm_planes_num];
	
	if(	cm_nodes_num >= MAX_CM_NODES+6 || 
		cm_brushes_num >= MAX_CM_BRUSHES+1 || 
		cm_leafbrushes_num >= MAX_CM_LEAFBRUSHES+1 || 
		cm_brushsides_num >= MAX_CM_BRUSHSIDES+6 || 
		cm_planes_num > MAX_CM_PLANES+12
	)
		Com_Error (ERR_DROP, "CM_InitBoxHull: Not enough room for box tree");

	
	box_brush = &cm_brushes[cm_brushes_num];
	box_brush->sides_num = 6;
	box_brush->sides_first = cm_brushsides_num;
	box_brush->contents = CONTENTS_BODY;

	box_leaf = &cm_leafs[cm_leafs_num];
	box_leaf->contents = CONTENTS_BODY;
	box_leaf->leafbrushes_first = cm_leafbrushes_num;
	box_leaf->leafbrushes_num = 1;

	cm_leafbrushes[cm_leafbrushes_num] = cm_brushes_num;

	for(i=0; i<6; i++)
	{
		side = i&1;

		// brush sides
		s = &cm_brushsides[cm_brushsides_num+i];
		s->plane = cm_planes + (cm_planes_num+i*2+side);
		s->shader = &cm_nullshader;

		// nodes
		c = &cm_nodes[box_headnode+i];
		c->plane = cm_planes + (cm_planes_num+i*2);
		c->children[side] = -1 - cm_leaf_empty;
		
		if(i != 5)
			c->children[side^1] = box_headnode+i + 1;
		else
			c->children[side^1] = -1 - cm_leafs_num;
		
		// planes
		p = &box_planes[i*2];
		p->_type = (plane_type_t)(i>>1);
		p->_normal.clear();
		p->_normal[i>>1] = 1;

		p = &box_planes[i*2+1];
		p->_type = (plane_type_t)(3 + (i>>1));
		p->_normal.clear();
		p->_normal[i>>1] = -1;
	}
#endif	
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
#if 0
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
#endif
	return box_headnode;
}


static int	CM_PointLeafnum_r(const vec3_c &p, int num)
{
	float		d;
	cnode_t		*node;
	cplane_c	*plane;

	while(num >= 0)
	{
		node = cm_nodes + num;
		plane = node->plane;
		
		d = plane->diff(p);
		
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
	if(!cm_planes_num)
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
	cplane_c	*plane;
	cnode_t		*node;
	int		s;

	while(true)
	{
		if(nodenum < 0)
		{
			if(leaf_count >= leaf_maxcount)
			{
				//Com_Printf ("CM_BoxLeafnums_r: overflow\n");
				return;
			}
			
			leaf_list[leaf_count++] = -1 - nodenum;
			return;
		}
	
		node = &cm_nodes[nodenum];
		plane = node->plane;
		
		s = leaf_bbox.onPlaneSide(plane);
		
		if(s == 1)
			nodenum = node->children[0];
		
		else if(s == 2)
			nodenum = node->children[1];
		
		else
		{	// go down both
			if(leaf_topnode == -1)
				leaf_topnode = nodenum;
			
			CM_BoxLeafnums_r(node->children[0]);
			
			nodenum = node->children[1];
		}

	}
}


static int	CM_BoxLeafnums_headnode(const cbbox_c &bbox, int *list, int listsize, int headnode, int *topnode)
{
	leaf_list = list;
	leaf_count = 0;
	leaf_maxcount = listsize;
	
	leaf_bbox = bbox;

	leaf_topnode = -1;

	CM_BoxLeafnums_r(headnode);

	if(topnode)
		*topnode = leaf_topnode;

	return leaf_count;
}


int	CM_BoxLeafnums(const cbbox_c &bbox, int *list, int listsize, int *topnode)
{
	return CM_BoxLeafnums_headnode(bbox, list, listsize, cm_models[0].headnode, topnode);
}



int	CM_PointContents(const vec3_c &p, int headnode)
{
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
}

/*
==================
CM_TransformedPointContents

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
int	CM_TransformedPointContents(const vec3_c &p, int headnode, const vec3_c &origin, const vec3_c &angles)
{
	vec3_c		p_l;
	
	if (!cm_nodes_num)	//map not loaded
		return 0;
	
	// subtract origin offset
	p_l = p - origin;

	// rotate start and end into the models frame of reference
	if(headnode != box_headnode && (angles[0] || angles[1] || angles[2]) )
	{
		p_l.rotate(angles);
	}
	
	return CM_PointContents(p_l, headnode);
}




void	CM_ClipBoxToBrush(const cbbox_c &bbox, const vec3_c &p1, const vec3_c &p2, trace_t *trace, cbrush_t *brush)
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

	if(!brush->sides_num)
		return;

	cm_brush_traces++;

	getout = false;
	startout = false;
	leadside = NULL;

	for(i=0; i<brush->sides_num; i++)
	{
		side = &cm_brushsides[brush->sides_first + i];
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
			d1 = plane->diff(ofs_ext);
			
			ofs_ext = ofs + p2;
			d2 = plane->diff(ofs_ext);	
		}
		else
		{	// special point case
			d1 = plane->diff(p1);
			d2 = plane->diff(p2);
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
		trace->startsolid = true;
		
		if(!getout)
			trace->allsolid = true;
		
		return;
	}
	
	if(enterfrac < leavefrac)
	{
		if(enterfrac > -1 && enterfrac < trace->fraction)
		{
			if(enterfrac < 0)
				enterfrac = 0;
			
			trace->fraction = enterfrac;
			trace->plane = *clipplane;
			trace->surface = leadside->shader;
			trace->contents = brush->contents;
		}
	}
}


void	CM_ClipBoxToPatch(const cbbox_c &bbox, const vec3_c &p1, const vec3_c &p2, trace_t *trace, cbrush_t *brush)
{
	int			i, j;
	cplane_c	*plane, *clipplane;
	float		enterfrac, leavefrac;
	vec3_c		ofs, ofs_ext;
	float		d1, d2;
	bool		startout;
	float		f;
	cbrushside_t	*side, *leadside;

	if(!brush->sides_num)
		return;

	cm_brush_traces++;

	enterfrac = -1;
	leavefrac = 1;
	clipplane = NULL;
	startout = false;
	leadside = NULL;

	for(i=0; i<brush->sides_num; i++)
	{
		side = &cm_brushsides[brush->sides_first + i];
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
			d1 = plane->diff(ofs_ext);
			
			ofs_ext = ofs + p2;
			d2 = plane->diff(ofs_ext);	
		}
		else
		{	// special point case
			d1 = plane->diff(p1);
			d2 = plane->diff(p2);
		}
		
		
		//if (d2 > 0)
		//	getout = true;	// endpoint is not in solid
		
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
			f = (d1 /*+CLIP_EPSILON*/ ) / (d1-d2);
			
			if(f < leavefrac)
				leavefrac = f;
		}
	}

	if(!startout)
	{
		// original point was inside the patch
		return;
	}
	
	if(enterfrac < leavefrac)
	{
		if(leadside && leadside->shader && enterfrac < trace->fraction)
		{
			if(enterfrac < 0)
				enterfrac = 0;
			
			trace->fraction = enterfrac;
			trace->plane = *clipplane;
			trace->surface = leadside->shader;
			trace->contents = brush->contents;
		}
	}
}


void	CM_ClipBoxToMesh(const cbbox_c &bbox, const vec3_c &p1, const vec3_c &p2, trace_t *trace, cmesh_t *mesh, cshader_t *shader)
{
	int		i;//, j;
	cplane_c	*plane, *clipplane;
	float		enterfrac, leavefrac;
	vec3_c		ofs, ofs_ext;
	float		d1, d2;
	bool		getout, startout;
	float		f;
	
	index_t*	index;
	vec3_c		v0, v1, v2;
	vec3_c		n0, n1, n2;
	vec3_c		normal;
	vec_t		dist;
	cplane_c	p;

	enterfrac = -1;
	leavefrac = 1;
	clipplane = NULL;

	if(!mesh->vertexes_num || !mesh->indexes_num)
		return;

	cm_mesh_traces++;

	getout = false;
	startout = false;

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
		
		// FIXME: special case for axial
		if(!trace_ispoint)
		{	// general box case

			// push the plane out apropriately for mins/maxs

			// FIXME: use signbits into 8 way lookup for each mins/maxs
			for(int j=0; j<3; j++)
			{
				if(plane->_normal[j] < 0)
					ofs[j] = bbox._maxs[j];
				else
					ofs[j] = bbox._mins[j];
			}
							
			ofs_ext = ofs + p1; 
			d1 = plane->diff(ofs_ext);
			
			ofs_ext = ofs + p2;
			d2 = plane->diff(ofs_ext);	
		}
		else
		{	// special point case
			d1 = plane->diff(p1);
			d2 = plane->diff(p2);
		}
		
		
		if(d2 > 0.01)
			getout = true;	// endpoint is not in solid
		
		if(d1 > 0.01)
			startout = true;

		// if completely in front of face, no intersection
		if(d1 > 0.01 && d2 >= d1)
			return;

		if(d1 <= 0.01 && d2 <= 0.01)
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
		trace->startsolid = true;
		
		if(!getout)
			trace->allsolid = true;
		
		return;
	}
	
	if(enterfrac < leavefrac)
	{
		if(enterfrac > -1 && enterfrac < trace->fraction)
		{
			if(enterfrac < 0)
				enterfrac = 0;
			
			trace->fraction = enterfrac;
			trace->plane = *clipplane;
			trace->surface = shader;
			trace->contents = shader->contents;
		}
	}
}

void	CM_ClipBoxToSurface(const cbbox_c &bbox, const vec3_c &p1, const vec3_c &p2, trace_t *trace, csurface_t *surf, cshader_t *shader)
{
	cplane_c	*plane, *clipplane;
	float		enterfrac, leavefrac;
	vec3_c		ofs, ofs_ext;
	float		d1, d2;
	bool		getout, startout;
	float		f;

	enterfrac = -1;
	leavefrac = 1;
	clipplane = NULL;

	getout = false;
	startout = false;

	{
		plane = &surf->plane;
		
		// FIXME: special case for axial
		if(!trace_ispoint)
		{	// general box case

			// push the plane out apropriately for mins/maxs

			// FIXME: use signbits into 8 way lookup for each mins/maxs
			for(int j=0; j<3; j++)
			{
				if(plane->_normal[j] < 0)
					ofs[j] = bbox._maxs[j];
				else
					ofs[j] = bbox._mins[j];
			}
							
			ofs_ext = ofs + p1; 
			d1 = plane->diff(ofs_ext);
			
			ofs_ext = ofs + p2;
			d2 = plane->diff(ofs_ext);	
		}
		else
		{	// special point case
			d1 = plane->diff(p1);
			d2 = plane->diff(p2);
		}
		
		
		if(d2 > 0.01)
			getout = true;	// endpoint is not in solid
		
		if(d1 > 0.01)
			startout = true;

		// if completely in front of face, no intersection
		if(d1 > 0.01 && d2 >= d1)
			return;

		//if(d1 <= 0.01 && d2 <= 0.01)
		//	continue;

		// crosses face
		if(d1 > d2)
		{	
			// enter
			f = (d1-CLIP_EPSILON) / (d1-d2);
			
			if(f > enterfrac)
			{
				enterfrac = f;
				clipplane = plane;
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
		trace->startsolid = true;
		
		if(!getout)
			trace->allsolid = true;
		
		return;
	}
	
	if(enterfrac < leavefrac)
	{
		if(enterfrac > -1 && enterfrac < trace->fraction)
		{
			if(enterfrac < 0)
				enterfrac = 0;
			
			trace->fraction = enterfrac;
			trace->plane = *clipplane;
			trace->surface = shader;
			trace->contents = shader->contents;
		}
	}
}



void	CM_TestBoxInBrush(const cbbox_c &bbox, const vec3_c &p1, trace_t *trace, cbrush_t *brush)
{
	int			i, j;
	cplane_c	*plane;
	vec3_c		ofs;
	float		d1;
	cbrushside_t	*side;

	if(!brush->sides_num)
		return;

	for(i=0; i<brush->sides_num; i++)
	{
		side = &cm_brushsides[brush->sides_first + i];
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
		
		d1 = plane->diff(ofs);

		// if completely in front of face, no intersection
		if(d1 > 0)
			return;
	}

	// inside this brush
	trace->startsolid = trace->allsolid = true;
	trace->fraction = 0;
	trace->contents = brush->contents;
}

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

void	CM_TestBoxInSurface(const cbbox_c &bbox, const vec3_c &p1, trace_t *trace, csurface_t *surf, cshader_t *shader)
{
	cplane_c	*plane;
	vec3_c		ofs;
	float		d1;
	

	{
		plane = &surf->plane;

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


static void	CM_TraceToLeaf(int leafnum)
{
	cleaf_t	 *leaf = &cm_leafs[leafnum];
	
	if(!(leaf->contents & trace_contents))
		return;
	
	if(cm_use_brushes->integer)
	{
		// trace line against all brushes in the leaf
		for(int i=0; i<leaf->leafbrushes_num; i++)
		{
			cbrush_t *brush = &cm_brushes[cm_leafbrushes[leaf->leafbrushes_first + i]];
		
			if(brush->checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
		
			brush->checkcount = cm_checkcount;

			if(!(brush->contents & trace_contents))
				continue;
		
			CM_ClipBoxToBrush(trace_bbox, trace_start, trace_end, &trace_trace, brush);
		
			if(!trace_trace.fraction)
				return;
		}
	}
	
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
}


static void	CM_TestInLeaf(int leafnum)
{
	cleaf_t *leaf = &cm_leafs[leafnum];
	
	if(!(leaf->contents & trace_contents))
		return;
	
	if(cm_use_brushes->integer)
	{
		// trace line against all brushes in the leaf
		for(int i=0; i<leaf->leafbrushes_num; i++)
		{
			cbrush_t *brush = &cm_brushes[cm_leafbrushes[leaf->leafbrushes_first + i]];
		
			if(brush->checkcount == cm_checkcount)
				continue;	// already checked this brush in another leaf
		
			brush->checkcount = cm_checkcount;

			if(!(brush->contents & trace_contents))
				continue;
		
			CM_TestBoxInBrush(trace_bbox, trace_start, &trace_trace, brush);
		
			if(!trace_trace.fraction)
				return;
		}
	}
	
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
	node = cm_nodes + num;
	plane = node->plane;

	if(plane->_type < 3)
	{
		t1 = p1[plane->_type] - plane->_dist;
		t2 = p2[plane->_type] - plane->_dist;
		
		offset = trace_extents[plane->_type];
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
	clamp(frac, 0, 1);
		
	midf = p1f + (p2f - p1f)*frac;
	
	for(i=0; i<3; i++)
		mid[i] = p1[i] + frac*(p2[i] - p1[i]);

	CM_HullCheck_r(node->children[side], p1f, midf, p1, mid);


	// go past the node
	clamp(frac2, 0, 1);
		
	midf = p1f + (p2f - p1f)*frac2;
	
	for(i=0; i<3; i++)
		mid[i] = p1[i] + frac2*(p2[i] - p1[i]);

	CM_HullCheck_r(node->children[side^1], midf, p2f, mid, p2);
}



trace_t	CM_BoxTrace(const vec3_c &start, const vec3_c &end, const cbbox_c &bbox, int headnode, int brushmask)
{
	int	i;
	vec3_c	p;

	cm_checkcount++;		// for multi-check avoidance

	cm_traces++;			// for statistics, may be zeroed

	//
	// fill in a default trace
	//
	memset (&trace_trace, 0, sizeof(trace_trace));
	trace_trace.fraction = 1;
	trace_trace.surface = &cm_nullshader;

	if(!cm_nodes_num)	// map not loaded
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
		int		leafs[1024];
		int		leafsnum;
		int		i;
		int		topnode;
		cbbox_c		c;

		c._mins = start + bbox._mins;
		c._maxs = start + bbox._maxs;
		
		for(i=0; i<3; i++)
		{
			c._mins[i] -= 1;
			c._maxs[i] += 1;
		}

		leafsnum = CM_BoxLeafnums_headnode(c, leafs, 1024, headnode, &topnode);
		
		for(i=0; i<leafsnum; i++)
		{
			CM_TestInLeaf(leafs[i]);
			
			if(trace_trace.allsolid)
				break;
		}
		
		trace_trace.endpos = start;
		
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
		trace_trace.endpos = end;
	}
	else
	{
		for(i=0; i<3; i++)
			trace_trace.endpos[i] = start[i] + trace_trace.fraction * (end[i] - start[i]);
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
						const vec3_c &origin, const vec3_c &angles)
{
	trace_t		trace;
	vec3_c		start_l, end_l;
	vec3_c		a;
	bool	rotated;

	// subtract origin offset
	start_l = start - origin;
	end_l = end - origin;

	// rotate start and end into the models frame of reference
	if(headnode != box_headnode && (angles[0] || angles[1] || angles[2]) )
		rotated = true;
	else
		rotated = false;

	if(rotated)
	{
		start_l.rotate(angles);
		end_l.rotate(angles);
	}

	// sweep the box through the model
	trace = CM_BoxTrace(start_l, end_l, bbox, headnode, brushmask);

	if(rotated && trace.fraction != 1.0)
	{
		// FIXME: figure out how to do this with existing angles
		a = angles;
		a.negate();
		trace.plane.rotate(a);
	}

	vec3_c dir = end - start;
	dir *= trace.fraction;
	
	trace.endpos = start + dir;
	
	return trace;
}



byte*	CM_ClusterPVS(int cluster)
{
	if(cluster != -1 && cm_pvs->clusters_num)
		return (byte*) cm_pvs->visdata + cluster * cm_pvs->cluster_size;
	
	return cm_nullcluster;
}

byte*	CM_ClusterPHS(int cluster)
{
#if 0
	if (cluster != -1 && cm_phs->clusters_num)
		return (byte*) cm_phs->visdata + cluster * cm_phs->cluster_size;
#else
	if (cluster != -1 && cm_pvs->clusters_num)
		return (byte*) cm_pvs->visdata + cluster * cm_pvs->cluster_size;
#endif
	return cm_nullcluster;
}


static void	CM_FloodAreaConnections()
{
	int		i, j;
	
	// area 0 is not used
	for(i=1; i<cm_areas_num; i++)
	{
		for (j=1; j<cm_areas_num; j++)
			cm_areas[i].numareaportals[j] = (j == i);
	}

}


void	CM_SetAreaPortalState(int area1, int area2, bool open)
{
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
}

bool	CM_AreasConnected(int area1, int area2)
{
	int		i;
	
	if(cm_noareas->value)
		return true;

	
	if(area1 > cm_areas_num || area2 > cm_areas_num)
		Com_Error (ERR_DROP, "CM_SetAreaPortalState: areas out of range");


	if(cm_areas[area1].numareaportals[area2])
		return true;
	
	
	// area 0 is not used
	for(i=1; i<cm_areas_num; i++)
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
int	CM_WriteAreaBits(byte *buffer, int area)
{
	int		i;
	int		bytes;

	bytes = (cm_areas_num+7)>>3;

	if(cm_noareas->value)
	{	
		// for debugging, send everything
		memset(buffer, 255, bytes);
	}
	else
	{
		memset(buffer, 0, bytes);

		// area 0 is not used
		for(i=1; i<cm_areas_num; i++)
		{
			if(!area || CM_AreasConnected(i, area) || i == area)
				buffer[i>>3] |= 1<<(i&7);
		}
	}

	return bytes;
}


void	CM_MergeAreaBits(byte *buffer, int area)
{
	// area 0 is not used
	for(int i=1; i<cm_areas_num; i++)
	{
		if(CM_AreasConnected (i, area) || i == area)
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

	
	/*
	if(!(leaf->contents & trace_contents))
		return;
	
	// trace line against all brushes in the leaf
	
	for(i=0; i<leaf->leafbrushes_num; i++)
	{
		brush = &cm_brushes[cm_leafbrushes[leaf->leafbrushes_first + i]];
		
		if(brush->checkcount == cm_checkcount)
			continue;	// already checked this brush in another leaf
		
		brush->checkcount = cm_checkcount;

		if(!(brush->contents & trace_contents))
			continue;
		
		CM_ClipBoxToBrush(trace_bbox, trace_start, trace_end, &trace_trace, brush);
		
		if(!trace_trace.fraction)
			return;
	}
	
	if(cm_nocurves->value)
		return;
	
	// trace line against all patches in the leaf
	for(i=0; i<leaf->leafpatches_num; i++)
	{
		patch = &cm_patches[cm_leafpatches[leaf->leafpatches_first + i]];
		
		if(patch->checkcount == cm_checkcount)
			continue;	// already checked this patch in another leaf
		
		patch->checkcount = cm_checkcount;

		if(!(patch->shader->contents & trace_contents))
			continue;
		
		if(!patch->bbox_abs.intersect(trace_bbox_abs))
			continue;
		
		
		for(j=0; j<patch->brushes_num; j++)
		{
			CM_ClipBoxToPatch(trace_bbox, trace_start, trace_end, &trace_trace, &patch->brushes[j]);
			
			if(!trace_trace.fraction)
				return;
		}
	}
	*/	
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
	
	float dist = node->plane->diff(mark_origin);
	
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


int	CM_MarkFragments(const vec3_c &origin, const matrix_c axis, float radius, int vertexes_max, vec3_c *vertexes, int fragments_max, cfragment_t *fragments)
{
	if(!cm_nodes_num)	// map not loaded
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
}



