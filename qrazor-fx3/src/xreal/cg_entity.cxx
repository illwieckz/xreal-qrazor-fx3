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
				//CG_UpdateGenericEntity(cent);
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



void	CG_UpdateOrigin(const cg_entity_t *cent, r_entity_t &rent, bool &update)
{
	if(cent->prev.origin != cent->current.origin)
		update = true;
	
	rent.origin.lerp(cent->prev.origin, cent->current.origin, cg.frame_lerp);
}

void	CG_UpdateFrame(const cg_entity_t *cent, r_entity_t &rent, bool &update)
{
	if(cent->prev.frame != cent->current.frame)
		update = true;
		
	rent.frame = cent->current.frame;
	rent.frame_old = cent->prev.frame;
}

void	CG_UpdateRotation(const cg_entity_t *cent, r_entity_t &rent, bool &update)
{
	if(cent->prev.quat != cent->current.quat)
		update = true;
		
	rent.quat.slerp(cent->prev.quat, cent->current.quat, cg.frame_lerp);
}

void	CG_UpdateModel(const cg_entity_t *cent, r_entity_t &rent, bool &update)
{
	if(cent->current.index_model && cg.model_draw[cent->prev.index_model] != cg.model_draw[cent->current.index_model])
		update = true;
		
	rent.model = cg.model_draw[cent->current.index_model];
}

void	CG_UpdateShader(const cg_entity_t *cent, r_entity_t &rent, bool &update)
{
	if(cent->current.index_shader && cg.shader_precache[cent->prev.index_shader] != cg.shader_precache[cent->current.index_shader])
		update = true;
		
	rent.custom_shader = cg.shader_precache[cent->current.index_shader];
}


void	CG_AddGenericEntity(const cg_entity_t *cent)
{
	cgi.Com_DPrintf("adding generic entity %i ...\n", cent->current.getNumber());
	
	r_entity_t	rent;

	if(!cent->current.index_model)
		cgi.Com_Error(ERR_DROP, "CG_AddGenericEntity: bad client game entity model index %i\n", cent->current.index_model);
	
	rent.model = cg.model_draw[cent->current.index_model];
	if(rent.model < 0)
		cgi.Com_Error(ERR_DROP, "CG_AddGenericEntity: bad renderer entity model index %i\n", rent.model);
		
	if(cent->current.index_shader)
		rent.custom_shader = cg.shader_precache[cent->current.index_shader];
	
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
	
	rent.frame = cent->current.frame;
	rent.frame_old = cent->current.frame;
	
	rent.flags = cent->current.renderfx;
	
	cgi.R_AddEntity(cent->current.getNumber(), rent);
}

void	CG_UpdateGenericEntity(const cg_entity_t *cent)
{
	//cgi.Com_DPrintf("updating generic entity ...\n");
	
	r_entity_t	rent;
	bool		update = false;
	
	CG_UpdateOrigin(cent, rent, update);
	
	CG_UpdateRotation(cent, rent, update);
	
	CG_UpdateModel(cent, rent, update);
	
	CG_UpdateShader(cent, rent, update);
	
	rent.shader_parms[0] = cent->current.shaderparms[0];
	rent.shader_parms[1] = cent->current.shaderparms[1];
	rent.shader_parms[2] = cent->current.shaderparms[2];
	rent.shader_parms[3] = cent->current.shaderparms[3];
	rent.shader_parms[4] = cent->current.shaderparms[4];
	rent.shader_parms[5] = cent->current.shaderparms[5];
	rent.shader_parms[6] = cent->current.shaderparms[6];
	rent.shader_parms[7] = cent->current.shaderparms[7];

	CG_UpdateFrame(cent, rent, update);	
	
	rent.flags = cent->current.renderfx;
	
	cgi.R_UpdateEntity(cent->current.getNumber(), rent, update);
}

void	CG_RemoveGenericEntity(const cg_entity_t *cent)
{
	cgi.Com_DPrintf("removing generic entity %i ...\n", cent->prev.getNumber());

	cgi.R_RemoveEntity(cent->prev.getNumber());
}


