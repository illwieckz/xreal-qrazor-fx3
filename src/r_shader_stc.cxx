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

bool	R_ParseExpressionToAST(r_iterator_t begin, r_iterator_t end, boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> &info);

bool	R_ParseExpressionToStringVector(const std::string &exp, std::vector<std::string> &v, bool skip_space = true);
bool	R_SplitExpression(const std::string &exp, std::string &exp0, std::string &exp1, bool skip_space = true);
bool	R_SplitExpression(const std::string &exp, std::string &exp0, std::string &exp1, std::string &exp2, bool skip_space = true);

void	R_Map_stc(char const* begin, char const *end);

extern r_shader_c*		r_current_shader;
extern r_shader_stage_c*	r_current_stage;


void	R_If_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> condition;
	
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), condition))
	{
		r_current_stage->condition	= condition;
	}
	else
	{
		ri.Com_Printf("R_If_stc: parsing failed\n");
	}
}

void	R_Blend_stc(char const* begin, char const *end)
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

void	R_BlendAdd_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_BLEND;
	
	r_current_stage->blend_src = GL_ONE;
	r_current_stage->blend_dst = GL_ONE;
}

void	R_BlendBlend_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_BLEND;
	
	r_current_stage->blend_src = GL_SRC_ALPHA;
	r_current_stage->blend_dst = GL_ONE_MINUS_SRC_ALPHA;
}

void	R_BlendFilter_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_BLEND;
	
	r_current_stage->blend_src = GL_DST_COLOR;
	r_current_stage->blend_dst = GL_ZERO;
}

void	R_Linear_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_NOMIPMAPS;
}

void	R_NoPicmip_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_NOPICMIP;
}

void	R_ZeroClamp_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_ZEROCLAMP;
}

void	R_Clamp_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_CLAMP;
}

void	R_MaskAlpha_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_MASKALPHA;
}

void	R_MaskColor_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_MASKCOLOR;
}

void	R_MaskDepth_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_MASKDEPTH;
}

void	R_ForceHighQuality_stc(char const* begin, char const *end)
{
	r_current_stage->flags |= SHADER_STAGE_FORCEHIGHQUALITY;
}

void	R_VideoMap_stc(char const* begin, char const *end)
{
	r_current_stage->image_name = std::string(begin, end);
	r_current_stage->flags |= SHADER_STAGE_VIDEOMAP;
}

void	R_Colored_stc(char const* begin, char const *end)
{
	r_current_stage->rgb_gen = SHADER_RGB_GEN_ENTITY;
	
	r_current_stage->alpha_gen = SHADER_ALPHA_GEN_ENTITY;
}

void	R_RGBA_stc(char const* begin, char const *end)
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

void	R_RGB_stc(char const* begin, char const *end)
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

void	R_Red_stc(char const* begin, char const *end)
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

void	R_Green_stc(char const* begin, char const *end)
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

void	R_Blue_stc(char const* begin, char const *end)
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

void	R_Alpha_stc(char const* begin, char const *end)
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

void	R_AlphaTest_stc(char const* begin, char const *end)
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

void	R_Scale3D_stc(char const* begin, char const *end)
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

void	R_Scale_stc(char const* begin, char const *end)
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

void	R_CenterScale_stc(char const* begin, char const *end)
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

void	R_Scroll_stc(char const* begin, char const *end)
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

void	R_Rotate_stc(char const* begin, char const *end)
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

void	R_Translate_stc(char const* begin, char const *end)
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

void	R_BumpScale_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> bump_scale;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), bump_scale))
		ri.Com_Printf("R_BumpScale_stc: parsing failed\n");
	
	r_current_stage->bump_scale = bump_scale;
}

void	R_HeightScale_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> height_scale;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), height_scale))
		ri.Com_Printf("R_HeightScale_stc: parsing failed\n");
	
	r_current_stage->height_scale = height_scale;
}

void	R_HeightBias_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> height_bias;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), height_bias))
		ri.Com_Printf("R_HeightBias_stc: parsing failed\n");
	
	r_current_stage->height_bias = height_bias;
}

void	R_SpecularExponent_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> specular_exponent;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), specular_exponent))
		ri.Com_Printf("R_SpecularExponent_stc: parsing failed\n");
	
	r_current_stage->specular_exponent = specular_exponent;
}

void	R_RefractionIndex_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> refraction_index;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), refraction_index))
		ri.Com_Printf("R_RefractionIndex_stc: parsing failed\n");
	
	r_current_stage->refraction_index = refraction_index;
}

void	R_FresnelPower_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> fresnel_power;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), fresnel_power))
		ri.Com_Printf("R_FresnelPower_stc: parsing failed\n");
	
	r_current_stage->fresnel_power = fresnel_power;
}

void	R_FresnelScale_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> fresnel_scale;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), fresnel_scale))
		ri.Com_Printf("R_FresnelScale_stc: parsing failed\n");
	
	r_current_stage->fresnel_scale = fresnel_scale;
}

void	R_FresnelBias_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> fresnel_bias;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), fresnel_bias))
		ri.Com_Printf("R_FresnelBias_stc: parsing failed\n");
	
	r_current_stage->fresnel_bias = fresnel_bias;
}

void	R_EtaDelta_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> eta_delta;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), eta_delta))
		ri.Com_Printf("R_EtaDelta_stc: parsing failed\n");
	
	r_current_stage->eta_delta = eta_delta;
}

void	R_Eta_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> eta;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), eta))
		ri.Com_Printf("R_Eta_stc: parsing failed\n");
	
	r_current_stage->eta = eta;
}

void	R_DeformMagnitude_stc(char const* begin, char const *end)
{
	std::string	exp(begin, end);

	boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> mag;
	
	if(!R_ParseExpressionToAST(exp.begin(), exp.end(), mag))
		ri.Com_Printf("R_DeformMagnitude_stc: parsing failed\n");
	
	r_current_stage->deform_magnitude = mag;
}

void	R_StageColorMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_COLORMAP;
}

void	R_StageDiffuseMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DIFFUSEMAP;
}

void	R_StageBumpMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_BUMPMAP;
}

void	R_StageSpecularMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_SPECULARMAP;
}

void	R_StageHeatHazeMap_stc(char const* begin, char const *end)
{
	r_current_shader->setORFlags(SHADER_POSTPROCESS);
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_HEATHAZEMAP;
}

void	R_StageLightMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_LIGHTMAP;
}

void	R_StageDeluxeMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DELUXEMAP;
}

void	R_StageReflectionMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_REFLECTIONMAP;
}

void	R_StageRefractionMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_REFRACTIONMAP;
}

void	R_StageDispersionMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DISPERSIONMAP;
}

void	R_StageSkyBoxMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_SKYBOXMAP;
}

void	R_StageLiquidMap_stc(char const* begin, char const *end)
{
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_LIQUIDMAP;
}

void	R_StageAttenuationMapXY_stc(char const* begin, char const *end)
{
	r_current_stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY;
}

void	R_StageAttenuationMapZ_stc(char const* begin, char const *end)
{
	r_current_stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_Z;
}

void	R_StageAttenuationMapXYZ_stc(char const* begin, char const *end)
{
	r_current_stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XYZ;
}

void	R_StageAttenuationMapCube_stc(char const* begin, char const *end)
{
	r_current_stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_CUBE;
}

