package xreal.client.game;

import xreal.Engine;
import xreal.client.Client;

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
				throw new Exception("processSnapshots: n < cg.latestSnapshotNum");
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

		/*
		// loop until we either have a valid nextSnap with a serverTime
		// greater than cg.time to interpolate towards, or we run
		// out of available snapshots
		do
		{
			// if we don't have a nextframe, try and read a new one in
			if(!cg.nextSnap)
			{
				snap = CG_ReadNextSnapshot();

				// if we still don't have a nextframe, we will just have to
				// extrapolate
				if(!snap)
				{
					break;
				}

				CG_SetNextSnap(snap);


				// if time went backwards, we have a level restart
				if(cg.nextSnap->serverTime < cg.snap->serverTime)
				{
					CG_Error("CG_ProcessSnapshots: Server time went backwards");
				}
			}

			// if our time is < nextFrame's, we have a nice interpolating state
			if(cg.time >= cg.snap->serverTime && cg.time < cg.nextSnap->serverTime)
			{
				break;
			}

			// we have passed the transition from nextFrame to frame
			CG_TransitionSnapshot();
		} while(1);

		// assert our valid conditions upon exiting
		if(cg.snap == NULL)
		{
			CG_Error("CG_ProcessSnapshots: cg.snap == NULL");
		}
		if(cg.time < cg.snap->serverTime)
		{
			// this can happen right after a vid_restart
			cg.time = cg.snap->serverTime;
		}
		if(cg.nextSnap != NULL && cg.nextSnap->serverTime <= cg.time)
		{
			CG_Error("CG_ProcessSnapshots: cg.nextSnap->serverTime <= cg.time");
		}
		*/
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
		boolean        r;
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
			if((snap != null) && (dest != null) && (dest.getServerTime() == snap.getServerTime()))
			{
				//continue;
			}

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
				
				ClientGame.lagometer.addSnapshotInfo(dest);
				return dest;
			}

			// a GetSnapshot will return failure if the snapshot
			// never arrived, or  is so old that its entities
			// have been shoved off the end of the circular
			// buffer in the client system.

			// record as a dropped packet
			ClientGame.lagometer.addSnapshotInfo(null);

			// If there are additional snapshots, continue trying to
			// read them.
		}

		// nothing left to read
		return null;
	}
	
	void setInitialSnapshot(Snapshot snap)
	{
		
	}
	
	
	public int getProcessedSnapshotNum() {
		return processedSnapshotNum;
	}
}
