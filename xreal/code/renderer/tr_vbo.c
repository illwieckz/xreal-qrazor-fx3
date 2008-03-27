/*
===========================================================================
Copyright (C) 2007 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_vbo.c
#include "tr_local.h"

/*
============
R_CreateStaticVBO
============
*/
VBO_t          *R_CreateStaticVBO(const char *name, byte * vertexes, int vertexesSize, byte * indexes, int indexesSize)
{
	VBO_t          *vbo;

	if(strlen(name) >= MAX_QPATH)
	{
		ri.Error(ERR_DROP, "R_CreateVBO: \"%s\" is too long\n", name);
	}

	// make sure the render thread is stopped
	R_SyncRenderThread();

	vbo = ri.Hunk_Alloc(sizeof(*vbo), h_low);
	Com_AddToGrowList(&tr.vbos, vbo);

	Q_strncpyz(vbo->name, name, sizeof(vbo->name));

	vbo->ofsXYZ = 0;
	vbo->ofsTexCoords = 0;
	vbo->ofsBinormals = 0;
	vbo->ofsTangents = 0;
	vbo->ofsNormals = 0;
	vbo->ofsColors = 0;

	vbo->ofsIndexes = 0;

	vbo->vertexesSize = vertexesSize;
	vbo->indexesSize = indexesSize;

	qglGenBuffersARB(1, &vbo->vertexesVBO);
	qglGenBuffersARB(1, &vbo->indexesVBO);

	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->vertexesVBO);
	qglBufferDataARB(GL_ARRAY_BUFFER_ARB, vertexesSize, vertexes, GL_STATIC_DRAW_ARB);

	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo->indexesVBO);
	qglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexesSize, indexes, GL_STATIC_DRAW_ARB);

	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	GL_CheckErrors();

	// unbind once created
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	return vbo;
}



/*
============
R_CreateStaticVBO2
============
*/
VBO_t          *R_CreateStaticVBO2(const char *name, int numVertexes, srfVert_t * verts, int numTriangles, srfTriangle_t * triangles, unsigned int stateBits)
{
	VBO_t          *vbo;

	int             i, j;

	byte           *data;
	int             dataSize;
	int             dataOfs;

	byte           *indexes;
	int             indexesSize;
	int             indexesOfs;

	vec4_t          tmp;
	int             index;

	if(!numVertexes || !numTriangles)
		return NULL;

	if(strlen(name) >= MAX_QPATH)
	{
		ri.Error(ERR_DROP, "R_CreateVBO: \"%s\" is too long\n", name);
	}

	// make sure the render thread is stopped
	R_SyncRenderThread();

	vbo = ri.Hunk_Alloc(sizeof(*vbo), h_low);
	Com_AddToGrowList(&tr.vbos, vbo);

	Q_strncpyz(vbo->name, name, sizeof(vbo->name));

	vbo->ofsXYZ = 0;
	vbo->ofsTexCoords = 0;
	vbo->ofsBinormals = 0;
	vbo->ofsTangents = 0;
	vbo->ofsNormals = 0;
	vbo->ofsColors = 0;

	vbo->ofsIndexes = 0;

	//ri.Printf(PRINT_DEVELOPER, "...calculating world mesh VBOs ( %s, %i verts %i tris )\n", shader->name, vertexesNum, indexesNum / 3);

	// create VBOs
	dataSize = numVertexes * (sizeof(vec4_t) * 6 + sizeof(color4ub_t));
	data = ri.Hunk_AllocateTempMemory(dataSize);
	dataOfs = 0;
	//vertexesNum = 0;

	indexesSize = numTriangles * 3 * sizeof(int);
	indexes = ri.Hunk_AllocateTempMemory(indexesSize);
	indexesOfs = 0;
	//indexesNum = 0;

	// build triangle indices
			
	// set up triangle indices
	if(numTriangles)
	{
		srfTriangle_t  *tri;

		for(i = 0, tri = triangles; i < numTriangles; i++, tri++)
		{
				for(j = 0; j < 3; j++)
				{
					index = /*numVertexesNum +*/ tri->indexes[j];

					memcpy(indexes + indexesOfs, &index, sizeof(int));
					indexesOfs += sizeof(int);
				}
		}

		//indexesNum = srf->numTriangles * 3;
	}

	if(numVertexes)
	{
		// set up xyz array
		for(i = 0; i < numVertexes; i++)
		{
			for(j = 0; j < 3; j++)
			{
				tmp[j] = verts[i].xyz[j];
			}
			tmp[3] = 1;

			memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
			dataOfs += sizeof(vec4_t);
		}

		// feed vertex texcoords
		if(stateBits & GLCS_TEXCOORD)
		{
			vbo->ofsTexCoords = dataOfs;
			for(i = 0; i < numVertexes; i++)
			{
				for(j = 0; j < 2; j++)
				{
					tmp[j] = verts[i].st[j];
				}
				tmp[2] = 0;
				tmp[3] = 1;

				memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
				dataOfs += sizeof(vec4_t);
			}
		}

		// feed vertex lightmap texcoords
		if(stateBits & GLCS_LIGHTCOORD)
		{
			vbo->ofsLightCoords = dataOfs;
			for(i = 0; i < numVertexes; i++)
			{
				for(j = 0; j < 2; j++)
				{
					tmp[j] = verts[i].lightmap[j];
				}
				tmp[2] = 0;
				tmp[3] = 1;

				memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
				dataOfs += sizeof(vec4_t);
			}
		}
		
		// feed vertex tangents
		if(stateBits & GLCS_TANGENT)
		{
			vbo->ofsTangents = dataOfs;
			for(i = 0; i < numVertexes; i++)
			{
				for(j = 0; j < 3; j++)
				{
					tmp[j] = verts[i].tangent[j];
				}
				tmp[3] = 1;

				memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
				dataOfs += sizeof(vec4_t);
			}
		}
		
		// feed vertex binormals
		if(stateBits & GLCS_BINORMAL)
		{
			vbo->ofsBinormals = dataOfs;
			for(i = 0; i < numVertexes; i++)
			{
				for(j = 0; j < 3; j++)
				{
					tmp[j] = verts[i].binormal[j];
				}
				tmp[3] = 1;

				memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
				dataOfs += sizeof(vec4_t);
			}
		}
		
		// feed vertex normals
		if(stateBits & GLCS_NORMAL)
		{
			vbo->ofsNormals = dataOfs;
			for(i = 0; i < numVertexes; i++)
			{
				for(j = 0; j < 3; j++)
				{
					tmp[j] = verts[i].normal[j];
				}
				tmp[3] = 1;

				memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
				dataOfs += sizeof(vec4_t);
			}
		}

		// feed vertex colors
		if(stateBits & GLCS_COLOR)
		{
			vbo->ofsColors = dataOfs;
			for(i = 0; i < numVertexes; i++)
			{
				memcpy(data + dataOfs, verts[i].color, sizeof(color4ub_t));
				dataOfs += sizeof(color4ub_t);
			}
		}
	}

	vbo->vertexesSize = dataSize;
	vbo->indexesSize = indexesSize;

	qglGenBuffersARB(1, &vbo->vertexesVBO);
	qglGenBuffersARB(1, &vbo->indexesVBO);

	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->vertexesVBO);
	qglBufferDataARB(GL_ARRAY_BUFFER_ARB, dataSize, data, GL_STATIC_DRAW_ARB);

	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo->indexesVBO);
	qglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexesSize, indexes, GL_STATIC_DRAW_ARB);

	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	GL_CheckErrors();

	// unbind once created
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	ri.Hunk_FreeTempMemory(indexes);
	ri.Hunk_FreeTempMemory(data);

	return vbo;
}

/*
============
R_BindVBO
============
*/
void R_BindVBO(VBO_t * vbo)
{
	if(!vbo)
	{
		R_BindNullVBO();
		return;
	}

	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get a call to va() every frame!
		GLimp_LogComment(va("--- R_BindVBO( %s ) ---\n", vbo->name));
	}

	if(glState.currentVBO != vbo)
	{
		qglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->vertexesVBO);
		qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo->indexesVBO);

		glState.currentVBO = vbo;

		backEnd.pc.c_vboIndexBuffers++;
		backEnd.pc.c_vboVertexBuffers++;
	}
}

/*
============
R_BindNullVBO
============
*/
void R_BindNullVBO(void)
{
	if(r_logFile->integer)
	{
		GLimp_LogComment("--- R_BindNullVBO ---\n");
	}

	if(glState.currentVBO)
	{
		qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		glState.currentVBO = NULL;
	}
}

/*
============
R_InitVBOs
============
*/
void R_InitVBOs(void)
{
	if(!glConfig.vertexBufferObjectAvailable)
		return;

	Com_InitGrowList(&tr.vbos, 100);

	R_BindNullVBO();
}

/*
============
R_ShutdownVBOs
============
*/
void R_ShutdownVBOs(void)
{
	int             i;
	VBO_t          *vbo;

	if(!glConfig.vertexBufferObjectAvailable)
		return;

	R_BindNullVBO();

	for(i = 0; i < tr.vbos.currentElements; i++)
	{
		vbo = (VBO_t *) Com_GrowListElement(&tr.vbos, i);

		if(vbo->vertexesVBO)
		{
			qglDeleteBuffersARB(1, &vbo->vertexesVBO);
		}

		if(vbo->indexesVBO)
		{
			qglDeleteBuffersARB(1, &vbo->indexesVBO);
		}
	}

	Com_DestroyGrowList(&tr.vbos);
}

/*
============
R_VBOList_f
============
*/
void R_VBOList_f(void)
{
	int             i;
	VBO_t          *vbo;
	int             vertexesSize = 0;
	int             indexesSize = 0;

	if(!glConfig.vertexBufferObjectAvailable)
	{
		ri.Printf(PRINT_ALL, "GL_ARB_vertex_buffer_object is not available.\n");
		return;
	}

	ri.Printf(PRINT_ALL, " size          name\n");
	ri.Printf(PRINT_ALL, "----------------------------------------------------------\n");

	for(i = 0; i < tr.vbos.currentElements; i++)
	{
		vbo = (VBO_t *) Com_GrowListElement(&tr.vbos, i);

		ri.Printf(PRINT_ALL, "%d.%02d MB %d.%02d MB %s\n", vbo->vertexesSize / (1024 * 1024),
				  (vbo->vertexesSize % (1024 * 1024)) * 100 / (1024 * 1024), vbo->indexesSize / (1024 * 1024),
				  (vbo->indexesSize % (1024 * 1024)) * 100 / (1024 * 1024), vbo->name);

		vertexesSize += vbo->vertexesSize;
		indexesSize += vbo->indexesSize;
	}

	ri.Printf(PRINT_ALL, " %i total VBOs\n", tr.vbos.currentElements);
	ri.Printf(PRINT_ALL, " %d.%02d MB total vertices memory\n", vertexesSize / (1024 * 1024),
			  (vertexesSize % (1024 * 1024)) * 100 / (1024 * 1024));
	ri.Printf(PRINT_ALL, " %d.%02d MB total triangle indices memory\n", indexesSize / (1024 * 1024),
			  (indexesSize % (1024 * 1024)) * 100 / (1024 * 1024));
}
