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



void	CG_ParseMuzzleFlash(bitmessage_c &msg)
{
	//vec3_c		fv, rv, upv;
	matrix_c	matrix;
	
	//cg_light_t*	light;
	int		i, weapon;
	cg_entity_t*	pl;
	float		volume;
	char		soundname[64];
	
	//trap_Com_Printf("CG_ParseMuzzleFlash:\n");

	i = msg.readShort();
	if(i < 1 || i >= MAX_ENTITIES)
		trap_Com_Error(ERR_DROP, "CG_ParseMuzzleFlash: bad entity");

	weapon = msg.readByte();

	pl = &cg.entities[i];

	/*
	//light = CG_AllocLight(i);
	Vector3_Copy(pl->current.origin,  light->origin);
	//Angles_ToVectors(pl->current.angles, fv, rv, upv);
	matrix.fromQuaternion(pl->current.quat);
	Vector3_MA (light->origin, 18, matrix[0], light->origin);
	Vector3_MA (light->origin, 16, matrix[1], light->origin);
	light->radius = vec3_c(1, 1, 1);
	light->radius.scale(200 + (rand()&31));
	light->minlight = 32;
	light->die = trap_cl->time; // + 0.1;
	*/

	volume = 1;

	switch(weapon)
	{
		case MZ_BLASTER:
			//light->color = color_green;
			
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("sounds/e1/we_ionshoota.wav"));
			break;
		
		case MZ_HYPERBLASTER:
			/*
			light->color[0] = 1;
			light->color[1] = 1;
			light->color[2] = 0;
			*/
			
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/hyprbf1a.wav"));
			break;
		
		case MZ_MACHINEGUN:
			/*
			light->color[0] = 1;
			light->color[1] = 1;
			light->color[2] = 0;
			*/
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound(soundname));
			break;
		
		case MZ_SHOTGUN:
			/*
			light->color[0] = 1;
			light->color[1] = 1;
			light->color[2] = 0;
			*/
		
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/shotgf1b.wav"));
			trap_S_StartSound (pl->current.origin, i, CHAN_AUTO,   trap_S_RegisterSound("weapons/shotgr1b.wav"));
			break;
		
		case MZ_SSHOTGUN:
			/*
			light->color[0] = 1;
			light->color[1] = 1;
			light->color[2] = 0;
			*/
			
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/sshotf1b.wav"));
			break;
			
		case MZ_CHAINGUN1:
			/*
			light->radius.scale(200 + (rand()&31));
			light->color[0] = 1;
			light->color[1] = 0.25;
			light->color[2] = 0;
			*/
			
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound(soundname));
			break;
			
		case MZ_CHAINGUN2:
			/*
			light->radius.scale(225 + (rand()&31));
			light->color[0] = 1;
			light->color[1] = 0.5;
			light->color[2] = 0;
			light->die = trap_cl->time  + 0.1;	// long delay
			*/
			
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound(soundname));
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound(soundname));
			break;
		
		case MZ_CHAINGUN3:
			/*
			light->radius.scale(250 + (rand()&31));
			light->color[0] = 1;
			light->color[1] = 1;
			light->color[2] = 0;
			light->die = trap_cl->time  + 0.1;	// long delay
			*/
		
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound(soundname));
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound(soundname));
			Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound(soundname));
			break;
		
		case MZ_RAILGUN:
			/*
			light->color[0] = 0.5;
			light->color[1] = 0.5;
			light->color[2] = 1.0;
			*/
			
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/railgf1a.wav"));
			break;
		
		case MZ_ROCKET:
			/*
			light->color[0] = 1;
			light->color[1] = 0.5;
			light->color[2] = 0.2;
			*/
		
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("sound/weapons/rocklf1a.wav"));
			trap_S_StartSound (pl->current.origin, i, CHAN_AUTO,   trap_S_RegisterSound("sound/weapons/rocklr1b.wav"));
			break;
		
		case MZ_GRENADE:
			/*
			light->color[0] = 1;
			light->color[1] = 0.5;
			light->color[2] = 0;
			*/
			
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("sound/weapons/grenlf1a.wav"));
			trap_S_StartSound (pl->current.origin, i, CHAN_AUTO,   trap_S_RegisterSound("sound/weapons/grenlr1b.wav"));
			break;
		
		case MZ_BFG:
			/*
			light->color[0] = 0;
			light->color[1] = 1;
			light->color[2] = 0;
			*/
			
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/bfg__f1y.wav"));
			break;

		case MZ_LOGIN:
			/*
			light->color[0] = 0;
			light->color[1] = 1; 
			light->color[2] = 0;
			light->die = trap_cl->time + 1.0;
			*/
			
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/grenlf1a.wav"));
			
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
			/*
			light->color[0] = 1;
			light->color[1] = 0;
			light->color[2] = 0;
			light->die = trap_cl->time + 1.0;
			*/
			
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/grenlf1a.wav"));
			
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
			/*
			light->color[0] = 1;
			light->color[1] = 1;
			light->color[2] = 0;
			light->die = trap_cl->time + 1.0;
			*/
			
			trap_S_StartSound (pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("weapons/grenlf1a.wav"));
			/*
			{
				vec4_c	color(1, 1, 0, 1);
				vec3_c	dir(0, 0, 0);
			
				CG_ParticleSpray(PART_BOSSTELE, pl->current.origin, dir, color, 1024);
			}
			*/
			break;
			
			
			
		case MZ_SIDEWINDER:
			/*
			light->color[0] = 1;
			light->color[1] = 0.5;
			light->color[2] = 0.2;
			*/
			//TODO
		
			trap_S_StartSound(pl->current.origin, i, CHAN_WEAPON, trap_S_RegisterSound("sounds/e1/we_sidewindershoot.wav"));
			break;

	}
}







