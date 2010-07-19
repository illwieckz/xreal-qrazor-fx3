package xreal.common;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import javax.vecmath.Matrix3f;
import javax.vecmath.Matrix4f;
import javax.vecmath.Vector3f;

import com.bulletphysics.collision.broadphase.BroadphasePair;
import com.bulletphysics.collision.dispatch.CollisionObject;
import com.bulletphysics.collision.dispatch.CollisionWorld;
import com.bulletphysics.collision.dispatch.PairCachingGhostObject;
import com.bulletphysics.collision.narrowphase.ManifoldPoint;
import com.bulletphysics.collision.narrowphase.PersistentManifold;
import com.bulletphysics.collision.shapes.ConvexShape;
import com.bulletphysics.collision.shapes.SphereShape;
import com.bulletphysics.dynamics.ActionInterface;
import com.bulletphysics.dynamics.RigidBody;
import com.bulletphysics.linearmath.IDebugDraw;
import com.bulletphysics.linearmath.Transform;

import xreal.Angle3f;
import xreal.CVars;
import xreal.Color;
import xreal.Engine;
import xreal.EntityStateAccess;
import xreal.PlayerStateAccess;
import xreal.UserCommand;
import xreal.client.game.ClientGame;
import xreal.client.game.util.RenderUtils;
import xreal.server.game.GameEntity;


/**
 * 
 * @author Robert Beckebans
 */
public class PlayerController implements ActionInterface {
	
	// all of the locals will be zeroed before each
	// pmove, just to make damn sure we don't have
	// any differences when running on client or server
	
	private class ContactInfo
	{
		public Vector3f	position;
		public Vector3f	normal;
		
		public ContactInfo(Vector3f position, Vector3f normal)
		{
			super();
			this.position = position;
			this.normal = normal;
		}
	}
	
	private class PlayerControllerLocals
	{
		public PlayerControllerLocals()
		{
			gravityVector = new Vector3f(CVars.g_gravityX.getValue(), CVars.g_gravityY.getValue(), CVars.g_gravityZ.getValue());
			// gravityVector = new Vector3f(0, 0, CVars.g_gravityZ.getValue());

			gravityNormal = new Vector3f(gravityVector);
			gravityNormal.normalize();

			gravityNormalFlipped = new Vector3f(gravityNormal);
			gravityNormalFlipped.negate();
		}

		public Vector3f										forward				= new Vector3f(0, 0, 0);
		public Vector3f										right				= new Vector3f(0, 0, 0);
		public Vector3f										up					= new Vector3f(0, 0, 0);

		public float										frameTime;

		public int											msec;

		public boolean										walking;
		public boolean										groundPlane;
		public KinematicClosestNotMeConvexResultCallback	groundTrace;

		public float										impactSpeed;

		public Vector3f										previousOrigin		= new Vector3f(0, 0, 0);
		public Vector3f										previousVelocity	= new Vector3f(0, 0, 0);
		public int											previousWaterlevel;

		public final Vector3f								gravityVector;
		public final Vector3f								gravityNormal;
		public final Vector3f								gravityNormalFlipped;

		private Queue<ContactInfo>							contactInfos		= new LinkedList<ContactInfo>();
	}

	private PlayerControllerLocals			pml				= new PlayerControllerLocals();

	private static final int				MAX_CLIP_PLANES	= 5;
	private static final float				OVERCLIP		= 1.001f;

	/** can't walk on very steep slopes */
	private static final float				MIN_WALK_NORMAL	= 0.7f;
	private static final float				JUMP_VELOCITY	= 270;
	private static final float				TIMER_LAND		= 130;
	private static final float				TIMER_GESTURE	= (34 * 66 + 50);
	
	
//	private Queue<PlayerMove> playerMovements = new LinkedList<PlayerMove>();
	private PlayerMove pmNewest;
	private PlayerMove pm;
	
	
	private final PairCachingGhostObject ghostObject;
	private final ConvexShape convexShape;
	private final CollisionWorld collisionWorld;
	
	private int c_pmove;
	
	public PlayerController(PairCachingGhostObject ghostObject, ConvexShape convexShape, CollisionWorld collisionWorld)
	{
		this.ghostObject = ghostObject;
		this.convexShape = convexShape;
		this.collisionWorld = collisionWorld;
	}
	
	/*
	public void addPlayerMove(PlayerMove pmove)
	{
		playerMovements.add(pmove);
	}
	*/
	
	public void setPlayerMove(PlayerMove pmove)
	{
		pmNewest = pmove;
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
		
		if(pmNewest != null){
			movePlayer(pmNewest);
		}
	}
	
	private void renderContactInfos()
	{
		if(pm.runningOnClient && pm.debugLevel > 0 && CVars.pm_drawContacts.getBoolean())
		{
			while (!pml.contactInfos.isEmpty()) {
				ContactInfo ci = pml.contactInfos.remove();
				
				Matrix3f rotation = new Matrix3f();
				Angle3f angles = new Angle3f(ci.normal);
				angles.get(rotation);
				
				Matrix4f transform = new Matrix4f(rotation, ci.position, 1);
			
			
				try
				{
					// draw position
					Vector3f mins = new Vector3f(-1, -1, -1);
					Vector3f maxs = new Vector3f(1, 1, 1);
					
					RenderUtils.renderBox(transform, mins, maxs, Color.Blue, ClientGame.getMedia().debugPlayerAABB_twoSided);
					
					// draw normal
					mins.set(-0.1f, -0.1f, -0.1f);
					maxs.set(10, 0.1f, 0.1f);
					
					if(isNormalTooSteep(ci.normal))
					{
						RenderUtils.renderBox(transform, mins, maxs, Color.Red, ClientGame.getMedia().debugPlayerAABB_twoSided);
					}
					else
					{
						RenderUtils.renderBox(transform, mins, maxs, Color.Green, ClientGame.getMedia().debugPlayerAABB_twoSided);
					}
					
				}
				catch(Exception e)
				{
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
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
			
			movePlayerSingle(pmove);
			
			renderContactInfos();

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
		//if(pm.ps->stats[STAT_HEALTH] > 0 && (pm.cmd.buttons & (UserCommand.BUTTON_ATTACK | UserCommand.BUTTON_ATTACK2 | UserCommand.BUTTON_USE_HOLDABLE)) > 0)
		{
			pm.ps.delPlayerState_pm_flags(PlayerMovementFlags.RESPAWNED);
		}

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
		pml.previousOrigin.set(pm.ps.getPlayerState_origin()); 

		// save old velocity for crashlanding
		pml.previousVelocity.set(pm.ps.getPlayerState_velocity());
		
		//Engine.println("PlayerController.movePlayerSingle() 4...");

		pml.frameTime = pml.msec * 0.001f;
		
		// update ghostObject world transform
		setOrigin(pm.ps.getPlayerState_origin());

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
				
				checkDuck();
				flyMove();
				//spectatorMove(true);
				
				dropTimers();
				return;
				
			case NOCLIP:
				updateViewAngles(pm.ps, pm.cmd);
				viewAngles = pm.ps.getPlayerState_viewAngles();
				viewAngles.getVectors(pml.forward, pml.right, pml.up);
				spectatorMove(false);
				dropTimers();
				return;
				
			case FREEZE:
			case INTERMISSION:
			case SPINTERMISSION:
				// no movement at all
				return;
				
			case NORMAL:
			{
				updateViewAngles(pm.ps, pm.cmd);
				viewAngles = pm.ps.getPlayerState_viewAngles();
				viewAngles.getVectors(pml.forward, pml.right, pml.up);
				
				// set watertype, and waterlevel
				//PM_SetWaterLevel();
				//pml.previous_waterlevel = pmove->waterlevel;

				// set mins, maxs, and viewheight
				checkDuck();

				// set groundentity
				groundTrace();

				// update the viewangles
				updateViewAngles(pm.ps, pm.cmd);

				if(pm_type == PlayerMovementType.DEAD)
				{
					//deadMove();
				}

				dropTimers();

				/*
			#ifdef MISSIONPACK
				if(pm.ps.powerups[PW_INVULNERABILITY])
				{
					PM_InvulnerabilityMove();
				}
				else
			#endif
				if(pm.ps.powerups[PW_FLIGHT])
				{
					// flight powerup doesn't allow jump and has different friction
					PM_FlyMove();
				}
				else if(pm.ps.pm_flags & PMF_GRAPPLE_PULL)
				{
					PM_GrappleMove();

					// we can wiggle a bit
					PM_AirMove();
				}
				else if(pm.ps.pm_flags & PMF_TIME_WATERJUMP)
				{
					PM_WaterJumpMove();
				}
				else if(pm->waterlevel > 1)
				{
					// swimming
					PM_WaterMove();
				}
				else */if(pml.walking)
				{
					/*
					if(pm.ps.pm_flags & PMF_WALLCLIMBING)
					{
						// TA: walking on any surface
						PM_ClimbMove();
					}
					else
					*/
					{
						// walking on ground
						walkMove();
					}
				}
				else
				{
					// airborne
					airMove();
				}

				//PM_Animate();

				// set groundentity, watertype, and waterlevel
				groundTrace();

				//TA: must update after every GroundTrace() - yet more clock cycles down the drain :( (14 vec rotations/frame)
				// update the viewangles
				updateViewAngles(pm.ps, pm.cmd);

				//PM_SetWaterLevel();

				// weapons
				//PM_Weapon();

				// torso animation
				//PM_TorsoAnimation();

				// footstep events / legs animations
				//PM_Footsteps();

				// entering / leaving water splashes
				//PM_WaterEvents();

				// snap some parts of playerstate to save network bandwidth
				/*
				if(pm->fixedPmove)
				{
					// the new way: don't care so much about 6 bytes/frame
					// or so of network bandwidth, and add some mostly framerate-
					// independent error to make up for the lack of rounding error

					// halt if not going fast enough (0.5 units/sec)
					if(VectorLengthSquared(pm.ps.velocity) < 0.25f)
					{
						VectorClear(pm.ps.velocity);
					}
					else
					{
						int             i;
						float           fac;

						fac = (float)pml.msec / (1000.0f / (float)pm->fixedPmoveFPS);

						// add some error...
						for(i = 0; i < 3; i++)
						{
							// ...if the velocity in this direction changed enough
							if(fabs(pm.ps.velocity[i] - pml.previous_velocity[i]) > 0.5f / fac)
							{
								if(pm.ps.velocity[i] < 0)
								{
									pm.ps.velocity[i] -= 0.5f * fac;
								}
								else
								{
									pm.ps.velocity[i] += 0.5f * fac;
								}
							}
						}

						// we can stand a little bit of rounding error for the sake
						// of lower bandwidth
						VectorScale(pm.ps.velocity, 64.0f, pm.ps.velocity);
						SnapVector(pm.ps.velocity);
						VectorScale(pm.ps.velocity, 1.0f / 64.0f, pm.ps.velocity);
					}
				}
				else
				*/
				{
					Vector3f snappedVelocity = pm.ps.getPlayerState_velocity();
					
					snappedVelocity.snap();
					pm.ps.setPlayerState_velocity(snappedVelocity);
				}
			}
		}
	}
	
	/**
	 * This can be used as another entry point when only the viewangles
	 * are being updated instead of a full move.
	 */
	public static void updateViewAngles(PlayerStateAccess ps, UserCommand cmd)
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
	
	private void setOrigin(Vector3f origin)
	{
		pm.ps.setPlayerState_origin(origin);
		
		// update ghost object as well
		Transform transform = ghostObject.getWorldTransform(new Transform());
		transform.origin.set(origin);
		
		ghostObject.setWorldTransform(transform);
	}
	
	/**
	 * Returns the scale factor to apply to cmd movements.
	 * 
	 * This allows the clients to use axial -127 to 127 values for all directions
	 * without getting a sqrt(2) distortion in speed.
	 * 
	 * @return The scale factor.
	 */
	private float calculateUserCommandScale(final UserCommand cmd)
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
	 * Determine the rotation of the legs relative
	 * to the facing direction.
	 */
	private void setMovementDir()
	{
		/*
	// Ridah, changed this for more realistic angles (at the cost of more network traffic?)
	#if 0
		float           speed;
		vec3_t          moved;
		int             moveyaw;

		VectorSubtract(pm.ps.origin, pml.previous_origin, moved);

		if((pm.cmd.forwardmove || pm.cmd.rightmove) && (pm.ps.groundEntityNum != ENTITYNUM_NONE) && (speed = VectorLength(moved)) && (speed > pml.frametime * 5))	// if moving slower than 20 units per second, just face head angles
		{

			vec3_t          dir;


			VectorNormalize2(moved, dir);
			VectorToAngles(dir, dir);

			moveyaw = (int)AngleDelta(dir[YAW], pm.ps.viewangles[YAW]);


			if(pm.cmd.forwardmove < 0)
				moveyaw = (int)AngleNormalize180(moveyaw + 180);


			if(abs(moveyaw) > 75)
			{
				if(moveyaw > 0)
				{
					moveyaw = 75;
				}
				else
				{
					moveyaw = -75;

				}
			}

			pm.ps.setPlayerState_movementDir((signed char)moveyaw;
		}
		else
		{
			pm.ps.setPlayerState_movementDir(0;
		}

	#else
		*/
		if(pm.cmd.forwardmove > 0 || pm.cmd.rightmove > 0)
		{
			if(pm.cmd.rightmove == 0 && pm.cmd.forwardmove > 0)
			{
				pm.ps.setPlayerState_movementDir(0);
			}
			else if(pm.cmd.rightmove < 0 && pm.cmd.forwardmove > 0)
			{
				pm.ps.setPlayerState_movementDir(1);
			}
			else if(pm.cmd.rightmove < 0 && pm.cmd.forwardmove == 0)
			{
				pm.ps.setPlayerState_movementDir(2);
			}
			else if(pm.cmd.rightmove < 0 && pm.cmd.forwardmove < 0)
			{
				pm.ps.setPlayerState_movementDir(3);
			}
			else if(pm.cmd.rightmove == 0 && pm.cmd.forwardmove < 0)
			{
				pm.ps.setPlayerState_movementDir(4);
			}
			else if(pm.cmd.rightmove > 0 && pm.cmd.forwardmove < 0)
			{
				pm.ps.setPlayerState_movementDir(5);
			}
			else if(pm.cmd.rightmove > 0 && pm.cmd.forwardmove == 0)
			{
				pm.ps.setPlayerState_movementDir(6);
			}
			else if(pm.cmd.rightmove > 0 && pm.cmd.forwardmove > 0)
			{
				pm.ps.setPlayerState_movementDir(7);
			}
		}
		else
		{
			// if they aren't actively going directly sideways,
			// change the animation to the diagonal so they
			// don't stop too crooked
			if(pm.ps.getPlayerState_movementDir() == 2)
			{
				pm.ps.setPlayerState_movementDir(1);
			}
			else if(pm.ps.getPlayerState_movementDir() == 6)
			{
				pm.ps.setPlayerState_movementDir(7);
			}
		}
	//#endif
	}
	
	private boolean isNormalTooSteep(Vector3f normal)
	{
		return (pml.gravityNormalFlipped.dot(normal) < MIN_WALK_NORMAL);
	}
	
	private void dropTimers()
	{
		// drop misc timing counter
		if(pm.ps.getPlayerState_pm_time() > 0)
		{
			if(pml.msec >= pm.ps.getPlayerState_pm_time())
			{
				pm.ps.delPlayerState_pm_flags(PlayerMovementFlags.ALL_TIMES);
				pm.ps.setPlayerState_pm_time(0);
			}
			else
			{
				pm.ps.setPlayerState_pm_time(pm.ps.getPlayerState_pm_time() - pml.msec);
			}
		}

		// drop animation counter
		/*
		if(pm->ps->legsTimer > 0)
		{
			pm->ps->legsTimer -= pml.msec;
			if(pm->ps->legsTimer < 0)
			{
				pm->ps->legsTimer = 0;
			}
		}

		if(pm->ps->torsoTimer > 0)
		{
			pm->ps->torsoTimer -= pml.msec;
			if(pm->ps->torsoTimer < 0)
			{
				pm->ps->torsoTimer = 0;
			}
		}
		*/
	}
	
	/**
	 * Handles user intended acceleration.
	 * 
	 * @param wishdir
	 * @param wishspeed
	 * @param accel
	 */
	private void accelerate(Vector3f wishdir, float wishspeed, float accel)
	{
		Vector3f currentvel = pm.ps.getPlayerState_velocity();
		//float currentspeed = currentvel.length();
		//float currentspeed = wishdir.length();
		float currentspeed = currentvel.dot(wishdir);
		float addspeed = wishspeed - currentspeed;
		if(addspeed <= 0)
		{
			return;
		}
		
		if(Float.isNaN(currentspeed))
			throw new RuntimeException("currentspeed is NaN");
		
		float accelspeed = accel * pml.frameTime * wishspeed;
		if(accelspeed > addspeed)
		{
			accelspeed = addspeed;
		}

		Vector3f newVelocity = new Vector3f();
		newVelocity.scaleAdd(accelspeed, wishdir, currentvel);
		
		//pm.ps.setPlayerState_velocity(newVelocity);
		//return newVelocity;
		
		if(Float.isNaN(newVelocity.x) || Float.isNaN(newVelocity.y) || Float.isNaN(newVelocity.z))
			throw new RuntimeException("newVelocity member(s) became NaN");
			
		pm.ps.setPlayerState_velocity(newVelocity);
	
		/*
		// proper way (avoids strafe jump maxspeed bug), but feels bad
		vec3_t          wishVelocity;
		vec3_t          pushDir;
		float           pushLen;
		float           canPush;

		VectorScale(wishdir, wishspeed, wishVelocity);
		VectorSubtract(wishVelocity, pm.ps.velocity, pushDir);
		pushLen = VectorNormalize(pushDir);

		canPush = accel * pml.frametime * wishspeed;
		if(canPush > pushLen)
		{
			canPush = pushLen;
		}

		VectorMA(pm.ps.velocity, canPush, pushDir, pm.ps.velocity);
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
	private void clipVelocity(Vector3f in, Vector3f normal, Vector3f out, float overbounce)
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
		float           speed, newspeed;
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
		if(speed < 1)				// && pm.ps.pm_type != PM_SPECTATOR && pm.ps.pm_type != PM_NOCLIP)
		{
			vel.x = 0;
			vel.y = 0;				// allow sinking underwater
			// FIXME: still have z friction underwater?
			
			pm.ps.setPlayerState_velocity(vel);
			return;
		}

		drop = 0;

		// apply ground friction
		if(pm.waterlevel <= 1)
		{
			if(pml.walking /* && !(pml.groundTrace.surfaceFlags & SURF_SLICK) */)
			{
				// if getting knocked back, no friction
				if(!pm.ps.hasPlayerState_pm_flags(PlayerMovementFlags.TIME_KNOCKBACK))
				{
					float control = speed < CVars.pm_stopSpeed.getValue() ? CVars.pm_stopSpeed.getValue() : speed;
					drop += control * CVars.pm_friction.getValue() * pml.frameTime;
				}
			}
		}

		/*
		// apply water friction even if just wading
		if(pm->waterlevel)
		{
			drop += speed * pm_waterfriction * pm->waterlevel * pml.frametime;
		}

		// apply flying friction
		if(pm.ps.powerups[PW_FLIGHT])
		{
			drop += speed * pm_flightfriction * pml.frametime;
		}
		*/

		PlayerMovementType pmType = pm.ps.getPlayerState_pm_type();
		
		if(pmType == PlayerMovementType.SPECTATOR)
		{
			drop += speed * CVars.pm_spectatorFriction.getValue() * pml.frameTime;
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

	Sets mins, maxs, and pm.ps.viewheight
	==============
	*/
	private void checkDuck()
	{
		//trace_t         trace;

		/*
		if(pm.ps.powerups[PW_INVULNERABILITY])
		{
			if(pm.ps.pm_flags & PMF_INVULEXPAND)
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
			pm.ps.pm_flags |= PMF_DUCKED;
			pm.ps.viewheight = CROUCH_VIEWHEIGHT;
			return;
		}
		pm.ps.pm_flags &= ~PMF_INVULEXPAND;
		*/

		/*
		pm->mins[0] = -15;
		pm->mins[1] = -15;

		pm->maxs[0] = 15;
		pm->maxs[1] = 15;

		pm->mins[2] = MINS_Z;

		if(pm.ps.pm_type == PM_DEAD)
		{
			pm->maxs[2] = -8;
			pm.ps.viewheight = DEAD_VIEWHEIGHT;
			return;
		}

		if(pm.cmd.upmove < 0)
		{							// duck
			pm.ps.pm_flags |= PMF_DUCKED;
		}
		else
		{							// stand up if possible
			if(pm.ps.pm_flags & PMF_DUCKED)
			{
				// try to stand up
				pm->maxs[2] = 32;
				pm->trace(&trace, pm.ps.origin, pm->mins, pm->maxs, pm.ps.origin, pm.ps.clientNum, pm->tracemask);
				if(!trace.allsolid)
					pm.ps.pm_flags &= ~PMF_DUCKED;
			}
		}

		if(pm.ps.pm_flags & PMF_DUCKED)
		{
			pm->maxs[2] = 16;
			pm.ps.viewheight = CROUCH_VIEWHEIGHT;
		}
		else
		{
			pm->maxs[2] = 32;
			pm.ps.viewheight = DEFAULT_VIEWHEIGHT;
		}
		*/
	}
	
	
	private boolean checkJump()
	{
		if(pm.ps.hasPlayerState_pm_flags(PlayerMovementFlags.RESPAWNED))
		{
			// don't allow jump until all buttons are up
			return false;			
		}

		if(pm.cmd.upmove < 10)
		{
			// not holding jump
			return false;
		}

		// must wait for jump to be released
		if(pm.ps.hasPlayerState_pm_flags(PlayerMovementFlags.JUMP_HELD))
		{
			// clear upmove so cmdscale doesn't lower running speed
			pm.cmd.upmove = 0;
			
			return false;
		}

		pml.groundPlane = false;	// jumping away
		pml.walking = false;
		
		pm.ps.addPlayerState_pm_flags(PlayerMovementFlags.JUMP_HELD);
		pm.ps.setPlayerState_groundEntityNum(Engine.ENTITYNUM_NONE);
		
		Vector3f velocity = pm.ps.getPlayerState_velocity();
		
		//velocity.z = JUMP_VELOCITY;

		Vector3f addVelocity = new Vector3f(pml.gravityNormal);
		
		//addVelocity.scale(-2.0f * CVars.pm_jumpHeight.getValue());
		//addVelocity.scale(addVelocity.length());
		
		//addVelocity.scale(-CVars.pm_jumpHeight.getValue());
		addVelocity.scale(-JUMP_VELOCITY);
		
		velocity.add(addVelocity);
		
		pm.ps.setPlayerState_velocity(velocity);
		
		// TODO PM_AddEvent(EV_JUMP);
		
		if(pm.cmd.forwardmove >= 0)
		{
			// TODO PM_ForceLegsAnim(LEGS_JUMP);
			pm.ps.delPlayerState_pm_flags(PlayerMovementFlags.BACKWARDS_JUMP);
		}
		else
		{
			//TODO PM_ForceLegsAnim(LEGS_JUMPB);
			pm.ps.addPlayerState_pm_flags(PlayerMovementFlags.BACKWARDS_JUMP);
		}

		return true;
	}
	
	private boolean correctAllSolid()
	{
		int             i, j, k;

		if(pm.debugLevel == 1)
		{
			Engine.println(c_pmove + ":allsolid");
		}
		
		Vector3f playerOrigin = pm.ps.getPlayerState_origin();

		// jitter around
		for(i = -1; i <= 1; i++)
		{
			for(j = -1; j <= 1; j++)
			{
				for(k = -1; k <= 1; k++)
				{
					Vector3f point = new Vector3f(playerOrigin);
					
					point.x += (float)i;
					point.y += (float)j;
					point.z += (float)k;
					
					KinematicClosestNotMeConvexResultCallback trace = traceAll(point, point);
					
					if(!trace.hasHit() || trace.closestHitFraction > 0)
					{
						point.x = playerOrigin.x;
						point.y = playerOrigin.y;
						point.z = playerOrigin.z - 0.25f;
						
						trace = traceAll(point, point);
						
						pml.groundTrace = trace;
						return true;
					}
				}
			}
		}

		pm.ps.setPlayerState_groundEntityNum(Engine.ENTITYNUM_NONE);
		pml.groundPlane = false;
		pml.walking = false;

		return false;
	}
	
	protected boolean recoverFromPenetration()
	{
		boolean penetration = false;

		collisionWorld.getDispatcher().dispatchAllCollisionPairs(ghostObject.getOverlappingPairCache(), collisionWorld.getDispatchInfo(),
				collisionWorld.getDispatcher());

		Vector3f currentPosition = pm.ps.getPlayerState_origin();
		//currentPosition.set(ghostObject.getWorldTransform(new Transform()).origin);
		
		if(pm.debugLevel == 1)
		{
			Engine.println("number of overlapping pairs = " + ghostObject.getOverlappingPairCache().getNumOverlappingPairs());
		}

		float maxPen = 0.0f;
		for(int i = 0; i < ghostObject.getOverlappingPairCache().getNumOverlappingPairs(); i++)
		{
			//manifoldArray.clear();

			BroadphasePair collisionPair = ghostObject.getOverlappingPairCache().getOverlappingPairArray().get(i);

			// keep track of the contact manifolds
			List<PersistentManifold> manifoldArray = new ArrayList<PersistentManifold>();
			if(collisionPair.algorithm != null)
			{
				collisionPair.algorithm.getAllContactManifolds(manifoldArray);
			}
			
			if(pm.debugLevel == 1)
			{
				Engine.println("number of contact manifolds = " + manifoldArray.size());
			}

			for(int j = 0; j < manifoldArray.size(); j++)
			{
				PersistentManifold manifold = manifoldArray.get(j);
				float directionSign = manifold.getBody0() == ghostObject ? -1.0f : 1.0f;
				for(int p = 0; p < manifold.getNumContacts(); p++)
				{
					ManifoldPoint pt = manifold.getContactPoint(p);

					if(pt.getDistance() < 0.0f)
					{
						if(pt.getDistance() < maxPen)
						{
							maxPen = pt.getDistance();
							
							/*
							touchingNormal.set(pt.normalWorldOnB);// ??
							touchingNormal.scale(directionSign);
							*/
							
							pml.groundTrace.hitNormalWorld.set(pt.normalWorldOnB);
							pml.groundTrace.hitNormalWorld.scale(directionSign);
						}

						currentPosition.scaleAdd(directionSign * pt.getDistance() * 0.2f, pt.normalWorldOnB, currentPosition);

						penetration = true;
					}
					else
					{
						if(pm.debugLevel == 1)
						{
							Engine.println("touching: " + pt.getDistance());
						}
					}
				}

				// manifold->clearManifold();
			}
		}

		/*
		Transform newTrans = ghostObject.getWorldTransform(new Transform());
		newTrans.origin.set(currentPosition);
		ghostObject.setWorldTransform(newTrans);
		*/
		setOrigin(currentPosition);
		
		if(pm.debugLevel == 1)
		{
			/*
			Engine.print(String.format("m_touchingNormal = %f,%f,%f\n", pml.groundTrace.hitNormalWorld.x,
																		pml.groundTrace.hitNormalWorld.y,
																		pml.groundTrace.hitNormalWorld.z));
																		*/
			
			Engine.println(c_pmove +  ":recoverFromPenetration: penetration=" + penetration + ", touchingNormal=" + pml.groundTrace.hitNormalWorld);
		}
		 

		return penetration;
	}
	
	protected boolean evaluateContacts()
	{
		boolean penetration = false;

		collisionWorld.getDispatcher().dispatchAllCollisionPairs(ghostObject.getOverlappingPairCache(), collisionWorld.getDispatchInfo(),
				collisionWorld.getDispatcher());

		Vector3f currentPosition = pm.ps.getPlayerState_origin();
		
		if(pm.debugLevel == 1)
		{
			Engine.println("number of overlapping pairs = " + ghostObject.getOverlappingPairCache().getNumOverlappingPairs());
		}
		
		pml.groundTrace.hitNormalWorld.set(0, 0, 0);

		float maxPen = 0.0f;
		for(int i = 0; i < ghostObject.getOverlappingPairCache().getNumOverlappingPairs(); i++)
		{
			//manifoldArray.clear();

			BroadphasePair collisionPair = ghostObject.getOverlappingPairCache().getOverlappingPairArray().get(i);

			// keep track of the contact manifolds
			List<PersistentManifold> manifoldArray = new ArrayList<PersistentManifold>();
			if(collisionPair.algorithm != null)
			{
				collisionPair.algorithm.getAllContactManifolds(manifoldArray);
			}
			
			
			if(pm.debugLevel == 1)
			{
				//Engine.println("number of contact manifolds = " + manifoldArray.size());
			}

			for(int j = 0; j < manifoldArray.size(); j++)
			{
				PersistentManifold manifold = manifoldArray.get(j);
				float directionSign = manifold.getBody0() == ghostObject ? -1.0f : 1.0f;
				for(int p = 0; p < manifold.getNumContacts(); p++)
				{
					ManifoldPoint pt = manifold.getContactPoint(p);
					
					if(pt.getDistance() < 0.0f)
					{
						if(pt.getDistance() < maxPen)
						{
							maxPen = pt.getDistance();
							
							// HACK
						
						}

						currentPosition.scaleAdd(directionSign * pt.getDistance() * 1.0f, pt.normalWorldOnB, currentPosition);
						
						//penetration = true;
					}
					else
					{
						if(pm.debugLevel == 1)
						{
							Engine.println("touching: " + pt.getDistance());
						}
					}
					
					pml.groundTrace.hitNormalWorld.scaleAdd(directionSign, pt.normalWorldOnB, pml.groundTrace.hitNormalWorld);
				}

				// manifold->clearManifold();
			}
		}

		pml.groundTrace.hitNormalWorld.normalize();
		
		if(isNormalTooSteep(pml.groundTrace.hitNormalWorld))
		{
			penetration = true;
		}
		
		setOrigin(currentPosition);
		
		if(pm.debugLevel == 1)
		{
			/*
			Engine.print(String.format("m_touchingNormal = %f,%f,%f\n", pml.groundTrace.hitNormalWorld.x,
																		pml.groundTrace.hitNormalWorld.y,
																		pml.groundTrace.hitNormalWorld.z));
																		*/
			
			//Engine.println(c_pmove +  ":recoverFromPenetration: penetration=" + penetration + ", touchingNormal=" + pml.groundTrace.hitNormalWorld);
			
			Engine.println(c_pmove +  ":touchingNormal=" + pml.groundTrace.hitNormalWorld);
		}
		 

		return penetration;
	}
	
	
	/**
	 * The ground trace didn't hit a surface, so we are in freefall.
	 */
	private void groundTraceMissed()
	{
		if(pm.ps.getPlayerState_groundEntityNum() != Engine.ENTITYNUM_NONE)
		{
			// we just transitioned into freefall
			if(pm.debugLevel == 1)
			{
				Engine.print(c_pmove + ":lift\n");
			}

			// if they aren't in a jumping animation and the ground is a ways away, force into it
			// if we didn't do the trace, the player would be backflipping down staircases
			Vector3f playerOrigin = pm.ps.getPlayerState_origin();
			
			Vector3f point = new Vector3f();
			point.scaleAdd(64, pml.gravityNormal, playerOrigin);
			
			KinematicClosestNotMeConvexResultCallback trace = traceAll(playerOrigin, point);
			
			if(!trace.hasHit())
			{
				if(pm.cmd.forwardmove >= 0)
				{
					// TODO PM_ForceLegsAnim(LEGS_JUMP);
					pm.ps.delPlayerState_pm_flags(PlayerMovementFlags.BACKWARDS_JUMP);
				}
				else
				{
					//TODO PM_ForceLegsAnim(LEGS_JUMPB);
					pm.ps.addPlayerState_pm_flags(PlayerMovementFlags.BACKWARDS_JUMP);
				}
			}
		}

		pm.ps.setPlayerState_groundEntityNum(Engine.ENTITYNUM_NONE);
		pml.groundPlane = false;
		pml.walking = false;
	}
	
	private void groundTrace()
	{
		Vector3f playerOrigin = pm.ps.getPlayerState_origin();
		
		Vector3f point = new Vector3f();
		point.scaleAdd(0.25f, pml.gravityNormal, playerOrigin);
		
		KinematicClosestNotMeConvexResultCallback trace = traceAll(playerOrigin, point);
		pml.groundTrace = trace;
		
		// do something corrective if the trace starts in a solid...
		/*
		if(trace.closestHitFraction == 0)
		{
			if(correctAllSolid())
				return;
		}
		*/

		// if the trace didn't hit anything, we are in free fall
		if(!trace.hasHit())
		{
			groundTraceMissed();
			
			pml.groundPlane = false;
			pml.walking = false;
			return;
		}
		
		pml.contactInfos.add(new ContactInfo(pml.groundTrace.hitPointWorld, pml.groundTrace.hitNormalWorld));

		// check if getting thrown off the ground
		Vector3f playerVelocity = pm.ps.getPlayerState_velocity();
		
		if(playerVelocity.dot(pml.gravityNormalFlipped) > 0 && playerVelocity.dot(trace.hitNormalWorld) > 10)
		{
			if(pm.debugLevel == 1)
			{
				Engine.print(c_pmove + ":kickoff\n");
			}
			
			// go into jump animation
			if(pm.cmd.forwardmove >= 0)
			{
				// TODO PM_ForceLegsAnim(LEGS_JUMP);
				pm.ps.delPlayerState_pm_flags(PlayerMovementFlags.BACKWARDS_JUMP);
			}
			else
			{
				//TODO PM_ForceLegsAnim(LEGS_JUMPB);
				pm.ps.addPlayerState_pm_flags(PlayerMovementFlags.BACKWARDS_JUMP);
			}

			pm.ps.setPlayerState_groundEntityNum(Engine.ENTITYNUM_NONE);
			pml.groundPlane = false;
			pml.walking = false;
			return;
		}

		// slopes that are too steep will not be considered onground
		if(isNormalTooSteep(trace.hitNormalWorld))
		{
			if(pm.debugLevel == 1)
			{
				Engine.print(c_pmove + ":steep\n");
			}
			// FIXME: if they can't slide down the slope, let them
			// walk (sharp crevices)
			
			/*
			int numPenetrationLoops = 0;
			
			//touchingContact = false;
			
			while(evaluateContacts())
			{
				numPenetrationLoops++;
				
				//touchingContact = true;
				
				if(numPenetrationLoops > 4)
				{
					Engine.println(c_pmove + ":character could not recover from penetration = " + numPenetrationLoops);
					break;
				}
			}
			*/
			
			pm.ps.setPlayerState_groundEntityNum(Engine.ENTITYNUM_NONE);
			pml.groundPlane = true;
			pml.walking = false;
			return;
		}

		pml.groundPlane = true;
		pml.walking = true;

		// hitting solid ground will end a waterjump
		/*
		if(pm.ps.pm_flags & PMF_TIME_WATERJUMP)
		{
			pm.ps.pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
			pm.ps.pm_time = 0;
		}
		*/

		if(pm.ps.getPlayerState_groundEntityNum() == Engine.ENTITYNUM_NONE)
		{
			// just hit the ground
			if(pm.debugLevel == 1)
			{
				Engine.print(c_pmove + ":land\n");
			}

			crashLand();

			// don't do landing time if we were just going down a slope
			if(pml.previousVelocity.dot(pml.gravityNormalFlipped) < -200)
			{
				// don't allow another jump for a little while
				pm.ps.addPlayerState_pm_flags(PlayerMovementFlags.TIME_LAND);
				pm.ps.setPlayerState_pm_time(250);
			}
		}

		pm.ps.setPlayerState_groundEntityNum(trace.getEntityNum());

		// don't reset the z velocity for slopes
	//  pm.ps.velocity[2] = 0;

		// TODO PM_AddTouchEnt(trace.entityNum);
	}
	
	/**
	 * Check for hard landings that generate sound events.
	 */
	private void crashLand()
	{
		/*
		float           delta;
		float           dist;
		float           vel, acc;
		float           t;
		float           a, b, c, den;

		// decide which landing animation to use
		if(pm.ps.pm_flags & PMF_BACKWARDS_JUMP)
		{
			PM_ForceLegsAnim(LEGS_LANDB);
		}
		else
		{
			PM_ForceLegsAnim(LEGS_LAND);
		}

		pm.ps.legsTimer = TIMER_LAND;

		// calculate the exact velocity on landing
		dist = pm.ps.origin[2] - pml.previous_origin[2];
		vel = pml.previous_velocity[2];
		acc = -pm.ps.gravity;

		a = acc / 2;
		b = vel;
		c = -dist;

		den = b * b - 4 * a * c;
		if(den < 0)
		{
			return;
		}
		t = (-b - sqrt(den)) / (2 * a);

		delta = vel + t * acc;
		delta = delta * delta * 0.0001;

		// ducking while falling doubles damage
		if(pm.ps.pm_flags & PMF_DUCKED)
		{
			delta *= 2;
		}

		// never take falling damage if completely underwater
		if(pm->waterlevel == 3)
		{
			return;
		}

		// reduce falling damage if there is standing water
		if(pm->waterlevel == 2)
		{
			delta *= 0.25;
		}
		if(pm->waterlevel == 1)
		{
			delta *= 0.5;
		}

		if(delta < 1)
		{
			return;
		}

		// create a local entity event to play the sound

		// SURF_NODAMAGE is used for bounce pads where you don't ever
		// want to take damage or play a crunch sound
		if(!(pml.groundTrace.surfaceFlags & SURF_NODAMAGE))
		{
			if(delta > 60)
			{
				PM_AddEvent(EV_FALL_FAR);
			}
			else if(delta > 40)
			{
				// this is a pain grunt, so don't play it if dead
				if(pm.ps.stats[STAT_HEALTH] > 0)
				{
					PM_AddEvent(EV_FALL_MEDIUM);
				}
			}
			else if(delta > 7)
			{
				PM_AddEvent(EV_FALL_SHORT);
			}
			else
			{
				PM_AddEvent(PM_FootstepForSurface());
			}
		}

		// start footstep cycle over
		pm.ps.bobCycle = 0;
		*/
	}
	
	private KinematicClosestNotMeConvexResultCallback traceAll(Vector3f startPos, Vector3f endPos)
	{
		//pm->trace(trace, start, pm->mins, pm->maxs, end, pm.ps.clientNum, pm->tracemask);
		
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
	
	private KinematicClosestNotMeConvexResultCallback traceLegs(Vector3f startPos, Vector3f endPos)
	{
		ConvexShape legsShape = new SphereShape(Config.PLAYER_WIDTH / 2); 
		
		// place it to the exact same position of the lower sphere in the CapsuleShapeZ
		Vector3f offset = new Vector3f(pml.gravityNormal);
		offset.scale(Config.PLAYER_HEIGHT / 2);
		
		Transform start = new Transform();
		Transform end = new Transform();

		start.setIdentity();
		end.setIdentity();

		//
		start.origin.set(startPos);
		start.origin.add(offset);
		
		end.origin.set(endPos);
		end.origin.add(offset);

		KinematicClosestNotMeConvexResultCallback callback = new KinematicClosestNotMeConvexResultCallback(ghostObject);
		callback.collisionFilterGroup = ghostObject.getBroadphaseHandle().collisionFilterGroup;
		callback.collisionFilterMask = ghostObject.getBroadphaseHandle().collisionFilterMask;

		if (CVars.pm_useGhostObjectSweepTest.getBoolean()) 
		{
			ghostObject.convexSweepTest(legsShape, start, end, callback, collisionWorld.getDispatchInfo().allowedCcdPenetration);
		}
		else 
		{
			collisionWorld.convexSweepTest(legsShape, start, end, callback);
		}
		
		return callback;
	}
	

	/**
	 * @return True if the velocity was clipped in some way
	 */
	private boolean slideMove(boolean gravity)//, boolean stepUp, boolean stepDown, boolean push)
	{
		int             bumpcount, numbumps;
		Vector3f        dir = new Vector3f();
		float           d;
		int             numplanes;
		Vector3f        primalVelocity;
		Vector3f        clipVelocity = new Vector3f();
		int             i, j, k;
		
		float           timeLeft;
		float           into;
		Vector3f        endVelocity = new Vector3f();

		numbumps = 4;
		
		Vector3f planes[] = new Vector3f[MAX_CLIP_PLANES];
		for(i = 0; i < planes.length; i++)
		{
			planes[i] = new Vector3f();
		}

		primalVelocity = pm.ps.getPlayerState_velocity();
		
		
		if(gravity)
		{
			Vector3f curVelocity = pm.ps.getPlayerState_velocity();
			
			endVelocity = new Vector3f(curVelocity);
			
			endVelocity.scaleAdd(pml.frameTime, pml.gravityVector, curVelocity);
			
			primalVelocity.set(endVelocity);
			
			curVelocity.add(endVelocity);
			curVelocity.scale(0.5f);
			 
			if(pml.groundPlane)
			{
				// slide along the ground plane
				clipVelocity(curVelocity, pml.groundTrace.hitNormalWorld, curVelocity, OVERCLIP);
			}
			
			pm.ps.setPlayerState_velocity(curVelocity);
		}
		else
		{
			endVelocity = pm.ps.getPlayerState_velocity();
		}
		

		timeLeft = pml.frameTime;

		// never turn against the ground plane
		if(pml.groundPlane)
		{
			numplanes = 1;
			planes[0].set(pml.groundTrace.hitNormalWorld);
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
			
			end.scaleAdd(timeLeft, pm.ps.getPlayerState_velocity(), start);

			// see if we can make it there
			KinematicClosestNotMeConvexResultCallback trace = traceAll(start, end);
			//

			if(trace.closestHitFraction == 0)
			{
				// entity is completely trapped in another solid
				
				// don't build up falling damage, but allow sideways acceleration	
				Vector3f vel = pm.ps.getPlayerState_velocity();
				vel.z = 0;
				pm.ps.setPlayerState_velocity(vel);
				
				return true;
			}

			if (!trace.hasHit()) 
			{
				// moved the entire distance
				setOrigin(end);
				break;
			}
			else
			{
				// actually covered some distance
				end.interpolate(start, end, trace.closestHitFraction);
			}
			
			pml.contactInfos.add(new ContactInfo(trace.hitPointWorld, trace.hitNormalWorld));
			
			setOrigin(end);
			
			/*
			boolean stepped = false;
			boolean pushed = false;
			
			// test the player position if they were a stepheight higher
			if(stepUp)
			{
				boolean nearGround = pml.groundPlane; // | pml.ladder
				
				KinematicClosestNotMeConvexResultCallback downTrace, stepTrace, upTrace;
				
				start = pm.ps.getPlayerState_origin();
				
				if(!nearGround)
				{
					// trace down to see if the player is near the ground
					end.scaleAdd(CVars.pm_stepHeight.getValue(), pml.gravityNormal, pm.ps.getPlayerState_origin());
					
					downTrace = traceAll(start, end);
					
					nearGround = downTrace.hasHit() && !isNormalTooSteep(downTrace.hitNormalWorld);
				}
				
				// may only step up if near the ground or on a ladder
				if(nearGround)
				{
					// step up, test the player position if they were a stepheight higher
					end.scaleAdd(CVars.pm_stepHeight.getValue(), pml.gravityNormalFlipped, start);
					
					upTrace = traceAll(start, end);

					if(upTrace.closestHitFraction == 0) 
					{
						// can't step up
						if(pm.debugLevel == 1)
						{
							Engine.println(c_pmove + ":bend can't step up");
						}
						//return;
					}
					else
					{
						if(pm.debugLevel == 1)
						{
							Engine.println(c_pmove + ":step up");
						}
						
						if(upTrace.hasHit())
						{
							end.interpolate(start, end, upTrace.closestHitFraction);
						}
						
						// calculate step size along the gravity normal
						Vector3f stepVector = new Vector3f(end);
						stepVector.sub(start);
						
						Vector3f stepNormal = new Vector3f(stepVector);
						stepNormal.normalize();
						
						float stepSize = /*pml.gravityNormalFlipped.dot(stepNormal) *  stepVector.length();
						
						// try slidemove from this position
						start.set(end);
						end.scaleAdd(timeLeft, pm.ps.getPlayerState_velocity(), end);
						
						stepTrace = traceAll(start, end);
						
						// push down the final amount
						if(stepTrace.hasHit())
						{
							end.interpolate(start, end, stepTrace.closestHitFraction);
						}
						
						start.set(end);
						end.scaleAdd(stepSize, pml.gravityNormal, start);
						
						downTrace = traceAll(start, end);
						
						
						if(!downTrace.hasHit() || !isNormalTooSteep(downTrace.hitNormalWorld))
						{
							if(!stepTrace.hasHit())
							{
								// moved the entire distance
								timeLeft = 0;
								
								end.interpolate(start, end, downTrace.closestHitFraction);
								setOrigin(end);
								break;
							}
							
							// if the move is further when stepping up
							if(stepTrace.closestHitFraction > trace.closestHitFraction)
							{
								timeLeft -= timeLeft * trace.closestHitFraction;
								
								end.interpolate(start, end, downTrace.closestHitFraction);
								setOrigin(end);
								
								stepped = true;
							}
						}
					}
				}
			}
			*/

			// TODO save entity for contact
			//PM_AddTouchEnt(trace.entityNum);

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
				if(trace.hitNormalWorld.dot(planes[i]) > 0.99)
				{
					Vector3f nudgedVelocity = pm.ps.getPlayerState_velocity();
					nudgedVelocity.add(trace.hitNormalWorld);
					break;
				}
			}
			if(i < numplanes)
			{
				continue;
			}
			
			planes[numplanes++].set(trace.hitNormalWorld);

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
		
		/*
		if(stepDown && pml.groundPlane)
		{
			// push down the final amount
			Vector3f start = pm.ps.getPlayerState_origin();
			
			Vector3f end = new Vector3f();
			end.scaleAdd(CVars.pm_stepHeight.getValue(), pml.gravityNormal, pm.ps.getPlayerState_origin());
			
			KinematicClosestNotMeConvexResultCallback callback = traceAll(start, end);

			if(callback.hasHit())
			{
				end.interpolate(start, end, callback.closestHitFraction);
				
				if(pm.debugLevel == 1)
				{
					Engine.println(c_pmove + ":stepped");
				}
			}
			
			setOrigin(end);
			
			
			if(callback.closestHitFraction < 1.0)
			{
				clipVelocity(pm.ps.getPlayerState_velocity(), callback.hitNormalWorld, startVelocity, OVERCLIP);
				
				pm.ps.setPlayerState_velocity(startVelocity);
			}	
		}
		*/

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
		if(!slideMove(gravity))//, false, false, false))
		{
			// we got exactly where we wanted to go first try
			if(pm.debugLevel == 1)
			{
				//Engine.println(c_pmove + ":slided");
			}
			return;
		}
		
		
		boolean nearGround = pml.groundPlane; // | pml.ladder
		
		KinematicClosestNotMeConvexResultCallback downTrace, stepTrace, upTrace;
		
		Vector3f start = pm.ps.getPlayerState_origin();
		Vector3f end = new Vector3f();
		
		Vector3f startVelocity = pm.ps.getPlayerState_velocity();
		
		if(!nearGround)
		{
			end.scaleAdd(CVars.pm_stepHeight.getValue(), pml.gravityNormal, start);
			
			downTrace = traceAll(start, end);	
			
			
			
			nearGround = 	startVelocity.dot(pml.gravityNormalFlipped) <= 0 &&
							//startVelocity.dot(downTrace.hitNormalWorld) <= 10 &&
							(downTrace.hasHit() || !isNormalTooSteep(downTrace.hitNormalWorld));
		}
		
		// never step up if getting thrown off the ground
		if(nearGround)
		{
			// step up, test the player position if they were a stepheight higher
			end.scaleAdd(CVars.pm_stepHeight.getValue(), pml.gravityNormalFlipped, start);
			
			upTrace = traceAll(start, end);

			if(upTrace.closestHitFraction == 0) 
			{
				// can't step up
				if(pm.debugLevel == 1)
				{
					Engine.println(c_pmove + ":bend can't step up");
				}
				//return;
			}
			else
			{
				if(pm.debugLevel == 1)
				{
					Engine.println(c_pmove + ":step up");
				}
				
				if(upTrace.hasHit())
				{
					end.interpolate(start, end, upTrace.closestHitFraction);
				}
				
				// step size along the gravity normal
				Vector3f stepVector = new Vector3f(end);
				stepVector.sub(start);
				
				Vector3f stepNormal = new Vector3f(stepVector);
				stepNormal.normalize();
				
				float stepSize = /*pml.gravityNormalFlipped.dot(stepNormal) * */ stepVector.length();
				
				// try slidemove from this position
				setOrigin(end);
				pm.ps.setPlayerState_velocity(startVelocity);
				
				if(!slideMove(gravity))//, false, false, false))
				{
					if(pm.debugLevel == 1)
					{
						Engine.println(c_pmove + ":step");
					}
				}
				else
				{
					if(pm.debugLevel == 1)
					{
						Engine.println(c_pmove + ":step clipped");
					}
				}
				
				// push down the final amount
				start = pm.ps.getPlayerState_origin();
				end.scaleAdd(stepSize, pml.gravityNormal, start);
				downTrace = traceAll(start, end);
				
				if(downTrace.hasHit())
				{
					end.interpolate(start, end, downTrace.closestHitFraction);
					
					clipVelocity(pm.ps.getPlayerState_velocity(), downTrace.hitNormalWorld, startVelocity, OVERCLIP);
					
					pm.ps.setPlayerState_velocity(startVelocity);
				}
				
				setOrigin(end);
				
				if(pm.debugLevel == 1)
				{
					Engine.println(c_pmove + ":step down");
				}
			}
		}
	}
	
	/*
	private void stepSlideMoveOld(boolean gravity)
	{
		//vec3_t          start_o, start_v;
		//vec3_t          down_o, down_v;
		//trace_t         trace;

		//float       down_dist, up_dist;
		//vec3_t      delta, delta2;
		//vec3_t          up, down;
		float           stepSize;

		Vector3f startOrigin = pm.ps.getPlayerState_origin();
		Vector3f startVelocity = pm.ps.getPlayerState_velocity();

		if(!slideMove(gravity, false, false, false))
		{
			// we got exactly where we wanted to go first try
			if(pm.debugLevel == 1)
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
			if(pm.debugLevel == 1)
			{
				Engine.println(c_pmove + ":bend can't step up");
			}
			return;
		}
		
		// try slidemove from this position
		if (callback.hasHit()) 
		{
			stepSize = callback.hitPointWorld.z - startOrigin.z;
			
			up.interpolate(startOrigin, up, callback.closestHitFraction);
		}
		
		stepSize = up.z - startOrigin.z;
		
		setOrigin(up);
		pm.ps.setPlayerState_velocity(startVelocity);
		
		slideMove(gravity, false, false, false);
		
		
		
		// push down the final amount
		down = pm.ps.getPlayerState_origin();
		down.z -= stepSize;
		
		
		callback = traceAll(startOrigin, down);

		if(callback.hasHit())
		{
			down.interpolate(startOrigin, down, callback.closestHitFraction);
		}
		
		setOrigin(down);
		
		if(callback.closestHitFraction < 1.0)
		{
			clipVelocity(pm.ps.getPlayerState_velocity(), callback.hitNormalWorld, startVelocity, OVERCLIP);
			
			pm.ps.setPlayerState_velocity(startVelocity);
		}
		
	
		/
		// if the down trace can trace back to the original position directly, don't step
		pm->trace(&trace, pm.ps.origin, pm->mins, pm->maxs, start_o, pm.ps.clientNum, pm->tracemask);
		if(trace.fraction == 1.0)
		{
			// use the original move
			VectorCopy(down_o, pm.ps.origin);
			VectorCopy(down_v, pm.ps.velocity);
			if(pm->debugLevel)
			{
				Com_Printf("%i:bend\n", c_pmove);
			}
		}
		else
			/
		
		{
			
			// use the step move
			float           delta;

			delta = pm.ps.origin[2] - start_o[2];
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
			
			
			if(pm.debugLevel == 1)
			{
				Engine.println(c_pmove + ":stepped");
			}
		}	
	}
	*/
	
	private void spectatorMove(boolean clipAgainstWorld)
	{
		float           speed, drop, friction, control, newspeed;
		Vector3f        wishvel;
		float           fmove, smove;
		float           wishspeed;
		float           scale;

		pm.ps.setPlayerState_viewHeight(Config.DEFAULT_VIEWHEIGHT);

		// friction
		Vector3f curvel = pm.ps.getPlayerState_velocity();
		//Vector3f curpos = pm.ps.getPlayerState_origin();
		
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
			drop += control * friction * pml.frameTime;

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

		accelerate(wishdir, wishspeed, CVars.pm_accelerate.getValue());
		

		// move
		Vector3f playerVelocity = pm.ps.getPlayerState_velocity();
			
		if(clipAgainstWorld)
		{
			Vector3f start = pm.ps.getPlayerState_origin();
			Vector3f end = new Vector3f();

			end.scaleAdd(pml.frameTime, playerVelocity, start);
			
			KinematicClosestNotMeConvexResultCallback callback = traceAll(start, end);
			
			if (callback.hasHit()) 
			{
				end.interpolate(start, end, callback.closestHitFraction);
			}
			
			setOrigin(end);
		}
		else
		{
			Vector3f origin = pm.ps.getPlayerState_origin();
			
			playerVelocity.scale(pml.frameTime);
			origin.add(playerVelocity);
			setOrigin(origin);
		}
	}
	
	private void flyMove()
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
			
			
			wishvel = new Vector3f(	pml.forward.x * pm.cmd.forwardmove * scale + pml.right.x * pm.cmd.rightmove * scale,
									pml.forward.y * pm.cmd.forwardmove * scale + pml.right.y * pm.cmd.rightmove * scale,
									pml.forward.z * pm.cmd.forwardmove * scale + pml.right.z * pm.cmd.rightmove * scale);

			wishvel.z += pm.cmd.upmove * scale;
		}
		
		if(Float.isNaN(wishvel.x) || Float.isNaN(wishvel.y) || Float.isNaN(wishvel.z))
			throw new RuntimeException("wishvel member(s) became NaN");

		wishdir = new Vector3f(wishvel);
		wishspeed = wishdir.length();
		
		if(wishspeed > 0)
		{
			wishdir.normalize();
		}
		
		accelerate(wishdir, wishspeed, CVars.pm_flyAccelerate.getValue());
		
		stepSlideMove(false);
	}
	
	private void airMove()
	{
		// normal slowdown
		applyFriction();
		
		// set the movementDir so clients can rotate the legs for strafing
		setMovementDir();

		float scale = calculateUserCommandScale(pm.cmd);
		
		// project moves down to flat plane
		//pml.forward.z = 0;
		//pml.right.z = 0;
		
		// forward = 
		Vector3f dir = new Vector3f(pml.gravityNormal);
		dir.scale(pml.forward.dot(pml.gravityNormal));
		pml.forward.sub(dir);
		
		dir.set(pml.gravityNormal);
		dir.scale(pml.right.dot(pml.gravityNormal));
		pml.right.sub(dir);
		
		pml.forward.normalize();
		pml.right.normalize();
		
		// user intentions
		Vector3f wishvel = new Vector3f(pml.forward.x * pm.cmd.forwardmove + pml.right.x * pm.cmd.rightmove,
										pml.forward.y * pm.cmd.forwardmove + pml.right.y * pm.cmd.rightmove,
										pml.forward.z * pm.cmd.forwardmove + pml.right.z * pm.cmd.rightmove);
		
		if(Float.isNaN(wishvel.x) || Float.isNaN(wishvel.y) || Float.isNaN(wishvel.z))
			throw new RuntimeException("wishvel member(s) became NaN");
				

		Vector3f wishdir = new Vector3f(wishvel);
		float wishspeed = wishdir.normalize();
		wishspeed *= scale;
		
		// not on ground, so little effect on velocity
		accelerate(wishdir, wishspeed, CVars.pm_airAccelerate.getValue());
		
		// we may have a ground plane that is very steep, even
		// though we don't have a groundentity
		// slide along the steep plane
		if(pml.groundPlane)
		{
			Vector3f velocity = pm.ps.getPlayerState_velocity();
			
			clipVelocity(velocity, pml.groundTrace.hitNormalWorld, velocity, OVERCLIP);
			
			pm.ps.setPlayerState_velocity(velocity);
		}
			
		stepSlideMove(true);
	}
	
	private void walkMove()
	{
		/*
		if(pm->waterlevel > 2 && DotProduct(pml.forward, pml.groundTrace.plane.normal) > 0)
		{
			// begin swimming
			PM_WaterMove();
			return;
		}
		*/

		if(checkJump())
		{
			/*
			// jumped away
			if(pm->waterlevel > 1)
			{
				PM_WaterMove();
			}
			else
			*/
			{
				airMove();
			}
			return;
		}

		
		applyFriction();

		float scale = calculateUserCommandScale(pm.cmd);

		// set the movementDir so clients can rotate the legs for strafing
		setMovementDir();

		// project moves down to flat plane
		Vector3f dir = new Vector3f(pml.gravityNormal);
		dir.scale(pml.forward.dot(pml.gravityNormal));
		
		pml.forward.sub(dir);
		
		dir.set(pml.gravityNormal);
		dir.scale(pml.right.dot(pml.gravityNormal));
		pml.right.sub(dir);

		// project the forward and right directions onto the ground plane
		clipVelocity(pml.forward, pml.groundTrace.hitNormalWorld, pml.forward, OVERCLIP);
		clipVelocity(pml.right, pml.groundTrace.hitNormalWorld, pml.right, OVERCLIP);
		//
		pml.forward.normalize();
		pml.right.normalize();
		
		Vector3f wishvel = new Vector3f(pml.forward.x * pm.cmd.forwardmove + pml.right.x * pm.cmd.rightmove,
										pml.forward.y * pm.cmd.forwardmove + pml.right.y * pm.cmd.rightmove,
										pml.forward.z * pm.cmd.forwardmove + pml.right.z * pm.cmd.rightmove);

		// when going up or down slopes the wish velocity should Not be zero
	//  wishvel[2] = 0;
		
		Vector3f wishdir = new Vector3f(wishvel);
		float wishspeed = wishdir.normalize();
		wishspeed *= scale;

		// clamp the speed lower if ducking
		if(pm.ps.hasPlayerState_pm_flags(PlayerMovementFlags.DUCKED))
		{
			if(wishspeed > pm.ps.getPlayerState_speed() * CVars.pm_duckScale.getValue())
			{
				wishspeed = pm.ps.getPlayerState_speed() * CVars.pm_duckScale.getValue();
			}
		}

		// clamp the speed lower if wading or walking on the bottom
		/*
		if(pm.waterLevel)
		{
			float           waterScale;

			waterScale = pm->waterlevel / 3.0;
			waterScale = 1.0 - (1.0 - pm_swimScale) * waterScale;
			if(wishspeed > pm.ps.speed * waterScale)
			{
				wishspeed = pm.ps.speed * waterScale;
			}
		}
		*/

		// when a player gets hit, they temporarily lose
		// full control, which allows them to be moved a bit
		float accelerate;
		if( /* Tr3B: FIXME ADD (pml.groundTrace.surfaceFlags & SURF_SLICK)  ||*/ pm.ps.hasPlayerState_pm_flags(PlayerMovementFlags.TIME_KNOCKBACK))
		{
			accelerate = CVars.pm_airAccelerate.getValue();
		}
		else
		{
			accelerate = CVars.pm_accelerate.getValue();
		}

		accelerate(wishdir, wishspeed, accelerate);

		
		
		Vector3f velocity = pm.ps.getPlayerState_velocity();
		
		if(pm.debugLevel == 2)
		{
			Engine.print(String.format("groundPlaneNormal = %1.1f %1.1f %1.1f\n", pml.groundTrace.hitNormalWorld.x, pml.groundTrace.hitNormalWorld.y, pml.groundTrace.hitNormalWorld.z));
			Engine.print(String.format("wishvel = %1.1f %1.1f %1.1f\n", wishvel.x, wishvel.y, wishvel.z));
			Engine.print(String.format("wishdir = %1.1f %1.1f %1.1f\n", wishdir.x, wishdir.y, wishdir.z));
			Engine.print(String.format("wishspeed = %1.1f\n", wishspeed));
			Engine.print(String.format("velocity = %1.1f %1.1f %1.1f\n", velocity.x, velocity.y, velocity.z));
			Engine.print(String.format("velocity1 = %1.1f\n", velocity.length()));
		}
		
		
		if(/* Tr3B: FIXME ADD (pml.groundTrace.surfaceFlags & SURF_SLICK) ||*/ pm.ps.hasPlayerState_pm_flags(PlayerMovementFlags.TIME_KNOCKBACK))
		{
			//pm.ps.velocity[2] -= pm.ps.gravity * pml.frametime;
			
			velocity.scaleAdd(pml.frameTime, pml.gravityVector, velocity);
		}
		else
		{
			// don't reset the z velocity for slopes
//	      pm.ps.velocity[2] = 0;
		}

		float vel = velocity.length();

		// slide along the ground plane
		clipVelocity(velocity, pml.groundTrace.hitNormalWorld, velocity, OVERCLIP);

		// don't decrease velocity when going up or down a slope
		velocity.normalize();
		velocity.scale(vel);
		
		pm.ps.setPlayerState_velocity(velocity);

		// don't do anything if standing still
		dir = new Vector3f(pml.gravityNormal);
		dir.scale(velocity.dot(pml.gravityNormal));
		
		velocity.sub(dir);
		if(velocity.lengthSquared() <= 0)
		{
			return;
		}

		stepSlideMove(false);

		if(CVars.pm_debugServer.getInteger() == 2)
		{
			Engine.print(String.format("velocity2 = %1.1f\n", velocity.length()));
		}
	}


	@Override
	public void debugDraw(IDebugDraw debugDrawer)
	{
		// TODO Auto-generated method stub

	}


	private static class KinematicClosestNotMeConvexResultCallback extends CollisionWorld.ClosestConvexResultCallback
	{
		protected CollisionObject	me;

		public KinematicClosestNotMeConvexResultCallback(CollisionObject me)
		{
			super(new Vector3f(), new Vector3f());
			this.me = me;
		}

		@Override
		public float addSingleResult(CollisionWorld.LocalConvexResult convexResult, boolean normalInWorldSpace)
		{
			if (convexResult.hitCollisionObject == me)
			{
				return 1.0f;
			}

			return super.addSingleResult(convexResult, normalInWorldSpace);
		}

		/**
		 * Emulates trace_t::entityNum
		 * 
		 * @return
		 */
		public int getEntityNum()
		{

			if (hasHit())
			{
				EntityStateAccess ent = (EntityStateAccess) hitCollisionObject.getUserPointer();
				if (ent != null)
				{

					return ent.getEntityState_number();
				}

				else
				{
					return Engine.ENTITYNUM_WORLD;
				}
			}

			return Engine.ENTITYNUM_NONE;
		}
	}
	
}
