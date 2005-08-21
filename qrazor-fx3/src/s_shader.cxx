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
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility.hpp>

// qrazor-fx ----------------------------------------------------------------
#include "s_local.h"

#include "vfs.h"

// xreal --------------------------------------------------------------------




s_shader_c::s_shader_c(const std::string &name)
{
	_name			= name;
	_registration_sequence	= s_registration_sequence;
	
	_type			= SND_TYPE_OMNI;
	
	_gain			= 1.0;
	_gain_min		= 0.0;
	_gain_max		= 1.0;
	
	_distance_ref		= 1.0;
	_distance_max		= -1;
	_rolloff_factor		= 1.0;
	
	_pitch			= 1.0;
		
	
	// find free sound shader slot
	std::vector<s_shader_c*>::iterator ir = find(s_shaders.begin(), s_shaders.end(), static_cast<s_shader_c*>(NULL));
	
	if(ir != s_shaders.end())
		*ir = this;
	else
		s_shaders.push_back(this);
}

s_shader_c::~s_shader_c()
{
#if 0
	// clear sound shader slot
	std::vector<s_shader_c*>::iterator ir = find(s_shaders.begin(), s_shaders.end(), static_cast<s_shader_c*>(this));
	
	if(ir != s_shaders.end())
		*ir = NULL;
#endif
}


void	s_shader_c::createDefaultBuffer()
{
	s_buffer_c *buffer = S_FindBuffer(_name);
	
	if(buffer)
		_buffers.push_back(buffer);
}

void	s_shader_c::createSource(const vec3_c &origin, const vec3_c &velocity, int ent_num, int ent_channel, bool looping)
{
	// select random buffer
	s_buffer_c *buffer = selectRandomBuffer();
	if(!buffer)
	{
		//Com_Error(ERR_DROP, "s_shader_c::createSound: no sound buffer");
		return;
	}


	s_source_c *source = new s_source_c();
	
	source->setBuffer(buffer);
	
	source->setPosition(origin);
	source->setVelocity(velocity);
	source->setEntityNum(ent_num);
	source->setEntityChannel(ent_channel);
	
	source->setLooping(looping);
	
//	source->setGain(_gain);
//	source->setMinGain(_gain_min);
//	source->setMaxGain(_gain_max);
	
//	source->setRefDistance(_distance_ref);
//	if(_distance_max >= 0)
//		source->setMaxDistance(_distance_max);
//	source->setRolloffFactor(_rolloff_factor);
	
//	source->setPitch(_pitch);
}

s_buffer_c*	s_shader_c::selectRandomBuffer()
{
	if(!_buffers.size())
		return NULL;
		
	return _buffers[(uint_t)((_buffers.size()-1) * X_frand()) % _buffers.size()];
}


class s_shader_cache_c
{
private:
	s_shader_cache_c();
	s_shader_cache_c(const s_shader_cache_c &cache);

public:
	
	s_shader_cache_c(const std::string &name, const std::string &path, unsigned int offset_begin, unsigned int offset_end)
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



std::vector<s_shader_cache_c*>	s_shaders_cache;


static s_shader_cache_c*	S_FindShaderCache(const std::string &name)
{
	std::string name_short = Com_StripExtension(name);
		
	for(std::vector<s_shader_cache_c*>::const_iterator ir = s_shaders_cache.begin(); ir != s_shaders_cache.end(); ir++)
	{
		if(X_strcaseequal(name_short.c_str(), (*ir)->getName()))
			return (*ir);
	}

	return NULL;
}

static s_shader_cache_c*	S_GetShaderCache(const std::string &cache_name, const std::string &cache_path, unsigned int offset_begin, unsigned int offset_end)
{
	s_shader_cache_c	*cache;
	
	//Com_DPrintf("S_GetShaderCache: '%s' '%s' '%i' '%i'\n", cache_name.c_str(), cache_path.c_str(), offset_begin, offset_end);
	
	cache = S_FindShaderCache(cache_name);
	
	if(cache)
		return cache;

	cache = new s_shader_cache_c(cache_name, cache_path, offset_begin, offset_end);

	// link the variable in
	s_shaders_cache.push_back(cache);

	return cache;
}

static void	S_SkipShaderBody(char **buf_p)
{
	char *token = Com_Parse(buf_p, true);	// skip '{'
	
	if(token[0] != '{')
	{
		Com_Error(ERR_DROP, "S_SkipShaderBody: found '%s' when expecting '{'", token);
		return;
	}
	
	while(buf_p)
	{
		char *token = Com_Parse(buf_p);
		
		if(token[0] ==  '}')
			break;
		
		//if(token[0] ==  '{')
		//	S_SkipShaderStage(buf_p);
	}
}

static void	S_PrecacheShaderFile(const std::string &filename)
{
	char*	data = NULL;
	char*	data_p;
	char*	token;
	
	int	len;
	int	offset_start;
	int	offset_end;
	

	len = VFS_FLoad(filename, (void**)&data);
	if(!data)
	{
		Com_Printf("S_PrecacheShaderFile: couldn't load '%s'\n", filename.c_str());
		return;
	}
	
	Com_Printf("precaching '%s' ...\n", filename.c_str());
		
	data_p = data;
	
	while(data_p)
	{
		token = Com_Parse(&data_p);
		
		if(!token[0])
		{
			break;
		}
		else
		{
			// it should be a shader name with a shader body
		
			std::string cname = token;
		
			offset_start = data_p - data;
		
			S_SkipShaderBody(&data_p);
			
			offset_end = data_p - data;
		
			S_GetShaderCache(cname, filename, offset_start, offset_end);
		}
	}

	VFS_FFree(data);
}

void		S_InitShaders()
{
	Com_Printf("------- S_InitShaders -------\n");

	std::vector<std::string>	shadernames;

	if((shadernames = VFS_ListFiles("sound", ".sshader")).size() != 0)
	{
		for(std::vector<std::string>::const_iterator ir = shadernames.begin(); ir != shadernames.end(); ir++)
		{
			S_PrecacheShaderFile(*ir);
		}
	}
}


void		S_ShutdownShaders()
{
	Com_Printf("------- S_ShutdownShaders -------\n");

	X_purge<std::vector<s_shader_c*> >(s_shaders);
	s_shaders.clear();
	
	X_purge<std::vector<s_shader_cache_c*> >(s_shaders_cache);
	s_shaders_cache.clear();
}



static s_shader_c*	s_current_shader;


static void	S_Unknown_sc(char const* begin, char const* end)
{
	Com_Printf("unknown shader command: '%s'\n", std::string(begin, end).c_str());
}

void	S_Buffer_sc(char const* begin, char const* end)
{
	std::string name(begin, end);
	
	s_buffer_c *buffer = S_FindBuffer(name);
	
	if(buffer)
		s_current_shader->_buffers.push_back(buffer);
}

static void	S_Gain_sc(float val)
{
	s_current_shader->setGain(val);
}

static void	S_MinGain_sc(float val)
{
	s_current_shader->setMinGain(val);
}

static void	S_MaxGain_sc(float val)
{
	s_current_shader->setMaxGain(val);
}

static void	S_RefDistance_sc(float val)
{
	s_current_shader->setRefDistance(val);
}

static void	S_MaxDistance_sc(float val)
{
	s_current_shader->setMaxDistance(val);
}

static void	S_RolloffFactor_sc(float val)
{
	s_current_shader->setRolloffFactor(val);
}

static void	S_Pitch_sc(float val)
{
	s_current_shader->setPitch(val);
}

struct s_shader_grammar_t : public boost::spirit::grammar<s_shader_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(s_shader_grammar_t const& self)
		{
			// start grammar definition
			cxx_comment
				=	boost::spirit::comment_p("//")
				;
			
			restofline
				=	boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::eol_p | boost::spirit::ch_p('}'))]]
				;
			
				// shader commands
			buffer_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("buffer")] >> restofline[&S_Buffer_sc]
				;
				
			gain_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("gain")] >> +boost::spirit::real_p[&S_Gain_sc]
				;
				
			mingain_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("mingain")] >> +boost::spirit::real_p[&S_MinGain_sc]
				;
			
			maxgain_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("maxgain")] >> +boost::spirit::real_p[&S_MaxGain_sc]
				;
				
			refdistance_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("refdistance")] >> +boost::spirit::real_p[&S_RefDistance_sc]
				;
			
			maxdistance_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("maxdistance")] >> +boost::spirit::real_p[&S_MaxDistance_sc]
				;
				
			rollofffactor_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("rollofffactor")] >> +boost::spirit::real_p[&S_RolloffFactor_sc]
				;
			
			pitch_sc
				=	boost::spirit::nocase_d[boost::spirit::str_p("pitch")] >> +boost::spirit::real_p[&S_Pitch_sc]
				;
				
			unknown_sc
				=	restofline[&S_Unknown_sc]
				;
				
			shader_command
				=	cxx_comment		|
					buffer_sc		|
					mingain_sc		|
					maxgain_sc		|
					gain_sc			|
					refdistance_sc		|
					maxdistance_sc		|
					rollofffactor_sc	|
					pitch_sc		|
					unknown_sc
				;
				
			expression
				=	boost::spirit::refactor_unary_d[*boost::spirit::anychar_p - boost::spirit::ch_p('{')] >>
					boost::spirit::ch_p('{') >> 
					+shader_command >> 
					boost::spirit::ch_p('}')
				;
				
			// end grammar definiton
		}
		
		boost::spirit::rule<ScannerT>	cxx_comment,
						restofline,
							shader_command,
								buffer_sc,
								gain_sc,
								mingain_sc,
								maxgain_sc,
								refdistance_sc,
								maxdistance_sc,
								rollofffactor_sc,
								pitch_sc,
								unknown_sc,
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};

static bool	S_ParseShader(s_shader_c *shader, const char* begin, const char *end)
{
	s_current_shader = shader;
	
	std::string s(begin, end);
	
	//ri.Com_Printf("%s\n", s.c_str());

	s_shader_grammar_t	grammar;
	
	boost::spirit::parse_info<> info = boost::spirit::parse(begin, end, grammar, boost::spirit::space_p);
	
	return info.full;
}

static s_shader_c*	S_LoadShader(const std::string &name)
{
	s_shader_c*		shader = NULL;
	s_shader_cache_c*	cache = NULL;
	char*			buf = NULL;
	
 	shader = new s_shader_c(name);
	
	cache = S_FindShaderCache(name);
	
	if(cache)
	{	
		if(VFS_FLoad(cache->getPath(), (void**)&buf) <= 0)
			cache = NULL;	// file doesn't exist
	}
	
	if(cache)
	{
		Com_Printf("loading custom sound shader '%s' ...\n", name.c_str());
				
		if(S_ParseShader(shader, buf + cache->getBeginOffset(), buf + cache->getEndOffset()))
		{
			//ri.Com_Printf("R_LoadShader: parsing succeeded\n");	
		}
		else
		{
			Com_Printf("S_LoadShader: parsing failed\n");
		}

		VFS_FFree(buf);
		
		if(!shader->hasBuffers())
			shader->createDefaultBuffer();
	}
	else
	{		
		Com_Printf("loading default sound shader '%s' ...\n", name.c_str());
	
		shader->createDefaultBuffer();
	}

	return shader;
}


s_shader_c*	S_FindShader(const std::string &name)
{
	if(name.empty())
		Com_Error(ERR_FATAL, "S_FindShader: empty name\n");
		
	std::string name_short = Com_StripExtension(name);
			
	for(std::vector<s_shader_c*>::const_iterator ir = s_shaders.begin(); ir != s_shaders.end(); ir++)
	{
		s_shader_c* shader = *ir;
		
		if(!shader)
			continue;
		
		if(X_strcaseequal(name_short.c_str(), shader->getName()))
		{
			shader->setRegistrationSequence();
			return shader;
		}
	}
	
	return S_LoadShader(name_short);
}

int	S_GetNumForShader(s_shader_c *shader)
{
	if(!shader)
	{
		//Com_Error(ERR_DROP, "S_GetNumForShader: NULL parameter\n");
		return -1;
	}

	for(unsigned int i=0; i<s_shaders.size(); i++)
	{
		if(s_shaders[i] == shader)
			return i;
	}
	
	Com_Error(ERR_DROP, "S_GetNumForShader: bad pointer\n");
	return -1;
}

s_shader_c*	S_GetShaderByNum(int num)
{
	if(num < 0 || num >= (int)s_shaders.size())
	{
		Com_Error(ERR_DROP, "S_GetShaderByNum: bad number %i\n", num);
		return NULL;
	}

	return s_shaders[num];
}

void	S_FreeUnusedShaders()
{
	for(std::vector<s_shader_c*>::iterator ir = s_shaders.begin(); ir != s_shaders.end(); ir++)
	{
		s_shader_c *shader = *ir;
		
		if(!shader)
			continue;
		
		if(shader->getRegistrationSequence() == s_registration_sequence)
		{
			for(std::vector<s_buffer_c*>::iterator ir2 = shader->_buffers.begin(); ir2 != shader->_buffers.end(); ir2++)
			{
				s_buffer_c *buffer = *ir2;
				
				if(!buffer)
					continue;
				
				buffer->setRegistrationSequence();
			}
			
			continue;		// used this sequence
		}
		
		// free it
		delete shader;
		
		*ir = NULL;
	}
}
