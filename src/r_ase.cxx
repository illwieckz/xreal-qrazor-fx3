/// ============================================================================
/*
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


/// includes ===================================================================
// system -------------------------------------------------------------------
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include 	"r_local.h"

// xreal --------------------------------------------------------------------


static r_ase_model_c*	r_ase_model = NULL;
static r_mesh_c*	r_ase_mesh = NULL;

static int		r_ase_vertexes_num = 0;
static int		r_ase_faces_num = 0;

static int			r_ase_tvertexes_num = 0;
static std::vector<vec3_c>	r_ase_tvertexes;
static int			r_ase_tfaces_num = 0;
static std::vector<int>		r_ase_tindexes;

static float		r_ase_float0;
static float		r_ase_float1;
static float		r_ase_float2;


static void	R_ASE_Version(int version)
{
	if(version != ASE_VERSION)
		ri.Com_Error(ERR_DROP, "R_ASE_Version: wrong version number (%i should be %i)", version, ASE_VERSION);
}

static void	R_ASE_UnknownMaterialOption(char const* begin, char const* end)
{
	//ri.Com_Printf("unknown ASE material option '%s'\n", std::string(begin, end).c_str());
}

static void	R_ASE_AddShader(char const* begin, char const* end)
{
	std::string shader(begin, end);

	if(!X_strncaseequal("Material", shader.c_str(), strlen("Material")))
		r_ase_model->addShader(new r_model_shader_c(shader, R_RegisterShader(shader), X_SURF_NONE, X_CONT_NONE));
	else
		r_ase_model->addShader(new r_model_shader_c(shader, R_RegisterShader("_default"), X_SURF_NONE, X_CONT_NONE));
}

static void	R_ASE_NewMesh(char const* begin, char const* end)
{
	r_ase_tvertexes.clear();
	r_ase_tindexes.clear();
	
	r_ase_mesh = new r_mesh_c();
}


static void	R_ASE_GetVertexesNum(int vertexes_num)
{
	if(vertexes_num <= 0)
		ri.Com_Error(ERR_DROP, "R_ASE_GetVertexesNum: mesh has invalid vertices number %i", vertexes_num);

	r_ase_vertexes_num = vertexes_num;
}

static void	R_ASE_CheckVertexesNum(char const)
{
	if(r_ase_vertexes_num != (int)r_ase_mesh->vertexes.size())
		ri.Com_Error(ERR_DROP, "R_ASE_CheckVertexesNum: mesh has bad vertices number %i != %i", r_ase_vertexes_num, r_ase_mesh->vertexes.size());
}


static void	R_ASE_GetFacesNum(int faces_num)
{
	if(faces_num <= 0)
		ri.Com_Error(ERR_DROP, "R_ASE_GetFacesNum: mesh has invalid faces number %i", faces_num);

	r_ase_faces_num = faces_num;
}

static void	R_ASE_CheckIndexesNum(char const)
{
	if((r_ase_faces_num * 3) != (int)r_ase_mesh->indexes.size())
		ri.Com_Error(ERR_DROP, "R_ASE_CheckIndexesNum: mesh has bad indices number %i != %i", (r_ase_faces_num * 3), r_ase_mesh->indexes.size());
}


static void	R_ASE_GetTVertexesNum(int tvertexes_num)
{
	if(tvertexes_num <= 0)
		ri.Com_Error(ERR_DROP, "R_ASE_GetTVertexesNum: mesh has invalid texcoords number %i", tvertexes_num);

	r_ase_tvertexes_num = tvertexes_num;
}

static void	R_ASE_CheckTVertexesNum(char const)
{
	if(r_ase_tvertexes_num != (int)r_ase_tvertexes.size())
		ri.Com_Error(ERR_DROP, "R_ASE_CheckTVertexesNum: bad texcoords number %i != %i", r_ase_tvertexes_num, r_ase_tvertexes.size());
}


static void	R_ASE_GetTFacesNum(int tfaces_num)
{
	if(tfaces_num <= 0)
		ri.Com_Error(ERR_DROP, "R_ASE_GetTFacesNum: mesh has invalid faces number %i", tfaces_num);

	r_ase_tfaces_num = tfaces_num;
}

static void	R_ASE_CheckTIndexesNum(char const)
{
	if((r_ase_tfaces_num * 3) != (int)r_ase_tindexes.size())
		ri.Com_Error(ERR_DROP, "R_ASE_CheckTIndexesNum: mesh has bad texcoord indices number %i != %i", (r_ase_tfaces_num * 3), r_ase_tindexes.size());
}


static void	R_ASE_Float0(float f0)
{
	r_ase_float0 = f0;
}

static void	R_ASE_Float1(float f1)
{
	r_ase_float1 = f1;
}

static void	R_ASE_Float2(float f2)
{
	r_ase_float2 = f2;
}

static void	R_ASE_PushVertex(char const* begin, char const* end)
{
	r_ase_mesh->vertexes.push_back(vec3_c(r_ase_float0, r_ase_float1, r_ase_float2));
	r_ase_mesh->texcoords.push_back(vec2_c());
	r_ase_mesh->tangents.push_back(vec3_c());
	r_ase_mesh->binormals.push_back(vec3_c());
	r_ase_mesh->normals.push_back(vec3_c());
	r_ase_mesh->colors.push_back(vec4_c());
}

static void	R_ASE_PushVertexIndex(int index)
{
	r_ase_mesh->indexes.push_back(index);
}

static void	R_ASE_PushTVertex(char const* begin, char const* end)
{
	r_ase_tvertexes.push_back(vec3_c(r_ase_float0, 1.0 - r_ase_float1, r_ase_float2));
}

static void	R_ASE_PushTVertexIndex(int index)
{
	r_ase_tindexes.push_back(index);
}

static void	R_ASE_AddMesh(char const begin)
{
	std::reverse(r_ase_mesh->indexes.begin(), r_ase_mesh->indexes.end());
	std::reverse(r_ase_tindexes.begin(), r_ase_tindexes.end());
	
	for(uint_t i=0; i<r_ase_tindexes.size(); i += 3)
	{
		r_ase_mesh->texcoords[r_ase_mesh->indexes[i+0]] = r_ase_tvertexes[r_ase_tindexes[i+0]];
		r_ase_mesh->texcoords[r_ase_mesh->indexes[i+1]] = r_ase_tvertexes[r_ase_tindexes[i+1]];
		r_ase_mesh->texcoords[r_ase_mesh->indexes[i+2]] = r_ase_tvertexes[r_ase_tindexes[i+0]];
	}

	

	r_ase_mesh->createBBoxFromVertexes();
	
	r_ase_model->_bbox.addPoint(r_ase_mesh->bbox._mins);
	r_ase_model->_bbox.addPoint(r_ase_mesh->bbox._maxs);

	r_ase_model->addMesh(r_ase_mesh);
}



struct r_ase_model_grammar_t : public boost::spirit::grammar<r_ase_model_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(r_ase_model_grammar_t const& self)
		{
			// start grammar definition
			restofline
				=	boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::eol_p | boost::spirit::ch_p('}'))]]
				;
				
			skip_restofline
				=	restofline
				;
				
			skip_block
				=	boost::spirit::ch_p('{') >>
					*skip_restofline >>
					boost::spirit::ch_p('}')
				;
				
			scene
				=	boost::spirit::str_p("*SCENE") >> skip_block
				;
				
			material_list
				=	boost::spirit::str_p("*MATERIAL_LIST") >> boost::spirit::ch_p('{') >>
					boost::spirit::str_p("*MATERIAL_COUNT") >> boost::spirit::int_p >>
					+material >> 
					boost::spirit::ch_p('}')
				;
				
			material
				=	boost::spirit::str_p("*MATERIAL") >> boost::spirit::int_p >> boost::spirit::ch_p('{') >>
					+(material_map_diffuse | material_submaterial | material_option) >>
					boost::spirit::ch_p('}')
				;
				
			material_option
				=	material_name	|
					restofline[&R_ASE_UnknownMaterialOption]
				;
			
			material_name
				=	boost::spirit::str_p("*MATERIAL_NAME") >>
					boost::spirit::ch_p('\"') >>
					//boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')]][&R_ASE_AddShader] >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')][&R_ASE_AddShader] >>
					skip_restofline
				;
				
			material_map_diffuse
				=	boost::spirit::str_p("*MAP_DIFFUSE") >> skip_block
				;
				
			material_submaterial
				=	boost::spirit::str_p("*SUBMATERIAL") >> boost::spirit::int_p >> skip_block
				;
				
			geomobject
				=	boost::spirit::str_p("*GEOMOBJECT") >> boost::spirit::ch_p('{') >>
					boost::spirit::str_p("*NODE_NAME") >> skip_restofline >>
					boost::spirit::str_p("*NODE_TM") >> skip_block >> 
					+mesh >>
					*skip_restofline >>
					boost::spirit::ch_p('}')
				;
				
			mesh
				= 	boost::spirit::str_p("*MESH")[&R_ASE_NewMesh] >> boost::spirit::ch_p('{') >>
					boost::spirit::str_p("*TIMEVALUE") >> boost::spirit::int_p >>
					boost::spirit::str_p("*MESH_NUMVERTEX") >> boost::spirit::int_p[&R_ASE_GetVertexesNum] >>
					boost::spirit::str_p("*MESH_NUMFACES") >> boost::spirit::int_p[&R_ASE_GetFacesNum] >>
					mesh_vertex_list >>
					mesh_face_list >>
					boost::spirit::str_p("*MESH_NUMTVERTEX") >> boost::spirit::int_p[&R_ASE_GetTVertexesNum] >>
					mesh_tvertlist >>
					boost::spirit::str_p("*MESH_NUMTVFACES") >> boost::spirit::int_p[&R_ASE_GetTFacesNum] >>
					mesh_tfacelist >>
					*skip_restofline >>
					boost::spirit::ch_p('}')[&R_ASE_AddMesh]
				;
				
			mesh_vertex_list
				=	boost::spirit::str_p("*MESH_VERTEX_LIST") >> boost::spirit::ch_p('{') >>
					+mesh_vertex[&R_ASE_PushVertex] >>
					boost::spirit::ch_p('}')[&R_ASE_CheckVertexesNum]
				;
				
			mesh_vertex
				=	boost::spirit::str_p("*MESH_VERTEX") >> 
					boost::spirit::int_p >> 
					boost::spirit::real_p[&R_ASE_Float0] >>
					boost::spirit::real_p[&R_ASE_Float1] >>
					boost::spirit::real_p[&R_ASE_Float2]
				;
				
			mesh_face_list
				=	boost::spirit::str_p("*MESH_FACE_LIST") >> boost::spirit::ch_p('{') >>
					+mesh_face >>
					boost::spirit::ch_p('}')[&R_ASE_CheckIndexesNum]
				;
				
			mesh_face
				=	boost::spirit::str_p("*MESH_FACE") >>
					boost::spirit::int_p >> boost::spirit::ch_p(':') >>
					boost::spirit::str_p("A:") >> boost::spirit::int_p[&R_ASE_PushVertexIndex] >>
					boost::spirit::str_p("B:") >> boost::spirit::int_p[&R_ASE_PushVertexIndex] >>
					boost::spirit::str_p("C:") >> boost::spirit::int_p[&R_ASE_PushVertexIndex] >>
					skip_restofline
				;
				
			mesh_tvertlist
				=	boost::spirit::str_p("*MESH_TVERTLIST") >> boost::spirit::ch_p('{') >>
					+mesh_tvert[&R_ASE_PushTVertex] >>
					boost::spirit::ch_p('}')[&R_ASE_CheckTVertexesNum]
				;
				
			mesh_tvert
				=	boost::spirit::str_p("*MESH_TVERT") >>
					boost::spirit::int_p >>
					boost::spirit::real_p[&R_ASE_Float0] >>
					boost::spirit::real_p[&R_ASE_Float1] >>
					boost::spirit::real_p[&R_ASE_Float2]
				;
				
			mesh_tfacelist
				=	boost::spirit::str_p("*MESH_TFACELIST") >> boost::spirit::ch_p('{') >>
					+mesh_tface >>
					boost::spirit::ch_p('}')[&R_ASE_CheckTIndexesNum]
				;
				
			mesh_tface
				=	boost::spirit::str_p("*MESH_TFACE") >>
					boost::spirit::int_p >> 
					boost::spirit::int_p[&R_ASE_PushTVertexIndex] >>
					boost::spirit::int_p[&R_ASE_PushTVertexIndex] >>
					boost::spirit::int_p[&R_ASE_PushTVertexIndex]
				;
				
			expression
				=	boost::spirit::str_p("*3DSMAX_ASCIIEXPORT") >> boost::spirit::int_p[&R_ASE_Version] >>
					boost::spirit::str_p("*COMMENT") >> restofline >>
					scene >>
					material_list >>
					geomobject >>
					*boost::spirit::anychar_p
				;
				
			// end grammar definiton
		}
		
		boost::spirit::rule<ScannerT>	restofline,
						skip_restofline,
						skip_block,
						
						scene,
						material_list,
							material,
								material_option,
									material_name,
								material_map_diffuse,
								material_submaterial,
						geomobject,
							mesh,
								mesh_vertex_list,
									mesh_vertex,
								mesh_face_list,
									mesh_face,
								mesh_tvertlist,
									mesh_tvert,
								mesh_tfacelist,
									mesh_tface,
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};


r_ase_model_c::r_ase_model_c(const std::string &name, byte *buffer, uint_t buffer_size)
:r_static_model_c(name, buffer, buffer_size)
{
	//ri.Com_Printf("r_ase_model_c::ctor: %s\n", name.c_str());
}

r_ase_model_c::~r_ase_model_c()
{
	//DO NOTHING
}

void	r_ase_model_c::load()
{
	r_ase_model = this;

	std::string exp = (const char*)_buffer;
	r_ase_model_grammar_t grammar;

	boost::spirit::parse_info<> info = boost::spirit::parse(exp.c_str(), grammar, boost::spirit::space_p);
	
	if(!info.full)
		ri.Com_Error(ERR_DROP, "r_ase_model_c::load: parsing failed for '%s'", getName());
		
		
	

	/*
	char *data_p = (char*)_buffer;
	
	Com_Parse(&data_p);	// skip ident
		
	int version = Com_ParseInt(&data_p);
	
	if(version != ASE_VERSION)
		ri.Com_Error(ERR_DROP, "r_ase_model_c::load: %s has wrong version number (%i should be %i)", getName(), version, ASE_VERSION);
	
	
	while(true)
	{
		char *token = Com_Parse(&data_p);
		
		if(!token[0])
			break;
			
		if(!data_p)
			break;
			
		if(X_strequal(token, "*MATERIAL_LIST"))
		{
			loadMaterials(&data_p);
		}
		else if(X_strequal(token, "*GEOMOBJECT"))
		{
			loadGeomObject(&data_p);
		}
	}
	*/
	
#if 1
	ri.Com_Printf("r_ase_model_c::load: model '%s' has %i meshes\n", getName(), _meshes.size());
	ri.Com_Printf("r_ase_model_c::load: model '%s' has %i shaders\n", getName(), _shaders.size());
#endif
}

/*
void	r_ase_model_c::loadMaterials(char **data_p)
{
	char *token	= NULL;

	Com_Parse(data_p, false);	// skip '{'
	
	token = Com_Parse(data_p, true);
	if(!X_strequal(token, "*MATERIAL_COUNT"))
	{
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMaterials: found '%s' when expecting *MATERIAL_COUNT", token);
		return;
	}
	
	int materialsnum = Com_ParseInt(data_p, false);
	if(materialsnum < 0)
	{
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMaterials: bad materials number %i", materialsnum);
		return;
	}
	
	for(int i=0; i<materialsnum; i++)
	{
		loadMaterial(data_p);
	}
	
	token = Com_Parse(data_p, true);
	if(!X_strequal(token, "}"))
	{
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMaterials: found '%s' when expecting '}'", token);
		return;
	}
}

void	r_ase_model_c::loadMaterial(char **data_p)
{
	ri.Com_Printf("r_ase_model_c::loadMaterial:\n");
	
	char *token = Com_Parse(data_p, true);
	if(!X_strequal(token, "*MATERIAL"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMaterial: found '%s' when expecting *MATERIAL", token);

	while(true)
	{
		token = Com_Parse(data_p);
		
		if(token[0] == '}')
			break;
		
		if(!*data_p)
			ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMaterial: EOF without closing brace");
		
		if(X_strequal(token, "*MATERIAL_NAME"))
		{
			char *shader = Com_Parse(data_p, false);	//shader name
			
			if(!X_strnequal(shader, "Material #", 10))
				_shaders.push_back(new r_model_shader_c(shader, R_RegisterShader(shader)));
			else
				_shaders.push_back(new r_model_shader_c(shader, R_RegisterShader("_default")));
		}
		else if(X_strequal(token, "*MAP_DIFFUSE"))
		{
			skipDiffuseMap(data_p);
		}
		else if(X_strequal(token, "*SUBMATERIAL"))
		{
			skipSubMaterial(data_p);
		}
	}
}

void	r_ase_model_c::skipDiffuseMap(char **data_p)
{
	ri.Com_Printf("r_ase_model_c::skipDiffuseMap:\n");

	char *token = Com_Parse(data_p, false);
	if(!X_strequal(token, "{"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::skipDiffuseMap: found '%s' instead of '{'", token);

	while(true)
	{
		char *token = Com_Parse(data_p, true);
				
		if(token[0] == '}')
			break;
			
		if(!*data_p)
			ri.Com_Error(ERR_DROP, "r_ase_model_c::skipDiffuseMap: EOF without closing brace");
	}
}

void	r_ase_model_c::skipSubMaterial(char **data_p)
{
	ri.Com_Printf("r_ase_model_c::skipSubMaterial:\n");

	Com_Parse(data_p, false);	// skip number
	
	char *token = Com_Parse(data_p, false);
	if(!X_strequal(token, "{"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::skipSubMaterial: found '%s' instead of '{'", token);

	while(true)
	{
		char *token = Com_Parse(data_p);
				
		if(token[0] == '}')
			break;
			
		if(!*data_p)
			ri.Com_Error(ERR_DROP, "r_ase_model_c::skipSubMaterial: EOF without closing brace");
	}
}

void	r_ase_model_c::loadGeomObject(char **data_p)
{
	ri.Com_Printf("r_ase_model_c::loadGeomObject:\n");

	char *token = Com_Parse(data_p, false);
	
	if(!X_strequal(token, "{"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadGeomObject: found '%s' instead of '{'", token);
	
	while(true)
	{
		token = Com_Parse(data_p, true);
		
		if(!token[0])
			break;
			
		if(!*data_p)
			ri.Com_Error(ERR_DROP, "r_ase_model_c::loadGeomObject: EOF without closing brace");
		
		if(X_strequal(token, "*MESH"))
		{
			loadMesh(data_p);
		}
	}
}

void	r_ase_model_c::loadMesh(char **data_p)
{
	ri.Com_Printf("r_ase_model_c::loadMesh:\n");

	char *token = Com_Parse(data_p, false);
	if(!X_strequal(token, "{"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: found '%s' instead of '{'", token);

	r_mesh_c *poutmesh = new r_mesh_c();
			
	Com_Parse(data_p, true);	// skip "*TIMEVALUE"
	Com_Parse(data_p, false);	// skip value
		
	Com_Parse(data_p, true);	// skip "*MESH_NUMVERTEX"
	int vertexes_num = Com_ParseInt(data_p, false);
	if(vertexes_num <= 0)
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: mesh in model '%s' has invalid vertexes number %i", getName(), vertexes_num);	
	
	poutmesh->fillVertexes(vertexes_num);
		
	Com_Parse(data_p, true);	// skip "*MESH_NUMFACES"
	int faces_num = Com_ParseInt(data_p, false);
	if(faces_num <= 0)
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: mesh in model '%s' has invalid faces number %i", getName(), faces_num);
			
	token = Com_Parse(data_p, true);
	if(!X_strequal(token, "*MESH_VERTEX_LIST"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: found '%s' when expecting *MESH_VERTEX_LIST", token);
	Com_Parse(data_p, false);	// skip '{'
	for(int i=0; i<vertexes_num; i++)
	{
		Com_Parse(data_p, true);	// skip "*MESH_VERTEX"
		Com_Parse(data_p, false);	// skip vertex number
				
		poutmesh->vertexes[i][0] = Com_ParseFloat(data_p, false);
		poutmesh->vertexes[i][1] = Com_ParseFloat(data_p, false);
		poutmesh->vertexes[i][2] = Com_ParseFloat(data_p, false);
	}
	Com_Parse(data_p, true);	// skip '}'
			
		
	token = Com_Parse(data_p, true);
	if(!X_strequal(token, "*MESH_FACE_LIST"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: found '%s' when expecting *MESH_FACE_LIST", token);
	Com_Parse(data_p, false);	// skip '{'
	poutmesh->indexes = std::vector<index_t>(faces_num * 3);
	for(int i=0; i<faces_num; i++)
	{
		token = Com_Parse(data_p, true);
		if(!X_strequal(token, "*MESH_FACE"))
			ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: found '%s' when expecting *MESH_FACE", token);
		
		int face_num = Com_ParseInt(data_p, false);	// face number
				
		Com_Parse(data_p, false);	// skip "A:"
		poutmesh->indexes[face_num+0] = Com_ParseInt(data_p, false);
			
		Com_Parse(data_p, false);	// skip "B:"
		poutmesh->indexes[face_num+1] = Com_ParseInt(data_p, false);
				
		Com_Parse(data_p, false);	// skip "C:"
		poutmesh->indexes[face_num+2] = Com_ParseInt(data_p, false);
			
		while(true)	// skip rest of line
		{
			token = Com_Parse(data_p, false);
			
			if(!token[0])
				break;
		}
	}
	Com_Parse(data_p, true);	// skip '}'
	
	
	token = Com_Parse(data_p, true);
	if(!X_strequal(token, "*MESH_NUMTVERTEX"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: found '%s' when expecting *MESH_NUMTVERTEX", token);
		
	int texcoords_num = Com_ParseInt(data_p, false);
	if(texcoords_num <= 0)
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: mesh in model '%s' has invalid texcoords number %i", getName(), texcoords_num);
			
	token = Com_Parse(data_p, true);
	if(!X_strequal(token, "*MESH_TVERTLIST"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: found '%s' when expecting *MESH_TVERTLIST", token);
	Com_Parse(data_p, false);	// skip '{'
	std::vector<vec2_c> texcoords(texcoords_num);
	for(int i=0; i<texcoords_num; i++)
	{
		token = Com_Parse(data_p, true);
		if(!X_strequal(token, "*MESH_TVERT"))
			ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: found '%s' when expecting *MESH_TVERT", token);
			
		Com_Parse(data_p, false);	// skip number
				
		texcoords[i][0] = Com_ParseFloat(data_p, false);
		texcoords[i][1] = Com_ParseFloat(data_p, false);
			
		Com_Parse(data_p, false);	// skip last texcoord
	}
	Com_Parse(data_p, true);	// skip '}'
	
	token = Com_Parse(data_p, true);
	if(!X_strequal(token, "*MESH_NUMTVFACES"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: found '%s' when expecting *MESH_TVFACES", token);
	int tfaces_num = Com_ParseInt(data_p, false);
	
	if(faces_num != tfaces_num)
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: mesh in model '%s' has faces_num != tfaces_num", getName());
			
	token = Com_Parse(data_p, true);
	if(!X_strequal(token, "*MESH_TFACELIST"))
		ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: found '%s' when expecting *MESH_TFACELIST", token);
	Com_Parse(data_p, false);	// skip '{'
	//std::vector<index_t> tindexes(tfaces_num * 4);
	for(int i=0; i<tfaces_num; i++)
	{
		token = Com_Parse(data_p, true);
		if(!X_strequal(token, "*MESH_TFACE"))
			ri.Com_Error(ERR_DROP, "r_ase_model_c::loadMesh: found '%s' when expecting *MESH_TFACE", token);
			
		int face_num = Com_ParseInt(data_p, false);	// face number
		
		poutmesh->texcoords[poutmesh->indexes[face_num+0]] = texcoords[Com_ParseInt(data_p, false)];
		poutmesh->texcoords[poutmesh->indexes[face_num+1]] = texcoords[Com_ParseInt(data_p, false)];
		poutmesh->texcoords[poutmesh->indexes[face_num+2]] = texcoords[Com_ParseInt(data_p, false)];
				
		//tindexes[i+0] = Com_ParseInt(data_p, false);
		//tindexes[i+1] = Com_ParseInt(data_p, false);
		//tindexes[i+2] = Com_ParseInt(data_p, false);
	}
	Com_Parse(data_p, true);	// skip '}'
	
	//std::reverse(tindexes.begin(), tindexes.end());
	
	//for(int i=0; i<(tfaces_num * 3); i += 3)
	//{
	//	poutmesh->texcoords[poutmesh->indexes[i+0]] = texcoords[tindexes[i+0]];
	//	poutmesh->texcoords[poutmesh->indexes[i+1]] = texcoords[tindexes[i+1]];
	//	poutmesh->texcoords[poutmesh->indexes[i+2]] = texcoords[tindexes[i+2]];
	//}
	
	//std::reverse(poutmesh->indexes.begin(), poutmesh->indexes.end());
			
	poutmesh->createBBoxFromVertexes();
	
	_bbox.addPoint(poutmesh->bbox._mins);
	_bbox.addPoint(poutmesh->bbox._maxs);
	
	_meshes.push_back(poutmesh);
}
*/

