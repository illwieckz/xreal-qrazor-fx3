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
#include "r_local.h"

// xreal --------------------------------------------------------------------

extern r_shader_c*		r_current_shader;
extern r_shader_stage_c*	r_current_stage;


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


void	R_Map_stc(char const* begin, char const *end)
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
