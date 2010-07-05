package xreal.common;

import java.util.LinkedList;
import java.util.Queue;

import javax.vecmath.Vector3f;

import com.bulletphysics.collision.dispatch.CollisionObject;
import com.bulletphysics.collision.dispatch.CollisionWorld;
import com.bulletphysics.collision.dispatch.PairCachingGhostObject;
import com.bulletphysics.collision.shapes.ConvexShape;
import com.bulletphysics.dynamics.ActionInterface;
import com.bulletphysics.linearmath.IDebugDraw;
import com.bulletphysics.linearmath.Transform;

import xreal.Angle3f;
import xreal.CVars;
import xreal.Engine;
import xreal.PlayerStateAccess;
import xreal.UserCommand;


/**
 * 
 * @author Robert Beckebans
 */
public class PlayerController implements ActionInterface {
	
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
	
	private PlayerControllerLocals pml = new PlayerControllerLocals();
	
	private Queue<PlayerMove> playerMovements = new LinkedList<PlayerMove>();
	private PlayerMove pm;
	
	private final PairCachingGhostObject ghostObject;
	private final ConvexShape convexShape;
	private final CollisionWorld collisionWorld;
	
	
	public PlayerController(PairCachingGhostObject ghostObject, ConvexShape convexShape, CollisionWorld collisionWorld) {
		this.ghostObject = ghostObject;
		this.convexShape = convexShape;
		this.collisionWorld = collisionWorld;
	}
	
	public void addPlayerMove(PlayerMove pmove)
	{
		//playerMovements.add(pmove);
	}
	
	@Override
	public void updateAction(CollisionWorld collisionWorld, float deltaTimeStep) {
		
		//Engine.println("PlayerController.updateAction(deltaTimeStep = " + deltaTimeStep + "), player movements = " + playerMovements.size());
		
		/*
		while (!playerMovements.isEmpty()) {
			PlayerMove pmove = playerMovements.remove();
			
			movePlayer(pmove);
		}
		*/
	}
	
	public void movePlayer(PlayerMove pmove) 
	{
		//Engine.println("Player.movePlayer(cmd.serverTime = " + pmove.cmd.serverTime + ", ps.commandTime = " + pmove.ps.getPlayerState_commandTime() + " )");
		
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
			
			try
			{
				movePlayerSingle(pmove);
			}
			catch(Exception e)
			{
				Engine.error("Something bad happened in Player.movePlayer(): " + e.getMessage());
				return;
			}

			if(pmove.ps.hasPlayerState_pm_flags(PlayerMovementFlags.JUMP_HELD))
			{
				pmove.cmd.upmove = 20;
			}
		}

		//PM_CheckStuck();
	}
	
	
	private void movePlayerSingle(PlayerMove pmove)
	{
		//Engine.println("PlayerController.movePlayerSingle()");
		
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
		
		//Engine.println("PlayerController.movePlayerSingle() 2...");

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
		
		//Engine.println("PlayerController.movePlayerSingle() 3...");

		// save old org in case we get stuck
		pml.previous_origin.set(pm.ps.getPlayerState_origin()); 

		// save old velocity for crashlanding
		pml.previous_velocity.set(pm.ps.getPlayerState_velocity());
		
		//Engine.println("PlayerController.movePlayerSingle() 4...");

		pml.frametime = pml.msec * 0.001f;

		// update the viewangles
		//updateViewAngles(pm.ps, pm.cmd);
		
		//Vector3f viewAngles = pm.ps.getPlayerState_viewAngles();
		
		//Engine.println("view angles:  " + pm.ps.getPlayerState_viewAngles());
		//Engine.println("delta angles: " + pm.ps.getPlayerState_deltaAngles());
		
		//Angle3f viewAngles = new Angle3f(pm.ps.getPlayerState_viewAngles());
		//viewAngles.getVectors(pml.forward, pml.right, pml.up);
		
		//Engine.println("PlayerController.movePlayerSingle() 5...");

		if(pm.cmd.upmove < 10)
		{
			// not holding jump
			pm.ps.delPlayerState_pm_flags(PlayerMovementFlags.JUMP_HELD);
		}

		// decide if backpedaling animations should be used
		if(pm.cmd.forwardmove < 0)
		{
			pm.ps.addPlayerState_pm_flags(PlayerMovementFlags.BACKWARDS_RUN);
		}
		else if(pm.cmd.forwardmove > 0 || (pm.cmd.forwardmove == 0 && pm.cmd.rightmove != 0))
		{
			pm.ps.delPlayerState_pm_flags(PlayerMovementFlags.BACKWARDS_RUN);
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
		
		//Engine.println("view angles:  " + pm.ps.getPlayerState_viewAngles());
		//Engine.println("delta angles: " + pm.ps.getPlayerState_deltaAngles());
		
		Angle3f viewAngles;
		switch(pm_type)
		{
			case SPECTATOR:
				updateViewAngles(pm.ps, pm.cmd);
				viewAngles = pm.ps.getPlayerState_viewAngles();
				viewAngles.getVectors(pml.forward, pml.right, pml.up);
				moveFlying(true);
				
				//PM_CheckDuck();
				//PM_FlyMove();
				//PM_DropTimers();
				return;
				
			case NOCLIP:
				updateViewAngles(pm.ps, pm.cmd);
				viewAngles = pm.ps.getPlayerState_viewAngles();
				viewAngles.getVectors(pml.forward, pml.right, pml.up);
				moveFlying(false);
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
	 * are being updated instead of a full move.
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
		short tmp = (short) (cmd.pitch + ps.getPlayerState_deltaPitch());
		if(tmp > 16000)
		{
			ps.setPlayerState_deltaPitch((short) (16000 - cmd.pitch));
			tmp = 16000;
		}
		else if(tmp < -16000)
		{
			ps.setPlayerState_deltaPitch((short) (-16000 - cmd.pitch));
			tmp = -16000;
		}
		
		float pitch = Angle3f.toFloat(tmp);
		float yaw = Angle3f.toFloat((short) (cmd.yaw + ps.getPlayerState_deltaYaw()));
		float roll = Angle3f.toFloat((short) (cmd.roll + ps.getPlayerState_deltaRoll()));
		
		/*	
		pitch = Angle3f.normalize180(pitch);
		yaw = Angle3f.normalize180(yaw);
		roll = Angle3f.normalize180(roll);
		*/
		
		ps.setPlayerState_viewAngles(pitch, yaw, roll);
	}
	
	/**
	 * Returns the scale factor to apply to cmd movements.
	 * 
	 * This allows the clients to use axial -127 to 127 values for all directions
	 * without getting a sqrt(2) distortion in speed.
	 * 
	 * @return The scale factor.
	 */
	float calculateUserCommandScale(final UserCommand cmd)
	{
		int             max;
		float           total;
		float           scale;

		max = Math.abs(cmd.forwardmove);
		
		if(Math.abs(cmd.rightmove) > max)
		{
			max = Math.abs(cmd.rightmove);
		}
		
		if(Math.abs(cmd.upmove) > max)
		{
			max = Math.abs(cmd.upmove);
		}
		
		if(max == 0)
		{
			return 0;
		}

		total = (float) Math.sqrt(cmd.forwardmove * cmd.forwardmove + cmd.rightmove * cmd.rightmove + cmd.upmove * cmd.upmove);
		scale = (float) ((float) pm.ps.getPlayerState_speed() * max / (127.0 * total));

		if(pm.ps.getPlayerState_pm_type() == PlayerMovementType.NOCLIP)
			scale *= 3;

		return scale;
	}
	
	/**
	 * Handles user intended acceleration.
	 * 
	 * @param wishdir
	 * @param wishspeed
	 * @param accel
	 * 
	 * @return The new velocity;
	 */
	Vector3f accelerate(Vector3f wishdir, float wishspeed, float accel)
	{
		Vector3f currentvel = pm.ps.getPlayerState_velocity();
		//float currentspeed = currentvel.length();
		//float currentspeed = wishdir.length();
		float currentspeed = currentvel.dot(wishdir);
		float addspeed = wishspeed - currentspeed;
		if(addspeed <= 0)
		{
			return null;
		}
		
		if(Float.isNaN(currentspeed))
			throw new RuntimeException("currentspeed is NaN");
		
		float accelspeed = accel * pml.frametime * wishspeed;
		if(accelspeed > addspeed)
		{
			accelspeed = addspeed;
		}

		Vector3f newVelocity = new Vector3f(wishdir);
		newVelocity.scale(accelspeed);
		
		//pm.ps.setPlayerState_velocity(newVelocity);
		return newVelocity;
	
		/*
		// proper way (avoids strafe jump maxspeed bug), but feels bad
		vec3_t          wishVelocity;
		vec3_t          pushDir;
		float           pushLen;
		float           canPush;

		VectorScale(wishdir, wishspeed, wishVelocity);
		VectorSubtract(wishVelocity, pm->ps->velocity, pushDir);
		pushLen = VectorNormalize(pushDir);

		canPush = accel * pml.frametime * wishspeed;
		if(canPush > pushLen)
		{
			canPush = pushLen;
		}

		VectorMA(pm->ps->velocity, canPush, pushDir, pm->ps->velocity);
		*/
	}
	
	private void friction()
	{
		Vector3f        vec;
		Vector3f		vel;
		float           speed, newspeed, control;
		float           drop;

		vel = pm.ps.getPlayerState_velocity();

		// TA: make sure vertical velocity is NOT set to zero when wall climbing
		vec = new Vector3f(vel);
		if(pml.walking && (pm.ps.getPlayerState_pm_flags() & PlayerMovementFlags.WALLCLIMBING) == 0)
		{
			// ignore slope movement
			vec.z = 0;
		}

		speed = vec.length();


		if(speed < 1)				// && pm->ps->pm_type != PM_SPECTATOR && pm->ps->pm_type != PM_NOCLIP)
		{
			vel.x = 0;
			vel.y = 0;				// allow sinking underwater
			// FIXME: still have z friction underwater?
			
			pm.ps.setPlayerState_velocity(vel);
			return;
		}

		drop = 0;

		// apply ground friction
		/*
		if(pm->waterlevel <= 1)
		{
			if(pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK))
			{
				// if getting knocked back, no friction
				if(!(pm->ps->pm_flags & PMF_TIME_KNOCKBACK))
				{
					control = speed < pm_stopspeed ? pm_stopspeed : speed;
					drop += control * pm_friction * pml.frametime;
				}
			}
		}

		// apply water friction even if just wading
		if(pm->waterlevel)
		{
			drop += speed * pm_waterfriction * pm->waterlevel * pml.frametime;
		}

		// apply flying friction
		if(pm->ps->powerups[PW_FLIGHT])
		{
			drop += speed * pm_flightfriction * pml.frametime;
		}
		*/

		PlayerMovementType pmType = pm.ps.getPlayerState_pm_type();
		
		if(pmType == PlayerMovementType.SPECTATOR)
		{
			drop += speed * CVars.pm_spectatorfriction.getValue() * pml.frametime;
		}

		// scale the velocity
		newspeed = speed - drop;
		if(newspeed < 0)
		{
			newspeed = 0;
		}
		newspeed /= speed;

		if(pmType == PlayerMovementType.SPECTATOR || pmType == PlayerMovementType.NOCLIP)
		{
			if(drop < 1.0f && speed < 3.0f)
			{
				newspeed = 0;
			}
		}

		vel.scale(newspeed);
		
		pm.ps.setPlayerState_velocity(vel);
	}
	
	void moveFlying(boolean clipAgainstWorld)
	{
		float           speed, drop, friction, control, newspeed;
		Vector3f        wishvel;
		float           fmove, smove;
		float           wishspeed;
		float           scale;

		pm.ps.setPlayerState_viewHeight(Config.DEFAULT_VIEWHEIGHT);

		// friction
		Vector3f curvel = pm.ps.getPlayerState_velocity();
		Vector3f curpos = pm.ps.getPlayerState_origin();
		
		//Engine.println("moveWithoutClipping(): old velocity = " + curvel + ", old origin = " + curpos);
		
		speed = curvel.length();
		if(speed < 1)
		{
			curvel.set(0, 0, 0);
		}
		else
		{
			drop = 0;

			friction = CVars.pm_friction.getValue() * 1.5f;	// extra friction
			control = speed < CVars.pm_stopSpeed.getValue() ? CVars.pm_stopSpeed.getValue() : speed;
			drop += control * friction * pml.frametime;

			// scale the velocity
			newspeed = speed - drop;
			if(newspeed < 0)
				newspeed = 0;
			newspeed /= speed;

			curvel.scale(newspeed);
		}
		pm.ps.setPlayerState_velocity(curvel);

		// accelerate
		scale = calculateUserCommandScale(pm.cmd);

		fmove = pm.cmd.forwardmove;
		smove = pm.cmd.rightmove;

		wishvel = new Vector3f(	pml.forward.x * fmove + pml.right.x * smove,
								pml.forward.y * fmove + pml.right.y * smove,
								pml.forward.z * fmove + pml.right.z * smove);
		
		if(Float.isNaN(wishvel.x) || Float.isNaN(wishvel.y) || Float.isNaN(wishvel.z))
			throw new RuntimeException("wishvel member(s) became NaN");
				
		wishvel.z += pm.cmd.upmove;

		Vector3f wishdir = new Vector3f(wishvel);
		
		wishspeed = wishdir.length();
		if(wishspeed > 0)
		{
			wishdir.normalize();
		}
		wishspeed *= scale;
		
		if(Float.isNaN(wishdir.x) || Float.isNaN(wishdir.y) || Float.isNaN(wishdir.z))
			throw new RuntimeException("wishdir member(s) became NaN");

		Vector3f newVelocity = accelerate(wishdir, wishspeed, CVars.pm_accelerate.getValue());
		

		// move
		//VectorMA(pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
		
		if(newVelocity != null)
		{
			if(Float.isNaN(newVelocity.x) || Float.isNaN(newVelocity.y) || Float.isNaN(newVelocity.z))
				throw new RuntimeException("newVelocity member(s) became NaN");
			
			pm.ps.setPlayerState_velocity(newVelocity);
			
			
			
			if(clipAgainstWorld)
			{
				Transform start = new Transform();
				Transform end = new Transform();
				
				Vector3f currentPosition = pm.ps.getPlayerState_origin();
				Vector3f targetPosition = new Vector3f();
				
				targetPosition.scaleAdd(pml.frametime, newVelocity, currentPosition);

				start.setIdentity();
				end.setIdentity();

				/* FIXME: Handle penetration properly */
				start.origin.set(currentPosition);
				end.origin.set(targetPosition);

				KinematicClosestNotMeConvexResultCallback callback = new KinematicClosestNotMeConvexResultCallback(ghostObject);
				callback.collisionFilterGroup = ghostObject.getBroadphaseHandle().collisionFilterGroup;
				callback.collisionFilterMask = ghostObject.getBroadphaseHandle().collisionFilterMask;

				if (CVars.pm_useGhostObjectSweepTest.getBoolean()) 
				{
					ghostObject.convexSweepTest(convexShape, start, end, callback, collisionWorld.getDispatchInfo().allowedCcdPenetration);
				}
				else 
				{
					collisionWorld.convexSweepTest(convexShape, start, end, callback);
				}

				if (callback.hasHit()) 
				{
					currentPosition.interpolate(currentPosition, targetPosition, callback.closestHitFraction);
				}
				else 
				{
					currentPosition.set(targetPosition);
				}
				
				pm.ps.setPlayerState_origin(currentPosition);
			}
			else
			{
			
				Vector3f origin = pm.ps.getPlayerState_origin();
				
				newVelocity.scale(pml.frametime);
				origin.add(newVelocity);
				pm.ps.setPlayerState_origin(origin);
			}
			
			//Vector3f vel = pm.ps.getPlayerState_velocity();
			//
		
			//org.scaleAdd(pml.frametime, vel);
			//org.scaleAdd(pml.frametime, wishvel);
			//org.z += 0.01f;
		
			//pm.ps.setPlayerState_velocity(wishvel);
			//pm.ps.setPlayerState_origin(org);
			
			//Engine.println("moveWithoutClipping(): new velocity = " + newVelocity + ", new origin = " + origin);
		}
	}
	
	/*
	void moveFlying()
	{
		float           wishspeed;
		Vector3f        wishvel;
		Vector3f		wishdir = new Vector3f();
		float           scale;

		
		// normal slowdown
		//friction();

		scale = calculateUserCommandScale(pm.cmd);
		
		// user intentions
		if(scale <= 0)
		{
			wishvel = new Vector3f();
		}
		else
		{
			
			
			wishvel = new Vector3f(	pml.forward.x * pm.cmd.forwardmove + pml.right.x * pm.cmd.rightmove,
									pml.forward.y * pm.cmd.forwardmove + pml.right.y * pm.cmd.rightmove,
									pml.forward.z * pm.cmd.forwardmove + pml.right.z * pm.cmd.rightmove);

			wishvel.z += scale * pm.cmd.upmove;
		}
		
		if(Float.isNaN(wishvel.x) || Float.isNaN(wishvel.y) || Float.isNaN(wishvel.z))
			throw new RuntimeException("wishvel member(s) became NaN");
				

		wishdir = new Vector3f(wishvel);
		wishspeed = wishdir.length();
		
		Vector3f newVelocity = accelerate(wishdir, wishspeed, pm_flyaccelerate);
		
		if(newVelocity != null)
		{
			if(Float.isNaN(newVelocity.x) || Float.isNaN(newVelocity.y) || Float.isNaN(newVelocity.z))
				throw new RuntimeException("newVelocity member(s) became NaN");
			
			pm.ps.setPlayerState_velocity(newVelocity);
			
			newVelocity.scale(pml.frametime);
			
			Vector3f origin = pm.ps.getPlayerState_origin();
			origin.add(newVelocity);
			pm.ps.setPlayerState_origin(origin);
			
			//Vector3f vel = pm.ps.getPlayerState_velocity();
			//
		
			//org.scaleAdd(pml.frametime, vel);
			//org.scaleAdd(pml.frametime, wishvel);
			//org.z += 0.01f;
		
			//pm.ps.setPlayerState_velocity(wishvel);
			//pm.ps.setPlayerState_origin(org);
			
			//Engine.println("moveWithoutClipping(): new velocity = " + newVelocity + ", new origin = " + origin);
		}
	}
	*/


	@Override
	public void debugDraw(IDebugDraw debugDrawer) {
		// TODO Auto-generated method stub
		
	}


	private static class KinematicClosestNotMeConvexResultCallback extends CollisionWorld.ClosestConvexResultCallback {
		protected CollisionObject me;

		public KinematicClosestNotMeConvexResultCallback(CollisionObject me) {
			super(new Vector3f(), new Vector3f());
			this.me = me;
		}

		@Override
		public float addSingleResult(CollisionWorld.LocalConvexResult convexResult, boolean normalInWorldSpace) {
			if (convexResult.hitCollisionObject == me) {
				return 1.0f;
			}

			return super.addSingleResult(convexResult, normalInWorldSpace);
		}
	}
	
}
