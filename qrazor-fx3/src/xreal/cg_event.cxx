/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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
==============
CL_EntityEvent

An entity has just been parsed that has an event value

the female events are there for backwards compatability
==============
*/
extern int	cl_sfx_footsteps[4];

void	CG_EntityEvent(const cg_entity_t *cent)
{
	vec4_c	color;
	vec3_c	dir;
	
//	trap_Com_Printf("CG_EntityEvent(%i)\n", cent->current.event);

	switch(cent->current.event)
	{
		case EV_NONE:
		default:
			break;
	
		case EV_ITEM_RESPAWN:
			trap_S_StartSound(cent->current.origin, cent->current.getNumber(), CHAN_WEAPON, trap_S_RegisterSound("items/respawn1.wav"));
			CG_ParticleSpray(PART_RESPAWN, cent->current.origin, dir, color, 64);
			break;
		
		//case EV_PLAYER_TELEPORT:
		//	trap_S_StartSound(NULL, cent->current.getNumber(), CHAN_WEAPON, trap_S_RegisterSound("misc/tele1.wav"), 1, ATTN_IDLE, 0);
		//	CG_ParticleSpray(PART_BOSSTELE, cent->current.origin, dir, color, 1024);
		//	break;
	
		case EV_PLAYER_FOOTSTEP:
			if(cg_footsteps->getValue())
				trap_S_StartSound(cent->current.origin, cent->current.getNumber(), CHAN_BODY, cl_sfx_footsteps[rand()&3]);
			break;
	
		case EV_PLAYER_FALL_SHORT:
			trap_S_StartSound(cent->current.origin, cent->current.getNumber(), CHAN_AUTO, trap_S_RegisterSound("player/land1.wav"));
			break;

		#if 0	
		case EV_PLAYER_FALL_MEDIUM:
			trap_S_StartSound(cent->current.origin, cent->current.getNumber(), CHAN_AUTO, trap_S_RegisterSound("*fall2.wav"));
			break;
	
		case EV_FALL_FAR:
			trap_S_StartSound(cent->current.origin, cent->current.getNumber(), CHAN_AUTO, trap_S_RegisterSound("*fall1.wav"));
			break;
			
		case EV__FAR:
			trap_S_StartSound(cent->current.origin, cent->current.getNumber(), CHAN_AUTO, trap_S_RegisterSound("*fall1.wav"));
			break;
		#endif
		
		case EV_ROCKET_EXPLOSION:
		{
			//trap_Com_Printf("CG_EntityEvent(EV_ROCKET_EXPLOSION)\n");
			trap_S_StartSound(cent->current.origin, cent->current.getNumber(), CHAN_AUTO, trap_S_RegisterSound("rocket_impact"));
			break;
		}
			
		//
		// muzzle flashes
		//
#if 0
		case MZ_BLASTER:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/blastf1a.wav"), volume, ATTN_NORM, 0);
			break;
		
		case MZ_HYPERBLASTER:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/hyprbf1a.wav"), volume, ATTN_NORM, 0);
			break;
		
		case MZ_MACHINEGUN:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound(soundname), volume, ATTN_NORM, 0);
			break;
		
		case MZ_SHOTGUN:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
		
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/shotgf1b.wav"), volume, ATTN_NORM, 0);
			trap_S_StartSound (NULL, i, CHAN_AUTO,   trap_S_RegisterSound("weapons/shotgr1b.wav"), volume, ATTN_NORM, 0.1);
			break;
		
		case MZ_SSHOTGUN:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/sshotf1b.wav"), volume, ATTN_NORM, 0);
			break;
			
		case MZ_CHAINGUN1:
			dl->radius = 200 + (rand()&31);
			dl->color[0] = 1;
			dl->color[1] = 0.25;
			dl->color[2] = 0;
			
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound(soundname), volume, ATTN_NORM, 0);
			break;
			
		case MZ_CHAINGUN2:
			dl->radius = 225 + (rand()&31);
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0;
			dl->die = trap_cl->time  + 0.1;	// long delay
			
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound(soundname), volume, ATTN_NORM, 0);
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound(soundname), volume, ATTN_NORM, 0.05);
			break;
		
		case MZ_CHAINGUN3:
			dl->radius = 250 + (rand()&31);
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			dl->die = trap_cl->time  + 0.1;	// long delay
		
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound(soundname), volume, ATTN_NORM, 0);
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound(soundname), volume, ATTN_NORM, 0.033);
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound(soundname), volume, ATTN_NORM, 0.066);
			break;
		
		case MZ_RAILGUN:
			dl->color[0] = 0.5;
			dl->color[1] = 0.5;
			dl->color[2] = 1.0;
			
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/railgf1a.wav"), volume, ATTN_NORM, 0);
			break;
		
		case MZ_ROCKET:
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0.2;
		
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("sound/weapons/rocklf1a.wav"), volume, ATTN_NORM, 0);
			trap_S_StartSound (NULL, i, CHAN_AUTO,   trap_S_RegisterSound("sound/weapons/rocklr1b.wav"), volume, ATTN_NORM, 0.1);
			break;
		
		case MZ_GRENADE:
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0;
			
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/grenlf1a.wav"), volume, ATTN_NORM, 0);
			trap_S_StartSound (NULL, i, CHAN_AUTO,   trap_S_RegisterSound("weapons/grenlr1b.wav"), volume, ATTN_NORM, 0.1);
			break;
		
		case MZ_BFG:
			dl->color[0] = 0;
			dl->color[1] = 1;
			dl->color[2] = 0;
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/bfg__f1y.wav"), volume, ATTN_NORM, 0);
			break;

		case MZ_LOGIN:
			dl->color[0] = 0;
			dl->color[1] = 1; 
			dl->color[2] = 0;
			dl->die = trap_cl->time + 1.0;
			
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/grenlf1a.wav"), 1, ATTN_NORM, 0);
			
			//TODO
			/*
			{
				vec4_c	color(0, 1, 0, 1);
				vec3_c	dir(0, 0, 0);
			
				CG_ParticleSpray(PART_BOSSTELE, pl->current.origin, dir, color, 1024);
			}
			*/
			break;
		
		case MZ_LOGOUT:
			dl->color[0] = 1;
			dl->color[1] = 0;
			dl->color[2] = 0;
			dl->die = trap_cl->time + 1.0;
			
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/grenlf1a.wav"), 1, ATTN_NORM, 0);
			
			//TODO
			/*
			{
				vec4_c	color(1, 0, 0, 1);
				vec3_c	dir(0, 0, 0);
			
				CG_ParticleSpray(PART_BOSSTELE, pl->current.origin, dir, color, 1024);
			}
			*/
			break;
			
		case MZ_RESPAWN:
			dl->color[0] = 1;
			dl->color[1] = 1;
			dl->color[2] = 0;
			dl->die = trap_cl->time + 1.0;
			
			trap_S_StartSound (NULL, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/grenlf1a.wav"), 1, ATTN_NORM, 0);
			/*
			{
				vec4_c	color(1, 1, 0, 1);
				vec3_c	dir(0, 0, 0);
			
				CG_ParticleSpray(PART_BOSSTELE, pl->current.origin, dir, color, 1024);
			}
			*/
			break;
			
			
			
		case MZ_SIDEWINDER:
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0.2;
			
			//TODO
		
			trap_S_StartSound(NULL, i, CHAN_WEAPON, trap_S_RegisterSound("sounds/e1/we_sidewindershoot.wav"), volume, ATTN_NORM, 0);
			break;
#endif
	}
}


void	CG_CheckEntityEvents()
{
	for(int i=0; i<cg.frame.entities_num; i++)
	{
		entity_state_t *state = &cg.entities_parse[(cg.frame.entities_first + i) % MAX_ENTITIES];
		cg_entity_t *cent = &cg.entities[state->getNumber()];
		
		CG_EntityEvent(cent);
	}
}



