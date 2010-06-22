package xreal.client.game;

import xreal.client.Snapshot;

/**
 * 
 * @author Robert Beckebans
 */
public class Lagometer {
	
	static private final int LAG_SAMPLES = 128;

	private int	frameSamples[] = new int[LAG_SAMPLES];
	private int	frameCount;
	private int	snapshotFlags[] = new int[LAG_SAMPLES];
	private int snapshotSamples[] = new int[LAG_SAMPLES];
	private int snapshotCount;
	
	/**
	 * Adds the current interpolate / extrapolate bar for this frame
	 */
	void addFrameInfo(int latestSnapshotTime)
	{
		int             offset;

		offset = ClientGame.getTime() - latestSnapshotTime;
		frameSamples[frameCount & (LAG_SAMPLES - 1)] = offset;
		frameCount++;
	}

	
	/**
	 * Each time a snapshot is received, log its ping time and
	 * the number of snapshots that were dropped before it.
	 * 
	 * @param snap	Pass NULL for a dropped packet.
	 */
	void addSnapshotInfo(Snapshot snap)
	{
		// dropped packet
		if(snap == null)
		{
			snapshotSamples[snapshotCount & (LAG_SAMPLES - 1)] = -1;
			snapshotCount++;
			return;
		}

		// add this snapshot's info
		snapshotSamples[snapshotCount & (LAG_SAMPLES - 1)] = snap.getPing();
		snapshotFlags[snapshotCount & (LAG_SAMPLES - 1)] = snap.getSnapFlags();
		snapshotCount++;
	}

}
