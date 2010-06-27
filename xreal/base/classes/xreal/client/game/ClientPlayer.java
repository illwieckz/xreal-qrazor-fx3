package xreal.client.game;

import xreal.Engine;
import xreal.client.EntityState;

public class ClientPlayer extends ClientEntity {
	
	public ClientPlayer(EntityState state) {
		super(state);
		
		Engine.println("ClientPlayer()");
	}

	/**
	 * A player just came into view or teleported, so reset all animation info.
	 */
	public void reset() {
		
		//super.reset();
		
		/*
		cent->errorTime = -99999;	// guarantee no error decay added
		cent->extrapolated = qfalse;
	
		CG_ClearLerpFrame(&cgs.clientinfo[cent->currentState.clientNum], &cent->pe.legs, cent->currentState.legsAnim);
		CG_ClearLerpFrame(&cgs.clientinfo[cent->currentState.clientNum], &cent->pe.torso, cent->currentState.torsoAnim);
	
		BG_EvaluateTrajectory(&cent->currentState.pos, cg.time, cent->lerpOrigin);
		BG_EvaluateTrajectory(&cent->currentState.apos, cg.time, cent->lerpAngles);
	
		VectorCopy(cent->lerpOrigin, cent->rawOrigin);
		VectorCopy(cent->lerpAngles, cent->rawAngles);
	
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
