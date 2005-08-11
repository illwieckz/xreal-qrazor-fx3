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
//	if(vertexes_num <= 0)
//		ri.Com_Error(ERR_DROP, "R_ASE_GetVertexesNum: mesh has invalid vertices number %i", vertexes_num);

	r_ase_vertexes_num = vertexes_num;
}

static void	R_ASE_CheckVertexesNum(char const)
{
	if(r_ase_vertexes_num != (int)r_ase_mesh->vertexes.size())
		ri.Com_Error(ERR_DROP, "R_ASE_CheckVertexesNum: mesh has bad vertices number %i != %i", r_ase_vertexes_num, r_ase_mesh->vertexes.size());
}


static void	R_ASE_GetFacesNum(int faces_num)
{
//	if(faces_num <= 0)
//		ri.Com_Error(ERR_DROP, "R_ASE_GetFacesNum: mesh has invalid faces number %i", faces_num);

	r_ase_faces_num = faces_num;
}

static void	R_ASE_CheckIndexesNum(char const)
{
	if((r_ase_faces_num * 3) != (int)r_ase_mesh->indexes.size())
		ri.Com_Error(ERR_DROP, "R_ASE_CheckIndexesNum: mesh has bad indices number %i != %i", (r_ase_faces_num * 3), r_ase_mesh->indexes.size());
}


static void	R_ASE_GetTVertexesNum(int tvertexes_num)
{
//	if(tvertexes_num <= 0)
//		ri.Com_Error(ERR_DROP, "R_ASE_GetTVertexesNum: mesh has invalid texcoords number %i", tvertexes_num);

	r_ase_tvertexes_num = tvertexes_num;
}

static void	R_ASE_CheckTVertexesNum(char const)
{
	if(r_ase_tvertexes_num != (int)r_ase_tvertexes.size())
		ri.Com_Error(ERR_DROP, "R_ASE_CheckTVertexesNum: bad texcoords number %i != %i", r_ase_tvertexes_num, r_ase_tvertexes.size());
}


static void	R_ASE_GetTFacesNum(int tfaces_num)
{
//	if(tfaces_num <= 0)
//		ri.Com_Error(ERR_DROP, "R_ASE_GetTFacesNum: mesh has invalid faces number %i", tfaces_num);

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
	r_ase_mesh->lights.push_back(vec3_c());
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
		r_ase_mesh->texcoords[r_ase_mesh->indexes[i+2]] = r_ase_tvertexes[r_ase_tindexes[i+2]];
	}

	r_ase_mesh->createBBoxFromVertexes();

	if(r_ase_mesh->vertexes.size() && r_ase_mesh->indexes.size())
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
					boost::spirit::ch_p('\"')
				;
				
			material_map_diffuse
				=	boost::spirit::str_p("*MAP_DIFFUSE") >> skip_block
				;
				
			material_submaterial
				=	boost::spirit::str_p("*SUBMATERIAL") >> boost::spirit::int_p >> 
					boost::spirit::ch_p('{') >>
					*(material_map_diffuse | skip_restofline) >>
					boost::spirit::ch_p('}')
				;
				
			geomobject
				=	boost::spirit::str_p("*GEOMOBJECT") >> boost::spirit::ch_p('{') >>
					node_name >>
					!node_parent >>
					node_tm >>
					+mesh >>
					*skip_restofline >>
					boost::spirit::ch_p('}')
				;
				
			node_name
				=	boost::spirit::str_p("*NODE_NAME") >> 
					boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')] >>
					boost::spirit::ch_p('\"')
				;
				
			node_parent
				=	boost::spirit::str_p("*NODE_PARENT") >>
					boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')] >>
					boost::spirit::ch_p('\"')
				;
				
			node_tm
				=	boost::spirit::str_p("*NODE_TM") >> skip_block
				;
				
			mesh
				= 	boost::spirit::str_p("*MESH")[&R_ASE_NewMesh] >> boost::spirit::ch_p('{') >>
					boost::spirit::str_p("*TIMEVALUE") >> boost::spirit::int_p >>
					boost::spirit::str_p("*MESH_NUMVERTEX") >> boost::spirit::int_p[&R_ASE_GetVertexesNum] >>
					boost::spirit::str_p("*MESH_NUMFACES") >> boost::spirit::int_p[&R_ASE_GetFacesNum] >>
					mesh_vertex_list >>
					mesh_face_list >>
					!(boost::spirit::str_p("*MESH_NUMTVERTEX") >> boost::spirit::int_p[&R_ASE_GetTVertexesNum]) >>
					!mesh_tvertlist >>
					!(boost::spirit::str_p("*MESH_NUMTVFACES") >> boost::spirit::int_p[&R_ASE_GetTFacesNum]) >>
					!mesh_tfacelist >>
					!(boost::spirit::str_p("*MESH_NUMCVERTEX") >> boost::spirit::int_p) >>
					!mesh_cvertlist >>
					!(boost::spirit::str_p("*MESH_NUMCVFACES") >> boost::spirit::int_p) >>
					!mesh_cfacelist >>
					!mesh_normals >>
					*skip_restofline >>
					boost::spirit::ch_p('}')[&R_ASE_AddMesh]
				;
				
			mesh_vertex_list
				=	boost::spirit::str_p("*MESH_VERTEX_LIST") >> boost::spirit::ch_p('{') >>
					*mesh_vertex[&R_ASE_PushVertex] >>
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
					*mesh_face >>
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
				
			mesh_cvertlist
				=	boost::spirit::str_p("*MESH_CVERTLIST") >> boost::spirit::ch_p('{') >>
					+mesh_vertcol >>
					boost::spirit::ch_p('}')
				;
				
			mesh_vertcol
				=	boost::spirit::str_p("*MESH_VERTCOL") >>
					boost::spirit::int_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p
				;
				
			mesh_cfacelist
				=	boost::spirit::str_p("*MESH_CFACELIST") >> boost::spirit::ch_p('{') >>
					+mesh_cface >>
					boost::spirit::ch_p('}')
				;
				
			mesh_cface
				=	boost::spirit::str_p("*MESH_CFACE") >>
					boost::spirit::int_p >> 
					boost::spirit::int_p >>
					boost::spirit::int_p >>
					boost::spirit::int_p
				;
				
			mesh_normals
				=	boost::spirit::str_p("*MESH_NORMALS") >> boost::spirit::ch_p('{') >>
					*(	mesh_facenormal >> 
						mesh_vertexnormal >> 
						mesh_vertexnormal >>
						mesh_vertexnormal
					) >>
					boost::spirit::ch_p('}')
				;
				
			mesh_facenormal
				=	boost::spirit::str_p("*MESH_FACENORMAL") >>
					boost::spirit::int_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p
				;
				
			mesh_vertexnormal
				=	boost::spirit::str_p("*MESH_VERTEXNORMAL") >>
					boost::spirit::int_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p
				;
				
			expression
				=	boost::spirit::str_p("*3DSMAX_ASCIIEXPORT") >> boost::spirit::int_p[&R_ASE_Version] >>
					boost::spirit::str_p("*COMMENT") >> restofline >>
					scene >>
					material_list >>
					+geomobject// >>
					//*boost::spirit::anychar_p
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
							node_name,
							node_parent,
							node_tm,
							mesh,
								mesh_vertex_list,
									mesh_vertex,
								mesh_face_list,
									mesh_face,
								mesh_tvertlist,
									mesh_tvert,
								mesh_tfacelist,
									mesh_tface,
								mesh_cvertlist,
									mesh_vertcol,
								mesh_cfacelist,
									mesh_cface,
								mesh_normals,
									mesh_facenormal,
									mesh_vertexnormal,
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
;
	r_ase_model_grammar_t grammar;

	boost::spirit::parse_info<> info = boost::spirit::parse((const char*)_buffer, grammar, boost::spirit::space_p);
	
	if(!info.full)
		ri.Com_Error(ERR_DROP, "r_ase_model_c::load: parsing failed for '%s'", getName());

#if 1
	ri.Com_Printf("r_ase_model_c::load: model '%s' has %i meshes\n", getName(), _meshes.size());
	ri.Com_Printf("r_ase_model_c::load: model '%s' has %i shaders\n", getName(), _shaders.size());
#endif
}

