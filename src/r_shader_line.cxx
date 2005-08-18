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
#include <iostream>
#include <sstream>

//#define BOOST_SPIRIT_NO_TREE_NODE_COLLAPSING

// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"

// xreal --------------------------------------------------------------------



static void	R_SetReal(boost::spirit::tree_node<boost::spirit::node_val_data<r_iterator_t, r_node_data_t> > &node, r_iterator_t begin, r_iterator_t end)
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

static void	R_SetParm(boost::spirit::tree_node<boost::spirit::node_val_data<r_iterator_t, r_node_data_t> > &node, r_iterator_t begin, r_iterator_t end)
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

			table
				=	boost::spirit::access_node_d[boost::spirit::nocase_d[r_shader_table_symbols_p[&R_GetTable]]][&R_SetTable]
				;
			
			factor
				=	real
				|	parm
				|	(	boost::spirit::root_node_d[table] >>
						boost::spirit::inner_node_d[boost::spirit::ch_p('[') >> expression >> boost::spirit::ch_p(']')]
					)
				|	(boost::spirit::inner_node_d[boost::spirit::ch_p('(') >> expression >> boost::spirit::ch_p(')')])
				|	(boost::spirit::root_node_d[boost::spirit::ch_p('-')] >> factor)
				|	(boost::spirit::root_node_d[boost::spirit::ch_p('+')] >> factor)
				;
				
			term
				=	factor >> 
				*(	(boost::spirit::root_node_d[boost::spirit::ch_p('*')] >> factor)	|
					(boost::spirit::root_node_d[boost::spirit::ch_p('/')] >> factor)	|
					(boost::spirit::root_node_d[boost::spirit::ch_p('%')] >> factor)	|
					(boost::spirit::root_node_d[boost::spirit::str_p("<=")] >> factor)	|
					(boost::spirit::root_node_d[boost::spirit::ch_p('<')] >> factor)	|
					(boost::spirit::root_node_d[boost::spirit::str_p(">=")] >> factor)	|
					(boost::spirit::root_node_d[boost::spirit::ch_p('>')] >> factor)	|
					(boost::spirit::root_node_d[boost::spirit::str_p("==")] >> factor)	|
					(boost::spirit::root_node_d[boost::spirit::str_p("!=")] >> factor)	|
					(boost::spirit::root_node_d[boost::spirit::str_p("&&")] >> factor)	|
					(boost::spirit::root_node_d[boost::spirit::str_p("||")] >> factor)
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
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_TABLE> >			table;
 		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_FACTOR> >			factor;
 		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_TERM> >			term;
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_EXPRESSION> >		expression;
		
		boost::spirit::rule<ScannerT, boost::spirit::parser_context<>, boost::spirit::parser_tag<SHADER_GENERIC_RULE_EXPRESSION> > const&
		start() const { return expression; }
	};
};


// Tr3B - parses a single shader line expression and builds an Abstract Syntax Tree, which we can evaluate
//  in the renderer backend later
bool	R_ParseExpressionToAST(r_iterator_t begin, r_iterator_t end, r_expression_t &exp)
{
	r_shader_line_grammar_t		grammar;
	
	exp.str = std::string(begin, end);
	exp.info = boost::spirit::ast_parse<r_factory_t>
	(
		begin,
		end,
		grammar,
		boost::spirit::space_p ||
		boost::spirit::comment_p("/*", "*/") ||
		boost::spirit::comment_p("//")
	);
		
	return exp.info.full;
}

bool	R_ParseExpressionToAST(const std::string &str, r_expression_t &exp)
{
	return R_ParseExpressionToAST(str.begin(), str.end(), exp);
}

void	R_DumpASTToXML(const r_expression_t &exp)
{
#if 1
	//std::streambuf in;
	//std::ostream out(in.rdbuf());
	boost::spirit::tree_to_xml(std::cout, exp.info.trees, exp.str.c_str(), r_shader_generic_rules.map);
#endif
}


