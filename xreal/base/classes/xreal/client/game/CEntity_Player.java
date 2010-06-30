package xreal.client.game;

import javax.vecmath.Vector3f;

import xreal.Angle3f;
import xreal.Engine;
import xreal.client.EntityState;

public class CEntity_Player extends CEntity {
	
	private int				muzzleFlashTime;
	
	public CEntity_Player(EntityState es) {
		super(es);
		
		//Engine.println("CEntity_Player(entity number = " + es.getNumber() + ")");
	}

	/**
	 * A player just came into view or teleported, so reset all animation info.
	 */
	public void reset() {
		
		//super.reset();
		
		errorTime = -99999;	// guarantee no error decay added
		extrapolated = false;
	
		//CG_ClearLerpFrame(&cgs.clientinfo[cent->currentState.clientNum], &cent->pe.legs, cent->currentState.legsAnim);
		//CG_ClearLerpFrame(&cgs.clientinfo[cent->currentState.clientNum], &cent->pe.torso, cent->currentState.torsoAnim);
	
		lerpOrigin = currentState.pos.evaluatePosition(ClientGame.getTime());
		lerpAngles = new Angle3f(currentState.apos.evaluatePosition(ClientGame.getTime()));
	
		rawOrigin = new Vector3f(lerpOrigin);
		rawAngles = new Angle3f(lerpAngles);
		
		/*	
		memset(&cent->pe.legs, 0, sizeof(cent->pe.legs));
		cent->pe.legs.yawAngle = cent->rawAngles[YAW];
		cent->pe.legs.yawing = qfalse;
		cent->pe.legs.pitchAngle = 0;
		cent->pe.legs.pitching = qfalse;
	
		memset(&cent->pe.torso, 0, sizeof(cent->pe.legs));
		cent->pe.torso.yawAngle = cent->rawAngles[YAW];
		cent->pe.torso.yawing = qfalse;
		cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
		cent->pe.torso.pitching = qfalse;
	
		if(cg_debugPosition.integer)
		{
			CG_Printf("%i ResetPlayerEntity yaw=%i\n", cent->currentState.number, cent->pe.torso.yawAngle);
		}
		 */
	}
}
