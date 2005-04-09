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



static std::map<std::string, map_shader_c*, strcasecmp_c>	map_shaders;

map_shader_c::map_shader_c(const std::string &name)
{
	_name	= name;
	
	map_shaders.insert(std::make_pair(name, this));
}





class map_shader_cache_c
{
private:
	map_shader_cache_c();
	map_shader_cache_c(const map_shader_cache_c &cache);

public:
	
	map_shader_cache_c(const std::string &name, const std::string &path, unsigned int offset_begin, unsigned int offset_end)
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

static std::map<std::string, map_shader_cache_c*, strcasecmp_c>	map_shaders_cache;


static std::string			map_sp_filename;
static char*				map_sp_data = NULL;

static std::string			map_sp_shader_name;
static uint_t 				map_sp_shader_offset_begin;
static uint_t				map_sp_shader_offset_end;

static std::string			map_sp_table_name;

static map_shader_cache_c*	Map_GetShaderCache(const std::string &cache_name, const std::string &cache_path, unsigned int offset_begin, unsigned int offset_end);

static void	Map_ShaderName(const char* begin, const char* end)
{
	map_sp_shader_name = std::string(begin, end);
	map_sp_shader_offset_begin = (end+1) - map_sp_data;
	
	//ri.Com_Printf("shader name: '%s'\n", map_sp_shader_name.c_str());
}

static void	Map_PrecacheShader(const char *begin, const char* end)
{
	map_sp_shader_offset_end = end - map_sp_data;
	
	Map_GetShaderCache(map_sp_shader_name, map_sp_filename, map_sp_shader_offset_begin, map_sp_shader_offset_end);
}

static void	Map_TableName(const char* begin, const char* end)
{
	map_sp_table_name = X_strlwr(std::string(begin, end));
}

static void	Map_ClampTable(const char* begin, const char* end)
{
//	map_sp_table.flags |= SHADER_TABLE_CLAMP;
}

static void	Map_SnapTable(const char* begin, const char* end)
{
//	map_sp_table.flags |= SHADER_TABLE_SNAP;
}

static void	Map_ClearTable(const char begin)
{
//	map_sp_table.flags = 0;
//	map_sp_table.values.clear();
}

static void	Map_CreateTable(const char* begin, const char* end)
{
//	Com_Printf("   creating table '%s' ...\n", map_sp_table_name.c_str());
	
//	map_shader_table_symbols_p.add(map_sp_table_name.c_str(), map_tables.size());
//	map_tables.push_back(map_sp_table);
}

struct map_shader_precache_grammar_t : public boost::spirit::grammar<map_shader_precache_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(map_shader_precache_grammar_t const& self)
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
					boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::space_p | boost::spirit::ch_p('{'))]][&Map_TableName] >>
					boost::spirit::ch_p('{')[&Map_ClearTable] >>
					*(table_clamp | table_snap) >>
					boost::spirit::ch_p('{') >>
					boost::spirit::list_p(boost::spirit::lexeme_d[boost::spirit::real_p], ',') >>
					boost::spirit::ch_p('}') >>
					boost::spirit::ch_p('}')
				;
				
			table_clamp
				=	boost::spirit::str_p("clamp")[&Map_ClampTable]
				;
				
			table_snap
				=	boost::spirit::str_p("snap")[&Map_SnapTable]
				;
				
			shader
				=	boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::space_p | boost::spirit::ch_p('{'))]][&Map_ShaderName] >>
					boost::spirit::ch_p('{') >> 
					*(skip_block | skip_tobracket) >>
					boost::spirit::ch_p('}')
				;
				
			expression
				=	*(table[&Map_CreateTable] | shader[&Map_PrecacheShader])
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

static map_shader_cache_c*	Map_FindShaderCache(const std::string &name)
{
//	Com_DPrintf("Map_FindShaderCache: '%s'\n", name.c_str());
	
//	std::string name_short = X_strlwr(Com_StripExtension(name));

	std::string name_short = Com_StripExtension(name);
	
	std::map<std::string, map_shader_cache_c*>::const_iterator ir = map_shaders_cache.find(name_short);
	
	if(ir != map_shaders_cache.end())
		return ir->second;
	
	/*
	for(std::vector<map_shader_cache_c*>::const_iterator ir = map_shaders_cache.begin(); ir != map_shaders_cache.end(); ++ir)
	{
		if(X_strcaseequal(name_short.c_str(), (*ir)->getName()))
			return (*ir);
	}
	*/

	return NULL;
}

static map_shader_cache_c*	Map_GetShaderCache(const std::string &name, const std::string &cache_path, unsigned int offset_begin, unsigned int offset_end)
{
	std::string cache_name = X_strlwr(Com_StripExtension(name));

	map_shader_cache_c* cache = Map_FindShaderCache(cache_name);
	
	if(cache)
		return cache;

	cache = new map_shader_cache_c(cache_name, cache_path, offset_begin, offset_end);

	map_shaders_cache.insert(std::make_pair(cache_name, cache));

	return cache;
}

static void	Map_PrecacheShaderFile(const std::string &filename)
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
	
	map_sp_filename = filename;
	map_sp_data = data;
	map_shader_precache_grammar_t	grammar;
	
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
		Com_Error(ERR_DROP, "Map_PrecacheShaderFile: failed parsing '%s'\n", filename.c_str());
	}
	
	VFS_FFree(data);
}


void	Map_InitShaders()
{
	Com_Printf("------- Map_InitShaders -------\n");

	std::vector<std::string>	shadernames;

	if((shadernames = VFS_ListFiles("materials", ".mtr")).size() != 0)
	{
		for(std::vector<std::string>::const_iterator ir = shadernames.begin(); ir != shadernames.end(); ++ir)
		{
			Map_PrecacheShaderFile(*ir);
		}
	}
}

void	Map_ShutdownShaders()
{
	Com_Printf("------- Map_ShutdownShaders -------\n");

//	X_purge(map_shaders);
	for(std::map<std::string, map_shader_c*>::const_iterator ir = map_shaders.begin(); ir != map_shaders.end(); ++ir)
	{
		map_shader_c* shader = ir->second;
		
		delete shader;
	}
	map_shaders.clear();
		
//	X_purge(map_shaders_cache);
	for(std::map<std::string, map_shader_cache_c*>::const_iterator ir = map_shaders_cache.begin(); ir != map_shaders_cache.end(); ++ir)
	{
		map_shader_cache_c* cache = ir->second;
		
		delete cache;
	}
	map_shaders_cache.clear();
}


void	Map_ShaderList_f()
{
	Com_Printf("------------------\n");

	for(std::map<std::string, map_shader_c*>::iterator ir = map_shaders.begin(); ir != map_shaders.end(); ++ir)
	{
		Com_Printf("%s\n", ir->first.c_str());
	}
	
	Com_Printf("Total shaders count: %i\n", map_shaders.size());
}

void	Map_ShaderCacheList_f()
{
	Com_Printf("------------------\n");
	
	for(std::map<std::string, map_shader_cache_c*>::const_iterator ir = map_shaders_cache.begin(); ir != map_shaders_cache.end(); ++ir)
	{
		map_shader_cache_c* cache = ir->second;
		
		Com_Printf("'%s' '%s' '%i' '%i'\n", ir->first.c_str(), cache->getPath(), cache->getBeginOffset(), cache->getEndOffset());
	}
	
	Com_Printf("Total shaders caches count: %i\n", map_shaders_cache.size());
}


static void	Map_Unknown_sc(char const* begin, char const* end)
{
	Com_Printf("unknown shader command: '%s'\n", std::string(begin, end).c_str());
}

struct map_shader_grammar_t : public boost::spirit::grammar<map_shader_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(map_shader_grammar_t const& self)
		{
			// start grammar definition
			restofline
				=	boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::eol_p | boost::spirit::ch_p('}'))]]
				;
				
			skip_restofline
				=	restofline
				;
				
			skip_tobracket
				=	boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::ch_p('{') | boost::spirit::ch_p('}'))]
				;
				
			skip_block
				=	boost::spirit::ch_p('{') >>
					skip_tobracket >>
					boost::spirit::ch_p('}')
				;
			
			shader_command
				=	cxx_comment		|
					unknown_sc
				;
				
			unknown_sc
				=	restofline[&Map_Unknown_sc]
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
		
		boost::spirit::rule<ScannerT>	cxx_comment,
						restofline,
						skip_restofline,
						skip_tobracket,
						skip_block,
						shader_command,
							unknown_sc,
						shader_stage,
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};


static bool	Map_ParseShader(map_shader_c *shader, const char* begin, const char *end)
{
#if 1
	//map_current_shader = shader;

	map_shader_grammar_t	grammar;
	
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

static map_shader_c*	Map_LoadShader(const std::string &name)
{
	map_shader_c*		shader = NULL;
	map_shader_cache_c*	cache = NULL;
	char*			buf = NULL;
	
 	shader = new map_shader_c(name);
	
	cache = Map_FindShaderCache(name);
	
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
				
			if(Map_ParseShader(shader, buf + cache->getBeginOffset(), buf + cache->getEndOffset()))
			{
				//ri.Com_Printf("R_LoadShader: parsing succeeded\n");
			}
			else
			{
				Com_Printf("Map_LoadShader: parsing failed\n");
			}
		
			VFS_FFree(buf);
		}
	}

	return shader;
}


map_shader_c*	Map_FindShader(const std::string &name)
{
  	if(!name.length())
	{	
		Com_Error(ERR_FATAL, "Map_FindShader: empty name");
		return NULL;
	}
	
	std::string name_short = X_strlwr(Com_StripExtension(name));
	
	std::map<std::string, map_shader_c*>::const_iterator ir = map_shaders.find(name_short);
	
	if(ir != map_shaders.end())
	{
		return ir->second;
	}
	else
	{
		return Map_LoadShader(name_short);
	}
}


void	Map_ShaderSearch_f()
{
	map_shader_cache_c*	cache = NULL;	
	
	if(Cmd_Argc() != 2)
	{
		Com_Printf("usage: shadersearch <shader>\n");
		return;
	}  	
  
 	cache = Map_FindShaderCache(Cmd_Argv(1));
	
	if(cache)
	{
		Com_Printf("'%s' '%s' '%i' '%i'\n", cache->getName(), cache->getPath(), cache->getBeginOffset(), cache->getEndOffset());
	}
	else
	{
		Com_Printf("shader: '%s' not found\n", Cmd_Argv(1));
	}
}
