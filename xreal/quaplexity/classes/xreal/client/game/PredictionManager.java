package xreal.client.game;

import javax.vecmath.Vector3f;

import com.bulletphysics.collision.broadphase.CollisionFilterGroups;
import com.bulletphysics.collision.dispatch.CollisionFlags;
import com.bulletphysics.collision.dispatch.GhostPairCallback;
import com.bulletphysics.collision.dispatch.PairCachingGhostObject;
import com.bulletphysics.collision.shapes.ConvexShape;
import com.bulletphysics.collision.shapes.CapsuleShapeZ;
import com.bulletphysics.collision.shapes.CylinderShapeZ;

import xreal.CVars;
import xreal.Engine;
import xreal.UserCommand;
import xreal.client.Client;
import xreal.client.EntityState;
import xreal.client.PlayerState;
import xreal.client.Snapshot;
import xreal.common.Config;
import xreal.common.PlayerController;
import xreal.common.PlayerMove;
import xreal.common.PlayerMovementFlags;
import xreal.common.PlayerMovementType;

public class PredictionManager {

	private boolean					hyperspace;
	
	private PlayerState				predictedPlayerState;
	private CEntity_Player			predictedPlayerEntity;
	private boolean					validPPS;				// clear until the first call to CG_PredictPlayerState
	private int						predictedErrorTime;
	private Vector3f				predictedError;

	private PairCachingGhostObject	ghostObject;
	private ConvexShape				collisionShape;
	private PlayerController		playerController;
	
	
	public PredictionManager() {
		super();
		this.hyperspace = false;
		this.predictedPlayerState = null; //new PlayerState();
		this.predictedPlayerEntity = null; //predictedPlayerEntity;
		this.validPPS = false;
		this.predictedErrorTime = 0;
		this.predictedError = new Vector3f();
		
		// create player controller ---------------------------------------------------------------
		
		ghostObject = new PairCachingGhostObject();
		//_ghostObject.setWorldTransform(startTransform);
		
		ClientGame.getBroadphase().getOverlappingPairCache().setInternalGhostPairCallback(new GhostPairCallback());
		
		//collisionShape = new CapsuleShapeZ(Config.PLAYER_WIDTH / 2, Config.PLAYER_HEIGHT / 2);
		//_collisionShape = new BoxShape(new Vector3f(18, 18, 37));
		collisionShape = new CylinderShapeZ(new Vector3f(CVars.pm_bodyWidth.getValue() / 2, CVars.pm_bodyWidth.getValue() / 2, CVars.pm_normalHeight.getValue() / 2));
		//_collisionShape = new SphereShape(Config.PLAYER_WIDTH / 2);
		
		/*
		Vector3f maxs = new Vector3f();
		Vector3f mins = new Vector3f();
		_collisionShape.getAabb(new Transform(), mins, maxs);
		setEntityState_solid(mins, maxs);
		*/
		
		ghostObject.setCollisionShape(collisionShape);
		ghostObject.setCollisionFlags(CollisionFlags.CHARACTER_OBJECT);
		
		playerController = new PlayerController(ghostObject, collisionShape, ClientGame.getDynamicsWorld());
		
		ClientGame.getDynamicsWorld().addCollisionObject(ghostObject, CollisionFilterGroups.CHARACTER_FILTER, (short)(CollisionFilterGroups.STATIC_FILTER | CollisionFilterGroups.DEFAULT_FILTER));
		ClientGame.getDynamicsWorld().addAction(playerController);
		
		// ----------------------------------------------------------------------------------------
	}




	/**
	 * Generates cg.predictedPlayerState for the current cg.time
	 * cg.predictedPlayerState is guaranteed to be valid after exiting.
	 * 
	 * For demo playback, this will be an interpolation between two valid
	 * playerState_t.
	 * 
	 * For normal gameplay, it will be the result of predicted usercmd_t on
	 * top of the most recent playerState_t received from the server.
	 * 
	 * Each new snapshot will usually have one or more new usercmd over the last,
	 * but we simulate all unacknowledged commands each time, not just the new ones.
	 * This means that on an internet connection, quite a few pmoves may be issued
	 * each frame.
	 * 
	 * OPTIMIZE: don't re-simulate unless the newly arrived snapshot playerState_t
	 * differs from the predicted one.  Would require saving all intermediate
	 * playerState_t during prediction.
	 * 
	 * We detect prediction errors and allow them to be decayed off over several frames
	 * to ease the jerk.
	 */
	public void predictPlayerState()
	{
		hyperspace = false;		// will be set if touching a trigger_teleport
		
		Snapshot snap = ClientGame.getSnapshotManager().getSnapshot();

		// if this is the first frame we must guarantee
		// predictedPlayerState is valid even if there is some
		// other error condition
		if(!validPPS)
		{
			validPPS = true;
			predictedPlayerState = (PlayerState) snap.getPlayerState().clone();
			predictedPlayerEntity = new CEntity_Player(predictedPlayerState.createEntityState(false));
		}


		// demo playback just copies the moves
		if(ClientGame.isDemoPlayback() || ((snap.getPlayerState().pm_flags & PlayerMovementFlags.FOLLOW) != 0))
		{
			interpolatePlayerState(false);
			
			predictedPlayerEntity.currentState = predictedPlayerState.createEntityState(false);
			predictedPlayerEntity.nextState = predictedPlayerEntity.currentState;
			
			predictedPlayerEntity.ps = predictedPlayerState;
			
			return;
		}
		// non-predicting local movement will grab the latest angles
		else if(CVars.cg_nopredict.getBoolean() || CVars.g_synchronousClients.getBoolean())
		{
			interpolatePlayerState(true); // FIXME true
			
			predictedPlayerEntity.currentState = predictedPlayerState.createEntityState(false);
			predictedPlayerEntity.nextState = predictedPlayerEntity.currentState;
			
			predictedPlayerEntity.ps = predictedPlayerState;
			
			return;
		}
		
		
		
		//predictedPlayerEntity = new CEntity_Player(predictedPlayerState.createEntityState(false));

		// prepare for pmove
		/*
		cg_pmove.ps = &cg.predictedPlayerState;
		cg_pmove.trace = CG_CapTrace;	// FIXME CG_CapTrace;
		cg_pmove.pointcontents = CG_PointContents;
		if(cg_pmove.ps->pm_type == PM_DEAD)
		{
			cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
		}
		else
		{
			cg_pmove.tracemask = MASK_PLAYERSOLID;
		}
		if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
		{
			cg_pmove.tracemask &= ~CONTENTS_BODY;	// spectators can fly through bodies
		}
		cg_pmove.noFootsteps = (cgs.dmflags & DF_NO_FOOTSTEPS) > 0;
		*/
		

		// save the state before the pmove so we can detect transitions
		PlayerState oldPlayerState = (PlayerState) predictedPlayerState.clone();

		// if we don't have the commands right after the snapshot, we
		// can't accurately predict a current position, so just freeze at
		// the last good position we had
		UserCommand oldestCommand = Client.getOldestUserCommand();
		
		if(oldestCommand.serverTime > snap.getPlayerState().commandTime && oldestCommand.serverTime < ClientGame.getTime())
		{
			// special check for map_restart
			if(CVars.cg_showmiss.getBoolean())
			{
				Engine.print("exceeded PACKET_BACKUP on commands\n");
			}
			return;
		}

		// get the latest command so we can know which commands are from previous map_restarts
		UserCommand latestCommand = Client.getCurrentUserCommand();

		// get the most recent information we have, even if
		// the server time is beyond our current cg.time,
		// because predicted player positions are going to
		// be ahead of everything else anyway
		SnapshotManager snaps = ClientGame.getSnapshotManager();
		
		if(snaps.getNextSnapshot() != null && !snaps.isNextFrameTeleport() && !snaps.isThisFrameTeleport())
		{
			predictedPlayerState = (PlayerState) snaps.getNextSnapshot().getPlayerState().clone();
			//cg.physicsTime = cg.nextSnap->serverTime;
		}
		else
		{
			predictedPlayerState = (PlayerState) snaps.getSnapshot().getPlayerState().clone();
			//cg.physicsTime = cg.snap->serverTime;
		}


		// run cmds
		boolean moved = false;
		final int current = Client.getCurrentCommandNumber();
		
		for(int cmdNum = current - Client.CMD_BACKUP + 1; cmdNum <= current; cmdNum++)
		{
			// get the command
			UserCommand cmd = Client.getUserCommand(cmdNum);

			// don't do anything if the time is before the snapshot player time
			if(cmd.serverTime <= predictedPlayerState.commandTime)
			{
				continue;
			}

			// don't do anything if the command was from a previous map_restart
			if(cmd.serverTime > latestCommand.serverTime)
			{
				continue;
			}

			// check for a prediction error from last frame
			// on a lan, this will often be the exact value
			// from the snapshot, but on a wan we will have
			// to predict several commands to get to the point
			// we want to compare
			
			if(predictedPlayerState.commandTime == oldPlayerState.commandTime)
			{
				Vector3f        delta = new Vector3f();
				float           len;
				
				if(snaps.isThisFrameTeleport())
				{
					// a teleport will not cause an error decay
					predictedError.set(0, 0, 0);
					
					if(CVars.cg_showmiss.getBoolean())
					{
						Engine.print("PredictionTeleport\n");
					}
					
					snaps.setThisFrameTeleport(false);
				}
				else
				{
					Vector3f          adjusted = new Vector3f(predictedPlayerState.origin);

					//CG_AdjustPositionForMover(cg.predictedPlayerState.origin,
					//						  cg.predictedPlayerState.groundEntityNum, cg.physicsTime, cg.oldTime, adjusted);

					if(CVars.cg_showmiss.getBoolean())
					{
						if(!oldPlayerState.origin.epsilonEquals(adjusted, 0.1f))
						{
							Engine.print("prediction error\n");
						}
					}
					
					delta.sub(oldPlayerState.origin, adjusted);
					len = delta.length();
					if(len > 0.1)
					{
						if(CVars.cg_showmiss.getBoolean())
						{
							Engine.print(String.format("Prediction miss: %f\n", len));
						}
						
						if(CVars.cg_errorDecay.getBoolean())
						{
							int             t;
							float           f;

							t = ClientGame.getTime() - predictedErrorTime;
							f = (CVars.cg_errorDecay.getValue() - t) / CVars.cg_errorDecay.getValue();
							
							if(f < 0)
							{
								f = 0;
							}
							
							if(f > 0 && CVars.cg_showmiss.getBoolean())
							{
								Engine.print(String.format("Double prediction decay: %f\n", f));
							}
							
							predictedError.scale(f);
						}
						else
						{
							predictedError.set(0, 0, 0);
						}
						
						predictedError.add(delta);
						predictedErrorTime = ClientGame.getOldTime();
					}
				}
			}
			

			// don't predict gauntlet firing, which is only supposed to happen
			// when it actually inflicts damage
			//cg_pmove.gauntletHit = qfalse;
			
			//cg_pmove.airControl = pm_airControl.integer;
			//cg_pmove.fastWeaponSwitches = pm_fastWeaponSwitches.integer;

			/*
			if(pm_fixedPmoveFPS.integer < 60)
				trap_Cvar_Set("pm_fixedPmoveFPS", "60");
			else if(pm_fixedPmoveFPS.integer > 333)
				trap_Cvar_Set("pm_fixedPmoveFPS", "333");

			cg_pmove.fixedPmove = pm_fixedPmove.integer;
			cg_pmove.fixedPmoveFPS = pm_fixedPmoveFPS.integer;
			*/
			
			
			
			PlayerMove pm = new PlayerMove(predictedPlayerState, cmd, true, 0, CVars.pm_debugClient.getInteger(), 0, 0, true, false, 0);
			
			// perform a pmove
			playerController.movePlayer(pm);

			moved = true;

			// add push trigger movement effects
			// TODO ? CG_TouchTriggerPrediction();

			// check for predictable events that changed from previous predictions
			//CG_CheckChangedPredictableEvents(&cg.predictedPlayerState);
			
			if(CVars.cg_showmiss.getInteger() > 1)
			{
				Engine.print(String.format("[%i : %i] ", cmd.serverTime, ClientGame.getTime()));
			}
		}

		

		if(!moved)
		{
			if(CVars.cg_showmiss.getBoolean())
			{
				Engine.print("not moved\n");
			}
			return;
		}

		// adjust for the movement of the groundentity
		//CG_AdjustPositionForMover(cg.predictedPlayerState.origin,
		//						  cg.predictedPlayerState.groundEntityNum, cg.physicsTime, cg.time, cg.predictedPlayerState.origin);

		if(CVars.cg_showmiss.getBoolean())
		{
			if(predictedPlayerState.eventSequence > oldPlayerState.eventSequence + PlayerState.MAX_PS_EVENTS)
			{
				Engine.print("WARNING: dropped event\n");
			}
		}

		// fire events and other transition triggered things
		//CG_TransitionPlayerState(&cg.predictedPlayerState, &oldPlayerState);

		/*
		if(CVars.cg_showmiss.getBoolean())
		{
			if(cg.eventSequence > cg.predictedPlayerState.eventSequence)
			{
				CG_Printf("WARNING: double event\n");
				cg.eventSequence = cg.predictedPlayerState.eventSequence;
			}
		}
		*/
		
		
		predictedPlayerEntity.currentState = predictedPlayerState.createEntityState(false);
		predictedPlayerEntity.nextState = predictedPlayerEntity.currentState;
		
		predictedPlayerEntity.ps = predictedPlayerState;
	}
	
	/**
	 * Generates cg.predictedPlayerState by interpolating between
	 * cg.snap->player_state and cg.nextFrame->player_state
	 * 
	 * @param grabAngles
	 */
	private void interpolatePlayerState(boolean grabAngles)
	{
		//CG_Printf("CG_InterpolatePlayerState(grabAngles = %d)\n", grabAngles);

		Snapshot prev = ClientGame.getSnapshotManager().getSnapshot();
		Snapshot next = ClientGame.getSnapshotManager().getNextSnapshot();

		predictedPlayerState = (PlayerState) prev.getPlayerState().clone();

		// if we are still allowing local input, short circuit the view angles
		if(grabAngles)
		{
			UserCommand cmd = Client.getCurrentUserCommand();

			// TODO
			//PM_UpdateViewAngles(out, cmd);
			PlayerController.updateViewAngles(predictedPlayerState, cmd);
		}

		// if the next frame is a teleport, we can't lerp to it
		if(ClientGame.getSnapshotManager().isNextFrameTeleport())
		{
			return;
		}

		if(next == null || next.getServerTime() <= prev.getServerTime())
		{
			return;
		}

		float f = (float)(ClientGame.getTime() - prev.getServerTime()) / (next.getServerTime() - prev.getServerTime());

		int i = next.getPlayerState().bobCycle;
		if(i < prev.getPlayerState().bobCycle)
		{
			// handle wraparound
			i += 256;				
		}
		
		predictedPlayerState.bobCycle = (int) (prev.getPlayerState().bobCycle + f * (i - prev.getPlayerState().bobCycle));

		Vector3f delta = new Vector3f();
		delta.sub(next.getPlayerState().origin, prev.getPlayerState().origin);
		delta.scale(f);
		
		predictedPlayerState.origin.add(prev.getPlayerState().origin, delta);
		if(!grabAngles)
		{
			predictedPlayerState.viewAngles.interpolate(prev.getPlayerState().viewAngles, next.getPlayerState().viewAngles, f);
		}
		
		predictedPlayerState.velocity.interpolate(prev.getPlayerState().velocity, next.getPlayerState().velocity, f);
		
			
		if (false) 
		{
			Engine.println("client origin:\t\t\t" + predictedPlayerState.origin.toString());
			Engine.println("client velocity:\t\t" + predictedPlayerState.velocity.toString());
			Engine.println("client view angles:\t\t" + predictedPlayerState.viewAngles.toString());
		}
	}
	
	public PlayerState getPredictedPlayerState() {
		return predictedPlayerState;
	}
	
	public CEntity_Player getPredictedPlayerEntity() {
		return predictedPlayerEntity;
	}
	
	public boolean isHyperspace() {
		return hyperspace;
	}
}
