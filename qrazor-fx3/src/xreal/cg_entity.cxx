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


void	CG_BeginFrame(const frame_t &frame)
{
	if(cg.frame_running)
		cgi.Com_Error(ERR_DROP, "CG_BeginFrame: running frame");
		
	cg.frame_running = true;

	cg.frame_old	= cg.frame;
	cg.frame	= frame;
}

void	CG_AddEntity(int newnum, const entity_state_t *state)
{
	cg_entity_t *cent = &cg.entities[newnum];
	
	cent->prev = cent->current = *state;
	
	switch(cent->prev.type)
	{
		case ET_GENERIC:
		case ET_FUNC_STATIC:
			CG_AddGenericEntity(cent);
			break;
			
		case ET_LIGHT_OMNI:
		case ET_LIGHT_PROJ:
			CG_AddLightEntity(cent);
			break;
			
		default:
			break;
	}
}

void	CG_UpdateEntity(int parse_index, int newnum, const entity_state_t *state, bool changed)
{
	//cgi.Com_Printf("CG_UpdateEntity: %i %i is type %i\n", newnum, state.getNumber(), state.type);
	
	// update entity
	cg.entities_parse_index = parse_index;
	cg.entities_parse[(cg.entities_parse_index) & (MAX_PARSE_ENTITIES-1)] = *state;
	cg.frame.entities_num++;
	

	// check entity
	cg_entity_t *cent = &cg.entities[newnum];
	
	// some data changes will force no lerping
	if(state->index_model != cent->current.index_model || state->event == EV_TELEPORT)
	{
		cent->serverframe = -99;
	}
	
	if(cent->serverframe != cg.frame.serverframe - 1)
	{
		// wasn't in last update, so initialize some things
		
		// duplicate the current state so lerping doesn't hurt anything
		cent->prev = *state;
	}
	else
	{	// shuffle the last state to previous
		cent->prev = cent->current;
	}

	cent->serverframe = cg.frame.serverframe;
	cent->current = *state;
	
	if(changed)
	{
		switch(cent->prev.type)
		{
			case ET_GENERIC:
				CG_UpdateGenericEntity(cent);
				break;
		
			case ET_LIGHT_OMNI:
			case ET_LIGHT_PROJ:
				CG_UpdateLightEntity(cent);
				break;
			
			default:
				break;
		}
	}
}

void	CG_RemoveEntity(int parse_index, int oldnum, const entity_state_t *state)
{
	//entity_state_t *state = &cg.entities_parse[(cg.entities_parse_index) & (MAX_PARSE_ENTITIES-1)];
	
	cg_entity_t *cent = &cg.entities[oldnum];
	
	cent->prev = cent->current;
	cent->current = *state;
	
	switch(cent->prev.type)
	{
		case ET_GENERIC:
		case ET_FUNC_STATIC:
			CG_RemoveGenericEntity(cent);
			break;
	
		case ET_LIGHT_OMNI:
		case ET_LIGHT_PROJ:
			CG_RemoveLightEntity(cent);
			break;
			
		default:
			break;
	}
}


void	CG_EndFrame(int entities_num)
{
	if(!cg.frame_running)
		cgi.Com_Error(ERR_DROP, "CG_EndFrame: no running frame");
		
	cg.frame_running = false;

	if(cg.frame.valid)
		return;

	//TODO
	CG_CheckEntityEvents();
	CG_CheckPredictionError();
	CG_UpdateAnimationBuffer();
}


void	CG_SetFrame(r_entity_t &ent, const cg_entity_t *cent)
{
	// brush models can auto animate their frames
	int autoanim = 2*cgi.CL_GetTime()/1000;
	
	const uint_t& effects = cent->current.effects;
	
	if(effects & EF_ANIM01)
		ent.frame = autoanim & 1;
		
	else if(effects & EF_ANIM23)
		ent.frame = 2 + (autoanim & 1);
		
	else if(effects & EF_ANIM_ALL)
		ent.frame = autoanim;
	
	else if(effects & EF_ANIM_ALLFAST)
		ent.frame = cgi.CL_GetTime() / 100;
	
	else
		ent.frame = cent->current.frame;
}

void	CG_SetRotation(r_entity_t &rent, const cg_entity_t *cent)
{
	if(cent->current.effects & EF_ROTATE)
	{	
		// some bonus items auto-rotate
		float autorotate = anglemod(cgi.CL_GetTime()/10);
		vec3_c angles(0, autorotate, 0);
		rent.quat.fromAngles(angles);
	}
	else
	{			
		rent.quat.slerp(cent->prev.quat, cent->current.quat, cg.frame_lerp);
	}
}

#if 0
static void	CG_AddPacketEntities()
{
	r_entity_t	ent;		
	unsigned int	effects, renderfx;
	vec4_c		color;

	//cgi.Com_Printf("CG_AddPacketEntities: entities num %i\n", cg.frame.entities_num);

	for(int pnum = 0; pnum<cg.frame.entities_num; pnum++)
	{
		entity_state_t *state = &cg.entities_parse[(cg.frame.entities_parse_index + pnum)&(MAX_PARSE_ENTITIES-1)];
		cg_entity_t *cent = &cg.entities[state->getNumber()];

		effects = state->effects;
		renderfx = state->renderfx;
		
		ent.clear();

		CG_SetFrame(ent, cent);
		
		CG_SetRotation(ent, cent);


		// quad can do different things on client
		/*
		if(effects & EF_QUAD)
		{
			effects &= ~EF_QUAD;
			effects |= EF_COLOR_SHELL;
		}


		// color shells generate a seperate entity for the main model
		if(effects & EF_COLOR_SHELL)
		{
			//ent.alpha = 0.30;

			//TODO add shell shader
			//ent.skin = shell shader
			//V_AddEntity (&ent);
		}
		*/


		ent.frame_old = cent->prev.frame;
		ent.backlerp = 1.0 - cg.frame_lerp;
		
		/*
		if(renderfx & (RF_FRAMELERP|RF_PORTALSURFACE))
		{	
			// step origin discretely, because the frames
			// do the animation properly
			ent.origin = cent->current.origin;
			ent.origin_old = cent->current.origin_old;
		}
		else
		*/
		{	
			// interpolate origin
#if 1
			for(int i=0; i<3; i++)
			{
				ent.origin[i] = cent->prev.origin[i] + cg.frame_lerp * (cent->current.origin[i] - cent->prev.origin[i]);
				ent.origin2[i] = cent->prev.origin2[i] + cg.frame_lerp * (cent->current.origin2[i] - cent->prev.origin2[i]);
			}
#else
			//ent.origin = cent->current.origin;
#endif
		}

		//
		// create a new entity
		//
	
		//if(renderfx & RF_PORTALSURFACE)	//TODO
		//{
		//	if (state->modelindex3)
		//		ent.frame = state->
		//}
		//else
		
							
		// render effects (fullbright, translucent, etc)
		/*
		if((effects & EF_COLOR_SHELL))
			ent.flags = 0;	// renderfx go on color shell entity
		else
			ent.flags = renderfx;
		*/
		
		
		
				

		//
		// check 
		//
		if(state->getNumber() == cgi.playernum + 1)
		{
#if 0
			ent.flags |= RF_VIEWERMODEL;	// only draw from mirrors
#else
			continue;
#endif
		}
		
			
		ent.color = state->color;	
		
		
		//cgi.Com_Printf("CG_AddPacketEntities: entity type %i\n", state->_s.type);
		//cgi.Com_Printf("CG_AddPacketEntities: entity %i\n", state->_s.getNumber());
		
		//
		// check entity type
		//
		ent.type = state->type;
		
		switch(state->type)
		{
			case ET_GENERIC:
			default:
				CG_AddGenericEntity(ent, cent);
				continue;
			
			case ET_PLAYER:
				CG_AddPlayerEntities(ent, cent, effects, renderfx);
				continue;
				
			
			case ET_LIGHT_OMNI:
			case ET_LIGHT_PROJ:
			//	CG_AddLightEntity(ent, cent);
				continue;
		}
		
	
		/*
		
		ent.custom_shader = -1;
		ent.custom_skin = -1;	// never use a custom skin on others
		ent.shader_time = 0;

		//ent.color[0] = 1;
		//ent.color[1] = 1;
		//ent.color[2] = 1;
		ent.color[3] = 1;
		
		ent.flags = 0;
		
		*/
						
		// add automatic particle trails
		/*
		if((effects & ~EF_ROTATE))
		{
			light.origin = ent.origin;
			light.radius = 200;
			light.color.set(1, 1, 0);
		
			if(effects & EF_ROCKET)
			{
				CG_RocketTrail(cent->lerp_origin, ent.origin, cent);
				cgi.R_AddLightToScene(light);
			}
			else if(effects & EF_BLASTER)
			{
				light.radius = 300;
				light.color = color_green;
								
				CG_ParticleTrail(PART_BLASTER, cent->lerp_origin, ent.origin, color, 0.3);
				cgi.R_AddLightToScene(light);
			}
			
			else if(effects & EF_HYPERBLASTER)
			{
				CG_ParticleTrail(PART_BLASTER, cent->lerp_origin, ent.origin, color, 0.3);
				cgi.R_AddLightToScene(light);
			}
			else if(effects & EF_GIB)
			{
				CG_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);
			}
			else if(effects & EF_GRENADE)
			{
				CG_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);
			}
			else if(effects & EF_BFG)
			{
				static int bfg_lightramp[6] = {300, 400, 600, 300, 150, 75};

				if(effects & EF_ANIM_ALLFAST)
				{
					CG_BfgParticles (&ent);
					i = 200;
				}
				else
				{
					i = bfg_lightramp[state->_s.frame];
				}
				
				light.color.set(0, 1, 0);
				cgi.R_AddLightToScene(light);
			}
		}

		cent->lerp_origin = ent.origin;
		*/
	}
}
#endif






/*
===============
CG_AddEntities

Emits all entities, particles, and lights to the refresh
===============
*/
void	CG_AddEntities()
{
	//CG_AddPacketEntities();
	//CG_AddViewWeapon();
	//CG_AddBeams();
	//CG_AddExplosions();
	//CG_AddLasers();
	//CG_AddParticles();
	//CG_AddLights();
}



/*
===============
CG_GetEntitySoundOrigin

Called to get the sound spatialization origin
===============
*/
void	CG_GetEntitySoundOrigin(int ent, vec3_c &org)
{
	cg_entity_t	*cent;

	if(ent < 0 || ent >= MAX_ENTITIES)
		cgi.Com_Error(ERR_DROP, "CG_GetEntitySoundOrigin: bad ent");
	cent = &cg.entities[ent];
	
	org = cent->current.origin;

	// FIXME: bmodel issues...
}





void	CG_AddGenericEntity(const cg_entity_t *cent)
{
	//cgi.Com_DPrintf("adding generic entity ...\n");
	
	r_entity_t	rent;

	rent.type = cent->current.type;
	rent.model = cg.model_draw[cent->current.index_model];
//	rent.custom_shader = cg.light_precache[cent->current.index_light];
	
	rent.shader_parms[0] = cent->current.shaderparms[0];
	rent.shader_parms[1] = cent->current.shaderparms[1];
	rent.shader_parms[2] = cent->current.shaderparms[2];
	rent.shader_parms[3] = cent->current.shaderparms[3];
	rent.shader_parms[4] = cent->current.shaderparms[4];
	rent.shader_parms[5] = cent->current.shaderparms[5];
	rent.shader_parms[6] = cent->current.shaderparms[6];
	rent.shader_parms[7] = cent->current.shaderparms[7];
	
	rent.origin = cent->current.origin;
	rent.origin2 = cent->current.origin2;
	
	rent.frame = cent->current.frame;
	rent.frame_old = cent->current.frame;
	
	rent.quat = cent->current.quat;
	
	cgi.R_AddEntity(cent->current.getNumber(), rent);
}

void	CG_UpdateGenericEntity(const cg_entity_t *cent)
{
	//cgi.Com_DPrintf("adding generic entity ...\n");
	
	r_entity_t	rent;
	bool		update = false;

	rent.type = cent->current.type;
	
	if(cg.model_draw[cent->prev.index_model] != cg.model_draw[cent->current.index_model])	update = true;
	rent.model = cg.model_draw[cent->current.index_model];
	
	rent.shader_parms[0] = cent->current.shaderparms[0];
	rent.shader_parms[1] = cent->current.shaderparms[1];
	rent.shader_parms[2] = cent->current.shaderparms[2];
	rent.shader_parms[3] = cent->current.shaderparms[3];
	rent.shader_parms[4] = cent->current.shaderparms[4];
	rent.shader_parms[5] = cent->current.shaderparms[5];
	rent.shader_parms[6] = cent->current.shaderparms[6];
	rent.shader_parms[7] = cent->current.shaderparms[7];
	
	if(cent->prev.origin != cent->current.origin)	update = true;
	rent.origin.lerp(cent->prev.origin, cent->current.origin, cg.frame_lerp);
	rent.origin2 = cent->current.origin2;

	if(cent->prev.frame != cent->current.frame)	update = true;
	rent.frame = cent->current.frame;
	rent.frame_old = cent->prev.frame;
	
	if(cent->prev.quat != cent->current.quat)	update = true;
	rent.quat.slerp(cent->prev.quat, cent->current.quat, cg.frame_lerp);
	
	cgi.R_UpdateEntity(cent->current.getNumber(), rent, update);
}

void	CG_RemoveGenericEntity(const cg_entity_t *cent)
{
	//cgi.Com_DPrintf("removing generic entity ...\n");

	cgi.R_RemoveEntity(cent->prev.getNumber());
}


