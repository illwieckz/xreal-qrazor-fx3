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
#ifndef R_LOCAL_H
#define R_LOCAL_H

/// includes ===================================================================
// system -------------------------------------------------------------------
/*
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <map>
*/
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility.hpp>
#include <boost/spirit/symbols/symbols.hpp>
#include <boost/spirit/tree/ast.hpp>
#include <boost/spirit/utility/lists.hpp>
#include <boost/spirit/utility/escape_char.hpp>

#ifdef WIN32
#include <windows.h>
#endif

//#define GL_GLEXT_PROTOTYPES
//#include <GL/gl.h>
//#include <GL/glext.h>

// qrazor-fx ----------------------------------------------------------------
#include "r_public.h"
#include "r_gl.h"

#include "files.h"

#include "img_tga.h"
#include "img_jpg.h"
#include "img_png.h"




#define	REF_VERSION	"GL 0.0.9"



extern	viddef_t	vid;





#define MAX_ARRAY_VERTEXES		4096*2
#define MAX_ARRAY_INDEXES		MAX_ARRAY_VERTEXES*6
#define MAX_ARRAY_TRIANGLES		MAX_ARRAY_VERTEXES/3
#define MAX_ARRAY_NEIGHBOURS		MAX_ARRAY_VERTEXES*3

#define MAX_SHADOW_INDEXES		MAX_ARRAY_VERTEXES*4

#define VBO_BUFFER_OFFSET(i) 		((char *)NULL + (i))


enum r_serr_e
{
	RSERR_OK,
	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,
	RSERR_UNKNOWN
};


enum
{
	IMAGE_NONE			= 0,
	IMAGE_CLAMP      		= 1 << 0,
	IMAGE_CLAMP_TO_EDGE		= 1 << 1,
	IMAGE_CLAMP_TO_BORDER		= 1 << 2,
	IMAGE_NOMIPMAP   		= 1 << 3,
	IMAGE_NOPICMIP   		= 1 << 4,
	IMAGE_NOCOMPRESSION		= 1 << 5,
	IMAGE_NORMALMAP			= 1 << 6,	// for normalmap mip mapping
	IMAGE_LIGHTMAP			= 1 << 7
};

enum r_image_upload_type_e
{	
	IMAGE_UPLOAD_COLORMAP,
	IMAGE_UPLOAD_ALPHAMAP,
	IMAGE_UPLOAD_BUMPMAP,
	IMAGE_UPLOAD_CUBEMAP,
	IMAGE_UPLOAD_VIDEOMAP
};

enum r_shader_type_e
{
	SHADER_2D,
	SHADER_3D,
	SHADER_PARTICLE,
	SHADER_LIGHT
};

enum r_shader_light_type_e
{
	SHADER_LIGHT_GENERIC,
	SHADER_LIGHT_AMBIENT,
	SHADER_LIGHT_FOG
};


// shader flags
enum
{
	SHADER_DEPTHWRITE		= 1 << 0,
	SHADER_SKY			= 1 << 1,
	SHADER_POLYGONOFFSET		= 1 << 2,
	SHADER_TWOSIDED			= 1 << 3,
	SHADER_NODRAW			= 1 << 4,
	SHADER_NOSHADOWS		= 1 << 5,
	SHADER_NOSELFSHADOW		= 1 << 6,
	SHADER_TRANSLUCENT		= 1 << 7,
	SHADER_DISCRETE			= 1 << 8,
	SHADER_AREAPORTAL		= 1 << 9
};


enum r_shader_material_stage_type_e
{
	SHADER_MATERIAL_STAGE_TYPE_COLORMAP,
	SHADER_MATERIAL_STAGE_TYPE_DIFFUSEMAP,
	SHADER_MATERIAL_STAGE_TYPE_BUMPMAP,
	SHADER_MATERIAL_STAGE_TYPE_SPECULARMAP,
	SHADER_MATERIAL_STAGE_TYPE_LIGHTMAP,
	SHADER_MATERIAL_STAGE_TYPE_DELUXEMAP,
	SHADER_MATERIAL_STAGE_TYPE_REFLECTIONMAP,
	SHADER_MATERIAL_STAGE_TYPE_REFRACTIONMAP,
	SHADER_MATERIAL_STAGE_TYPE_DISPERSIONMAP,
	SHADER_MATERIAL_STAGE_TYPE_LIQUIDMAP
};


enum r_shader_light_stage_type_e
{
	SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY,
	SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_Z,
	SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XYZ,
	SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_CUBE
};

// shader stage flags
enum
{
	SHADER_STAGE_DEPTHWRITE		= 1 << 0,
	SHADER_STAGE_BLEND		= 1 << 1,
	SHADER_STAGE_ALPHATEST		= 1 << 2,
	SHADER_STAGE_NOSHADOWS		= 1 << 3,
	SHADER_STAGE_NOMIPMAPS		= 1 << 4,
	SHADER_STAGE_NOPICMIP		= 1 << 5,
	SHADER_STAGE_CLAMP		= 1 << 6,
	SHADER_STAGE_ZEROCLAMP		= 1 << 7,
	SHADER_STAGE_FORCEHIGHQUALITY	= 1 << 8,
	SHADER_STAGE_MASKALPHA		= 1 << 9,
	SHADER_STAGE_MASKCOLOR		= 1 << 10,
	SHADER_STAGE_MASKDEPTH		= 1 << 11,
	SHADER_STAGE_VIDEOMAP		= 1 << 12
};


enum r_shader_tcmod_type_e
{
	SHADER_TCMOD_NONE,
	SHADER_TCMOD_SCALE,
	SHADER_TCMOD_SCALE3D,
	SHADER_TCMOD_CENTERSCALE,
	SHADER_TCMOD_SCROLL,
	SHADER_TCMOD_ROTATE,
	SHADER_TCMOD_TRANSLATE,
	SHADER_TCMOD_SHEAR
};

// shader sort types
enum r_shader_sort_type_e
{
	SHADER_SORT_FARTHEST	= 0,
	SHADER_SORT_FAR		= 1,
	SHADER_SORT_CLOSE	= 5,
	SHADER_SORT_DECAL	= 6,
	SHADER_SORT_NEAREST	= 16
};

enum r_shader_deform_type_e
{
	SHADER_DEFORM_NONE,
	SHADER_DEFORM_FLARE,
	SHADER_DEFORM_AUTOSPRITE,
};

// rgb generation flags
enum r_rgbgen_type_e
{
	SHADER_RGB_GEN_IDENTITY,
	SHADER_RGB_GEN_CUSTOM,
	SHADER_RGB_GEN_ENTITY
};

// alpha channel generation flags
enum r_alphagen_type_e
{
	SHADER_ALPHA_GEN_IDENTITY,
	SHADER_ALPHA_GEN_CUSTOM,
	SHADER_ALPHA_GEN_ENTITY
};


typedef unsigned long long r_sortkey_t;


//
// interface
//
extern ref_import_t	ri;


//
// forward declarations
//
class r_shader_c;
class r_command_t;
class r_model_c;
class r_entity_c;
class r_light_c;
class r_surface_c;
class r_proc_model_c;
class r_bsptree_leaf_c;


enum r_shader_parms_e
{
	SHADER_PARM_TIME,
	
	SHADER_PARM_PARM0,
	SHADER_PARM_PARM1,
	SHADER_PARM_PARM2,
	SHADER_PARM_PARM3,
	SHADER_PARM_PARM4,
	SHADER_PARM_PARM5,
	SHADER_PARM_PARM6,
	SHADER_PARM_PARM7,
	
	SHADER_PARM_GLOBAL0,
	SHADER_PARM_GLOBAL1,
	SHADER_PARM_GLOBAL2,
	
	SHADER_PARM_SOUND
};


enum
{
	SHADER_GENERIC_RULE_REAL		= 1,
	SHADER_GENERIC_RULE_PARM,
	SHADER_GENERIC_RULE_TABLE_EVAL,
	SHADER_GENERIC_RULE_TABLE_INDEX,
	SHADER_GENERIC_RULE_TABLE_VALUE,
	SHADER_GENERIC_RULE_FACTOR,
	SHADER_GENERIC_RULE_TERM,
	SHADER_GENERIC_RULE_EXPRESSION
};

enum
{
	SHADER_TABLE_CLAMP			= 1 << 0,
	SHADER_TABLE_SNAP			= 1 << 1
};

enum r_tree_type_e
{
	TREE_BSP,
	TREE_PROC
};


enum r_render_type_e
{
	RENDER_TYPE_DEFAULT,
	RENDER_TYPE_OCCLUSION_QUERY,
	RENDER_TYPE_GENERIC,
	RENDER_TYPE_ZFILL,
	RENDER_TYPE_REFLECTION,
	RENDER_TYPE_REFRACTION,
	RENDER_TYPE_DISPERSION,
	RENDER_TYPE_LIQUID,
	RENDER_TYPE_SHADOWING,
	RENDER_TYPE_LIGHTING_R,
	RENDER_TYPE_LIGHTING_RB,
	RENDER_TYPE_LIGHTING_RBH,
	RENDER_TYPE_LIGHTING_RBHS,
	RENDER_TYPE_LIGHTING_RBS,
	RENDER_TYPE_LIGHTING_D_omni,
	RENDER_TYPE_LIGHTING_D_proj,
	RENDER_TYPE_LIGHTING_DB_omni,
	RENDER_TYPE_LIGHTING_DBH_omni,
	RENDER_TYPE_LIGHTING_DBHS_omni,
	RENDER_TYPE_LIGHTING_DBS_omni
};

enum
{
	FRUSTUM_LEFT		= 0,
	FRUSTUM_RIGHT,
	FRUSTUM_BOTTOM,
	FRUSTUM_TOP,
	FRUSTUM_NEAR,
	FRUSTUM_FAR,
	FRUSTUM_PLANES		= 6,
	FRUSTUM_CLIPALL		= 1 | 2 | 4 | 8 | 16 | 32
};

typedef cplane_c	r_frustum_t[FRUSTUM_PLANES];


typedef float							r_node_data_t;

typedef std::string::const_iterator				r_iterator_t;
typedef boost::spirit::node_val_data_factory<r_node_data_t>	r_factory_t;

extern uint_t		r_registration_sequence;
extern uint_t		r_framecount;
extern uint_t		r_visframecount;			// bumped when going to a new PVS


class r_image_c
{
public:
	//
	// constructor / destructor
	//
	r_image_c(uint_t target, const std::string &name, uint_t width, uint_t height, uint_t flags, roq_info_t *roq);
		
	~r_image_c();
	
	inline const char*	getName() const			{return _name.c_str();}
	inline uint_t		getTarget() const 		{return _target;}
	
	inline uint_t		getWidth() const		{return _width;}
	inline uint_t		getHeight() const 		{return _height;}
	
	inline uint_t		getFlags() const		{return _flags;}
	inline bool		hasFlags(uint_t flags) const 	{return _flags & flags;}
	
	inline uint_t		getId() const			{return _id;}
	
	inline uint_t		getRegistrationSequence()	{return _registration_sequence;}
	inline void		setRegistrationSequence()	{_registration_sequence = r_registration_sequence;}
	
	inline bool		hasVideo() const		{return _roq ? true : false;}
	
	void			updateTexture();
	
private:
	void			convertColors();

	std::string		_name;
	uint_t			_target;
	uint_t			_width;
	uint_t			_height;
	uint_t			_flags;
	uint_t			_id;
	uint_t			_registration_sequence;
	roq_info_t*		_roq;
};

struct r_tcmod_t
{
	r_tcmod_t()
	{
		clear();
	}
	
	inline void	clear()
	{
		type	= SHADER_TCMOD_NONE;
	}

	r_shader_tcmod_type_e						type;
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	x;	// generic AST expression
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	y;
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	z;
};



struct r_wedge_t
{
	r_wedge_t()	{}
	
	r_wedge_t(int v0, int v1, int plane0, int plane1)
	{
		v[0] = v0;
		v[1] = v1;
		
		plane[0] = plane0;
		plane[1] = plane1;
	}
	
	int	v[2];			// vertex index
	int	plane[2];		// plane index
	//int	tri[2];			// triangle index: for "open" models, w[1] == -1 on open edges
};

/*
struct r_triangle_t
{
	inline r_triangle_t(index_t v0, index_t v1, index_t v2, uint_t n)
	{
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		
		num = n;
	}
	
	inline index_t	operator [] (const int index) const
	{
		return v[index];
	}
	
	inline index_t&	operator [] (const int index)
	{
		return v[index];
	}
	
	index_t		v[3];
	uint_t		num;
};
*/

class r_mesh_c
{
public:
	r_mesh_c();
	virtual ~r_mesh_c();
	
	void		fillVertexes(int vertexes_num, bool with_lightmap_texcoords = false);
	bool		isNotValid() const;
	void		calcTangentSpaces();
	void		calcTangentSpaces2();
	void		calcTangentSpaces3();
	void		createBBoxFromVertexes();
	
	void		addVertex(const vec3_c &v);
	
	void		addTriangle(index_t v0, index_t v1, index_t v2, uint_t num);
	void		addTriangle(index_t v0, index_t v1, index_t v2);
	
	bool		hasTriangle(uint_t num);
	
private:
	void		addEdge(const r_wedge_t &we);
	void		addEdge(int v0, int v1, int plane);
	int		cap();
public:
	void		calcEdges();
	
	// vertex data
	std::vector<vec3_c>		vertexes;
	std::vector<vec2_c>		texcoords;
	std::vector<vec2_c>		texcoords_lm;
	std::vector<vec3_c>		tangents;
	std::vector<vec3_c>		binormals;
	std::vector<vec3_c>		normals;
	std::vector<vec4_c>		colors;
	
	// triangles data
	std::vector<uint_t>		triangles;	// triangle numbers
	std::vector<index_t>		indexes;	// raw triangle indices data for OpenGL
	std::vector<vec3_c>		normals_tri;
	std::vector<cplane_c>		planes;
	std::vector<r_wedge_t>		edges;
	std::list<r_wedge_t>		edges_unmatched;
	
	// updated every time silhouettes edges are computed
	std::vector<bool>		triangles_sh;
	std::vector<int>		edges_sh;
	
	// culling
	cbbox_c				bbox;
	
	// skin support
	std::string			name;
	
	// vertex buffer object support
	GLuint				vbo_array_buffer;
	GLuint				vbo_vertexes_ofs;
	GLuint				vbo_texcoords_ofs;
	GLuint				vbo_texcoords_lm_ofs;
	GLuint				vbo_tangents_ofs;
	GLuint				vbo_binormals_ofs;
	GLuint				vbo_normals_ofs;
	GLuint				vbo_colors_ofs;
	
	GLuint				vbo_element_array_buffer;
	GLuint				vbo_indexes_ofs;
};



class r_shader_stage_c
{
public:
	r_shader_stage_c();
	~r_shader_stage_c();
	
	r_shader_material_stage_type_e	type;
	r_shader_light_stage_type_e	type_light;
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	condition;
	
	uint_t				flags;
	
	uint_t				blend_src;
	uint_t				blend_dst;
	
	r_rgbgen_type_e							rgb_gen;
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	red;
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	green;
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	blue;
	
	r_alphagen_type_e						alpha_gen;
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	alpha;
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	alpha_ref;
			
	std::vector<r_tcmod_t>						tcmod_cmds;
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	bump_scale;
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	height_scale;
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	height_bias;
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	specular_exponent;
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	refraction_index;
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	fresnel_power;
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	fresnel_scale;
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	fresnel_bias;
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	eta;
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	eta_delta;
	
	bool				make_intensity;
	bool				make_alpha;
	
	std::string			image_name;
	r_image_c*			image;
};


class r_shader_c
{
public:
	r_shader_c(const std::string &name, r_shader_type_e type);
	
	~r_shader_c();
	
	inline const char*		getName() const			{return _name.c_str();}
	inline r_shader_type_e		getType() const 		{return _type;}
	inline r_shader_light_type_e	getLightType() const		{return _type_light;}
	inline void			setLightType(r_shader_light_type_e type)	{_type_light = type;}
	
	inline int			getSort() const				{return _sort;}
	inline void			setSort(int sort)			{_sort = sort;}
	
	inline r_shader_deform_type_e	getDeformType() const				{return _deform;}
	inline void			setDeformType(r_shader_deform_type_e deform)	{_deform = deform;}
	
	inline uint_t			getFlags() const		{return _flags;}
	inline bool			hasFlags(uint_t flags) const 	{return _flags & flags;}
	inline void			setFlags(uint_t flags)		{_flags = flags;}
	inline void			setORFlags(uint_t flags)	{_flags |= flags;}
	inline void			removeFlags(uint_t flags)	{_flags &= ~flags;}
	
	inline uint_t			getRegistrationSequence()	{return _registration_sequence;}
	inline void			setRegistrationSequence()	{_registration_sequence = r_registration_sequence;}
	
private:
	std::string			_name;
	r_shader_type_e			_type;
	r_shader_light_type_e		_type_light;	// only used if shader is a light shader
	
	int				_sort;
	r_shader_deform_type_e		_deform;
	uint_t				_flags;
	uint_t				_registration_sequence;
	
public:
	// used if material shader
	r_shader_stage_c*		stage_diffusemap;
	r_shader_stage_c*		stage_bumpmap;
	r_shader_stage_c*		stage_specularmap;
	
	r_shader_stage_c*		stage_lightmap;
	r_shader_stage_c*		stage_deluxemap;
	
	// used if light shader
	r_shader_stage_c*		stage_attenuationmap_z;
	r_shader_stage_c*		stage_attenuationmap_cube;
		
	std::vector<r_shader_stage_c*>	stages;
};


class r_model_shader_c
{
public:
	r_model_shader_c(const std::string &name, r_shader_c *shader, uint_t flags, uint_t contents)
	{
		_name		= name;
		_shader		= shader;
		_flags		= flags;
		_contents	= contents;
	}
	
	inline const char*	getName() const		{return _name.c_str();}
	
	inline r_shader_c*	getShader()		{return _shader;}
	
	inline uint_t		getFlags() const		{return _flags;}
	inline bool		hasFlags(uint_t flags)		{return _flags & flags;}
	
	inline uint_t		getContents() const		{return _contents;}
	inline bool		hasContents(uint_t contents)	{return _contents & contents;}	
	
private:
	std::string		_name;
	r_shader_c*		_shader;
	uint_t			_flags;
	uint_t			_contents;
};

class r_model_skin_c
{
public:
	r_model_skin_c(const std::string &name)
	{
		_name = name;
		_registration_sequence = r_registration_sequence;
	}

	inline const char*	getName() const	{return _name.c_str();}
	
	inline uint_t		getRegistrationSequence()	{return _registration_sequence;}
	inline void		setRegistrationSequence()	{_registration_sequence = r_registration_sequence;}
	
	inline void		addShader(const std::string &meshname, r_shader_c *shader)
	{
		_shaders.insert(make_pair(meshname, shader));
	}
	
	inline r_shader_c*	getShader(const std::string &name)
	{
		for(std::map<std::string, r_shader_c*>::iterator ir = _shaders.begin(); ir != _shaders.end(); ir++)
		{
			if(X_strcaseequal(ir->first.c_str(), name.c_str()))
				return ir->second;
		}
		
		return NULL;
	}
	
	inline void		updateShaders()
	{
		for(std::map<std::string, r_shader_c*>::iterator ir = _shaders.begin(); ir != _shaders.end(); ir++)
		{
			ir->second->setRegistrationSequence();
		}
	}
	

private:
	std::string		_name;
	unsigned int		_registration_sequence;
	std::map<std::string, r_shader_c*>	_shaders;
};


class r_interaction_c
{
public:
	inline r_interaction_c(r_surface_c *surf, r_light_c *light, std::vector<index_t> &indexes)
	{
		_surf		= surf;
		_light		= light;
		_indexes	= indexes;
	}
	
	inline r_surface_c*	getSurface() const	{return _surf;}
	inline r_light_c*	getLight() const	{return _light;}
	inline const std::vector<index_t>&	getIndexes() const	{return _indexes;}
	
private:
	r_surface_c*		_surf;
	r_light_c*		_light;
	std::vector<index_t>	_indexes;
};

class r_surface_c
{
	friend class r_bsptree_c;
public:
	r_surface_c();
	
	inline uint_t			getFrameCount() const	{return _framecount;}
	inline void			setFrameCount()		{_framecount = r_framecount;}

	inline r_mesh_c*		getMesh() const		{return _mesh;}
	inline void			setMesh(r_mesh_c *mesh)	{_mesh = mesh;}
	
	inline r_model_shader_c*	getShaderRef() const	{return _shaderref;}
	inline void			setShaderRef(r_model_shader_c *ref)	{_shaderref = ref;}
	inline r_shader_c*		getShader() const	{return _shaderref ? _shaderref->getShader() : NULL;}
	
	inline bsp_surface_type_t	getFaceType() const	{return _facetype;}
	inline const cplane_c&		getPlane() const	{return _plane;}
	
	inline uint_t			getLightMapNum() const	{return _lightmap;}

private:
	uint_t			_framecount;		// should be drawn when node is crossed

	r_mesh_c*		_mesh;
	r_model_shader_c*	_shaderref;

	bsp_surface_type_t	_facetype;
	cplane_c		_plane;			// if BSPST_PLANAR
	int			_lightmap;		// if BSP surface
};

// virtual screen coordinates
struct r_vrect_t
{
	inline r_vrect_t()
	{
		x	= 0;
		y	= 0;
		
		width	= 0;
		height	= 0;
	}
	
	int	x, y;
	int	width, height;
};

class r_scissoriface_a
{
public:
	void			updateScissor(const matrix_c &mvp, const r_vrect_t &vrect, const cbbox_c &bbox);
	void			setScissor(const r_vrect_t &vrect);
	
private:
	void			addVertex(const matrix_c &mvp, const r_vrect_t &vrect, const vec3_c &v);
	void			addEdge(const matrix_c &mvp, const r_vrect_t &vrect, const vec3_c &v0, const vec3_c &v1);
public:

	//inline const vec4_c&	getScissorCoords() const	{return _coords;}
	inline int		getScissorX() const		{return (int)_mins[0];}
	inline int		getScissorY() const		{return (int)_mins[1];}
	inline int		getScissorWidth() const		{return (int)(_maxs[0] - _mins[0]);}
	inline int		getScissorHeight() const	{return (int)(_maxs[1] - _mins[1]);}
	
private:
//	vec4_c		_coords;
	vec2_c		_mins;
	vec2_c		_maxs;
};

class r_visiface_a
{
protected:
	r_visiface_a()
	{
	}
	
public:
//	void		updateVis(const r_entity_t &shared);

	inline uint_t	getVisFrameCount() const	{return _visframecount;}
	inline void	setVisFrameCount(uint_t c)	{_visframecount = c;}
	inline void	setVisFrameCount()		{_visframecount = r_visframecount;}
	inline bool	isVisible() const		{return r_visframecount == _visframecount;}
	
	inline int	getCluster() const		{return _cluster;}
	inline int	getArea() const			{return _area;}
	inline const std::vector<r_bsptree_leaf_c*>&	getLeafs() const	{return _leafs;}
	
	inline const std::vector<int>&			getAreas() const	{return _areas;}
	
	
protected:
	uint_t		_visframecount;
	
	// if Q3A BSP used
	int				_cluster;
	int				_area;
	std::vector<r_bsptree_leaf_c*>	_leafs;
	
	// if Doom3 proc used
	std::vector<int>		_areas;
};

class r_occlusioniface_a
{
protected:
	r_occlusioniface_a()
	{
		_query = 0;
	
		if(xglGenQueriesARB)
			xglGenQueriesARB(1, &_query);
	}
	
	~r_occlusioniface_a()
	{
		if(xglDeleteQueriesARB && _query)
			xglDeleteQueriesARB(1, &_query);
	}
	
public:
	//inline uint_t		getQuery() const	{return _query;}
	
	inline void		beginOcclusionQuery() const	{xglBeginQueryARB(GL_SAMPLES_PASSED_ARB, _query);}	
	inline void		endOcclusionQuery() const	{xglEndQueryARB(GL_SAMPLES_PASSED_ARB);}
	
	inline uint_t		getOcclusionSamplesNum() const
	{
		uint_t samples;
		xglGetQueryObjectuivARB(_query, GL_QUERY_RESULT_ARB, &samples);
		return samples;
	}
	
private:
	uint_t		_query;
};


class r_entity_c :
public r_visiface_a,
public r_occlusioniface_a
{
public:
	r_entity_c();
	r_entity_c(const r_entity_t &shared, bool update);

	void			setupTransform();
	void			setupTransformLeftHanded();
	void			setupTransformToViewer();
	
	inline const r_entity_t&	getShared() const	{return _s;}
	
	inline const matrix_c&		getTransform() const	{return _transform;}
	
	inline void			setColor(const vec4_c &color)
	{
		_s.shader_parms[0] = color[0];
		_s.shader_parms[1] = color[1];
		_s.shader_parms[2] = color[2];
		_s.shader_parms[3] = color[3];
	}
	
	inline bool			needsUpdate() const	{return _needsupdate;}
	inline void			needsUpdate(bool b)	{_needsupdate = b;}
	
	

private:
	r_entity_t		_s;
		
	matrix_c		_transform;
	
	bool			_needsupdate;
};


class r_light_c :
public r_visiface_a,
public r_scissoriface_a,
public r_occlusioniface_a
{
	friend void	RB_AddCommand(	r_entity_c*		entity,
					r_model_c*		entity_model,
					r_mesh_c*		entity_mesh,
					r_shader_c*		entity_shader,
					r_light_c*		light,
					std::vector<index_t>*	light_indexes,
					int			infokey,
					vec_t			distance);

public:
	r_light_c(const r_entity_t &shared, r_light_type_t type);
	~r_light_c();

	void			setupTransform();
	void			setupAttenuation();
	void			setupProjection();
	void			setupFrustum();
	
	bool			hasSurface(int areanum, const r_surface_c *surf);
	void			addSurface(int areanum, const r_surface_c *surf);	// clear interaction if any exists
	
	
	
	inline const r_entity_t&	getShared() const	{return _s;}
	inline r_light_type_t		getType() const		{return _type;}
	
	inline const vec3_c&		getOrigin() const	{return _origin;}
	
	inline const matrix_c&		getTransform() const	{return _transform;}
	inline const matrix_c&		getAttenuation() const	{return _attenuation;}
	inline const matrix_c&		getProjection() const	{return _projection;}
	
	inline const r_frustum_t&	getFrustum() const	{return _frustum;}
	
	inline bool			needsUpdate() const	{return _needsupdate;}
	inline void			needsUpdate(bool b)	{_needsupdate = b;}
	
	inline void			setAreaNum(int areanum)
	{
		_areasurfaces = std::vector<std::map<const r_surface_c*, std::vector<index_t> > >(areanum ? areanum : 1);
	}
	
	inline const std::map<const r_surface_c*, std::vector<index_t> >&
	getAreaSurfaces(int areanum) const
	{
		return _areasurfaces[areanum];
	}

private:
	r_entity_t		_s;
	r_light_type_t		_type;
	
	vec3_c			_origin;
		
	matrix_c		_transform;
	matrix_c		_attenuation;
	matrix_c		_projection;
		
	r_frustum_t		_frustum;
	
	bool			_needsupdate;
						
	std::vector<std::map<const r_surface_c*, std::vector<index_t> >	>	_areasurfaces;
};


class r_command_t
{
	friend class r_light_c;

	friend void	RB_AddCommand(	r_entity_c*		entity,
					r_model_c*		entity_model,
					r_mesh_c*		entity_mesh,
					r_shader_c*		entity_shader,
					r_light_c*		light,
					std::vector<index_t>*	light_indexes,
					int			infokey,
					vec_t			distance);

public:
	r_command_t();

	r_command_t(	r_entity_c*		entity,
			r_model_c*		entity_model,
			r_mesh_c*		entity_mesh,
			r_shader_c*		entity_shader,
			r_light_c*		light,
			r_shader_c*		light_shader,
			std::vector<index_t>*	light_indexes,
			int			infokey,
			vec_t			distance);
			
	inline r_entity_c*		getEntity() const	{return _entity;}
	inline r_model_c*		getEntityModel() const	{return _entity_model;}
	inline r_mesh_c*		getEntityMesh() const	{return _entity_mesh;}
	inline r_shader_c*		getEntityShader() const	{return _entity_shader;}
	
	inline r_light_c*		getLight() const	{return _light;}
	inline r_shader_c*		getLightShader() const	{return _light_shader;}
	inline std::vector<index_t>*	getLightIndexes() const	{return _light_indexes;}
	
	inline const matrix_c&		getTransform() const	{return _transform;}
		
	inline int			getInfoKey() const	{return _infokey;}
	
	inline vec_t			getDistance() const	{return _distance;}
	
private:
	r_entity_c*		_entity;
	r_model_c*		_entity_model;
	r_mesh_c*		_entity_mesh;
	r_shader_c*		_entity_shader;
	
	r_light_c*		_light;
	r_shader_c*		_light_shader;
	std::vector<index_t>*	_light_indexes;
	
	matrix_c		_transform;		// used by autosprites, so we don't have to recalc
							// the transform every time we draw the entity
	
	int			_infokey;
	
	vec_t			_distance;		// for sort by distance
};


struct r_scene_t
{
	uint_t				cmds_num;
	std::vector<r_command_t>	cmds;
	
	uint_t				cmds_radiosity_num;
	std::vector<r_command_t>	cmds_radiosity;
	
	uint_t				cmds_light_num;
	std::vector<r_command_t>	cmds_light;
	
	uint_t				cmds_translucent_num;
	std::vector<r_command_t>	cmds_translucent;
};



class r_tree_elem_c
{
public:
	// wether if node a leaf, all tree elements have this
	int			contents;		// -1, to differentiate from leafs
	cbbox_c			bbox;			// for bounding box culling
	
	r_tree_elem_c*		parent;
	
	uint_t			visframecount;		// node needs to be traversed if current
};


class r_node_c : public r_tree_elem_c
{
public:	
	//TODO
};

class r_leaf_c : public r_tree_elem_c
{
public:	
	std::vector<r_surface_c*>	surfaces;
	std::vector<r_light_c*>		lights;
};


class r_bsptree_node_c : public r_node_c
{
public:
	cplane_c*		plane;
	r_tree_elem_c*		children[2];
};
	
class r_bsptree_leaf_c : public r_leaf_c
{
public:
	int			cluster;
	int			area;
};

/*
class r_bsptree_area_c : public r_tree_elem_c
{
public:
	std::vector<r_surface_c*>	surfaces;
	std::vector<r_light_c*>		lights;
};
*/

/*
class r_bsptree_portal_c
{
public:
	void			adjustFrustum();
	
	r_bsptree_leaf_c*	leafs[2];
	r_bsptree_area_c*	areas[2];
	bool			areaportal;
	
	vec3_c			center;
	cbbox_c			bbox;
	cplane_c		plane;
	
	std::vector<vec3_c>	points;
	std::vector<vec3_c>	points_inner;
	
	uint_t			visframe;
	r_frustum_t		frustum;
};
*/

struct r_bsptree_model_t
{
	cbbox_c			bbox;
	float			radius;
	
	int			modelsurfaces_first;
	int			modelsurfaces_num;
};


class r_proctree_node_c : public r_node_c
{
public:
	cplane_c		plane;
	int			children[2];
};


class r_areaportal_c
{
public:
	void			adjustFrustum();
		
	void			draw();
	
	uint_t			visframe;

	int			areas[2];
	
	cbbox_c			bbox;
	cplane_c		plane;
	
	std::vector<vec3_c>	points;
	
	r_frustum_t		frustum;
};

class r_proctree_area_c : public r_leaf_c
{
public:
	r_proc_model_c*			model;
	
	std::vector<r_areaportal_c*>	areaportals;
};


class r_tree_c
{
public:
	virtual void		precacheLight(r_light_c *light) = 0;
	virtual void		update() = 0;
	virtual void		draw() = 0;
};


class r_bsptree_c  : public r_tree_c
{
	friend class r_bsp_model_c;
public:
	r_bsptree_c(const std::string &name);
	virtual ~r_bsptree_c();
	
	virtual void		precacheLight(r_light_c *light);
	virtual void		update();
	virtual void		draw();
	
	r_bsptree_leaf_c*	pointInLeaf(const vec3_c &p);
	int			pointInCluster(const vec3_c &p);
	bool			pointIsVisible(const vec3_c &p);
//	r_bsptree_area_c*	pointInArea(const vec3_c &p);

	// Fills in a list of all the leafs touched
	void			boxLeafs_r(const cbbox_c &bbox, std::vector<r_bsptree_leaf_c*> &leafs, r_tree_elem_c *elem);
	void			boxLeafs(const cbbox_c &bbox, std::vector<r_bsptree_leaf_c*> &leafs);

	
	
private:
	void			loadVisibility(const byte *buffer, const bsp_lump_t *l);
	void			loadVertexes(const byte *buffer, const bsp_lump_t *l);
	void			loadModels(const byte *buffer, const bsp_lump_t *l);
	void			loadShaders(const byte *buffer, const bsp_lump_t *l);
	void			loadSurfaces(const byte *buffer, const bsp_lump_t *l);
	void			loadNodes(const byte *buffer, const bsp_lump_t *l);
	void			loadLeafs(const byte *buffer, const bsp_lump_t *l);
	void			loadLeafSurfaces(const byte *buffer, const bsp_lump_t *l);
	void			loadIndexes(const byte *buffer, const bsp_lump_t *l);
	void			loadPlanes(const byte *buffer, const bsp_lump_t *l);
	
//	void			loadPortals(char **buf_p);
//	void			loadPortal(char **buf_p);
//	void			createAreas();
	r_mesh_c*		createMesh(const bsp_dsurface_t *in);
	r_mesh_c*		createBezierMesh(const bsp_dsurface_t *in);
	
	void			setParent(r_tree_elem_c *elem, r_tree_elem_c *parent);

	void			drawNode_r(r_tree_elem_c *elem, int clipflags);
	void			litNode_r(r_tree_elem_c *elem, r_light_c *light, bool precache);
	
	void			addSurfaceToList(r_surface_c *surf, int clipflags);

	void			markLeaves();
	void			markLights();
	void			markEntities();
	
	byte*			clusterPVS(int cluster);
		
	//
	// members
	//
private:
	std::string				_name;

	std::vector<r_surface_c*>		_surfaces;
	
	std::vector<r_surface_c*>		_surfaces_leaf;
	
	std::vector<r_model_shader_c*>		_shaders;
	
	std::vector<r_bsp_model_c*>		_models;	// first model is main world model
	
	std::vector<r_bsptree_node_c*>		_nodes;
	
	std::vector<r_bsptree_leaf_c*>		_leafs;
	
//	std::vector<r_bsptree_area_c*>		_areas;
	
//	std::vector<r_bsptree_portal_c*>	_portals;
	
	
	std::vector<vec3_c>			_vertexes;
	std::vector<vec2_c>			_texcoords;
	std::vector<vec2_c>			_texcoords_lm;
	std::vector<vec3_c>			_tangents;
	std::vector<vec3_c>			_binormals;
	std::vector<vec3_c>			_normals;
	
	std::vector<index_t>			_indexes;

	int					_planes_num;
	cplane_c*				_planes;
	
	std::vector<byte>			_pvs;
	int					_pvs_clusters_num;
	int					_pvs_clusters_size;
	
	int					_viewcluster;
	int					_viewcluster_old;
	
	GLuint					_vbo_array_buffer;
	GLuint					_vbo_element_array_buffer;
};



class r_proctree_c  : public r_tree_c
{
public:
	r_proctree_c(const std::string &name);
	virtual ~r_proctree_c();
	
	virtual void		precacheLight(r_light_c *light);
	virtual void		update();
	virtual void		draw();
	
	int			pointInArea_r(const vec3_c &p, int num);
	int			pointInArea(const vec3_c &p);
	
	void			boxAreas_r(const cbbox_c &bbox, std::vector<int> &areas, int nodenum);
	void			boxAreas(const cbbox_c &bbox, std::vector<int> &areas);
	
private:
	void			drawArea_r(int area, const r_frustum_t frustum, int clipflags);
	void			litArea_r(int area, r_light_c *light);
	
	void			markAreas();
	void			markLights();
	void			markEntities();

	void			loadNodes(char **buf_p);
	void			loadInterAreaPortals(char **buf_p);
		
	//
	// members
	//
	int					_sourcearea;	// the portal with the render origin
	
	std::vector<r_areaportal_c*>		_areaportals;

	std::vector<r_proctree_area_c*>		_areas;

	std::vector<r_proctree_node_c*>		_nodes;
};



//
// Whole model
//
enum r_mod_type_t
{
	MOD_BAD,
	MOD_BSP,
	MOD_STATIC,
	MOD_ALIAS,
	MOD_SKEL,
	MOD_POLY,
	MOD_PROC
};


//
// common for animated models
//

//
// alias model specific
//
struct r_alias_tag_t
{
	std::string		name;
	vec3_c			origin;
	quaternion_c		quat;
};

struct r_alias_mesh_frame_t
{
	~r_alias_mesh_frame_t();
	
	void		calcTangentSpaces(const std::vector<index_t> &indexes);
	
	std::vector<vec3_c>	vertexes;
	std::vector<vec2_c>	texcoords;
	std::vector<vec3_c>	tangents;
	std::vector<vec3_c>	binormals;
	std::vector<vec3_c>	normals;
};

struct r_alias_frame_t
{	
	~r_alias_frame_t();

	cbbox_c			bbox;
	float			radius;
	vec3_c			translate;
	
	
	std::vector<r_alias_mesh_frame_t*>	meshframes;
	std::vector<r_alias_tag_t*>		tags;
};



//
// skeletal model specific
//
/*
struct r_skel_bone_frame_t
{
	quaternion_c		rot_quat;
	quaternion_c		ofs_quat;
};
*/

/*
struct r_skel_frame_t
{	
	cbbox_c			bbox;
	float			radius;
	vec3_c			translate;
	
	vec3_c			parent_offset;
	std::vector<r_skel_bone_frame_t*>	boneframes;
};
*/


struct r_skel_channel_t;

struct r_skel_bone_t
{
	r_skel_bone_t()
	{
		parent_index = -1;
		channels = std::vector<r_skel_channel_t*>(6);
	}

	std::string		name;
//	std::string		parent_name;
	int			parent_index;		// bone parent index (-1 if root)
		
	vec3_c			position;
	matrix_c		matrix;
	quaternion_c		quat;
	
	std::vector<r_skel_bone_t*>	children;
	std::vector<r_skel_channel_t*>	channels;
};

struct r_skel_channel_t
{
	std::string		joint;
	
	chan_attrib_type_t	attribute;
	
	float			time_start;
	float			time_end;
	float			time_fps;
	
	int			strings;
	int			range[2];
	
	std::vector<float>	keys;
};

class r_skel_animation_c
{
public:
	r_skel_animation_c(const std::string &name);
	~r_skel_animation_c();
	
	void	loadChannels(char **data_p);
	
	inline const char*	getName()			{return _name.c_str();}
	inline uint_t		getRegistrationSequence()	{return _registration_sequence;}
	inline void		setRegistrationSequence()	{_registration_sequence = r_registration_sequence;}

private:
	std::string		_name;
	uint_t			_registration_sequence;

public:
	std::vector<r_skel_channel_t*>	_channels;
};

/*
struct r_skel_tag_t
{
	std::string		name;
	float			torso_weight;
	int			bone_index;
};
*/

struct r_skel_weight_t
{
	int			bone_index;
	float			weight;
	vec3_c			position;
};


class r_skel_mesh_c : public r_mesh_c
{
public:
	std::vector<int>		weights_first;		// for each vertex
	std::vector<int>		weights_num;		// for each vertex
	std::vector<r_skel_weight_t>	weights;		// for this mesh
	
	std::vector<std::vector<r_skel_weight_t*> >	vertexweights;
};



//
// model classes
//
class r_model_c
{
public:
	//
	// constructor / destructor
	//
	r_model_c(const std::string &name, byte *buffer, uint_t buffer_size, r_mod_type_t type);
	virtual ~r_model_c();
	
	//
	// virtual functions
	//
	virtual void	load()									{}
	virtual	void	precacheLightInteractions(r_entity_c *ent)				{}
	virtual void	updateBBox(r_entity_c *ent)						{}
	virtual void	addModelToList(r_entity_c *ent) = 0;
	virtual void 	draw(const r_command_t *cmd, r_render_type_e type) = 0;
	virtual void	setupMeshes();
	virtual bool	setupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name)	{return false;}
	virtual bool	setupAnimation(r_skel_animation_c *anim)				{return false;}
	
	//
	// access
	//
	const char*	getName() const			{return _name.c_str();}
	unsigned int	getRegistrationSequence() const	{return _registration_sequence;}
	void		setRegistrationSequence()	{_registration_sequence = r_registration_sequence;}
	r_mod_type_t	getType() const			{return _type;}
	const cbbox_c&	getBBox() const			{return _bbox;}
	
	void		addMesh(r_mesh_c* mesh)			{_meshes.push_back(mesh);}
	void		addShader(r_model_shader_c* shader)	{_shaders.push_back(shader);}
		
	//
	// members
	//
	
private:
	std::string	_name;
	uint_t		_registration_sequence;
protected:
	r_mod_type_t	_type;
	byte*		_buffer;	// for loading
	uint_t		_buffer_size;
	
	cbbox_c		_bbox;		// bbox that bounds around every mesh and surface
	
	
	std::vector<r_mesh_c*>		_meshes;
	std::vector<r_model_shader_c*>	_shaders;
};


class r_static_model_c : public r_model_c
{
public:
	//
	// constructor / destructor
	//
	r_static_model_c(const std::string &name, byte *buffer, uint_t buffer_size);
	virtual ~r_static_model_c();
	
	//
	// virtual functions
	//
	virtual void	addModelToList(r_entity_c *ent);
	virtual void 	draw(const r_command_t *cmd, r_render_type_e type);
};


class r_ase_model_c : public r_static_model_c
{
public:
	//
	// constructor / destructor
	//
	r_ase_model_c(const std::string &name, byte *buffer, uint_t buffer_size);
	virtual ~r_ase_model_c();
	
	//
	// virtual functions
	//
	virtual void	load();
	
private:
	void		loadMaterials(char **data_p);
	void		loadMaterial(char **data_p);
	void		skipDiffuseMap(char **data_p);
	void		skipSubMaterial(char **data_p);
	
	void		loadGeomObject(char **data_p);
	void		loadMesh(char **data_p);
};


class r_alias_model_c : public r_model_c
{
public:
	//
	// constructor / destructor
	//
	r_alias_model_c(const std::string &name, byte *buffer, uint_t buffer_size);
	virtual ~r_alias_model_c();
	
	//
	// virtual functions
	//
	virtual void	updateBBox(r_entity_c *ent);
	virtual void	addModelToList(r_entity_c *ent);
	virtual void 	draw(const r_command_t *cmd, r_render_type_e type);
	virtual void	setupMeshes();
	virtual bool	setupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name);
	
protected:
	bool	cull(r_entity_c *ent);
	void	drawFrameLerp(const r_command_t *cmd, r_render_type_e type);


	std::vector<r_alias_frame_t*>	_frames;
};



class r_md2_model_c : public r_alias_model_c
{
public:
	//
	// constructor / destructor
	//
	r_md2_model_c(const std::string &name, byte *buffer, uint_t buffer_size);
	virtual ~r_md2_model_c();
	
	//
	// virtual functions
	//
	virtual void	load();
};


class r_md3_model_c : public r_alias_model_c
{
public:
	//
	// constructor / destructor
	//
	r_md3_model_c(const std::string &name, byte *buffer, uint_t buffer_size);
	virtual ~r_md3_model_c();
	
	//
	// virtual functions
	//
	virtual void	load();
};

/*
class r_mdc_model_c : public r_alias_model_c
{
public:
	//
	// constructor / destructor
	//
	r_mdc_model_c(const std::string &name, void *buffer);
	virtual ~r_mdc_model_c();
	
	//
	// virtual functions
	//
	virtual void	load();
};
*/


class r_skel_model_c : public r_model_c
{
public:
	//
	// constructor / destructor
	//
	r_skel_model_c(const std::string &name, byte *buffer, uint_t buffer_size);
	virtual ~r_skel_model_c();
	
	//
	// virtual functions
	//
	virtual void	updateBBox(r_entity_c *ent);
	virtual void	addModelToList(r_entity_c *ent);
	virtual void 	draw(const r_command_t *cmd, r_render_type_e type);
	virtual bool	setupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name);
	virtual bool	setupAnimation(r_skel_animation_c *anim);
	
	void		addBone(r_skel_bone_t *bone)		{_bones.push_back(bone);}
	
protected:
	bool	cull(r_entity_c *ent);
	void	drawFrameLerp(const r_command_t *cmd, r_render_type_e type);
	
	void	updateBone(const r_entity_c *ent, r_skel_bone_t *bone);
	void	drawBone(r_skel_bone_t *bone);
	int	getNumForBoneName(const std::string &name);


	std::vector<r_skel_bone_t*>		_bones;
};

/*
class r_mds_model_c : public r_skel_model_c
{
public:
	//
	// constructor / destructor
	//
	r_mds_model_c(const std::string &name, byte *buffer);
	virtual ~r_mds_model_c();
	
	//
	// virtual functions
	//
	virtual void	load();
};
*/

class r_md5_model_c : public r_skel_model_c
{
public:
	//
	// constructor / destructor
	//
	r_md5_model_c(const std::string &name, byte *buffer, uint_t buffer_size);
	virtual ~r_md5_model_c();
	
	//
	// virtual functions
	//
	virtual void	load();
	
private:
	void		loadBones(char **data_p);
	void		loadMeshes(char **data_p);
};


class r_bsp_model_c : public r_model_c
{ 
	friend class r_bsptree_c;
public:
	//
	// constructor / destructor
	//
	r_bsp_model_c(const std::string &name, bool inline_model);
	virtual ~r_bsp_model_c();
	
	//
	// virtual functions
	//
	void		precacheLightInteractions(r_entity_c *ent);
	virtual void	addModelToList(r_entity_c *ent);
	virtual void 	draw(const r_command_t *cmd, r_render_type_e type);
	
private:
	std::vector<r_surface_c*>	_surfaces;

	bool				_inline;
};


class r_proc_model_c : public r_model_c
{  
public:
	//
	// constructor / destructor
	//
	r_proc_model_c(const std::string &name);
	virtual ~r_proc_model_c();
	
	//
	// virtual functions
	//
	virtual void	addModelToList(r_entity_c *ent);
	virtual void 	draw(const r_command_t *cmd, r_render_type_e type);
	virtual void	setupMeshes();
	
	void	load(char **buf_p);
	
	std::vector<r_surface_c*>	_surfaces;
	
	GLuint				_vbo_array_buffer;
	GLuint				_vbo_element_array_buffer;
};



extern std::vector<r_model_c*>	r_models;

extern float			r_depthmin, r_depthmax;



//Tr3B - don't forget to add these textures to void R_FreeUnusedImages() !
extern r_image_c*	r_img_default;
extern r_image_c*	r_img_white;
extern r_image_c*	r_img_black;
extern r_image_c*	r_img_flat;
extern r_image_c*	r_img_quadratic;
extern r_image_c*	r_img_cubemap_white;
extern r_image_c*	r_img_cubemap_normal;
extern r_image_c*	r_img_cubemap_sky;
extern r_image_c*	r_img_nofalloff;
extern r_image_c*	r_img_attenuation_3d;
extern r_image_c*	r_img_lightview_depth;
extern r_image_c*	r_img_lightview_color;



extern r_scene_t*	r_current_scene;

extern r_frustum_t	r_frustum;


extern std::vector<r_image_c*>	r_images;
extern std::vector<r_image_c*>	r_images_lm;	// lightmap images

extern int		r_filter_min;
extern int		r_filter_max;


extern int		r_depth_format;

extern uint_t	c_leafs;
extern uint_t	c_entities;
extern uint_t	c_lights;
extern uint_t	c_cmds;
extern uint_t	c_cmds_radiosity;
extern uint_t	c_cmds_light;
extern uint_t	c_cmds_translucent;
extern uint_t	c_triangles;
extern uint_t	c_draws;
extern uint_t	c_expressions;

struct r_table_t
{
	uint_t			flags;
	std::vector<float>	values;
};

extern std::vector<r_table_t>	r_tables;


//
// view origin
//
extern vec3_c		r_up;
extern vec3_c		r_forward;
extern vec3_c		r_right;

extern vec3_c		r_origin;


extern	bool		r_portal_view;	// if true, get vis data at
extern	vec3_c		r_portal_org;	// portalorg instead of vieworg

extern	bool		r_mirrorview;	// if true, lock pvs

extern	cplane_c	r_clipplane;



//
// screen size info
//
extern r_refdef_t	r_newrefdef;


extern r_entity_c	r_world_entity;
extern r_bsptree_c*	r_world_tree;


extern std::vector<index_t>	r_quad_indexes;



//
// scene info
//
extern std::map<int, r_entity_c>	r_entities;
extern std::map<int, r_light_c>		r_lights;

extern int		r_particles_num;
extern r_particle_t	r_particles[MAX_PARTICLES];

extern int		r_polys_num;
extern r_poly_t		r_polys[MAX_POLYS];

extern r_scene_t	r_world_scene;



extern cvar_t	*r_lefthand;
extern cvar_t	*r_draw2d;
extern cvar_t	*r_drawentities;
extern cvar_t	*r_drawworld;
extern cvar_t	*r_drawparticles;
extern cvar_t	*r_drawpolygons;
extern cvar_t	*r_drawsky;
extern cvar_t	*r_drawextra;
extern cvar_t	*r_drawtranslucent;
extern cvar_t	*r_speeds;
extern cvar_t	*r_fullbright;
extern cvar_t	*r_lerpmodels;
extern cvar_t	*r_log;
extern cvar_t	*r_shadows;
extern cvar_t	*r_shadows_alpha;
extern cvar_t	*r_shadows_nudge;
extern cvar_t	*r_lighting;
extern cvar_t	*r_lighting_omni;
extern cvar_t	*r_lighting_proj;
extern cvar_t	*r_lightmap;
extern cvar_t	*r_lightscale;
extern cvar_t	*r_znear;
extern cvar_t	*r_zfar;
extern cvar_t	*r_nobind;
extern cvar_t	*r_picmip;
extern cvar_t	*r_skymip;
extern cvar_t	*r_showtris;
extern cvar_t	*r_showbbox;
extern cvar_t	*r_showareas;
extern cvar_t	*r_showareaportals;
extern cvar_t	*r_shownormals;
extern cvar_t	*r_showtangents;
extern cvar_t	*r_showbinormals;
extern cvar_t	*r_showinvisible;
extern cvar_t	*r_showlightbboxes;
extern cvar_t	*r_showlightscissors;
extern cvar_t	*r_clear;
extern cvar_t	*r_cull;
extern cvar_t	*r_cullplanes;
extern cvar_t	*r_cullportals;
extern cvar_t	*r_polyblend;
extern cvar_t	*r_flashblend;
extern cvar_t	*r_playermip;
extern cvar_t	*r_drawbuffer;
extern cvar_t	*r_texturemode;
extern cvar_t	*r_polygonmode;
extern cvar_t	*r_lockpvs;
extern cvar_t	*r_vis;
extern cvar_t	*r_maxtexsize;
extern cvar_t	*r_finish;
extern cvar_t	*r_subdivisions;
extern cvar_t	*r_flares;
extern cvar_t	*r_flaresize;
extern cvar_t	*r_flarefade;
extern cvar_t	*r_octree;
extern cvar_t	*r_bump_mapping;
extern cvar_t	*r_gloss;
extern cvar_t	*r_parallax;
extern cvar_t	*r_cmds_max;
extern cvar_t	*r_cmds_light_max;
extern cvar_t	*r_cmds_translucent_max;
extern cvar_t	*r_video_export;
extern cvar_t	*r_evalexpressions;

extern cvar_t	*r_arb_multitexture;
extern cvar_t	*r_arb_texture_compression;
extern cvar_t	*r_arb_vertex_buffer_object;
extern cvar_t	*r_arb_occlusion_query;

extern cvar_t	*r_ext_compiled_vertex_array;
extern cvar_t	*r_ext_texture_filter_anisotropic;
extern cvar_t	*r_ext_texture_filter_anisotropic_level;

extern cvar_t	*r_sgix_fbconfig;
extern cvar_t	*r_sgix_pbuffer;

extern cvar_t	*vid_fullscreen;
extern cvar_t	*vid_gamma;
extern cvar_t	*vid_mode;
extern cvar_t	*vid_gldriver;
extern cvar_t	*vid_colorbits;
extern cvar_t	*vid_depthbits;
extern cvar_t	*vid_stencilbits;

extern cvar_t	*vid_pbuffer_texsize;
extern cvar_t	*vid_pbuffer_colorbits;
extern cvar_t	*vid_pbuffer_depthbits;
extern cvar_t	*vid_pbuffer_stencilbits;





//
// r_alias.cxx
//



//
// r_backend.cxx
//
void		RB_InitBackend();
void		RB_ShutdownBackend();

void		RB_BeginBackendFrame();
void		RB_EndBackendFrame();

void		RB_SetShaderTime(double time);

void		RB_CheckForError_(const std::string &file, int line);

#ifdef HAVE_DEBUG
#define		RB_CheckForError()	RB_CheckForError_(__FILE__, __LINE__)
#else
#define		RB_CheckForError()	
#endif

void		RB_OpenGLFrustum(matrix_c &m, double l, double r, double b, double t, double n, double f);
void		RB_QuakeFrustum(matrix_c &m, double l, double r, double b, double t, double n, double f);

void		RB_SetupModelviewMatrix(const matrix_c &m, bool force = false);
void		RB_SetupLightviewMatrix(const matrix_c &m);
void		RB_SetupGL2D();
void		RB_SetupGL3D();

void		RB_Clear();

void		RB_RenderLightScale();

void 		RB_SelectTexture(GLenum);
void 		RB_TexEnv(GLenum value);
void 		RB_Bind(r_image_c *image);
void		RB_TextureMode(const std::string &string);

void		RB_PushMesh(r_mesh_c *mesh, bool create_vbo_array_buffer, bool create_vbo_element_array_buffer);
void		RB_FlushMesh(const r_command_t *cmd);

void		RB_LockArrays(int vertexes_num);
void		RB_UnlockArrays();

void		RB_EnableShaderStates(const r_shader_c *shader, r_render_type_e type);
void		RB_DisableShaderStates(const r_shader_c *shader, r_render_type_e type);

void		RB_EnableShaderStageStates(const r_entity_c *ent, const r_shader_stage_c *stage, r_render_type_e type);
void		RB_DisableShaderStageStates(const r_entity_c *ent, const r_shader_stage_c *stage, r_render_type_e type);

float		RB_Evaluate(const r_entity_t &shared, const boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> &info, float default_value);

void		RB_ModifyTextureMatrix(const r_entity_c *ent, const r_shader_stage_c *stage);
void		RB_ModifyOmniLightTextureMatrix(const r_command_t *cmd, const r_shader_stage_c *stage);
void		RB_ModifyOmniLightCubeTextureMatrix(const r_command_t *cmd, const r_shader_stage_c *stage);
void		RB_ModifyProjLightTextureMatrix(const r_command_t *cmd, const r_shader_stage_c *stage);
void		RB_ModifyColor(const r_entity_t &shared, const r_shader_stage_c *stage, vec4_c &color);

void		RB_RenderCommand(const r_command_t *cmd, r_render_type_e type);
void		RB_RenderCommands();
void		RB_AddCommand(	r_entity_c*		entity,
				r_model_c*		entity_model,
				r_mesh_c*		entity_mesh,
				r_shader_c*		entity_shader,
				r_light_c*		light,
				std::vector<index_t>*	light_indexes,
				int			infokey,
				vec_t			distance);

void		RB_DrawSkyBox();



//
// r_bsp.cxx
//
void 		R_DrawWorld();



//
// r_draw.cxx
//
void 		R_InitDraw();
void		R_DrawStretchPic(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, r_shader_c *shader);
void		R_DrawStretchPicExp(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, int shader);

void		R_DrawPic(int x, int y, int w, int h, const vec4_c &color, r_shader_c *shader);
void		R_DrawPicExp(int x, int y, int w, int h, const vec4_c &color, int shader);

void		R_DrawFill(int x, int y, int w, int h, const vec4_c &color);



//
// r_image.cxx
//
void		R_InitImages();
void		R_ShutdownImages();
void		R_ImageList_f();
r_image_c*	R_FindImage(const std::string &name, uint_t flags, r_image_upload_type_e upload_type);
void		R_LoadLightMapImages(const std::string &mapname);
void		R_FreeUnusedImages();

int		R_GetNumForImage(r_image_c *image);
r_image_c*	R_GetImageByNum(int num);

r_image_c*	R_GetLightMapImageByNum(int num);



//
// r_light.cxx
//




//
// r_mesh.cxx
//



//
// r_model.cxx
//
void		R_InitModels();
void		R_ShutdownModels();
void		R_Modellist_f();
r_model_c*	R_GetModel(const std::string &name, bool crash);


void		R_BeginRegistration(const std::string &map);
r_model_c*	R_RegisterModel(const std::string &name);
int		R_RegisterModelExp(const std::string &name);

int		R_GetNumForModel(r_model_c *mod);
r_model_c*	R_GetModelByNum(int num);

void		R_EndRegistration();




//
// r_main.cxx
//
bool 		R_CullBBox(const r_frustum_t frustum, const cbbox_c &bbox, int clipmask = FRUSTUM_CLIPALL);
bool		R_CullBSphere(const r_frustum_t frustum, const vec3_c &center, vec_t radius, int clipmask = FRUSTUM_CLIPALL);
bool		R_CullPoint(const r_frustum_t frustum, const vec3_c &origin, int clipmask = FRUSTUM_CLIPALL);

void 		R_DrawNULL(const vec3_c &origin, const vec3_c &angles);
void		R_DrawBBox(const cbbox_c &bbox);

void		R_CalcTangentSpace(	vec3_c &tangent, vec3_c &binormal, vec3_c &normal, 
					const vec3_c &v0, const vec3_c &v1, const vec3_c &v2,
					const vec2_c &t0, const vec2_c &t1, const vec2_c &t2,
					const vec3_c &n	);
					
void		R_InitTree(r_tree_type_e type, const std::string &name);
void		R_ShutdownTree();

bool		R_Init(void *hinstance, void *hWnd);
void		R_Shutdown();

bool		R_SetupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name);
bool		R_SetupAnimation(int model, int anim);

//
// r_octree.cxx
//
void 		R_InitOCT();
void 		R_DrawOCT();


//
// r_particle.cxx
//
void		R_InitParticles();
//void		R_PushParticle(r_mesh_buffer_t *mb);
void		R_DrawParticles();

//
// r_poly.cxx
//
void		R_InitPolys();
//void		R_DrawPolys(r_mesh_buffer_t *mb);
void		R_AddPolysToBuffer();



//
// r_shader.cxx
//
void		R_InitShaders();
void		R_ShutdownShaders();

void		R_FreeUnusedShaders();
void		R_ShaderList_f();
void		R_ShaderCacheList_f();
void		R_ShaderSearch_f();
void		R_SpiritTest_f();


r_shader_c*	R_FindShader(const std::string &name, r_shader_type_e type);

r_shader_c*	R_RegisterShader(const std::string &name);
r_shader_c*	R_RegisterPic(const std::string &name);
r_shader_c*	R_RegisterParticle(const std::string &name);
r_shader_c*	R_RegisterLight(const std::string &name);


int		R_GetNumForShader(r_shader_c *shader);
r_shader_c*	R_GetShaderByNum(int num);

int		R_RegisterShaderExp(const std::string &name);
int		R_RegisterPicExp(const std::string &name);
int		R_RegisterParticleExp(const std::string &name);
int		R_RegisterLightExp(const std::string &name);


//
// r_skel.cxx
//
void		R_InitAnimations();
void		R_ShutdownAnimations();
r_skel_animation_c*	R_LoadAnimation(const std::string &name);
r_skel_animation_c*	R_FindAnimation(const std::string &name);

int		R_GetNumForAnimation(r_skel_animation_c *anim);
r_skel_animation_c*	R_GetAnimationByNum(int num);

void		R_FreeUnusedAnimations();

int		R_RegisterAnimationExp(const std::string &name);

void		R_AnimationList_f();



//
// r_skin.cxx
//
void		R_InitSkins();
void		R_ShutdownSkins();
r_model_skin_c*	R_LoadSkin(const std::string &name);
r_model_skin_c*	R_FindSkin(const std::string &name);

int		R_GetNumForSkin(r_model_skin_c *skin);
r_model_skin_c*	R_GetSkinByNum(int num);

void		R_FreeUnusedSkins();

int		R_RegisterSkinExp(const std::string &name);

void		R_SkinList_f();


//
// r_sky.cxx
//
void		R_DrawFastSkyBox();
void 		R_ClearSkyBox();
void		R_DrawSky();
void		R_AddSkySurface(r_surface_c *surf);
void 		R_SetSky(const std::string &name);




/*
====================================================================
			GL CONFIG STUFF
====================================================================
*/
#define GL_RENDERER_NV			0x00000001
#define GL_RENDERER_ATI			0x00000002
#define GL_RENDERER_OTHER		0x80000000


struct glconfig_t
{
	int         	renderer;
	const char*	renderer_string;
	const char*	vendor_string;
	const char*	version_string;
	const char*	extensions_string;

	bool		allow_cds;
	
	// save here what extensions are currently available
	bool		arb_multitexture;
	bool		arb_texture_compression;
	bool		arb_vertex_buffer_object;
	bool		arb_occlusion_query;
	
	bool		ext_compiled_vertex_array;
	bool		ext_texture_filter_anisotropic;
};

struct glstate_t
{
	float 		inverse_intensity;
	bool 		fullscreen;

	int     	prev_mode;

	uint_t		currenttextures[16];
	uint_t		currenttmu;

	uint_t		maxtexsize;
	
	int		anisotropylevel;

	bool		is2d;
	bool		hwgamma;
	bool		lights_on;
};

extern glconfig_t  gl_config;
extern glstate_t   gl_state;



/*
====================================================================
		IMPLEMENTATION SPECIFIC FUNCTIONS
====================================================================
*/
void		GLimp_BeginFrame();
void		GLimp_EndFrame();
int 		GLimp_Init(void *hinstance, void *hWnd);
void		GLimp_Shutdown();
int		GLimp_SetMode(int *pwidth, int *pheight, int mode, bool fullscreen);
void		GLimp_AppActivate(bool active);
void		GLimp_Gamma();
void		GLimp_InitPbuffer(bool share_contexts, bool share_objects);
void		GLimp_ActivatePbuffer();
void		GLimp_DeactivatePbuffer();
void		GLimp_EnableLogging(bool enable);
void		GLimp_LogNewFrame();

#endif // R_LOCAL_H
