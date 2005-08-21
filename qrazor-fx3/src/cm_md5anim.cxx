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
#include <boost/spirit/symbols/symbols.hpp>
#include <boost/spirit/tree/ast.hpp>
#include <boost/spirit/utility/lists.hpp>
#include <boost/spirit/utility/escape_char.hpp>

// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "cm_md5anim.h"

#include "common.h"
#include "vfs.h"
#include "files.h"

// xreal --------------------------------------------------------------------

static cskel_animation_md5_c*	cm_md5_anim = NULL;
static int			cm_md5_anim_frames_num;

static void	CM_MD5_Version(int version)
{
	if(version != MD5_VERSION)
		Com_Error(ERR_DROP, "CM_MD5_Version: wrong version number (%i should be %i)", version, MD5_VERSION);
}

static void	CM_MD5Anim_NumFrames(int num)
{
	cm_md5_anim_frames_num = num;
}

struct cskel_animation_md5_grammar_t : public boost::spirit::grammar<cskel_animation_md5_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(cskel_animation_md5_grammar_t const& self)
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
				
			ident
				=	boost::spirit::str_p("MD5Version") >> boost::spirit::int_p[&CM_MD5_Version]
				;
				
			commandline
				=	boost::spirit::str_p("commandline") >>
					boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')] >>
					boost::spirit::ch_p('\"')
				;
				
			numframes
				=	boost::spirit::str_p("numFrames") >> boost::spirit::int_p[&CM_MD5Anim_NumFrames]
				;
				
			expression
				=	ident >>
					commandline >>
					*boost::spirit::anychar_p
				;
				
			// end grammar definiton
		}
			
		boost::spirit::rule<ScannerT>	restofline,
						skip_restofline,
						skip_block,
						
						ident,
						commandline,
						numframes,
						
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};


cskel_animation_md5_c::cskel_animation_md5_c(const std::string &name)
:cskel_animation_c(name)
{
}

void	cskel_animation_md5_c::loadChannels(char **data_p)
{
	Com_Parse(data_p);	// skip "numchannels"
	int channels_num = Com_ParseInt(data_p);
			
	if(channels_num <= 0)
	{
		Com_Error(ERR_DROP, "cskel_animation_md5_c::loadChannels: animation '%s' has invalid channels number %i", getName(), channels_num);
		return;
	}
	
	for(int i=0; i<channels_num; i++)
	{
		cskel_channel_t *channel = new cskel_channel_t();
	
		Com_Parse(data_p, true);	// skip "channel"
		Com_Parse(data_p, false);	// skip channel number
		Com_Parse(data_p, false);	// skip '{'
		
		Com_Parse(data_p, true);	// skip "joint"
		channel->joint = Com_Parse(data_p, false);
						
		Com_Parse(data_p, true);	// skip "attribute"
		char* attr = Com_Parse(data_p, false);
		
		if(X_strequal(attr, "x"))
			channel->attribute = CHANNEL_ATTRIB_X;
			
		else if(X_strequal(attr, "y"))
			channel->attribute = CHANNEL_ATTRIB_Y;
			
		else if(X_strequal(attr, "z"))
			channel->attribute = CHANNEL_ATTRIB_Z;
		
		else if(X_strequal(attr, "pitch"))
			channel->attribute = CHANNEL_ATTRIB_PITCH;
			
		else if(X_strequal(attr, "yaw"))
			channel->attribute = CHANNEL_ATTRIB_YAW;
		
		else if(X_strequal(attr, "roll"))
			channel->attribute = CHANNEL_ATTRIB_ROLL;
		
		else
		{
			Com_Error(ERR_DROP, "cskel_animation_md5_c::loadChannels: channel %i has bad attribute '%s'", i, attr);
			return;
		}
		
		Com_Parse(data_p, true);	// skip "starttime"
		channel->time_start = Com_ParseFloat(data_p, false);
		_time_start = X_min(_time_start, channel->time_start);
		
		Com_Parse(data_p, true);	// skip "endtime"
		channel->time_end = Com_ParseFloat(data_p, false);
		_time_end = X_max(_time_end, channel->time_end);
		
		Com_Parse(data_p, true);	// skip "framerate"
		channel->time_fps = Com_ParseFloat(data_p, false);
		
		Com_Parse(data_p, true);	// skip "strings"
		Com_Parse(data_p, false);	// skip strings number
		
		Com_Parse(data_p, true);	// skip "range"
		channel->range[0] = Com_ParseInt(data_p, false);
		channel->range[1] = Com_ParseInt(data_p, false);
		
		_frame_first = X_min(_frame_first, channel->range[0]);
		_frame_last = X_max(_frame_last, channel->range[1]);
		
		Com_Parse(data_p, true);	// skip "keys"
		int keys_num = Com_ParseInt(data_p, false);
		//ri.Com_Printf("channel %i has keys num %i\n", i, keys_num);
		
		channel->keys = std::vector<float>(keys_num);
		for(int j=0; j<keys_num; j++)
		{
			channel->keys[j] = Com_ParseFloat(data_p, true);
		}
		
		char *bracket = Com_Parse(data_p);	// skip '}'
		if(!X_strequal(bracket, "}"))
		{
			Com_Error(ERR_DROP, "cskel_animation_md5_c::loadChannels: found '%s' instead of '}' in channel %i", bracket, i);
			return;
		}
		
		_channels.push_back(channel);
	}
}






static std::vector<cskel_animation_c*>	cm_animations;


void	CM_InitAnimations()
{
	Com_Printf("------- CM_InitAnimations -------\n");

	//TODO ?
}


void	CM_ShutdownAnimations()
{
	Com_Printf("------- CM_ShutdownAnimations -------\n");
	
	X_purge<std::vector<cskel_animation_c*> >(cm_animations);
	
	cm_animations.clear();
}


static cskel_animation_md5_c*	CM_GetFreeAnimation(const std::string &name)
{
	cskel_animation_md5_c *anim = new cskel_animation_md5_c(name);
	
	std::vector<cskel_animation_c*>::iterator ir = find(cm_animations.begin(), cm_animations.end(), static_cast<cskel_animation_c*>(NULL));
	
	if(ir != cm_animations.end())
		*ir = anim;
	else
		cm_animations.push_back(anim);
	
	return anim;
}




static cskel_animation_c*	CM_LoadAnimation(const std::string &name)
{
	char *buf = NULL;
	//char *data_p = NULL;
	
	//Com_Printf("loading '%s' ...\n", name.c_str());

	//
	// load the file
	//
	VFS_FLoad(name, (void **)&buf);
	if(!buf)
	{
		Com_Error(ERR_DROP, "CM_LoadAnimation: couldn't load '%s'", name.c_str());
		return false;
	}
	
	//data_p = buf;
	//std::string exp = (const char*)_buffer;
	
	cm_md5_anim = CM_GetFreeAnimation(name);
	
	cskel_animation_md5_grammar_t grammar;

	boost::spirit::parse_info<> info = boost::spirit::parse(buf, grammar, boost::spirit::space_p);
	
	if(!info.full)
		Com_Error(ERR_DROP, "CM_LoadAnimation: parsing failed for '%s'", name.c_str());
		
	/*
	Com_Parse(&data_p);	// skip ident
	
	int version = Com_ParseInt(&data_p);
	
	if(version != MD5_VERSION)
	{
		Com_Error(ERR_DROP, "CM_LoadAnimation: '%s' has wrong version number (%i should be %i)", name.c_str(), version, MD5_VERSION);
		return false;
	}
	
	Com_Parse(&data_p);	// skip "commandline"
	Com_Parse(&data_p);	// skip commandline string
	
	
	cskel_animation_md5_c *anim = CM_GetFreeAnimation(name);
	
	anim->loadChannels(&data_p);
	*/
	
	VFS_FFree(buf);
	
	//TODO update public animation info
	
	//ri.Com_Printf("CM_LoadAnimation: anim '%s' has %i channels\n", name.c_str(), _channels.size());
	
	return cm_md5_anim;
}





cskel_animation_c*	CM_RegisterAnimation(const std::string &name)
{
	if(!name.length())
	{	
		Com_Error(ERR_DROP, "CM_RegisterAnimation: empty name");
		return NULL;
	}
	
	for(std::vector<cskel_animation_c*>::iterator ir = cm_animations.begin(); ir != cm_animations.end(); ir++)
	{
		cskel_animation_c *anim = *ir;
		
		if(!anim)
			continue;
		
		if(X_strcaseequal(name.c_str(), anim->getName()))
		{
			//anim->setRegistrationSequence();
			return anim;
		}
	}

	return CM_LoadAnimation(name);
}

void	CM_FreeUnusedAnimations()
{
	/*
	for(std::vector<cskel_animation_c*>::iterator ir = r_animations.begin(); ir != r_animations.end(); ir++)
	{
		cskel_animation_c *anim = *ir;
		
		if(!anim)
			continue;
		
		if(anim->getRegistrationSequence() == cm_registration_sequence)
			continue;		// used this sequence
				
		delete anim;
		*ir = NULL;
	}
	*/
}
