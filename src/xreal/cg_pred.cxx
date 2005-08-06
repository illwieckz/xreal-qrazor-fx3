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


void	CG_CheckPredictionError()
{
	if(!cg_predict->getValue() || (cg.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
		return;

	// calculate the last usercmd_t we sent that the server has processed
	int incoming_ack, outgoing_seq;
	
	trap_CLS_GetCurrentNetState(incoming_ack, outgoing_seq);
	incoming_ack &= CMD_MASK;

	// compare what the server returned with what we had predicted it to be
	vec3_c delta = cg.frame.playerstate.pmove.origin - cg.predicted_origins[incoming_ack];

	// save the prediction error for interpolation
	if(fabs(delta[0]) > 128*8 || fabs(delta[1]) > 128*8 || fabs(delta[2]) > 128*8)
	{	
		// a teleport or something
		cg.prediction_error.clear();
	}
	else
	{
		if(cg_showmiss->getInteger() && (delta[0] || delta[1] || delta[2]))
			Com_Printf("prediction miss on %i: %i\n", cg.frame.serverframe, delta[0] + delta[1] + delta[2]);

		cg.predicted_origins[incoming_ack] = cg.frame.playerstate.pmove.origin;

		// save for error interpolation
		cg.prediction_error = delta * (1.0/16.0);
	}
}

static void	CG_ClipMoveToEntities(const vec3_c &start, const aabb_c &bbox, const vec3_c &end, trace_t *tr, int contentmask, int ignore)
{
//	int		i, x, zd, zu;
//	trace_t		trace;
//	int		headnode;
//	vec_t		*angles;
//	cmodel_t	*cmodel;	
//	cbbox_c		bbox2;

	for(int i=0; i<cg.frame.entities_num; i++)
	{
		entity_state_t *state;
		state = &cg.entities_parse[(cg.frame.entities_first + i) % MAX_ENTITIES];

		//TODO
		/*
		if(!state->solid)
			continue;

		if(ent->_s.getNumber() == ignore)
			continue;

		if(ent->_s.solid == 31)
		{	
			// special value for bmodel
			cmodel = trap_cl->model_clip[ent->_s.modelindex];
			
			if (!cmodel)
				continue;
				
			headnode = cmodel->headnode;
			angles = ent->_s.angles;
		}
		else
		{	
			// encoded bbox
			x  = 8*( ent->_s.solid & 31);
			zd = 8*((ent->_s.solid>>5) & 31);
			zu = 8*((ent->_s.solid>>10) & 63) - 32;

			bbox2._mins[0] = bbox2._mins[1] = -x;
			bbox2._maxs[0] = bbox2._maxs[1] = x;
	
			bbox2._mins[2] = -zd;
			bbox2._maxs[2] =  zu;

			headnode = trap_CM_HeadnodeForBox(bbox2);
			angles = vec3_origin;	// boxes don't rotate
		}

		if (tr->allsolid)
			return;

		trace = trap_CM_TransformedBoxTrace (start, end, bbox, headnode, contentmask, ent->_s.origin, angles);

		if (trace.allsolid || trace.startsolid || trace.fraction < tr->fraction)
		{
			trace.ent = ent;
				
		 	if (tr->startsolid)
			{
				*tr = trace;
				tr->startsolid = true;
			}
			else
				*tr = trace;
		}
		else if (trace.startsolid)
			tr->startsolid = true;
		*/
	}
}


trace_t	CG_Trace(const vec3_c &start, const aabb_c &aabb, const vec3_c &end, int contentmask, int ignore)
{
	trace_t	t;

	// check against world
	t = cg.world_cmodel->traceAABB(start, end, aabb, contentmask);
	
	/*
	if(t.fraction < 1.0)
	{
		t.ent = (entity_c*)1;
		//Com_Printf("CG_Trace: t.fraction < 1.0\n");
	}
	*/

	if(t.fraction == 0.0)
		return t;		// blocked by world

	// check all other solid models
	CG_ClipMoveToEntities(start, aabb, end, &t, contentmask, ignore);

	return t;
}

/*
static trace_t	CG_PMTrace(const vec3_c &start, const cbbox_c &bbox, const vec3_c &end)
{
	return CG_Trace(start, bbox, end, MASK_PLAYERSOLID, IGNORE_PLAYER);
}
*/

/*
static int	CG_PMpointcontents(const vec3_c &point)
{
	int			i;
	
	entity_c*	ent_state;
	int		ent_num;
	
	cmodel_t	*cmodel;
	int		contents;

	contents = trap_CM_PointContents(point, 0);


	for(i=0; i<trap_cl->frame.entities_num; i++)
	{
		ent_num = (trap_cl->frame.entities_parse_index + i) & (MAX_PARSE_ENTITIES-1);
		ent_state = &cg.entities_parse[ent_num];

		if(ent_state->_s.solid != 31) // special value for bmodel
			continue;

		cmodel = trap_cl->model_clip[ent_state->_s.modelindex];
		
		if(!cmodel)
			continue;

		contents |= trap_CM_TransformedPointContents(point, cmodel->headnode, ent_state->_s.origin, ent_state->_s.angles);
	}

	return contents;
}
*/

/*
cl.predicted_origin[0] = cl.frame.playerstate.pmove.origin[0];
cl.predicted_origin[1] = cl.frame.playerstate.pmove.origin[1];
cl.predicted_origin[2] = cl.frame.playerstate.pmove.origin[2];
			
cl.predicted_angles = cl.frame.playerstate.view_angles;
*/

void	CG_PredictMovement()
{
#if 0
	int			ack, current;
	int			frame;
	int			oldframe;
	usercmd_t	*cmd;
	pmove_t		pm;
	int			step;
	float			step_old;
	int			oldz;

	if(trap_cls->state != CA_ACTIVE)
		return;
		
	if(!cg_predict->value || (trap_cl->frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
	{	
		// just set angles
		trap_cl->predicted_angles = trap_cl->viewangles + trap_cl->frame.playerstate.pmove.delta_angles;
		return;
	}

	ack = trap_cls->netchan.getIncomingAcknowledged();
	current = trap_cls->netchan.getOutgoingSequence();

	// if we are too far out of date, just freeze
	if(current - ack >= CMD_BACKUP)
	{
		if(cg_showmiss->value)
			Com_Printf("exceeded CMD_BACKUP\n");
		return;	
	}

	// copy current state to pmove
	pm.clear();
	pm.trace = CG_PMTrace;
	pm.pointcontents = CG_PMpointcontents;
	//pm_airaccelerate = atof(trap_cl->configstrings[CS_AIRACCEL]);	//FIXME
	pm.s = trap_cl->frame.playerstate.pmove;

	// run frames
	while(++ack < current)
	{
		frame = ack & (CMD_BACKUP-1);
		cmd = &trap_cl->cmds[frame];

		pm.cmd = *cmd;
		trap_Com_Pmove(&pm);

		// save for debug checking
		trap_cl->predicted_origins[frame] = pm.s.origin;
	}

	oldframe = (ack-2) & (CMD_BACKUP-1);
	oldz = (int)trap_cl->predicted_origins[oldframe][2];
	step = (int)(pm.s.origin[2] - oldz);
	
	if(step > 63 && step < 160 && (pm.s.pm_flags & PMF_ON_GROUND))
	{
		step_old = 0;
		
		//if(trap_cls->realtime - trap_cl->predicted_step_time < 150)
		//	step_old = trap_cl->predicted_step_time * (150 - (trap_cls->realtime - trap_cl->predicted_step_time)) * (1.0/150);
		
		trap_cl->predicted_step = step_old + step;
		trap_cl->predicted_step_time = (int)(trap_cls->realtime - trap_cls->frametime / 2);
	}


	// copy results out for rendering
	trap_cl->predicted_origin = pm.s.origin;

	trap_cl->predicted_angles = pm.viewangles;
#else
	usercmd_t cmd;
	
	trap_CL_GetCurrentUserCommand(cmd);

	// just set angles
	cg.predicted_angles = cmd.angles + cg.frame.playerstate.pmove.delta_angles;
	return;
#endif
}
