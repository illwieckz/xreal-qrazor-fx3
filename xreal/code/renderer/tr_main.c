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
// tr_main.c -- main control flow for each frame

#include "tr_local.h"

trGlobals_t     tr;

const float     s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};


refimport_t     ri;

// entities that will have procedurally generated surfaces will just
// point at this for their sorting surface
surfaceType_t   entitySurface = SF_ENTITY;



/*
=============
R_CalcNormalForTriangle
=============
*/
void R_CalcNormalForTriangle(vec3_t normal, const vec3_t v0, const vec3_t v1, const vec3_t v2)
{
	vec3_t          udir, vdir;
	
	// compute the face normal based on vertex points
	VectorSubtract(v2, v0, udir);
	VectorSubtract(v1, v0, vdir);
	CrossProduct(udir, vdir, normal);

	VectorNormalizeFast(normal);
}

/*
=============
R_CalcTangentsForTriangle
http://members.rogers.com/deseric/tangentspace.htm
=============
*/
void R_CalcTangentsForTriangle(vec3_t tangent, vec3_t binormal,
							   const vec3_t v0, const vec3_t v1, const vec3_t v2,
							   const vec2_t t0, const vec2_t t1, const vec2_t t2)
{
	int             i;
	vec3_t          planes[3];
	vec3_t          e0;
	vec3_t          e1;

	for(i = 0; i < 3; i++)
	{
		VectorSet(e0, v1[i] - v0[i], t1[0] - t0[0], t1[1] - t0[1]);
		VectorSet(e1, v2[i] - v0[i], t2[0] - t0[0], t2[1] - t0[1]);

		CrossProduct(e0, e1, planes[i]);
	}

	//So your tangent space will be defined by this :
	//Normal = Normal of the triangle or Tangent X Binormal (careful with the cross product, 
	// you have to make sure the normal points in the right direction)
	//Tangent = ( dp(Fx(s,t)) / ds,  dp(Fy(s,t)) / ds, dp(Fz(s,t)) / ds )   or     ( -Bx/Ax, -By/Ay, - Bz/Az )
	//Binormal =  ( dp(Fx(s,t)) / dt,  dp(Fy(s,t)) / dt, dp(Fz(s,t)) / dt )  or     ( -Cx/Ax, -Cy/Ay, -Cz/Az )

	// tangent...
	tangent[0] = -planes[0][1] / planes[0][0];
	tangent[1] = -planes[1][1] / planes[1][0];
	tangent[2] = -planes[2][1] / planes[2][0];
	VectorNormalizeFast(tangent);

	// binormal...
	binormal[0] = -planes[0][2] / planes[0][0];
	binormal[1] = -planes[1][2] / planes[1][0];
	binormal[2] = -planes[2][2] / planes[2][0];
	VectorNormalizeFast(binormal);

#if 0
	// normal...
	// compute the cross product TxB
	CrossProduct(tangent, binormal, normal);
	VectorNormalize(normal);

	// Gram-Schmidt orthogonalization process for B
	// compute the cross product B=NxT to obtain 
	// an orthogonal basis
	CrossProduct(normal, tangent, binormal);

	if(DotProduct(normal, n) < 0)
	{
		VectorInverse(normal);
	}

//	VectorCopy(n, normal);
#endif
}

/*
=============
R_CalcTangentSpace
Tr3B - recoded from Nvidia's SDK
=============
*/
void R_CalcTangentSpace(vec3_t tangent, vec3_t binormal, vec3_t normal,
						const vec3_t v0, const vec3_t v1, const vec3_t v2,
						const vec2_t t0, const vec2_t t1, const vec2_t t2,
						const vec3_t n)
{

	vec3_t          cp, e0, e1;

	VectorSet(e0, v1[0] - v0[0], t1[0] - t0[0], t1[1] - t0[1]);
	VectorSet(e1, v2[0] - v0[0], t2[0] - t0[0], t2[1] - t0[1]);

	CrossProduct(e0, e1, cp);

	if(Q_fabs(cp[0]) > 10e-6)
	{
		tangent[0] = -cp[1] / cp[0];
		binormal[0] = -cp[2] / cp[0];
	}

	e0[0] = v1[1] - v0[1];
	e1[0] = v2[1] - v0[1];

	CrossProduct(e0, e1, cp);
	if(Q_fabs(cp[0]) > 10e-6)
	{
		tangent[1] = -cp[1] / cp[0];
		binormal[1] = -cp[2] / cp[0];
	}

	e0[0] = v1[2] - v0[2];
	e1[0] = v2[2] - v0[2];

	CrossProduct(e0, e1, cp);
	if(Q_fabs(cp[0]) > 10e-6)
	{
		tangent[2] = -cp[1] / cp[0];
		binormal[2] = -cp[2] / cp[0];
	}

	VectorNormalizeFast(tangent);
	VectorNormalizeFast(binormal);

	// normal...
	// compute the cross product TxB
	CrossProduct(tangent, binormal, normal);
	VectorNormalizeFast(normal);

	// Gram-Schmidt orthogonalization process for B
	// compute the cross product B=NxT to obtain 
	// an orthogonal basis
	CrossProduct(normal, tangent, binormal);

	if(DotProduct(normal, n) < 0)
	{
		VectorInverse(normal);
	}
}

/*
=================
R_CullLocalBox

Returns CULL_IN, CULL_CLIP, or CULL_OUT
=================
*/
int R_CullLocalBox(vec3_t bounds[2])
{
	int             i, j;
	vec3_t          transformed[8];
	float           dists[8];
	vec3_t          v;
	cplane_t       *frust;
	int             anyBack;
	int             front, back;

	if(r_nocull->integer)
	{
		return CULL_CLIP;
	}

	// transform into world space
	for(i = 0; i < 8; i++)
	{
		v[0] = bounds[i & 1][0];
		v[1] = bounds[(i >> 1) & 1][1];
		v[2] = bounds[(i >> 2) & 1][2];
		
		R_LocalPointToWorld(v, transformed[i]);
	}

	// check against frustum planes
	anyBack = 0;
	for(i = 0; i < 4; i++)
	{
		frust = &tr.viewParms.frustum[i];

		front = back = 0;
		for(j = 0; j < 8; j++)
		{
			dists[j] = DotProduct(transformed[j], frust->normal);
			if(dists[j] > frust->dist)
			{
				front = 1;
				if(back)
				{
					break;		// a point is in front
				}
			}
			else
			{
				back = 1;
			}
		}
		if(!front)
		{
			// all points were behind one of the planes
			return CULL_OUT;
		}
		anyBack |= back;
	}

	if(!anyBack)
	{
		return CULL_IN;			// completely inside frustum
	}

	return CULL_CLIP;			// partially clipped
}


/*
** R_CullLocalPointAndRadius
*/
int R_CullLocalPointAndRadius(vec3_t pt, float radius)
{
	vec3_t          transformed;

	R_LocalPointToWorld(pt, transformed);

	return R_CullPointAndRadius(transformed, radius);
}


/*
** R_CullPointAndRadius
*/
int R_CullPointAndRadius(vec3_t pt, float radius)
{
	int             i;
	float           dist;
	cplane_t       *frust;
	qboolean        mightBeClipped = qfalse;

	if(r_nocull->integer)
	{
		return CULL_CLIP;
	}

	// check against frustum planes
	for(i = 0; i < 4; i++)
	{
		frust = &tr.viewParms.frustum[i];

		dist = DotProduct(pt, frust->normal) - frust->dist;
		if(dist < -radius)
		{
			return CULL_OUT;
		}
		else if(dist <= radius)
		{
			mightBeClipped = qtrue;
		}
	}

	if(mightBeClipped)
	{
		return CULL_CLIP;
	}

	return CULL_IN;				// completely inside frustum
}



/*
=================
R_LocalNormalToWorld
=================
*/
void R_LocalNormalToWorld(vec3_t local, vec3_t world)
{
	MatrixTransformNormal(tr.or.transformMatrix, local, world);
}

/*
=================
R_LocalPointToWorld
=================
*/
void R_LocalPointToWorld(vec3_t local, vec3_t world)
{
	MatrixTransformPoint(tr.or.transformMatrix, local, world);
}


/*
==========================
R_TransformWorldToClip
==========================
*/
void R_TransformWorldToClip(const vec3_t src, const float *cameraViewMatrix, const float *projectionMatrix, vec4_t eye, vec4_t dst)
{
	vec4_t			src2;
	VectorCopy(src, src2);
	src2[3] = 1;
	
	MatrixTransform4(cameraViewMatrix, src2, eye);

	MatrixTransform4(projectionMatrix, eye, dst);
}

/*
==========================
R_TransformModelToClip
==========================
*/
void R_TransformModelToClip(const vec3_t src, const float *modelViewMatrix, const float *projectionMatrix, vec4_t eye, vec4_t dst)
{
	vec4_t			src2;
	VectorCopy(src, src2);
	src2[3] = 1;
	
	MatrixTransform4(modelViewMatrix, src2, eye);

	MatrixTransform4(projectionMatrix, eye, dst);
}

/*
==========================
R_TransformClipToWindow
==========================
*/
void R_TransformClipToWindow(const vec4_t clip, const viewParms_t * view, vec4_t normalized, vec4_t window)
{
	normalized[0] = clip[0] / clip[3];
	normalized[1] = clip[1] / clip[3];
	normalized[2] = (clip[2] + clip[3]) / (2 * clip[3]);

	window[0] = 0.5f * (1.0f + normalized[0]) * view->viewportWidth;
	window[1] = 0.5f * (1.0f + normalized[1]) * view->viewportHeight;
	window[2] = normalized[2];

	window[0] = (int)(window[0] + 0.5);
	window[1] = (int)(window[1] + 0.5);
}

/*
==========================
R_TransformClipToWindow2
==========================
*/
void R_TransformClipToWindow2(const vec4_t clip, const viewParms_t * view, vec4_t normalized, vec4_t window)
{
	normalized[0] = clip[0] / clip[3];
	normalized[1] = clip[1] / clip[3];
	normalized[2] = (clip[2] + clip[3]) / (2 * clip[3]);

	window[0] = view->viewportX + (0.5f * (1.0f + normalized[0]) * view->viewportWidth);
	window[1] = view->viewportY + (0.5f * (1.0f + normalized[1]) * view->viewportHeight);
	window[2] = normalized[2];

	window[0] = (int)(window[0] + 0.5);
	window[1] = (int)(window[1] + 0.5);
}


/*
=================
R_SetupEntityWorldBounds
Tr3B - needs R_RotateForEntity
=================
*/
void R_SetupEntityWorldBounds(trRefEntity_t * ent)
{
	int             j;
	vec3_t          v, transformed;
	
	ClearBounds(ent->worldBounds[0], ent->worldBounds[1]);
		
	for(j = 0; j < 8; j++)
	{
		v[0] = ent->localBounds[j & 1][0];
		v[1] = ent->localBounds[(j >> 1) & 1][1];
		v[2] = ent->localBounds[(j >> 2) & 1][2];
	
		// transform local bounds vertices into world space
		R_LocalPointToWorld(v, transformed);
			
		AddPointToBounds(transformed, ent->worldBounds[0], ent->worldBounds[1]);
	}
}


/*
=================
R_RotateForEntity

Generates an orientation for an entity and viewParms
Does NOT produce any GL calls
Called by both the front end and the back end
=================
*/
void R_RotateForEntity(const trRefEntity_t * ent, const viewParms_t * viewParms, orientationr_t * or)
{
	vec3_t          delta;
	float           axisLength;

	if(ent->e.reType != RT_MODEL)
	{
		*or = viewParms->world;
		return;
	}

	VectorCopy(ent->e.origin, or->origin);

	VectorCopy(ent->e.axis[0], or->axis[0]);
	VectorCopy(ent->e.axis[1], or->axis[1]);
	VectorCopy(ent->e.axis[2], or->axis[2]);

	MatrixSetupTransform(or->transformMatrix, or->axis[0], or->axis[1], or->axis[2], or->origin);
	MatrixAffineInverse(or->transformMatrix, or->viewMatrix);
	MatrixMultiply(viewParms->world.viewMatrix, or->transformMatrix, or->modelViewMatrix);

	// calculate the viewer origin in the model's space
	// needed for fog, specular, and environment mapping
	VectorSubtract(viewParms->or.origin, or->origin, delta);

	// compensate for scale in the axes if necessary
	if(ent->e.nonNormalizedAxes)
	{
		axisLength = VectorLength(ent->e.axis[0]);
		if(!axisLength)
		{
			axisLength = 0;
		}
		else
		{
			axisLength = 1.0f / axisLength;
		}
	}
	else
	{
		axisLength = 1.0f;
	}

	or->viewOrigin[0] = DotProduct(delta, or->axis[0]) * axisLength;
	or->viewOrigin[1] = DotProduct(delta, or->axis[1]) * axisLength;
	or->viewOrigin[2] = DotProduct(delta, or->axis[2]) * axisLength;
}


/*
=================
R_RotateForDlight
=================
*/
void R_RotateForDlight(const trRefDlight_t * light, const viewParms_t * viewParms, orientationr_t * or)
{
	vec3_t          delta;
	float           axisLength;

	VectorCopy(light->l.origin, or->origin);

	VectorCopy(light->l.axis[0], or->axis[0]);
	VectorCopy(light->l.axis[1], or->axis[1]);
	VectorCopy(light->l.axis[2], or->axis[2]);

	MatrixSetupTransform(or->transformMatrix, or->axis[0], or->axis[1], or->axis[2], or->origin);
	MatrixAffineInverse(or->transformMatrix, or->viewMatrix);
	MatrixMultiply(viewParms->world.viewMatrix, or->transformMatrix, or->modelViewMatrix);

	// calculate the viewer origin in the light's space
	// needed for fog, specular, and environment mapping
	VectorSubtract(viewParms->or.origin, or->origin, delta);

	// compensate for scale in the axes if necessary
	if(light->l.nonNormalizedAxes)
	{
		axisLength = VectorLength(light->l.axis[0]);
		if(!axisLength)
		{
			axisLength = 0;
		}
		else
		{
			axisLength = 1.0f / axisLength;
		}
	}
	else
	{
		axisLength = 1.0f;
	}

	or->viewOrigin[0] = DotProduct(delta, or->axis[0]) * axisLength;
	or->viewOrigin[1] = DotProduct(delta, or->axis[1]) * axisLength;
	or->viewOrigin[2] = DotProduct(delta, or->axis[2]) * axisLength;
}


/*
=================
R_RotateForViewer

Sets up the modelview matrix for a given viewParm
=================
*/
void R_RotateForViewer(void)
{
	matrix_t        transformMatrix;
	matrix_t        viewMatrix;

	Com_Memset(&tr.or, 0, sizeof(tr.or));
	tr.or.axis[0][0] = 1;
	tr.or.axis[1][1] = 1;
	tr.or.axis[2][2] = 1;
	VectorCopy(tr.viewParms.or.origin, tr.or.viewOrigin);

	// transform by the camera placement
	MatrixSetupTransform(transformMatrix,
						 tr.viewParms.or.axis[0], tr.viewParms.or.axis[1], tr.viewParms.or.axis[2], tr.viewParms.or.origin);

	MatrixAffineInverse(transformMatrix, viewMatrix);
//	MatrixAffineInverse(transformMatrix, tr.or.viewMatrix);

	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	MatrixIdentity(tr.or.transformMatrix);
	MatrixMultiply(s_flipMatrix, viewMatrix, tr.or.viewMatrix);
	MatrixCopy(tr.or.viewMatrix, tr.or.modelViewMatrix);

	tr.viewParms.world = tr.or;
}


/*
===============
R_SetupProjection
===============
*/
// *INDENT-OFF*
void R_SetupProjection(void)
{
	float           xMin, xMax, yMin, yMax;
	float           width, height, depth;
	float           zNear, zFar;

//	matrix_t        proj;
	float          *proj = tr.viewParms.projectionMatrix;

	// set up projection matrix
	zNear = r_znear->value;
	zFar = r_zfar->value;

	yMax = zNear * tan(tr.refdef.fov_y * M_PI / 360.0f);
	yMin = -yMax;

	xMax = zNear * tan(tr.refdef.fov_x * M_PI / 360.0f);
	xMin = -xMax;

	width = xMax - xMin;
	height = yMax - yMin;
	depth = zFar - zNear;

	// Tr3B - far plane at infinity, see RobustShadowVolumes.pdf by Nvidia
	proj[0] = 2 * zNear / width;	proj[4] = 0;					proj[8] = (xMax + xMin) / width;	proj[12] = 0;
	proj[1] = 0;					proj[5] = 2 * zNear / height;	proj[9] = (yMax + yMin) / height;	proj[13] = 0;
	proj[2] = 0;					proj[6] = 0;					proj[10] = -1;						proj[14] = -2 * zNear;
	proj[3] = 0;					proj[7] = 0;					proj[11] = -1;						proj[15] = 0;

	if(zFar > zNear)
	{
		proj[10] = -(zFar + zNear) / depth;
		proj[14] = -2 * zFar * zNear / depth;
	}
	
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
//	MatrixMultiply(proj, s_flipMatrix, tr.viewParms.projectionMatrix);
}
// *INDENT-ON*

/*
=================
R_SetupFrustum

Setup that culling frustum planes for the current view
=================
*/
void R_SetupFrustum(void)
{
	int             i;
	float           xs, xc;
	float           ang;

	ang = tr.viewParms.fovX / 180 * M_PI * 0.5f;
	xs = sin(ang);
	xc = cos(ang);

	VectorScale(tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[0].normal);
	VectorMA(tr.viewParms.frustum[0].normal, xc, tr.viewParms.or.axis[1], tr.viewParms.frustum[0].normal);

	VectorScale(tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[1].normal);
	VectorMA(tr.viewParms.frustum[1].normal, -xc, tr.viewParms.or.axis[1], tr.viewParms.frustum[1].normal);

	ang = tr.viewParms.fovY / 180 * M_PI * 0.5f;
	xs = sin(ang);
	xc = cos(ang);

	VectorScale(tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[2].normal);
	VectorMA(tr.viewParms.frustum[2].normal, xc, tr.viewParms.or.axis[2], tr.viewParms.frustum[2].normal);

	VectorScale(tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[3].normal);
	VectorMA(tr.viewParms.frustum[3].normal, -xc, tr.viewParms.or.axis[2], tr.viewParms.frustum[3].normal);

	for(i = 0; i < 4; i++)
	{
		tr.viewParms.frustum[i].type = PLANE_NON_AXIAL;
		tr.viewParms.frustum[i].dist = DotProduct(tr.viewParms.or.origin, tr.viewParms.frustum[i].normal);
		SetPlaneSignbits(&tr.viewParms.frustum[i]);
	}
	
	// Tr3B - set extra near plane
	tr.viewParms.frustum[4].type = PLANE_NON_AXIAL;
	VectorCopy(tr.viewParms.or.axis[0], tr.viewParms.frustum[4].normal);
	tr.viewParms.frustum[4].dist = r_znear->value;
	SetPlaneSignbits(&tr.viewParms.frustum[4]);
}



/*
=================
R_MirrorPoint
=================
*/
void R_MirrorPoint(vec3_t in, orientation_t * surface, orientation_t * camera, vec3_t out)
{
	int             i;
	vec3_t          local;
	vec3_t          transformed;
	float           d;

	VectorSubtract(in, surface->origin, local);

	VectorClear(transformed);
	for(i = 0; i < 3; i++)
	{
		d = DotProduct(local, surface->axis[i]);
		VectorMA(transformed, d, camera->axis[i], transformed);
	}

	VectorAdd(transformed, camera->origin, out);
}

void R_MirrorVector(vec3_t in, orientation_t * surface, orientation_t * camera, vec3_t out)
{
	int             i;
	float           d;

	VectorClear(out);
	for(i = 0; i < 3; i++)
	{
		d = DotProduct(in, surface->axis[i]);
		VectorMA(out, d, camera->axis[i], out);
	}
}



/*
=============
R_PlaneForSurface
=============
*/
void R_PlaneForSurface(surfaceType_t * surfType, cplane_t * plane)
{
	srfTriangles_t *tri;
	srfPoly_t      *poly;
	srfVert_t      *v1, *v2, *v3;
	vec4_t          plane4;

	if(!surfType)
	{
		Com_Memset(plane, 0, sizeof(*plane));
		plane->normal[0] = 1;
		return;
	}
	switch (*surfType)
	{
		case SF_FACE:
			*plane = ((srfSurfaceFace_t *) surfType)->plane;
			return;
		case SF_TRIANGLES:
			tri = (srfTriangles_t *) surfType;
			v1 = tri->verts + tri->indexes[0];
			v2 = tri->verts + tri->indexes[1];
			v3 = tri->verts + tri->indexes[2];
			PlaneFromPoints(plane4, v1->xyz, v2->xyz, v3->xyz, qtrue);
			VectorCopy(plane4, plane->normal);
			plane->dist = plane4[3];
			return;
		case SF_POLY:
			poly = (srfPoly_t *) surfType;
			PlaneFromPoints(plane4, poly->verts[0].xyz, poly->verts[1].xyz, poly->verts[2].xyz, qtrue);
			VectorCopy(plane4, plane->normal);
			plane->dist = plane4[3];
			return;
		default:
			Com_Memset(plane, 0, sizeof(*plane));
			plane->normal[0] = 1;
			return;
	}
}


/*
=================
R_GetPortalOrientation

entityNum is the entity that the portal surface is a part of, which may
be moving and rotating.

Returns qtrue if it should be mirrored
=================
*/
qboolean R_GetPortalOrientations(drawSurf_t * drawSurf, int entityNum,
								 orientation_t * surface, orientation_t * camera, vec3_t pvsOrigin, qboolean * mirror)
{
	int             i;
	cplane_t        originalPlane, plane;
	trRefEntity_t  *e;
	float           d;
	vec3_t          transformed;

	// create plane axis for the portal we are seeing
	R_PlaneForSurface(drawSurf->surface, &originalPlane);

	// rotate the plane if necessary
	if(entityNum != ENTITYNUM_WORLD)
	{
		tr.currentEntityNum = entityNum;
		tr.currentEntity = &tr.refdef.entities[entityNum];

		// get the orientation of the entity
		R_RotateForEntity(tr.currentEntity, &tr.viewParms, &tr.or);

		// rotate the plane, but keep the non-rotated version for matching
		// against the portalSurface entities
		R_LocalNormalToWorld(originalPlane.normal, plane.normal);
		plane.dist = originalPlane.dist + DotProduct(plane.normal, tr.or.origin);

		// translate the original plane
		originalPlane.dist = originalPlane.dist + DotProduct(originalPlane.normal, tr.or.origin);
	}
	else
	{
		plane = originalPlane;
	}

	VectorCopy(plane.normal, surface->axis[0]);
	PerpendicularVector(surface->axis[1], surface->axis[0]);
	CrossProduct(surface->axis[0], surface->axis[1], surface->axis[2]);

	// locate the portal entity closest to this plane.
	// origin will be the origin of the portal, origin2 will be
	// the origin of the camera
	for(i = 0; i < tr.refdef.numEntities; i++)
	{
		e = &tr.refdef.entities[i];
		if(e->e.reType != RT_PORTALSURFACE)
		{
			continue;
		}

		d = DotProduct(e->e.origin, originalPlane.normal) - originalPlane.dist;
		if(d > 64 || d < -64)
		{
			continue;
		}

		// get the pvsOrigin from the entity
		VectorCopy(e->e.oldorigin, pvsOrigin);

		// if the entity is just a mirror, don't use as a camera point
		if(e->e.oldorigin[0] == e->e.origin[0] && e->e.oldorigin[1] == e->e.origin[1] && e->e.oldorigin[2] == e->e.origin[2])
		{
			VectorScale(plane.normal, plane.dist, surface->origin);
			VectorCopy(surface->origin, camera->origin);
			VectorSubtract(vec3_origin, surface->axis[0], camera->axis[0]);
			VectorCopy(surface->axis[1], camera->axis[1]);
			VectorCopy(surface->axis[2], camera->axis[2]);

			*mirror = qtrue;
			return qtrue;
		}

		// project the origin onto the surface plane to get
		// an origin point we can rotate around
		d = DotProduct(e->e.origin, plane.normal) - plane.dist;
		VectorMA(e->e.origin, -d, surface->axis[0], surface->origin);

		// now get the camera origin and orientation
		VectorCopy(e->e.oldorigin, camera->origin);
		AxisCopy(e->e.axis, camera->axis);
		VectorSubtract(vec3_origin, camera->axis[0], camera->axis[0]);
		VectorSubtract(vec3_origin, camera->axis[1], camera->axis[1]);

		// optionally rotate
		if(e->e.oldframe)
		{
			// if a speed is specified
			if(e->e.frame)
			{
				// continuous rotate
				d = (tr.refdef.time / 1000.0f) * e->e.frame;
				VectorCopy(camera->axis[1], transformed);
				RotatePointAroundVector(camera->axis[1], camera->axis[0], transformed, d);
				CrossProduct(camera->axis[0], camera->axis[1], camera->axis[2]);
			}
			else
			{
				// bobbing rotate, with skinNum being the rotation offset
				d = sin(tr.refdef.time * 0.003f);
				d = e->e.skinNum + d * 4;
				VectorCopy(camera->axis[1], transformed);
				RotatePointAroundVector(camera->axis[1], camera->axis[0], transformed, d);
				CrossProduct(camera->axis[0], camera->axis[1], camera->axis[2]);
			}
		}
		else if(e->e.skinNum)
		{
			d = e->e.skinNum;
			VectorCopy(camera->axis[1], transformed);
			RotatePointAroundVector(camera->axis[1], camera->axis[0], transformed, d);
			CrossProduct(camera->axis[0], camera->axis[1], camera->axis[2]);
		}
		*mirror = qfalse;
		return qtrue;
	}

	// if we didn't locate a portal entity, don't render anything.
	// We don't want to just treat it as a mirror, because without a
	// portal entity the server won't have communicated a proper entity set
	// in the snapshot

	// unfortunately, with local movement prediction it is easily possible
	// to see a surface before the server has communicated the matching
	// portal surface entity, so we don't want to print anything here...

	//ri.Printf( PRINT_ALL, "Portal surface without a portal entity\n" );

	return qfalse;
}


static qboolean IsMirror(const drawSurf_t * drawSurf, int entityNum)
{
	int             i;
	cplane_t        originalPlane, plane;
	trRefEntity_t  *e;
	float           d;

	// create plane axis for the portal we are seeing
	R_PlaneForSurface(drawSurf->surface, &originalPlane);

	// rotate the plane if necessary
	if(entityNum != ENTITYNUM_WORLD)
	{
		tr.currentEntityNum = entityNum;
		tr.currentEntity = &tr.refdef.entities[entityNum];

		// get the orientation of the entity
		R_RotateForEntity(tr.currentEntity, &tr.viewParms, &tr.or);

		// rotate the plane, but keep the non-rotated version for matching
		// against the portalSurface entities
		R_LocalNormalToWorld(originalPlane.normal, plane.normal);
		plane.dist = originalPlane.dist + DotProduct(plane.normal, tr.or.origin);

		// translate the original plane
		originalPlane.dist = originalPlane.dist + DotProduct(originalPlane.normal, tr.or.origin);
	}
	else
	{
		plane = originalPlane;
	}

	// locate the portal entity closest to this plane.
	// origin will be the origin of the portal, origin2 will be
	// the origin of the camera
	for(i = 0; i < tr.refdef.numEntities; i++)
	{
		e = &tr.refdef.entities[i];
		if(e->e.reType != RT_PORTALSURFACE)
		{
			continue;
		}

		d = DotProduct(e->e.origin, originalPlane.normal) - originalPlane.dist;
		if(d > 64 || d < -64)
		{
			continue;
		}

		// if the entity is just a mirror, don't use as a camera point
		if(e->e.oldorigin[0] == e->e.origin[0] && e->e.oldorigin[1] == e->e.origin[1] && e->e.oldorigin[2] == e->e.origin[2])
		{
			return qtrue;
		}

		return qfalse;
	}
	return qfalse;
}

/*
** SurfIsOffscreen
**
** Determines if a surface is completely offscreen.
*/
static qboolean SurfIsOffscreen(const drawSurf_t * drawSurf, vec4_t clipDest[128])
{
	float           shortest = 100000000;
	int             entityNum;
	int             numTriangles;
	shader_t       *shader;
	int             fogNum;
	vec4_t          clip, eye;
	int             i;
	unsigned int    pointOr = 0;
	unsigned int    pointAnd = (unsigned int)~0;

	if(glConfig.smpActive)
	{							// FIXME!  we can't do RB_BeginSurface/RB_EndSurface stuff with smp!
		return qfalse;
	}

	R_RotateForViewer();

	R_DecomposeSort(drawSurf->sort, &entityNum, &shader, &fogNum);
	
	RB_BeginSurface(shader, NULL, fogNum, qfalse, 0, NULL);
	rb_surfaceTable[*drawSurf->surface] (drawSurf->surface);

	assert(tess.numVertexes < 128);

	for(i = 0; i < tess.numVertexes; i++)
	{
		int             j;
		unsigned int    pointFlags = 0;

		R_TransformModelToClip(tess.xyz[i], tr.or.modelViewMatrix, tr.viewParms.projectionMatrix, eye, clip);

		for(j = 0; j < 3; j++)
		{
			if(clip[j] >= clip[3])
			{
				pointFlags |= (1 << (j * 2));
			}
			else if(clip[j] <= -clip[3])
			{
				pointFlags |= (1 << (j * 2 + 1));
			}
		}
		pointAnd &= pointFlags;
		pointOr |= pointFlags;
	}

	// trivially reject
	if(pointAnd)
	{
		return qtrue;
	}

	// determine if this surface is backfaced and also determine the distance
	// to the nearest vertex so we can cull based on portal range.  Culling
	// based on vertex distance isn't 100% correct (we should be checking for
	// range to the surface), but it's good enough for the types of portals
	// we have in the game right now.
	numTriangles = tess.numIndexes / 3;

	for(i = 0; i < tess.numIndexes; i += 3)
	{
		vec3_t          normal;
		float           dot;
		float           len;

		VectorSubtract(tess.xyz[tess.indexes[i]], tr.viewParms.or.origin, normal);

		len = VectorLengthSquared(normal);	// lose the sqrt
		if(len < shortest)
		{
			shortest = len;
		}

		if((dot = DotProduct(normal, tess.normals[tess.indexes[i]])) >= 0)
		{
			numTriangles--;
		}
	}
	if(!numTriangles)
	{
		return qtrue;
	}

	// mirrors can early out at this point, since we don't do a fade over distance
	// with them (although we could)
	if(IsMirror(drawSurf, entityNum))
	{
		return qfalse;
	}

	if(shortest > (tess.surfaceShader->portalRange * tess.surfaceShader->portalRange))
	{
		return qtrue;
	}

	return qfalse;
}


/*
========================
R_MirrorViewBySurface

Returns qtrue if another view has been rendered
========================
*/
qboolean R_MirrorViewBySurface(drawSurf_t * drawSurf, int entityNum)
{
	vec4_t          clipDest[128];
	viewParms_t     newParms;
	viewParms_t     oldParms;
	orientation_t   surface, camera;

	// don't recursively mirror
	if(tr.viewParms.isPortal)
	{
		ri.Printf(PRINT_DEVELOPER, "WARNING: recursive mirror/portal found\n");
		return qfalse;
	}

	if(r_noportals->integer || (r_fastsky->integer == 1))
	{
		return qfalse;
	}

	// trivially reject portal/mirror
	if(SurfIsOffscreen(drawSurf, clipDest))
	{
		return qfalse;
	}

	// save old viewParms so we can return to it after the mirror view
	oldParms = tr.viewParms;

	newParms = tr.viewParms;
	newParms.isPortal = qtrue;
	if(!R_GetPortalOrientations(drawSurf, entityNum, &surface, &camera, newParms.pvsOrigin, &newParms.isMirror))
	{
		return qfalse;			// bad portal, no portalentity
	}

	R_MirrorPoint(oldParms.or.origin, &surface, &camera, newParms.or.origin);

	VectorSubtract(vec3_origin, camera.axis[0], newParms.portalPlane.normal);
	newParms.portalPlane.dist = DotProduct(camera.origin, newParms.portalPlane.normal);

	R_MirrorVector(oldParms.or.axis[0], &surface, &camera, newParms.or.axis[0]);
	R_MirrorVector(oldParms.or.axis[1], &surface, &camera, newParms.or.axis[1]);
	R_MirrorVector(oldParms.or.axis[2], &surface, &camera, newParms.or.axis[2]);

	// OPTIMIZE: restrict the viewport on the mirrored view

	// render the mirror view
	R_RenderView(&newParms);

	tr.viewParms = oldParms;

	return qtrue;
}

/*
=================
R_SpriteFogNum

See if a sprite is inside a fog volume
=================
*/
int R_SpriteFogNum(trRefEntity_t * ent)
{
	int             i, j;
	fog_t          *fog;

	if(tr.refdef.rdflags & RDF_NOWORLDMODEL)
	{
		return 0;
	}

	for(i = 1; i < tr.world->numfogs; i++)
	{
		fog = &tr.world->fogs[i];
		for(j = 0; j < 3; j++)
		{
			if(ent->e.origin[j] - ent->e.radius >= fog->bounds[1][j])
			{
				break;
			}
			if(ent->e.origin[j] + ent->e.radius <= fog->bounds[0][j])
			{
				break;
			}
		}
		if(j == 3)
		{
			return i;
		}
	}

	return 0;
}


/*
==========================================================================================

DRAWSURF SORTING

==========================================================================================
*/

/*
=================
qsort replacement

=================
*/
#define	SWAP_DRAW_SURF(a,b) temp=((int *)a)[0];((int *)a)[0]=((int *)b)[0];((int *)b)[0]=temp; temp=((int *)a)[1];((int *)a)[1]=((int *)b)[1];((int *)b)[1]=temp;

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

#define CUTOFF 8				/* testing shows that this is good value */

static void shortsort(drawSurf_t * lo, drawSurf_t * hi)
{
	drawSurf_t     *p, *max;
	int             temp;

	while(hi > lo)
	{
		max = lo;
		for(p = lo + 1; p <= hi; p++)
		{
			if(p->sort > max->sort)
			{
				max = p;
			}
		}
		SWAP_DRAW_SURF(max, hi);
		hi--;
	}
}



/* sort the array between lo and hi (inclusive)
FIXME: this was lifted and modified from the microsoft lib source...
 */

void qsortFast(void *base, unsigned num, unsigned width)
{
	char           *lo, *hi;	/* ends of sub-array currently sorting */
	char           *mid;		/* points to middle of subarray */
	char           *loguy, *higuy;	/* traveling pointers for partition step */
	unsigned        size;		/* size of the sub-array */
	char           *lostk[30], *histk[30];
	int             stkptr;		/* stack for saving sub-array to be processed */
	int             temp;

	if(sizeof(drawSurf_t) != 8)
	{
		ri.Error(ERR_DROP, "change SWAP_DRAW_SURF macro");
	}

	/* Note: the number of stack entries required is no more than
	   1 + log2(size), so 30 is sufficient for any array */

	if(num < 2 || width == 0)
		return;					/* nothing to do */

	stkptr = 0;					/* initialize stack */

	lo = base;
	hi = (char *)base + width * (num - 1);	/* initialize limits */

	/* this entry point is for pseudo-recursion calling: setting
	   lo and hi and jumping to here is like recursion, but stkptr is
	   prserved, locals aren't, so we preserve stuff on the stack */
  recurse:

	size = (hi - lo) / width + 1;	/* number of el's to sort */

	/* below a certain size, it is faster to use a O(n^2) sorting method */
	if(size <= CUTOFF)
	{
		shortsort((drawSurf_t *) lo, (drawSurf_t *) hi);
	}
	else
	{
		/* First we pick a partititioning element.  The efficiency of the
		   algorithm demands that we find one that is approximately the
		   median of the values, but also that we select one fast.  Using
		   the first one produces bad performace if the array is already
		   sorted, so we use the middle one, which would require a very
		   wierdly arranged array for worst case performance.  Testing shows
		   that a median-of-three algorithm does not, in general, increase
		   performance. */

		mid = lo + (size / 2) * width;	/* find middle element */
		SWAP_DRAW_SURF(mid, lo);	/* swap it to beginning of array */

		/* We now wish to partition the array into three pieces, one
		   consisiting of elements <= partition element, one of elements
		   equal to the parition element, and one of element >= to it.  This
		   is done below; comments indicate conditions established at every
		   step. */

		loguy = lo;
		higuy = hi + width;

		/* Note that higuy decreases and loguy increases on every iteration,
		   so loop must terminate. */
		for(;;)
		{
			/* lo <= loguy < hi, lo < higuy <= hi + 1,
			   A[i] <= A[lo] for lo <= i <= loguy,
			   A[i] >= A[lo] for higuy <= i <= hi */

			do
			{
				loguy += width;
			} while(loguy <= hi && (((drawSurf_t *) loguy)->sort <= ((drawSurf_t *) lo)->sort));

			/* lo < loguy <= hi+1, A[i] <= A[lo] for lo <= i < loguy,
			   either loguy > hi or A[loguy] > A[lo] */

			do
			{
				higuy -= width;
			} while(higuy > lo && (((drawSurf_t *) higuy)->sort >= ((drawSurf_t *) lo)->sort));

			/* lo-1 <= higuy <= hi, A[i] >= A[lo] for higuy < i <= hi,
			   either higuy <= lo or A[higuy] < A[lo] */

			if(higuy < loguy)
				break;

			/* if loguy > hi or higuy <= lo, then we would have exited, so
			   A[loguy] > A[lo], A[higuy] < A[lo],
			   loguy < hi, highy > lo */

			SWAP_DRAW_SURF(loguy, higuy);

			/* A[loguy] < A[lo], A[higuy] > A[lo]; so condition at top
			   of loop is re-established */
		}

		/*     A[i] >= A[lo] for higuy < i <= hi,
		   A[i] <= A[lo] for lo <= i < loguy,
		   higuy < loguy, lo <= higuy <= hi
		   implying:
		   A[i] >= A[lo] for loguy <= i <= hi,
		   A[i] <= A[lo] for lo <= i <= higuy,
		   A[i] = A[lo] for higuy < i < loguy */

		SWAP_DRAW_SURF(lo, higuy);	/* put partition element in place */

		/* OK, now we have the following:
		   A[i] >= A[higuy] for loguy <= i <= hi,
		   A[i] <= A[higuy] for lo <= i < higuy
		   A[i] = A[lo] for higuy <= i < loguy    */

		/* We've finished the partition, now we want to sort the subarrays
		   [lo, higuy-1] and [loguy, hi].
		   We do the smaller one first to minimize stack usage.
		   We only sort arrays of length 2 or more. */

		if(higuy - 1 - lo >= hi - loguy)
		{
			if(lo + width < higuy)
			{
				lostk[stkptr] = lo;
				histk[stkptr] = higuy - width;
				++stkptr;
			}					/* save big recursion for later */

			if(loguy < hi)
			{
				lo = loguy;
				goto recurse;	/* do small recursion */
			}
		}
		else
		{
			if(loguy < hi)
			{
				lostk[stkptr] = loguy;
				histk[stkptr] = hi;
				++stkptr;		/* save big recursion for later */
			}

			if(lo + width < higuy)
			{
				hi = higuy - width;
				goto recurse;	/* do small recursion */
			}
		}
	}

	/* We have sorted the array, except for any pending sorts on the stack.
	   Check if there are any, and do them. */

	--stkptr;
	if(stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto recurse;			/* pop subarray from stack */
	}
	else
		return;					/* all subarrays done */
}


//==========================================================================================

/*
=================
R_AddDrawSurf
=================
*/
void R_AddDrawSurf(surfaceType_t * surface, shader_t * shader, int fogIndex)
{
	int             index;

	// instead of checking for overflow, we just mask the index
	// so it wraps around
	index = tr.refdef.numDrawSurfs & DRAWSURF_MASK;
	// the sort data is packed into a single 32 bit value so it can be
	// compared quickly during the qsorting process
	tr.refdef.drawSurfs[index].sort = (shader->sortedIndex << QSORT_SHADERNUM_SHIFT)
		| tr.shiftedEntityNum | (fogIndex << QSORT_FOGNUM_SHIFT) | (int)0;
	tr.refdef.drawSurfs[index].surface = surface;
	tr.refdef.numDrawSurfs++;
}


/*
=================
R_DecomposeSort
=================
*/
void R_DecomposeSort(unsigned sort, int *entityNum, shader_t ** shader, int *fogNum)
{
	*fogNum = (sort >> QSORT_FOGNUM_SHIFT) & 31;
	*shader = tr.sortedShaders[(sort >> QSORT_SHADERNUM_SHIFT) & (MAX_SHADERS - 1)];
	*entityNum = (sort >> QSORT_ENTITYNUM_SHIFT) & 1023;
//	*dlightMap = sort & 3;
}


/*
=================
R_SortDrawSurfs
=================
*/
void R_SortDrawSurfs(drawSurf_t * drawSurfs, int numDrawSurfs, interaction_t * interactions, int numInteractions)
{
	shader_t       *shader;
	int             fogNum;
	int             entityNum;
	int             i;

	// it is possible for some views to not have any surfaces
	if(numDrawSurfs < 1)
	{
		// we still need to add it for hyperspace cases
		R_AddDrawSurfCmd(drawSurfs, numDrawSurfs, interactions, numInteractions);
		return;
	}

	// if we overflowed MAX_DRAWSURFS, the drawsurfs
	// wrapped around in the buffer and we will be missing
	// the first surfaces, not the last ones
	if(numDrawSurfs > MAX_DRAWSURFS)
	{
		numDrawSurfs = MAX_DRAWSURFS;
	}
	
	// if we overflowed MAX_INTERACTIONS, the interactions
	// wrapped around in the buffer and we will be missing
	// the first interactions, not the last ones
	if(numInteractions > MAX_INTERACTIONS)
	{
		interaction_t  *ia;
		
		numInteractions = numInteractions;
		
		// reset last interaction's next pointer
		ia = &interactions[numInteractions -1];
		ia->next = NULL;
	}

	// sort the drawsurfs by sort type, then orientation, then shader
	qsortFast(drawSurfs, numDrawSurfs, sizeof(drawSurf_t));

	// check for any pass through drawing, which
	// may cause another view to be rendered first
	for(i = 0; i < numDrawSurfs; i++)
	{
		R_DecomposeSort((drawSurfs + i)->sort, &entityNum, &shader, &fogNum);

		if(shader->sort > SS_PORTAL)
		{
			break;
		}

		// no shader should ever have this sort type
		if(shader->sort == SS_BAD)
		{
			ri.Error(ERR_DROP, "Shader '%s'with sort == SS_BAD", shader->name);
		}

		// if the mirror was completely clipped away, we may need to check another surface
		if(R_MirrorViewBySurface((drawSurfs + i), entityNum))
		{
			// this is a debug option to see exactly what is being mirrored
			if(r_portalOnly->integer)
			{
				return;
			}
			break;				// only one mirror view at a time
		}
	}

	R_AddDrawSurfCmd(drawSurfs, numDrawSurfs, interactions, numInteractions);
}


/*
=============
R_AddEntitySurfaces
=============
*/
void R_AddEntitySurfaces(void)
{
	trRefEntity_t  *ent;
	shader_t       *shader;

	if(!r_drawentities->integer)
	{
		return;
	}

	for(tr.currentEntityNum = 0; tr.currentEntityNum < tr.refdef.numEntities; tr.currentEntityNum++)
	{
		ent = tr.currentEntity = &tr.refdef.entities[tr.currentEntityNum];

		// preshift the value we are going to OR into the drawsurf sort
		tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;

		//
		// the weapon model must be handled special --
		// we don't want the hacked weapon position showing in 
		// mirrors, because the true body position will already be drawn
		//
		if((ent->e.renderfx & RF_FIRST_PERSON) && tr.viewParms.isPortal)
		{
			continue;
		}

		// determine if we need zfail algorithm instead of zpass
#if 0
		if(ent->e.renderfx & RF_THIRD_PERSON)
		{
			if(r_shadows->integer == 3 && tr.viewParms.isPortal)
			{
				ent->needZFail = qtrue;
			}
		}
		else
		{
			ent->needZFail = qfalse;
		}
#else
		ent->needZFail = qtrue;
#endif

		// simple generated models, like sprites and beams, are not culled
		switch (ent->e.reType)
		{
			case RT_PORTALSURFACE:
				break;			// don't draw anything
			case RT_SPRITE:
			case RT_BEAM:
			case RT_LIGHTNING:
			case RT_RAIL_CORE:
			case RT_RAIL_RINGS:
				// self blood sprites, talk balloons, etc should not be drawn in the primary
				// view.  We can't just do this check for all entities, because md3
				// entities may still want to cast shadows from them
				if((ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal)
				{
					continue;
				}
				shader = R_GetShaderByHandle(ent->e.customShader);
				R_AddDrawSurf(&entitySurface, shader, R_SpriteFogNum(ent));
				break;

			case RT_MODEL:
				// we must set up parts of tr.or for model culling
				R_RotateForEntity(ent, &tr.viewParms, &tr.or);

				tr.currentModel = R_GetModelByHandle(ent->e.hModel);
				if(!tr.currentModel)
				{
					R_AddDrawSurf(&entitySurface, tr.defaultShader, 0);
				}
				else
				{
					switch (tr.currentModel->type)
					{
						case MOD_MD3:
							R_AddMD3Surfaces(ent);
							break;

						case MOD_MDS:
							R_AddMDSSurfaces(ent);
							break;
							
						case MOD_MD5:
							R_AddMD5Surfaces(ent);
							break;

						case MOD_BRUSH:
							R_AddBrushModelSurfaces(ent);
							break;

						case MOD_BAD:	// null model axis
							if((ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal)
							{
								break;
							}
							shader = R_GetShaderByHandle(ent->e.customShader);
							R_AddDrawSurf(&entitySurface, tr.defaultShader, 0);
							break;

						default:
							ri.Error(ERR_DROP, "R_AddEntitySurfaces: Bad modeltype");
							break;
					}
				}
				break;

			default:
				ri.Error(ERR_DROP, "R_AddEntitySurfaces: Bad reType");
		}
	}
}


/*
=============
R_AddEntityInteractions
=============
*/
void R_AddEntityInteractions(trRefDlight_t * light)
{
	trRefEntity_t  *ent;

	if(!r_drawentities->integer)
	{
		return;
	}

	for(tr.currentEntityNum = 0; tr.currentEntityNum < tr.refdef.numEntities; tr.currentEntityNum++)
	{
		ent = tr.currentEntity = &tr.refdef.entities[tr.currentEntityNum];

		// preshift the value we are going to OR into the drawsurf sort
		tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;

		//
		// the weapon model must be handled special --
		// we don't want the hacked weapon position showing in 
		// mirrors, because the true body position will already be drawn
		//
		if((ent->e.renderfx & RF_FIRST_PERSON) && tr.viewParms.isPortal)
		{
			continue;
		}

		// simple generated models, like sprites and beams, are not culled
		switch (ent->e.reType)
		{
			case RT_PORTALSURFACE:
				break;			// don't draw anything
			case RT_SPRITE:
			case RT_BEAM:
			case RT_LIGHTNING:
			case RT_RAIL_CORE:
			case RT_RAIL_RINGS:
				break;

			case RT_MODEL:
				tr.currentModel = R_GetModelByHandle(ent->e.hModel);
				if(!tr.currentModel)
				{
					//R_AddDrawSurf(&entitySurface, tr.defaultShader, 0);
				}
				else
				{
					switch (tr.currentModel->type)
					{
						case MOD_MD3:
							R_AddMD3Interactions(ent, light);
							break;

						case MOD_MDS:
							//R_AddMDSSurfaces(ent);
							break;
							
						case MOD_MD5:
							R_AddMD5Interactions(ent, light);
							break;

						case MOD_BRUSH:
							R_AddBrushModelInteractions(ent, light);
							break;

						case MOD_BAD:	// null model axis
							break;

						default:
							ri.Error(ERR_DROP, "R_AddEntityInteractions: Bad modeltype");
							break;
					}
				}
				break;

			default:
				ri.Error(ERR_DROP, "R_AddEntityInteractions: Bad reType");
		}
	}

}


/*
=============
R_AddSlightInteractions
=============
*/
void R_AddSlightInteractions()
{
	int             i;
	trRefDlight_t  *dl;
	
	if(tr.refdef.rdflags & RDF_NOWORLDMODEL)
	{
		return;
	}

	if(r_dynamiclight->integer != 2)
	{
		return;
	}
	
	//ri.Printf(PRINT_ALL, "R_AddSlightInteractions: adding %i lights\n", tr.world->numDlights);

	for(i = 0; i < tr.world->numDlights; i++)
	{
		dl = tr.currentDlight = &tr.world->dlights[i];
		
		// we must set up parts of tr.or for light culling
		R_RotateForDlight(dl, &tr.viewParms, &tr.or);
		
		// look if we have to draw the light including its interactions
		switch (R_CullLocalBox(dl->localBounds))
		{
			case CULL_IN:
				tr.pc.c_box_cull_slight_in++;
				dl->cull = CULL_IN;
				break;
			
			case CULL_CLIP:
				tr.pc.c_box_cull_slight_clip++;
				dl->cull = CULL_CLIP;
				break;
			
			case CULL_OUT:
			default:
				// light is not visible so skip other light setup stuff to save speed
				tr.pc.c_box_cull_slight_out++;
				dl->cull = CULL_OUT;
				continue;
		}
		
		// ignore if not in visible bounds
		if(!BoundsIntersect(dl->worldBounds[0], dl->worldBounds[1], tr.viewParms.visBounds[0], tr.viewParms.visBounds[1]))
			continue;
		
		// create temporary light scissor
		R_SetDlightScissor(dl);
		
		// setup interactions
		dl->lastInteraction = NULL;
	
		R_AddPrecachedWorldInteractions(dl);
		R_AddEntityInteractions(dl);
		
		if(dl->lastInteraction != NULL)
			tr.pc.c_slights++;
	}
}


/*
=============
R_AddDlightInteractions
=============
*/
void R_AddDlightInteractions()
{
	trRefDlight_t  *dl;

	if(!r_dynamiclight->integer)
	{
		return;
	}

	for(tr.currentDlightNum = 0; tr.currentDlightNum < tr.refdef.numDlights; tr.currentDlightNum++)
	{
		dl = tr.currentDlight = &tr.refdef.dlights[tr.currentDlightNum];
		
		// we must set up parts of tr.or for light culling
		R_RotateForDlight(dl, &tr.viewParms, &tr.or);
		
		// calc local bounds for culling
		R_SetupDlightLocalBounds(dl);
		
		// look if we have to draw the light including its interactions
		switch (R_CullLocalBox(dl->localBounds))
		{
			case CULL_IN:
				tr.pc.c_box_cull_dlight_in++;
				dl->cull = CULL_IN;
				break;
			
			case CULL_CLIP:
				tr.pc.c_box_cull_dlight_clip++;
				dl->cull = CULL_CLIP;
				break;
			
			case CULL_OUT:
			default:
				// light is not visible so skip other light setup stuff to save speed
				tr.pc.c_box_cull_dlight_out++;
				dl->cull = CULL_OUT;
				continue;
		}
		
		// set up light transform matrix
		MatrixSetupTransform(dl->transformMatrix, dl->l.axis[0], dl->l.axis[1], dl->l.axis[2], dl->l.origin);
		
		// set up light origin for lighting and shadowing
		R_SetupDlightOrigin(dl);
		
		// setup world bounds for intersection tests
		R_SetupDlightWorldBounds(dl);
		
		// setup frustum planes for intersection tests
		R_SetupDlightFrustum(dl);
		
		// ignore if not in visible bounds
		if(!BoundsIntersect(dl->worldBounds[0], dl->worldBounds[1], tr.viewParms.visBounds[0], tr.viewParms.visBounds[1]))
			continue;
		
		// set up model to light view matrix
		MatrixAffineInverse(dl->transformMatrix, dl->viewMatrix);
		
		// set up projection
		switch (dl->l.rlType)
		{
			case RL_OMNI:
				MatrixSetupScale(dl->projectionMatrix, 1.0 / dl->l.radius[0], 1.0 / dl->l.radius[1], 1.0 / dl->l.radius[2]);
				break;

			default:
				ri.Error(ERR_DROP, "R_AddDlightInteractions: Bad rlType");
		}
				
		// set up first part of the attenuation matrix
		MatrixSetupTranslation(dl->attenuationMatrix, 0.5, 0.5, 0.5);	// bias
		MatrixMultiplyScale(dl->attenuationMatrix, 0.5, 0.5, 0.5);		// scale
		MatrixMultiply2(dl->attenuationMatrix, dl->projectionMatrix);	// light projection (frustum)
		
		R_SetDlightScissor(dl);
		
		// setup interactions
		dl->lastInteraction = NULL;
	
		R_AddWorldInteractions(dl);
		R_AddEntityInteractions(dl);
		
		if(dl->lastInteraction != NULL)
			tr.pc.c_dlights++;
	}
}

void R_DebugAxis(const vec3_t origin, const matrix_t transformMatrix)
{
	vec3_t          forward, left, up;
	
	MatrixToVectorsFLU(transformMatrix, forward, left, up);
	VectorMA(origin, 16, forward, forward);
	VectorMA(origin, 16, left, left);
	VectorMA(origin, 16, up, up);
	
	// draw axis
	GL_Program(0);
	GL_State(GLS_DEPTHTEST_DISABLE);
	GL_SelectTexture(0);
	GL_Bind(tr.whiteImage);
	
	qglLineWidth(3);
	qglBegin(GL_LINES);
	
	qglColor3f(1, 0, 0);
	qglVertex3fv(origin);
	qglVertex3fv(forward);
	
	qglColor3f(0, 1, 0);
	qglVertex3fv(origin);
	qglVertex3fv(left);
	
	qglColor3f(0, 0, 1);
	qglVertex3fv(origin);
	qglVertex3fv(up);
	
	qglEnd();
	qglLineWidth(1);
}

// Tr3B - from botlib
void R_DebugBoundingBox(const vec3_t origin, const vec3_t mins, const vec3_t maxs, vec4_t color)
{
	vec3_t          corners[8];
	int             i;

	// upper corners
	corners[0][0] = origin[0] + maxs[0];
	corners[0][1] = origin[1] + maxs[1];
	corners[0][2] = origin[2] + maxs[2];
	//
	corners[1][0] = origin[0] + mins[0];
	corners[1][1] = origin[1] + maxs[1];
	corners[1][2] = origin[2] + maxs[2];
	//
	corners[2][0] = origin[0] + mins[0];
	corners[2][1] = origin[1] + mins[1];
	corners[2][2] = origin[2] + maxs[2];
	//
	corners[3][0] = origin[0] + maxs[0];
	corners[3][1] = origin[1] + mins[1];
	corners[3][2] = origin[2] + maxs[2];
	
	// lower corners
	Com_Memcpy(corners[4], corners[0], sizeof(vec3_t) * 4);
	for(i = 0; i < 4; i++)
		corners[4 + i][2] = origin[2] + mins[2];
	
	// draw bounding box
	GL_Program(0);
	GL_State(GLS_DEPTHTEST_DISABLE);
	GL_SelectTexture(0);
	GL_Bind(tr.whiteImage);

	qglBegin(GL_LINES);
	qglColor4fv(color);
	for(i = 0; i < 4; i++)
	{
		// top plane
		qglVertex3fv(corners[i]);
		qglVertex3fv(corners[(i + 1) & 3]);
		
		// bottom plane
		qglVertex3fv(corners[4 + i]);
		qglVertex3fv(corners[4 + ((i + 1) & 3)]);
		
		// vertical lines
		qglVertex3fv(corners[i]);
		qglVertex3fv(corners[4 + i]);
	}
	qglEnd();
}

/*
================
R_DebugPolygon
================
*/
void R_DebugPolygon(int color, int numPoints, float *points)
{
	int             i;

	GL_Program(0);
	GL_ClientState(GLCS_DEFAULT);
	GL_State(GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);

	// draw solid shade
	qglColor3f(color & 1, (color >> 1) & 1, (color >> 2) & 1);
	qglBegin(GL_POLYGON);
	for(i = 0; i < numPoints; i++)
	{
		qglVertex3fv(points + i * 3);
	}
	qglEnd();

	// draw wireframe outline
	GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
	qglDepthRange(0, 0);
	qglColor3f(1, 1, 1);
	qglBegin(GL_POLYGON);
	for(i = 0; i < numPoints; i++)
	{
		qglVertex3fv(points + i * 3);
	}
	qglEnd();
	qglDepthRange(0, 1);
}

/*
====================
R_DebugGraphics

Visualization aid for movement clipping debugging
====================
*/
static void R_DebugGraphics(void)
{
	if(r_debugSurface->integer)
	{
		// the render thread can't make callbacks to the main thread
		R_SyncRenderThread();

		GL_Program(0);
		GL_SelectTexture(0);
		GL_Bind(tr.whiteImage);
		GL_Cull(CT_FRONT_SIDED);
		ri.CM_DrawDebugSurface(R_DebugPolygon);
	}
}



/*
================
R_RenderView

A view may be either the actual camera view,
or a mirror / remote location
================
*/
void R_RenderView(viewParms_t * parms)
{
	int             firstDrawSurf;
	int             firstInteraction;

	if(parms->viewportWidth <= 0 || parms->viewportHeight <= 0)
	{
		return;
	}

	tr.viewCount++;

	tr.viewParms = *parms;
	tr.viewParms.frameSceneNum = tr.frameSceneNum;
	tr.viewParms.frameCount = tr.frameCount;

	firstDrawSurf = tr.refdef.numDrawSurfs;
	firstInteraction = tr.refdef.numInteractions;

	tr.viewCount++;

	// set viewParms.world
	R_RotateForViewer();

	R_SetupFrustum();
	
	// set the projection matrix now that we have the world bounded
	// this needs to be done before entities are
	// added, because they use the projection
	// matrix for lod calculation
	R_SetupProjection();

	R_AddWorldSurfaces();

	R_AddPolygonSurfaces();

	R_AddEntitySurfaces();
	
	R_AddSlightInteractions();
	
	R_AddDlightInteractions();
	
	/*
	ri.Printf(PRINT_ALL, "R_RenderView: %i %i %i %i\n",
			  firstDrawSurf,
			  tr.refdef.numDrawSurfs,
			  firstInteraction,
			  tr.refdef.numInteractions);
	*/

	R_SortDrawSurfs(tr.refdef.drawSurfs + firstDrawSurf,
					tr.refdef.numDrawSurfs - firstDrawSurf,
				   	tr.refdef.interactions + firstInteraction,
				    tr.refdef.numInteractions - firstInteraction);

	// draw main system development information (surface outlines, etc)
	R_DebugGraphics();
}
