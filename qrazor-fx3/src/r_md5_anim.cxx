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
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"

// xreal --------------------------------------------------------------------

static r_md5_animation_c*	r_md5anim = NULL;
static r_skel_channel_t*	r_md5anim_channel = NULL;
static r_skel_frame_t*		r_md5anim_frame = NULL;

static int		r_md5anim_joints_num = 0;
static int		r_md5anim_channels_counter = 0;

static int		r_md5anim_frames_num = 0;
static int		r_md5anim_frames_counter = 0;

static int		r_md5anim_components_num = 0;
static int		r_md5anim_framerate = 0;

static float		r_md5anim_float0;
static float		r_md5anim_float1;
static float		r_md5anim_float2;



static void	R_MD5Anim_Version(int version)
{
	if(version != MD5_VERSION)
		Com_Error(ERR_DROP, "R_MD5Anim_Version: wrong version number (%i should be %i)", version, MD5_VERSION);
}

static void	R_MD5Anim_JointsNum(int num)
{
	if(num <= 0)
		ri.Com_Error(ERR_DROP, "R_MD5Anim_JointsNum: animation has invalid joints number %i", num);

	r_md5anim_joints_num = num;
	
	for(int i=0; i<num; i++)
	{
		r_md5anim->_channels.push_back(new r_skel_channel_t());
	}
}

static void	R_MD5Anim_FramesNum(int num)
{
	if(num <= 0)
		ri.Com_Error(ERR_DROP, "R_MD5Anim_FramesNum: animation has invalid frames number %i", num);

	r_md5anim_frames_num = num;
	
	for(int i=0; i<num; i++)
	{
		r_md5anim->_frames.push_back(new r_skel_frame_t());
	}
}

static void	R_MD5Anim_FrameRate(int rate)
{
	if(rate <= 0)
		ri.Com_Error(ERR_DROP, "R_MD5Anim_FramesNum: animation has invalid framerate %i", rate);

	r_md5anim_framerate = rate;
}

static void	R_MD5Anim_ComponentsNum(int num)
{
	if(num <= 0)
		ri.Com_Error(ERR_DROP, "R_MD5Anim_FramesNum: animation has invalid components number %i", num);

	r_md5anim_components_num = num;
}

static void	R_MD5Anim_CurrentChannel1(char const* begin, char const* end)
{
	r_md5anim_channel = r_md5anim->_channels.at(r_md5anim_channels_counter);
	r_md5anim_channel->name = std::string(begin, end);
	
//	ri.Com_Printf("parsing channel '%s' ...\n", r_md5anim_channel->name.c_str());
}

static void	R_MD5Anim_ChannelParentIndex(int parent_index)
{
	r_md5anim_channel->parent_index = parent_index;
}

static void	R_MD5Anim_ChannelComponentsBits(int bits)
{
	r_md5anim_channel->components_bits = bits;
}

static void	R_MD5Anim_ChannelComponentsOffset(int ofs)
{
//	if(ofs+6 > r_md5anim_components_num)
//		ri.Com_Error(ERR_DROP, "R_MD5Anim_ChannelComponentsOffset: animation has invalid components offset: %i+6 > %i", ofs+6, r_md5anim_components_num);

	r_md5anim_channel->components_offset = ofs;
}


static void	R_MD5Anim_CurrentChannel2(char const)
{
	r_md5anim_channel = r_md5anim->_channels.at(r_md5anim_channels_counter);
}

static void	R_MD5Anim_IncChannelsCounter(char const* begin, char const* end)
{
	r_md5anim_channels_counter++;
}

static void	R_MD5Anim_ResetChannelsCounter(char const)
{
	r_md5anim_channels_counter = 0;
}

static void	R_MD5Anim_CheckChannelsCounter(char const)
{
	if(r_md5anim_channels_counter != r_md5anim_joints_num)
		ri.Com_Error(ERR_DROP, "R_MD5Anim_CheckChannelsCounter: bad channels counter %i should be %i", r_md5anim_channels_counter, r_md5anim_joints_num);
}


static void	R_MD5Anim_CurrentFrame(char const)
{
	r_md5anim_frame = r_md5anim->_frames.at(r_md5anim_frames_counter);
}

static void	R_MD5Anim_SetCurrentFrameByIndex(int idx)
{
	r_md5anim_frame = r_md5anim->_frames.at(idx);
}

static void	R_MD5Anim_ResetFramesCounter(char const)
{
	r_md5anim_frames_counter = 0;
}

static void	R_MD5Anim_IncFramesCounter(char const* begin, char const* end)
{
	r_md5anim_frames_counter++;
}

static void	R_MD5Anim_CheckFramesCounter(char const)
{
	if(r_md5anim_frames_counter != r_md5anim_frames_num)
		ri.Com_Error(ERR_DROP, "R_MD5Anim_CheckFramesCounter: bad frames counter %i should be %i", r_md5anim_frames_counter, r_md5anim_frames_num);
		
	r_md5anim_frames_counter = 0;
}

static void	R_MD5Anim_MinsX(float f)
{
	r_md5anim_frame->bbox._mins[0] = f;
}

static void	R_MD5Anim_MinsY(float f)
{
	r_md5anim_frame->bbox._mins[1] = f;
}

static void	R_MD5Anim_MinsZ(float f)
{
	r_md5anim_frame->bbox._mins[2] = f;
}

static void	R_MD5Anim_MaxsX(float f)
{
	r_md5anim_frame->bbox._maxs[0] = f;
}

static void	R_MD5Anim_MaxsY(float f)
{
	r_md5anim_frame->bbox._maxs[1] = f;
}

static void	R_MD5Anim_MaxsZ(float f)
{
	r_md5anim_frame->bbox._maxs[2] = f;
}


static void	R_MD5Anim_ChannelOrigin(char const)
{
	r_md5anim_channel->origin.set(r_md5anim_float0, r_md5anim_float1, r_md5anim_float2);
}

static void	R_MD5Mesh_ChannelQuaternion(char const)
{
	r_md5anim_channel->quat.set(r_md5anim_float0, r_md5anim_float1, r_md5anim_float2);
//	r_md5anim_channel->quat.normalize();
}


static void	R_MD5Anim_FramePushComponent(float f)
{
	r_md5anim_frame->components.push_back(f);
}

static void	R_MD5Anim_CheckComponentsNumber(char const)
{
	if((int)r_md5anim_frame->components.size() != r_md5anim_components_num)
		ri.Com_Error(ERR_DROP, "R_MD5Anim_CheckComponentsNumber: bad components number in frame %i (%i should be %i)",	r_md5anim_frames_counter, 
																r_md5anim_frame->components.size(), 
																r_md5anim_components_num);
}


static void	R_MD5Anim_Float0(float f)
{
	r_md5anim_float0 = f;
}

static void	R_MD5Anim_Float1(float f)
{
	r_md5anim_float1 = f;
}

static void	R_MD5Anim_Float2(float f)
{
	r_md5anim_float2 = f;
}


struct r_md5_animation_grammar_t : public boost::spirit::grammar<r_md5_animation_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(r_md5_animation_grammar_t const& self)
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
				=	boost::spirit::str_p("MD5Version") >> boost::spirit::int_p[&R_MD5Anim_Version]
				;
				
			commandline
				=	boost::spirit::str_p("commandline") >>
					boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')] >>
					boost::spirit::ch_p('\"')
				;
			
			numframes
				=	boost::spirit::nocase_d[boost::spirit::str_p("numframes")] >> boost::spirit::int_p[&R_MD5Anim_FramesNum]
				;
					
			numjoints
				=	boost::spirit::nocase_d[boost::spirit::str_p("numjoints")] >> boost::spirit::int_p[&R_MD5Anim_JointsNum]
				;
				
			framerate
				=	boost::spirit::nocase_d[boost::spirit::str_p("framerate")] >> boost::spirit::int_p[&R_MD5Anim_FrameRate]
				;
				
			numanimatedcomponents
				=	boost::spirit::nocase_d[boost::spirit::str_p("numanimatedcomponents")] >> boost::spirit::int_p[&R_MD5Anim_ComponentsNum]
				;
				
			hierarchy
				=	boost::spirit::str_p("hierarchy") >> 
					boost::spirit::ch_p('{')[&R_MD5Anim_ResetChannelsCounter] >> 
					+channel[&R_MD5Anim_IncChannelsCounter] >>
					boost::spirit::ch_p('}')[&R_MD5Anim_CheckChannelsCounter]
				;
				
			channel
				=	boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')][&R_MD5Anim_CurrentChannel1] >>
					boost::spirit::ch_p('\"') >>
					boost::spirit::int_p[&R_MD5Anim_ChannelParentIndex] >>
					boost::spirit::int_p[&R_MD5Anim_ChannelComponentsBits] >>
					boost::spirit::int_p[&R_MD5Anim_ChannelComponentsOffset]
				;
				
			bounds
				=	boost::spirit::str_p("bounds") >>
					boost::spirit::ch_p('{')[&R_MD5Anim_ResetFramesCounter] >> 
					+aabb[&R_MD5Anim_IncFramesCounter] >>
					boost::spirit::ch_p('}')[&R_MD5Anim_CheckFramesCounter]
				;
				
			aabb
				=	boost::spirit::ch_p('(')[&R_MD5Anim_CurrentFrame] >>
					boost::spirit::real_p[&R_MD5Anim_MinsX] >>
					boost::spirit::real_p[&R_MD5Anim_MinsY] >>
					boost::spirit::real_p[&R_MD5Anim_MinsZ] >>
					boost::spirit::ch_p(')') >>
					boost::spirit::ch_p('(') >>
					boost::spirit::real_p[&R_MD5Anim_MaxsX] >>
					boost::spirit::real_p[&R_MD5Anim_MaxsY] >>
					boost::spirit::real_p[&R_MD5Anim_MaxsZ] >>
					boost::spirit::ch_p(')')
				;
				
			baseframe
				=	boost::spirit::str_p("baseframe") >> 
					boost::spirit::ch_p('{')[&R_MD5Anim_ResetChannelsCounter] >> 
					+basechannel[&R_MD5Anim_IncChannelsCounter] >>
					boost::spirit::ch_p('}')[&R_MD5Anim_CheckChannelsCounter]
				;
				
			basechannel
				=	boost::spirit::ch_p('(')[&R_MD5Anim_CurrentChannel2] >>
					boost::spirit::real_p[&R_MD5Anim_Float0] >>
					boost::spirit::real_p[&R_MD5Anim_Float1] >>
					boost::spirit::real_p[&R_MD5Anim_Float2] >>
					boost::spirit::ch_p(')')[&R_MD5Anim_ChannelOrigin] >>
					boost::spirit::ch_p('(') >>
					boost::spirit::real_p[&R_MD5Anim_Float0] >>
					boost::spirit::real_p[&R_MD5Anim_Float1] >>
					boost::spirit::real_p[&R_MD5Anim_Float2] >>
					boost::spirit::ch_p(')')[&R_MD5Mesh_ChannelQuaternion]
				;
				
			frames
				=	+frame[&R_MD5Anim_IncFramesCounter]
				;
				
			frame
				=	boost::spirit::str_p("frame") >>
					boost::spirit::int_p[&R_MD5Anim_SetCurrentFrameByIndex] >>
					boost::spirit::ch_p('{') >>
					+boost::spirit::real_p[&R_MD5Anim_FramePushComponent] >>
					boost::spirit::ch_p('}')[&R_MD5Anim_CheckComponentsNumber]
				;
				
			expression
				=	ident >>
					commandline >>
					numframes >>
					numjoints >>
					framerate >>
					numanimatedcomponents >>
					hierarchy >>
					bounds >>
					baseframe >>
					frames
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
						hierarchy,
							channel,
						bounds,
							aabb,
						baseframe,
							basechannel,
						frames,
							frame,
						
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};


r_md5_animation_c::r_md5_animation_c(const std::string &name, byte *buffer, uint_t buffer_size)
:r_skel_animation_c(name, buffer, buffer_size)
{
	//ri.Com_Printf("r_md5_animation_c::ctor: %s\n", name.c_str());
}

r_md5_animation_c::~r_md5_animation_c()
{
}

void	r_md5_animation_c::load()
{
	r_md5anim = this;

	std::string exp = (const char*)_buffer;
	r_md5_animation_grammar_t grammar;

	boost::spirit::parse_info<> info = boost::spirit::parse
	(
		exp.c_str(),
		grammar,
		boost::spirit::space_p ||
		boost::spirit::comment_p("/*", "*/") ||
		boost::spirit::comment_p("//")
	);
	
	if(!info.full)
		ri.Com_Error(ERR_DROP, "r_md5_animation_c::load: parsing failed for '%s'", getName());
}
