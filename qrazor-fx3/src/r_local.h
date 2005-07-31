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
#include <boost/spirit/tree/tree_to_xml.hpp>
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

#define VBO_BUFFER_OFFSET(i) 		((char *)NULL + (i))


extern	viddef_t	vid;


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
	IMAGE_UPLOAD_NORMALMAP,
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
	SHADER_DEBUG			= 1 << 0,
	SHADER_SKY			= 1 << 1,
	SHADER_POLYGONOFFSET		= 1 << 2,
	SHADER_TWOSIDED			= 1 << 3,
	SHADER_NODRAW			= 1 << 4,
	SHADER_NOSHADOWS		= 1 << 5,
	SHADER_NOSELFSHADOW		= 1 << 6,
	SHADER_TRANSLUCENT		= 1 << 7,
	SHADER_DISCRETE			= 1 << 8,
	SHADER_AREAPORTAL		= 1 << 9,
	SHADER_NOENVMAP			= 1 << 10,
	SHADER_POSTPROCESS		= 1 << 11,
	SHADER_FORCEOPAQUE		= 1 << 12
};


enum r_shader_material_stage_type_e
{
	SHADER_MATERIAL_STAGE_TYPE_COLORMAP,
	SHADER_MATERIAL_STAGE_TYPE_DIFFUSEMAP,
	SHADER_MATERIAL_STAGE_TYPE_BUMPMAP,
	SHADER_MATERIAL_STAGE_TYPE_SPECULARMAP,
	SHADER_MATERIAL_STAGE_TYPE_HEATHAZEMAP,
	SHADER_MATERIAL_STAGE_TYPE_LIGHTMAP,
	SHADER_MATERIAL_STAGE_TYPE_DELUXEMAP,
	SHADER_MATERIAL_STAGE_TYPE_REFLECTIONMAP,
	SHADER_MATERIAL_STAGE_TYPE_REFRACTIONMAP,
	SHADER_MATERIAL_STAGE_TYPE_DISPERSIONMAP,
	SHADER_MATERIAL_STAGE_TYPE_SKYBOXMAP,
	SHADER_MATERIAL_STAGE_TYPE_SKYCLOUDMAP,
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
/*
enum r_shader_sort_type_e
{
	SHADER_SORT_SUBVIEW	= 0,
	SHADER_SORT_FAR		= 1,
	SHADER_SORT_CLOSE	= 5,
	SHADER_SORT_DECAL	= 6,
	SHADER_SORT_NEAREST	= 16
};
*/

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
//class r_proc_model_c;
class r_bsptree_leaf_c;
class r_skel_animation_c;


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
	
	SHADER_PARM_SOUND,
	
	SHADER_PARM_FRANDOM,
	SHADER_PARM_CRANDOM
};

struct r_shader_parameter_symbols_t : boost::spirit::symbols<r_shader_parms_e, char>
{
	r_shader_parameter_symbols_t()
	{
		add
			("time",	SHADER_PARM_TIME)
			
			("parm0",	SHADER_PARM_PARM0)
			("parm1",	SHADER_PARM_PARM1)
			("parm2",	SHADER_PARM_PARM2)
			("parm3",	SHADER_PARM_PARM3)
			("parm4",	SHADER_PARM_PARM4)
			("parm5",	SHADER_PARM_PARM5)
			("parm6",	SHADER_PARM_PARM6)
			("parm7",	SHADER_PARM_PARM7)
			
			("global0",	SHADER_PARM_GLOBAL0)
			("global1",	SHADER_PARM_GLOBAL1)
			("global2",	SHADER_PARM_GLOBAL2)
			
			("sound",	SHADER_PARM_SOUND)
			
			("frandom",	SHADER_PARM_FRANDOM)
			("crandom",	SHADER_PARM_CRANDOM)
			;
	}
};

extern r_shader_parameter_symbols_t r_shader_parameter_symbols_p;


struct r_shader_table_symbols_t : boost::spirit::symbols<uint_t, char>//, std::set<const char*> >
{
	// add symbols later
};

extern r_shader_table_symbols_t r_shader_table_symbols_p;


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

enum r_render_type_e
{
	RENDER_TYPE_DEFAULT,
	RENDER_TYPE_GENERIC,
	RENDER_TYPE_ZFILL,
	RENDER_TYPE_SKY,
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
	RENDER_TYPE_LIGHTING_D_vstatic,
	RENDER_TYPE_LIGHTING_DB_vstatic,
	RENDER_TYPE_LIGHTING_DBH_vstatic,
	RENDER_TYPE_LIGHTING_DBHS_vstatic,
	RENDER_TYPE_LIGHTING_DBS_vstatic,
	RENDER_TYPE_LIGHTING_D_omni,
	RENDER_TYPE_LIGHTING_D_proj,
	RENDER_TYPE_LIGHTING_DB_omni,
	RENDER_TYPE_LIGHTING_DBH_omni,
	RENDER_TYPE_LIGHTING_DBHS_omni,
	RENDER_TYPE_LIGHTING_DBS_omni,
	RENDER_TYPE_FOG_UNIFORM,
	RENDER_TYPE_POSTPROCESS
};

enum
{
	FRUSTUM_LEFT		= 0,
	FRUSTUM_RIGHT,
	FRUSTUM_BOTTOM,
	FRUSTUM_TOP,
	FRUSTUM_NEAR,
	FRUSTUM_FAR,
	FRUSTUM_PLANES		= 5,
	FRUSTUM_CLIPALL		= 1 | 2 | 4 | 8 | 16 //| 32
};


class r_frustum_c
{
public:
	bool	cull(const aabb_c &bbox, int clipmask = FRUSTUM_CLIPALL) const;
	bool	cull(const vec3_c &center, vec_t radius, int clipmask = FRUSTUM_CLIPALL) const;
	bool	cull(const vec3_c &origin, int clipmask = FRUSTUM_CLIPALL) const;

	inline const plane_c&	operator [] (const int index) const
	{
		return _planes[index];
	}

	inline plane_c&	operator [] (const int index)
	{
		return _planes[index];
	}
private:
	plane_c		_planes[FRUSTUM_PLANES];
};




typedef float							r_node_data_t;

typedef std::string::const_iterator				r_iterator_t;
typedef boost::spirit::node_val_data_factory<r_node_data_t>	r_factory_t;

extern uint_t		r_registrationcount;
extern uint_t		r_framecount;
extern uint_t		r_visframecount;			// bumped when going to a new PVS
extern uint_t		r_lightframecount;			// bumped when walking down the BSP by a new dynamic light
extern uint_t		r_shadowframecount;			// bumped when rendering shadow maps
extern uint_t		r_checkcount;				// bumped when box runs against BSP leaves to collect them


class r_registrationcount_iface_a
{
protected:
	r_registrationcount_iface_a()
	{
	}
	
public:
	inline void	setRegistrationCount()		{_registrationcount = r_registrationcount;}
	inline void	resetRegistrationCount()	{_registrationcount = 0;}
	inline bool	isRegistered() const	{return r_registrationcount == _registrationcount;}
	
protected:
	uint_t		_registrationcount;
};

class r_image_c :
public r_registrationcount_iface_a
{
public:
	r_image_c(uint_t target, const std::string &name, uint_t width, uint_t height, uint_t flags, roq_info_t *roq, bool global = true);
	~r_image_c();
	
	inline const char*	getName() const			{return _name.c_str();}
	inline uint_t		getTarget() const 		{return _target;}
	
	inline uint_t		getWidth() const		{return _width;}
	inline uint_t		getHeight() const 		{return _height;}
	
	inline uint_t		getFlags() const		{return _flags;}
	inline bool		hasFlags(uint_t flags) const 	{return _flags & flags;}
	
	inline uint_t		getId() const			{return _id;}
	
	inline bool		hasVideo() const		{return _roq ? true : false;}
	
	void			bind(bool force = false) const;
	
	void			copyFromContext() const;
	void			copyFromVideo() const;
private:
	void			convertColors() const;

	std::string		_name;
	uint_t			_target;
	uint_t			_width;
	uint_t			_height;
	uint_t			_flags;
	uint_t			_id;
	roq_info_t*		_roq;
};


class r_renderbuffer_c :
public r_registrationcount_iface_a
{
public:
	r_renderbuffer_c();
	~r_renderbuffer_c();

	void			validate() const;
	void			bind() const;
	void			getParameteriv(GLenum pname, GLint *params) const;

	inline uint_t		getId() const			{return _id;}

private:
	uint_t			_target;
	uint_t			_id;
};

class r_framebuffer_c :
public r_registrationcount_iface_a
{
public:
	r_framebuffer_c();
	~r_framebuffer_c();

	void			validate() const;
	void			bind() const;
	void			unbind() const;
	void			checkStatus() const;

	void			attachTexture1D(GLenum attachment, GLenum textarget, GLuint texture, GLint level) const;
	void			attachTexture2D(GLenum attachment, GLenum textarget, GLuint texture, GLint level) const;
	void			attachTexture3D(GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) const;
	void			renderBuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) const;
	void			getAttachmentParameteriv(GLenum attachment, GLenum pname, GLint *params) const;
	
	inline uint_t		getId() const			{return _id;}

private:
	uint_t			_target;
	uint_t			_id;
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
	
	void		fillVertexes(int vertexes_num, bool with_lightmap_texcoords = false, bool with_light_vectors = false);
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
	std::vector<vec2_c>		texcoords_lm;	// for lightmapping
	std::vector<vec3_c>		tangents;	// for creating tangent space
	std::vector<vec3_c>		binormals;	// for creating tangent space
	std::vector<vec3_c>		normals;
	std::vector<vec3_c>		lights;		// for static N dot L lighting
	std::vector<vec4_c>		colors;
	
	// triangles data
	std::vector<uint_t>		triangles;	// triangle numbers
	std::vector<index_t>		indexes;	// raw triangle indices data for OpenGL
	std::vector<vec3_c>		normals_tri;
	std::vector<plane_c>		planes;
	std::vector<r_wedge_t>		edges;
	std::list<r_wedge_t>		edges_unmatched;
	
	// updated every time silhouettes edges are computed
	std::vector<bool>		triangles_sh;
	std::vector<int>		edges_sh;
	
	// culling
	aabb_c				bbox;
	
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
	GLuint				vbo_lights_ofs;
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
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	deform_magnitude;
	
	bool				make_intensity;
	bool				make_alpha;
	bool				heightmap;
	float				heightmap_scale;
	
	std::string			image_name;
	r_image_c*			image;
};


class r_shader_c :
public r_registrationcount_iface_a
{
public:
	r_shader_c(const std::string &name, r_shader_type_e type);
	
	~r_shader_c();
	
	inline const char*		getName() const			{return _name.c_str();}
	inline r_shader_type_e		getType() const 		{return _type;}
	inline r_shader_light_type_e	getLightType() const		{return _type_light;}
	inline void			setLightType(r_shader_light_type_e type)	{_type_light = type;}
	
	inline const boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>&
					getSort() const			{return _sort;}
	inline void			setSort(boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>& sort)
	{
		_sort = sort;
	}
	
	inline r_shader_deform_type_e	getDeformType() const				{return _deform;}
	inline void			setDeformType(r_shader_deform_type_e deform)	{_deform = deform;}
	
	inline uint_t			getFlags() const		{return _flags;}
	inline bool			hasFlags(uint_t flags) const 	{return _flags & flags;}
	inline void			setFlags(uint_t flags)		{_flags = flags;}
	inline void			setORFlags(uint_t flags)	{_flags |= flags;}
	inline void			removeFlags(uint_t flags)	{_flags &= ~flags;}
	
private:
	std::string			_name;
	r_shader_type_e			_type;
	r_shader_light_type_e		_type_light;	// only used if shader is a light shader
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t>	_sort;
	r_shader_deform_type_e		_deform;
	uint_t				_flags;
	
public:
	// used if material shader
	r_shader_stage_c*		stage_diffusemap;
	r_shader_stage_c*		stage_bumpmap;
	r_shader_stage_c*		stage_specularmap;
	r_shader_stage_c*		stage_heathazemap;
	
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

class r_model_skin_c :
public r_registrationcount_iface_a
{
public:
	r_model_skin_c(const std::string &name)
	{
		_name = name;
		setRegistrationCount();
	}

	inline const char*	getName() const	{return _name.c_str();}
	
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
			ir->second->setRegistrationCount();
		}
	}
	

private:
	std::string		_name;
	std::map<std::string, r_shader_c*>	_shaders;
};





class r_framecount_iface_a
{
protected:
	r_framecount_iface_a()
	{
	}
	
public:
//	inline uint_t	getFrameCount() const	{return _framecount;}
	inline void	setFrameCount()		{_framecount = r_framecount;}
	inline void	resetFrameCount()		{_framecount = 0;}
	inline bool	isFramed() const	{return r_framecount == _framecount;}
	
protected:
	uint_t		_framecount;
};


class r_visframecount_iface_a
{
protected:
	r_visframecount_iface_a()
	{
	}
	
public:
//	inline uint_t	getVisFrameCount() const	{return _visframecount;}
	inline void	setVisFrameCount()		{_visframecount = r_visframecount;}
	inline void	resetVisFrameCount()		{_visframecount = 0;}
	inline bool	isVisFramed() const		{return r_visframecount == _visframecount;}
	
protected:
	uint_t		_visframecount;
};


class r_lightframecount_iface_a
{
protected:
	r_lightframecount_iface_a()
	{
	}
	
public:
//	inline uint_t	getLightFrameCount() const	{return _lightframecount;}
	inline void	setLightFrameCount()		{_lightframecount = r_lightframecount;}
	inline bool	isLightFramed() const		{return r_lightframecount == _lightframecount;}
	
protected:
	uint_t		_lightframecount;
};


class r_shadowframecount_iface_a
{
protected:
	r_shadowframecount_iface_a()
	{
	}
	
public:
//	inline uint_t	getShadowFrameCount() const	{return _shadowframecount;}
	inline void	setShadowFrameCount()		{_shadowframecount = r_shadowframecount;}
	inline bool	isShadowed() const		{return r_shadowframecount == _shadowframecount;}
	
protected:
	uint_t		_shadowframecount;
};


class r_checkcount_iface_a
{
protected:
	r_checkcount_iface_a()
	{
	}
	
public:
	inline void	setCheckCount()			{_checkcount = r_checkcount;}
	inline bool	isChecked() const		{return r_checkcount == _checkcount;}
	
protected:
	uint_t		_checkcount;
};





class r_surface_c : 
public r_framecount_iface_a,
public r_lightframecount_iface_a
{
	friend class r_bsptree_c;
public:
	r_surface_c();
	
	inline r_mesh_c*		getMesh() const		{return _mesh;}
	inline void			setMesh(r_mesh_c *mesh)	{_mesh = mesh;}
	
	inline uint_t			getSubEntityNum() const	{return _sub;}
	inline void			setSubEntityNum(uint_t sub) {_sub = sub;}
	
	inline r_model_shader_c*	getShaderRef() const	{return _shaderref;}
	inline void			setShaderRef(r_model_shader_c *ref)	{_shaderref = ref;}
	inline r_shader_c*		getShader() const	{return _shaderref ? _shaderref->getShader() : NULL;}
	
	inline bsp_surface_type_t	getFaceType() const	{return _facetype;}
	inline const plane_c&		getPlane() const	{return _plane;}
	
	inline uint_t			getLightMapNum() const	{return _lightmap;}

private:
	r_mesh_c*		_mesh;
	
	int			_sub;
	
	r_model_shader_c*	_shaderref;

	bsp_surface_type_t	_facetype;
	plane_c			_plane;			// if BSPST_PLANAR
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

class r_scissor_iface_a
{
public:
	void			updateScissor(const matrix_c &mvp, const r_vrect_t &vrect, const aabb_c &bbox);
	void			setScissor(const r_vrect_t &vrect);
	
private:
	void			addVertex(const matrix_c &mvp, const r_vrect_t &vrect, const vec3_c &v);
	void			addEdge(const matrix_c &mvp, const r_vrect_t &vrect, const vec3_c &v0, const vec3_c &v1);
public:

	//inline const vec4_c&	getScissorCoords() const	{return _coords;}
	inline int		getScissorX() const		{return (int)_mins[0];}
	inline int		getScissorY() const		{return (int)_mins[1];}
	inline int		getScissorWidth() const		{return (int)X_fabs(_maxs[0] - _mins[0]);}
	inline int		getScissorHeight() const	{return (int)X_fabs(_maxs[1] - _mins[1]);}
	
private:
//	vec4_c		_coords;
	vec2_c		_mins;
	vec2_c		_maxs;
};

class r_vis_iface_a
{
protected:
	r_vis_iface_a()
	{
	}
	
public:
	void		updateVis(const r_entity_t &shared);
	
	inline int	getCluster() const		{return _cluster;}
	inline const std::vector<r_bsptree_leaf_c*>&	getLeafs() const	{return _leafs;}
	
	inline const std::vector<int>&			getAreas() const	{return _areas;}
	inline void	addArea(int areanum)
	{
		std::vector<int>::iterator ir = find(_areas.begin(), _areas.end(), areanum);
		if(ir == _areas.end())
			_areas.push_back(areanum);
	}
	inline bool	hasArea(int areanum)
	{
		std::vector<int>::iterator ir = find(_areas.begin(), _areas.end(), areanum);
		
		return (ir != _areas.end());
	}
	
	
protected:
	int				_cluster;
	std::vector<r_bsptree_leaf_c*>	_leafs;
	std::vector<int>		_areas;
};

class r_occlusion_iface_a
{
protected:
	r_occlusion_iface_a()
	{
		_query = 0;
	
		if(xglGenQueriesARB)
			xglGenQueriesARB(1, &_query);
	}
	
	~r_occlusion_iface_a()
	{
		if(xglDeleteQueriesARB && _query)
			xglDeleteQueriesARB(1, &_query);
	}
	
public:
	//inline uint_t		getQuery() const	{return _query;}
	
	inline void		beginOcclusionQuery() const	{xglBeginQueryARB(GL_SAMPLES_PASSED_ARB, _query);}	
	inline void		endOcclusionQuery() const	{xglEndQueryARB(GL_SAMPLES_PASSED_ARB);}
	
	inline bool		getOcclusionSamplesAvailable() const
	{
		int avail;
		xglGetQueryObjectivARB(_query, GL_QUERY_RESULT_AVAILABLE_ARB, &avail);
		return avail >= 1;
	}
	inline uint_t		getOcclusionSamplesNum() const
	{
		uint_t samples = 0;
		xglGetQueryObjectuivARB(_query, GL_QUERY_RESULT_ARB, &samples);
		return samples;
	}
	
private:
	uint_t		_query;
};


r_model_c*	R_GetModelByNum(int num);
class r_model_iface_a
{
protected:
	inline r_model_iface_a(int anim)
	{
		_model = R_GetModelByNum(anim);
	}
	
public:
	inline const r_model_c*	getModel() const	{return _model;}
	
private:
	r_model_c*	_model;
};


r_skel_animation_c*	R_GetAnimationByNum(int num);
class r_animation_iface_a
{
protected:
	inline r_animation_iface_a(int anim)
	{
		_animation = R_GetAnimationByNum(anim);
	}
	
public:
	inline const r_skel_animation_c*	getAnimation() const	{return _animation;}
	
private:
	r_skel_animation_c*	_animation;
};

class r_aabb_iface_a
{
protected:
	inline r_aabb_iface_a()
	{
	}
	
public:
	inline const aabb_c&	getAABB() const		{return _aabb;}
	
//protected:
	aabb_c		_aabb;
};


class r_interaction_c
{
public:
	r_interaction_c(r_entity_c* ent, r_light_c* light, const std::vector<index_t> &indexes);
	
	inline r_entity_c*		getEntity() const	{return _entity;}
	inline r_light_c*		getLight() const	{return _light;}
	inline const matrix_c&		getAttenuation() const	{return _attenuation;}
	
	inline const std::vector<index_t>&	getIndexes() const	{return _indexes;}
	
private:
	r_entity_c*		_entity;
	r_light_c*		_light;
	matrix_c		_attenuation;
	std::vector<index_t>	_indexes;
};


class r_entity_sub_c
{
public:
	inline void					addInteraction(r_interaction_c* ia)
	{
		_interactions.push_back(ia);
	}
	
	inline const std::vector<r_interaction_c*>&	getInteractions() const	{return _interactions;}
	
private:
	std::vector<r_interaction_c*>	_interactions;
};


class r_entity_c :
public r_vis_iface_a,
public r_framecount_iface_a,
public r_visframecount_iface_a,
public r_occlusion_iface_a,
public r_model_iface_a,
public r_animation_iface_a,
public r_aabb_iface_a
{
public:
	r_entity_c();
	r_entity_c(const r_entity_t &shared);

	void			setupTransform();
	void			setupTransformLeftHanded();
	void			setupTransformToViewer();
	
	inline const r_entity_t&	getShared() const	{return _s;}
	
	inline const matrix_c&		getTransform() const	{return _transform;}
	
//	inline void			setLerp(float lerp)	{_s.lerp = lerp;}
	inline void			setColor(const vec4_c &color)
	{
		_s.shader_parms[0] = color[0];
		_s.shader_parms[1] = color[1];
		_s.shader_parms[2] = color[2];
		_s.shader_parms[3] = color[3];
	}
	
	inline bool	isStatic() const			{return (_s.flags & RF_STATIC);}
	inline bool	isVisible() const
	{
		if(isStatic())
		{
			return isVisFramed() && isFramed();
		}
		else
		{
			return isFramed();
		}
	}
	
	const r_entity_sub_c&	getSubEntity(int num) const;
	void			addInteractionToSubEntity(int num, r_interaction_c* ia);

private:
	r_entity_t			_s;
	matrix_c			_transform;
	std::vector<r_entity_sub_c>	_subs;
};


class r_light_c :
public r_vis_iface_a,
public r_framecount_iface_a,
public r_visframecount_iface_a,
public r_shadowframecount_iface_a,
public r_scissor_iface_a,
public r_occlusion_iface_a
{
	friend void	RB_AddCommand(	r_entity_c*		entity,
					r_model_c*		entity_model,
					r_mesh_c*		entity_mesh,
					r_shader_c*		entity_shader,
					r_light_c*		light,
					std::vector<index_t>*	light_indexes,
					int			infokey,
					vec_t			distance,
					const matrix_c&		light_attenuation);

public:
//	r_light_c();
	r_light_c(const r_entity_t &shared, r_light_type_t type);
	~r_light_c();
	
	void			update(const r_entity_t &shared, r_light_type_t type);

	void			setupTransform();
	void			setupAttenuation();
	void			setupProjection();
	void			setupFrustum();
	void			setupShadowMapProjection();
	void			setupShadowMapView();
	
	r_interaction_c*	createInteraction(r_entity_c* ent, const r_mesh_c *mesh);
	
	inline const r_entity_t&	getShared() const	{return _s;}
	inline r_light_type_t		getType() const		{return _type;}
	
	inline const vec3_c&		getOrigin() const	{return _origin;}
	
	inline const matrix_c&		getTransform() const	{return _transform;}
	inline const matrix_c&		getView() const		{return _transform_inv;}
	inline const matrix_c&		getAttenuation() const	{return _attenuation;}
	inline const matrix_c&		getProjection() const	{return _projection;}
	
	inline const matrix_c&		getShadowMapProjection() const	{return _shadowmap_projection;}
	inline const matrix_c&		getShadowMapView() const	{return _shadowmap_view;}
	
	inline const r_frustum_c&	getFrustum() const	{return _frustum;}
	
	inline bool	isStatic() const			{return (_s.flags & RF_STATIC);}
	inline bool	isVisible() const
	{
		if(isStatic())
		{
			return isVisFramed() && isFramed();
		}
		else
		{
			return isFramed();
		}
	}
	

private:
	r_entity_t		_s;
	r_light_type_t		_type;
	
	vec3_c			_origin;
		
	matrix_c		_transform;
	matrix_c		_transform_inv;
	
	matrix_c		_attenuation;
	matrix_c		_projection;
	
	matrix_c		_shadowmap_projection;
	matrix_c		_shadowmap_view;
		
	r_frustum_c		_frustum;
						
//	std::vector<std::map<const r_surface_c*, std::vector<index_t> >	>	_areasurfaces;	
//	std::vector<std::vector< >	>			_entity_interactions;

//	std::vector<r_entity_sub>	_entity_interactions;
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
					vec_t			distance,
					const matrix_c&		light_attenuation);

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
	
	inline r_light_c*		getLight() const		{return _light;}
	inline r_shader_c*		getLightShader() const		{return _light_shader;}
	inline std::vector<index_t>*	getLightIndexes() const		{return _light_indexes;}
	inline bool			hasLightMap() const		{return _light_map;}
	inline bool			hasLightVertexes() const	{return _light_vertexes;}
//	inline const matrix_c&		getLightTransform() const	{return _light_transform;}
	inline const matrix_c&		getLightAttenuation() const	{return _light_attenuation;}
		
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
	bool			_light_map;
	bool			_light_vertexes;
//	matrix_c		_light_transform;
	matrix_c		_light_attenuation;
	
	int			_infokey;
	
	vec_t			_distance;		// for sort by distance
};


struct r_scene_t
{
	uint_t				cmds_num;
	std::vector<r_command_t>	cmds;
	
	uint_t				cmds_light_num;
	std::vector<r_command_t>	cmds_light;
	
	uint_t				cmds_translucent_num;
	std::vector<r_command_t>	cmds_translucent;
};



class r_tree_elem_c :
public r_framecount_iface_a,
public r_visframecount_iface_a,
public r_aabb_iface_a
{
public:
	// wether if node a leaf, all tree elements have this
	int			contents;		// -1, to differentiate from leafs
	
	r_tree_elem_c*		parent;
};


class r_node_c : public r_tree_elem_c
{
public:	
	//TODO
};

class r_leaf_c : 
public r_tree_elem_c,
public r_lightframecount_iface_a,
public r_checkcount_iface_a
{
public:	
	std::vector<r_surface_c*>	surfaces;
	std::vector<r_light_c*>		lights;
};


class r_bsptree_node_c : 
public r_node_c
{
public:
	plane_c*		plane;
	r_tree_elem_c*		children[2];
};
	
class r_bsptree_leaf_c : 
public r_leaf_c
{
public:
	int			cluster;
	int			area;
};

/*
class r_bsptree_area_c : 
public r_tree_elem_c
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
	aabb_c			bbox;
	plane_c			plane;
	
	std::vector<vec3_c>	points;
	std::vector<vec3_c>	points_inner;
	
	uint_t			visframe;
	r_frustum_t		frustum;
};
*/

struct r_bsptree_model_t
{
	aabb_c			bbox;
	float			radius;
	
	int			modelsurfaces_first;
	int			modelsurfaces_num;
};

/*
class r_proctree_node_c : 
public r_node_c
{
public:
	plane_c			plane;
	int			children[2];
};
*/

class r_areaportal_c : 
public r_aabb_iface_a
{
public:
	r_areaportal_c(const std::vector<vec3_c> &vertexes, int areas[2]);

	void		adjustFrustum(const r_frustum_c &frustum);
	
private:
	void		clipEdge(const r_frustum_c &frustum, const vec3_c &v1, const vec3_c &v2, vec3_c &out1, vec3_c &out2);
	
public:
	void		drawPortal();
	void		drawFrustum();
	
	inline uint_t	getVisFrameCount() const	{return _visframecount;}
	inline void	setVisFrameCount()		{_visframecount = r_visframecount;}
	inline bool	isVisible() const		{return r_visframecount == _visframecount;}
	
	inline int	getArea(int side) const		{return _areas[side];}
	
	inline const plane_c&	getPlane() const	{return _plane;}
	
	inline const r_frustum_c& getFrustum() const	{return _frustum;}
	
private:
	uint_t			_visframecount;

	int			_areas[2];

	plane_c			_plane;
	
	std::vector<vec3_c>	_vertexes_original;
	std::vector<vec3_c>	_vertexes;
	
	r_frustum_c		_frustum;
};

/*
class r_proctree_area_c : 
public r_leaf_c
{
public:
	r_proc_model_c*			model;
	
	std::vector<r_areaportal_c*>	areaportals;
};
*/

class r_tree_c
{
public:
	virtual void		precacheLight(r_light_c *light) = 0;
	virtual void		update() = 0;
	virtual void		draw() = 0;
	virtual void		drawAreaPortals()		{}
};


enum r_litnode_mode_e
{
	LITNODE_MODE_PRECACHE,
	LITNODE_MODE_STATIC,
	LITNODE_MODE_DYNAMIC
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
	int			pointInArea(const vec3_c &p);
	bool			pointIsVisible(const vec3_c &p);

	// Fills in a list of all the leafs touched
private:
	void			boxLeafs_r(const aabb_c &bbox, std::vector<r_bsptree_leaf_c*> &leafs, r_tree_elem_c *elem);
public:
	void			boxLeafs(const aabb_c &bbox, std::vector<r_bsptree_leaf_c*> &leafs);
	
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
	int			litNode_r(r_tree_elem_c *elem, r_light_c *light, r_litnode_mode_e mode, int count = 0);

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
	std::vector<vec3_c>			_lights;	// for static N dot L lighting
	std::vector<vec4_c>			_colors;
	
	std::vector<index_t>			_indexes;

	int					_planes_num;
	plane_c*				_planes;
	
	std::vector<byte>			_pvs;
	std::vector<boost::dynamic_bitset<byte> >
						_pvs_clusters;
	int					_pvs_clusters_num;
	int					_pvs_clusters_size;
	
	int					_viewcluster;
	int					_viewcluster_old;
	
	GLuint					_vbo_array_buffer;
	GLuint					_vbo_element_array_buffer;
};


/*
class r_proctree_c  : public r_tree_c
{
public:
	r_proctree_c(const std::string &name);
	virtual ~r_proctree_c();
	
	virtual void		precacheLight(r_light_c *light);
	virtual void		update();
	virtual void		draw();
	virtual void		drawAreaPortals();
	
	int			pointInArea_r(const vec3_c &p, int num);
	int			pointInArea(const vec3_c &p);
	
	void			boxAreas_r(const aabb_c &bbox, std::vector<int> &areas, int nodenum);
	void			boxAreas(const aabb_c &bbox, std::vector<int> &areas);
	
private:
	void			updateArea_r(int area, const r_frustum_c &frustum);
//	void			drawArea_r(int area);
//	void			litArea_r(int area, r_light_c *light, bool precache);

	void			loadNodes(char **buf_p);
	void			loadInterAreaPortals(char **buf_p);
		
	//
	// members
	//
	int					_sourcearea;	// the area with the render origin
	
	std::vector<r_areaportal_c*>		_areaportals;

	std::vector<r_proctree_area_c*>		_areas;

	std::vector<r_proctree_node_c*>		_nodes;
};
*/


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

	aabb_c			bbox;
	float			radius;
	vec3_c			translate;
	
	
	std::vector<r_alias_mesh_frame_t*>	meshframes;
	std::vector<r_alias_tag_t*>		tags;
};



//
// skeletal model specific
//
enum
{
	COMPONENT_BIT_TX	= 1 << 0,
	COMPONENT_BIT_TY	= 1 << 1,
	COMPONENT_BIT_TZ	= 1 << 2,
	COMPONENT_BIT_QX	= 1 << 3,
	COMPONENT_BIT_QY	= 1 << 4,
	COMPONENT_BIT_QZ	= 1 << 5
};

struct r_skel_frame_t
{	
	aabb_c			bbox;
	std::vector<float>	components;
};

struct r_skel_channel_t
{
	std::string		name;
	int			parent_index;
	
	uint_t			components_bits;	// e.g. (COMPONENT_BIT_TX | COMPONENT_BIT_TY | COMPONENT_BIT_TZ)
	int			components_offset;
		
	vec3_c			origin;			// relative value to parent bone
	quaternion_c		quat;
};

struct r_skel_bone_t
{
	r_skel_bone_t()
	{
		index = -1;
		parent_index = -1;
	}

	std::string			name;
	int				index;
	
	std::string			parent_name;
	int				parent_index;		// bone parent index (-1 if root)
	
	vec3_c				default_origin;		// absolute value
	quaternion_c			default_quat;
	
	vec3_c				origin;
	quaternion_c			quat;
	
	std::vector<r_skel_bone_t*>	childrens;
};

class r_skel_animation_c :
public r_registrationcount_iface_a
{
public:
	r_skel_animation_c(const std::string &name, byte *buffer, uint_t buffer_size);
	virtual ~r_skel_animation_c();
	
	virtual void		load()				{}
	
	inline const char*	getName() const			{return _name.c_str();}
	
	inline uint_t		getFramesNum() const		{return _frames.size();}
	inline uint_t		getChannelsNum() const		{return _channels.size();}

private:
	std::string		_name;

protected:
	byte*			_buffer;
	uint_t			_buffer_size;

public:
	std::vector<r_skel_frame_t*>	_frames;
	std::vector<r_skel_channel_t*>	_channels;
};

class r_md5_animation_c : public r_skel_animation_c
{
public:
	r_md5_animation_c(const std::string &name, byte *buffer, uint_t buffer_size);
	virtual ~r_md5_animation_c();
	
	virtual void		load();
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
class r_model_c :
public r_aabb_iface_a,
public r_registrationcount_iface_a
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
	virtual void		load()									{}
	//! return axis-aligned bounding box in model space
	virtual const aabb_c	createAABB(r_entity_c *ent) const = 0;
	virtual int		precacheLight(r_entity_c *ent, r_light_c *light) const			{return 0;}
	virtual void		addModelToList(r_entity_c *ent) = 0;
	virtual void		draw(const r_command_t *cmd, r_render_type_e type) = 0;
	virtual void		setupMeshes();
	virtual void		setupVBO()								{}
	virtual bool		setupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name)	{return false;}
	virtual bool		setupAnimation(r_skel_animation_c *anim)				{return false;}
	virtual uint_t		getMeshesNum() const							{return _meshes.size();}
	
	//
	// access
	//
	const char*	getName() const			{return _name.c_str();}
	r_mod_type_t	getType() const			{return _type;}
	
	void		addMesh(r_mesh_c* mesh)			{_meshes.push_back(mesh);}
	void		addShader(r_model_shader_c* shader)	{_shaders.push_back(shader);}
		
	//
	// members
	//
	
private:
	std::string	_name;
protected:
	r_mod_type_t	_type;
	byte*		_buffer;	// for loading
	uint_t		_buffer_size;
	
	std::vector<r_mesh_c*>		_meshes;
	std::vector<r_model_shader_c*>	_shaders;
};


class r_static_model_c :
public r_model_c
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
	virtual const aabb_c	createAABB(r_entity_c *ent) const;
	virtual int		precacheLight(r_entity_c *ent, r_light_c *light) const;
	virtual void		addModelToList(r_entity_c *ent);
	virtual void 		draw(const r_command_t *cmd, r_render_type_e type);
};


class r_ase_model_c :
public r_static_model_c
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


class r_alias_model_c :
public r_model_c
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
	virtual const aabb_c	createAABB(r_entity_c *ent) const;
	virtual void		addModelToList(r_entity_c *ent);
	virtual void 		draw(const r_command_t *cmd, r_render_type_e type);
	virtual void		setupMeshes();
	virtual bool		setupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name);
	
protected:
	bool	cull(r_entity_c *ent);
	void	drawFrameLerp(const r_command_t *cmd, r_render_type_e type);


	std::vector<r_alias_frame_t*>	_frames;
};



class r_md2_model_c :
public r_alias_model_c
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


class r_md3_model_c :
public r_alias_model_c
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
class r_mdc_model_c :
public r_alias_model_c
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


class r_skel_model_c :
public r_model_c
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
	virtual const aabb_c	createAABB(r_entity_c *ent) const;
	virtual void		addModelToList(r_entity_c *ent);
	virtual void 		draw(const r_command_t *cmd, r_render_type_e type);
	virtual bool		setupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name);
	virtual bool		setupAnimation(r_skel_animation_c *anim);
	
	void		addBone(r_skel_bone_t *bone);
	int		getNumForBoneName(const std::string &name);
	
protected:
	bool		cull(r_entity_c *ent);
	void		drawFrameLerp(const r_command_t *cmd, r_render_type_e type);
	
	void		updateBones(const r_entity_c *ent);
	void		updateBones_r(const r_entity_c *ent, int bone_index);
	
	void		drawBones();
	
	std::vector<r_skel_bone_t*>	_bones;
};

/*
class r_mds_model_c :
public r_skel_model_c
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

class r_md5_model_c :
public r_skel_model_c
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


class r_bsp_model_c :
public r_model_c
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
	virtual const aabb_c	createAABB(r_entity_c *ent) const;
	virtual int		precacheLight(r_entity_c *ent, r_light_c *light) const;
	virtual void		addModelToList(r_entity_c *ent);
	virtual void 		draw(const r_command_t *cmd, r_render_type_e type);
	virtual uint_t		getMeshesNum() const							{return _surfaces.size();}
	
private:
	std::vector<r_surface_c*>	_surfaces;
	bool				_inline;
};

/*
class r_proc_model_c : 
public r_model_c,
public r_aabb_iface_a
{  
public:
	//
	// constructor / destructor
	//
	r_proc_model_c(const std::string &name, bool inline_model);
	virtual ~r_proc_model_c();
	
	//
	// virtual functions
	//
	virtual const aabb_c	createAABB(r_entity_c *ent) const;
	virtual void		addModelToList(r_entity_c *ent);
	virtual void 		draw(const r_command_t *cmd, r_render_type_e type);
//	virtual void		setupMeshes();
	virtual void		setupVBO();
	
	void	load(char **buf_p);
	
	std::vector<r_surface_c*>	_surfaces;
	bool				_inline;
	
	GLuint				_vbo_array_buffer;
	GLuint				_vbo_element_array_buffer;
};
*/


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
extern r_image_c*	r_img_currentrender;
extern r_image_c*	r_img_currentrender_depth;
extern r_image_c*	r_img_currentenvironment;

extern r_framebuffer_c*		r_fb_lightview;
extern r_renderbuffer_c*	r_rb_lightview_color;
extern r_renderbuffer_c*	r_rb_lightview_depth;

extern r_shader_c*	r_shader_currentrender;


extern r_scene_t*	r_current_scene;

extern r_frustum_c	r_frustum;


extern std::vector<r_image_c*>	r_images;
extern std::vector<r_image_c*>	r_images_lm;	// lightmap images

extern std::vector<r_renderbuffer_c*>	r_renderbuffers;
extern std::vector<r_framebuffer_c*>	r_framebuffers;

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
extern uint_t	c_cmds_fog;
extern uint_t	c_cmds_postprocess;
extern uint_t	c_triangles;
extern uint_t	c_draws;
extern uint_t	c_expressions;

extern int	time_start;
extern int	time_end;
extern int	time_setup;
extern int	time_zfill;
extern int	time_lighting_static;
extern int	time_lighting_dynamic;
extern int	time_lighting_static;
extern int	time_lighting_static;
extern int	time_extra;
extern int	time_translucent;
extern int	time_post;


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
extern vec3_c		r_origin_old;

extern bool		r_portal_view;	// if true, get vis data at
extern vec3_c		r_portal_org;	// portalorg instead of vieworg

extern bool		r_mirror_view;	// if true, lock pvs

extern bool		r_envmap;

extern plane_c		r_clipplane;



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
extern std::vector<r_entity_c*>		r_entities;
extern std::vector<r_light_c*>		r_lights;

extern int		r_particles_num;
extern r_particle_t	r_particles[MAX_PARTICLES];

extern int		r_polys_num;
extern r_poly_t		r_polys[MAX_POLYS];

extern std::vector<r_contact_t>				r_contacts;

extern r_scene_t	r_scene_main;
extern r_scene_t	r_scene_portal;



extern cvar_t	*r_lefthand;
extern cvar_t	*r_draw2d;
extern cvar_t	*r_drawentities;
extern cvar_t	*r_drawworld;
extern cvar_t	*r_drawparticles;
extern cvar_t	*r_drawpolygons;
extern cvar_t	*r_drawsky;
extern cvar_t	*r_drawextra;
extern cvar_t	*r_drawtranslucent;
extern cvar_t	*r_drawpostprocess;
extern cvar_t	*r_speeds;
extern cvar_t	*r_fullbright;
extern cvar_t	*r_lerpmodels;
extern cvar_t	*r_debug;
extern cvar_t	*r_log;
extern cvar_t	*r_shadows;
extern cvar_t	*r_shadows_alpha;
extern cvar_t	*r_shadows_export;
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
extern cvar_t	*r_showlighttransforms;
extern cvar_t	*r_showentitybboxes;
extern cvar_t	*r_showentitytransforms;
extern cvar_t	*r_showcontacts;
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
extern cvar_t	*r_arb_texture_rectangle;

extern cvar_t	*r_ext_texture3D;
extern cvar_t	*r_ext_compiled_vertex_array;
extern cvar_t	*r_ext_draw_range_elements;
extern cvar_t	*r_ext_texture_filter_anisotropic;
extern cvar_t	*r_ext_texture_filter_anisotropic_level;
extern cvar_t	*r_ext_framebuffer_object;

extern cvar_t	*r_sgix_fbconfig;
extern cvar_t	*r_sgix_pbuffer;

extern cvar_t	*vid_fullscreen;
extern cvar_t	*vid_gamma;
extern cvar_t	*vid_mode;
extern cvar_t	*vid_gldriver;
extern cvar_t	*vid_colorbits;
extern cvar_t	*vid_depthbits;
extern cvar_t	*vid_stencilbits;

extern cvar_t	*vid_pbuffer_width;
extern cvar_t	*vid_pbuffer_height;
extern cvar_t	*vid_pbuffer_colorbits;
extern cvar_t	*vid_pbuffer_alphabits;
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

//void		RB_LockArrays(int vertexes_num);
//void		RB_UnlockArrays();

void		RB_EnableShaderStates(const r_shader_c *shader);
void		RB_DisableShaderStates(const r_shader_c *shader);

void		RB_EnableShaderStageStates(const r_entity_c *ent, const r_shader_stage_c *stage);
void		RB_DisableShaderStageStates(const r_entity_c *ent, const r_shader_stage_c *stage);

float		RB_Evaluate(const r_entity_t &shared, const boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> &info, float default_value);

void		RB_ModifyTextureMatrix(const r_entity_c *ent, const r_shader_stage_c *stage);
void		RB_ModifyOmniLightTextureMatrix(const r_command_t *cmd, const r_shader_stage_c *stage);
void		RB_ModifyOmniLightCubeTextureMatrix(const r_command_t *cmd, const r_shader_stage_c *stage);
void		RB_ModifyProjLightTextureMatrix(const r_command_t *cmd, const r_shader_stage_c *stage);
void		RB_ModifyProjShadowTextureMatrix(const r_command_t *cmd);
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
				vec_t			distance, 
				const matrix_c&		light_attenuation = matrix_identity);



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
// r_entity.cxx
//
void		R_InitEntities();
void		R_ShutdownEntities();

int		R_GetNumForEntity(r_entity_c *ent);
r_entity_c*	R_GetEntityByNum(int num);


//
// r_fbo.cxx
//
void		R_InitFBOs();
void		R_ShutdownFBOs();


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
void		R_InitLights();
void		R_ShutdownLights();

int		R_GetNumForLight(r_light_c *light);
r_light_c*	R_GetLightByNum(int num);


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
void 		R_DrawNULL(const vec3_c &origin, const vec3_c &angles);
void		R_DrawBBox(const aabb_c &bbox, const vec4_c &color = color_white);

void		R_CalcTangentSpace(	vec3_c &tangent, vec3_c &binormal, vec3_c &normal, 
					const vec3_c &v0, const vec3_c &v1, const vec3_c &v2,
					const vec2_c &t0, const vec2_c &t1, const vec2_c &t2,
					const vec3_c &n	);
					
void		R_InitTree(const std::string &name);
void		R_ShutdownTree();

bool		R_Init(void *hinstance, void *hWnd);
void		R_Shutdown();

bool		R_SetupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name);
bool		R_SetupAnimation(int model, int anim);


void		R_BeginFrame();
void		R_RenderFrame(const r_refdef_t &fd);
void		R_EndFrame();



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
/*
void		R_DrawFastSkyBox();
void 		R_ClearSkyBox();
void		R_DrawSky();
void		R_AddSkySurface(r_surface_c *surf);
void 		R_SetSky(const std::string &name);
*/



/*
====================================================================
			GL CONFIG STUFF
====================================================================
*/

struct glconfig_t
{
	int         	renderer;
	const char*	renderer_string;
	const char*	vendor_string;
	const char*	version_string;
	const char*	extensions_string;

	// save here what extensions are currently available and enabled
	bool		arb_multitexture;
	bool		arb_transpose_matrix;
	bool		arb_texture_compression;
	bool		arb_vertex_program;
	bool		arb_vertex_buffer_object;
	bool		arb_occlusion_query;
	bool		arb_shader_objects;
	bool		arb_vertex_shader;
	bool		arb_fragment_shader;
	bool		arb_shading_language_100;
	bool		arb_texture_rectangle;
	
	bool		ext_texture3D;
	bool		ext_compiled_vertex_array;
	bool		ext_draw_range_elements;
	bool		ext_texture_filter_anisotropic;
	bool		ext_framebuffer_object;
};

struct glstate_t
{
	float 		inverse_intensity;
	bool 		fullscreen;

	int     	prev_mode;

	uint_t		current_tmu_images[16];
	matrix_c	current_tmu_mats[16];
	r_entity_c*	current_tmu_entities[16];
	r_light_c*	current_tmu_lights[16];
	uint_t		current_tmu;
	
	uint_t		current_vbo_array_buffer;
	uint_t		current_vbo_vertexes_ofs;

	uint_t		maxtexsize;
	
	int		anisotropylevel;
	
	uint_t		polygon_mode;

	bool		is2d;
	bool		hwgamma;
	bool		active_pbuffer;
	
	matrix_c	matrix_quake_to_opengl;
	matrix_c	matrix_view;			// inverse of camera translation and rotation matrix
	matrix_c	matrix_model;			// each model has its own translation and rotation matrix
	matrix_c	matrix_model_view;		// product of camera and model matrix
	matrix_c	matrix_light;
	matrix_c	matrix_projection;
	matrix_c	matrix_model_view_projection;

	r_vrect_t	vrect_viewport;
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
void		GLimp_ShutdownPbuffer();
void		GLimp_ActivatePbuffer();
void		GLimp_DeactivatePbuffer();
void		GLimp_EnableLogging(bool enable);
void		GLimp_LogNewFrame();

#endif // R_LOCAL_H
