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
=================
CG_PrepRefresh

Call before entering a new level, or after changing dlls
=================
*/
void	CG_PrepRefresh()
{
	/*
	CG_RegisterSounds();
		
	if(!cgi.CL_GetConfigString(CS_MAPNAME)[0])
		return;		// no map loaded
	
	//
	// let the render dll load the map
	//
	std::string mapname = cgi.CL_GetConfigString(CS_MAPNAME);
		
	cgi.Com_Printf("CG_PrepRefresh: map '%s'\n", mapname.c_str());

	unsigned	map_checksum;		// for detecting cheater maps
	cgi.CM_BeginRegistration(mapname, true, &map_checksum);
	
	if((int)map_checksum != atoi(cgi.CL_GetConfigString(CS_MAPCHECKSUM)))
	{
		cgi.Com_Error(ERR_DROP, "Local map version differs from server: %i != '%s'\n", map_checksum, cgi.CL_GetConfigString(CS_MAPCHECKSUM));
		return;
	}
	
	cgi.R_BeginRegistration(mapname);
	
	
	//
	// register models, shaders et cetera ...
	//
	cgi.Com_Printf("CG_PrepRefresh: models\n");
	CG_RegisterTEntModels();

	for(int i=1; i<MAX_MODELS; i++)
	{
		if(!cgi.CL_GetConfigString(CS_MODELS+i)[0])
			continue;
	
		cg.model_clip[i] = cgi.CM_RegisterModel(cgi.CL_GetConfigString(CS_MODELS+i));
	
		cg.model_draw[i] = cgi.R_RegisterModel(cgi.CL_GetConfigString(CS_MODELS+i));
		
		//CG_UpdateScreen();
	}


	cgi.Com_Printf("CG_PrepRefresh: shaders\n");
	//CG_UpdateScreen();
	CG_RegisterPics();
	
	for(int i=1; i<MAX_SHADERS; i++)
	{
		if(!cgi.CL_GetConfigString(CS_SHADERS+i)[0])
			continue;
		 
		cg.shader_precache[i] = cgi.R_RegisterPic(cgi.CL_GetConfigString(CS_SHADERS+i));
	}
	
	
	cgi.Com_Printf("CG_PrepRefresh: animations\n");
	//CG_UpdateScreen();
	
	for(int i=1; i<MAX_ANIMATIONS; i++)
	{
		if(!cgi.CL_GetConfigString(CS_ANIMATIONS+i)[0])
			continue;
	
		cg.animation_precache[i] = cgi.R_RegisterAnimation(cgi.CL_GetConfigString(CS_ANIMATIONS+i));
	}
	
	
	cgi.Com_Printf("CG_PrepRefresh: lights\n");
	//CG_UpdateScreen();
	
	for(int i=1; i<MAX_LIGHTS; i++)
	{
		if(!cgi.CL_GetConfigString(CS_LIGHTS+i)[0])
			continue;
	
		cg.light_precache[i] = cgi.R_RegisterLight(cgi.CL_GetConfigString(CS_LIGHTS+i));
	}
	

	//
	// register clients
	//
	cgi.Com_Printf("CG_PrepRefresh: clients\n");
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(!cgi.CL_GetConfigString(CS_PLAYERSKINS+i)[0])
			continue;
		 
		CG_ParseClientinfo(i);
	}

	std::string default_clientinfo = "unnamed\\" + std::string(DEFAULT_PLAYERMODEL) + "/" + std::string(DEFAULT_PLAYERSKIN);
	CG_LoadClientinfo(&cg.baseclientinfo, default_clientinfo);
	*/
	
	cgi.R_SetSky(cgi.CL_GetConfigString(CS_SKY));
	
	
	//
	// the renderer can now free unneeded stuff
	//
	cgi.CM_EndRegistration();
	cgi.R_EndRegistration();


	//
	// clear any lines of console text
	//
	cgi.Con_ClearNotify();

	//CG_UpdateScreen();
	
	cgi.CL_SetRefreshPrepped(true);
	cgi.CL_SetForceRefdef(true);	// make sure we have a valid refdef
}




void	CG_CalcLerpFrac()
{
	if(cgi.CLS_GetConnectionState() != CA_ACTIVE)
		return;

	if(cgi.CL_GetTime() > cg.frame.servertime)
	{
		if(cg_showclamp->getValue())
			cgi.Com_Printf("high clamp %i\n", cgi.CL_GetTime() - cg.frame.servertime);
			
		cgi.CL_SetTime(cg.frame.servertime);
		cg.frame_lerp = 1.0;
	}
	else if(cgi.CL_GetTime() < cg.frame.servertime - 100)
	{
		if(cg_showclamp->getValue())
			cgi.Com_Printf("low clamp %i\n", cg.frame.servertime-100 - cgi.CL_GetTime());
			
		cgi.CL_SetTime(cg.frame.servertime - 100);
		cg.frame_lerp = 0;
	}
	else
	{
		cg.frame_lerp = 1.0 - (cg.frame.servertime - cgi.CL_GetTime()) * 0.01;
	}

	if(cgi.Cvar_VariableValue("timedemo"))
		cg.frame_lerp = 1.0;
}



static void	CG_CalcViewValues()
{
	float		lerp, backlerp;
	cg_entity_t	*ent;
	frame_t		*oldframe = NULL;
	player_state_t	*ps, *ps_old;
	
	if(cgi.CLS_GetConnectionState() != CA_ACTIVE)
		return;

	// find the previous frame to interpolate from
	ps = &cg.frame.playerstate;
	
	if(cg.frame_old.serverframe != cg.frame.serverframe -1 || !cg.frame_old.valid)
		oldframe = &cg.frame;		// previous frame was dropped or involid
	else
		oldframe = &cg.frame_old;
	
	ps_old = &oldframe->playerstate;

	
	// see if the player entity was teleported this frame
	if(	   fabs(ps_old->pmove.origin[0] - ps->pmove.origin[0]) > 256
		|| fabs(ps_old->pmove.origin[1] - ps->pmove.origin[1]) > 256
		|| fabs(ps_old->pmove.origin[2] - ps->pmove.origin[2]) > 256	)
	{
		ps_old = ps;		// don't interpolate
	}

	ent = &cg.entities[cgi.playernum+1];
	lerp = cg.frame_lerp;

	// calculate the origin
	if(cg_predict->getValue() && !(cg.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
	{
		// use predicted values
		unsigned	delta;

		backlerp = 1.0 - lerp;
		
		for(int i=0; i<3; i++)
		{
			cg.refdef.view_origin[i] = cg.predicted_origin[i] + ps_old->view_offset[i] 
			+ cg.frame_lerp * (ps->view_offset[i] - ps_old->view_offset[i])
				- backlerp * cg.prediction_error[i];
		}

		// smooth out stair climbing
		delta = (int)cgi.CLS_GetRealTime() - cg.predicted_step_time;
		if(delta < 100)
			cg.refdef.view_origin[2] -= cg.predicted_step * (100 - delta) * 0.01;
	}
	else
	{	// just use interpolated values
		for(int i=0; i<3; i++)
			cg.refdef.view_origin[i] = ps_old->pmove.origin[i] + ps_old->view_offset[i] 
			+ lerp * (ps->pmove.origin[i] + ps->view_offset[i] 
				- (ps_old->pmove.origin[i] + ps_old->view_offset[i]));
	}

	// if not running a demo or on a locked frame, add the local angle movement
	if(cg.frame.playerstate.pmove.pm_type < PM_DEAD)
	{
		// use predicted values
		cg.refdef.view_angles = cg.predicted_angles;
	}
	else
	{	
		// just use interpolated values
		for(int i=0; i<3; i++)
			cg.refdef.view_angles[i] = LerpAngle(ps_old->view_angles[i], ps->view_angles[i], lerp);
	}

	// add interpolated kick angles
	for(int i=0; i<3; i++)
		cg.refdef.view_angles[i] += LerpAngle(ps_old->kick_angles[i], ps->kick_angles[i], lerp);

	// convert angles to vectors
	Angles_ToVectors(cg.refdef.view_angles, cg.v_forward, cg.v_right, cg.v_up);

	// interpolate field of view
	cg.refdef.fov_x = ps_old->fov + lerp * (ps->fov - ps_old->fov);
	
	// interpolate linear velocity
	cg.v_velocity.lerp(ps_old->pmove.velocity_linear, ps->pmove.velocity_linear, lerp);

	// don't interpolate blend color
	cg.v_blend = ps->blend;
}

void	CG_RenderView()
{
	if(cgi.CLS_GetConnectionState() != CA_ACTIVE)
		return;

	if(!cgi.CL_GetRefreshPrepped())
		return;			// still loading

	/*
	if(cgi.Cvar_VariableValue("timedemo"))
	{
		if(!cgi.CL_GetTime()demo_start)
			cgi.CL_GetTime()demo_start = cgi.Sys_Milliseconds();
			
		cgi.CL_GetTime()demo_frames++;
	}
	*/

	// an invalid frame will just use the exact previous refdef
	// we can't use the old frame if the video mode has changed, though...
	if(cg.frame.valid && (cgi.CL_GetForceRefdef() || !cg_paused->getValue()))
	{
		cgi.CL_SetForceRefdef(false);

		cgi.R_ClearScene();

		CG_CalcLerpFrac();
		
		CG_CalcViewValues();
		
		CG_AddEntities();
		
		cg.refdef.x = scr_vrect.x;
		cg.refdef.y = scr_vrect.y;
		cg.refdef.width = scr_vrect.width;
		cg.refdef.height = scr_vrect.height;
		cg.refdef.fov_y = CalcFOV(cg.refdef.fov_x, cg.refdef.width, cg.refdef.height);
		cg.refdef.time = cgi.CL_GetTime()*0.001;

		cg.refdef.areabits = cg.frame.areabits;

		cg.refdef.rdflags = cg.frame.playerstate.rdflags;
	}

	cgi.R_RenderFrame(cg.refdef);
	
	
	// clear any dirty part of the background
	//CG_TileClear();
		
	CG_DrawCrosshair();
		
	//CG_DrawBlend();
	
	//CG_DrawStats();
			
	//CG_DrawLayout();
		
	//CG_DrawInventory();

	//CG_DrawNet();
		
	//CG_CheckDrawCenterString();
			
	CG_DrawFPS();

	//CG_DrawMenu();
}


void	CG_Viewpos_f()
{
	cgi.Com_Printf("%s : %i\n", cg.refdef.view_origin.toString(), (int)cg.refdef.view_angles[YAW]);
}


void	CG_Sky_f()
{
	if(cgi.Cmd_Argc() != 2)
	{
		cgi.Com_Printf("usage: sky <name>\n");
		return;
	}
	
	// set sky
	cgi.R_SetSky(cgi.Cmd_Argv(1));
}

void 	CG_InitView()
{
	cgi.Cmd_AddCommand("viewpos", CG_Viewpos_f);
	cgi.Cmd_AddCommand("sky", CG_Sky_f);
}



