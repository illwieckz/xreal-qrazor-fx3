/// ============================================================================
/*
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
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
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility.hpp>

// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "map_local.h"
#include "vfs.h"
#include "cmd.h"

// xreal --------------------------------------------------------------------

namespace map
{

static std::map<std::string, shader_c*, strcasecmp_c>	shaders;


shader_c::shader_c(const std::string &name)
{
	_name			= name;
	
	shaders.insert(std::make_pair(name, this));
}

class shader_cache_c
{
private:
	shader_cache_c();
	shader_cache_c(const shader_cache_c &cache);

public:
	
	shader_cache_c(const std::string &name, const std::string &path, unsigned int offset_begin, unsigned int offset_end)
	{
		_name = X_strlwr(Com_StripExtension(name));
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
typedef shader_cache_c*			shader_cache_p;
typedef std::map<std::string, shader_cache_p, strcasecmp_c>	shader_caches_t;
typedef shader_caches_t::iterator				shader_caches_i;
typedef shader_caches_t::const_iterator				shader_caches_ci;

static shader_caches_t						shaders_cache;


static std::string			sp_filename;
static char*				sp_data = NULL;

static std::string			sp_shader_name;
static uint_t 				sp_shader_offset_begin;
static uint_t				sp_shader_offset_end;

static std::string			sp_table_name;

static shader_cache_c*	GetShaderCache(const std::string &cache_name, const std::string &cache_path, unsigned int offset_begin, unsigned int offset_end);

static void	ShaderName(const char* begin, const char* end)
{
	sp_shader_name = std::string(begin, end);
	sp_shader_offset_begin = (end+1) - sp_data;
	
	//ri.Com_Printf("shader name: '%s'\n", sp_shader_name.c_str());
}

static void	PrecacheShader(const char *begin, const char* end)
{
	sp_shader_offset_end = end - sp_data;
	
	GetShaderCache(sp_shader_name, sp_filename, sp_shader_offset_begin, sp_shader_offset_end);
}

static void	TableName(const char* begin, const char* end)
{
	sp_table_name = X_strlwr(std::string(begin, end));
}

static void	ClampTable(const char* begin, const char* end)
{
//	sp_table.flags |= SHADER_TABLE_CLAMP;
}

static void	SnapTable(const char* begin, const char* end)
{
//	sp_table.flags |= SHADER_TABLE_SNAP;
}

static void	ClearTable(const char begin)
{
//	sp_table.flags = 0;
//	sp_table.values.clear();
}

static void	CreateTable(const char* begin, const char* end)
{
//	Com_Printf("   creating table '%s' ...\n", sp_table_name.c_str());
	
//	map_shader_table_symbols_p.add(sp_table_name.c_str(), map_tables.size());
//	map_tables.push_back(sp_table);
}

struct shader_precache_grammar_t : public boost::spirit::grammar<shader_precache_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(shader_precache_grammar_t const& self)
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
					boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::space_p | boost::spirit::ch_p('{'))]][&TableName] >>
					boost::spirit::ch_p('{')[&ClearTable] >>
					*(table_clamp | table_snap) >>
					boost::spirit::ch_p('{') >>
					boost::spirit::list_p(boost::spirit::lexeme_d[boost::spirit::real_p], ',') >>
					boost::spirit::ch_p('}') >>
					boost::spirit::ch_p('}')
				;
				
			table_clamp
				=	boost::spirit::str_p("clamp")[&ClampTable]
				;
				
			table_snap
				=	boost::spirit::str_p("snap")[&SnapTable]
				;
				
			shader
				=	boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::space_p | boost::spirit::ch_p('{'))]][&ShaderName] >>
					boost::spirit::ch_p('{') >> 
					*(skip_block | skip_tobracket) >>
					boost::spirit::ch_p('}')
				;
				
			expression
				=	*(table[&CreateTable] | shader[&PrecacheShader])
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

static shader_cache_c*	FindShaderCache(const std::string &name)
{
//	Com_DPrintf("FindShaderCache: '%s'\n", name.c_str());
	
//	std::string name_short = X_strlwr(Com_StripExtension(name));

	std::string name_short = Com_StripExtension(name);
	
	std::map<std::string, shader_cache_c*>::const_iterator ir = shaders_cache.find(name_short);
	
	if(ir != shaders_cache.end())
		return ir->second;

	return NULL;
}

static shader_cache_c*	GetShaderCache(const std::string &name, const std::string &cache_path, unsigned int offset_begin, unsigned int offset_end)
{
	std::string cache_name = X_strlwr(Com_StripExtension(name));

	shader_cache_c* cache = FindShaderCache(cache_name);
	
	if(cache)
		return cache;

	cache = new shader_cache_c(cache_name, cache_path, offset_begin, offset_end);

	shaders_cache.insert(std::make_pair(cache_name, cache));

	return cache;
}

static void	PrecacheShaderFile(const std::string &filename)
{
	char*	data = NULL;
	int	len;
	
	len = VFS_FLoad(filename, (void**)&data);
	if(!data)
	{
		Com_Printf("couldn't load '%s'\n", filename.c_str());
		return;
	}
	
	Com_Printf("precaching '%s' ...\n", filename.c_str());
	
	sp_filename = filename;
	sp_data = data;
	shader_precache_grammar_t	grammar;
	
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
		Com_Error(ERR_DROP, "PrecacheShaderFile: failed parsing '%s'\n", filename.c_str());
	}
	
	VFS_FFree(data);
}


void	InitShaders()
{
	Com_Printf("------- map::InitShaders -------\n");

	std::vector<std::string>	shadernames;

	if((shadernames = VFS_ListFiles("materials", ".mtr")).size() != 0)
	{
		for(std::vector<std::string>::const_iterator ir = shadernames.begin(); ir != shadernames.end(); ++ir)
		{
			PrecacheShaderFile(*ir);
		}
	}
}

void	ShutdownShaders()
{
	Com_Printf("------- map::ShutdownShaders -------\n");

	for(std::map<std::string, shader_c*>::const_iterator ir = shaders.begin(); ir != shaders.end(); ++ir)
	{
		shader_c* shader = ir->second;
		
		delete shader;
	}
	shaders.clear();
	
	for(std::map<std::string, shader_cache_c*>::const_iterator ir = shaders_cache.begin(); ir != shaders_cache.end(); ++ir)
	{
		shader_cache_c* cache = ir->second;
		
		delete cache;
	}
	shaders_cache.clear();
}





class shader_action_a
{
private:
	shader_action_a();
	
protected:
	shader_action_a(shader_p shader)
	: _shader(shader)
	{
	}
	
	shader_p _shader;
};

class nodraw_c :
public shader_action_a
{
public:
	nodraw_c(shader_p shader)
	: shader_action_a(shader)
	{
	}

	template<typename iterator_T>
	void	operator()(iterator_T begin, iterator_T end) const
	{
		Com_Printf("nodraw_c::()\n");
		
		_shader->addSurfaceFlags(X_SURF_NODRAW);
	}
};

class areaportal_c :
public shader_action_a
{
public:
	areaportal_c(shader_p shader)
	: shader_action_a(shader)
	{
	}

	template<typename iterator_T>
	void	operator()(iterator_T begin, iterator_T end) const
	{
		Com_Printf("areaportal_c::()\n");
		
		_shader->addContentFlags(X_CONT_AREAPORTAL);
		_shader->delContentFlags(X_CONT_SOLID);
		
		_shader->addCompileFlags(C_AREAPORTAL | C_TRANSLUCENT);
		_shader->delCompileFlags(C_SOLID);
	}
};

class unknown_c
{
public:
	template<typename iterator_T>
	void	operator()(iterator_T begin, iterator_T end) const
	{
		Com_Printf("unknown shader command: '%s'\n", std::string(begin, end).c_str());
	}
};

struct shader_grammar_t : public boost::spirit::grammar<shader_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(shader_grammar_t const& self)
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
			
			shader_command
				=	qer_editorimage_sc		|
					qer_trans_sc			|
					surfaceparm			|
					colormap_sc			|
					diffusemap_sc			|
					bumpmap_sc			|
					specularmap_sc			|
					heathazemap_sc			|
					lightmap_sc			|
					deluxemap_sc			|
					reflectionmap_sc		|
					refractionmap_sc		|
					dispersionmap_sc		|
					liquidmap_sc			|
					unknown_sc
				;
				
			qer_editorimage_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("qer_editorimage")] >> skip_restofline
				;
			
			qer_trans_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("qer_trans")] >> boost::spirit::real_p
				;
				
			colormap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("colormap")] >> restofline
				;
			
			diffusemap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("diffusemap")] >> restofline
				;
			
			bumpmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("bumpmap")] >> restofline
				;
			
			specularmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("specularmap")] >> restofline
				;
				
			heathazemap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("heathazemap")] >> restofline
				;
			
			lightmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("lightmap")] >> restofline
				;
				
			deluxemap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("deluxemap")] >> restofline
				;
				
			reflectionmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("reflectionmap")] >> restofline
				;
			
			refractionmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("refractionmap")] >> restofline
				;
				
			dispersionmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("dispersionmap")] >> restofline
				;
			
			liquidmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("liquidmap")] >> restofline
				;
				
			surfaceparm
				=	areaportal_sc			|
					nodraw_sc			|
					nolightmap_sc			|
					nonsolid_sc
				;
			
			areaportal_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("areaportal")][areaportal_c(self._shader)]
				;
				
			nodraw_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("nodraw")][nodraw_c(self._shader)]
				;
				
			nolightmap_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("nolightmap")]
				;
				
			nonsolid_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("nonsolid")]
				;
				
			unknown_sc
				=	restofline[unknown_c()]
				;
				
			shader_stage
				=	skip_block
				;
				
			expression
				=	boost::spirit::refactor_unary_d[*boost::spirit::anychar_p - boost::spirit::ch_p('{')] >>
					boost::spirit::ch_p('{') >> 
					+(shader_stage | shader_command) >> 
					boost::spirit::ch_p('}')
				;
				
			// end grammar definiton
		}
		
		boost::spirit::rule<ScannerT>	restofline,
						skip_restofline,
						skip_tobracket,
						skip_block,
						shader_command,
							qer_editorimage_sc,
							qer_trans_sc,
							surfaceparm,
								areaportal_sc,
								nodraw_sc,
								nolightmap_sc,
								nonsolid_sc,
							colormap_sc,
							diffusemap_sc,
							bumpmap_sc,
							specularmap_sc,
							heathazemap_sc,
							lightmap_sc,
							deluxemap_sc,
							reflectionmap_sc,
							refractionmap_sc,
							dispersionmap_sc,
							liquidmap_sc,
							unknown_sc,
						shader_stage,
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
	
	shader_grammar_t(shader_p shader)
	: _shader(shader)
	{
	}
	shader_p _shader;
};


static bool	ParseShader(shader_c *shader, const char* begin, const char *end)
{
#if 1
	shader_grammar_t grammar(shader);
	
	boost::spirit::parse_info<> info = boost::spirit::parse
	(
		begin,
		end,
		grammar,
		boost::spirit::space_p ||
		boost::spirit::comment_p("/*", "*/") ||
		boost::spirit::comment_p("//")
	);
	
	return info.full;
#else
	return false;
#endif
}

static shader_c*	LoadShader(const std::string &name)
{
	shader_c*		shader = NULL;
	shader_cache_c*		cache = NULL;
	char*			buf = NULL;
	
 	shader = new shader_c(name);
	
	cache = FindShaderCache(name);
	
	if(cache)
	{	
		if(VFS_FLoad(cache->getPath(), (void**)&buf) <= 0)
		{
				
			Com_Printf("loading default shader '%s' ...\n", name.c_str());
		
			//TODO
		}
		else
		{
			Com_Printf("loading custom shader '%s' ...\n", name.c_str());
				
			if(ParseShader(shader, buf + cache->getBeginOffset(), buf + cache->getEndOffset()))
			{
				//ri.Com_Printf("R_LoadShader: parsing succeeded\n");
			}
			else
			{
				Com_Printf("map::LoadShader: parsing failed\n");
			}
		
			VFS_FFree(buf);
		}
	}

	return shader;
}


shader_c*	FindShader(const std::string &name)
{
  	if(!name.length())
	{	
		Com_Error(ERR_FATAL, "map::FindShader: empty name");
		return NULL;
	}
	
	std::string name_short = X_strlwr(Com_StripExtension(name));
	
	std::map<std::string, shader_c*>::const_iterator ir = shaders.find(name_short);
	
	if(ir != shaders.end())
	{
		return ir->second;
	}
	else
	{
		return LoadShader(name_short);
	}
}


void	ShaderList_f()
{
	Com_Printf("------------------\n");

	for(std::map<std::string, shader_c*>::iterator ir = shaders.begin(); ir != shaders.end(); ++ir)
	{
		Com_Printf("%s\n", ir->first.c_str());
	}
	
	Com_Printf("Total shaders count: %i\n", shaders.size());
}

void	ShaderCacheList_f()
{
	Com_Printf("------------------\n");
	
	for(std::map<std::string, shader_cache_c*>::const_iterator ir = shaders_cache.begin(); ir != shaders_cache.end(); ++ir)
	{
		shader_cache_c* cache = ir->second;
		
		Com_Printf("'%s' '%s' '%i' '%i'\n", ir->first.c_str(), cache->getPath(), cache->getBeginOffset(), cache->getEndOffset());
	}
	
	Com_Printf("Total shaders caches count: %i\n", shaders_cache.size());
}

void	ShaderSearch_f()
{
	shader_cache_c*	cache = NULL;	
	
	if(Cmd_Argc() != 2)
	{
		Com_Printf("usage: shadersearch <shader>\n");
		return;
	}

	cache = FindShaderCache(Cmd_Argv(1));

	if(cache)
	{
		Com_Printf("'%s' '%s' '%i' '%i'\n", cache->getName(), cache->getPath(), cache->getBeginOffset(), cache->getEndOffset());
	}
	else
	{
		Com_Printf("shader: '%s' not found\n", Cmd_Argv(1));
	}
}

} // namespace map

