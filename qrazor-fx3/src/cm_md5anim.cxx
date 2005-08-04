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

static int		cm_md5anim_frames_num = 0;
static int		cm_md5anim_joints_num = 0;
static int		cm_md5anim_components_num = 0;
static int		cm_md5anim_framerate = 0;

static void	CM_MD5_Version(int version)
{
	if(version != MD5_VERSION)
		Com_Error(ERR_DROP, "CM_MD5_Version: wrong version number (%i should be %i)", version, MD5_VERSION);
}

static void	CM_MD5Anim_FramesNum(int num)
{
	if(num <= 0)
		Com_Error(ERR_DROP, "CM_MD5Anim_FramesNum: animation has invalid frames number %i", num);

	cm_md5anim_frames_num = num;
}

static void	CM_MD5Anim_JointsNum(int num)
{
	if(num <= 0)
		Com_Error(ERR_DROP, "CM_MD5Anim_JointsNum: animation has invalid joints number %i", num);

	cm_md5anim_joints_num = num;
}

static void	CM_MD5Anim_FrameRate(int rate)
{
	if(rate <= 0)
		Com_Error(ERR_DROP, "CM_MD5Anim_FramesNum: animation has invalid framerate %i", rate);

	cm_md5anim_framerate = rate;
}

static void	CM_MD5Anim_ComponentsNum(int num)
{
	if(num <= 0)
		Com_Error(ERR_DROP, "CM_MD5Anim_FramesNum: animation has invalid components number %i", num);

	cm_md5anim_components_num = num;
}

class animation_md5_grammar_c : public boost::spirit::grammar<animation_md5_grammar_c>
{
public:
	template <typename ScannerT>
	struct definition
	{
        	definition(animation_md5_grammar_c const& self)
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
				=	boost::spirit::nocase_d[boost::spirit::str_p("numframes")] >> boost::spirit::int_p[&CM_MD5Anim_FramesNum]
				;
					
			numjoints
				=	boost::spirit::nocase_d[boost::spirit::str_p("numjoints")] >> boost::spirit::int_p[&CM_MD5Anim_JointsNum]
				;
				
			framerate
				=	boost::spirit::nocase_d[boost::spirit::str_p("framerate")] >> boost::spirit::int_p[&CM_MD5Anim_FrameRate]
				;
				
			numanimatedcomponents
				=	boost::spirit::nocase_d[boost::spirit::str_p("numanimatedcomponents")] >> boost::spirit::int_p[&CM_MD5Anim_ComponentsNum]
				;
				
			expression
				=	ident >>
					commandline >>
					numframes >>
					numjoints >>
					framerate >>
					numanimatedcomponents >>
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
						numjoints,
						framerate,
						numanimatedcomponents,
						
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};


animation_md5_c::animation_md5_c(const std::string &name, int frames, int joints, int components, int framerate)
:animation_c(name, frames, joints, components, framerate)
{
}

static std::map<std::string, animation_c*, strcasecmp_c>	cm_animations;

void	CM_InitAnimations()
{
	Com_Printf("------- CM_InitAnimations -------\n");

	//TODO ?
}


void	CM_ShutdownAnimations()
{
	Com_Printf("------- CM_ShutdownAnimations -------\n");
	
	//FIXME
//	X_purge(cm_animations);
	cm_animations.clear();
}

static animation_c*	CM_LoadAnimation(const std::string &name)
{
	char *buf = NULL;
	
	Com_Printf("loading '%s' ...\n", name.c_str());

	VFS_FLoad(name, (void **)&buf);
	if(!buf)
	{
		Com_Error(ERR_DROP, "CM_LoadAnimation: couldn't load '%s'", name.c_str());
		return false;
	}
	
	animation_md5_grammar_c grammar;

	boost::spirit::parse_info<> info = boost::spirit::parse(buf, grammar, boost::spirit::space_p);
	
	if(!info.full)
		Com_Error(ERR_DROP, "CM_LoadAnimation: parsing failed for '%s'", name.c_str());

	animation_md5_c *anim = new animation_md5_c
	(
		name,
		cm_md5anim_frames_num,
		cm_md5anim_joints_num,
		cm_md5anim_framerate,
		cm_md5anim_components_num
	);

	cm_animations.insert(std::make_pair(name, anim));
	
	VFS_FFree(buf);
		
	return anim;
}

animation_c*	CM_GetAnimationByName(const std::string &name)
{
	if(!name.length())
		Com_Error(ERR_DROP, "CM_GetAnimationByName: empty name");

	std::map<std::string, animation_c*>::const_iterator ir = cm_animations.find(name);

	if(ir != cm_animations.end())
		return ir->second;

	Com_Printf("CM_GetModelByName: couldn't find '%s'\n", name.c_str());
	return NULL;
}

animation_c*	CM_RegisterAnimation(const std::string &name)
{
	if(!name.length())
	{	
		Com_Error(ERR_DROP, "CM_RegisterAnimation: empty name");
		return NULL;
	}

	std::map<std::string, animation_c*>::const_iterator ir = cm_animations.find(name);

	if(ir != cm_animations.end())
		return ir->second;

	return CM_LoadAnimation(name);
}

void	CM_FreeUnusedAnimations()
{
	//TODO
}
