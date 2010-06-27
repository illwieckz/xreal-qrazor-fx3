package xreal.client.game;

import xreal.CVars;
import xreal.Engine;
import xreal.client.Client;
import xreal.client.EntityState;
import xreal.client.PlayerState;
import xreal.client.Snapshot;
import xreal.common.EntityType;
import xreal.common.PlayerMovementFlags;

public class SnapshotManager {
	
	// there are only one or two snapshot_t that are relevent at a time
	
	/** the number of snapshots the client system has received */
	private int             latestSnapshotNum;
	
	/** the time from latestSnapshotNum, so we don't need to read the snapshot yet */
	private int             latestSnapshotTime;
	
	private int				processedSnapshotNum;	// the number of snapshots cgame has requested
	
	/** cg.snap->serverTime <= cg.time */
	private Snapshot		snap;
	
	/** cg.nextSnap->serverTime > cg.time, or NULL */
	private Snapshot		nextSnap;
	
	private Snapshot		activeSnapshots[] = new Snapshot[2];
	
	private boolean			thisFrameTeleport;
	private boolean			nextFrameTeleport;
	
	
	SnapshotManager(int processedSnapshotNum) {
		this.processedSnapshotNum = processedSnapshotNum;
	}
	
	
	/**
	 * We are trying to set up a renderable view, so determine
	 * what the simulated time is, and try to get snapshots
	 * both before and after that time if available.
	 * 
	 * If we don't have a valid cg.snap after exiting this function,
	 * then a 3D game view cannot be rendered.  This should only happen
	 * right after the initial connection.  After cg.snap has been valid
	 * once, it will never turn invalid.
	 * 
	 * Even if cg.snap is valid, cg.nextSnap may not be, if the snapshot
	 * hasn't arrived yet (it becomes an extrapolating situation instead
	 * of an interpolating one)
	 * @throws Exception 
	 */
	public void processSnapshots() throws Exception
	{
		Snapshot        snap;
		int             n;

		// see what the latest snapshot the client system has is
		n = Client.getCurrentSnapshotNumber();
		latestSnapshotTime = Client.getCurrentSnapshotTime();
		
		if(n != latestSnapshotNum)
		{
			if(n < latestSnapshotNum)
			{
				// this should never happen
				throw new Exception("processSnapshots: n < this.latestSnapshotNum");
			}
			latestSnapshotNum = n;
		}

		
		// If we have yet to receive a snapshot, check for it.
		// Once we have gotten the first snapshot, cg.snap will
		// always have valid data for the rest of the game
		while(this.snap == null)
		{
			snap = readNextSnapshot();
			if(snap == null)
			{
				// we can't continue until we get a snapshot
				return;
			}

			// set our weapon selection to what
			// the playerstate is currently using
			if(!snap.isInactive())
			{
				setInitialSnapshot(snap);
			}
		}

		// loop until we either have a valid nextSnap with a serverTime
		// greater than cg.time to interpolate towards, or we run
		// out of available snapshots
		do
		{
			// if we don't have a nextframe, try and read a new one in
			if(nextSnap == null)
			{
				snap = readNextSnapshot();

				// if we still don't have a nextframe, we will just have to
				// extrapolate
				if(snap == null)
				{
					break;
				}

				setNextSnap(snap);


				// if time went backwards, we have a level restart
				if(nextSnap.getServerTime() < this.snap.getServerTime())
				{
					throw new Exception("processSnapshots: Server time went backwards");
				}
			}

			// if our time is < nextFrame's, we have a nice interpolating state
			if((ClientGame.getTime() >= this.snap.getServerTime()) && (ClientGame.getTime() < this.nextSnap.getServerTime())) {
				break;
			}

			// we have passed the transition from nextFrame to frame
			transitionSnapshot();
		} while(true);

		// assert our valid conditions upon exiting
		if(this.snap == null)
		{
			throw new Exception("processSnapshots: this.snap == null");
		}
		
		if(ClientGame.getTime() < this.snap.getServerTime())
		{
			// this can happen right after a vid_restart
			ClientGame.resetTime(this.snap.getServerTime());
		}
		
		if(this.nextSnap != null && this.nextSnap.getServerTime() <= ClientGame.getTime())
		{
			throw new Exception("processSnapshots: this.nextSnap.serverTime <= ClientGame.time");
		}
	}
	
	/**
	 * This is the only place new snapshots are requested.
	 * 
	 * This may increment cgs.processedSnapshotNum multiple
	 * times if the client system fails to return a	valid snapshot.
	 * 
	 * @return 
	 */
	private Snapshot readNextSnapshot()
	{
		Snapshot       dest;

		if(latestSnapshotNum > processedSnapshotNum + 1000)
		{
			Engine.println("WARNING: SnapshotManager.readNextSnapshot: way out of range, " + latestSnapshotNum + " > " + processedSnapshotNum);
		}

		while(processedSnapshotNum < latestSnapshotNum)
		{
			// try to read the snapshot from the client system
			processedSnapshotNum++;
			
			//r = trap_GetSnapshot(processedSnapshotNum, dest);
			dest = Client.getSnapshot(processedSnapshotNum);

			// FIXME: why would trap_GetSnapshot return a snapshot with the same server time
			//if((snap != null) && (dest != null) && (dest.getServerTime() == snap.getServerTime()))
			//{
				//continue;
			//}

			// if it succeeded, return
			if(dest != null)
			{
				// decide which of the two slots to load it into
				if(snap == activeSnapshots[0])
				{
					activeSnapshots[1] = dest;
				}
				else
				{
					activeSnapshots[0] = dest;
				}
				
				ClientGame.getLagometer().addSnapshotInfo(dest);
				return dest;
			}

			// a GetSnapshot will return failure if the snapshot
			// never arrived, or  is so old that its entities
			// have been shoved off the end of the circular
			// buffer in the client system.

			// record as a dropped packet
			ClientGame.getLagometer().addSnapshotInfo(null);

			// If there are additional snapshots, continue trying to
			// read them.
		}

		// nothing left to read
		return null;
	}
	
	/**
	 * 
	 * @param snap
	 */
	private void setInitialSnapshot(Snapshot snap)
	{
		Engine.println("setInitialSnapshot(" + snap.toString() + ")");
		
		this.snap = snap;

		int ownClientNum = snap.getPlayerState().clientNum;
		ClientEntity cent = ClientGame.getEntities().get(ownClientNum);
		if(cent == null)
		{
			Engine.println("setInitialSnapshot: null own ClientPlayer");
			
			cent = new ClientPlayer(snap.getPlayerState().createEntityState(false));
			ClientGame.getEntities().setElementAt(cent, ownClientNum);
		}

		// sort out solid entities
		//TODO CG_BuildSolidList();

		//TODO CG_ExecuteNewServerCommands(snap->serverCommandSequence);

		// set our local weapon selection pointer to
		// what the server has indicated the current weapon is
		//TODO CG_Respawn();

		for(EntityState es : snap.getEntities())
		{
			cent = ClientGame.getEntities().get(es.getNumber());
			
			if(cent == null || cent.currentState.eType != es.eType)
			{
				cent = ClientGame.createClientEntity(es);
			}

			cent.currentState = es;
			
			//cent->currentState = *state;
			cent.interpolate = false;
			cent.currentValid = true;

			//CG_ResetEntity(cent);

			cent.checkEvents();
		}
		
		Engine.println("setInitialSnapshot:" + ClientGame.getEntities().toString());
	}
	
	/**
	 * A new snapshot has just been read in from the client system.
	 */
	private void setNextSnap(Snapshot snap)
	{
		this.nextSnap = snap;
		
		int ownClientNum = snap.getPlayerState().clientNum;
		ClientEntity cent = ClientGame.getEntities().get(ownClientNum);
		if(cent == null)
		{
			Engine.println("setNextSnapshot: null own ClientPlayer");
			
			cent = new ClientPlayer(snap.getPlayerState().createEntityState(false));
		}
		
		cent.nextState = snap.getPlayerState().createEntityState(false);
		cent.interpolate = true;

		// check for extrapolation errors
		for(EntityState es : snap.getEntities())
		{
			cent = ClientGame.getEntities().get(es.getNumber());

			if(cent == null || cent.currentState.eType != es.eType)
			{
				cent = ClientGame.createClientEntity(es);
			}
				
			cent.nextState = es;

			// if this frame is a teleport, or the entity wasn't in the
			// previous frame, don't interpolate
			if(!cent.currentValid || (cent.currentState.isEntityFlag_teleport() ^ es.isEntityFlag_teleport()))
			{
				cent.interpolate = false;
			}
			else
			{
				cent.interpolate = true;
			}
		}

		// if the next frame is a teleport for the playerstate, we
		// can't interpolate during demos
		if((this.snap != null) && (snap.getPlayerState().isEntityFlag_teleport() ^ this.snap.getPlayerState().isEntityFlag_teleport()))
		{
			nextFrameTeleport = true;
		}
		else
		{
			nextFrameTeleport = false;
		}

		// if changing follow mode, don't interpolate
		if(nextSnap.getPlayerState().clientNum != this.snap.getPlayerState().clientNum)
		{
			nextFrameTeleport = true;
		}
		
		// if changing server restarts, don't interpolate
		if(this.nextSnap.isServerCount() ^ this.snap.isServerCount())
		{
			nextFrameTeleport = true;
		}

		// sort out solid entities
		// TODO CG_BuildSolidList();
	}
	
	/**
	 * The transition point from snap to nextSnap has passed.
	 * @throws Exception 
	 */
	private void transitionSnapshot() throws Exception
	{
		ClientEntity cent;

		if(snap == null)
		{
			throw new Exception("transitionSnapshot: NULL this.snap");
		}
		
		if(nextSnap == null)
		{
			throw new Exception("transitionSnapshot: NULL this.nextSnap");
		}

		// execute any server string commands before transitioning entities
		// TODO CG_ExecuteNewServerCommands(cg.nextSnap->serverCommandSequence);

		// if we had a map_restart, set everything with initial
		//if(!cg.snap)
		//{
		//}

		// clear the currentValid flag for all entities in the existing snapshot
		for(EntityState es : snap.getEntities())
		{
			cent = ClientGame.getEntities().get(es.getNumber());
			
			if(cent == null || cent.currentState.eType != es.eType)
			{
				cent = ClientGame.createClientEntity(es);
			}
			
			cent.currentValid = false;
		}

		// move nextSnap to snap and do the transitions
		Snapshot oldFrame = snap;
		snap = nextSnap;

		int ownClientNum = snap.getPlayerState().clientNum;
		ClientPlayer player = (ClientPlayer) ClientGame.getEntities().get(ownClientNum);
		if(player == null)
		{
			Engine.println("transitionSnapshot: null own ClientPlayer");
			
			player = new ClientPlayer(snap.getPlayerState().createEntityState(false));
			ClientGame.getEntities().setElementAt(player, ownClientNum);
		}
		else
		{
			player.currentState = snap.getPlayerState().createEntityState(false);
			player.interpolate = true;
		}
		
		// check for playerstate transition events
		if(oldFrame != null)
		{
			PlayerState  ops, ps;

			ops = oldFrame.getPlayerState();
			ps = snap.getPlayerState();
			
			// teleporting checks are irrespective of prediction
			if(ps.isEntityFlag_teleport() ^ ops.isEntityFlag_teleport())
			{
				thisFrameTeleport = true;	// will be cleared by prediction code
			}

			// if we are not doing client side movement prediction for any
			// reason, then the client events and view changes will be issued now
			if(ClientGame.isDemoPlayback() || ((snap.getPlayerState().pm_flags & PlayerMovementFlags.FOLLOW) != 0) || CVars.cg_nopredict.getBoolean() || CVars.g_synchronousClients.getBoolean())
			{
				// TODO player.transitionState()
				//CG_TransitionPlayerState(ps, ops);
			}
		}

		for(EntityState es : snap.getEntities())
		{
			cent = ClientGame.getEntities().get(es.getNumber());
			
			if(cent == null || cent.currentState.eType != es.eType)
			{
				cent = ClientGame.createClientEntity(es);
			}
			
			cent.transitionState();

			// remember time of snapshot this entity was last updated in
			cent.snapShotTime = snap.getServerTime();
		}

		nextSnap = null;
	}
	
	
	public int getProcessedSnapshotNum() {
		return processedSnapshotNum;
	}
}
