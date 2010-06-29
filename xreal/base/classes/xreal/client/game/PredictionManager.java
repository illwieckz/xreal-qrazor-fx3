package xreal.client.game;

import javax.vecmath.Vector3f;

import xreal.CVars;
import xreal.Engine;
import xreal.UserCommand;
import xreal.client.Client;
import xreal.client.PlayerState;
import xreal.client.Snapshot;
import xreal.common.PlayerMovementFlags;

public class PredictionManager {

	private boolean			hyperspace;
	private PlayerState		predictedPlayerState;
	private ClientPlayer	predictedPlayerEntity;
	private boolean			validPPS;	// clear until the first call to CG_PredictPlayerState
	private int				predictedErrorTime;
	private Vector3f		predictedError;
	
	
	
	
	public PredictionManager() {
		super();
		this.hyperspace = false;
		this.predictedPlayerState = null; //new PlayerState();
		this.predictedPlayerEntity = null; //predictedPlayerEntity;
		this.validPPS = false;
		this.predictedErrorTime = 0;
		this.predictedError = null;
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
		int             cmdNum, current;
	//	playerState_t   oldPlayerState;
		boolean         moved;
		UserCommand     oldestCmd;
		UserCommand     latestCmd;

		hyperspace = false;		// will be set if touching a trigger_teleport
		
		Snapshot snap = ClientGame.getSnapshotManager().getSnapshot();

		// if this is the first frame we must guarantee
		// predictedPlayerState is valid even if there is some
		// other error condition
		if(!validPPS)
		{
			validPPS = true;
			predictedPlayerState = (PlayerState) snap.getPlayerState().clone();
		}


		// demo playback just copies the moves
		if(ClientGame.isDemoPlayback() || ((snap.getPlayerState().pm_flags & PlayerMovementFlags.FOLLOW) != 0))
		{
			interpolatePlayerState(false);
			return;
		}

		// non-predicting local movement will grab the latest angles
		if(CVars.cg_nopredict.getBoolean() || CVars.g_synchronousClients.getBoolean())
		{
			interpolatePlayerState(false); // FIXME true
			return;
		}

		// TODO
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
		
			
		if (false) {
			Engine.println("client origin:\t\t\t" + predictedPlayerState.origin.toString());
			Engine.println("client velocity:\t\t" + predictedPlayerState.velocity.toString());
			Engine.println("client view angles:\t\t" + predictedPlayerState.viewAngles.toString());
		}
	}
	
	public PlayerState getPredictedPlayerState() {
		return predictedPlayerState;
	}
}
