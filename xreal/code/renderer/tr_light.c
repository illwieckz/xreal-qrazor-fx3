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
// tr_light.c

#include "tr_local.h"

/*
===============
R_TransformDlights

Transforms the origins of an array of dlights.
Used by both the front end (for DlightBmodel) and
the back end (before doing the lighting calculation)
===============
*/
void R_TransformDlights(int count, trRefDlight_t * dl, orientationr_t * or)
{
	int             i;
	vec3_t          temp;

	for(i = 0; i < count; i++, dl++)
	{
		VectorSubtract(dl->l.origin, or->origin, temp);
		dl->transformed[0] = DotProduct(temp, or->axis[0]);
		dl->transformed[1] = DotProduct(temp, or->axis[1]);
		dl->transformed[2] = DotProduct(temp, or->axis[2]);
	}
}

/*
=============
R_AddBrushModelInteractions

Determine which dynamic lights may effect this bmodel
=============
*/
void R_AddBrushModelInteractions(trRefEntity_t * ent, trRefDlight_t * light)
{
	int             i;
	msurface_t     *surf;
	bmodel_t       *bModel = NULL;
	model_t        *pModel = NULL;
	qboolean        shadowOnly = qfalse;

	// cull the entire model if it is outside the view frustum
	// and we don't care about proper shadowing
	if(ent->cull == CULL_OUT)
	{
		if(r_shadows->integer <= 2)
			return;
		else
			shadowOnly = qtrue;
	}

	pModel = R_GetModelByHandle(ent->e.hModel);
	bModel = pModel->bmodel;

	// cull the entire model if merged bounding box of both frames
	// does not intersect with light
	if(light->worldBounds[1][0] < ent->worldBounds[0][0] ||
	   light->worldBounds[1][1] < ent->worldBounds[0][1] ||
	   light->worldBounds[1][2] < ent->worldBounds[0][2] ||
	   light->worldBounds[0][0] > ent->worldBounds[1][0] ||
	   light->worldBounds[0][1] > ent->worldBounds[1][1] || light->worldBounds[0][2] > ent->worldBounds[1][2])
	{
		tr.pc.c_dlightSurfacesCulled += bModel->numSurfaces;
		return;
	}

	// set the dlight bits in all the surfaces
	for(i = 0; i < bModel->numSurfaces; i++)
	{
		surf = bModel->firstSurface + i;

		// FIXME: do more culling?

		/*
		   if(*surf->data == SF_FACE)
		   {
		   ((srfSurfaceFace_t *) surf->data)->dlightBits[tr.smpFrame] = mask;
		   }
		   else if(*surf->data == SF_GRID)
		   {
		   ((srfGridMesh_t *) surf->data)->dlightBits[tr.smpFrame] = mask;
		   }
		   else if(*surf->data == SF_TRIANGLES)
		   {
		   ((srfTriangles_t *) surf->data)->dlightBits[tr.smpFrame] = mask;
		   }
		 */

		R_AddDlightInteraction(light, surf->data, surf->shader, shadowOnly);
		tr.pc.c_dlightSurfaces++;
	}
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/



/*
=================
R_SetupEntityLightingGrid
=================
*/
static void R_SetupEntityLightingGrid(trRefEntity_t * ent)
{
	vec3_t          lightOrigin;
	int             pos[3];
	int             i, j;
	byte           *gridData;
	float           frac[3];
	int             gridStep[3];
	vec3_t          direction;
	float           totalFactor;

	if(ent->e.renderfx & RF_LIGHTING_ORIGIN)
	{
		// seperate lightOrigins are needed so an object that is
		// sinking into the ground can still be lit, and so
		// multi-part models can be lit identically
		VectorCopy(ent->e.lightingOrigin, lightOrigin);
	}
	else
	{
		VectorCopy(ent->e.origin, lightOrigin);
	}

	VectorSubtract(lightOrigin, tr.world->lightGridOrigin, lightOrigin);
	for(i = 0; i < 3; i++)
	{
		float           v;

		v = lightOrigin[i] * tr.world->lightGridInverseSize[i];
		pos[i] = floor(v);
		frac[i] = v - pos[i];
		if(pos[i] < 0)
		{
			pos[i] = 0;
		}
		else if(pos[i] >= tr.world->lightGridBounds[i] - 1)
		{
			pos[i] = tr.world->lightGridBounds[i] - 1;
		}
	}

	VectorClear(ent->ambientLight);
	VectorClear(ent->directedLight);
	VectorClear(direction);

	assert(tr.world->lightGridData);	// bk010103 - NULL with -nolight maps

	// trilerp the light value
	gridStep[0] = 8;
	gridStep[1] = 8 * tr.world->lightGridBounds[0];
	gridStep[2] = 8 * tr.world->lightGridBounds[0] * tr.world->lightGridBounds[1];
	gridData = tr.world->lightGridData + pos[0] * gridStep[0] + pos[1] * gridStep[1] + pos[2] * gridStep[2];

	totalFactor = 0;
	for(i = 0; i < 8; i++)
	{
		float           factor;
		byte           *data;
		int             lat, lng;
		vec3_t          normal;

#if idppc
		float           d0, d1, d2, d3, d4, d5;
#endif
		factor = 1.0;
		data = gridData;
		for(j = 0; j < 3; j++)
		{
			if(i & (1 << j))
			{
				factor *= frac[j];
				data += gridStep[j];
			}
			else
			{
				factor *= (1.0f - frac[j]);
			}
		}

		if(!(data[0] + data[1] + data[2]))
		{
			continue;			// ignore samples in walls
		}
		totalFactor += factor;
#if idppc
		d0 = data[0];
		d1 = data[1];
		d2 = data[2];
		d3 = data[3];
		d4 = data[4];
		d5 = data[5];

		ent->ambientLight[0] += factor * d0;
		ent->ambientLight[1] += factor * d1;
		ent->ambientLight[2] += factor * d2;

		ent->directedLight[0] += factor * d3;
		ent->directedLight[1] += factor * d4;
		ent->directedLight[2] += factor * d5;
#else
		ent->ambientLight[0] += factor * data[0];
		ent->ambientLight[1] += factor * data[1];
		ent->ambientLight[2] += factor * data[2];

		ent->directedLight[0] += factor * data[3];
		ent->directedLight[1] += factor * data[4];
		ent->directedLight[2] += factor * data[5];
#endif
		lat = data[7];
		lng = data[6];
		lat *= (FUNCTABLE_SIZE / 256);
		lng *= (FUNCTABLE_SIZE / 256);

		// decode X as cos( lat ) * sin( long )
		// decode Y as sin( lat ) * sin( long )
		// decode Z as cos( long )

		normal[0] = tr.sinTable[(lat + (FUNCTABLE_SIZE / 4)) & FUNCTABLE_MASK] * tr.sinTable[lng];
		normal[1] = tr.sinTable[lat] * tr.sinTable[lng];
		normal[2] = tr.sinTable[(lng + (FUNCTABLE_SIZE / 4)) & FUNCTABLE_MASK];

		VectorMA(direction, factor, normal, direction);
	}

	if(totalFactor > 0 && totalFactor < 0.99)
	{
		totalFactor = 1.0f / totalFactor;
		VectorScale(ent->ambientLight, totalFactor, ent->ambientLight);
		VectorScale(ent->directedLight, totalFactor, ent->directedLight);
	}

	VectorScale(ent->ambientLight, r_ambientScale->value, ent->ambientLight);
	VectorScale(ent->directedLight, r_directedScale->value, ent->directedLight);

	VectorNormalize2(direction, ent->lightDir);
}


/*
===============
LogLight
===============
*/
static void LogLight(trRefEntity_t * ent)
{
	int             max1, max2;

	if(!(ent->e.renderfx & RF_FIRST_PERSON))
	{
		return;
	}

	max1 = ent->ambientLight[0];
	if(ent->ambientLight[1] > max1)
	{
		max1 = ent->ambientLight[1];
	}
	else if(ent->ambientLight[2] > max1)
	{
		max1 = ent->ambientLight[2];
	}

	max2 = ent->directedLight[0];
	if(ent->directedLight[1] > max2)
	{
		max2 = ent->directedLight[1];
	}
	else if(ent->directedLight[2] > max2)
	{
		max2 = ent->directedLight[2];
	}

	ri.Printf(PRINT_ALL, "amb:%i  dir:%i\n", max1, max2);
}

/*
=================
R_SetupEntityLighting

Calculates all the lighting values that will be used
by the Calc_* functions
=================
*/
void R_SetupEntityLighting(const trRefdef_t * refdef, trRefEntity_t * ent)
{
	int             i;
	vec3_t          lightDir;
	vec3_t          lightOrigin;
	float           d;

	// lighting calculations 
	if(ent->lightingCalculated)
	{
		return;
	}
	ent->lightingCalculated = qtrue;

	// trace a sample point down to find ambient light
	if(ent->e.renderfx & RF_LIGHTING_ORIGIN)
	{
		// seperate lightOrigins are needed so an object that is
		// sinking into the ground can still be lit, and so
		// multi-part models can be lit identically
		VectorCopy(ent->e.lightingOrigin, lightOrigin);
	}
	else
	{
		VectorCopy(ent->e.origin, lightOrigin);
	}

	// if NOWORLDMODEL, only use dynamic lights (menu system, etc)
	if(!(refdef->rdflags & RDF_NOWORLDMODEL) && tr.world->lightGridData)
	{
		R_SetupEntityLightingGrid(ent);
	}
	else
	{
		ent->ambientLight[0] = ent->ambientLight[1] = ent->ambientLight[2] = tr.identityLight * 150;
		ent->directedLight[0] = ent->directedLight[1] = ent->directedLight[2] = tr.identityLight * 150;
		VectorCopy(tr.sunDirection, ent->lightDir);
	}

	// bonus items and view weapons have a fixed minimum add
	if(1 /* ent->e.renderfx & RF_MINLIGHT */ )
	{
		// give everything a minimum light add
		ent->ambientLight[0] += tr.identityLight * 32;
		ent->ambientLight[1] += tr.identityLight * 32;
		ent->ambientLight[2] += tr.identityLight * 32;
	}

	// clamp ambient
	for(i = 0; i < 3; i++)
	{
		if(ent->ambientLight[i] > tr.identityLightByte)
		{
			ent->ambientLight[i] = tr.identityLightByte;
		}
	}

	if(r_debugLight->integer)
	{
		LogLight(ent);
	}

	// save out the byte packet version
	((byte *) & ent->ambientLightInt)[0] = myftol(ent->ambientLight[0]);
	((byte *) & ent->ambientLightInt)[1] = myftol(ent->ambientLight[1]);
	((byte *) & ent->ambientLightInt)[2] = myftol(ent->ambientLight[2]);
	((byte *) & ent->ambientLightInt)[3] = 0xff;

	// transform the direction to local space
	d = VectorLength(ent->directedLight);
	VectorScale(ent->lightDir, d, lightDir);
	VectorNormalize(lightDir);
	ent->lightDir[0] = DotProduct(lightDir, ent->e.axis[0]);
	ent->lightDir[1] = DotProduct(lightDir, ent->e.axis[1]);
	ent->lightDir[2] = DotProduct(lightDir, ent->e.axis[2]);
}

/*
=================
R_LightForPoint
=================
*/
int R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir)
{
	trRefEntity_t   ent;

	// bk010103 - this segfaults with -nolight maps
	if(tr.world->lightGridData == NULL)
		return qfalse;

	Com_Memset(&ent, 0, sizeof(ent));
	VectorCopy(point, ent.e.origin);
	R_SetupEntityLightingGrid(&ent);
	VectorCopy(ent.ambientLight, ambientLight);
	VectorCopy(ent.directedLight, directedLight);
	VectorCopy(ent.lightDir, lightDir);

	return qtrue;
}


/*
=================
R_SetupDlightLocalBounds
=================
*/
void R_SetupDlightLocalBounds(trRefDlight_t * dl)
{
	dl->localBounds[0][0] = dl->l.radius[0];
	dl->localBounds[0][1] = dl->l.radius[1];
	dl->localBounds[0][2] = dl->l.radius[2];
	dl->localBounds[1][0] = -dl->l.radius[0];
	dl->localBounds[1][1] = -dl->l.radius[1];
	dl->localBounds[1][2] = -dl->l.radius[2];
}

/*
=================
R_SetupDlightWorldBounds
Tr3B - needs finished transformMatrix
=================
*/
void R_SetupDlightWorldBounds(trRefDlight_t * dl)
{
	int             j;
	vec3_t          v, transformed;

	ClearBounds(dl->worldBounds[0], dl->worldBounds[1]);

	for(j = 0; j < 8; j++)
	{
		v[0] = dl->localBounds[j & 1][0];
		v[1] = dl->localBounds[(j >> 1) & 1][1];
		v[2] = dl->localBounds[(j >> 2) & 1][2];

		// transform local bounds vertices into world space
		MatrixTransformPoint(dl->transformMatrix, v, transformed);

		AddPointToBounds(transformed, dl->worldBounds[0], dl->worldBounds[1]);
	}
}


/*
=================
R_AddDlightInteraction
=================
*/
void R_AddDlightInteraction(trRefDlight_t * light, surfaceType_t * surface, shader_t * surfaceShader, qboolean shadowOnly)
{
	int             index;
	interaction_t  *ia;

	// instead of checking for overflow, we just mask the index
	// so it wraps around
	index = tr.refdef.numInteractions & INTERACTION_MASK;
	ia = &tr.refdef.interactions[index];
	tr.refdef.numInteractions++;

	// connect to interaction grid
	if(light->lastInteraction)
	{
		light->lastInteraction->next = ia;
	}
	light->lastInteraction = ia;

	// check what kind of attenuationShader is used
	if(!light->l.attenuationShader)
	{
		if(light->isStatic)
		{
			ia->dlightShader = tr.defaultPointLightShader;
		}
		else
		{
			ia->dlightShader = tr.defaultDlightShader;
		}
	}
	else
	{
		ia->dlightShader = R_GetShaderByHandle(light->l.attenuationShader);
	}

	ia->next = NULL;
	ia->dlight = light;
	ia->entity = tr.currentEntity;
	ia->surface = surface;
	ia->surfaceShader = surfaceShader;
	ia->shadowOnly = shadowOnly;

	ia->scissorX = light->scissor.coords[0];
	ia->scissorY = light->scissor.coords[1];
	ia->scissorWidth = light->scissor.coords[2] - light->scissor.coords[0];
	ia->scissorHeight = light->scissor.coords[3] - light->scissor.coords[1];

	if(light->isStatic)
	{
		tr.pc.c_slightInteractions++;
	}
	else
	{
		tr.pc.c_dlightInteractions++;
	}
}

/*
=================
R_DlightIntersectsPoint
=================
*/
qboolean R_DlightIntersectsPoint(trRefDlight_t * light, const vec3_t p)
{
	// TODO light frustum test

	return BoundsIntersectPoint(light->worldBounds[0], light->worldBounds[1], p);
}


/*
=================
R_IntersectRayPlane
=================
*/
static void R_IntersectRayPlane(const vec3_t v1, const vec3_t v2, cplane_t * plane, vec3_t res)
{
	vec3_t          v;
	float           sect;

	VectorSubtract(v1, v2, v);
	sect = -(DotProduct(plane->normal, v1) - plane->dist) / DotProduct(plane->normal, v);
	VectorScale(v, sect, v);
	VectorAdd(v1, v, res);
}


/*
=================
R_AddPointToLightScissor
=================
*/
static void R_AddPointToLightScissor(trRefDlight_t * light, const vec3_t world)
{
	vec4_t          eye, clip, normalized, window;
	
	R_TransformWorldToClip(world, tr.viewParms.world.viewMatrix, tr.viewParms.projectionMatrix, eye, clip);
	R_TransformClipToWindow2(clip, &tr.viewParms, normalized, window);
	
	if(window[0] > light->scissor.coords[2])
		light->scissor.coords[2] = (int)window[0];
	
	if(window[0] < light->scissor.coords[0])
		light->scissor.coords[0] = (int)window[0];
	
	if(window[1] > light->scissor.coords[3])
		light->scissor.coords[3] = (int)window[1];
	
	if(window[1] < light->scissor.coords[1])
		light->scissor.coords[1] = (int)window[1];
}

/*
=================
R_AddEdgeToLightScissor
=================
*/
static void R_AddEdgeToLightScissor(trRefDlight_t * light, vec3_t local1, vec3_t local2)
{
	int             i;
	vec3_t          intersect;
	vec3_t          world1, world2;
	qboolean        side1, side2;
	cplane_t       *frust;
	
	for(i = 0; i < 5; i++)
	{
		R_LocalPointToWorld(local1, world1);
		R_LocalPointToWorld(local2, world2);
		
		frust = &tr.viewParms.frustum[i];
	
		// check edge to frustrum plane
		side1 = ((DotProduct(frust->normal, world1) - frust->dist) >= 0.0);
		side2 = ((DotProduct(frust->normal, world2) - frust->dist) >= 0.0);

		if(!side1 && !side2)
			continue;					// edge behind plane

		if(!side1 || !side2)
			R_IntersectRayPlane(world1, world2, frust, intersect);

		if(!side1)
		{
			VectorCopy(intersect, world1);
		}
		else if(!side2)
		{
			VectorCopy(intersect, world2);
		}
		
		R_AddPointToLightScissor(light, world1);
		R_AddPointToLightScissor(light, world2);
	}
}

/*
=================
R_SetDlightScissor
Recturns the screen space rectangle taken by the box.
	(Clips the box to the near plane to have correct results even if the box intersects the near plane)
Tr3B - recoded from Tenebrae2
=================
*/
void R_SetDlightScissor(trRefDlight_t * light)
{
	int             i, j;
	vec3_t          v1, v2;
	vec3_t          v;
	vec3_t          transformed[8];
	float           dists[8];
	cplane_t       *frust;
	int             anyBack;
	int             front, back;

	if(r_noLightScissors->integer || R_DlightIntersectsPoint(light, tr.viewParms.or.origin))
	{
		light->scissor.coords[0] = tr.viewParms.viewportX;
		light->scissor.coords[1] = tr.viewParms.viewportY;
		light->scissor.coords[2] = tr.viewParms.viewportX + tr.viewParms.viewportWidth;
		light->scissor.coords[3] = tr.viewParms.viewportY + tr.viewParms.viewportHeight;
		return;
	}

	// transform local light corners to world space -> eye space -> clip space -> window space
	// and extend the light scissor's mins maxs by resulting window coords
	light->scissor.coords[0] = 100000000;
	light->scissor.coords[1] = 100000000;
	light->scissor.coords[2] = -100000000;
	light->scissor.coords[3] = -100000000;
	
	// top plane
	VectorSet(v1, light->localBounds[1][0], light->localBounds[1][1], light->localBounds[1][2]);
	VectorSet(v2, light->localBounds[0][0], light->localBounds[1][1], light->localBounds[1][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	VectorSet(v1, light->localBounds[1][0], light->localBounds[1][1], light->localBounds[1][2]);
	VectorSet(v2, light->localBounds[1][0], light->localBounds[0][1], light->localBounds[1][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	VectorSet(v1, light->localBounds[0][0], light->localBounds[0][1], light->localBounds[1][2]);
	VectorSet(v2, light->localBounds[0][0], light->localBounds[1][1], light->localBounds[1][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	VectorSet(v1, light->localBounds[0][0], light->localBounds[0][1], light->localBounds[1][2]);
	VectorSet(v2, light->localBounds[1][0], light->localBounds[0][1], light->localBounds[1][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	// bottom plane
	VectorSet(v1, light->localBounds[1][0], light->localBounds[1][1], light->localBounds[0][2]);
	VectorSet(v2, light->localBounds[0][0], light->localBounds[1][1], light->localBounds[0][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	VectorSet(v1, light->localBounds[1][0], light->localBounds[1][1], light->localBounds[0][2]);
	VectorSet(v2, light->localBounds[1][0], light->localBounds[0][1], light->localBounds[0][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	VectorSet(v1, light->localBounds[0][0], light->localBounds[0][1], light->localBounds[0][2]);
	VectorSet(v2, light->localBounds[0][0], light->localBounds[1][1], light->localBounds[0][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	VectorSet(v1, light->localBounds[0][0], light->localBounds[0][1], light->localBounds[0][2]);
	VectorSet(v2, light->localBounds[1][0], light->localBounds[0][1], light->localBounds[0][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	// sides
	VectorSet(v1, light->localBounds[0][0], light->localBounds[1][1], light->localBounds[0][2]);
	VectorSet(v2, light->localBounds[0][0], light->localBounds[1][1], light->localBounds[1][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	VectorSet(v1, light->localBounds[1][0], light->localBounds[1][1], light->localBounds[0][2]);
	VectorSet(v2, light->localBounds[1][0], light->localBounds[1][1], light->localBounds[1][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	VectorSet(v1, light->localBounds[0][0], light->localBounds[0][1], light->localBounds[0][2]);
	VectorSet(v2, light->localBounds[0][0], light->localBounds[0][1], light->localBounds[1][2]);
	R_AddEdgeToLightScissor(light, v1, v2);

	VectorSet(v1, light->localBounds[1][0], light->localBounds[0][1], light->localBounds[0][2]);
	VectorSet(v2, light->localBounds[1][0], light->localBounds[0][1], light->localBounds[1][2]);
	R_AddEdgeToLightScissor(light, v1, v2);
}
