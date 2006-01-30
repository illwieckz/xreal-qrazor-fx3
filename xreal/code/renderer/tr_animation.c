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

All bones should be an identity orientation to display the mesh exactly
as it is specified.

For all other frames, the bones represent the transformation from the 
orientation of the bone in the base frame to the orientation in this
frame.

*/

/*
==============
R_AddMD4Surfaces
==============
*/
void R_AddMD4Surfaces(trRefEntity_t * ent)
{
	md4Header_t    *header;
	md4Surface_t   *surface;
	md4LOD_t       *lod;
	shader_t       *shader;
	int             i;

	header = tr.currentModel->md4;
	lod = (md4LOD_t *) ((byte *) header + header->ofsLODs);

	surface = (md4Surface_t *) ((byte *) lod + lod->ofsSurfaces);
	for(i = 0; i < lod->numSurfaces; i++)
	{
		shader = R_GetShaderByHandle(surface->shaderIndex);
		R_AddDrawSurf((void *)surface, shader, 0 /*fogNum */);
		surface = (md4Surface_t *) ((byte *) surface + surface->ofsEnd);
	}
}


/*
==============
RB_SurfaceMD4
==============
*/
void RB_SurfaceMD4(md4Surface_t * surface)
{
	int             i, j, k;
	float           frontlerp, backlerp;
	int            *triangles;
	int             indexes;
	int             baseIndex, baseVertex;
	int             numVerts;
	md4Vertex_t    *v;
	md4Bone_t       bones[MD4_MAX_BONES];
	md4Bone_t      *bonePtr, *bone;
	md4Header_t    *header;
	md4Frame_t     *frame;
	md4Frame_t     *oldFrame;
	int             frameSize;


	if(backEnd.currentEntity->e.oldframe == backEnd.currentEntity->e.frame)
	{
		backlerp = 0;
		frontlerp = 1;
	}
	else
	{
		backlerp = backEnd.currentEntity->e.backlerp;
		frontlerp = 1.0f - backlerp;
	}
	header = (md4Header_t *) ((byte *) surface + surface->ofsHeader);

	frameSize = (int)(&((md4Frame_t *) 0)->bones[header->numBones]);

	frame = (md4Frame_t *) ((byte *) header + header->ofsFrames + backEnd.currentEntity->e.frame * frameSize);
	oldFrame = (md4Frame_t *) ((byte *) header + header->ofsFrames + backEnd.currentEntity->e.oldframe * frameSize);

	RB_CheckOverflow(surface->numVerts, surface->numTriangles * 3);

	triangles = (int *)((byte *) surface + surface->ofsTriangles);
	indexes = surface->numTriangles * 3;
	baseIndex = tess.numIndexes;
	baseVertex = tess.numVertexes;
	for(j = 0; j < indexes; j++)
	{
		tess.indexes[baseIndex + j] = baseIndex + triangles[j];
	}
	tess.numIndexes += indexes;

	//
	// lerp all the needed bones
	//
	if(!backlerp)
	{
		// no lerping needed
		bonePtr = frame->bones;
	}
	else
	{
		bonePtr = bones;
		for(i = 0; i < header->numBones * 12; i++)
		{
			((float *)bonePtr)[i] = frontlerp * ((float *)frame->bones)[i]
				+ backlerp * ((float *)oldFrame->bones)[i];
		}
	}

	//
	// deform the vertexes by the lerped bones
	//
	numVerts = surface->numVerts;
	// FIXME
	// This makes TFC's skeletons work.  Shouldn't be necessary anymore, but left
	// in for reference.
	//v = (md4Vertex_t *) ((byte *)surface + surface->ofsVerts + 12);
	v = (md4Vertex_t *) ((byte *) surface + surface->ofsVerts);
	for(j = 0; j < numVerts; j++)
	{
		vec3_t          tempVert, tempNormal;
		md4Weight_t    *w;

		VectorClear(tempVert);
		VectorClear(tempNormal);
		w = v->weights;
		for(k = 0; k < v->numWeights; k++, w++)
		{
			bone = bonePtr + w->boneIndex;

			tempVert[0] +=
				w->boneWeight * (DotProduct(bone->matrix[0], w->offset) + bone->matrix[0][3]);
			tempVert[1] +=
				w->boneWeight * (DotProduct(bone->matrix[1], w->offset) + bone->matrix[1][3]);
			tempVert[2] +=
				w->boneWeight * (DotProduct(bone->matrix[2], w->offset) + bone->matrix[2][3]);

			tempNormal[0] += w->boneWeight * DotProduct(bone->matrix[0], v->normal);
			tempNormal[1] += w->boneWeight * DotProduct(bone->matrix[1], v->normal);
			tempNormal[2] += w->boneWeight * DotProduct(bone->matrix[2], v->normal);
		}

		tess.xyz[baseVertex + j][0] = tempVert[0];
		tess.xyz[baseVertex + j][1] = tempVert[1];
		tess.xyz[baseVertex + j][2] = tempVert[2];

		tess.normals[baseVertex + j][0] = tempNormal[0];
		tess.normals[baseVertex + j][1] = tempNormal[1];
		tess.normals[baseVertex + j][2] = tempNormal[2];

		tess.texCoords[baseVertex + j][0][0] = v->texCoords[0];
		tess.texCoords[baseVertex + j][0][1] = v->texCoords[1];

		// FIXME
		// This makes TFC's skeletons work.  Shouldn't be necessary anymore, but left
		// in for reference.
		//v = (md4Vertex_t *)( ( byte * )&v->weights[v->numWeights] + 12 );
		v = (md4Vertex_t *) & v->weights[v->numWeights];
	}

	tess.numVertexes += surface->numVerts;
}



/*
=============
R_CullMDS
=============
*/
static int R_CullMDS(mdsHeader_t * header, trRefEntity_t * ent)
{
	vec3_t          bounds[2];
	mdsFrame_t     *oldFrame, *newFrame;
	int             i;

	// compute frame pointers
	newFrame = (mdsFrame_t *) ((byte *) header + header->ofsFrames) + ent->e.frame;
	oldFrame = (mdsFrame_t *) ((byte *) header + header->ofsFrames) + ent->e.oldframe;

	// cull bounding sphere ONLY if this is not an upscaled entity
	if(!ent->e.nonNormalizedAxes)
	{
		if(ent->e.frame == ent->e.oldframe)
		{
			switch(R_CullLocalPointAndRadius(newFrame->localOrigin, newFrame->radius))
			{
				case CULL_OUT:
					tr.pc.c_sphere_cull_mds_out++;
					return CULL_OUT;

				case CULL_IN:
					tr.pc.c_sphere_cull_mds_in++;
					return CULL_IN;

				case CULL_CLIP:
					tr.pc.c_sphere_cull_mds_clip++;
					break;
			}
		}
		else
		{
			int             sphereCull, sphereCullB;

			sphereCull = R_CullLocalPointAndRadius(newFrame->localOrigin, newFrame->radius);
			if(newFrame == oldFrame)
			{
				sphereCullB = sphereCull;
			}
			else
			{
				sphereCullB = R_CullLocalPointAndRadius(oldFrame->localOrigin, oldFrame->radius);
			}

			if(sphereCull == sphereCullB)
			{
				if(sphereCull == CULL_OUT)
				{
					tr.pc.c_sphere_cull_mds_out++;
					return CULL_OUT;
				}
				else if(sphereCull == CULL_IN)
				{
					tr.pc.c_sphere_cull_mds_in++;
					return CULL_IN;
				}
				else
				{
					tr.pc.c_sphere_cull_mds_clip++;
				}
			}
		}
	}

	// calculate a bounding box in the current coordinate system
	for(i = 0; i < 3; i++)
	{
		bounds[0][i] = oldFrame->bounds[0][i] < newFrame->bounds[0][i] ? oldFrame->bounds[0][i] : newFrame->bounds[0][i];
		bounds[1][i] = oldFrame->bounds[1][i] > newFrame->bounds[1][i] ? oldFrame->bounds[1][i] : newFrame->bounds[1][i];
	}

	switch(R_CullLocalBox(bounds))
	{
		case CULL_IN:
			tr.pc.c_box_cull_mds_in++;
			return CULL_IN;
		case CULL_CLIP:
			tr.pc.c_box_cull_mds_clip++;
			return CULL_CLIP;
		case CULL_OUT:
		default:
			tr.pc.c_box_cull_mds_out++;
			return CULL_OUT;
	}
}

/*
=================
R_ComputeFogNumForMDS
=================
*/
static int R_ComputeFogNumForMDS(mdsHeader_t * header, trRefEntity_t * ent)
{
	int             i, j;
	fog_t          *fog;
	mdsFrame_t     *mdsFrame;
	vec3_t          localOrigin;

	if(tr.refdef.rdflags & RDF_NOWORLDMODEL)
	{
		return 0;
	}

	// FIXME: non-normalized axis issues
	mdsFrame = (mdsFrame_t *) ((byte *) header + header->ofsFrames) + ent->e.frame;
	VectorAdd(ent->e.origin, mdsFrame->localOrigin, localOrigin);
	for(i = 1; i < tr.world->numfogs; i++)
	{
		fog = &tr.world->fogs[i];
		for(j = 0; j < 3; j++)
		{
			if(localOrigin[j] - mdsFrame->radius >= fog->bounds[1][j])
			{
				break;
			}
			if(localOrigin[j] + mdsFrame->radius <= fog->bounds[0][j])
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
==============
R_AddMDSSurfaces
==============
*/
void R_AddMDSSurfaces(trRefEntity_t * ent)
{
	mdsHeader_t    *header;
	mdsSurface_t   *surface;
	shader_t       *shader;
	int             i;
	int             cull;
	int             fogNum;
	qboolean        personalModel;
	
	header = tr.currentModel->mds;
	
	// don't add third_person objects if not in a portal
	personalModel = (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal;

	if(ent->e.renderfx & RF_WRAP_FRAMES)
	{
		ent->e.frame %= header->numFrames;
		ent->e.oldframe %= header->numFrames;
	}
	
	// Validate the frames so there is no chance of a crash.
	// This will write directly into the entity structure, so
	// when the surfaces are rendered, they don't need to be
	// range checked again.
	if((ent->e.frame >= header->numFrames) || (ent->e.frame < 0) || (ent->e.oldframe >= header->numFrames) || (ent->e.oldframe < 0))
	{
		ri.Printf(PRINT_DEVELOPER, "R_AddMDSSurfaces: no such frame %d to %d for '%s'\n",
				  ent->e.oldframe, ent->e.frame, tr.currentModel->name);
		ent->e.frame = 0;
		ent->e.oldframe = 0;
	}
	
	// cull the entire model if merged bounding box of both frames
	// is outside the view frustum
	cull = R_CullMDS(header, ent);
	if(cull == CULL_OUT)
	{
		return;
	}
	
	// set up lighting now that we know we aren't culled
	if(!personalModel || r_shadows->integer > 1)
	{
		R_SetupEntityLighting(&tr.refdef, ent);
	}

	// see if we are in a fog volume
	fogNum = R_ComputeFogNumForMDS(header, ent);

	// finally add surfaces
	surface = (mdsSurface_t *) ((byte *) header + header->ofsSurfaces);
	for(i = 0; i < header->numSurfaces; i++)
	{
		if(ent->e.customShader)
		{
			shader = R_GetShaderByHandle(ent->e.customShader);
		}
		else if(ent->e.customSkin > 0 && ent->e.customSkin < tr.numSkins)
		{
			skin_t         *skin;
			int             j;

			skin = R_GetSkinByHandle(ent->e.customSkin);

			// match the surface name to something in the skin file
			shader = tr.defaultShader;
			for(j = 0; j < skin->numSurfaces; j++)
			{
				// the names have both been lowercased
				if(!strcmp(skin->surfaces[j]->name, surface->name))
				{
					shader = skin->surfaces[j]->shader;
					break;
				}
			}
			if(shader == tr.defaultShader)
			{
				ri.Printf(PRINT_DEVELOPER, "WARNING: no shader for surface %s in skin %s\n", surface->name, skin->name);
			}
			else if(shader->defaultShader)
			{
				ri.Printf(PRINT_DEVELOPER, "WARNING: shader %s in skin %s not found\n", shader->name, skin->name);
			}
		}
		else
		{
			//md3Shader = (md3Shader_t *) ((byte *) surface + surface->ofsShaders);
			//md3Shader += ent->e.skinNum % surface->numShaders;
			//shader = tr.shaders[md3Shader->shaderIndex];
			
			shader = R_GetShaderByHandle(surface->shaderIndex);
		}
		
		// we will add shadows even if the main object isn't visible in the view

		// projection shadows work fine with personal models
		if(r_shadows->integer == 2 && fogNum == 0 && (ent->e.renderfx & RF_SHADOW_PLANE) && shader->sort == SS_OPAQUE)
		{
			R_AddDrawSurf((void *)surface, tr.projectionShadowShader, 0);
		}

		// don't add third_person objects if not viewing through a portal
		if(!personalModel)
		{
			R_AddDrawSurf((void *)surface, shader, fogNum);
		}
		
		surface = (mdsSurface_t *) ((byte *) surface + surface->ofsEnd);
	}
}


/*
==============
R_AddMD5Surfaces
==============
*/
void R_AddMD5Surfaces(trRefEntity_t * ent)
{
	md5Model_t     *model;
	md5Surface_t   *surface;
	shader_t       *shader;
	int             i;
	int             cull;
	int             fogNum = 0;
	qboolean        personalModel;
	
	model = tr.currentModel->md5;
	
	// don't add third_person objects if not in a portal
	personalModel = (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal;

	/*
	if(ent->e.renderfx & RF_WRAP_FRAMES)
	{
		ent->e.frame %= header->numFrames;
		ent->e.oldframe %= header->numFrames;
	}
	
	// Validate the frames so there is no chance of a crash.
	// This will write directly into the entity structure, so
	// when the surfaces are rendered, they don't need to be
	// range checked again.
	if((ent->e.frame >= header->numFrames) || (ent->e.frame < 0) || (ent->e.oldframe >= header->numFrames) || (ent->e.oldframe < 0))
	{
		ri.Printf(PRINT_DEVELOPER, "R_AddMDSSurfaces: no such frame %d to %d for '%s'\n",
				  ent->e.oldframe, ent->e.frame, tr.currentModel->name);
		ent->e.frame = 0;
		ent->e.oldframe = 0;
	}
	
	// cull the entire model if merged bounding box of both frames
	// is outside the view frustum
	cull = R_CullMDS(header, ent);
	if(cull == CULL_OUT)
	{
		return;
	}
	
	// set up lighting now that we know we aren't culled
	if(!personalModel || r_shadows->integer > 1)
	{
		R_SetupEntityLighting(&tr.refdef, ent);
	}

	// see if we are in a fog volume
	fogNum = R_ComputeFogNumForMDS(header, ent);
	*/

	// finally add surfaces
	for(i = 0, surface = model->surfaces; i < model->numSurfaces; i++, surface++)
	{
		if(ent->e.customShader)
		{
			shader = R_GetShaderByHandle(ent->e.customShader);
		}
		/*
		else if(ent->e.customSkin > 0 && ent->e.customSkin < tr.numSkins)
		{
			skin_t         *skin;
			int             j;

			skin = R_GetSkinByHandle(ent->e.customSkin);

			// match the surface name to something in the skin file
			shader = tr.defaultShader;
			for(j = 0; j < skin->numSurfaces; j++)
			{
				// the names have both been lowercased
				if(!strcmp(skin->surfaces[j]->name, surface->name))
				{
					shader = skin->surfaces[j]->shader;
					break;
				}
			}
			if(shader == tr.defaultShader)
			{
				ri.Printf(PRINT_DEVELOPER, "WARNING: no shader for surface %s in skin %s\n", surface->name, skin->name);
			}
			else if(shader->defaultShader)
			{
				ri.Printf(PRINT_DEVELOPER, "WARNING: shader %s in skin %s not found\n", shader->name, skin->name);
			}
		}
		*/
		else
		{
			shader = R_GetShaderByHandle(surface->shaderIndex);
		}
		
		// we will add shadows even if the main object isn't visible in the view

		// projection shadows work fine with personal models
		if(r_shadows->integer == 2 && fogNum == 0 && (ent->e.renderfx & RF_SHADOW_PLANE) && shader->sort == SS_OPAQUE)
		{
			R_AddDrawSurf((void *)surface, tr.projectionShadowShader, 0);
		}

		// don't add third_person objects if not viewing through a portal
		if(!personalModel)
		{
			R_AddDrawSurf((void *)surface, shader, fogNum);
		}
	}
}
