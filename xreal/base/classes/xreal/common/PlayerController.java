package xreal.common;

import javax.vecmath.Vector3f;

import xreal.Angle3f;
import xreal.PlayerStateAccess;
import xreal.UserCommand;


/**
 * 
 * @author Robert Beckebans
 */
public class PlayerController {
	
	// all of the locals will be zeroed before each
	// pmove, just to make damn sure we don't have
	// any differences when running on client or server
	
	private class PlayerControllerLocals
	{
		public Vector3f forward = new Vector3f(0, 0, 0);
		public Vector3f right = new Vector3f(0, 0, 0);
		public Vector3f up = new Vector3f(0, 0, 0);
	
		public float frametime;

		public int msec;

		public boolean walking;
		public boolean groundPlane;
		// trace_t groundTrace;

		public float impactSpeed;

		public Vector3f previous_origin = new Vector3f(0, 0, 0);
		public Vector3f previous_velocity = new Vector3f(0, 0, 0);
		public int previous_waterlevel;
	}
	
	PlayerControllerLocals pml = new PlayerControllerLocals();
	PlayerMove pm;
	
	public void movePlayer(PlayerMove pmove) 
	{
		int finalTime = pmove.cmd.serverTime;

		if(finalTime < pmove.ps.getPlayerState_commandTime())
		{
			// should not happen
			return;
		}

		if(finalTime > (pmove.ps.getPlayerState_commandTime() + 1000))
		{
			pmove.ps.setPlayerState_commandTime(finalTime - 1000);
		}

		//pmove.ps->pmove_framecount = (pmove.ps->pmove_framecount + 1) & ((1 << PS_PMOVEFRAMECOUNTBITS) - 1);

		// chop the move up if it is too long, to prevent framerate
		// dependent behavior
		while(pmove.ps.getPlayerState_commandTime() != finalTime)
		{
			int msec = finalTime - pmove.ps.getPlayerState_commandTime();

			if(msec > 66)
			{
				msec = 66;
			}
			
			pmove.cmd.serverTime = pmove.ps.getPlayerState_commandTime() + msec;
			
			movePlayerSingle(pmove);

			if((pmove.ps.getPlayerState_pm_flags() & PlayerMovementFlags.JUMP_HELD) > 0)
			{
				pmove.cmd.upmove = 20;
			}
		}

		//PM_CheckStuck();
	}
	
	
	private void movePlayerSingle(PlayerMove pmove)
	{
		pm = pmove;

		// this counter lets us debug movement problems with a journal
		// by setting a conditional breakpoint fot the previous frame
		//c_pmove++;

		// clear results
		pm.numtouch = 0;
		pm.watertype = 0;
		pm.waterlevel = 0;

		/*
		if(pm.ps->stats[STAT_HEALTH] <= 0)
		{
			pm.tracemask &= ~CONTENTS_BODY;	// corpses can fly through bodies
		}
		*/

		// make sure walking button is clear if they are running, to avoid
		// proxy no-footsteps cheats
		if(Math.abs(pm.cmd.forwardmove) > 64 || Math.abs(pm.cmd.rightmove) > 64)
		{
			pm.cmd.buttons &= ~UserCommand.BUTTON_WALKING;
		}

		// set the talk balloon flag
		/*
		if((pm.cmd.buttons & UserCommand.BUTTON_TALK) > 0)
		{
			pm.ps->eFlags |= EF_TALK;
		}
		else
		{
			pm.ps->eFlags &= ~EF_TALK;
		}
		*/

		// clear the respawned flag if attack and use are cleared
		/*
		if(pm.ps->stats[STAT_HEALTH] > 0 && (pm.cmd.buttons & (UserCommand.BUTTON_ATTACK | UserCommand.BUTTON_ATTACK2 | UserCommand.BUTTON_USE_HOLDABLE)) > 0)
		{
			pm.ps->pm_flags &= ~PMF_RESPAWNED;
		}
		*/

		// if talk button is down, dissallow all other input
		// this is to prevent any possible intercept proxy from
		// adding fake talk balloons
		if((pmove.cmd.buttons & UserCommand.BUTTON_TALK) > 0)
		{
			pmove.cmd.buttons = UserCommand.BUTTON_TALK;
			pmove.cmd.forwardmove = 0;
			pmove.cmd.rightmove = 0;
			pmove.cmd.upmove = 0;
		}

		// clear all pmove local vars
		pml = new PlayerControllerLocals();

		// determine the time
		pml.msec = pmove.cmd.serverTime - pm.ps.getPlayerState_commandTime();
		if(pml.msec < 1)
		{
			pml.msec = 1;
		}
		else if(pml.msec > 200)
		{
			pml.msec = 200;
		}
		
		pm.ps.setPlayerState_commandTime(pmove.cmd.serverTime);

		// save old org in case we get stuck
		pml.previous_origin.set(pm.ps.getPlayerState_origin()); 

		// save old velocity for crashlanding
		pml.previous_velocity.set(pm.ps.getPlayerState_velocity());

		pml.frametime = pml.msec * 0.001f;

		// update the viewangles
		updateViewAngles(pm.ps, pm.cmd);
		
		Angle3f viewAngles = new Angle3f(pm.ps.getPlayerState_viewAngles());
		viewAngles.getVectors(pml.forward, pml.right, pml.up);

		if(pm.cmd.upmove < 10)
		{
			// not holding jump
			//pm.ps->pm_flags &= ~PMF_JUMP_HELD;
			pm.ps.setPlayerState_pm_flags(pm.ps.getPlayerState_pm_flags() & ~PlayerMovementFlags.JUMP_HELD);
		}

		// decide if backpedaling animations should be used
		if(pm.cmd.forwardmove < 0)
		{
			//pm.ps->pm_flags |= PMF_BACKWARDS_RUN;
			pm.ps.setPlayerState_pm_flags(pm.ps.getPlayerState_pm_flags() | PlayerMovementFlags.BACKWARDS_RUN);
		}
		else if(pm.cmd.forwardmove > 0 || (pm.cmd.forwardmove == 0 && pm.cmd.rightmove != 0))
		{
			//pm.ps->pm_flags &= ~PMF_BACKWARDS_RUN;
			pm.ps.setPlayerState_pm_flags(pm.ps.getPlayerState_pm_flags() & ~PlayerMovementFlags.BACKWARDS_RUN);
		}

		PlayerMovementType pm_type = pm.ps.getPlayerState_pm_type();
		switch(pm_type)
		{
			case DEAD:
			case FREEZE:
			case INTERMISSION:
			case SPINTERMISSION:
				pm.cmd.forwardmove = 0;
				pm.cmd.rightmove = 0;
				pm.cmd.upmove = 0;
				break;
		}
		
		switch(pm_type)
		{
			case SPECTATOR:
				//PM_UpdateViewAngles(pm.ps, &pm.cmd);
				//PM_CheckDuck();
				//PM_FlyMove();
				//PM_DropTimers();
				return;
				
			case NOCLIP:
				//PM_UpdateViewAngles(pm.ps, &pm.cmd);
				//PM_NoclipMove();
				//PM_DropTimers();
				return;
				
			case FREEZE:
			case INTERMISSION:
			case SPINTERMISSION:
				// no movement at all
				return;
		}

		// TODO regular movement
	}
	
	/**
	 * This can be used as another entry point when only the viewangles
	 * are being updated instead of a full move-
	 *
	 */
	private void updateViewAngles(PlayerStateAccess ps, UserCommand cmd)
	{
		PlayerMovementType pm_type = ps.getPlayerState_pm_type();
		
		switch(pm_type)
		{
			case INTERMISSION:
			case SPINTERMISSION:
				// no view changes at all
				return;
		}

		/*
		if(pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0)
		{
			// no view changes at all
			return;
		}
		*/

		// don't let the player look up or down more than 90 degrees
		float pitch = cmd.pitch + ps.getPlayerState_deltaPitch();
		if(pitch > 16000)
		{
			ps.setPlayerState_deltaPitch(16000 - cmd.pitch);
			pitch = 16000;
		}
		else if(pitch < -16000)
		{
			ps.setPlayerState_deltaPitch(-16000 - cmd.pitch);
			pitch = -16000;
		}
		
		float yaw = Angle3f.short2float(cmd.yaw + ps.getPlayerState_deltaYaw());
		float roll = Angle3f.short2float(cmd.yaw + ps.getPlayerState_deltaYaw());
		
		ps.setPlayerState_viewAngles(pitch, yaw, roll);
	}
}
