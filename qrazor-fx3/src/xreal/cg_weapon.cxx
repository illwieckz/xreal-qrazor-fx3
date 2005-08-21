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


static int			gun_frame = 0;
static int			gun_model = -1;
static int			gun_anim = -1;

static void 	CG_Gun_Next_f()
{
	gun_frame++;
	
	cgi.Com_Printf("frame %i\n", gun_frame);
}

static void 	CG_Gun_Prev_f()
{
	gun_frame--;
	
	if(gun_frame < 0)
		gun_frame = 0;
		
	cgi.Com_Printf("frame %i\n", gun_frame);
}

static void 	CG_Gun_Model_f()
{
	if(cgi.Cmd_Argc() != 2)
	{
		gun_model = -1;
		return;
	}
	
	gun_model = cgi.R_RegisterModel(cgi.Cmd_Argv(1));
}

static void 	CG_Gun_Anim_f()
{
	if(cgi.Cmd_Argc() != 2)
	{
		gun_anim = -1;
		return;
	}
	
	gun_anim = cgi.R_RegisterAnimation(cgi.Cmd_Argv(1));
}


void 	CG_InitWeapon()
{
	// gun frame debugging functions
	cgi.Cmd_AddCommand("gun_next",	CG_Gun_Next_f);
	cgi.Cmd_AddCommand("gun_prev",	CG_Gun_Prev_f);
	cgi.Cmd_AddCommand("gun_model", CG_Gun_Model_f);
	cgi.Cmd_AddCommand("gun_anim",	CG_Gun_Anim_f);
}


/*
void	CG_AddViewWeapon()
{
	//static int	frame = 0;
	//static int	time = 0;
	
	r_entity_t	gun;		// view model
	vec3_c		angles;
	frame_t*	oldframe;
	player_state_t	*ps = NULL, *ps_old;
	
	if(cgi.CLS_GetConnectionState() != CA_ACTIVE)
		return;

	// allow the gun to be completely removed
	if(!cg_gun->getValue())
		return;
	
	// find the previous frame to interpolate from
	ps = &cg.frame.playerstate;
	
	if(cg.frame_old.serverframe != cg.frame.serverframe-1 || !cg.frame_old.valid)
		oldframe = &cg.frame;		// previous frame was dropped or involid
	else
		oldframe = &cg.frame_old;
	
	ps_old = &oldframe->playerstate;
	
	// don't draw gun if in wide angle view
	//if(ps->fov > 90)
	//	return;



	//memset (&gun, 0, sizeof(gun));
	
	if(gun_model != -1 && gun_anim != -1)
	{
		gun.model = gun_model;	// development tool
		
		cgi.R_SetupAnimation(gun.model, gun_anim);
	}
	else
	{
		gun.model = cg.model_draw[ps->gun_model_index];
		int anim = cg.animation_precache[ps->gun_anim_index];
	
		//gun.model = cgi.R_RegisterModel("models/weapons/pistol/cycles/idle.md5mesh");
		//int anim = cgi.R_RegisterAnimation("models/weapons/pistol/cycles/idle.md5anim");
		
		if(!cgi.R_SetupAnimation(gun.model, anim))
			return;
	}
		
	if(gun.model == -1)
	{
		cgi.Com_Printf("CG_AddViewWeapon: weapon has bad model index %i\n", gun.model);
		return;
	}
		
	gun.custom_shader = -1;
	gun.custom_skin = -1;
		
	// set up gun position
	for(int i=0; i<3; i++)
	{
		gun.origin[i] = cg.refdef.view_origin[i];// + ops->gunoffset[i] + cgi.cl->lerpfrac * (ps->gunoffset[i] - ops->gunoffset[i]);
		angles[i] = cg.refdef.view_angles[i];// + LerpAngle(ps_old->gun_angles[i], ps->gun_angles[i], cg.frame_lerp);
		
	}
	
	gun.quat.fromAngles(angles);
	
	//gun.origin += (cg.v_forward * 30);
	
	if(gun_frame)
	{
		gun.frame = gun_frame;		// development tool
		gun.frame_old = gun_frame;	// development tool
	}
	else
	{
		gun.frame = ps->gun_anim_frame;
		
		if(gun.frame == 0)
			gun.frame_old = 0;	// just changed weapons, don't lerp from old
		else
			gun.frame_old = ps_old->gun_anim_frame;
	}
	
	//gun.frame = frame;

	gun.flags = RF_DEPTHHACK | RF_WEAPONMODEL;
	gun.backlerp = 1.0 - cg.frame_lerp;
	gun.origin2 = gun.origin;		// don't lerp at all	
	
	cgi.R_AddEntity(gun);
	
	//cgi.Com_Printf("CG_AddViewWeapon\n");
}
*/

