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
		if(light->die < trap_cl->time)
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
	light->die = trap_cl->time + time;
}


void	CG_RunLights()
{
	int			i;
	cg_light_t	*light;

	
	for(i=0, light = cg_lights; i<MAX_REFLIGHTS ; i++, light++)
	{
		if(!light->radius.length())
			continue;
		
		if(light->die < trap_cl->time)
		{
			light->radius.clear();
			return;
		}
		
		light->radius[0] -= trap_cls->frametime * light->decay;
		light->radius[1] -= trap_cls->frametime * light->decay;
		light->radius[2] -= trap_cls->frametime * light->decay;
		
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
		//	trap_Com_Printf("CG_AddLights: %i is a spot light\n", i);
					
		r_light_t rlight;
		
		rlight.type	= clight->type;
		
		rlight.shader	= clight->shader;
		
		rlight.origin	= clight->origin;
		rlight.color	= clight->color;
		
		//rlight.radius_aabb	= clight->radius_aabb;
		//rlight.radius_value	= clight->radius_value;
		rlight.radius_aabb._maxs = clight->origin + clight->radius;
		rlight.radius_aabb._mins = clight->origin - clight->radius;
		//rlight.radius_aabb._mins.negate();
		rlight.radius_value = rlight.radius_aabb.radius();
		
		rlight.dir		= clight->dir;
		rlight.cone_inner	= clight->cone_inner;
		rlight.cone_outer	= clight->cone_outer;

		trap_R_AddLightToScene(rlight);
	}
}


void	CG_AddPacketLight(cg_entity_t *cent, entity_state_t *state)
{
	cg_light_t *light = CG_AllocLight(state->getNumber());
	
	light->color = state->color;
	
	if(light->color.isZero())
		light->color = color_white;
	
	//light->shader = trap_R_RegisterLight("lights/defaultProjectedLight");
	light->shader = trap_cl->light_precache[state->index_light];
	
	light->radius = state->lengths;
	
	switch(state->type)
	{
		case ET_LIGHT_POINT:
			light->type = LIGHT_POINT;
			light->dir.set(1, 0, 0);
			break;
		
		case ET_LIGHT_SPOT:
			light->type = LIGHT_SPOT;
			//trap_MSG_ReadDir(trap_net_message, light->dir);
			//light->cone_inner = trap_MSG_ReadFloat(trap_net_message);
			//light->cone_outer = trap_MSG_ReadFloat(trap_net_message);
			
			//trap_Com_Printf("light entity %i is a spot light\n", i);
			break;
		default:
			trap_Com_Error(ERR_DROP, "CG_ParseLight: bad light type %i\n", light->type);
	}

	for(int i=0; i<3; i++)
	{
		light->origin[i] = cent->prev.origin[i] + trap_cl->lerpfrac * (cent->current.origin[i] - cent->prev.origin[i]);
	}
	
	//light->origin = ent->current.origin;
	//Angles_ToVectors(ent->current.angles, fv, rv, upv);
	//Vector3_MA (light->origin, 18, fv, light->origin);
	//Vector3_MA (light->origin, 16, rv, light->origin);
	
	//light->radius = 200;// + (rand()&31);
	light->minlight = 32;
	
	light->die = trap_cl->time + 100;
}
*/



void	CG_AddLightEntity(const cg_entity_t *cent)
{
	r_entity_t rent;

	if(!cent->current.index_light)
		trap_Com_Error(ERR_DROP, "CG_AddLightEntity: bad light index");

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
			//trap_Com_DPrintf("adding omni-directional light ...\n");
			
			rent.radius = cent->current.vectors[0];
			
			rent.radius_aabb.clear();
			
			// compute bbox vertices in light space
			vec3_c vert0( cent->current.vectors[0][0], -cent->current.vectors[0][1], -cent->current.vectors[0][2]);
			vec3_c vert1( cent->current.vectors[0][0], -cent->current.vectors[0][1],  cent->current.vectors[0][2]);
			vec3_c vert2(-cent->current.vectors[0][0], -cent->current.vectors[0][1],  cent->current.vectors[0][2]);
			vec3_c vert3(-cent->current.vectors[0][0], -cent->current.vectors[0][1], -cent->current.vectors[0][2]);
	
			vec3_c vert4( cent->current.vectors[0][0],  cent->current.vectors[0][1], -cent->current.vectors[0][2]);
			vec3_c vert5( cent->current.vectors[0][0],  cent->current.vectors[0][1],  cent->current.vectors[0][2]);
			vec3_c vert6(-cent->current.vectors[0][0],  cent->current.vectors[0][1],  cent->current.vectors[0][2]);
			vec3_c vert7(-cent->current.vectors[0][0],  cent->current.vectors[0][1], -cent->current.vectors[0][2]);
			
			// rotate vertices in light space
			vert0.rotate(cent->current.quat);
			vert1.rotate(cent->current.quat);
			vert2.rotate(cent->current.quat);
			vert3.rotate(cent->current.quat);
			vert4.rotate(cent->current.quat);
			vert5.rotate(cent->current.quat);
			vert6.rotate(cent->current.quat);
			vert7.rotate(cent->current.quat);
			
			// transform vertices into world space and add them to the light world aabb
			rent.radius_aabb.addPoint(cent->current.origin + vert0);
			rent.radius_aabb.addPoint(cent->current.origin + vert1);
			rent.radius_aabb.addPoint(cent->current.origin + vert2);
			rent.radius_aabb.addPoint(cent->current.origin + vert3);
			rent.radius_aabb.addPoint(cent->current.origin + vert4);
			rent.radius_aabb.addPoint(cent->current.origin + vert5);
			rent.radius_aabb.addPoint(cent->current.origin + vert6);
			rent.radius_aabb.addPoint(cent->current.origin + vert7);
			
			rent.radius_value = rent.radius_aabb.radius();
			
			trap_R_AddLight(cent->current.getNumber(), rent, LIGHT_OMNI);
			break;
		}
				
		case ET_LIGHT_PROJ:
		{
			//trap_Com_DPrintf("adding projective light ...\n");
			
			rent.target = cent->current.vectors[0];
			rent.right = cent->current.vectors[1];
			rent.up = cent->current.vectors[2];
			
			rent.radius_aabb.clear();
			
			//vec3_c lt = rent.target - rent.origin;
			//vec3_c lr = rent.right - rent.origin;
			//vec3_c lu = rent.up - rent.origin;
			
			//rent.radius_aabb._maxs = rent.origin + vec3_c(300, 300, 300);
			//rent.radius_aabb._mins = rent.origin - vec3_c(300, 300, 300);
			
			//rent.radius_aabb.addPoint(cent->current.vectors[0]);
			//rent.radius_aabb.addPoint(cent->current.vectors[1]);
			//rent.radius_aabb.addPoint(cent->current.vectors[2]);
			
			rent.radius_aabb.addPoint(rent.origin + rent.target);
			
			rent.radius_aabb.addPoint(rent.origin + rent.right);
			rent.radius_aabb.addPoint(rent.origin - rent.right);
			
			rent.radius_aabb.addPoint(rent.origin + rent.up);
			rent.radius_aabb.addPoint(rent.origin - rent.up);
		
			rent.radius_value = rent.radius_aabb.radius();
			
			trap_R_AddLight(cent->current.getNumber(), rent, LIGHT_PROJ);
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
			}
			
			if(update)
			{
				rent.radius.lerp(cent->prev.vectors[0], cent->current.vectors[0], cg.frame_lerp);
		
				rent.radius_aabb.clear();
			
				// compute bbox vertices in light space
				vec3_c vert0( rent.radius[0], -rent.radius[1], -rent.radius[2]);
				vec3_c vert1( rent.radius[0], -rent.radius[1],  rent.radius[2]);
				vec3_c vert2(-rent.radius[0], -rent.radius[1],  rent.radius[2]);
				vec3_c vert3(-rent.radius[0], -rent.radius[1], -rent.radius[2]);
	
				vec3_c vert4( rent.radius[0],  rent.radius[1], -rent.radius[2]);
				vec3_c vert5( rent.radius[0],  rent.radius[1],  rent.radius[2]);
				vec3_c vert6(-rent.radius[0],  rent.radius[1],  rent.radius[2]);
				vec3_c vert7(-rent.radius[0],  rent.radius[1], -rent.radius[2]);
			
				// rotate vertices in light space
				vert0.rotate(rent.quat);
				vert1.rotate(rent.quat);
				vert2.rotate(rent.quat);
				vert3.rotate(rent.quat);
				vert4.rotate(rent.quat);
				vert5.rotate(rent.quat);
				vert6.rotate(rent.quat);
				vert7.rotate(rent.quat);
			
				// transform vertices into world space and add them to the light world aabb
				rent.radius_aabb.addPoint(rent.origin + vert0);
				rent.radius_aabb.addPoint(rent.origin + vert1);
				rent.radius_aabb.addPoint(rent.origin + vert2);
				rent.radius_aabb.addPoint(rent.origin + vert3);
				rent.radius_aabb.addPoint(rent.origin + vert4);
				rent.radius_aabb.addPoint(rent.origin + vert5);
				rent.radius_aabb.addPoint(rent.origin + vert6);
				rent.radius_aabb.addPoint(rent.origin + vert7);
			
				rent.radius_value = rent.radius_aabb.radius();
			
				trap_Com_DPrintf("updating omni-directional light ...\n");
				trap_R_UpdateLight(cent->current.getNumber(), rent, LIGHT_OMNI);
			}
			break;
		}
				
		case ET_LIGHT_PROJ:
		{
			//trap_Com_DPrintf("adding projective light ...\n");
			break;
		}
		
		default:
			break;
	}
}

void	CG_RemoveLightEntity(const cg_entity_t *cent)
{
	//trap_Com_DPrintf("removing light ...\n");

	trap_R_RemoveLight(cent->prev.getNumber());
}


