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
#include <set>

// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"



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
			;
	}
} r_shader_parameter_symbols_p;


struct r_shader_table_symbols_t : boost::spirit::symbols<uint_t, char>//, std::set<const char*> >
{
	// add symbols later
} r_shader_table_symbols_p;


void	R_SetReal(boost::spirit::tree_node<boost::spirit::node_val_data<r_iterator_t, r_node_data_t> > &node, r_iterator_t begin, r_iterator_t end)
{
	//ri.Com_Printf("access node: real %f\n", atof(std::string(begin, end).c_str()));
		
	node.value.value(atof(std::string(begin, end).c_str()));
}

static r_shader_parms_e r_shader_parm_tmp;
void	R_GetParm(r_shader_parms_e parm)
{
	//ri.Com_Printf("access node: parm %i\n", parm);
	
	r_shader_parm_tmp = parm;
}

void	R_SetParm(boost::spirit::tree_node<boost::spirit::node_val_data<r_iterator_t, r_node_data_t> > &node, r_iterator_t begin, r_iterator_t end)
{
	//ri.Com_Printf("access node: parm\n");
		
	node.value.value(r_shader_parm_tmp);
}


static unsigned int r_shader_table_tmp;
void	R_GetTable(unsigned int table)
{
//	ri.Com_Printf("access node: table %i\n", table);
	
	r_shader_table_tmp = table;
}

void	R_SetTable(boost::spirit::tree_node<boost::spirit::node_val_data<r_iterator_t, r_node_data_t> > &node, r_iterator_t begin, r_iterator_t end)
{
//	ri.Com_Printf("access node: table\n");
	
	node.value.value(r_shader_table_tmp);
}


struct r_shader_line_grammar_t : public boost::spirit::grammar<r_shader_line_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(r_shader_line_grammar_t const& self)
		{
			// start grammar definition
			real
				=	boost::spirit::access_node_d[boost::spirit::real_p][&R_SetReal]
				;
				
			parm
				=	boost::spirit::access_node_d[boost::spirit::nocase_d[r_shader_parameter_symbols_p[&R_GetParm]]][&R_SetParm]
				;
			
			table_index
				=	boost::spirit::access_node_d[boost::spirit::nocase_d[r_shader_table_symbols_p[&R_GetTable]]][&R_SetTable]
				;
				
			table_eval
				= 	table_index >> table_value
				;
							
			// table value on the right side [...] expression used by all tables
			table_value
				=	boost::spirit::inner_node_d[boost::spirit::ch_p('[') >> expression >> boost::spirit::ch_p(']')]
				;
				
			factor
				=	real
				|	parm
				|	(boost::spirit::inner_node_d[boost::spirit::ch_p('(') >> expression >> boost::spirit::ch_p(')')])
				|	table_eval
				|	(boost::spirit::root_node_d[boost::spirit::ch_p('-')] >> factor)
				|	(boost::spirit::root_node_d[boost::spirit::ch_p('+')] >> factor)
				;
				
			term
				=	factor >> 
				*(	(boost::spirit::root_node_d[boost::spirit::ch_p('*')] >> factor) |
					(boost::spirit::root_node_d[boost::spirit::ch_p('/')] >> factor)
				)
				;
		
			expression
				= 	term >>
				*(	(boost::spirit::root_node_d[boost::spirit::ch_p('+')] >> term) |
					(boost::spirit::root_node_d[boost::spirit::ch_p('-')] >> term)
				)
				;
				
			// end grammar definiton
		}
		
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_REAL> >			real;
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_PARM> >			parm;
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_TABLE_EVAL> >		table_eval;
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_TABLE_INDEX> >		table_index;
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_TABLE_VALUE> >		table_value;
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_FACTOR> >			factor;
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_TERM> >			term;
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_EXPRESSION> >		expression;		
		
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_EXPRESSION> > const&
		start() const { return expression; }
	};
};



static r_shader_c*		r_current_shader;
static r_shader_stage_c*	r_current_stage;


struct r_shader_map_stc_grammar_t : public boost::spirit::grammar<r_shader_map_stc_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(r_shader_map_stc_grammar_t const& self)
		{
			// start grammar definition
			makeintensity
				=
					boost::spirit::nocase_d[boost::spirit::str_p("makeintensity")][boost::spirit::assign_a(r_current_stage->make_intensity, true)] >>
					boost::spirit::ch_p('(') >>
					boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::space_p | boost::spirit::ch_p(')'))]][boost::spirit::assign(r_current_stage->image_name)] >>
					boost::spirit::ch_p(')')
				;
				
			makealpha
				=
					boost::spirit::nocase_d[boost::spirit::str_p("makealpha")][boost::spirit::assign_a(r_current_stage->make_alpha, true)] >>
					boost::spirit::ch_p('(') >>
					boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::space_p | boost::spirit::ch_p(')'))]][boost::spirit::assign(r_current_stage->image_name)] >>
					boost::spirit::ch_p(')')
				;
			
			heightmap
				=
					boost::spirit::nocase_d[boost::spirit::str_p("heightmap")] >>
					boost::spirit::ch_p('(') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p(',')] >>
					boost::spirit::ch_p(',') >>
					boost::spirit::real_p >>
					boost::spirit::ch_p(')')
				;
				
			addnormals
				=	boost::spirit::nocase_d[boost::spirit::str_p("addnormals")] >>
					boost::spirit::ch_p('(') >>
					boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::space_p | boost::spirit::ch_p(','))]][boost::spirit::assign(r_current_stage->image_name)] >>
					boost::spirit::ch_p(',') >>
					heightmap >>
					boost::spirit::ch_p(')')
				;
				
			imagename
				= boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::space_p]][boost::spirit::assign(r_current_stage->image_name)]
				;
						
			expression
				=	makeintensity
				|	makealpha
				|	addnormals
				|	heightmap
				|	imagename
				;
				
			// end grammar definiton
		}
		
		boost::spirit::rule<ScannerT>	imagename;
		boost::spirit::rule<ScannerT>	addnormals;
		boost::spirit::rule<ScannerT>	heightmap;
		boost::spirit::rule<ScannerT>	makeintensity;
		boost::spirit::rule<ScannerT>	makealpha;
		boost::spirit::rule<ScannerT>	expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};





static void	R_Map_stc(char const* begin, char const *end);





//static bool	R_ParseExpressionToAST(const std::string &exp, boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> &info);

r_shader_stage_c::r_shader_stage_c()
{
	type			= SHADER_MATERIAL_STAGE_TYPE_COLORMAP;
	type_light		= SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY;

	flags			= 0;
	
	blend_src		= 0;
	blend_dst		= 0;
	
	rgb_gen 		= SHADER_RGB_GEN_IDENTITY;
	
	alpha_gen		= SHADER_ALPHA_GEN_IDENTITY;
	
	tcmod_cmds.clear();
	
//	R_ParseExpressionToAST("1.0",		bump_scale);

//	R_ParseExpressionToAST("0.04",		height_scale);
//	R_ParseExpressionToAST("-0.02",		height_bias);
//	R_ParseExpressionToAST("0.128",		height_scale);
//	R_ParseExpressionToAST("-0.064",	height_bias);

//	R_ParseExpressionToAST("32.0",		specular_exponent);

//	R_ParseExpressionToAST("1.0",		refraction_index);
	
//	R_ParseExpressionToAST("2.0",		fresnel_power);
//	R_ParseExpressionToAST("2.0",		fresnel_scale);
//	R_ParseExpressionToAST("1.0",		fresnel_bias);
	
//	R_ParseExpressionToAST("1.1",		eta);
//	R_ParseExpressionToAST("-0.02",		eta_delta);

	make_intensity		= false;
	make_alpha		= false;
	
	image_name		= "";
	image			= NULL;
}

r_shader_stage_c::~r_shader_stage_c()
{
}

r_shader_c::r_shader_c(const std::string &name, r_shader_type_e type)
{
	_name	= name;
	_type	= type;
	
	_sort	= SHADER_SORT_FARTHEST;
	_flags	= 0;
	
	setRegistrationSequence();
	
	stage_diffusemap	= NULL;
	stage_bumpmap		= NULL;
	stage_specularmap	= NULL;
	
	stage_lightmap		= NULL;
	stage_deluxemap		= NULL;
	
	stage_attenuationmap_z	= NULL;
	stage_attenuationmap_cube	= NULL;
}


r_shader_c::~r_shader_c()
{
	X_purge<std::vector<r_shader_stage_c*> >(stages);
}



class r_shader_cache_c
{
private:
	r_shader_cache_c();
	r_shader_cache_c(const r_shader_cache_c &cache);

public:
	
	r_shader_cache_c(const std::string &name, const std::string &path, unsigned int offset_begin, unsigned int offset_end)
	{
		_name = Com_StripExtension(name);
		_path = path;
		_offset_begin = offset_begin;
		_offset_end = offset_end;
	}
	
	const char*	getName()		{return _name.c_str();}
	const char*	getPath()		{return _path.c_str();}
	unsigned int	getBeginOffset()	{return _offset_begin;}
	unsigned int	getEndOffset()		{return _offset_end;}
	
private:
	std::string	_name;
	std::string	_path;
	unsigned int	_offset_begin;
	unsigned int	_offset_end;
};


static std::vector<r_shader_c*>		r_shaders;
static std::vector<r_shader_cache_c*>	r_shaders_cache;	


// Tr3B - parses a single shader line expression and builds an Abstract Syntax Tree, which we can evaluate
//  in the renderer backend later
static bool	R_ParseExpressionToAST(r_iterator_t begin, r_iterator_t end, boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> &info)
{
	r_shader_line_grammar_t		grammar;
	
	info = boost::spirit::ast_parse<r_factory_t>(begin, end, grammar, boost::spirit::space_p);
		
	return info.full;
}

/*
static bool	R_ParseExpressionToAST(const std::string &exp, boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> &info)
{
	return R_ParseExpressionToAST(exp.begin(), exp.end(), info);
}
*/

// Tr3B - parses a list like "anyname { 0, .3, 2.0, 5 }"
/*
static bool	R_ParseExpressionToFloatVector(const std::string &exp, std::string &name, std::vector<float> &v)
{	
	boost::spirit::parse_info<> info = boost::spirit::parse(exp.c_str(),
		(
			// start grammar
			boost::spirit::refactor_unary_d[*boost::spirit::anychar_p - boost::spirit::ch_p('{')][boost::spirit::assign(name)] >>
			boost::spirit::ch_p('{') >>			
			boost::spirit::list_p(boost::spirit::lexeme_d[boost::spirit::real_p[boost::spirit::append(v)]], ',') >>
			boost::spirit::ch_p('}')
			// end grammar	
		), boost::spirit::space_p);
		
	name = X_strlwr(name);
	
	return info.full;
}
*/

// Tr3B - parses a comma separated list of any strings
static bool	R_ParseExpressionToStringVector(const std::string &exp, std::vector<std::string> &v, bool skip_space = true)
{
	boost::spirit::parse_info<> info;

 	if(skip_space)
	{
		info = boost::spirit::parse(exp.c_str(),
			(
			// start grammar
			boost::spirit::list_p((*boost::spirit::lexeme_d[boost::spirit::anychar_p])[boost::spirit::append(v)], ',')
			// end grammar	
			)
		, boost::spirit::space_p);
	}
	else
	{
		info = boost::spirit::parse(exp.c_str(),
			(
			// start grammar
			boost::spirit::list_p((*boost::spirit::anychar_p)[boost::spirit::append(v)], ',')
			// end grammar	
			)
			);
	}
	
	return info.full;
}


static bool	R_SplitExpression(const std::string &exp, std::string &exp0, std::string &exp1, bool skip_space = true)
{
	std::vector<std::string>	v;
	
	std::vector<std::string>	parms;

	if(!R_ParseExpressionToStringVector(exp, parms, skip_space))
		return false;
	
	if(parms.size() != 2)
	{
		//ri.Com_Printf("R_SplitExpression: bad number of parms %i\n", parms.size());
		return false;
	}
	
	exp0 = parms[0];
	exp1 = parms[1];
	
	return true;
}

static bool	R_SplitExpression(const std::string &exp, std::string &exp0, std::string &exp1, std::string &exp2, bool skip_space = true)
{
	std::vector<std::string>	v;
	
	std::vector<std::string>	parms;

	if(!R_ParseExpressionToStringVector(exp, parms, skip_space))
		return false;
	
	if(parms.size() != 3)
	{
		return false;
	}
	
	exp0 = parms[0];
	exp1 = parms[1];
	exp2 = parms[2];
	
	return true;
}

static void	R_SetShaderStageImageFlags(const r_shader_stage_c *stage, int &flags)
{
	if(stage->flags & SHADER_STAGE_NOMIPMAPS)
		flags |= IMAGE_NOMIPMAP;
		
	if(stage->flags & SHADER_STAGE_NOPICMIP)
		flags |= IMAGE_NOPICMIP;
		
	if(stage->flags & SHADER_STAGE_CLAMP)
		flags |= IMAGE_CLAMP;
	
	if(stage->flags & SHADER_STAGE_ZEROCLAMP)
		flags |= IMAGE_CLAMP_TO_BORDER;
		
	if(stage->flags & SHADER_STAGE_FORCEHIGHQUALITY)
		flags |= IMAGE_NOCOMPRESSION;
}


/*
================================================================================
			Q3A SHADER KEYWORDS
================================================================================
*/

/*
void	R_SkyParms_sk(r_shader_c *shader, r_shader_stage_c &stage, const std::vector<std::string> &argv, const std::string &args)
{
	shader->flags |= SHADER_SKY;
	shader->sort = SHADER_SORT_SKY;
}
*/

/*
void	R_Cull_sk(r_shader_c *shader, r_shader_stage_c &stage, const std::vector<std::string> &argv, const std::string &args)
{
	// reset shader cull flags
	shader->flags &= ~(SHADER_CULL_FRONT | SHADER_CULL_BACK);
	
	if(argv.size() == 0)
	{

		// if there are no arguments it defaults to "cull font"
		shader->flags |= SHADER_CULL_FRONT;
		return;
	}
	else
	{
		if(	X_strcaseequal(argv[0].c_str(), "disable")	||
			X_strcaseequal(argv[0].c_str(), "none")	||
			X_strcaseequal(argv[0].c_str(), "twosided")
		)
		{
			// don't add any cull flags
		}
		else if(X_strcaseequal(argv[0].c_str(), "front"))
			shader->flags |= SHADER_CULL_FRONT;
		
		else if(	X_strcaseequal(argv[0].c_str(), "back")	||
				X_strcaseequal(argv[0].c_str(), "backside")	||
				X_strcaseequal(argv[0].c_str(), "backsided")
		)
			shader->flags |= SHADER_CULL_BACK;
		else
			shader->flags |= SHADER_CULL_FRONT;		
	}
}
*/

/*
void	R_FogParms_sk(r_shader_c *shader, r_shader_stage_c &stage, const std::vector<std::string> &argv, const std::string &args)
{
	ri.Com_DPrintf("R_FogParms_sk: TODO\n");
	
	//TODO
}
*/

/*
void	R_Portal_sk(r_shader_c *shader, r_shader_stage_c &stage, const std::vector<std::string> &argv, const std::string &args)
{
	shader->sort = SHADER_SORT_PORTAL;
}
*/

/*
void	R_Sort_sk(r_shader_c *shader, r_shader_stage_c &stage, const std::vector<std::string> &argv, const std::string &args)
{
	if(X_strcaseequal(argv[0].c_str(), "portal"))
		shader->sort = SHADER_SORT_PORTAL;
	
	else if(X_strcaseequal(argv[0].c_str(), "sky"))
		shader->sort = SHADER_SORT_SKY;
	
	else if(X_strcaseequal(argv[0].c_str(), "opaque"))
		shader->sort = SHADER_SORT_OPAQUE;
		
	else if(X_strcaseequal(argv[0].c_str(), "banner"))
		shader->sort = SHADER_SORT_BANNER;

	else if(X_strcaseequal(argv[0].c_str(), "underwater"))
		shader->sort = SHADER_SORT_UNDERWATER;
	
	else if(X_strcaseequal(argv[0].c_str(), "additive"))
		shader->sort = SHADER_SORT_ADDITIVE;
		
	else if(X_strcaseequal(argv[0].c_str(), "nearest"))
		shader->sort = SHADER_SORT_NEAREST;

	else
	{
		shader->sort = atoi(argv[0].c_str());
		
		X_clamp(shader->sort, SHADER_SORT_NONE, SHADER_SORT_NEAREST);
	}
}
*/

/*
void	R_Q3MapSurfaceLight_sk(r_shader_c *shader, r_shader_stage_c &stage, const std::vector<std::string> &argv, const std::string &args)
{
	//shader->_flags |= SHADER_NOLIGHTING;
}
*/

/*
void	R_Q3MapLightImage_sk(r_shader_c *shader, r_shader_stage_c &stage, const std::vector<std::string> &argv, const std::string &args)
{
	//shader->_flags |= SHADER_NOLIGHTING;
}
*/

/*
void	R_SurfaceParm_sk(r_shader_c *shader, r_shader_stage_c &stage, const std::vector<std::string> &argv, const std::string &args)
{
	if(X_strcaseequal(argv[0].c_str(), "nodlight"))
		shader->flags |= SHADER_NOLIGHTING;
	
	else if(X_strcaseequal(argv[0].c_str(), "nolightmap"))
		shader->flags |= SHADER_NOLIGHTING;
}
*/


/*
================================================================================
				SHADER KEYWORDS
================================================================================
*/

static void	R_ColorMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_COLORMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_DiffuseMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DIFFUSEMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_diffusemap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_BumpMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_BUMPMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_bumpmap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_SpecularMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_SPECULARMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_specularmap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_LightMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_LIGHTMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_lightmap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_DeluxeMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DELUXEMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_deluxemap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_ReflectionMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_REFLECTIONMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_RefractionMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_REFRACTIONMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_DispersionMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DISPERSIONMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_LiquidMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_LIQUIDMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_NoDraw_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_NODRAW);
}

static void	R_NoShadows_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_NODRAW | SHADER_NOSHADOWS);
}

static void	R_NoSelfShadow_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_NOSELFSHADOW);
}

static void	R_SortFarthest_sc(char const* begin, char const* end)
{
	r_current_shader->setSort(SHADER_SORT_FARTHEST);
}

static void	R_SortFar_sc(char const* begin, char const* end)
{
	r_current_shader->setSort(SHADER_SORT_FAR);
}

static void	R_SortClose_sc(char const* begin, char const* end)
{
	r_current_shader->setSort(SHADER_SORT_CLOSE);
}

static void	R_SortDecal_sc(char const* begin, char const* end)
{
	r_current_shader->setSort(SHADER_SORT_DECAL);
}

static void	R_SortNearest_sc(char const* begin, char const* end)
{
	r_current_shader->setSort(SHADER_SORT_NEAREST);
}

static void	R_Sort_sc(int sort)
{
	r_current_shader->setSort(sort);
}

static void	R_TwoSided_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_TWOSIDED);
}

static void	R_Translucent_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_TRANSLUCENT);
}

static void	R_PolygonOffset_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_POLYGONOFFSET);
}

static void	R_Discrete_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_DISCRETE);
}

static void	R_ForceOpaque_sc(char const* begin, char const* end)
{
	//r_current_shader->setSort(SHADER_SORT_OPAQUE);	//FIXME
}

static void	R_LightFalloffImage_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_Z;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_attenuationmap_z = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

static void	R_AmbientLight_sc(char const* begin, char const* end)
{
	r_current_shader->setLightType(SHADER_LIGHT_AMBIENT);
}

static void	R_FogLight_sc(char const* begin, char const* end)
{
	r_current_shader->setLightType(SHADER_LIGHT_FOG);
}

static void	R_DecalMacro_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_POLYGONOFFSET);
	r_current_shader->setORFlags(SHADER_DISCRETE);
	r_current_shader->setORFlags(SHADER_NOSHADOWS);
	r_current_shader->setORFlags(SHADER_TRANSLUCENT);
	r_current_shader->setORFlags(SHADER_POLYGONOFFSET);
	
	/*
		polygonoffset \
			discrete \
			noShadows \
			translucent \
			sort 1
	*/
}

static void	R_DeformFlare_sc(char const* begin, char const* end)
{
	//TODO
	
	r_current_shader->setDeformType(SHADER_DEFORM_FLARE);
}

static void	R_SurfaceparmAreaPortal_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_AREAPORTAL);
}



/*
================================================================================
			SHADER STAGE KEYWORDS
================================================================================
*/

static void	R_Blend_stc(char const* begin, char const *end)
{
	std::string exp(begin, end);

	std::vector<std::string>	parms;

	if(!R_ParseExpressionToStringVector(exp, parms))
	{
		ri.Com_Printf("R_Blend_stc: parsing failed\n");
		return;
	}

	if(parms.size() != 2)
	{
		ri.Com_Printf("R_Blend_stc: bad number of parms %i\n", parms.size());
		return;
	}
	
	for(int i=0; i<2; i++)
	{
		unsigned int& blend = (i == 0) ? r_current_stage->blend_src : r_current_stage->blend_dst;
			
		if(X_strcaseequal(parms[i].c_str(), "gl_zero"))
			blend = GL_ZERO;
			
		else if(X_strcaseequal(parms[i].c_str(), "gl_one"))
			blend = GL_ONE;
				
		else if(X_strcaseequal(parms[i].c_str(), "gl_src_color"))
			blend = GL_SRC_COLOR;
				
		else if(X_strcaseequal(parms[i].c_str(), "gl_one_minus_src_color"))
			blend = GL_ONE_MINUS_SRC_COLOR;
				
		else if(X_strcaseequal(parms[i].c_str(), "gl_src_alpha"))
			blend = GL_SRC_ALPHA;
				
		else if(X_strcaseequal(parms[i].c_str(), "gl_one_minus_src_alpha"))
			blend = GL_ONE_MINUS_SRC_ALPHA;
				
		else if(X_strcaseequal(parms[i].c_str(), "gl_dst_alpha"))
			blend = GL_DST_ALPHA;
				
		else if(X_strcaseequal(parms[i].c_str(), "gl_one_minus_dst_alpha"))
			blend = GL_ONE_MINUS_DST_ALPHA;
			
		else if(X_strcaseequal(parms[i].c_str(), "gl_dst_color"))
			blend = GL_DST_COLOR;
			
		else if(X_strcaseequal(parms[i].c_str(), "gl_one_minus_dst_color"))
			blend = GL_ONE_MINUS_DST_COLOR;
			
		else if(X_strcaseequal(parms[i].c_str(), "gl_src_alpha_saturate"))
			blend = GL_SRC_ALPHA_SATURATE;
				
		else
			ri.Com_Printf("R_Blend_stc: unknown blend command: '%s'\n", parms[i].c_str());
	}
	
	r_current_stage->flags |= SHADER_STAGE_BLEND;
}

static void	R_BlendAdd_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_BLEND;
	
	r_current_stage->blend_src = GL_ONE;
	r_current_stage->blend_dst = GL_ONE;
}

static void	R_BlendBlend_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_BLEND;
	
	r_current_stage->blend_src = GL_SRC_ALPHA;
	r_current_stage->blend_dst = GL_ONE_MINUS_SRC_ALPHA;
}

static void	R_BlendFilter_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_BLEND;
	
	r_current_stage->blend_src = GL_DST_COLOR;
	r_current_stage->blend_dst = GL_ZERO;
}

static void	R_Linear_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_NOMIPMAPS;
}

static void	R_NoPicmip_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_NOPICMIP;
}

static void	R_ZeroClamp_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_ZEROCLAMP;
}

static void	R_Clamp_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_CLAMP;
}

static void	R_MaskAlpha_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_MASKALPHA;
}

static void	R_MaskColor_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_MASKCOLOR;
}

static void	R_MaskDepth_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_MASKDEPTH;
}

static void	R_ForceHighQuality_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_FORCEHIGHQUALITY;
}

static void	R_VideoMap_stc(char const* begin, char const *end)
{
	r_current_stage->image_name = std::string(begin, end);
	r_current_stage->flags |= SHADER_STAGE_VIDEOMAP;
}

static void	R_Map_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	r_shader_map_stc_grammar_t	grammar;
	
	boost::spirit::parse_info<> info = boost::spirit::parse
	(
		exp.c_str(),
		grammar,
		boost::spirit::space_p ||
		boost::spirit::comment_p("/*", "*/") ||
		boost::spirit::comment_p("//")
	);
	
	if(!info.full)
	{
		ri.Com_Printf("R_Map_stc: parsing failed\n");
	}
}

static void	R_Colored_stc(char const* begin, char const *end)
{
	r_current_stage->rgb_gen = SHADER_RGB_GEN_ENTITY;
	
	r_current_stage->alpha_gen = SHADER_ALPHA_GEN_ENTITY;
}

static void	R_RGBA_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> rgba;
	
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), rgba))
	{
		r_current_stage->rgb_gen = SHADER_RGB_GEN_CUSTOM;
	
		r_current_stage->red	= rgba;
		r_current_stage->green	= rgba;
		r_current_stage->blue	= rgba;
		
		r_current_stage->alpha_gen = SHADER_ALPHA_GEN_CUSTOM;
		r_current_stage->alpha	= rgba;
	}
	else
	{
		ri.Com_Printf("R_RGBA_stc: parsing failed\n");
	}
}

static void	R_RGB_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> rgb;
	
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), rgb))
	{
		r_current_stage->rgb_gen = SHADER_RGB_GEN_CUSTOM;
	
		r_current_stage->red	= rgb;
		r_current_stage->green	= rgb;
		r_current_stage->blue	= rgb;
	}
	else
	{
		ri.Com_Printf("R_RGB_stc: parsing failed\n");
	}
}

static void	R_Red_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> red;
	
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), red))
	{
		r_current_stage->rgb_gen = SHADER_RGB_GEN_CUSTOM;
	
		r_current_stage->red	= red;
	}
	else
	{
		ri.Com_Printf("R_Red_stc: parsing failed\n");
	}
}

static void	R_Green_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> green;
	
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), green))
	{
		r_current_stage->rgb_gen = SHADER_RGB_GEN_CUSTOM;
	
		r_current_stage->green	= green;
	}
	else
	{
		ri.Com_Printf("R_Green_stc: parsing failed\n");
	}
}

static void	R_Blue_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> blue;
	
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), blue))
	{
		r_current_stage->rgb_gen = SHADER_RGB_GEN_CUSTOM;
	
		r_current_stage->blue	= blue;
	}
	else
	{
		ri.Com_Printf("R_Blue_stc: parsing failed\n");
	}
}

static void	R_Alpha_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> alpha;
	
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), alpha))
	{
		r_current_stage->alpha_gen = SHADER_ALPHA_GEN_CUSTOM;
	
		r_current_stage->alpha	= alpha;
	}
	else
	{
		ri.Com_Printf("R_Alpha_stc: parsing failed\n");
	}
}

static void	R_AlphaTest_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> alpharef;
	
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), alpharef))
	{
		r_current_stage->flags |= SHADER_STAGE_ALPHATEST;
	
		r_current_stage->alpha_ref	= alpharef;
	}
	else
	{
		ri.Com_Printf("R_AlphaTest_stc: parsing failed\n");
	}
}

static void	R_Scale3D_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);
		
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> x, y, z;
	
	std::string exp0, exp1, exp2;

	if(!R_SplitExpression(exp, exp0, exp1, exp2, false))
	{
		ri.Com_Printf("R_Scale3D_stc: splitting expression failed\n");
		return;
	}
	
	if(
		R_ParseExpressionToAST(exp0.begin(), exp0.end(), x) &&
		R_ParseExpressionToAST(exp1.begin(), exp1.end(), y) &&
		R_ParseExpressionToAST(exp1.begin(), exp1.end(), z)
	)
	{
		r_tcmod_t tcmod;
		
		tcmod.type = SHADER_TCMOD_SCALE3D;
	
		tcmod.x = x;
		tcmod.y = y;
		tcmod.z = z;
		
		r_current_stage->tcmod_cmds.push_back(tcmod);
	}
	else
	{
		ri.Com_Printf("R_Scale3D_stc: parsing failed\n");
	}
}

static void	R_Scale_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);
		
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> x, y;
	
	std::string exp0, exp1;

	if(!R_SplitExpression(exp, exp0, exp1, false))
	{
		ri.Com_Printf("R_Scale_stc: splitting expression failed\n");
		return;
	}
	
	if(R_ParseExpressionToAST(exp0.begin(), exp0.end(), x) && R_ParseExpressionToAST(exp1.begin(), exp1.end(), y))
	{
		r_tcmod_t tcmod;
		
		tcmod.type = SHADER_TCMOD_SCALE;
	
		tcmod.x = x;
		tcmod.y = y;
		
		r_current_stage->tcmod_cmds.push_back(tcmod);
	}
	else
	{
		ri.Com_Printf("R_Scale_stc: parsing failed\n");
	}
}

static void	R_CenterScale_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);
		
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> x, y;
	
	std::string exp0, exp1;

	if(!R_SplitExpression(exp, exp0, exp1, false))
	{
		ri.Com_Printf("R_CenterScale_stc: splitting expression failed\n");
		return;
	}
	
	if(R_ParseExpressionToAST(exp0.begin(), exp0.end(), x) && R_ParseExpressionToAST(exp1.begin(), exp1.end(), y))
	{
		r_tcmod_t tcmod;
		
		tcmod.type = SHADER_TCMOD_CENTERSCALE;
	
		tcmod.x = x;
		tcmod.y = y;
		
		r_current_stage->tcmod_cmds.push_back(tcmod);
	}
	else
	{
		ri.Com_Printf("R_CenterScale_stc: parsing failed\n");
	}
}

static void	R_Scroll_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> x, y;
	
	std::string exp0, exp1;

	if(!R_SplitExpression(exp, exp0, exp1, false))
	{
		ri.Com_Printf("R_Scroll_stc: splitting expression failed\n");
		return;
	}

	if(R_ParseExpressionToAST(exp0.begin(), exp0.end(), x) && R_ParseExpressionToAST(exp1.begin(), exp1.end(), y))
	{
		r_tcmod_t tcmod;
		
		tcmod.type = SHADER_TCMOD_SCROLL;
	
		tcmod.x = x;
		tcmod.y = y;
		
		r_current_stage->tcmod_cmds.push_back(tcmod);
	}
	else
	{
		ri.Com_Printf("R_Scroll_stc: parsing failed\n");
	}
}

static void	R_Rotate_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);
	
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> x;
	
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), x))
	{
		r_tcmod_t tcmod;
		
		tcmod.type = SHADER_TCMOD_ROTATE;
	
		tcmod.x = x;
		
		r_current_stage->tcmod_cmds.push_back(tcmod);
	}
	else
	{
		ri.Com_Printf("R_Rotate_stc: parsing failed\n");
	}
}

static void	R_Translate_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);
		
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> x, y;
	
	std::string exp0, exp1;

	if(!R_SplitExpression(exp, exp0, exp1, false))
	{
		ri.Com_Printf("R_Translate_stc: splitting expression failed\n");
		return;
	}
	
	if(R_ParseExpressionToAST(exp0.begin(), exp0.end(), x) && R_ParseExpressionToAST(exp1.begin(), exp1.end(), y))
	{
		r_tcmod_t tcmod;
		
		tcmod.type = SHADER_TCMOD_TRANSLATE;
	
		tcmod.x = x;
		tcmod.y = y;
		
		r_current_stage->tcmod_cmds.push_back(tcmod);
	}
	else
	{
		ri.Com_Printf("R_Translate_stc: parsing failed\n");
	}
}

static void	R_BumpScale_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> bump_scale;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), bump_scale))
		ri.Com_Printf("R_BumpScale_stc: parsing failed\n");
	
	r_current_stage->bump_scale = bump_scale;
}

static void	R_HeightScale_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> height_scale;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), height_scale))
		ri.Com_Printf("R_HeightScale_stc: parsing failed\n");
	
	r_current_stage->height_scale = height_scale;
}

static void	R_HeightBias_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> height_bias;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), height_bias))
		ri.Com_Printf("R_HeightBias_stc: parsing failed\n");
	
	r_current_stage->height_bias = height_bias;
}

static void	R_SpecularExponent_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> specular_exponent;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), specular_exponent))
		ri.Com_Printf("R_SpecularExponent_stc: parsing failed\n");
	
	r_current_stage->specular_exponent = specular_exponent;
}

static void	R_RefractionIndex_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> refraction_index;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), refraction_index))
		ri.Com_Printf("R_RefractionIndex_stc: parsing failed\n");
	
	r_current_stage->refraction_index = refraction_index;
}

static void	R_FresnelPower_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> fresnel_power;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), fresnel_power))
		ri.Com_Printf("R_FresnelPower_stc: parsing failed\n");
	
	r_current_stage->fresnel_power = fresnel_power;
}

static void	R_FresnelScale_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> fresnel_scale;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), fresnel_scale))
		ri.Com_Printf("R_FresnelScale_stc: parsing failed\n");
	
	r_current_stage->fresnel_scale = fresnel_scale;
}

static void	R_FresnelBias_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> fresnel_bias;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), fresnel_bias))
		ri.Com_Printf("R_FresnelBias_stc: parsing failed\n");
	
	r_current_stage->fresnel_bias = fresnel_bias;
}

static void	R_EtaDelta_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> eta_delta;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), eta_delta))
		ri.Com_Printf("R_EtaDelta_stc: parsing failed\n");
	
	r_current_stage->eta_delta = eta_delta;
}

static void	R_Eta_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> eta;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), eta))
		ri.Com_Printf("R_Eta_stc: parsing failed\n");
	
	r_current_stage->eta = eta;
}

static void	R_StageColorMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_COLORMAP;
}

static void	R_StageDiffuseMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DIFFUSEMAP;
}

static void	R_StageBumpMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_BUMPMAP;
}

static void	R_StageSpecularMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_SPECULARMAP;
}

static void	R_StageLightMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_LIGHTMAP;
}

static void	R_StageDeluxeMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DELUXEMAP;
}

static void	R_StageReflectionMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_REFLECTIONMAP;
}

static void	R_StageRefractionMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_REFRACTIONMAP;
}

static void	R_StageDispersionMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DISPERSIONMAP;
}

static void	R_StageLiquidMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_LIQUIDMAP;
}

static void	R_StageAttenuationMapXY_stc(char const* begin, char const *end)
{
	r_current_stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY;
}

static void	R_StageAttenuationMapZ_stc(char const* begin, char const *end)
{
	r_current_stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_Z;
}

static void	R_StageAttenuationMapXYZ_stc(char const* begin, char const *end)
{
	r_current_stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XYZ;
}

static void	R_StageAttenuationMapCube_stc(char const* begin, char const *end)
{
	r_current_stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_CUBE;
}




static void	R_Unknown_sc(char const* begin, char const* end)
{
	ri.Com_Printf("unknown shader command: '%s'\n", std::string(begin, end).c_str());
}

static void	R_Unknown_stc(char const* begin, char const* end)
{
	ri.Com_Printf("unknown shader stage command: '%s'\n", std::string(begin, end).c_str());
}

static void	R_ClearCurrentStage(char const begin)//, char const* end)
{
	r_current_stage = new r_shader_stage_c();
}

static void	R_PushBackCurrentStage(char const begin)//, char const* end)
{
	if(r_current_shader->getType() != SHADER_LIGHT)
	{
		switch(r_current_stage->type)
		{
			case SHADER_MATERIAL_STAGE_TYPE_DIFFUSEMAP:
				r_current_shader->stage_diffusemap = r_current_stage;
				break;
		
			case SHADER_MATERIAL_STAGE_TYPE_BUMPMAP:
				r_current_shader->stage_bumpmap = r_current_stage;
				break;
			
			case SHADER_MATERIAL_STAGE_TYPE_SPECULARMAP:
				r_current_shader->stage_specularmap = r_current_stage;
				break;
				
			case SHADER_MATERIAL_STAGE_TYPE_LIGHTMAP:
				r_current_shader->stage_lightmap = r_current_stage;
				break;
				
			case SHADER_MATERIAL_STAGE_TYPE_DELUXEMAP:
				r_current_shader->stage_deluxemap = r_current_stage;
				break;
		
			default:
				break;
		}
	}
	else
	{
		switch(r_current_stage->type_light)
		{
			case SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_Z:
				r_current_shader->stage_attenuationmap_z = r_current_stage;
				break;
				
			case SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_CUBE:
				r_current_shader->stage_attenuationmap_cube = r_current_stage;
		
			default:
				break;
		}
	}
	
	r_current_shader->stages.push_back(r_current_stage);
}


struct r_shader_grammar_t : public boost::spirit::grammar<r_shader_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(r_shader_grammar_t const& self)
		{
			// start grammar definition
			c_comment
				=	boost::spirit::comment_p("/*", "*/")
				;
			
			cxx_comment
				=	boost::spirit::comment_p("//")
				;
			
			restofline
				=	boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::eol_p | boost::spirit::ch_p('}'))]] - (c_comment | cxx_comment)
				;
			
				// shader commands	
			unknown_sc
				=	restofline[&R_Unknown_sc]
				;
				
			colormap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("colormap")] >> restofline[&R_ColorMap_sc]
				;
			
			diffusemap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("diffusemap")] >> restofline[&R_DiffuseMap_sc]
				;
			
			bumpmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("bumpmap")] >> restofline[&R_BumpMap_sc]
				;
			
			specularmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("specularmap")] >> restofline[&R_SpecularMap_sc]
				;
			
			lightmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("lightmap")] >> restofline[&R_LightMap_sc]
				;
				
			deluxemap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("deluxemap")] >> restofline[&R_DeluxeMap_sc]
				;
				
			reflectionmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("reflectionmap")] >> restofline[&R_ReflectionMap_sc]
				;
			
			refractionmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("refractionmap")] >> restofline[&R_RefractionMap_sc]
				;
				
			dispersionmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("dispersionmap")] >> restofline[&R_DispersionMap_sc]
				;
			
			liquidmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("liquidmap")] >> restofline[&R_LiquidMap_sc]
				;
				
			nodraw_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("nodraw")][&R_NoDraw_sc]
				;
				
			nofragment_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("nofragment")]
				;
				
			noimpact_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("noimpact")]
				;
				
			nonsolid_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("nonsolid")]
				;
			
			noshadows_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("noshadows")][&R_NoShadows_sc]
				;
				
			noselfshadow_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("noselfshadow")][&R_NoSelfShadow_sc]
				;
				
			ricochet_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("ricochet")]
				;
				
			sort_farthest_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("sort")] >> boost::spirit::nocase_d[boost::spirit::str_p("farthest")][&R_SortFarthest_sc]
				;
				
			sort_far_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("sort")] >> boost::spirit::nocase_d[boost::spirit::str_p("far")][&R_SortFar_sc]
				;
				
			sort_close_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("sort")] >> boost::spirit::nocase_d[boost::spirit::str_p("close")][&R_SortClose_sc]
				;
				
			sort_decal_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("sort")] >> boost::spirit::nocase_d[boost::spirit::str_p("decal")][&R_SortDecal_sc]
				;
				
			sort_nearest_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("sort")] >> boost::spirit::nocase_d[boost::spirit::str_p("nearest")][&R_SortNearest_sc]
				;
				
			sort_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("sort")] >> boost::spirit::int_p[&R_Sort_sc]
				;
				
			twosided_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("twosided")][&R_TwoSided_sc]
				;
			
			translucent_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("translucent")][&R_Translucent_sc]
				;
				
			polygonoffset_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("polygonoffset")][&R_PolygonOffset_sc]
				;
				
			discrete_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("discrete")][&R_Discrete_sc]
				;
				
			forceopaque_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("forceopaque")][&R_ForceOpaque_sc]
				;
				
			lightfalloffimage_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("lightfalloffimage")] >> restofline[&R_LightFalloffImage_sc]
				;
				
			ambientlight_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("ambientlight")][&R_AmbientLight_sc]
				;
				
			foglight_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("foglight")][&R_FogLight_sc]
				;
			
			decal_macro_sc	// Tr3B - get rid of macros
				=	boost::spirit::nocase_d[boost::spirit::str_p("decal_macro")][&R_DecalMacro_sc]
				;
				
			deform_flare_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("deform") >> boost::spirit::str_p("flare")] >> restofline[&R_DeformFlare_sc]
				;
				
			surfaceparm_areaportal_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("surfaceparm") >> boost::spirit::str_p("areaportal")][&R_SurfaceparmAreaPortal_sc]
				;
					
			surfaceparm_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("surfaceparm")] >> restofline
				;
				
			qer_editorimage_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("qer_editorimage")] >> restofline
				;
			
			qer_trans_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("qer_trans")] >> restofline
				;
				
			description_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("description")] >> restofline
				;
				
			renderbump_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("renderbump")] >> restofline
				;
			
			shader_command
				=	colormap_sc			|
					diffusemap_sc			|
					bumpmap_sc			|
					specularmap_sc			|
					lightmap_sc			|
					deluxemap_sc			|
					reflectionmap_sc		|
					refractionmap_sc		|
					dispersionmap_sc		|
					liquidmap_sc			|
					nodraw_sc			|
					nofragment_sc			|
					noimpact_sc			|
					nonsolid_sc			|
					noshadows_sc			|
					noselfshadow_sc			|
					ricochet_sc			|
					twosided_sc			|
					sort_farthest_sc		|
					sort_far_sc			|
					sort_close_sc			|
					sort_decal_sc			|
					sort_nearest_sc			|
					sort_sc				|
					translucent_sc			|
					polygonoffset_sc		|
					discrete_sc			|
					forceopaque_sc			|
					lightfalloffimage_sc		|
					ambientlight_sc			|
					foglight_sc			|
					decal_macro_sc			|
					deform_flare_sc			|
					surfaceparm_areaportal_sc	|
					surfaceparm_sc			|
					qer_editorimage_sc		|
					qer_trans_sc			|
					description_sc			|
					renderbump_sc			|
					unknown_sc
				;
				
				// shader stage commands
			unknown_stc
				=	restofline[&R_Unknown_stc]
				;
				
			blend_add_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("blend") >> boost::spirit::str_p("add")][&R_BlendAdd_stc]
				;
				
			blend_blend_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("blend") >> boost::spirit::str_p("blend")][&R_BlendBlend_stc]
				;
			
			blend_filter_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("blend") >> boost::spirit::str_p("filter")][&R_BlendFilter_stc]
				;
			
			blend_diffusemap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("blend") >> boost::spirit::str_p("diffusemap")][&R_StageDiffuseMap_stc]
				;
				
			blend_bumpmap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("blend") >> boost::spirit::str_p("bumpmap")][&R_StageBumpMap_stc]
				;
			
			blend_specularmap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("blend") >> boost::spirit::str_p("specularmap")][&R_StageSpecularMap_stc]
				;
			
			blend_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("blend")] >> restofline[&R_Blend_stc]
				;
			
			linear_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("linear")][&R_Linear_stc]
				;
			
			nopicmip_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("nopicmip")][&R_NoPicmip_stc]
				;
			
			zeroclamp_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("zeroclamp")][&R_ZeroClamp_stc]
				;
				
			clamp_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("clamp")][&R_Clamp_stc]
				;
				
			maskalpha_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("maskalpha")][&R_MaskAlpha_stc]
				;
				
			maskcolor_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("maskcolor")][&R_MaskColor_stc]
				;
				
			maskdepth_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("maskdepth")][&R_MaskDepth_stc]
				;
				
			forcehighquality_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("forcehighquality")][&R_ForceHighQuality_stc]
				;
				
			cubemap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("cubemap")] >> restofline[&R_Map_stc]
				;
				
			videomap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("videomap")] >> restofline[&R_VideoMap_stc]
				;
			
			map_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("map")] >> restofline[&R_Map_stc]
				;
			
			colored_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("colored")][&R_Colored_stc]
				;
			
			rgba_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("rgba")] >> restofline[&R_RGBA_stc]
				;
			
			rgb_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("rgb")] >> restofline[&R_RGB_stc]
				;
				
			red_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("red")] >> restofline[&R_Red_stc]
				;
			
			green_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("green")] >> restofline[&R_Green_stc]
				;
			
			blue_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("blue")] >> restofline[&R_Blue_stc]
				;
			
			alphatest_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("alphatest")] >> restofline[&R_AlphaTest_stc]
				;
				
			alpha_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("alpha")] >> restofline[&R_Alpha_stc]
				;
				
			scale3d_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("scale3d")] >> restofline[&R_Scale3D_stc]
				;
				
			scale_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("scale")] >> restofline[&R_Scale_stc]
				;
			
			centerscale_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("centerscale")] >> restofline[&R_CenterScale_stc]
				;
				
			scroll_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("scroll")] >> restofline[&R_Scroll_stc]
				;
				
			rotate_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("rotate")] >> restofline[&R_Rotate_stc]
				;
			
			translate_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("translate")] >> restofline[&R_Translate_stc]
				;
				
			bumpscale_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("bumpscale")] >> restofline[&R_BumpScale_stc]
				;
				
			heightscale_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("heightscale")] >> restofline[&R_HeightScale_stc]
				;
				
			heightbias_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("heightbias")] >> restofline[&R_HeightBias_stc]
				;

			specularexponent_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("specularexponent")] >> restofline[&R_SpecularExponent_stc]
				;
				
			refractionindex_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("refractionindex")] >> restofline[&R_RefractionIndex_stc]
				;
				
			fresnelpower_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("fresnelpower")] >> restofline[&R_FresnelPower_stc]
				;
				
			fresnelscale_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("fresnelscale")] >> restofline[&R_FresnelScale_stc]
				;
				
			fresnelbias_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("fresnelbias")] >> restofline[&R_FresnelBias_stc]
				;
				
			etadelta_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("etadelta")] >> restofline[&R_EtaDelta_stc]
				;
				
			eta_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("eta")] >> restofline[&R_Eta_stc]
				;
				
			texgen_reflect_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("texgen") >> boost::spirit::str_p("reflect")][&R_StageReflectionMap_stc]
				;
				
			stage_colormap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("colormap")][&R_StageColorMap_stc]
				;
				
			stage_diffusemap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("diffusemap")][&R_StageDiffuseMap_stc]
				;
				
			stage_bumpmap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("bumpmap")][&R_StageBumpMap_stc]
				;
			
			stage_specularmap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("specularmap")][&R_StageSpecularMap_stc]
				;
			
			stage_lightmap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("lightmap")][&R_StageLightMap_stc]
				;
				
			stage_deluxemap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("deluxemap")][&R_StageDeluxeMap_stc]
				;
				
			stage_reflectionmap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("reflectionmap")][&R_StageReflectionMap_stc]
				;
				
			stage_refractionmap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("refractionmap")][&R_StageRefractionMap_stc]
				;
			
			stage_dispersionmap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("dispersionmap")][&R_StageDispersionMap_stc]
				;
			
			stage_liquidmap_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("liquidmap")][&R_StageLiquidMap_stc]
				;
				
			stage_attenuationmap_xy_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("attenuationmap_xy")][&R_StageAttenuationMapXY_stc]
				;
			
			stage_attenuationmap_z_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("attenuationmap_z")][&R_StageAttenuationMapZ_stc]
				;
				
			stage_attenuationmap_xyz_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("attenuationmap_xyz")][&R_StageAttenuationMapXYZ_stc]
				;
				
			stage_attenuationmap_cube_stc
				=	boost::spirit::nocase_d[boost::spirit::str_p("stage") >> boost::spirit::str_p("attenuationmap_cube")][&R_StageAttenuationMapCube_stc]
				;
					
			shader_stage_command
				=	blend_add_stc			|
					blend_blend_stc			|
					blend_filter_stc		|
					blend_diffusemap_stc		|
					blend_bumpmap_stc		|
					blend_specularmap_stc		|
					blend_stc			|
					linear_stc			|
					nopicmip_stc			|
					zeroclamp_stc			|
					clamp_stc			|
					maskalpha_stc			|
					maskcolor_stc			|
					maskdepth_stc			|
					forcehighquality_stc		|
					cubemap_stc			|
					videomap_stc			|
					map_stc				|
					colored_stc			|
					rgba_stc			|
					rgb_stc				|
					red_stc				|
					green_stc			|
					blue_stc			|
					alphatest_stc			|
					alpha_stc			|
					scale3d_stc			|
					scale_stc			|
					centerscale_stc			|
					scroll_stc			|
					rotate_stc			|
					translate_stc			|
					bumpscale_stc			|
					heightscale_stc			|
					heightbias_stc			|
					specularexponent_stc		|
					refractionindex_stc		|
					fresnelpower_stc		|
					fresnelscale_stc		|
					fresnelbias_stc			|
					etadelta_stc			|
					eta_stc				|
					texgen_reflect_stc		|
					stage_colormap_stc		|
					stage_diffusemap_stc		|
					stage_bumpmap_stc		|
					stage_specularmap_stc		|
					stage_lightmap_stc		|
					stage_deluxemap_stc		|
					stage_reflectionmap_stc		|
					stage_refractionmap_stc		|
					stage_dispersionmap_stc		|
					stage_liquidmap_stc		|
					stage_attenuationmap_xy_stc	|
					stage_attenuationmap_z_stc	|
					stage_attenuationmap_xyz_stc	|
					stage_attenuationmap_cube_stc	|
					unknown_stc
				;
				
			shader_stage
				=	//boost::spirit::refactor_unary_d[*boost::spirit::anychar_p - boost::spirit::ch_p('{')] >>
					boost::spirit::ch_p('{')[&R_ClearCurrentStage] >> 
					+shader_stage_command >> 
					boost::spirit::ch_p('}')[&R_PushBackCurrentStage]
				;
						
			expression
				=	//boost::spirit::refactor_unary_d[*boost::spirit::anychar_p - boost::spirit::ch_p('{')] >>
					boost::spirit::ch_p('{') >> 
					+(shader_stage | shader_command) >> 
					boost::spirit::ch_p('}')
				;
				
			// end grammar definiton
		}
		
		
		boost::spirit::rule<ScannerT>	c_comment,
						cxx_comment,
						restofline,
							shader_command,
								colormap_sc,
								diffusemap_sc,
								bumpmap_sc,
								specularmap_sc,
								lightmap_sc,
								deluxemap_sc,
								reflectionmap_sc,
								refractionmap_sc,
								dispersionmap_sc,
								liquidmap_sc,
								nodraw_sc,
								nofragment_sc,
								noimpact_sc,
								nonsolid_sc,
								noshadows_sc,
								noselfshadow_sc,
								ricochet_sc,
								twosided_sc,
								sort_farthest_sc,
								sort_far_sc,
								sort_close_sc,
								sort_decal_sc,
								sort_nearest_sc,
								sort_sc,
								translucent_sc,
								polygonoffset_sc,
								discrete_sc,
								forceopaque_sc,
								lightfalloffimage_sc,
								ambientlight_sc,
								foglight_sc,
								decal_macro_sc,
								deform_flare_sc,
								surfaceparm_areaportal_sc,
								surfaceparm_sc,
								qer_editorimage_sc,
								qer_trans_sc,
								description_sc,
								renderbump_sc,
								unknown_sc,
							
							shader_stage,
								shader_stage_command,
									blend_add_stc,
									blend_blend_stc,
									blend_filter_stc,
									blend_diffusemap_stc,
									blend_bumpmap_stc,
									blend_specularmap_stc,
									blend_stc,
									linear_stc,
									nopicmip_stc,
									zeroclamp_stc,
									clamp_stc,
									maskalpha_stc,
									maskcolor_stc,
									maskdepth_stc,
									forcehighquality_stc,
									cubemap_stc,
									videomap_stc,
									map_stc,
									colored_stc,
									rgba_stc,
									rgb_stc,
									red_stc,
									green_stc,
									blue_stc,
									alphatest_stc,
									alpha_stc,
									scale3d_stc,
									scale_stc,
									centerscale_stc,
									scroll_stc,
									rotate_stc,
									translate_stc,
									bumpscale_stc,
									specularexponent_stc,
									heightscale_stc,
									heightbias_stc,
									refractionindex_stc,
									fresnelpower_stc,
									fresnelscale_stc,
									fresnelbias_stc,
									etadelta_stc,
									eta_stc,
									texgen_reflect_stc,
									stage_colormap_stc,
									stage_diffusemap_stc,
									stage_bumpmap_stc,
									stage_specularmap_stc,
									stage_lightmap_stc,
									stage_deluxemap_stc,
									stage_reflectionmap_stc,
									stage_refractionmap_stc,
									stage_dispersionmap_stc,
									stage_liquidmap_stc,
									stage_attenuationmap_xy_stc,
									stage_attenuationmap_z_stc,
									stage_attenuationmap_xyz_stc,
									stage_attenuationmap_cube_stc,
									unknown_stc,
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};

static std::string	r_sp_filename;
static char*		r_sp_data = NULL;

static std::string	r_sp_shader_name;
static uint_t 		r_sp_shader_offset_begin;
static uint_t		r_sp_shader_offset_end;

static std::string		r_sp_table_name;
static r_table_t		r_sp_table;

static r_shader_cache_c*	R_GetShaderCache(const std::string &cache_name, const std::string &cache_path, unsigned int offset_begin, unsigned int offset_end);

static void	R_ShaderName(const char* begin, const char* end)
{
	r_sp_shader_name = std::string(begin, end);
	r_sp_shader_offset_begin = (end+1) - r_sp_data;
	
	//ri.Com_Printf("shader name: '%s'\n", r_sp_shader_name.c_str());
}

static void	R_PrecacheShader(const char *begin, const char* end)
{
	r_sp_shader_offset_end = end - r_sp_data;
	
	R_GetShaderCache(r_sp_shader_name, r_sp_filename, r_sp_shader_offset_begin, r_sp_shader_offset_end);
}

static void	R_TableName(const char* begin, const char* end)
{
	r_sp_table_name = X_strlwr(std::string(begin, end));
}

static void	R_ClampTable(const char* begin, const char* end)
{
	r_sp_table.flags |= SHADER_TABLE_CLAMP;
}

static void	R_SnapTable(const char* begin, const char* end)
{
	r_sp_table.flags |= SHADER_TABLE_SNAP;
}

static void	R_ClearTable(const char begin)
{
	r_sp_table.flags = 0;
	r_sp_table.values.clear();
}

static void	R_CreateTable(const char* begin, const char* end)
{
	ri.Com_Printf("   creating table '%s' ...\n", r_sp_table_name.c_str());
	
	//for(std::vector<float>::iterator ir = r_sp_table.values.begin(); ir != r_sp_table.values.end(); ++ir)
	//	ri.Com_Printf("%f ");
	//ri.Com_Printf("\n");
	
	r_shader_table_symbols_p.add(r_sp_table_name.c_str(), r_tables.size());
	r_tables.push_back(r_sp_table);
}

struct r_shader_precache_grammar_t : public boost::spirit::grammar<r_shader_precache_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(r_shader_precache_grammar_t const& self)
		{
			// start grammar definition
			skip_tobracket
				=	boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::ch_p('{') | boost::spirit::ch_p('}'))]
				;
				
			skip_block
				=	boost::spirit::ch_p('{') >>
					skip_tobracket >>
					boost::spirit::ch_p('}')
				;
				
			table
				=	boost::spirit::nocase_d[boost::spirit::str_p("table")] >>
					boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::space_p | boost::spirit::ch_p('{'))]][&R_TableName] >>
					boost::spirit::ch_p('{')[&R_ClearTable] >>
					*(table_clamp | table_snap) >>
					boost::spirit::ch_p('{') >>
					boost::spirit::list_p(boost::spirit::lexeme_d[boost::spirit::real_p[boost::spirit::append(r_sp_table.values)]], ',') >>
					boost::spirit::ch_p('}') >>
					boost::spirit::ch_p('}')
				;
				
			table_clamp
				=	boost::spirit::str_p("clamp")[&R_ClampTable]
				;
				
			table_snap
				=	boost::spirit::str_p("snap")[&R_SnapTable]
				;
				
			shader
				=	boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::space_p | boost::spirit::ch_p('{'))]][&R_ShaderName] >>
					boost::spirit::ch_p('{') >> 
					*(skip_block | skip_tobracket) >>
					boost::spirit::ch_p('}')
				;
				
			expression
				=	*(table[&R_CreateTable] | shader[&R_PrecacheShader])
				;
				
			// end grammar definiton
		}
		
		boost::spirit::rule<ScannerT>	skip_tobracket,
						skip_block,
						table,
							table_clamp,
							table_snap,
						shader,
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};

/*

================================================================================
			MISC SHADER FUNCTIONS
================================================================================
*/


static r_shader_cache_c*	R_FindShaderCache(const std::string &name)
{
	//ri.Com_DPrintf ("R_FindShaderCache: '%s'\n", cache_name);
	
	std::string name_short = Com_StripExtension(name);
		
	for(std::vector<r_shader_cache_c*>::const_iterator ir = r_shaders_cache.begin(); ir != r_shaders_cache.end(); ++ir)
	{
		if(X_strcaseequal(name_short.c_str(), (*ir)->getName()))
			return (*ir);
	}

	return NULL;
}

static r_shader_cache_c*	R_GetShaderCache(const std::string &cache_name, const std::string &cache_path, unsigned int offset_begin, unsigned int offset_end)
{
	r_shader_cache_c	*cache;
	
	//ri.Com_DPrintf("R_GetShaderCache: '%s' '%s' '%i' '%i'\n", cache_name.c_str(), cache_path.c_str(), offset_begin, offset_end);
	
	cache = R_FindShaderCache(cache_name);
	
	if(cache)
		return cache;

	cache = new r_shader_cache_c(cache_name, cache_path, offset_begin, offset_end);

	// link the variable in
	r_shaders_cache.push_back(cache);	

	return cache;
}

static void	R_PrecacheShaderFile(const std::string &filename)
{
	char*	data = NULL;
	int	len;
	
	len = ri.VFS_FLoad(filename, (void**)&data);
	if(!data)
	{
		ri.Com_Printf("couldn't load %s\n", filename.c_str());
		return;
	}
	
	ri.Com_Printf("precaching '%s' ...\n", filename.c_str());
	
	r_sp_filename = filename;
	r_sp_data = data;
	r_shader_precache_grammar_t	grammar;
	
	boost::spirit::parse_info<> info = boost::spirit::parse
	(
		data,
		grammar,
		boost::spirit::space_p ||
		boost::spirit::comment_p("/*", "*/") ||
		boost::spirit::comment_p("//")
	);
	
	if(!info.full)
	{
		ri.Com_Error(ERR_DROP, "R_PrecacheShaderFile: failed parsing '%s'\n", filename.c_str());
	}
	
	ri.VFS_FFree(data);
}


static void	R_FindMaterialShaderStageImage(r_shader_c *shader, r_shader_stage_c *stage)
{
	int			imageflags;
	r_image_c*		image = NULL;

	switch(stage->type)
	{
		case SHADER_MATERIAL_STAGE_TYPE_COLORMAP:
		{
			if(stage->image_name.length())
			{		
				imageflags = IMAGE_NONE;
	
				R_SetShaderStageImageFlags(stage, imageflags);
	
				if(stage->flags & SHADER_STAGE_VIDEOMAP)
				{
					image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_VIDEOMAP);
		
					if(!image)
					{
						ri.Com_Printf("R_FindMaterialShaderStageImage: shader '%s' has no videomap '%s'\n", shader->getName(), stage->image_name.c_str());
						image = r_img_default;
					}
		
					stage->image = image;
				}
				else if(stage->make_alpha)
				{
					image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_ALPHAMAP);
		
					if(!image)
					{
						ri.Com_Printf("R_FindMaterialShaderStageImage: shader '%s' has no alphamap '%s'\n", shader->getName(), stage->image_name.c_str());
						image = r_img_default;
					}
				}
				else
				{
					image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_COLORMAP);
		
					if(!image)
					{
						ri.Com_Printf("R_FindMaterialShaderStageImage: shader '%s' has no colormap '%s'\n", shader->getName(), stage->image_name.c_str());
						image = r_img_default;
					}
				}
				
				stage->image = image;
			}
			else
			{
				stage->image = r_img_default;
			}
			
			break;
		}
	
		case SHADER_MATERIAL_STAGE_TYPE_DIFFUSEMAP:
		{
			if(stage->image_name.length())
			{				
				imageflags = IMAGE_NONE;
	
				R_SetShaderStageImageFlags(stage, imageflags);
		
				image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_COLORMAP);
		
				if(!image)
				{
					ri.Com_Printf("R_FindMaterialShaderStageImage: shader '%s' has no diffusemap '%s'\n", shader->getName(), stage->image_name.c_str());
					image = r_img_default;
				}
		
				stage->image = image;
			}
			else
			{
				stage->image = r_img_default;
			}
			
			break;
		}
		
		case SHADER_MATERIAL_STAGE_TYPE_BUMPMAP:
		{
			if(stage->image_name.length())
			{
				imageflags = IMAGE_NORMALMAP;
			
				R_SetShaderStageImageFlags(stage, imageflags);
	
				image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_COLORMAP);
		
				if(!image)
				{
					ri.Com_Printf("R_FindMaterialShaderStageImage: shader '%s' has no normalmap '%s'\n", shader->getName(), stage->image_name.c_str());
					image = r_img_flat;
				}
			
				stage->image = image;
			}
			else
			{
				stage->image = r_img_flat;
			}
			
			break;
		}
		
		case SHADER_MATERIAL_STAGE_TYPE_SPECULARMAP:
		{
			if(stage->image_name.length())
			{		
				imageflags = IMAGE_NONE;
				
				R_SetShaderStageImageFlags(stage, imageflags);
	
				image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_COLORMAP);
		
				if(!image)
				{
					ri.Com_Printf("R_FindMaterialShaderStageImage: shader '%s' has no specularmap '%s'\n", shader->getName(), stage->image_name.c_str());
					image = r_img_white;
				}
			
				stage->image = image;
			}
			else
			{
				stage->image = r_img_white;
			}
			
			break;
		}
		
		case SHADER_MATERIAL_STAGE_TYPE_LIGHTMAP:
		case SHADER_MATERIAL_STAGE_TYPE_DELUXEMAP:
		{
			//DO NOTHING
			break;
		}
		
		case SHADER_MATERIAL_STAGE_TYPE_REFLECTIONMAP:
		{
			if(stage->image_name.length())
			{
				imageflags = IMAGE_NONE;
			
				R_SetShaderStageImageFlags(stage, imageflags);

				image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_CUBEMAP);
			
				if(!image)
				{
					ri.Com_Printf("R_FindMaterialShaderStageImage: shader '%s' has no reflection cubemap '%s'\n", shader->getName(), stage->image_name.c_str());
					image = r_img_cubemap_normal;
				}
			
				stage->image = image;
			}
			else
			{
				stage->image = r_img_cubemap_normal;
			}
			
			break;
		}
		
		case SHADER_MATERIAL_STAGE_TYPE_REFRACTIONMAP:
		{
			if(stage->image_name.length())
			{
				imageflags = IMAGE_NONE;
			
				R_SetShaderStageImageFlags(stage, imageflags);

				image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_CUBEMAP);
		
				if(!image)
				{
					ri.Com_Printf("R_FindMaterialShaderStageImage: shader '%s' has no refraction cubemap '%s'\n", shader->getName(), stage->image_name.c_str());
					image = r_img_cubemap_normal;
				}
			
				stage->image = image;
			}
			else
			{
				stage->image = r_img_cubemap_normal;
			}
			
			break;
		}
		
		case SHADER_MATERIAL_STAGE_TYPE_DISPERSIONMAP:
		{
			if(stage->image_name.length())
			{
				imageflags = IMAGE_NONE;
			
				R_SetShaderStageImageFlags(stage, imageflags);

				image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_CUBEMAP);
		
				if(!image)
				{
					ri.Com_Printf("R_FindMaterialShaderStageImage: shader '%s' has no dispersion cubemap '%s'\n", shader->getName(), stage->image_name.c_str());
					image = r_img_cubemap_normal;
				}
			
				stage->image = image;
			}
			else
			{
				stage->image = r_img_cubemap_normal;
			}
			
			break;
		}
		
		case SHADER_MATERIAL_STAGE_TYPE_LIQUIDMAP:
		{
			if(stage->image_name.length())
			{
				imageflags = IMAGE_NONE;
			
				R_SetShaderStageImageFlags(stage, imageflags);

				image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_CUBEMAP);
		
				if(!image)
				{
					ri.Com_Printf("R_FindMaterialShaderStageImage: shader '%s' has no liquid cubemap '%s'\n", shader->getName(), stage->image_name.c_str());
					image = r_img_cubemap_normal;
				}
			
				stage->image = image;
			}
			else
			{
				stage->image = r_img_cubemap_normal;
			}
			
			break;
		}
		
		default:
		{
			ri.Com_Error(ERR_DROP, "R_FindMaterialShaderStageImage: unknown shader stage type %i", stage->type);
			stage->image = r_img_default;
		}
	}
}


static void	R_FindLightShaderStageImage(r_shader_c *shader, r_shader_stage_c *stage)
{
	int			imageflags;
	r_image_c*		image = NULL;

	switch(stage->type_light)
	{
		case SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY:
		{
			/*
			if(stage->videomap_name.length())
			{		
				imageflags = IMAGE_NONE;
	
				R_SetShaderStageImageFlags(stage, imageflags);
	
				image = R_FindImage(stage->videomap_name, imageflags, IMAGE_UPLOAD_VIDEOMAP);
		
				if(!image)
				{
					ri.Com_Printf("R_FindLightShaderStageImage: shader '%s' has no xy attenuation videomap '%s'\n", shader->getName(), stage->videomap_name.c_str());
					image = r_img_default;
				}
		
				stage->image = image;
			}
			else 
			*/
			if(stage->image_name.length())
			{		
				imageflags = IMAGE_NONE;
	
				R_SetShaderStageImageFlags(stage, imageflags);
	
				image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_COLORMAP);
		
				if(!image)
				{
					ri.Com_Printf("R_FindLightShaderStageImage: shader '%s' has no xy attenuationmap '%s'\n", shader->getName(), stage->image_name.c_str());
					image = r_img_default;
				}
		
				stage->image = image;
			}
			else
			{
				stage->image = r_img_white;
			}
			
			break;
		}
		
		case SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_Z:
		{
			if(stage->image_name.length())
			{
				imageflags = IMAGE_CLAMP_TO_EDGE;
							
				R_SetShaderStageImageFlags(stage, imageflags);
	
				image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_COLORMAP);
			
				if(!image)
				{
					ri.Com_Printf("R_FindLightShaderStageImage: shader '%s' has no z attenuationmap '%s'\n", shader->getName(), stage->image_name.c_str());
					image = r_img_nofalloff;
				}
			
				stage->image = image;
			}
			else
			{
				stage->image = r_img_nofalloff;
			}
			
			break;
		}
		
		case SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_CUBE:
		{
			if(stage->image_name.length())
			{
				imageflags = IMAGE_NONE;
			
				R_SetShaderStageImageFlags(stage, imageflags);

				image = R_FindImage(stage->image_name, imageflags, IMAGE_UPLOAD_CUBEMAP);
		
				if(!image)
				{
					ri.Com_Printf("R_FindLightShaderStageImage: shader '%s' has no attenuation cubemap '%s'\n", shader->getName(), stage->image_name.c_str());
					image = r_img_cubemap_normal;
				}
			
				stage->image = image;
			}
			else
			{
				stage->image = r_img_cubemap_normal;
			}
			
			break;
		}
		
		default:
		{
			ri.Com_Error(ERR_DROP, "R_FindLightShaderStageImage: unknown shader stage type %i", stage->type_light);
			stage->image = r_img_white;
		}
	}
}


static void	R_FinishShader(r_shader_c *shader)
{
	if(shader->getType() != SHADER_LIGHT)
	{
		if(shader->stage_diffusemap)
		{
			if(!shader->stage_bumpmap)
			{
				// create default bumpmap stage
				r_shader_stage_c* stage = new r_shader_stage_c();
				stage->type = SHADER_MATERIAL_STAGE_TYPE_BUMPMAP;
				stage->image_name = "_flat";
			
				shader->stage_bumpmap = stage;
				shader->stages.push_back(stage);
			}
		
			if(!shader->stage_specularmap)
			{
				// create default specularmap stage
				r_shader_stage_c* stage = new r_shader_stage_c();
				stage->type = SHADER_MATERIAL_STAGE_TYPE_SPECULARMAP;
				stage->image_name = "_black";
			
				shader->stage_specularmap = stage;
				shader->stages.push_back(stage);
			}
			
			if(!shader->stage_lightmap)
			{
				// create default lightmap stage
				r_shader_stage_c* stage = new r_shader_stage_c();
				stage->type = SHADER_MATERIAL_STAGE_TYPE_LIGHTMAP;
							
				shader->stage_lightmap = stage;
				shader->stages.push_back(stage);
			}
			
			if(!shader->stage_deluxemap)
			{
				// create default lightmap stage
				r_shader_stage_c* stage = new r_shader_stage_c();
				stage->type = SHADER_MATERIAL_STAGE_TYPE_DELUXEMAP;
							
				shader->stage_deluxemap = stage;
				shader->stages.push_back(stage);
			}
		}
	
		for(std::vector<r_shader_stage_c*>::iterator ir = shader->stages.begin(); ir != shader->stages.end(); ++ir)
			R_FindMaterialShaderStageImage(shader, *ir);
	}
	else
	{
		if(!shader->stage_attenuationmap_z)
		{
			// create default z attenuationmap stage
			r_shader_stage_c* stage = new r_shader_stage_c();
			stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_Z;
			//stage->flags |= SHADER_STAGE_ZEROCLAMP;
			stage->image_name = "lights/squarelight1a.tga";
			
			shader->stage_attenuationmap_z = stage;
			shader->stages.push_back(stage);
		}
		
		if(!shader->stage_attenuationmap_cube)
		{
			// create default cube attenuationmap stage
			r_shader_stage_c* stage = new r_shader_stage_c();
			stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_CUBE;
			stage->image_name = "_cubemap_white";
			
			shader->stage_attenuationmap_cube = stage;
			shader->stages.push_back(stage);
		}
				
		for(std::vector<r_shader_stage_c*>::iterator ir = shader->stages.begin(); ir != shader->stages.end(); ++ir)
			R_FindLightShaderStageImage(shader, *ir);
	}
	
	/*
	for(std::vector<r_shader_stage_c*>::iterator ir = shader->stages.begin(); ir != shader->stages.end(); ++ir)
	{
		if(!(*ir)->image)
			ri.Com_Error(ERR_DROP, "R_FinishShader: shader '%s' has stage with no image", shader->getName());
	}
	*/
	
	/*
	if(shader->hasFlags(SHADER_TRANSLUCENT))
	{
		shader->setSort(SHADER_SORT_ADDITIVE);
	}
	
	if(!shader->getSort())
		shader->setSort(SHADER_SORT_OPAQUE);
	*/
	
	if(shader->hasFlags(SHADER_SKY) && shader->hasFlags(SHADER_DEPTHWRITE))
	{
		shader->removeFlags(SHADER_DEPTHWRITE);
	}


	/*
	if(!shader->stages.size() && !shader->sort)
	{
		shader->sort = SHADER_SORT_ADDITIVE;
		return;
	}
	*/


	/*
	for(std::deque<r_shader_stage_c>::iterator ir = shader->stages.begin(); ir != shader->stages.end(); ++ir)
	{
		r_shader_stage_c& stage = *ir;
		
		if(stage.flags & SHADER_STAGE_ALPHATEST)
			shader->sort = SHADER_SORT_ADDITIVE;
	}
	*/
		
	/*
	if(!(shader->flags & SHADER_DEPTHWRITE) && !(shader->flags & SHADER_SKY))
	{
		shader->flags |= SHADER_DEPTHWRITE;
	
		for(std::deque<r_shader_stage_c>::iterator ir = shader->stages.begin(); ir != shader->stages.end(); ++ir)
		{
			r_shader_stage_c& stage = *ir;
			
			stage.flags |= SHADER_STAGE_DEPTHWRITE;
		}
	}
	*/
	
	/*
	if(shader->flags & SHADER_NOSHADOWS)
	{
		for(std::deque<r_shader_stage_c>::iterator ir = shader->stages.begin(); ir != shader->stages.end(); ++ir)
		{
			r_shader_stage_c& stage = *ir;
			
			stage.flags |= SHADER_STAGE_NOSHADOWS;
		}
	}
	*/
}


void	R_InitShaders()
{
	ri.Com_Printf("------- R_InitShaders -------\n");

	std::vector<std::string>	shadernames;

	if((shadernames = ri.VFS_ListFiles("materials", ".mtr")).size() != 0)
	{
		for(std::vector<std::string>::const_iterator ir = shadernames.begin(); ir != shadernames.end(); ++ir)
		{
			R_PrecacheShaderFile(*ir);
		}
	}
}


void	R_ShutdownShaders()
{
	ri.Com_Printf("------- R_ShutdownShaders -------\n");

	X_purge<std::vector<r_shader_c*> >(r_shaders);
	r_shaders.clear();
		
	X_purge<std::vector<r_shader_cache_c*> >(r_shaders_cache);
	r_shaders_cache.clear();
}

void	R_FreeUnusedShaders()
{
	for(std::vector<r_shader_c*>::iterator ir = r_shaders.begin(); ir != r_shaders.end(); ++ir)
	{
		r_shader_c* shader = *ir;
		
		if(!shader)
			continue;		// free shader slot
				
		if(shader->getRegistrationSequence() != r_registration_sequence)
		{
			delete shader;
			*ir = NULL;
			continue;		// not used this sequence
		}
		
		for(std::vector<r_shader_stage_c*>::iterator ir2 = shader->stages.begin(); ir2 != shader->stages.end(); ++ir2)
		{
			r_shader_stage_c* stage = *ir2;
			
			if(stage->image)
				stage->image->setRegistrationSequence();
		}
	}
}


void	R_ShaderList_f()
{
	r_shader_c*	shader;

	ri.Com_Printf("------------------\n");

	for(std::vector<r_shader_c*>::iterator ir = r_shaders.begin(); ir != r_shaders.end(); ++ir)
	{
		shader = *ir;
		
		if(!shader)
			continue;
		
		ri.Com_Printf("%s\n", shader->getName());
	}
	
	//ri.Com_Printf("Total shaders count: %i\n", i);
}


void	R_ShaderCacheList_f()
{
	r_shader_cache_c*	cache;
	

	ri.Com_Printf("------------------\n");
	
	for(std::vector<r_shader_cache_c*>::const_iterator ir = r_shaders_cache.begin(); ir != r_shaders_cache.end(); ++ir)
	{
		cache = *ir;
		
		ri.Com_Printf("'%s' '%s' '%i' '%i'\n", cache->getName(), cache->getPath(), cache->getBeginOffset(), cache->getEndOffset());
	}
	
	ri.Com_Printf("Total shaders caches count: %i\n", r_shaders_cache.size());
}



void	R_ShaderSearch_f()
{
	r_shader_cache_c*	cache = NULL;	
	
	if(ri.Cmd_Argc() != 2)
	{
		ri.Com_Printf("usage: shadersearch <shader>\n");
		return;
	}  	
  
 	cache = R_FindShaderCache(ri.Cmd_Argv(1));
	
	if(cache)
	{
		ri.Com_Printf("'%s' '%s' '%i' '%i'\n", cache->getName(), cache->getPath(), cache->getBeginOffset(), cache->getEndOffset());
	}
	else
		ri.Com_Printf("shader: '%s' not found\n", ri.Cmd_Argv(1));
}

void	R_SpiritTest_f()
{
	const std::string exp = ri.Cmd_Args();
	
#if 1
	r_entity_t dummy;
	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> info;
		
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), info))
	{
		Com_Printf("parsing succeeded\n");
		Com_Printf("result: %f\n", RB_Evaluate(dummy, info, -1.0));
	}
	else
	{
		Com_Printf("parsing failed\n");
	}
	
#elif 0
	std::string		dummy_name;
	std::vector<float>	dummy_values;

	if(R_ParseExpressionToFloatVector(exp, dummy_name, dummy_values))
	{
		Com_Printf("parsing succeeded\n");
	}
	else
	{
		Com_Printf("parsing failed\n");
	}
	
#elif 0
	std::vector<std::string>	v;

	if(R_ParseExpressionToStringVector(exp, v))
	{
		for(std::vector<std::string>::const_iterator ir = v.begin(); ir != v.end(); ++ir)
			ri.Com_Printf("'%s'\n", (*ir).c_str());
	}
	else
	{
		Com_Printf("parsing failed\n");
	}
	
#elif 0
	std::string	name;
	float		scale;

	if(R_ParseHeightmap(exp, name, scale))
	{		
		Com_Printf("parsing succeeded\n");
		Com_Printf("result: '%s' %f\n", name.c_str(), scale);
	}
	else
	{
		Com_Printf("parsing failed\n");
	}
#endif
}


static bool	R_ParseShader(r_shader_c *shader, const char* begin, const char *end)
{
	r_current_shader = shader;
	
	std::string s(begin, end);
	
	//ri.Com_Printf("%s\n", s.c_str());

	r_shader_grammar_t	grammar;
	
	boost::spirit::parse_info<r_iterator_t> info = boost::spirit::parse
	(
		begin,
		end,
		grammar,
		boost::spirit::space_p ||
		boost::spirit::comment_p("/*", "*/") ||
		boost::spirit::comment_p("//")
	);
	
	return info.full;
}



static r_shader_c*	R_GetFreeShader(const std::string &name, r_shader_type_e type)
{
	r_shader_c *shader = new r_shader_c(name, type);
	
	std::vector<r_shader_c*>::iterator ir = find(r_shaders.begin(), r_shaders.end(), static_cast<r_shader_c*>(NULL));
	
	if(ir != r_shaders.end())
		*ir = shader;
	else
		r_shaders.push_back(shader);
	
	return shader;
}

static r_shader_c*	R_LoadShader(const std::string &name, r_shader_type_e type)
{
	r_shader_c*		shader = NULL;
	r_shader_stage_c*	stage = NULL;
	r_shader_cache_c*	cache = NULL;
	char*			buf = NULL;
	
 	shader = R_GetFreeShader(name, type);
	
	cache = R_FindShaderCache(name);
	
	if(cache)
	{	
		if(ri.VFS_FLoad(cache->getPath(), (void**)&buf) <= 0)
			cache = NULL;	// file doesn't exist
	}
	
	if(cache)
	{
		ri.Com_Printf("loading custom shader '%s' ...\n", name.c_str());
				
		if(R_ParseShader(shader, buf + cache->getBeginOffset(), buf + cache->getEndOffset()))
		{
			//ri.Com_Printf("R_LoadShader: parsing succeeded\n");	
		}
		else
		{
			ri.Com_Printf("R_LoadShader: parsing failed\n");
		}

		R_FinishShader(shader);
		
		ri.VFS_FFree(buf);
	}
	else
	{		
		ri.Com_Printf("loading default shader '%s' ...\n", name.c_str());
	
		// make default shader
		switch(type)
		{
			case SHADER_2D:
				stage = new r_shader_stage_c();
				stage->type = SHADER_MATERIAL_STAGE_TYPE_COLORMAP;
				stage->flags = SHADER_STAGE_BLEND | SHADER_STAGE_NOMIPMAPS | SHADER_STAGE_NOPICMIP;
				stage->blend_src = GL_SRC_ALPHA;
				stage->blend_dst = GL_ONE_MINUS_SRC_ALPHA;
				stage->rgb_gen = SHADER_RGB_GEN_ENTITY;
				stage->alpha_gen = SHADER_ALPHA_GEN_ENTITY;
				stage->image_name = name;
				
				//shader->flags = SHADER_NOPICMIC | SHADER_NOMIPMAPS;
				//shader->setSort(SHADER_SORT_ADDITIVE);
				shader->setORFlags(SHADER_TRANSLUCENT);
				shader->stages.push_back(stage);
				break;

			case SHADER_3D:
				stage = new r_shader_stage_c();
				stage->type = SHADER_MATERIAL_STAGE_TYPE_DIFFUSEMAP;
				stage->flags = SHADER_STAGE_DEPTHWRITE;
				//stage->blend_src = GL_SRC_ALPHA;
				//stage->blend_dst = GL_ONE_MINUS_SRC_ALPHA;
				stage->rgb_gen = SHADER_RGB_GEN_IDENTITY;
				stage->alpha_gen = SHADER_ALPHA_GEN_IDENTITY;
				stage->image_name = name;

				//shader->flags = SHADER_DEPTHWRITE;// | SHADER_CULL_FRONT;
				//shader->setSort(SHADER_SORT_OPAQUE);
				shader->stage_diffusemap = stage;
				shader->stages.push_back(stage);
				break;
				
			case SHADER_PARTICLE:
				stage = new r_shader_stage_c();
				stage->type = SHADER_MATERIAL_STAGE_TYPE_COLORMAP;
				stage->flags = SHADER_STAGE_DEPTHWRITE | SHADER_STAGE_BLEND;
				stage->blend_src = GL_ONE;
				stage->blend_dst = GL_ONE;
				stage->rgb_gen = SHADER_RGB_GEN_ENTITY;
				stage->alpha_gen = SHADER_ALPHA_GEN_ENTITY;
				//alphagen_func
				stage->image_name = name;
						
				//shader->flags = 0;/*SHADER_DEPTHWRITE | SHADER_CULL_FRONT*/;
				shader->setORFlags(SHADER_TRANSLUCENT);
				shader->setSort(10);
				shader->stages.push_back(stage);
				break;
			
			case SHADER_LIGHT:
				stage = new r_shader_stage_c();
				stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY;
				stage->flags = SHADER_STAGE_ZEROCLAMP;
				//stage->blend_src = GL_SRC_ALPHA;
				//stage->blend_dst = GL_ONE_MINUS_SRC_ALPHA;
				stage->rgb_gen = SHADER_RGB_GEN_ENTITY;
				stage->alpha_gen = SHADER_ALPHA_GEN_ENTITY;
				stage->image_name = name;

				//shader->flags = SHADER_DEPTHWRITE;// | SHADER_CULL_FRONT;
				//shader->sort = SHADER_SORT_OPAQUE;
				shader->stages.push_back(stage);
				break;

	
			default:
				ri.Com_Error(ERR_DROP, "R_LoadShader: bad shader type %i", type);
     
  		}
		
		R_FinishShader(shader);
	}

	return shader;
}


r_shader_c*	R_FindShader(const std::string &name, r_shader_type_e type)
{
  	r_shader_c*	shader;

	if(!name.length())
	{	
		ri.Com_Error(ERR_DROP, "R_FindShader: empty name");
		return NULL;
	}
	
	std::string name_short = X_strlwr(Com_StripExtension(name));

	for(std::vector<r_shader_c*>::iterator ir = r_shaders.begin(); ir != r_shaders.end(); ++ir)
	{
		shader = *ir;
		
		if(!shader)
			continue;
		
		if(X_strcaseequal(name_short.c_str(), shader->getName()))
		{
			shader->setRegistrationSequence();
			return shader;
		}
	}

	//
	// load the shader from disk
	//
	shader = R_LoadShader(name_short, type);

	return shader;
}


r_shader_c*	R_RegisterShader(const std::string &name)
{
	return R_FindShader(name, SHADER_3D);
}

r_shader_c*	R_RegisterPic(const std::string &name)
{
	return R_FindShader(name, SHADER_2D);
}


r_shader_c*	R_RegisterParticle(const std::string &name)
{
	return R_FindShader(name, SHADER_PARTICLE);
}

r_shader_c*	R_RegisterLight(const std::string &name)
{
	return R_FindShader(name, SHADER_LIGHT);
}


int	R_GetNumForShader(r_shader_c *shader)
{
	if(!shader)
	{
		ri.Com_Error(ERR_DROP, "R_GetNumForShader: NULL parameter\n");
		return -1;
	}

	for(unsigned int i=0; i<r_shaders.size(); i++)
	{
		if(shader == r_shaders[i])
			return i;
	}
	
	ri.Com_Error(ERR_DROP, "R_GetNumForShader: bad pointer\n");
	return -1;
}


r_shader_c*	R_GetShaderByNum(int num)
{
	if(num < 0 || num >= (int)r_shaders.size())
	{
		ri.Com_Error(ERR_DROP, "R_GetShaderByNum: bad number %i\n", num);
		
		//assert(num);	//FIXME
		
		return NULL;
	}

	return r_shaders[num];
}



int	R_RegisterShaderExp(const std::string &name)
{
	return R_GetNumForShader(R_FindShader(name, SHADER_3D));
}

int	R_RegisterPicExp(const std::string &name)
{
	return R_GetNumForShader(R_FindShader(name, SHADER_2D));
}

int	R_RegisterParticleExp(const std::string &name)
{
	return R_GetNumForShader(R_FindShader(name, SHADER_PARTICLE));
}

int	R_RegisterLightExp(const std::string &name)
{
	return R_GetNumForShader(R_FindShader(name, SHADER_LIGHT));
}






