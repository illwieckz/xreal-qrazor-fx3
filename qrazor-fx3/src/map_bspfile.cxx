/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
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
#include "map_local.h"

#include "common.h"
#include "vfs.h"

// xreal --------------------------------------------------------------------
//#include "cmdlib.h"
//#include "mathlib.h"
//#include "bspfile.h"
//#include "scriplib.h"

void	GetLeafNums();


std::vector<bsp_dentdata_t>	dentdata;

std::vector<bsp_dshader_t>	dshaders;

std::vector<bsp_dplane_t>	dplanes;

std::vector<bsp_dnode_t>	dnodes;

std::vector<bsp_dleaf_t>	dleafs;

std::vector<bsp_dleafsurface_t>	dleafsurfaces;

std::vector<bsp_dleafbrush_t>	dleafbrushes;

std::vector<bsp_dmodel_t>	dmodels;

std::vector<bsp_dbrush_t>	dbrushes;

std::vector<bsp_dbrushside_t>	dbrushsides;

std::vector<bsp_dvertex_t>	dvertexes;

std::vector<bsp_dindex_t>	dindexes;

std::vector<bsp_dsurface_t>	dsurfaces;

std::vector<bsp_dvisdata_t>	dvisdata;

/*
int	CompressVis(byte *vis, byte *dest)
{
	int		j;
	int		rep;
	int		visrow;
	byte	*dest_p;
	
	dest_p = dest;
//	visrow = (r_numvisleafs + 7)>>3;
	visrow = (dvis->numclusters + 7)>>3;
	
	for(j=0; j<visrow; j++)
	{
		*dest_p++ = vis[j];
		
		if(vis[j])
			continue;

		rep = 1;
		for(j++; j<visrow; j++)
		{
			if(vis[j] || rep == 255)
				break;
			else
				rep++;
		}
		
		*dest_p++ = rep;
		
		j--;
	}
	
	return dest_p - dest;
}


void	DecompressVis(byte *in, byte *decompressed)
{
	int		c;
	byte	*out;
	int		row;

//	row = (r_numvisleafs+7)>>3;	
	row = (dvis->numclusters+7)>>3;	
	out = decompressed;

	do
	{
		if(*in)
		{
			*out++ = *in++;
			continue;
		}
	
		c = in[1];
		if(!c)
			Com_Error(ERR_FATAL, "DecompressVis: 0 repeat");
		in += 2;
		while(c)
		{
			*out++ = 0;
			c--;
		}
	}
	while(out - decompressed < row);
}
*/



/*
=============
SwapBSPFile

Byte swaps all data in a bsp file.
=============
*/
static void	SwapBSPFile()
{
	//
	// shaders
	//	
	for(std::vector<bsp_dshader_t>::iterator ir = dshaders.begin(); ir != dshaders.end(); ir++)
	{		
		//char			shader[64];			// shader name		
		(*ir).flags = LittleLong((*ir).flags);
		(*ir).contents = LittleLong((*ir).contents);
	}
	
	
	//
	// planes
	//	
	for(std::vector<bsp_dplane_t>::iterator ir = dplanes.begin(); ir != dplanes.end(); ir++)
	{
		for(int j=0; j<3; j++)
			(*ir).normal[j] = LittleFloat((*ir).normal[j]);
			
		(*ir).dist = LittleFloat((*ir).dist);
	}
	
	
	//
	// nodes
	//
	for(std::vector<bsp_dnode_t>::iterator ir = dnodes.begin(); ir != dnodes.end(); ir++)
	{
		(*ir).plane_num = LittleLong((*ir).plane_num);
		
		(*ir).children[0] = LittleLong((*ir).children[0]);
		(*ir).children[1] = LittleLong((*ir).children[1]);
		
		for(int j=0; j<3; j++)
		{
			(*ir).mins[j] = LittleLong((*ir).mins[j]);
			(*ir).maxs[j] = LittleLong((*ir).maxs[j]);
		}
	}
	
	
	//
	// leafs
	//
	for(std::vector<bsp_dleaf_t>::iterator ir = dleafs.begin(); ir != dleafs.end(); ir++)
	{
		(*ir).cluster = LittleLong((*ir).cluster);
		(*ir).area = LittleLong((*ir).area);
		
		for(int j=0; j<3; j++)
		{
			(*ir).mins[j] = LittleLong((*ir).mins[j]);
			(*ir).maxs[j] = LittleLong((*ir).maxs[j]);
		}
		
		(*ir).leafsurfaces_first = LittleLong((*ir).leafsurfaces_first);
		(*ir).leafsurfaces_num = LittleLong((*ir).leafsurfaces_num);
		
		(*ir).leafbrushes_first = LittleLong((*ir).leafbrushes_first);
		(*ir).leafbrushes_num = LittleLong((*ir).leafbrushes_num);
	}
	

	//
	// leafsurfaces
	//
	for(std::vector<int>::iterator ir = dleafsurfaces.begin(); ir != dleafsurfaces.end(); ir++)
	{
		*ir = LittleLong(*ir);
	}
	

	//
	// leafbrushes
	//
	for(std::vector<int>::iterator ir = dleafbrushes.begin(); ir != dleafbrushes.end(); ir++)
	{
		*ir = LittleLong(*ir);
	}


	//
	// models	
	//
	for(std::vector<bsp_dmodel_t>::iterator ir = dmodels.begin(); ir != dmodels.end(); ir++)
	{
		for(int j=0; j<3; j++)
		{
			(*ir).mins[j] = LittleFloat((*ir).mins[j]);
			(*ir).maxs[j] = LittleFloat((*ir).maxs[j]);
		}

		(*ir).modelsurfaces_first = LittleLong((*ir).modelsurfaces_first);
		(*ir).modelsurfaces_num	= LittleLong((*ir).modelsurfaces_num);
		
		(*ir).brushes_first = LittleLong((*ir).brushes_first);
		(*ir).brushes_num = LittleLong((*ir).brushes_num);
	}
	
	
	//
	// brushes
	//
	for(std::vector<bsp_dbrush_t>::iterator ir = dbrushes.begin(); ir != dbrushes.end(); ir++)
	{
		(*ir).sides_first = LittleLong((*ir).sides_first);
		(*ir).sides_num = LittleLong((*ir).sides_num);
		
		(*ir).shader_num = LittleLong((*ir).shader_num);
	}
	
	
	//
	// brushsides
	//
	for(std::vector<bsp_dbrushside_t>::iterator ir = dbrushsides.begin(); ir != dbrushsides.end(); ir++)
	{		
		(*ir).plane_num = LittleLong((*ir).plane_num);
		(*ir).shader_num = LittleLong((*ir).shader_num);
	}


	//
	// vertexes
	//
	for(std::vector<bsp_dvertex_t>::iterator ir = dvertexes.begin(); ir != dvertexes.end(); ir++)
	{
		for(int j=0; j<2; j++)
		{
			(*ir).texcoord_st[j] = LittleFloat((*ir).texcoord_st[j]);
			(*ir).texcoord_lm[j] = LittleFloat((*ir).texcoord_lm[j]);
		}
		
		for(int j=0; j<3; j++)
		{
			(*ir).position[j] = LittleFloat((*ir).position[j]);
			(*ir).normal[j] = LittleFloat((*ir).normal[j]);
		}
	}
	
	
	//
	// indexes
	//
	for(std::vector<int>::iterator ir = dindexes.begin(); ir != dindexes.end(); ir++)
	{
		*ir = LittleLong(*ir);
	}

	
	
	//
	// surfaces
	//
	for(std::vector<bsp_dsurface_t>::iterator ir = dsurfaces.begin(); ir != dsurfaces.end(); ir++)
	{
		(*ir).shader_num = LittleLong((*ir).shader_num);
		(*ir).fog_num = LittleLong((*ir).fog_num);
		(*ir).face_type = LittleLong((*ir).face_type);
		
		(*ir).vertexes_first = LittleLong((*ir).vertexes_first);
		(*ir).vertexes_num = LittleLong((*ir).vertexes_num);
		
		(*ir).indexes_first = LittleLong((*ir).indexes_first);
		(*ir).indexes_num = LittleLong((*ir).indexes_num);
		
		(*ir).lm_num = LittleLong((*ir).lm_num);
		(*ir).lm_x = LittleLong((*ir).lm_x);
		(*ir).lm_y = LittleLong((*ir).lm_y);
		(*ir).lm_w = LittleLong((*ir).lm_w);
		(*ir).lm_h = LittleLong((*ir).lm_h);
		
		for(int j=0; j<3; j++)
		{
			(*ir).origin[j] = LittleFloat((*ir).origin[j]);
		
			(*ir).mins[j] = LittleFloat((*ir).mins[j]);
			(*ir).maxs[j] = LittleFloat((*ir).maxs[j]);
			
			(*ir).normal[j] = LittleFloat((*ir).normal[j]);
		}
		
		(*ir).mesh_cp[0] = LittleLong((*ir).mesh_cp[0]);
		(*ir).mesh_cp[1] = LittleLong((*ir).mesh_cp[1]);
	}


	//
	// areas
	//
	/*
	for(i=0 ; i<numareas ; i++)
	{
		dareas[i].numareaportals = LittleLong (dareas[i].numareaportals);
		dareas[i].firstareaportal = LittleLong (dareas[i].firstareaportal);
	}
	*/

	//
	// areasportals
	//
	/*
	for (i=0 ; i<numareaportals ; i++)
	{
		dareaportals[i].portalnum = LittleLong (dareaportals[i].portalnum);
		dareaportals[i].otherarea = LittleLong (dareaportals[i].otherarea);
	}
	*/


	//
	// visibility
	//
	/*
	if(todisk)
		j = dvis->numclusters;
	else
		j = LittleLong(dvis->numclusters);
	dvis->numclusters = LittleLong (dvis->numclusters);
	for (i=0 ; i<j ; i++)
	{
		dvis->bitofs[i][0] = LittleLong (dvis->bitofs[i][0]);
		dvis->bitofs[i][1] = LittleLong (dvis->bitofs[i][1]);
	}
	*/
	
	((int*)&dvisdata)[0] = LittleLong(((int*)&dvisdata)[0]);
	((int*)&dvisdata)[1] = LittleLong(((int*)&dvisdata)[1]);
}


template<typename T>
static void	CopyLump(bsp_dheader_t*	header, int lump, std::vector<T> &data)
{
	int		length, ofs, size, count;
	
	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;
	size = sizeof(T);
	count = length / size;
	
	if(length % size)
		Com_Error(ERR_FATAL, "CopyLump: lump %i has odd lump size", lump);
	
	//memcpy(dest, (byte *)header + ofs, length);
	
	T *in = (T*)((byte*)header + ofs);
	T out;
	
	for(int i=0; i<count; i++, in++)
	{
		out = *in;
		
		data.push_back(out);
	}
}

/*
=============
LoadBSPFile
=============
*/
void	LoadBSPFile(const std::string &filename)
{
	bsp_dheader_t*	header = NULL;
	

	//
	// load the file header
	//
	VFS_FLoad(filename, (void**)&header);
	if(!header)
	{
		Com_Error(ERR_FATAL, "LoadBSPFile: could not load '%s'", filename.c_str());
		//Com_Printf("LoadBSPFile: could not load '%s'\n", filename.c_str());
		return;
	}
	
	Com_Printf("loading '%s' ...\n", filename.c_str());

	// swap the header
	for(unsigned int i=0; i< sizeof(bsp_dheader_t)/4; i++)
		((int*)header)[i] = LittleLong(((int*)header)[i]);

	if(header->ident != BSP_IDENT)
		Com_Error(ERR_FATAL, "%s is not a IBSP file", filename.c_str());
		
	if(header->version != BSP_VERSION)
		Com_Error(ERR_FATAL, "%s is version %i, not %i", filename.c_str(), header->version, BSP_VERSION);

	CopyLump<bsp_dentdata_t>	(header, BSP_LUMP_ENTITIES,	dentdata);
	CopyLump<bsp_dshader_t>		(header, BSP_LUMP_SHADERS,	dshaders);
	CopyLump<bsp_dplane_t>		(header, BSP_LUMP_PLANES,	dplanes);
	CopyLump<bsp_dnode_t>		(header, BSP_LUMP_NODES,	dnodes);
	CopyLump<bsp_dleaf_t>		(header, BSP_LUMP_LEAFS,	dleafs);
	CopyLump<bsp_dleafsurface_t>	(header, BSP_LUMP_LEAFSURFACES,	dleafsurfaces);
	CopyLump<bsp_dleafbrush_t>	(header, BSP_LUMP_LEAFBRUSHES,	dleafbrushes);
	CopyLump<bsp_dmodel_t>		(header, BSP_LUMP_MODELS,	dmodels);
	CopyLump<bsp_dbrush_t>		(header, BSP_LUMP_BRUSHES,	dbrushes);
	CopyLump<bsp_dbrushside_t>	(header, BSP_LUMP_BRUSHSIDES,	dbrushsides);
	CopyLump<bsp_dvertex_t>		(header, BSP_LUMP_VERTEXES,	dvertexes);
	CopyLump<bsp_dindex_t>		(header, BSP_LUMP_INDEXES,	dindexes);
	CopyLump<bsp_dsurface_t>	(header, BSP_LUMP_SURFACES,	dsurfaces);
	//CopyLump<bsp_darea_t>		(BSP_LUMP_AREAS,		dareas);
	//CopyLump<bsp_dareaportal_t>	(BSP_LUMP_AREAPORTALS,		dareaportals);
	CopyLump<bsp_dvisdata_t>	(header, BSP_LUMP_VISIBILITY,	dvisdata);

	VFS_FFree(header);		// everything has been copied out
		
	//
	// swap everything
	//	
	SwapBSPFile();
}

template<typename T>
static void	AddLump(VFILE *stream, bsp_dheader_t *header, int lumpnum, const std::vector<T> &data)
{
	int		len;
	bsp_lump_t*	lump;
	
	len = sizeof(T)*data.size();

	lump = &header->lumps[lumpnum];
	
	lump->fileofs = LittleLong(VFS_FTell(stream));
	lump->filelen = LittleLong(len);
	
	for(unsigned int i=0; i<data.size(); i++)
		VFS_FWrite((void*)&data[i], sizeof(T), stream);
}

/*
=============
WriteBSPFile

Swaps the bsp file in place, so it should not be referenced again
=============
*/
void	WriteBSPFile(const std::string &filename)
{
	bsp_dheader_t	outheader;
	bsp_dheader_t*	header;
	VFILE*		stream = NULL;
	
	header = &outheader;
	memset(header, 0, sizeof(bsp_dheader_t));
	
	SwapBSPFile();

	header->ident = LittleLong(BSP_IDENT);
	header->version = LittleLong(BSP_VERSION);
	
	VFS_FOpenWrite(filename, &stream);
	
	VFS_FWrite(header, sizeof(bsp_dheader_t), stream);
	
	AddLump<bsp_dentdata_t>		(stream, header, BSP_LUMP_ENTITIES,	dentdata);
	AddLump<bsp_dshader_t>		(stream, header, BSP_LUMP_SHADERS,	dshaders);
	AddLump<bsp_dplane_t>		(stream, header, BSP_LUMP_PLANES, 	dplanes);
	AddLump<bsp_dnode_t>		(stream, header, BSP_LUMP_NODES, 	dnodes);
	AddLump<bsp_dleaf_t>		(stream, header, BSP_LUMP_LEAFS, 	dleafs);
	AddLump<bsp_dleafsurface_t>	(stream, header, BSP_LUMP_LEAFSURFACES, dleafsurfaces);
	AddLump<bsp_dleafbrush_t>	(stream, header, BSP_LUMP_LEAFBRUSHES, 	dleafbrushes);
	AddLump<bsp_dmodel_t>		(stream, header, BSP_LUMP_MODELS,	dmodels);
	AddLump<bsp_dbrush_t>		(stream, header, BSP_LUMP_BRUSHES,	dbrushes);
	AddLump<bsp_dbrushside_t>	(stream, header, BSP_LUMP_BRUSHSIDES,	dbrushsides);
	AddLump<bsp_dvertex_t>		(stream, header, BSP_LUMP_VERTEXES,	dvertexes);
	AddLump<bsp_dindex_t>		(stream, header, BSP_LUMP_INDEXES,	dindexes);
	AddLump<bsp_dsurface_t>		(stream, header, BSP_LUMP_SURFACES,	dsurfaces);
	//AddLump<bsp_darea_t>		(stream, header, BSP_LUMP_AREAS,	dareas);
	//AddLump<bsp_dareaportal_t>	(stream, header, BSP_LUMP_AREAPORTALS,	dareaportals);
	AddLump<bsp_dvisdata_t>		(stream, header, BSP_LUMP_VISIBILITY,	dvisdata);
	
	
	VFS_FSeek(stream, 0, VFS_SEEK_SET);
	VFS_FWrite(header, sizeof(bsp_dheader_t), stream);
	VFS_FClose(&stream);
}



template<typename T>
static void	PrintLump(const std::string &name, const std::vector<T> &data)
{
	Com_Printf("%6i %s %7i\n", data.size(), name.c_str(), data.size() * sizeof(T));
}	

/*
=============
PrintBSPFileSizes

Dumps info about current file
=============
*/
void	PrintBSPFileSizes()
{
	PrintLump<bsp_dentdata_t>	("entdata       ", dentdata);
	PrintLump<bsp_dshader_t>	("shaders       ", dshaders);
	PrintLump<bsp_dplane_t>		("planes        ", dplanes);
	PrintLump<bsp_dnode_t>		("nodes         ", dnodes);
	PrintLump<bsp_dleaf_t>		("leafs         ", dleafs);
	PrintLump<bsp_dleafsurface_t>	("leafsurfaces  ", dleafsurfaces);
	PrintLump<bsp_dleafbrush_t>	("leafbrushes   ", dleafbrushes);
	PrintLump<bsp_dmodel_t>		("models        ", dmodels);
	PrintLump<bsp_dbrush_t>		("brushes       ", dbrushes);
	PrintLump<bsp_dbrushside_t>	("brushsides    ", dbrushsides);
	PrintLump<bsp_dvertex_t>	("vertices      ", dvertexes);
	PrintLump<bsp_dindex_t>		("indices       ", dindexes);
	PrintLump<bsp_dsurface_t>	("surfaces      ", dsurfaces);
	PrintLump<bsp_dvisdata_t>	("visdata       ", dvisdata);
	
	//ParseEntities();
	//Com_Printf("%6i entities\n", entities.size());
}



