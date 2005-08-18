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

bool	R_ParseExpressionToAST(r_iterator_t begin, r_iterator_t end, r_expression_t &exp);
bool	R_ParseExpressionToAST(const std::string &str, r_expression_t &exp);
void	R_Map_stc(char const* begin, char const *end);

extern r_shader_c*		r_current_shader;
extern r_shader_stage_c*	r_current_stage;


void	R_ColorMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_COLORMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_DiffuseMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DIFFUSEMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_diffusemap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_BumpMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_BUMPMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_bumpmap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_SpecularMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_SPECULARMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_specularmap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_HeatHazeMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_HEATHAZEMAP;
	r_current_shader->setORFlags(SHADER_POSTPROCESS);
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_heathazemap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_LightMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_LIGHTMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_lightmap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_DeluxeMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DELUXEMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_deluxemap = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_ReflectionMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_REFLECTIONMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_RefractionMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_REFRACTIONMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_DispersionMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_DISPERSIONMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_LiquidMap_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type = SHADER_MATERIAL_STAGE_TYPE_LIQUIDMAP;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_Debug_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_DEBUG);
}

void	R_NoDraw_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_NODRAW);
}

void	R_NoShadows_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_NOSHADOWS);
}

void	R_NoSelfShadow_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_NOSELFSHADOW);
}

void	R_NoEnvMap_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_NOENVMAP);
}

void	R_SortSubview_sc(char const* begin, char const* end)
{
	r_expression_t sort;
	R_ParseExpressionToAST("-2.0", sort);
	
	r_current_shader->setSort(sort);
}

void	R_SortOpaque_sc(char const* begin, char const* end)
{
	r_expression_t sort;
	R_ParseExpressionToAST("-1.0", sort);
	
	r_current_shader->setSort(sort);
}

void	R_SortDecal_sc(char const* begin, char const* end)
{
	r_expression_t sort;
	R_ParseExpressionToAST("4.0", sort);
	
	r_current_shader->setSort(sort);
}

void	R_SortFar_sc(char const* begin, char const* end)
{
	r_expression_t sort;
	R_ParseExpressionToAST("0.0", sort);
	
	r_current_shader->setSort(sort);
}

void	R_SortMedium_sc(char const* begin, char const* end)
{
	r_expression_t sort;
	R_ParseExpressionToAST("1.0", sort);
	
	r_current_shader->setSort(sort);
}

void	R_SortClose_sc(char const* begin, char const* end)
{
	r_expression_t sort;
	R_ParseExpressionToAST("2.0", sort);
	
	r_current_shader->setSort(sort);
}

void	R_SortAlmostNearest_sc(char const* begin, char const* end)
{
	r_expression_t sort;
	R_ParseExpressionToAST("5.0", sort);
	
	r_current_shader->setSort(sort);
}

void	R_SortNearest_sc(char const* begin, char const* end)
{
	r_expression_t sort;
	R_ParseExpressionToAST("6.0", sort);
	
	r_current_shader->setSort(sort);
}

void	R_SortPostProcess_sc(char const* begin, char const* end)
{
	r_expression_t sort;
	R_ParseExpressionToAST("7.0", sort);
	
	r_current_shader->setSort(sort);
}

void	R_Sort_sc(char const* begin, char const* end)
{
	std::string	exp(begin, end);

	r_expression_t sort;
	
	if(R_ParseExpressionToAST(exp.begin(), exp.end(), sort))
	{
		r_current_shader->setSort(sort);
	}
	else
	{
		ri.Com_Printf("R_Sort_sc: parsing failed\n");
	}
}

void	R_TwoSided_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_TWOSIDED);
}

void	R_Translucent_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_TRANSLUCENT);
}

void	R_PolygonOffset_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_POLYGONOFFSET);
}

void	R_Discrete_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_DISCRETE);
}

void	R_ForceOpaque_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_FORCEOPAQUE);
}

void	R_LightFalloffImage_sc(char const* begin, char const* end)
{
	r_current_stage = new r_shader_stage_c();
	
	r_current_stage->type_light = SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_Z;
	
	R_Map_stc(begin, end);
	
	r_current_shader->stage_attenuationmap_z = r_current_stage;
	
	r_current_shader->stages.push_back(r_current_stage);
}

void	R_AmbientLight_sc(char const* begin, char const* end)
{
	r_current_shader->setLightType(SHADER_LIGHT_AMBIENT);
}

void	R_FogLight_sc(char const* begin, char const* end)
{
	r_current_shader->setLightType(SHADER_LIGHT_FOG);
}

void	R_DecalMacro_sc(char const* begin, char const* end)
{
	/*
		polygonoffset \
		discrete \
		noShadows \
		translucent \
		sort 1
	*/

	r_current_shader->setORFlags(SHADER_POLYGONOFFSET);
	r_current_shader->setORFlags(SHADER_DISCRETE);
	r_current_shader->setORFlags(SHADER_NOSHADOWS);
//	r_current_shader->setORFlags(SHADER_TRANSLUCENT);
	
	R_SortDecal_sc(begin, end);
}

void	R_DeformFlare_sc(char const* begin, char const* end)
{
	//TODO
	
	r_current_shader->setDeformType(SHADER_DEFORM_FLARE);
}

void	R_SurfaceparmAreaPortal_sc(char const* begin, char const* end)
{
	r_current_shader->setORFlags(SHADER_AREAPORTAL);
}
