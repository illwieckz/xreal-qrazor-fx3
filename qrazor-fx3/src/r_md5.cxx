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

static r_md5_model_c*	r_md5_model = NULL;
static r_skel_bone_t*	r_md5_bone = NULL;
static r_skel_mesh_c*	r_md5_mesh = NULL;

static int		r_md5_joints_num = 0;
static int		r_md5_meshes_num = 0;

static int		r_md5_triangles_num = 0;

static float		r_md5_float0;
static float		r_md5_float1;
static float		r_md5_float2;

static int		r_md5_vertex_index = 0;
static int		r_md5_weight_index;



static void	R_MD5Mesh_Version(int version)
{
	if(version != MD5_VERSION)
		Com_Error(ERR_DROP, "R_MD5_Version: wrong version number (%i should be %i)", version, MD5_VERSION);
}

static void	R_MD5Mesh_JointsNum(int joints_num)
{
	if(joints_num <= 0)
		ri.Com_Error(ERR_DROP, "R_MD5Mesh_JointsNum: model has invalid joints number %i", joints_num);

	r_md5_joints_num = joints_num;
}

static void	R_MD5Mesh_MeshesNum(int meshes_num)
{
	if(meshes_num <= 0)
		ri.Com_Error(ERR_DROP, "R_MD5Mesh_MeshesNum: model has invalid meshes number %i", meshes_num);

	r_md5_meshes_num = meshes_num;
}


static void	R_MD5Mesh_CreateBone(char const* begin, char const* end)
{
	r_md5_bone = new r_skel_bone_t();
	r_md5_bone->name = std::string(begin, end);
}

static void	R_MD5Mesh_BoneIndex(int parent_index)
{
	r_md5_bone->parent_index = parent_index;
}

static void	R_MD5Mesh_BoneOrigin(char const)
{
	r_md5_bone->position.set(r_md5_float0, r_md5_float1, r_md5_float2);
}

static void	R_MD5Mesh_BoneRotation(char const)
{
	r_md5_bone->quat.set(	r_md5_float0,
				r_md5_float1,
				r_md5_float2	);
}

static void	R_MD5Mesh_AddBone(char const* begin, char const* end)
{
	r_md5_model->addBone(r_md5_bone);
}

static void	R_MD5Mesh_AddShader(char const* begin, char const* end)
{
	std::string shader(begin, end);

	r_md5_model->addShader(new r_model_shader_c(shader, R_RegisterShader(shader), X_SURF_NONE, X_CONT_NONE));
}


static void	R_MD5Mesh_NewMesh(char const* begin, char const* end)
{
	r_md5_mesh = new r_skel_mesh_c();
}


static void	R_MD5Mesh_VertexesNum(int vertexes_num)
{
	if(vertexes_num <= 0)
		ri.Com_Error(ERR_DROP, "R_MD5Mesh_VertexesNum: mesh has invalid vertices number %i", vertexes_num);

	r_md5_mesh->fillVertexes(vertexes_num);
		
	r_md5_mesh->weights_first = std::vector<int>(vertexes_num);
	r_md5_mesh->weights_num   = std::vector<int>(vertexes_num);
	
	r_md5_mesh->vertexweights = std::vector<std::vector<r_skel_weight_t*> >(vertexes_num);
}

static void	R_MD5Mesh_VertexIndex(int vertex_index)
{
	if(vertex_index < 0 || vertex_index >= (int)r_md5_mesh->vertexes.size())
		ri.Com_Error(ERR_DROP, "R_MD5Mesh_VertexIndex: vertex out of range %i", vertex_index);
		
	r_md5_vertex_index = vertex_index;
}

static void	R_MD5Mesh_VertexTexCoord0(float texcoord)
{
	r_md5_mesh->texcoords[r_md5_vertex_index][0] = texcoord;
}

static void	R_MD5Mesh_VertexTexCoord1(float texcoord)
{
	r_md5_mesh->texcoords[r_md5_vertex_index][1] = texcoord;
}

static void	R_MD5Mesh_VertexWeightsFirst(int weights_first)
{
	r_md5_mesh->weights_first[r_md5_vertex_index] = weights_first;
}

static void	R_MD5Mesh_VertexWeightsNum(int weights_num)
{
	r_md5_mesh->weights_num[r_md5_vertex_index] = weights_num;
}

static void	R_MD5Mesh_TrianglesNum(int triangles_num)
{
	if(triangles_num <= 0)
		ri.Com_Error(ERR_DROP, "R_MD5Mesh_TrianglesNum: mesh has invalid triangles number %i", triangles_num);

	r_md5_triangles_num = triangles_num;
}

static void	R_MD5Mesh_PushVertexIndex(int index)
{
	r_md5_mesh->indexes.push_back(index);
}

static void	R_MD5Mesh_WeightsNum(int weights_num)
{
	if(weights_num <= 0)
		ri.Com_Error(ERR_DROP, "R_MD5Mesh_WeightsNum: mesh has invalid weights number %i", weights_num);
	
	r_md5_mesh->weights = std::vector<r_skel_weight_t>(weights_num);
}

static void	R_MD5Mesh_WeightIndex(int index)
{
	if(index < 0 || index >= (int)r_md5_mesh->weights.size())
		ri.Com_Error(ERR_DROP, "R_MD5Mesh_Index: weight out of range %i", index);
		
	r_md5_weight_index = index;
}

static void	R_MD5Mesh_WeightBone(int index)
{
	r_md5_mesh->weights[r_md5_weight_index].bone_index = index;
}

static void	R_MD5Mesh_WeightWeight(float weight)
{
	r_md5_mesh->weights[r_md5_weight_index].weight = weight;
}

static void	R_MD5Mesh_WeightPosition0(float val)
{
	r_md5_mesh->weights[r_md5_weight_index].position[0] = val;
}

static void	R_MD5Mesh_WeightPosition1(float val)
{
	r_md5_mesh->weights[r_md5_weight_index].position[1] = val;
}

static void	R_MD5Mesh_WeightPosition2(float val)
{
	r_md5_mesh->weights[r_md5_weight_index].position[2] = val;
}

static void	R_MD5Mesh_AddMesh(char const)
{
	//if(r_md5_vertexes_num != (int)r_md5_mesh->vertexes.size())
	//	ri.Com_Error(ERR_DROP, "R_MD5Mesh_AddMesh: mesh has bad vertices number %i != %i", r_md5_vertexes_num, r_md5_mesh->vertexes.size());
		
	if((r_md5_triangles_num * 3) != (int)r_md5_mesh->indexes.size())
		ri.Com_Error(ERR_DROP, "R_MD5Mesh_AddMesh: mesh has bad indices number %i != %i", (r_md5_triangles_num * 3), r_md5_mesh->indexes.size());
		
	
	// setup vertex weights
	for(uint_t j=0; j<r_md5_mesh->vertexes.size(); j++)
	{					
		for(int k=0; k<r_md5_mesh->weights_num[j]; k++)
		{
			r_md5_mesh->vertexweights[j].push_back(&r_md5_mesh->weights[r_md5_mesh->weights_first[j] + k]);
		}
	}	
	
	r_md5_model->addMesh(r_md5_mesh);
}



static void	R_MD5Mesh_Float0(float f0)
{
	r_md5_float0 = f0;
}

static void	R_MD5Mesh_Float1(float f1)
{
	r_md5_float1 = f1;
}

static void	R_MD5Mesh_Float2(float f2)
{
	r_md5_float2 = f2;
}


struct r_md5_model_grammar_t : public boost::spirit::grammar<r_md5_model_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(r_md5_model_grammar_t const& self)
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
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('}')] >>
					boost::spirit::ch_p('}')
				;
				
			ident
				=	boost::spirit::str_p("MD5Version") >> boost::spirit::int_p[&R_MD5Mesh_Version]
				;
				
			commandline
				=	boost::spirit::str_p("commandline") >>
					boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')] >>
					boost::spirit::ch_p('\"')
				;
				
			numjoints
				=	boost::spirit::str_p("numJoints") >> boost::spirit::int_p[&R_MD5Mesh_JointsNum]
				;
				
			nummeshes
				=	boost::spirit::str_p("numMeshes") >> boost::spirit::int_p[&R_MD5Mesh_MeshesNum]
				;
				
			joints
				=	boost::spirit::str_p("joints") >> boost::spirit::ch_p('{') >>
					+joint[&R_MD5Mesh_AddBone] >> 
					boost::spirit::ch_p('}')
				;
				
			joint
				=	boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')][&R_MD5Mesh_CreateBone] >>
					boost::spirit::ch_p('\"') >>
					boost::spirit::int_p[&R_MD5Mesh_BoneIndex] >>
					boost::spirit::ch_p('(') >>
					boost::spirit::real_p[&R_MD5Mesh_Float0] >>
					boost::spirit::real_p[&R_MD5Mesh_Float1] >>
					boost::spirit::real_p[&R_MD5Mesh_Float2] >>
					boost::spirit::ch_p(')')[&R_MD5Mesh_BoneOrigin] >>
					boost::spirit::ch_p('(') >>
					boost::spirit::real_p[&R_MD5Mesh_Float0] >>
					boost::spirit::real_p[&R_MD5Mesh_Float1] >>
					boost::spirit::real_p[&R_MD5Mesh_Float2] >>
					boost::spirit::ch_p(')')[&R_MD5Mesh_BoneRotation]
				;
				
			mesh
				=	boost::spirit::str_p("mesh")[&R_MD5Mesh_NewMesh] >> boost::spirit::ch_p('{') >>
					boost::spirit::str_p("shader") >> 
					boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')][&R_MD5Mesh_AddShader] >>
					boost::spirit::ch_p('\"') >>
					boost::spirit::str_p("numverts") >> boost::spirit::int_p[&R_MD5Mesh_VertexesNum] >>
					+vert >>
					boost::spirit::str_p("numtris") >> boost::spirit::int_p[&R_MD5Mesh_TrianglesNum] >>
					+triangle >>
					boost::spirit::str_p("numweights") >> boost::spirit::int_p[&R_MD5Mesh_WeightsNum] >>
					+weight >>
					boost::spirit::ch_p('}')[&R_MD5Mesh_AddMesh]
				;
			
			vert
				=	boost::spirit::str_p("vert") >>
					boost::spirit::int_p[&R_MD5Mesh_VertexIndex] >>
					boost::spirit::ch_p('(') >>
					boost::spirit::real_p[&R_MD5Mesh_VertexTexCoord0] >>
					boost::spirit::real_p[&R_MD5Mesh_VertexTexCoord1] >>
					boost::spirit::ch_p(')') >>
					boost::spirit::int_p[&R_MD5Mesh_VertexWeightsFirst] >>
					boost::spirit::int_p[&R_MD5Mesh_VertexWeightsNum]
				;
				
			triangle
				=	boost::spirit::str_p("tri") >>
					boost::spirit::int_p >>
					boost::spirit::int_p[&R_MD5Mesh_PushVertexIndex] >>
					boost::spirit::int_p[&R_MD5Mesh_PushVertexIndex] >>
					boost::spirit::int_p[&R_MD5Mesh_PushVertexIndex]
				;
				
			weight
				=	boost::spirit::str_p("weight") >>
					boost::spirit::int_p[&R_MD5Mesh_WeightIndex] >>
					boost::spirit::int_p[&R_MD5Mesh_WeightBone] >>
					boost::spirit::real_p[&R_MD5Mesh_WeightWeight] >>
					boost::spirit::ch_p('(') >>
					boost::spirit::real_p[&R_MD5Mesh_WeightPosition0] >>
					boost::spirit::real_p[&R_MD5Mesh_WeightPosition1] >>
					boost::spirit::real_p[&R_MD5Mesh_WeightPosition2] >>
					boost::spirit::ch_p(')')
				;
				
			expression
				=	ident >>
					commandline >>
					numjoints >>
					nummeshes >>
					joints >>
					+mesh
				;
				
			// end grammar definiton
		}
			
		boost::spirit::rule<ScannerT>	restofline,
						skip_restofline,
						skip_block,
						
						ident,
						commandline,
						numjoints,
						nummeshes,
						joints,
							joint,
						mesh,
							vert,
							triangle,
							weight,
						
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};


r_md5_model_c::r_md5_model_c(const std::string &name, byte *buffer, uint_t buffer_size)
:r_skel_model_c(name, buffer, buffer_size)
{
	//ri.Com_Printf("r_md5_model_c::ctor: %s\n", name.c_str());
}

r_md5_model_c::~r_md5_model_c()
{
	//DO NOTHING
}

void	r_md5_model_c::load()
{
	r_md5_model = this;

	std::string exp = (const char*)_buffer;
	r_md5_model_grammar_t grammar;

	boost::spirit::parse_info<> info = boost::spirit::parse
	(
		exp.c_str(),
		grammar,
		boost::spirit::space_p ||
		boost::spirit::comment_p("/*", "*/") ||
		boost::spirit::comment_p("//")
	);
	
	if(!info.full)
		ri.Com_Error(ERR_DROP, "r_ase_model_c::load: parsing failed for '%s'", getName());

	/*
	char *data_p = (char*)_buffer;
	
	Com_Parse(&data_p);	// skip ident
		
	int version = Com_ParseInt(&data_p);
	
	if(version != MD5_VERSION)
		ri.Com_Error(ERR_DROP, "r_md5_model_c::load: %s has wrong version number (%i should be %i)", getName(), version, MD5_VERSION);
	
	
	Com_Parse(&data_p);	// skip "commandline"
	Com_Parse(&data_p);	// skip commandline string
	
	loadBones(&data_p);
	loadMeshes(&data_p);
	*/
	
#if 0
	ri.Com_Printf("r_md5_model_c::load: model '%s' has %i bones\n", getName(), _bones.size());
	ri.Com_Printf("r_md5_model_c::load: model '%s' has %i meshes\n", getName(), _meshes.size());
	ri.Com_Printf("r_md5_model_c::load: model '%s' finished\n", getName());
#endif
}


/*
void	r_md5_model_c::loadBones(char **data_p)
{
	r_skel_bone_t*	poutbone = NULL;
	char*		token = NULL;
	
	Com_Parse(data_p);	// skip "numbones"
	int bones_num = Com_ParseInt(data_p);
			
	if(bones_num <= 0)
		ri.Com_Error(ERR_DROP, "r_md5_model_c::loadBones: model '%s' has invalid bones number %i", getName(), bones_num);
		
	//
	// create custom bones
	//
	for(int i=0; i<bones_num; i++)
	{
		poutbone = new r_skel_bone_t();
	
		Com_Parse(data_p, true);	// skip "bone"
		Com_Parse(data_p, false);	// skip bone number
		Com_Parse(data_p, false);	// skip '{'
		
		while(true)
		{
			token = Com_Parse(data_p, true);
		
			if(X_strequal(token, "name"))
			{
				poutbone->name = Com_Parse(data_p, false); // bone name
			}
			else if(X_strequal(token, "bindpos"))
			{
				// parse bindpos
				for(int j=0; j<3; j++)
				{
					poutbone->position[j] = Com_ParseFloat(data_p);
				}
			}
			else if(X_strequal(token, "bindmat"))
			{
				// parse bind matrix
				poutbone->matrix.identity();
				for(int j=0; j<3; j++)
				{
					for(int k=0; k<3; k++)
					{
						poutbone->matrix[j][k] = Com_ParseFloat(data_p, false);
					}
				}
				poutbone->matrix.transpose();
		
				//ri.Com_Printf("r_md5_model_c::loadBones: bone '%s' has matrix:\n%s\n", poutbone->name.c_str(), poutbone->matrix.toString());
			}
			else if(X_strequal(token, "parent"))
			{
				poutbone->parent_name = Com_Parse(data_p, false);
				poutbone->parent_index = getNumForBoneName(poutbone->parent_name);
			}
			else if(X_strequal(token, "}"))
			{
				break;
			}
			else
			{
				ri.Com_Error(ERR_DROP, "r_md5_model_c::loadBones: found invalid token '%s'", token);
				return;
			}
		}
		
		_bones.push_back(poutbone);
	}
	

	//
	// create boundsMin bone
	//
	poutbone = new r_skel_bone_t();
	poutbone->name = "boundsMin";
	poutbone->matrix.identity();
	_bones.push_back(poutbone);
	
	
	//
	// create boundsMax bone
	//
	poutbone = new r_skel_bone_t();
	poutbone->name = "boundsMax";
	poutbone->matrix.identity();
	_bones.push_back(poutbone);

	
	//
	// setup bone children
	//
	for(std::vector<r_skel_bone_t*>::const_iterator ir = _bones.begin(); ir != _bones.end(); ir++)
	{
		r_skel_bone_t* bone = *ir;
		
		if(bone->parent_index != -1)
		{
			_bones[bone->parent_index]->children.push_back(bone);
		}
	}
	
	
	//for(std::vector<r_skel_bone_t*>::const_iterator ir = _bones.begin(); ir != _bones.end(); ir++)
	//	ri.Com_Printf("r_md5_model_c::loadBones: bone '%s' has %i children\n", (*ir)->name.c_str(), (*ir)->children.size());
}
*/

/*
void	r_md5_model_c::loadMeshes(char **data_p)
{
	Com_Parse(data_p);	// skip "nummeshes"
	int meshes_num = Com_ParseInt(data_p);
			
	if(meshes_num <= 0)
		ri.Com_Error(ERR_DROP, "r_md5_model_c::loadMeshes: model '%s' has invalid meshes number %i", getName(), meshes_num);

	for(int i=0; i<meshes_num; i++)
	{
		r_skel_mesh_c *poutmesh = new r_skel_mesh_c();
		
		Com_Parse(data_p);	// skip "mesh"
		poutmesh->name = Com_Parse(data_p);
		Com_Parse(data_p);	// skip '{'
		
		
		//
		// load the shader
		//
		Com_Parse(data_p);	// skip "shader"
		std::string shader = Com_Parse(data_p);	//shader name
		r_model_shader_c *poutshader = new r_model_shader_c(shader, R_RegisterShader(shader));
			
		_shaders.push_back(poutshader);
	

		//
		// parse vertices
		//
		Com_Parse(data_p, true);	// skip "numverts"
		
		int vertexes_num = Com_ParseInt(data_p, false);				
		if(vertexes_num <= 0)
			ri.Com_Error(ERR_DROP, "r_md5_model_c::loadMeshes: mesh '%s' in model '%s' has invalid vertexes number %i", poutmesh->name.c_str(), getName(), vertexes_num);
		
		poutmesh->fillVertexes(vertexes_num);
		
		poutmesh->weights_first = std::vector<int>(vertexes_num);	
		poutmesh->weights_num   = std::vector<int>(vertexes_num);
		
		for(int j=0; j<vertexes_num; j++)
		{			
			Com_Parse(data_p, true);	// skip "vert"
			Com_Parse(data_p, false);	// skip vertex number
			
			poutmesh->texcoords[j][0] = Com_ParseFloat(data_p, false);
			poutmesh->texcoords[j][1] = Com_ParseFloat(data_p, false);
			
			poutmesh->weights_first[j] = Com_ParseInt(data_p, false);
			poutmesh->weights_num[j] = Com_ParseInt(data_p, false);
		}
		
		
		//		
		// parse triangles
		//
		Com_Parse(data_p, true);	// skip "numtriangles"
		
		int triangles_num = Com_ParseInt(data_p, false);
		if(triangles_num <= 0)
			ri.Com_Error(ERR_DROP, "r_md5_model_c::loadMeshes: mesh '%s' in model '%s' has invalid triangles number %i", poutmesh->name.c_str(), getName(), triangles_num);
			
		std::vector<index_t> indexes;
		for(int j=0; j<triangles_num; j++)
		{	
			Com_Parse(data_p, true);	// skip "tri"
			Com_Parse(data_p, false);	// skip triangle number
			
			for(int k=0; k<3; k++)
				indexes.push_back(Com_ParseInt(data_p, false));
		}
		
		poutmesh->indexes = indexes;
		poutmesh->normals_tri = std::vector<vec3_c>(triangles_num);

		
		//		
		// parse weights
		//
		Com_Parse(data_p, true);	// skip "numweights"
		
		int weights_num = Com_ParseInt(data_p, false);
		if(weights_num <= 0)
			ri.Com_Error(ERR_DROP, "r_md5_model_c::loadMeshes: mesh '%s' in model '%s' has invalid weights number %i", poutmesh->name.c_str(), getName(), weights_num);
		
		poutmesh->weights = std::vector<r_skel_weight_t>(weights_num);
		for(int j=0; j<weights_num; j++)
		{
			Com_Parse(data_p, true);	// skip "weight"
			Com_Parse(data_p, false);	// skip weight number
			
			poutmesh->weights[j].bone_index = Com_ParseInt(data_p, false);
			poutmesh->weights[j].weight = Com_ParseFloat(data_p, false);
			
			for(int k=0; k<3; k++)
				poutmesh->weights[j].position[k] = Com_ParseFloat(data_p, false);
		}
		
		Com_Parse(data_p);	// skip '}'
		
		
		//
		// setup vertex weights
		//
		poutmesh->vertexweights = std::vector<std::vector<r_skel_weight_t*> >(vertexes_num);
		for(int j=0; j<vertexes_num; j++)
		{					
			for(int k=0; k<poutmesh->weights_num[j]; k++)
			{
				poutmesh->vertexweights[j].push_back(&poutmesh->weights[poutmesh->weights_first[j] + k]);
			}
		}
		
	
		_meshes.push_back(poutmesh);
	}
}
*/
