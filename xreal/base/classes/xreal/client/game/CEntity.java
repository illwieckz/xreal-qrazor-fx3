package xreal.client.game;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;

import xreal.Angle3f;
import xreal.CVars;
import xreal.Engine;
import xreal.TrajectoryType;
import xreal.client.EntityState;



/**
 * CEntity has a direct correspondence with GameEntity in the game, but
 * only the entityState_t is directly communicated to the client game.
 * 
 * @author Robert Beckebans
 *
 */
public abstract class CEntity {
	public EntityState		currentState;	// from cg.frame
	public EntityState		nextState;	// from cg.nextFrame, if available
	public boolean			interpolate;	// true if next is valid to interpolate to
	public boolean			currentValid;	// true if cg.frame holds this entity

	public int				previousEvent;
	public int				teleportFlag;

	public int				trailTime;	// so missile trails can handle dropped initial packets
	public int				dustTrailTime;
	public int				miscTime;

	public int				snapShotTime;	// last time this entity was found in a snapshot

	//playerEntity_t  pe;	// goes to ClientPlayer

	public int				errorTime;	// decay the error from this time
	public Vector3f			errorOrigin = new Vector3f();
	public Vector3f			errorAngles = new Vector3f();

	boolean					extrapolated;	// false if origin / angles is an interpolation
	public Vector3f			rawOrigin = new Vector3f();
	public Angle3f			rawAngles = new Angle3f();

	public Vector3f			beamEnd = new Vector3f();

	// exact interpolated position of entity on this frame
	public Vector3f			lerpOrigin = new Vector3f();
	public Angle3f			lerpAngles = new Angle3f();
	public Quat4f			lerpQuat = new Quat4f();
	
	
	public CEntity(EntityState es) {
		
		Engine.println("called constructor " + this.getClass().getName() + "(entity number = " + es.getNumber() + ")");
		
		ClientGame.getEntities().setElementAt(this, es.getNumber());
		
		currentState = es;
		nextState = es;
		
		reset();
	}
	
	public void reset() {
		// if the previous snapshot this entity was updated in is at least
		// an event window back in time then we can reset the previous event
		if(snapShotTime < (ClientGame.getTime() - EntityState.EVENT_VALID_MSEC)) {
			previousEvent = 0;
		}

		trailTime = ClientGame.getTime();

		lerpOrigin = new Vector3f(currentState.origin);
		lerpAngles = new Angle3f(currentState.angles);
	}
	
	public void checkEvents() {
		// TODO
	}
	
	/**
	 * nextState is moved to currentState and events are fired
	 */
	public void transitionState()
	{
		currentState = nextState;
		currentValid = true;

		// reset if the entity wasn't in the last frame or was teleported
		if(!interpolate) {
			reset();
		}

		// clear the next state.  if will be set by the next CG_SetNextSnap
		interpolate = false;

		// check for events
		checkEvents();
	}
	
	public void addToRenderer() throws Exception {
		
		// calculate the current origin
		calcEntityLerpPositions();

		// add automatic effects
		//CG_EntityEffects(cent);
	}
	
	protected void calcEntityLerpPositions() throws Exception
	{
		// if this player does not want to see extrapolated players
		if(!CVars.cg_smoothClients.getBoolean())
		{
			// make sure the clients use TR_INTERPOLATE
			if(currentState.getNumber() < Engine.MAX_CLIENTS)
			{
				currentState.pos.trType = TrajectoryType.INTERPOLATE;
				nextState.pos.trType = TrajectoryType.INTERPOLATE;
			}
		}

		if(interpolate && currentState.pos.trType == TrajectoryType.INTERPOLATE)
		{
			interpolateEntityPosition();
			return;
		}

		// first see if we can interpolate between two snaps for
		// linear extrapolated clients
		if(interpolate && currentState.pos.trType == TrajectoryType.LINEAR_STOP && currentState.getNumber() < Engine.MAX_CLIENTS)
		{
			interpolateEntityPosition();
			return;
		}

		// just use the current frame and evaluate as best we can
		lerpOrigin = currentState.pos.evaluatePosition(ClientGame.getTime());
		
		Vector3f tmp = currentState.apos.evaluatePosition(ClientGame.getTime());
		lerpAngles = new Angle3f(tmp.x, tmp.y, tmp.z);
		
		lerpQuat = currentState.apos.evaluateRotation(ClientGame.getTime());

		// adjust for riding a mover if it wasn't rolled into the predicted
		// player state
		/*
		if(cent != &cg.predictedPlayerEntity)
		{
			CG_AdjustPositionForMover(lerpOrigin, currentState.groundEntityNum,
									  cg.snap->serverTime, cg.time, lerpOrigin);
		}
		*/
	}
	
	private void interpolateEntityPosition() throws Exception
	{
		// it would be an internal error to find an entity that interpolates without
		// a snapshot ahead of the current one
		if(ClientGame.getSnapshotManager().getNextSnapshot() == null)
		{
			throw new Exception("interpolateEntityPosition: cg.nextSnap == null");
		}

		float f = ClientGame.getSnapshotManager().getFrameInterpolation();
		
		int currentSnapshotTime = ClientGame.getSnapshotManager().getSnapshot().getServerTime();
		int nextSnapshotTime = ClientGame.getSnapshotManager().getNextSnapshot().getServerTime();

		// this will linearize a sine or parabolic curve, but it is important
		// to not extrapolate player positions if more recent data is available
		Vector3f currentPos = currentState.pos.evaluatePosition(currentSnapshotTime);
		Vector3f nextPos = nextState.pos.evaluatePosition(nextSnapshotTime);

		lerpOrigin.interpolate(currentPos, nextPos, f);
		
		Angle3f currentRot = new Angle3f(currentState.apos.evaluatePosition(currentSnapshotTime));
		Angle3f nextRot = new Angle3f(currentState.apos.evaluatePosition(nextSnapshotTime));

		lerpAngles.interpolate(currentRot, nextRot, f);
	}
}
