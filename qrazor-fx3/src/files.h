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
#ifndef FILES_H
#define FILES_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"


/*
================================================================================
			ASE static file format
================================================================================
*/
#define ASE_IDENTSTRING		"*3DSMAX_ASCIIEXPORT"
#define ASE_VERSION		200


/*
================================================================================
			LWO2 file format
================================================================================
*/


/* chunk and subchunk IDs */

#define MAKE_ID(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

#define ID_FORM  MAKE_ID('F','O','R','M')
#define ID_LWO2  MAKE_ID('L','W','O','2')
#define ID_LWOB  MAKE_ID('L','W','O','B')

/* top-level chunks */
#define ID_LAYR  MAKE_ID('L','A','Y','R')
#define ID_TAGS  MAKE_ID('T','A','G','S')
#define ID_PNTS  MAKE_ID('P','N','T','S')
#define ID_BBOX  MAKE_ID('B','B','O','X')
#define ID_VMAP  MAKE_ID('V','M','A','P')
#define ID_VMAD  MAKE_ID('V','M','A','D')
#define ID_POLS  MAKE_ID('P','O','L','S')
#define ID_PTAG  MAKE_ID('P','T','A','G')
#define ID_ENVL  MAKE_ID('E','N','V','L')
#define ID_CLIP  MAKE_ID('C','L','I','P')
#define ID_SURF  MAKE_ID('S','U','R','F')
#define ID_DESC  MAKE_ID('D','E','S','C')
#define ID_TEXT  MAKE_ID('T','E','X','T')
#define ID_ICON  MAKE_ID('I','C','O','N')

/* polygon types */
#define ID_FACE  MAKE_ID('F','A','C','E')
#define ID_CURV  MAKE_ID('C','U','R','V')
#define ID_PTCH  MAKE_ID('P','T','C','H')
#define ID_MBAL  MAKE_ID('M','B','A','L')
#define ID_BONE  MAKE_ID('B','O','N','E')

/* polygon tags */
#define ID_SURF  MAKE_ID('S','U','R','F')
#define ID_PART  MAKE_ID('P','A','R','T')
#define ID_SMGP  MAKE_ID('S','M','G','P')

/* envelopes */
#define ID_PRE   MAKE_ID('P','R','E',' ')
#define ID_POST  MAKE_ID('P','O','S','T')
#define ID_KEY   MAKE_ID('K','E','Y',' ')
#define ID_SPAN  MAKE_ID('S','P','A','N')
#define ID_TCB   MAKE_ID('T','C','B',' ')
#define ID_HERM  MAKE_ID('H','E','R','M')
#define ID_BEZI  MAKE_ID('B','E','Z','I')
#define ID_BEZ2  MAKE_ID('B','E','Z','2')
#define ID_LINE  MAKE_ID('L','I','N','E')
#define ID_STEP  MAKE_ID('S','T','E','P')

/* clips */
#define ID_STIL  MAKE_ID('S','T','I','L')
#define ID_ISEQ  MAKE_ID('I','S','E','Q')
#define ID_ANIM  MAKE_ID('A','N','I','M')
#define ID_XREF  MAKE_ID('X','R','E','F')
#define ID_STCC  MAKE_ID('S','T','C','C')
#define ID_TIME  MAKE_ID('T','I','M','E')
#define ID_CONT  MAKE_ID('C','O','N','T')
#define ID_BRIT  MAKE_ID('B','R','I','T')
#define ID_SATR  MAKE_ID('S','A','T','R')
#define ID_HUE   MAKE_ID('H','U','E',' ')
#define ID_GAMM  MAKE_ID('G','A','M','M')
#define ID_NEGA  MAKE_ID('N','E','G','A')
#define ID_IFLT  MAKE_ID('I','F','L','T')
#define ID_PFLT  MAKE_ID('P','F','L','T')

/* surfaces */
#define ID_COLR  MAKE_ID('C','O','L','R')
#define ID_LUMI  MAKE_ID('L','U','M','I')
#define ID_DIFF  MAKE_ID('D','I','F','F')
#define ID_SPEC  MAKE_ID('S','P','E','C')
#define ID_GLOS  MAKE_ID('G','L','O','S')
#define ID_REFL  MAKE_ID('R','E','F','L')
#define ID_RFOP  MAKE_ID('R','F','O','P')
#define ID_RIMG  MAKE_ID('R','I','M','G')
#define ID_RSAN  MAKE_ID('R','S','A','N')
#define ID_TRAN  MAKE_ID('T','R','A','N')
#define ID_TROP  MAKE_ID('T','R','O','P')
#define ID_TIMG  MAKE_ID('T','I','M','G')
#define ID_RIND  MAKE_ID('R','I','N','D')
#define ID_TRNL  MAKE_ID('T','R','N','L')
#define ID_BUMP  MAKE_ID('B','U','M','P')
#define ID_SMAN  MAKE_ID('S','M','A','N')
#define ID_SIDE  MAKE_ID('S','I','D','E')
#define ID_CLRH  MAKE_ID('C','L','R','H')
#define ID_CLRF  MAKE_ID('C','L','R','F')
#define ID_ADTR  MAKE_ID('A','D','T','R')
#define ID_SHRP  MAKE_ID('S','H','R','P')
#define ID_LINE  MAKE_ID('L','I','N','E')
#define ID_LSIZ  MAKE_ID('L','S','I','Z')
#define ID_ALPH  MAKE_ID('A','L','P','H')
#define ID_AVAL  MAKE_ID('A','V','A','L')
#define ID_GVAL  MAKE_ID('G','V','A','L')
#define ID_BLOK  MAKE_ID('B','L','O','K')

/* texture layer */
#define ID_TYPE  MAKE_ID('T','Y','P','E')
#define ID_CHAN  MAKE_ID('C','H','A','N')
#define ID_NAME  MAKE_ID('N','A','M','E')
#define ID_ENAB  MAKE_ID('E','N','A','B')
#define ID_OPAC  MAKE_ID('O','P','A','C')
#define ID_FLAG  MAKE_ID('F','L','A','G')
#define ID_PROJ  MAKE_ID('P','R','O','J')
#define ID_STCK  MAKE_ID('S','T','C','K')
#define ID_TAMP  MAKE_ID('T','A','M','P')

/* texture coordinates */
#define ID_TMAP  MAKE_ID('T','M','A','P')
#define ID_AXIS  MAKE_ID('A','X','I','S')
#define ID_CNTR  MAKE_ID('C','N','T','R')
#define ID_SIZE  MAKE_ID('S','I','Z','E')
#define ID_ROTA  MAKE_ID('R','O','T','A')
#define ID_OREF  MAKE_ID('O','R','E','F')
#define ID_FALL  MAKE_ID('F','A','L','L')
#define ID_CSYS  MAKE_ID('C','S','Y','S')

#define ID_TXUV  MAKE_ID('T','X','U','V')

/* image map */
#define ID_IMAP  MAKE_ID('I','M','A','P')
#define ID_IMAG  MAKE_ID('I','M','A','G')
#define ID_WRAP  MAKE_ID('W','R','A','P')
#define ID_WRPW  MAKE_ID('W','R','P','W')
#define ID_WRPH  MAKE_ID('W','R','P','H')
#define ID_VMAP  MAKE_ID('V','M','A','P')
#define ID_AAST  MAKE_ID('A','A','S','T')
#define ID_PIXB  MAKE_ID('P','I','X','B')

/* procedural */
#define ID_PROC  MAKE_ID('P','R','O','C')
#define ID_COLR  MAKE_ID('C','O','L','R')
#define ID_VALU  MAKE_ID('V','A','L','U')
#define ID_FUNC  MAKE_ID('F','U','N','C')
#define ID_FTPS  MAKE_ID('F','T','P','S')
#define ID_ITPS  MAKE_ID('I','T','P','S')
#define ID_ETPS  MAKE_ID('E','T','P','S')

/* gradient */
#define ID_GRAD  MAKE_ID('G','R','A','D')
#define ID_GRST  MAKE_ID('G','R','S','T')
#define ID_GREN  MAKE_ID('G','R','E','N')
#define ID_PNAM  MAKE_ID('P','N','A','M')
#define ID_INAM  MAKE_ID('I','N','A','M')
#define ID_GRPT  MAKE_ID('G','R','P','T')
#define ID_FKEY  MAKE_ID('F','K','E','Y')
#define ID_IKEY  MAKE_ID('I','K','E','Y')

/* shader */
#define ID_SHDR  MAKE_ID('S','H','D','R')
#define ID_DATA  MAKE_ID('D','A','T','A')


typedef char            I1;
typedef short           I2;
typedef int             I4;
typedef unsigned char   U1;
typedef unsigned short  U2;
typedef unsigned int    U4;
typedef float           F4;
typedef unsigned int    VX;
typedef float           FP4;
typedef float           ANG4;
typedef float           VEC12[3];
typedef float           COL12[3];
typedef char            ID4[5];
typedef char            S0[255];
typedef char            FNAM0[255];

typedef struct
{
	uint_t		index_vertex;
	vec2_c		texcoord;
} lwo_vmap_t;

typedef struct
{
	uint_t		index_vertex;
	uint_t		index_polygon;
	vec2_c		texcoord;
} lwo_vmad_t;

typedef struct
{
	uint_t		index_polygon;
	uint_t		index_tag;
} lwo_ptag_t;


/*
================================================================================
			MD2 alias file format
================================================================================
*/
#define MD2_IDENTSTRING		"IDP2"
#define MD2_IDENT		(('2'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD2_VERSION		8

#define	MD2_MAX_TRIANGLES	4096
#define	MD2_MAX_INDEXES		MD2_MAX_TRIANGLES*3
#define MD2_MAX_VERTS		2048
#define MD2_MAX_FRAMES		512*2
#define MD2_MAX_SKINS		32
#define	MD2_MAX_SKINNAME	64

typedef struct
{
	short	st[2];
} md2_dtexcoord_t;

typedef struct 
{
	short	index_xyz[3];
	short	index_st[3];
} md2_dtriangle_t;

typedef struct
{
	byte	v[3];			// scaled byte to fit in frame mins/maxs
	byte	lightnormalindex;
} md2_dvertex_t;

typedef struct
{
	float		scale[3];	// multiply byte verts by this
	float		translate[3];	// then add this
	char		name[16];	// frame name from grabbing
	md2_dvertex_t	vertexes[1];	// variable sized
} md2_dframe_t;


// the glcmd format:
// a positive integer starts a tristrip command, followed by that many
// vertex structures.
// a negative integer starts a trifan command, followed by -x vertexes
// a zero indicates the end of the command list.
// a vertex consists of a floating point s, a floating point t,
// and an integer vertex index.


typedef struct
{
	int			ident;
	int			version;

	int			skin_width;
	int			skin_height;
	int			framesize;		// byte size of each frame

	int			skins_num;
	int			vertexes_num;
	int			texcoords_num;		// greater than num_xyz for seams
	int			triangles_num;
	int			glcmds_num;		// dwords in strip/fan command list
	int			frames_num;

	int			skins_ofs;		// each skin is a MAX_SKINNAME string
	int			texcoords_ofs;		// byte offset from start for stverts
	int			triangles_ofs;		// offset for dtriangles
	int			frames_ofs;		// offset for first frame
	int			glcmds_ofs;	
	int			end_ofs;		// end of file

} md2_dheader_t;




/*
================================================================================
			MD3 alias file format
================================================================================
*/
#define MD3_IDENTSTRING		"IDP3"
#define MD3_IDENT		(('3'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD3_VERSION		15

#define	MD3_MAX_TRIANGLES	8192	// per mesh
#define	MD3_MAX_INDEXES		MD3_MAX_TRIANGLES*3	// per mesh
#define MD3_MAX_VERTEXES	4096	// per mesh
#define MD3_MAX_FRAMES		1024	// per model

#define MD3_MAX_SHADERS		256	// per mesh
#define MD3_MAX_MESHES		32	// per model
#define MD3_MAX_TAGS		16	// per frame
#define MD3_MAX_PATH		64

#define MD3_VERTEX_SCALE	(1.0f/64.0f)


typedef struct
{
	float			st[2];
} md3_dtexcoord_t;

typedef struct 
{
	int			indexes[3];
} md3_dtriangle_t;

typedef struct
{
	short			vertex[3];	// scaled byte to fit in frame mins/maxs
	short			normal;
} md3_dvertex_t;


typedef struct
{
	float			mins[3];	// multiply byte verts by this
	float			maxs[3];	// then add this
	
	float			translate[3];	// local origin
	float 			radius;
	
	char			name[16];	// frame name from grabbing
} md3_dframe_t;

typedef struct
{
	char			name[MD3_MAX_PATH];
	
	float			origin[3];
	float			axis[3][3];
} md3_dtag_t;

typedef struct
{
	int			ident;
	int			version;

	char			name[MD3_MAX_PATH];
	
	int 			flags;
	
	int			frames_num;
	int			tags_num;
	int			meshes_num;
	int			skins_num;
	
	int			frames_ofs;		// offset for first frame
	int			tags_ofs;
	int			meshes_ofs;
	int			end_ofs;		// end of file
} md3_dheader_t;


typedef struct
{
	char			ident[4];
	
	char 			name[MD3_MAX_PATH];
	
	int 			flags;
	
	int			frames_num;
	int			shaders_num;
	int			vertexes_num;
	int			triangles_num;
	
	int			indexes_ofs;
	int			shaders_ofs;
	int			texcoords_ofs;
	int			vertexes_ofs;
	
	int			end_ofs;		// next surface follows
} md3_dmesh_t;


typedef struct
{
	char			name[MD3_MAX_PATH];
	int			index;
} md3_dshader_t;


/*
================================================================================
			MDC alias file format
================================================================================
*/

//http://www.planetwolfenstein.com/themdcfile/mdcfile.htm

#define MDC_IDENTSTRING		"IDPC"
#define MDC_IDENT		(('C'<<24)+('P'<<16)+('D'<<8)+'I')
#define MDC_VERSION		2

#define	MDC_MAX_TRIANGLES	8192	// per mesh
#define	MDC_MAX_INDEXES		MD3_MAX_TRIANGLES*3	// per mesh
#define MDC_MAX_VERTEXES	4096	// per mesh
#define MDC_MAX_FRAMES		1024	// per model

#define MDC_MAX_SHADERS		256	// per mesh
#define MDC_MAX_MESHES		32	// per model
#define MDC_MAX_TAGS		16	// per frame
#define MDC_MAX_PATH		64

#define MDC_VERTEX_SCALE	(1.0f/64.0f)
#define MDC_VERTEX_COMP_SCALE	(3.0f/64.0f)


typedef struct
{
	float			st[2];
} mdc_dtexcoord_t;

typedef struct 
{
	int			indexes[3];
} mdc_dtriangle_t;

typedef struct
{
	short			vertex[3];	// scaled byte to fit in frame mins/maxs
	short			normal;
} mdc_dvertex_t;


typedef struct
{
	//byte			vertex[3];	// scaled byte to fit in frame mins/maxs
	//byte			normal;
	int			vertex;
} mdc_dvertex_comp_t;

typedef struct
{
	float			mins[3];	// multiply byte verts by this
	float			maxs[3];	// then add this
	
	float			translate[3];	// local origin
	float 			radius;
	
	char			name[16];	// frame name from grabbing
} mdc_dframe_t;

typedef struct
{
	char			name[MDC_MAX_PATH];
	
	short			origin[3];
	short			angles[3];
} mdc_dtag_t;

typedef struct
{
	int			ident;
	int			version;

	char			name[MDC_MAX_PATH];
	
	int 			flags;
	
	int			frames_num;
	int			tags_num;
	int			meshes_num;
	int			skins_num;
	
	int			frames_ofs;		// offset for first frame
	int			tags_name_ofs;
	int			tags_ofs;
	int			meshes_ofs;
	int			end_ofs;		// end of file
} mdc_dheader_t;


typedef struct
{
	char			ident[4];
	
	char 			name[MDC_MAX_PATH];
	
	int 			flags;
	
	int			frames_comp_num;	// compressed frames
	int			frames_num;
	int			shaders_num;
	int			vertexes_num;
	int			triangles_num;
	
	int			indexes_ofs;
	int			shaders_ofs;
	int			texcoords_ofs;
	int			vertexes_ofs;
	int			vertexes_comp_ofs;
	int			frames_ofs;
	int			frames_comp_ofs;
	
	int			end_ofs;		// next surface follows
} mdc_dmesh_t;


typedef struct
{
	char			name[MD3_MAX_PATH];
	int			index;
} mdc_dshader_t;


/*
================================================================================
			MDS skeletal file format
================================================================================
*/

//http://www.quake.cz/winclan/mdsexporter/MDSFormat.html

#define MDS_IDENTSTRING		"MDSW"
#define MDS_IDENT		(('W'<<24)+('S'<<16)+('D'<<8)+'M')
#define MDS_VERSION		0x0004

#define	MDS_MAX_TRIANGLES	8192	// per mesh
#define	MDS_MAX_INDEXES		MD3_MAX_TRIANGLES*3	// per mesh
#define MDS_MAX_VERTEXES	6000	// per mesh
#define MDS_MAX_MESHES		32	// per model
#define MDS_MAX_SHADERS		256	// per mesh
#define MDS_MAX_FRAMES		1024*8	// per model
#define MDS_MAX_TAGS		128	// per model
#define MDS_MAX_PATH		64

#define MDS_VERTEX_SCALE	(1.0f/64.0f)

#define MDS_MAX_BONES		128	// per model
#define MDS_MAX_BONES_VERTEXES	16



typedef struct 
{
	int			indexes[3];
} mds_dtriangle_t;

typedef struct
{
	float			normal[3];		// scaled byte to fit in frame mins/maxs
	float			st[2];
	
	int			weights_num;		// number of bone weights affecting this vertex

	int			fixed_parent;		// stay equidistant from this parent
	float			fixed_dist;		// fixed distance from parent
} mds_dvertex_t;

typedef struct
{
	int			bone_index;		// index of bone in the main bone list
	float			weight;			// bone weighting (from 0.0 to 1.0)
  	float			origin[3];		// xyz bone space position of vertex
} mds_dweight_t;

typedef struct
{
	float			mins[3];		// multiply byte verts by this
	float			maxs[3];		// then add this

	float			translate[3];		// local origin
	float 			radius;
	
	float			parent_offset[3]; 	// Offset of parent bone from origin
} mds_dframe_t;

typedef struct
{
	char			name[MDS_MAX_PATH];
	
	float			torso_weight;
	int			bone_index;
} mds_dtag_t;

typedef struct
{
	short			angles[4];		// defines the bone orientation
	short			angles_ofs[2];		// defines the direction of the bone pivot from this bone's parent
} mds_dcompressed_bone_frame_t;

typedef struct
{
	char			name[MDS_MAX_PATH];
	int			parent_index;		// bone parent index (-1 if root)
	float			torso_weight;		// 0.0 to 1.0
	float			parent_dist;		// distance from parent bone to this bone's pivot point
	int			flags;			// bit 0 is set if bone is a tag
} mds_dbone_t;

typedef struct
{
	int			ident;
	int			version;

	char			name[MDS_MAX_PATH];
	
	float			lod_scale;
	float			lod_bias;
	
	int			frames_num;
	int			bones_num;
	
	int			frames_ofs;
	int			bones_ofs;
	
	int			torso_parent;
	
	int			meshes_num;
	int			meshes_ofs;
	
	int			tags_num;
	int			tags_ofs;
	
	int			end_ofs;		// end of file
} mds_dheader_t;


typedef struct
{
	char			ident[4];
	
	char 			name[MDS_MAX_PATH];
	char			shader[MDS_MAX_PATH];	// name of shader
	int			shader_index;
	
	int			min_lod;
	int			header_ofs;
	
	int			vertexes_num;
	int			vertexes_ofs;
	
	int			triangles_num;
	int			triangles_ofs;
	
	int			collapsemap_ofs;
	
	int			bones_num;
	int			bones_ofs;
	
	int			end_ofs;		// next surface follows
} mds_dmesh_t;


/*
================================================================================
			MD5 skeletal file format
================================================================================
*/

#define MD5_IDENTSTRING		"MD5Version"
#define MD5_VERSION		10


/*
================================================================================
			QRazor FX's own BSP format
================================================================================
*/
#define BSP_IDENTSTRING		"XBSP"
#define BSP_IDENT		(('P'<<24)+('S'<<16)+('B'<<8)+'X')	// little-endian "XBSP"
#define BSP_VERSION		1


// upper design bounds
// leaffaces, leafbrushes, planes, and verts are still bounded by
// 16 bit short limits
#define	MAX_BSP_MODELS		0x400
#define	MAX_BSP_BRUSHES		0x8000
#define	MAX_BSP_ENTITIES	0x800
#define	MAX_BSP_ENTSTRING	0x40000
#define MAX_BSP_SHADERS		0x400

#define	MAX_BSP_AREAS		0x100
#define MAX_BSP_FOGS		0x100
#define	MAX_BSP_PLANES		0x20000
#define	MAX_BSP_NODES		0x20000
#define	MAX_BSP_BRUSHSIDES	0x20000
#define	MAX_BSP_LEAFS		0x20000
#define	MAX_BSP_VERTEXES	0x80000
#define	MAX_BSP_SURFACES	0x20000
#define	MAX_BSP_LEAFSURFACES	0x20000
#define	MAX_BSP_LEAFBRUSHES 	0x40000
#define	MAX_BSP_INDEXES		0x80000
#define	MAX_BSP_LIGHTING	0x800000
#define	MAX_BSP_VISIBILITY	0x200000
#define	MAX_BSP_HEARABILITY	0x200000


// key / value pair sizes

#define	MAX_BSP_KEY		32
#define	MAX_BSP_VALUE		1024


#define	BSP_LUMP_ENTITIES		0
#define	BSP_LUMP_SHADERS		1
#define	BSP_LUMP_PLANES			2
#define	BSP_LUMP_NODES			3
#define	BSP_LUMP_LEAFS			4
#define	BSP_LUMP_LEAFSURFACES		5
#define	BSP_LUMP_LEAFBRUSHES		6
#define	BSP_LUMP_MODELS			7
#define	BSP_LUMP_BRUSHES		8
#define	BSP_LUMP_BRUSHSIDES		9
#define BSP_LUMP_VERTEXES		10
#define	BSP_LUMP_INDEXES		11
#define	BSP_LUMP_FOGS			12
#define	BSP_LUMP_SURFACES		13
#define	BSP_LUMP_LIGHTING		14
#define	BSP_LUMP_LIGHTGRID		15
#define	BSP_LUMP_VISIBILITY		16
#define BSP_LUMP_AREAS			17
#define BSP_LUMP_AREAPORTALS		18
#define	BSP_HEADER_LUMPS		19

#define BSP_PVS_HEADERSIZE	8


enum bsp_surface_type_t
{
	BSPST_BAD,
	BSPST_PLANAR,
	BSPST_BEZIER,
	BSPST_MESH,
	BSPST_FLARE,
	
	// used internally by QRazor-FX and not submitted by q3map2
	BSPST_PLANAR_NOCULL
};

struct bsp_lump_t
{
	int			fileofs, filelen;
};


struct bsp_dheader_t
{
	int			ident;
	int			version;	
	bsp_lump_t		lumps[BSP_HEADER_LUMPS];
};

typedef char bsp_dentdata_t;

struct bsp_dshader_t
{
	char			shader[64];			// shader name
	int			flags;
	int			contents;
};


struct bsp_dplane_t
{
	float			normal[3];
	float			dist;
};


struct bsp_dnode_t
{
	int			plane_num;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	int			mins[3];		// for frustom culling
	int			maxs[3];
};

struct bsp_dleaf_t
{
	int			cluster;
	int			area;
	
	int			mins[3];			// for frustum culling
	int			maxs[3];

	int			leafsurfaces_first;
	int			leafsurfaces_num;

	int			leafbrushes_first;
	int			leafbrushes_num;
};

typedef int bsp_dleafsurface_t;

typedef int bsp_dleafbrush_t;

struct bsp_dmodel_t
{
	float			mins[3];
	float			maxs[3];
	
	int			modelsurfaces_first;
	int			modelsurfaces_num;			// submodels just draw faces without walking the bsp tree
	
	int			brushes_first;
	int			brushes_num;
};

struct bsp_dbrush_t
{
	int			sides_first;
	int			sides_num;
	int			shader_num;
};

struct bsp_dbrushside_t
{
	int			plane_num;		// facing out of the leaf
	int			shader_num;
};

struct bsp_dvertex_t
{
	float			position[3];
	float			texcoord_st[2];
	float			texcoord_lm[2];
	float			normal[3];
	float			light[3];
	byte			color[4];
};

typedef int bsp_dindex_t;

struct bsp_dsurface_t
{
	int			shader_num;
	int			fog_num;
	int			face_type;

	int			vertexes_first;
	int			vertexes_num;
	
	int			indexes_first;
	int			indexes_num;
	
	int			lm_num;
	int			lm_x;
	int			lm_y;
	int			lm_w;
	int			lm_h;
	
	float			origin[3];		// BSPST_PLANAR only
	
	float			mins[3];		//
	float			maxs[3];		// BSPST_MESH only
	
	float			normal[3];		// BSPST_PLANAR only
	
	int			mesh_cp[2];		// BSPST_BEZIER only
};

typedef byte bsp_dvisdata_t;

/*
struct bsp_dvis_t
{
	int			clusters_num;
	int			cluster_size;
	byte			visdata[1];
};
*/



/*
================================================================================
				Doom3 CM format
================================================================================
*/

#define CM_IDENTSTRING		"CM"
#define CM_VERSION		1.0



/*
================================================================================
				Doom3 MAP format
================================================================================
*/

#define MAP_IDENTSTRING		"Version"
#define MAP_VERSION		2.0


/*
================================================================================
				Doom3 PROC format
================================================================================
*/

#define PROC_IDENTSTRING	"mapProcFile"
#define PROC_VERSION		"003"


#endif // FILES_H



