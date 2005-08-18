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
	r_light_t rlight;

	if(!cent->current.index_light)
		trap_Com_Error(ERR_DROP, "CG_AddLightEntity: bad light index");

	rlight.custom_light = cg.light_precache[cent->current.index_light];
	
	rlight.origin = cent->current.origin;
	rlight.quat = cent->current.quat;
	
	rlight.center = cent->current.origin2;

	rlight.flags = cent->current.renderfx;

	for(int i=0; i<MAX_SHADERPARMS; i++)
		rlight.shader_parms[i] = cent->current.shader_parms[i];
	
	/*
	if(cent->current.index_sound)
	{
		trap_S_StartLoopSound
		(
			cent->current.origin, 
			cent->current.velocity_linear, 
			cent->current.getNumber(), 
			CHAN_AUTO, 
			trap_S_RegisterSound(trap_CL_GetConfigString(CS_SOUNDS + cent->current.index_sound))
		);
	}
	*/

	switch(cent->current.type)
	{
		case ET_LIGHT_OMNI:
		{
			//trap_Com_DPrintf("adding omni-directional light ...\n");
			
			rlight.type = LIGHT_OMNI;

			rlight.radius = cent->current.vectors[0];
			
			rlight.radius_aabb.clear();
			
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
			rlight.radius_aabb.addPoint(cent->current.origin + vert0);
			rlight.radius_aabb.addPoint(cent->current.origin + vert1);
			rlight.radius_aabb.addPoint(cent->current.origin + vert2);
			rlight.radius_aabb.addPoint(cent->current.origin + vert3);
			rlight.radius_aabb.addPoint(cent->current.origin + vert4);
			rlight.radius_aabb.addPoint(cent->current.origin + vert5);
			rlight.radius_aabb.addPoint(cent->current.origin + vert6);
			rlight.radius_aabb.addPoint(cent->current.origin + vert7);
			
			rlight.radius_value = rlight.radius_aabb.radius();
			
			trap_R_AddDeltaLight(cent->current.getNumber(), rlight);
			break;
		}
				
		case ET_LIGHT_PROJ:
		{
			//trap_Com_DPrintf("adding projective light ...\n");

			rlight.type = LIGHT_PROJ;
			
			rlight.target = cent->current.vectors[0];
			rlight.right = cent->current.vectors[1];
			rlight.up = cent->current.vectors[2];
			
			rlight.target.rotate(cent->current.quat);
			rlight.right.rotate(cent->current.quat);
			rlight.up.rotate(cent->current.quat);
			
			rlight.radius_aabb.clear();
			
			rlight.radius_aabb.addPoint(rlight.origin + rlight.target);
			
			rlight.radius_aabb.addPoint(rlight.origin + rlight.right);
			rlight.radius_aabb.addPoint(rlight.origin - rlight.right);
			
			rlight.radius_aabb.addPoint(rlight.origin + rlight.up);
			rlight.radius_aabb.addPoint(rlight.origin - rlight.up);
		
			rlight.radius_value = rlight.radius_aabb.radius();
			
			trap_R_AddDeltaLight(cent->current.getNumber(), rlight);
			break;
		}
		
		default:
			break;
	}
}

void	CG_UpdateLightEntity(const cg_entity_t *cent)
{
	r_light_t	rlight;
	bool		update = false;
	
	CG_UpdateOrigin(cent, rlight, update);
	
	CG_UpdateRotation(cent, rlight, update);

	CG_UpdateRenderFXFlags(cent, rlight, update);

	CG_UpdateShaderParms(cent, rlight, update);
	
 	CG_UpdateLightShader(cent, rlight, update);
	
	switch(cent->current.type)
	{
		case ET_LIGHT_OMNI:
		{
			rlight.type = LIGHT_OMNI;

			if(cent->prev.vectors[0] != cent->current.vectors[0])
			{
				update = true;
			}
			
			if(update)
			{
				rlight.radius.lerp(cent->prev.vectors[0], cent->current.vectors[0], cg.frame_lerp);
		
				rlight.radius_aabb.clear();
			
				// compute bbox vertices in light space
				vec3_c vert0( rlight.radius[0], -rlight.radius[1], -rlight.radius[2]);
				vec3_c vert1( rlight.radius[0], -rlight.radius[1],  rlight.radius[2]);
				vec3_c vert2(-rlight.radius[0], -rlight.radius[1],  rlight.radius[2]);
				vec3_c vert3(-rlight.radius[0], -rlight.radius[1], -rlight.radius[2]);
	
				vec3_c vert4( rlight.radius[0],  rlight.radius[1], -rlight.radius[2]);
				vec3_c vert5( rlight.radius[0],  rlight.radius[1],  rlight.radius[2]);
				vec3_c vert6(-rlight.radius[0],  rlight.radius[1],  rlight.radius[2]);
				vec3_c vert7(-rlight.radius[0],  rlight.radius[1], -rlight.radius[2]);
			
				// rotate vertices in light space
				vert0.rotate(rlight.quat);
				vert1.rotate(rlight.quat);
				vert2.rotate(rlight.quat);
				vert3.rotate(rlight.quat);
				vert4.rotate(rlight.quat);
				vert5.rotate(rlight.quat);
				vert6.rotate(rlight.quat);
				vert7.rotate(rlight.quat);
			
				// transform vertices into world space and add them to the light world aabb
				rlight.radius_aabb.addPoint(rlight.origin + vert0);
				rlight.radius_aabb.addPoint(rlight.origin + vert1);
				rlight.radius_aabb.addPoint(rlight.origin + vert2);
				rlight.radius_aabb.addPoint(rlight.origin + vert3);
				rlight.radius_aabb.addPoint(rlight.origin + vert4);
				rlight.radius_aabb.addPoint(rlight.origin + vert5);
				rlight.radius_aabb.addPoint(rlight.origin + vert6);
				rlight.radius_aabb.addPoint(rlight.origin + vert7);
			
				rlight.radius_value = rlight.radius_aabb.radius();
			
				trap_Com_DPrintf("updating omni-directional light ...\n");
				trap_R_UpdateDeltaLight(cent->current.getNumber(), rlight);
			}
			break;
		}
				
		case ET_LIGHT_PROJ:
		{
			rlight.type = LIGHT_PROJ;

			rlight.target.lerp(cent->prev.vectors[0], cent->current.vectors[0], cg.frame_lerp);
			rlight.right.lerp(cent->prev.vectors[1], cent->current.vectors[1], cg.frame_lerp);
			rlight.up.lerp(cent->prev.vectors[2], cent->current.vectors[2], cg.frame_lerp);
			
			rlight.target.rotate(rlight.quat);
			rlight.right.rotate(rlight.quat);
			rlight.up.rotate(rlight.quat);
			
			rlight.radius_aabb.clear();
			
			rlight.radius_aabb.addPoint(rlight.origin + rlight.target);
			
			rlight.radius_aabb.addPoint(rlight.origin + rlight.right);
			rlight.radius_aabb.addPoint(rlight.origin - rlight.right);
			
			rlight.radius_aabb.addPoint(rlight.origin + rlight.up);
			rlight.radius_aabb.addPoint(rlight.origin - rlight.up);
		
			rlight.radius_value = rlight.radius_aabb.radius();
			
			trap_Com_DPrintf("updating projective light ...\n");
			trap_R_UpdateDeltaLight(cent->current.getNumber(), rlight);
			break;
		}
		
		default:
			break;
	}
}

void	CG_RemoveLightEntity(const cg_entity_t *cent)
{
	//trap_Com_DPrintf("removing light ...\n");

	trap_R_RemoveDeltaLight(cent->prev.getNumber());
}


