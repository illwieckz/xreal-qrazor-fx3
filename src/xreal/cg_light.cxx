/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
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
// xreal --------------------------------------------------------------------
#include "cg_local.h"


/*
static cg_light_t		cg_lights[MAX_REFLIGHTS];


void	CG_ClearLights()
{
	memset(cg_lights, 0, sizeof(cg_lights));
}

cg_light_t*	CG_AllocLight(int key)
{
	int		i;
	cg_light_t	*light;

	// first look for an exact key match
	if(key)
	{
		
		for (i=0, light = cg_lights; i<MAX_REFLIGHTS; i++, light++)
		{
			if(light->key == key)
			{
				memset(light, 0, sizeof(*light));
				light->key = key;
				return light;
			}
		}
	}

	// then look for anything else
	for(i=0, light = cg_lights; i<MAX_REFLIGHTS; i++, light++)
	{
		if(light->die < cgi.cl->time)
		{
			memset(light, 0, sizeof(*light));
			light->key = key;
			return light;
		}
	}

	light = &cg_lights[0];
	memset(light, 0, sizeof(*light));
	light->key = key;
	return light;
}


void	CG_NewLight(int key, const vec3_c &origin, float radius, float time) 
{
	cg_light_t	*light;

	light = CG_AllocLight(key);
	light->origin = origin;
	light->radius[0] = radius;
	light->radius[1] = radius;
	light->radius[2] = radius;
	light->die = cgi.cl->time + time;
}


void	CG_RunLights()
{
	int			i;
	cg_light_t	*light;

	
	for(i=0, light = cg_lights; i<MAX_REFLIGHTS ; i++, light++)
	{
		if(!light->radius.length())
			continue;
		
		if(light->die < cgi.cl->time)
		{
			light->radius.clear();
			return;
		}
		
		light->radius[0] -= cgi.cls->frametime * light->decay;
		light->radius[1] -= cgi.cls->frametime * light->decay;
		light->radius[2] -= cgi.cls->frametime * light->decay;
		
		if(light->radius.length() < 0)
			light->radius.clear();
	}
}


void	CG_AddLights()
{
	int		i;
	cg_light_t	*clight;


	for(i=0, clight = cg_lights; i<MAX_REFLIGHTS ; i++, clight++)
	{
		if(!clight->radius)
			continue;
		
		//if(light->type == LIGHT_SPOT)
		//	cgi.Com_Printf("CG_AddLights: %i is a spot light\n", i);
					
		r_light_t rlight;
		
		rlight.type	= clight->type;
		
		rlight.shader	= clight->shader;
		
		rlight.origin	= clight->origin;
		rlight.color	= clight->color;
		
		//rlight.radius_bbox	= clight->radius_bbox;
		//rlight.radius_value	= clight->radius_value;
		rlight.radius_bbox._maxs = clight->origin + clight->radius;
		rlight.radius_bbox._mins = clight->origin - clight->radius;
		//rlight.radius_bbox._mins.negate();
		rlight.radius_value = rlight.radius_bbox.radius();
		
		rlight.dir		= clight->dir;
		rlight.cone_inner	= clight->cone_inner;
		rlight.cone_outer	= clight->cone_outer;

		cgi.R_AddLightToScene(rlight);
	}
}


void	CG_AddPacketLight(cg_entity_t *cent, entity_state_t *state)
{
	cg_light_t *light = CG_AllocLight(state->getNumber());
	
	light->color = state->color;
	
	if(light->color.isZero())
		light->color = color_white;
	
	//light->shader = cgi.R_RegisterLight("lights/defaultProjectedLight");
	light->shader = cgi.cl->light_precache[state->index_light];
	
	light->radius = state->lengths;
	
	switch(state->type)
	{
		case ET_LIGHT_POINT:
			light->type = LIGHT_POINT;
			light->dir.set(1, 0, 0);
			break;
		
		case ET_LIGHT_SPOT:
			light->type = LIGHT_SPOT;
			//cgi.MSG_ReadDir(cgi.net_message, light->dir);
			//light->cone_inner = cgi.MSG_ReadFloat(cgi.net_message);
			//light->cone_outer = cgi.MSG_ReadFloat(cgi.net_message);
			
			//cgi.Com_Printf("light entity %i is a spot light\n", i);
			break;
		default:
			cgi.Com_Error(ERR_DROP, "CG_ParseLight: bad light type %i\n", light->type);
	}

	for(int i=0; i<3; i++)
	{
		light->origin[i] = cent->prev.origin[i] + cgi.cl->lerpfrac * (cent->current.origin[i] - cent->prev.origin[i]);
	}
	
	//light->origin = ent->current.origin;
	//Angles_ToVectors(ent->current.angles, fv, rv, upv);
	//Vector3_MA (light->origin, 18, fv, light->origin);
	//Vector3_MA (light->origin, 16, rv, light->origin);
	
	//light->radius = 200;// + (rand()&31);
	light->minlight = 32;
	
	light->die = cgi.cl->time + 100;
}
*/



void	CG_AddLightEntity(const cg_entity_t *cent)
{
	r_entity_t rent;

	if(!cent->current.index_light)
		cgi.Com_Error(ERR_DROP, "CG_AddLightEntity: bad light index");

	rent.custom_light = cg.light_precache[cent->current.index_light];
	
	rent.shader_parms[0] = cent->current.shaderparms[0];
	rent.shader_parms[1] = cent->current.shaderparms[1];
	rent.shader_parms[2] = cent->current.shaderparms[2];
	rent.shader_parms[3] = cent->current.shaderparms[3];
	rent.shader_parms[4] = cent->current.shaderparms[4];
	rent.shader_parms[5] = cent->current.shaderparms[5];
	rent.shader_parms[6] = cent->current.shaderparms[6];
	rent.shader_parms[7] = cent->current.shaderparms[7];
	
	rent.origin = cent->current.origin;
	rent.quat = cent->current.quat;
	
	rent.center = cent->current.origin2;
	
	rent.flags = cent->current.renderfx;

	switch(cent->current.type)
	{
		case ET_LIGHT_OMNI:
		{
			//cgi.Com_DPrintf("adding omni-directional light ...\n");
			
			rent.radius = cent->current.vectors[0];
			rent.radius_bbox._maxs = rent.origin + cent->current.vectors[0];
			rent.radius_bbox._mins = rent.origin - cent->current.vectors[0];
			rent.radius_bbox.rotate(cent->current.quat);
			rent.radius_value = rent.radius_bbox.radius();
			
			cgi.R_AddLight(cent->current.getNumber(), 0, rent, LIGHT_OMNI);
			break;
		}
				
		case ET_LIGHT_PROJ:
		{
			//cgi.Com_DPrintf("adding projective light ...\n");
			
			rent.target = cent->current.vectors[0];
			rent.right = cent->current.vectors[1];
			rent.up = cent->current.vectors[2];
			
			rent.radius_bbox.clear();
			
			//vec3_c lt = rent.target - rent.origin;
			//vec3_c lr = rent.right - rent.origin;
			//vec3_c lu = rent.up - rent.origin;
			
			//rent.radius_bbox._maxs = rent.origin + vec3_c(300, 300, 300);
			//rent.radius_bbox._mins = rent.origin - vec3_c(300, 300, 300);
			
			//rent.radius_bbox.addPoint(cent->current.vectors[0]);
			//rent.radius_bbox.addPoint(cent->current.vectors[1]);
			//rent.radius_bbox.addPoint(cent->current.vectors[2]);
			
			rent.radius_bbox.addPoint(rent.origin + rent.target);
			
			rent.radius_bbox.addPoint(rent.origin + rent.right);
			rent.radius_bbox.addPoint(rent.origin - rent.right);
			
			rent.radius_bbox.addPoint(rent.origin + rent.up);
			rent.radius_bbox.addPoint(rent.origin - rent.up);
		
			rent.radius_value = rent.radius_bbox.radius();
			
			cgi.R_AddLight(cent->current.getNumber(), 0, rent, LIGHT_PROJ);
			break;
		}
		
		default:
			break;
	}
}

void	CG_UpdateLightEntity(const cg_entity_t *cent)
{
	r_entity_t	rent;
	bool		update = false;
	
	CG_UpdateOrigin(cent, rent, update);
	
	CG_UpdateRotation(cent, rent, update);
	
// 	CG_UpdateModel(cent, rent, update);
	
 	CG_UpdateLightShader(cent, rent, update);
	
	CG_UpdateShaderParms(cent, rent, update);

// 	CG_UpdateFrame(cent, rent, update);	
	
	CG_UpdateRenderFXFlags(cent, rent, update);
	
	switch(cent->current.type)
	{
		case ET_LIGHT_OMNI:
		{
			if(cent->prev.vectors[0] != cent->current.vectors[0])
			{
				update = true;
				
				rent.radius.lerp(cent->prev.vectors[0], cent->current.vectors[0], cg.frame_lerp);
		
				rent.radius_bbox._maxs = rent.origin + rent.radius;
				rent.radius_bbox._mins = rent.origin - rent.radius;
				rent.radius_bbox.rotate(rent.quat);
				rent.radius_value = rent.radius_bbox.radius();
			}
			
			if(update)
			{
				cgi.Com_DPrintf("updating omni-directional light ...\n");
				cgi.R_UpdateLight(cent->current.getNumber(), 0, rent, LIGHT_OMNI);
			}
			break;
		}
				
		case ET_LIGHT_PROJ:
		{
			//cgi.Com_DPrintf("adding projective light ...\n");
			break;
		}
		
		default:
			break;
	}
}

void	CG_RemoveLightEntity(const cg_entity_t *cent)
{
	//cgi.Com_DPrintf("removing light ...\n");

	cgi.R_RemoveLight(cent->prev.getNumber());
}


