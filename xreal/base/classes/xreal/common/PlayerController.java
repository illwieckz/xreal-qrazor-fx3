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
		public Vector3f groundPlaneNormal = new Vector3f();
		// trace_t groundTrace;

		public float impactSpeed;

		public Vector3f previous_origin = new Vector3f(0, 0, 0);
		public Vector3f previous_velocity = new Vector3f(0, 0, 0);
		public int previous_waterlevel;
	}
	private PlayerControllerLocals pml = new PlayerControllerLocals();
	
	private static final int MAX_CLIP_PLANES = 5;
	private static final float OVERCLIP	= 1.001f;
	
	private Queue<PlayerMove> playerMovements = new LinkedList<PlayerMove>();
	private PlayerMove pm;
	
	private final PairCachingGhostObject ghostObject;
	private final ConvexShape convexShape;
	private final CollisionWorld collisionWorld;
	
	private int c_pmove;
	
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
		c_pmove++;

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
				//flyMove();
				spectatorMove(true);
				
				//PM_CheckDuck();
				//PM_FlyMove();
				//PM_DropTimers();
				return;
				
			case NOCLIP:
				updateViewAngles(pm.ps, pm.cmd);
				viewAngles = pm.ps.getPlayerState_viewAngles();
				viewAngles.getVectors(pml.forward, pml.right, pml.up);
				spectatorMove(false);
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

		Vector3f newVelocity = new Vector3f();
		newVelocity.scaleAdd(accelspeed, wishdir, currentvel);
		
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
	
	/**
	 * Slide off of the impacting surface.
	 * 
	 * @param in
	 * @param normal
	 * @param out
	 * @param overbounce
	 */
	void clipVelocity(Vector3f in, Vector3f normal, Vector3f out, float overbounce)
	{
		float backoff = in.dot(normal);
		
		if(backoff < 0)
		{
			backoff *= overbounce;
		}
		else
		{
			backoff /= overbounce;
		}

		out.x = in.x - (normal.x * backoff);
		out.y = in.y - (normal.y * backoff);
		out.z = in.z - (normal.z * backoff);
	}
	
	private void applyFriction()
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
		
		if(speed != 0)
		{
			newspeed /= speed;
		}

		/*
		if(pmType == PlayerMovementType.SPECTATOR || pmType == PlayerMovementType.NOCLIP)
		{
			if(drop < 1.0f && speed < 3.0f)
			{
				newspeed = 0;
			}
		}
		*/

		vel.scale(newspeed);
		
		pm.ps.setPlayerState_velocity(vel);
	}
	
	/*
	==============
	PM_CheckDuck

	Sets mins, maxs, and pm->ps->viewheight
	==============
	*/
	void checkDuck()
	{
		//trace_t         trace;

		/*
		if(pm->ps->powerups[PW_INVULNERABILITY])
		{
			if(pm->ps->pm_flags & PMF_INVULEXPAND)
			{
				// invulnerability sphere has a 42 units radius
				VectorSet(pm->mins, -42, -42, -42);
				VectorSet(pm->maxs, 42, 42, 42);
			}
			else
			{
				VectorSet(pm->mins, -15, -15, MINS_Z);
				VectorSet(pm->maxs, 15, 15, 16);
			}
			pm->ps->pm_flags |= PMF_DUCKED;
			pm->ps->viewheight = CROUCH_VIEWHEIGHT;
			return;
		}
		pm->ps->pm_flags &= ~PMF_INVULEXPAND;
		*/

		/*
		pm->mins[0] = -15;
		pm->mins[1] = -15;

		pm->maxs[0] = 15;
		pm->maxs[1] = 15;

		pm->mins[2] = MINS_Z;

		if(pm->ps->pm_type == PM_DEAD)
		{
			pm->maxs[2] = -8;
			pm->ps->viewheight = DEAD_VIEWHEIGHT;
			return;
		}

		if(pm->cmd.upmove < 0)
		{							// duck
			pm->ps->pm_flags |= PMF_DUCKED;
		}
		else
		{							// stand up if possible
			if(pm->ps->pm_flags & PMF_DUCKED)
			{
				// try to stand up
				pm->maxs[2] = 32;
				pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
				if(!trace.allsolid)
					pm->ps->pm_flags &= ~PMF_DUCKED;
			}
		}

		if(pm->ps->pm_flags & PMF_DUCKED)
		{
			pm->maxs[2] = 16;
			pm->ps->viewheight = CROUCH_VIEWHEIGHT;
		}
		else
		{
			pm->maxs[2] = 32;
			pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
		}
		*/
	}
	
	private KinematicClosestNotMeConvexResultCallback traceAll(Vector3f startPos, Vector3f endPos)
	{
		//pm->trace(trace, start, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);
		
		Transform start = new Transform();
		Transform end = new Transform();

		start.setIdentity();
		end.setIdentity();

		/* FIXME: Handle penetration properly */
		start.origin.set(startPos);
		end.origin.set(endPos);

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
		
		return callback;
	}
	

	/**
	 * @return True if the velocity was clipped in some way
	 */
	boolean slideMove(boolean gravity)
	{
		int             bumpcount, numbumps;
		Vector3f        dir = new Vector3f();
		float           d;
		int             numplanes;
		Vector3f        primalVelocity;
		Vector3f        clipVelocity = new Vector3f();
		int             i, j, k;
		
		float           time_left;
		float           into;
		Vector3f        endVelocity = new Vector3f();

		numbumps = 4;
		
		Vector3f planes[] = new Vector3f[MAX_CLIP_PLANES];
		for(i = 0; i < planes.length; i++)
		{
			planes[i] = new Vector3f();
		}

		primalVelocity = pm.ps.getPlayerState_velocity();
		
		/*
		if(gravity)
		{
			endVelocity = pm.ps.getPlayerState_velocity();
			endVelocity.z -= pm.ps.getPlayerState_gravity() * pml.frametime;
			
			pm->ps->velocity[2] = (pm->ps->velocity[2] + endVelocity[2]) * 0.5;
			primal_velocity[2] = endVelocity[2];
			
			if(pml.groundPlane)
			{
				// slide along the ground plane
				PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);
			}
		}
		*/
		

		time_left = pml.frametime;

		// never turn against the ground plane
		if(pml.groundPlane)
		{
			numplanes = 1;
			planes[0].set(pml.groundPlaneNormal);
		}
		else
		{
			numplanes = 0;
		}

		// never turn against original velocity
		planes[numplanes] = pm.ps.getPlayerState_velocity();
		planes[numplanes].normalize();
		numplanes++;

		for(bumpcount = 0; bumpcount < numbumps; bumpcount++)
		{
			Vector3f          endClipVelocity = new Vector3f();
			
			// calculate position we are trying to move to
			Vector3f start = pm.ps.getPlayerState_origin();
			Vector3f end = new Vector3f();
			
			end.scaleAdd(time_left, pm.ps.getPlayerState_velocity(), start);

			// see if we can make it there
			KinematicClosestNotMeConvexResultCallback callback = traceAll(start, end);
			//

			if(callback.closestHitFraction == 0)
			{
				// entity is completely trapped in another solid
				
				// don't build up falling damage, but allow sideways acceleration
				//pm->ps->velocity[2] = 0;	
				return true;
			}

			if (callback.hasHit()) 
			{
				// actually covered some distance
				end.interpolate(start, end, callback.closestHitFraction);
			}
			
			pm.ps.setPlayerState_origin(end);
			
			if(!callback.hasHit()) {
				// moved the entire distance
				break;
			}

			// save entity for contact
			//PM_AddTouchEnt(trace.entityNum);

			time_left -= time_left * callback.closestHitFraction;

			if(numplanes >= MAX_CLIP_PLANES)
			{
				// this shouldn't really happen
				pm.ps.setPlayerState_velocity(new Vector3f());
				return true;
			}

			//
			// if this is the same plane we hit before, nudge velocity
			// out along it, which fixes some epsilon issues with
			// non-axial planes
			//
			for(i = 0; i < numplanes; i++)
			{
				if(callback.hitNormalWorld.dot(planes[i]) > 0.99)
				{
					Vector3f nudgedVelocity = pm.ps.getPlayerState_velocity();
					nudgedVelocity.add(callback.hitNormalWorld);
					break;
				}
			}
			if(i < numplanes)
			{
				continue;
			}
			
			planes[numplanes++].set(callback.hitNormalWorld);

			//
			// modify velocity so it parallels all of the clip planes
			//

			// find a plane that it enters
			for(i = 0; i < numplanes; i++)
			{
				Vector3f currentVelocity = pm.ps.getPlayerState_velocity(); 
				
				into = currentVelocity.dot(planes[i]);
				if(into >= 0.1)
				{
					// move doesn't interact with the plane
					continue;		
				}

				// see how hard we are hitting things
				if(-into > pml.impactSpeed)
				{
					pml.impactSpeed = -into;
				}

				// slide along the plane
				clipVelocity(currentVelocity, planes[i], clipVelocity, OVERCLIP);

				// slide along the plane
				clipVelocity(endVelocity, planes[i], endClipVelocity, OVERCLIP);

				// see if there is a second plane that the new move enters
				for(j = 0; j < numplanes; j++)
				{
					if(j == i)
					{
						continue;
						
					}
					if(clipVelocity.dot(planes[j]) >= 0.1)
					{
						// move doesn't interact with the plane
						continue;	
					}

					// try clipping the move to the plane
					clipVelocity(clipVelocity, planes[j], clipVelocity, OVERCLIP);
					clipVelocity(endClipVelocity, planes[j], endClipVelocity, OVERCLIP);

					// see if it goes back into the first clip plane
					if(clipVelocity.dot(planes[i]) >= 0)
					{
						continue;
					}

					// slide the original velocity along the crease
					dir.cross(planes[i], planes[j]);
					dir.normalize();
					
					d = dir.dot(currentVelocity);
					clipVelocity.scale(d, dir);
				
					d = dir.dot(endVelocity);
					endClipVelocity.scale(d, dir);

					// see if there is a third plane the the new move enters
					for(k = 0; k < numplanes; k++)
					{
						if(k == i || k == j)
						{
							continue;
						}
						
						if(clipVelocity.dot(planes[k]) >= 0.1)
						{
							// move doesn't interact with the plane
							continue;
						}

						// stop dead at a tripple plane interaction
						pm.ps.setPlayerState_velocity(new Vector3f());
						return true;
					}
				}

				// if we have fixed all interactions, try another move
				pm.ps.setPlayerState_velocity(clipVelocity);
				endVelocity.set(endClipVelocity);
				break;
			}
		}

		if(gravity)
		{
			pm.ps.setPlayerState_velocity(endVelocity);
		}

		// don't change velocity if in a timer (FIXME: is this correct?)
		if(pm.ps.getPlayerState_pm_time() > 0)
		{
			pm.ps.setPlayerState_velocity(primalVelocity);
		}

		return (bumpcount != 0);
	}
	
	private void stepSlideMove(boolean gravity)
	{
		//vec3_t          start_o, start_v;
		//vec3_t          down_o, down_v;
		//trace_t         trace;

	//  float       down_dist, up_dist;
	//  vec3_t      delta, delta2;
		//vec3_t          up, down;
		float           stepSize;

		Vector3f startOrigin = pm.ps.getPlayerState_origin();
		Vector3f startVelocity = pm.ps.getPlayerState_velocity();

		if(slideMove(gravity) == false)
		{
			// we got exactly where we wanted to go first try
			if(CVars.pm_debug.getBoolean())
			{
				//Engine.println(c_pmove + ":slided");
			}
			return;
		}

		Vector3f down = new Vector3f(startOrigin);
		down.z -= Config.STEPSIZE;
		
		KinematicClosestNotMeConvexResultCallback callback = traceAll(startOrigin, down);

		Vector3f up = new Vector3f(0, 0, 1);
		
		// never step up when you still have up velocity
		if(startVelocity.z > 0 && (!callback.hasHit() || callback.hitNormalWorld.dot(up) < 0.7))
		{
			return;
		}

		//Vector3f downOrigin = pm.ps.getPlayerState_origin();
		//Vector3f downVelocity = pm.ps.getPlayerState_velocity();

		up.set(startOrigin);
		up.z += Config.STEPSIZE;

		// test the player position if they were a stepheight higher
		
		callback = traceAll(startOrigin, up);

		if (callback.closestHitFraction == 0) 
		{
			// can't step up
			if(CVars.pm_debug.getBoolean())
			{
				Engine.println(c_pmove + ":bend can't step up");
			}
			return;
		}
		
		// try slidemove from this position
		if (callback.hasHit()) 
		{
			stepSize = callback.hitPointWorld.z - startOrigin.z;
			
			pm.ps.setPlayerState_origin(callback.hitPointWorld);
		}
		else
		{
			stepSize = up.z - startOrigin.z;
			
			pm.ps.setPlayerState_origin(up);
		}
		
		
		pm.ps.setPlayerState_velocity(startVelocity);
		
		slideMove(gravity);

		// push down the final amount
		down = pm.ps.getPlayerState_origin();
		down.z -= stepSize;
		
		
		callback = traceAll(startOrigin, down);

		if(callback.hasHit())
		{
			pm.ps.setPlayerState_origin(callback.hitPointWorld);
		}
		else
		{
			pm.ps.setPlayerState_origin(down);
		}
		
		if(callback.closestHitFraction < 1.0)
		{
			clipVelocity(pm.ps.getPlayerState_velocity(), callback.hitNormalWorld, startVelocity, OVERCLIP);
			
			pm.ps.setPlayerState_velocity(startVelocity);
		}
		
	
		/*
		// if the down trace can trace back to the original position directly, don't step
		pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, start_o, pm->ps->clientNum, pm->tracemask);
		if(trace.fraction == 1.0)
		{
			// use the original move
			VectorCopy(down_o, pm->ps->origin);
			VectorCopy(down_v, pm->ps->velocity);
			if(pm->debugLevel)
			{
				Com_Printf("%i:bend\n", c_pmove);
			}
		}
		else
	*/
		
		{
			/*
			// use the step move
			float           delta;

			delta = pm->ps->origin[2] - start_o[2];
			if(delta > 2)
			{
				if(delta < 7)
				{
					PM_AddEvent(EV_STEP_4);
				}
				else if(delta < 11)
				{
					PM_AddEvent(EV_STEP_8);
				}
				else if(delta < 15)
				{
					PM_AddEvent(EV_STEP_12);
				}
				else
				{
					PM_AddEvent(EV_STEP_16);
				}
			}
			*/
			
			if(CVars.pm_debug.getBoolean())
			{
				Engine.println(c_pmove + ":stepped");
			}
		}
		
	}
	
	void spectatorMove(boolean clipAgainstWorld)
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
				Vector3f start = pm.ps.getPlayerState_origin();
				Vector3f end = new Vector3f();

				end.scaleAdd(pml.frametime, newVelocity, start);
				
				KinematicClosestNotMeConvexResultCallback callback = traceAll(start, end);
				
				if (callback.hasHit()) 
				{
					end.interpolate(start, end, callback.closestHitFraction);
				}
				
				pm.ps.setPlayerState_origin(end);
			}
			else
			{
				Vector3f origin = pm.ps.getPlayerState_origin();
				
				newVelocity.scale(pml.frametime);
				origin.add(newVelocity);
				pm.ps.setPlayerState_origin(origin);
			}
		}
	}
	
	void flyMove()
	{
		float           wishspeed;
		Vector3f        wishvel;
		Vector3f		wishdir = new Vector3f();
		float           scale;

		
		// normal slowdown
		applyFriction();

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
		
		if(wishspeed > 0)
		{
			wishdir.normalize();
		}
		wishspeed *= scale;
		
		Vector3f newVelocity = accelerate(wishdir, wishspeed, CVars.pm_flyaccelerate.getValue());
		
		if(newVelocity != null)
		{
			if(Float.isNaN(newVelocity.x) || Float.isNaN(newVelocity.y) || Float.isNaN(newVelocity.z))
				throw new RuntimeException("newVelocity member(s) became NaN");
			
			pm.ps.setPlayerState_velocity(newVelocity);
			
			stepSlideMove(false);
			
			/*
			{
				Vector3f start = pm.ps.getPlayerState_origin();
				Vector3f end = new Vector3f();

				end.scaleAdd(pml.frametime, newVelocity, start);
				
				KinematicClosestNotMeConvexResultCallback callback = traceAll(start, end);
				
				if (callback.hasHit()) 
				{
					end.interpolate(start, end, callback.closestHitFraction);
				}
				
				pm.ps.setPlayerState_origin(end);
			}
			*/
		}
	}


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
