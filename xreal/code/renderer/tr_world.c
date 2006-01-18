/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "tr_local.h"



/*
=================
R_CullTriSurf

Returns true if the grid is completely culled away.
Also sets the clipped hint bit in tess
=================
*/
static qboolean R_CullTriSurf(srfTriangles_t * cv)
{
	int             boxCull;

	boxCull = R_CullLocalBox(cv->bounds);

	if(boxCull == CULL_OUT)
	{
		return qtrue;
	}
	return qfalse;
}

/*
=================
R_CullGrid

Returns true if the grid is completely culled away.
Also sets the clipped hint bit in tess
=================
*/
static qboolean R_CullGrid(srfGridMesh_t * cv)
{
	int             boxCull;
	int             sphereCull;

	if(r_nocurves->integer)
	{
		return qtrue;
	}

	if(tr.currentEntityNum != ENTITYNUM_WORLD)
	{
		sphereCull = R_CullLocalPointAndRadius(cv->localOrigin, cv->meshRadius);
	}
	else
	{
		sphereCull = R_CullPointAndRadius(cv->localOrigin, cv->meshRadius);
	}
	boxCull = CULL_OUT;

	// check for trivial reject
	if(sphereCull == CULL_OUT)
	{
		tr.pc.c_sphere_cull_patch_out++;
		return qtrue;
	}
	// check bounding box if necessary
	else if(sphereCull == CULL_CLIP)
	{
		tr.pc.c_sphere_cull_patch_clip++;

		boxCull = R_CullLocalBox(cv->meshBounds);

		if(boxCull == CULL_OUT)
		{
			tr.pc.c_box_cull_patch_out++;
			return qtrue;
		}
		else if(boxCull == CULL_IN)
		{
			tr.pc.c_box_cull_patch_in++;
		}
		else
		{
			tr.pc.c_box_cull_patch_clip++;
		}
	}
	else
	{
		tr.pc.c_sphere_cull_patch_in++;
	}

	return qfalse;
}


/*
================
R_CullSurface

Tries to back face cull surfaces before they are lighted or
added to the sorting list.

This will also allow mirrors on both sides of a model without recursion.
================
*/
static qboolean R_CullSurface(surfaceType_t * surface, shader_t * shader)
{
	srfSurfaceFace_t *sface;
	float           d;

	if(r_nocull->integer)
	{
		return qfalse;
	}

	if(*surface == SF_GRID)
	{
		return R_CullGrid((srfGridMesh_t *) surface);
	}

	if(*surface == SF_TRIANGLES)
	{
		return R_CullTriSurf((srfTriangles_t *) surface);
	}

	if(*surface != SF_FACE)
	{
		return qfalse;
	}

	if(shader->cullType == CT_TWO_SIDED)
	{
		return qfalse;
	}

	// face culling
	if(!r_facePlaneCull->integer)
	{
		return qfalse;
	}

	sface = (srfSurfaceFace_t *) surface;
	d = DotProduct(tr.or.viewOrigin, sface->plane.normal);

	// don't cull exactly on the plane, because there are levels of rounding
	// through the BSP, ICD, and hardware that may cause pixel gaps if an
	// epsilon isn't allowed here 
	if(shader->cullType == CT_FRONT_SIDED)
	{
		if(d < sface->plane.dist - 8)
		{
			return qtrue;
		}
	}
	else
	{
		if(d > sface->plane.dist + 8)
		{
			return qtrue;
		}
	}

	return qfalse;
}

// *INDENT-OFF*
static qboolean R_DlightFace(srfSurfaceFace_t * face, trRefDlight_t  * dl)
{
#if 0
	if(	dl->l.origin[0] - dl->l.radius[0] > face->bounds[1][0] ||
		dl->l.origin[0] + dl->l.radius[0] < face->bounds[0][0] ||
		dl->l.origin[1] - dl->l.radius[0] > face->bounds[1][1] ||
		dl->l.origin[1] + dl->l.radius[0] < face->bounds[0][1] ||
		dl->l.origin[2] - dl->l.radius[0] > face->bounds[1][2] ||
		dl->l.origin[2] + dl->l.radius[0] < face->bounds[0][2])
	{
		// dlight doesn't reach the bounds
		return qfalse;
	}
#else
	if(	dl->worldBounds[1][0] < face->bounds[0][0] ||
		dl->worldBounds[1][1] < face->bounds[0][1] ||
		dl->worldBounds[1][2] < face->bounds[0][2] ||
		dl->worldBounds[0][0] > face->bounds[1][0] ||
		dl->worldBounds[0][1] > face->bounds[1][1] ||
		dl->worldBounds[0][2] > face->bounds[1][2])
	{
		return qfalse;
	}
#endif
	
	return qtrue;
}
// *INDENT-ON*

static int R_DlightGrid(srfGridMesh_t * grid, trRefDlight_t * dl)
{
	if(	dl->worldBounds[1][0] < grid->meshBounds[0][0] ||
		dl->worldBounds[1][1] < grid->meshBounds[0][1] ||
		dl->worldBounds[1][2] < grid->meshBounds[0][2] ||
		dl->worldBounds[0][0] > grid->meshBounds[1][0] ||
		dl->worldBounds[0][1] > grid->meshBounds[1][1] ||
		dl->worldBounds[0][2] > grid->meshBounds[1][2])
	{
		// dlight doesn't reach the bounds
		return qfalse;
	}

	return qtrue;
}


static int R_DlightTrisurf(srfTriangles_t * tri, trRefDlight_t * dl)
{
	if(	dl->worldBounds[1][0] < tri->bounds[0][0] ||
		   dl->worldBounds[1][1] < tri->bounds[0][1] ||
		   dl->worldBounds[1][2] < tri->bounds[0][2] ||
		   dl->worldBounds[0][0] > tri->bounds[1][0] ||
		   dl->worldBounds[0][1] > tri->bounds[1][1] ||
		   dl->worldBounds[0][2] > tri->bounds[1][2])
	{
		// dlight doesn't reach the bounds
		return qfalse;
	}

	return qtrue;
}


/*
======================
R_AddInteractionSurface
======================
*/
static void R_AddInteractionSurface(msurface_t * surf, trRefDlight_t * light)
{
	qboolean intersects;
	
	if(surf->viewCount != tr.viewCount)
	{
		return;					// not in this view
	}
	
	if(surf->lightCount == tr.lightCount)
	{
		return;					// already checked this surface
	}
	surf->lightCount = tr.lightCount;
	
	// Tr3B - skip all translucent surfaces that don't matter for lighting only pass
	if(surf->shader->sort > SS_OPAQUE || (surf->shader->surfaceFlags & (SURF_NODLIGHT | SURF_SKY)))
		return;

	if(*surf->data == SF_FACE)
	{
		intersects = R_DlightFace((srfSurfaceFace_t *) surf->data, light);
	}
	else if(*surf->data == SF_GRID)
	{
		intersects = R_DlightGrid((srfGridMesh_t *) surf->data, light);
	}
	else if(*surf->data == SF_TRIANGLES)
	{
		intersects = R_DlightTrisurf((srfTriangles_t *) surf->data, light);
	}
	else
	{
		intersects = qfalse;	
	}
	
	if(intersects)
	{
		R_AddDlightInteraction(light, surf->data, surf->shader, surf->fogIndex);
		tr.pc.c_dlightSurfaces++;
	}
	else
	{
		tr.pc.c_dlightSurfacesCulled++;	
	}
}


/*
======================
R_AddWorldSurface
======================
*/
static void R_AddWorldSurface(msurface_t * surf)
{
	if(surf->viewCount == tr.viewCount)
	{
		return;					// already in this view
	}
	surf->viewCount = tr.viewCount;
	
	// FIXME: bmodel fog?

	// try to cull before dlighting or adding
	if(R_CullSurface(surf->data, surf->shader))
	{
		return;
	}

	R_AddDrawSurf(surf->data, surf->shader, surf->fogIndex);
}

/*
=============================================================

	BRUSH MODELS

=============================================================
*/

/*
=================
R_AddBrushModelSurfaces
=================
*/
void R_AddBrushModelSurfaces(trRefEntity_t * ent)
{
	bmodel_t       *bmodel;
	int             clip;
	model_t        *pModel;
	int             i;

	pModel = R_GetModelByHandle(ent->e.hModel);

	bmodel = pModel->bmodel;

	clip = R_CullLocalBox(bmodel->bounds);
	if(clip == CULL_OUT)
	{
		return;
	}

//	R_DlightBmodel(bmodel);

	for(i = 0; i < bmodel->numSurfaces; i++)
	{
		R_AddWorldSurface(bmodel->firstSurface + i);
	}
}


/*
=============================================================

	WORLD MODEL

=============================================================
*/


/*
================
R_RecursiveWorldNode
================
*/
static void R_RecursiveWorldNode(mnode_t * node, int planeBits)
{
	do
	{
		// if the node wasn't marked as potentially visible, exit
		if(node->visCount != tr.visCount)
		{
			return;
		}

		// if the bounding volume is outside the frustum, nothing
		// inside can be visible OPTIMIZE: don't do this all the way to leafs?
		if(!r_nocull->integer)
		{
			int             i;
			int             r;

			for(i = 0; i < 4; i++)
			{
				if(planeBits & (1 << i))
				{
					r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[i]);
					if(r == 2)
					{
						return;		// culled
					}
					if(r == 1)
					{
						planeBits &= ~(1 << i);	// all descendants will also be in front
					}
				}
			}
		}

		if(node->contents != -1)
		{
			break;
		}

		// recurse down the children, front side first
		R_RecursiveWorldNode(node->children[0], planeBits);

		// tail recurse
		node = node->children[1];
	} while(1);

	{
		// leaf node, so add mark surfaces
		int             c;
		msurface_t     *surf, **mark;

		tr.pc.c_leafs++;

		// add to z buffer bounds
		if(node->mins[0] < tr.viewParms.visBounds[0][0])
		{
			tr.viewParms.visBounds[0][0] = node->mins[0];
		}
		if(node->mins[1] < tr.viewParms.visBounds[0][1])
		{
			tr.viewParms.visBounds[0][1] = node->mins[1];
		}
		if(node->mins[2] < tr.viewParms.visBounds[0][2])
		{
			tr.viewParms.visBounds[0][2] = node->mins[2];
		}

		if(node->maxs[0] > tr.viewParms.visBounds[1][0])
		{
			tr.viewParms.visBounds[1][0] = node->maxs[0];
		}
		if(node->maxs[1] > tr.viewParms.visBounds[1][1])
		{
			tr.viewParms.visBounds[1][1] = node->maxs[1];
		}
		if(node->maxs[2] > tr.viewParms.visBounds[1][2])
		{
			tr.viewParms.visBounds[1][2] = node->maxs[2];
		}

		// add the individual surfaces
		mark = node->firstmarksurface;
		c = node->nummarksurfaces;
		while(c--)
		{
			// the surface may have already been added if it
			// spans multiple leafs
			surf = *mark;
			R_AddWorldSurface(surf);
			mark++;
		}
	}
}

/*
================
R_RecursiveInteractionNode
================
*/
static void R_RecursiveInteractionNode(mnode_t * node, trRefDlight_t * light, int planeBits)
{
	int             i;
	int             r;
	
	// if the node wasn't marked as potentially visible, exit
	if(node->visCount != tr.visCount)
	{
		return;
	}
	
	// light already hit node
	if(node->lightCount == tr.lightCount)
	{
		return;
	}
	node->lightCount = tr.lightCount;

	// if the bounding volume is outside the frustum, nothing
	// inside can be visible OPTIMIZE: don't do this all the way to leafs?
#if 0
	if(!r_nocull->integer)
	{
		for(i = 0; i < 4; i++)
		{
			if(planeBits & (1 << i))
			{
				r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[i]);
				
				if(r == SIDE_BACK)
				{
					return;		// culled
				}
				
				if(r == SIDE_FRONT)
				{
					planeBits &= ~(1 << i);	// all descendants will also be in front
				}
			}
		}
	}
#endif

	if(node->contents != -1)
	{
		// leaf node, so add mark surfaces
		int             c;
		msurface_t     *surf, **mark;

		// add the individual surfaces
		mark = node->firstmarksurface;
		c = node->nummarksurfaces;
		while(c--)
		{
			// the surface may have already been added if it
			// spans multiple leafs
			surf = *mark;
			R_AddInteractionSurface(surf, light);
			mark++;
		}
		return;
	}

	// node is just a decision point, so go down both sides
	// since we don't care about sort orders, just go positive to negative
#if 0
	r = BoxOnPlaneSide(light->worldBounds[0], light->worldBounds[1], node->plane);
	
	switch (r)
	{
		case SIDE_FRONT:
			R_RecursiveInteractionNode(node->children[0], light, planeBits);
			break;
			
		case SIDE_BACK:
			R_RecursiveInteractionNode(node->children[1], light, planeBits);
			break;
		
		case SIDE_ON:
		default:
			// recurse down the children, front side first
			R_RecursiveInteractionNode(node->children[0], light, planeBits);
			R_RecursiveInteractionNode(node->children[1], light, planeBits);
			break;
	}
#else
	// recurse down the children, front side first
	R_RecursiveInteractionNode(node->children[0], light, planeBits);
	R_RecursiveInteractionNode(node->children[1], light, planeBits);
#endif
}


/*
===============
R_PointInLeaf
===============
*/
static mnode_t *R_PointInLeaf(const vec3_t p)
{
	mnode_t        *node;
	float           d;
	cplane_t       *plane;

	if(!tr.world)
	{
		ri.Error(ERR_DROP, "R_PointInLeaf: bad model");
	}

	node = tr.world->nodes;
	while(1)
	{
		if(node->contents != -1)
		{
			break;
		}
		plane = node->plane;
		d = DotProduct(p, plane->normal) - plane->dist;
		if(d > 0)
		{
			node = node->children[0];
		}
		else
		{
			node = node->children[1];
		}
	}

	return node;
}

/*
==============
R_ClusterPVS
==============
*/
static const byte *R_ClusterPVS(int cluster)
{
	if(!tr.world || !tr.world->vis || cluster < 0 || cluster >= tr.world->numClusters)
	{
		return tr.world->novis;
	}

	return tr.world->vis + cluster * tr.world->clusterBytes;
}

/*
=================
R_inPVS
=================
*/
qboolean R_inPVS(const vec3_t p1, const vec3_t p2)
{
	mnode_t        *leaf;
	byte           *vis;

	leaf = R_PointInLeaf(p1);
	vis = CM_ClusterPVS(leaf->cluster);
	leaf = R_PointInLeaf(p2);

	if(!(vis[leaf->cluster >> 3] & (1 << (leaf->cluster & 7))))
	{
		return qfalse;
	}
	return qtrue;
}

/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
static void R_MarkLeaves(void)
{
	const byte     *vis;
	mnode_t        *leaf, *parent;
	int             i;
	int             cluster;

	// lockpvs lets designers walk around to determine the
	// extent of the current pvs
	if(r_lockpvs->integer)
	{
		return;
	}

	// current viewcluster
	leaf = R_PointInLeaf(tr.viewParms.pvsOrigin);
	cluster = leaf->cluster;

	// if the cluster is the same and the area visibility matrix
	// hasn't changed, we don't need to mark everything again

	// if r_showcluster was just turned on, remark everything
	if(tr.viewCluster == cluster && !tr.refdef.areamaskModified && !r_showcluster->modified)
	{
		return;
	}

	if(r_showcluster->modified || r_showcluster->integer)
	{
		r_showcluster->modified = qfalse;
		if(r_showcluster->integer)
		{
			ri.Printf(PRINT_ALL, "cluster:%i  area:%i\n", cluster, leaf->area);
		}
	}

	tr.visCount++;
	tr.viewCluster = cluster;

	if(r_novis->integer || tr.viewCluster == -1)
	{
		for(i = 0; i < tr.world->numnodes; i++)
		{
			if(tr.world->nodes[i].contents != CONTENTS_SOLID)
			{
				tr.world->nodes[i].visCount = tr.visCount;
			}
		}
		return;
	}

	vis = R_ClusterPVS(tr.viewCluster);

	for(i = 0, leaf = tr.world->nodes; i < tr.world->numnodes; i++, leaf++)
	{
		cluster = leaf->cluster;
		if(cluster < 0 || cluster >= tr.world->numClusters)
		{
			continue;
		}

		// check general pvs
		if(!(vis[cluster >> 3] & (1 << (cluster & 7))))
		{
			continue;
		}

		// check for door connection
		if((tr.refdef.areamask[leaf->area >> 3] & (1 << (leaf->area & 7))))
		{
			continue;			// not visible
		}

		parent = leaf;
		do
		{
			if(parent->visCount == tr.visCount)
				break;
			parent->visCount = tr.visCount;
			parent = parent->parent;
		} while(parent);
	}
}


/*
** SetFarClip
*/
static void R_SetFarClip(void)
{
	float           farthestCornerDistance = 0;
	int             i;

	// if not rendering the world (icons, menus, etc)
	// set a 2k far clip plane
	if(tr.refdef.rdflags & RDF_NOWORLDMODEL)
	{
		tr.viewParms.skyFar = 2048;
		return;
	}

	// set far clipping planes dynamically
	farthestCornerDistance = 0;
	for(i = 0; i < 8; i++)
	{
		vec3_t          v;
		vec3_t          vecTo;
		float           distance;

		if(i & 1)
		{
			v[0] = tr.viewParms.visBounds[0][0];
		}
		else
		{
			v[0] = tr.viewParms.visBounds[1][0];
		}

		if(i & 2)
		{
			v[1] = tr.viewParms.visBounds[0][1];
		}
		else
		{
			v[1] = tr.viewParms.visBounds[1][1];
		}

		if(i & 4)
		{
			v[2] = tr.viewParms.visBounds[0][2];
		}
		else
		{
			v[2] = tr.viewParms.visBounds[1][2];
		}

		VectorSubtract(v, tr.viewParms.or.origin, vecTo);

		distance = vecTo[0] * vecTo[0] + vecTo[1] * vecTo[1] + vecTo[2] * vecTo[2];

		if(distance > farthestCornerDistance)
		{
			farthestCornerDistance = distance;
		}
	}
	tr.viewParms.skyFar = sqrt(farthestCornerDistance);
}

/*
=============
R_AddWorldSurfaces
=============
*/
void R_AddWorldSurfaces(void)
{
	if(!r_drawworld->integer)
	{
		return;
	}

	if(tr.refdef.rdflags & RDF_NOWORLDMODEL)
	{
		return;
	}

	tr.currentEntityNum = ENTITYNUM_WORLD;
	tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;

	// determine which leaves are in the PVS / areamask
	R_MarkLeaves();

	// clear out the visible min/max
	ClearBounds(tr.viewParms.visBounds[0], tr.viewParms.visBounds[1]);

	// perform frustum culling and add all the potentially visible surfaces
	R_RecursiveWorldNode(tr.world->nodes, 15);
	
	// dynamically compute far clip plane distance for sky
	R_SetFarClip();
}



/*
=============
R_AddWorldInteractions
=============
*/
void R_AddWorldInteractions(trRefDlight_t * light)
{
	if(!r_drawworld->integer)
	{
		return;
	}

	if(tr.refdef.rdflags & RDF_NOWORLDMODEL)
	{
		return;
	}

	tr.currentEntityNum = ENTITYNUM_WORLD;
	tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;

	// perform frustum culling and add all the potentially visible surfaces
	tr.lightCount++;
	R_RecursiveInteractionNode(tr.world->nodes, light, 15);
}
