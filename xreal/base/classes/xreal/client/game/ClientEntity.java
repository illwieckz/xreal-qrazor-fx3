package xreal.client.game;

import javax.vecmath.Vector3f;

import xreal.Angle3f;
import xreal.Engine;
import xreal.client.EntityState;



/**
 * ClientEntity has a direct correspondence with GameEntity in the game, but
 * only the entityState_t is directly communicated to the client game.
 * 
 * @author Robert Beckebans
 *
 */
public class ClientEntity {
	public EntityState		currentState;	// from cg.frame
	public EntityState		nextState;	// from cg.nextFrame, if available
	public boolean			interpolate;	// true if next is valid to interpolate to
	public boolean			currentValid;	// true if cg.frame holds this entity

	public int				muzzleFlashTime;	// move to playerEntity?
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
	public Vector3f			rawAngles = new Vector3f();

	public Vector3f			beamEnd = new Vector3f();

	// exact interpolated position of entity on this frame
	public Vector3f			lerpOrigin = new Vector3f();
	public Angle3f			lerpAngles = new Angle3f();
	
	
	public ClientEntity(EntityState es) {
		
		Engine.println("ClientEntity()");
		
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
}
