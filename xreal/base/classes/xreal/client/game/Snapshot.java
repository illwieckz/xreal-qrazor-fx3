package xreal.client.game;

import xreal.EntityState;
import xreal.PlayerState;


/**
 * Snapshots are a view of the server at a given time.
 * 
 * Snapshots are generated at regular time intervals by the server,
 * but they may not be sent if a client's rate level is exceeded, or
 * they may be dropped by the network.
 * 
 * @author Robert Beckebans
 */
public class Snapshot {
	
	
	//private static final int MAX_ENTITIES_IN_SNAPSHOT = 512;	// was 256 in vanilla Q3A
	
	//
	// same as in q_shared.h
	//
	private static final int SNAPFLAG_RATE_DELAYED = 1;
	
	/** snapshot used during connection and for zombies */
	private static final int SNAPFLAG_NOT_ACTIVE = 2;
	
	/** toggled every map_restart so transitions can be detected */
	private static final int SNAPFLAG_SERVERCOUNT = 4;

	/** SNAPFLAG_RATE_DELAYED, etc */
	private int             snapFlags;
	private int             ping;

	/** server time the message is valid for (in msec) */
	private int             serverTime;

	/** portalarea visibility bits */
	private byte            areamask[]; //MAX_MAP_AREA_BYTES];	// 

	/** complete information about the current player at this time */
	private PlayerState		ps;

	/**
	 * all of the entities that need to be presented at the time of this snapshot
	 */
	private EntityState		entities[];		//[MAX_ENTITIES_IN_SNAPSHOT]; 
	
	/** text based server commands to execute when this */
	private int             numServerCommands;
	private int             serverCommandSequence;
	
	
	public Snapshot(int snapFlags, int ping, int serverTime, byte areamask[], PlayerState ps, EntityState[] entities, int numServerCommands, int serverCommandSequence)
	{
		super();
		this.snapFlags = snapFlags;
		this.ping = ping;
		this.serverTime = serverTime;
		this.areamask = areamask;
		this.ps = ps;
		this.entities = entities;
		this.numServerCommands = numServerCommands;
		this.serverCommandSequence = serverCommandSequence;
	}


	public int getSnapFlags() {
		return snapFlags;
	}


	public int getPing() {
		return ping;
	}


	public int getServerTime() {
		return serverTime;
	}


	public PlayerState getPlayerState() {
		return ps;
	}


	public EntityState[] getEntities() {
		return entities;
	}
	

	public int getNumServerCommands() {
		return numServerCommands;
	}


	public int getServerCommandSequence() {
		return serverCommandSequence;
	}
	
	public boolean isRateDelayed() {
		return ((snapFlags & SNAPFLAG_RATE_DELAYED) != 0);
	}
	
	public boolean isInactive() {
		return ((snapFlags & SNAPFLAG_NOT_ACTIVE) != 0);
	}
	
	public boolean isServerCount() {
		return ((snapFlags & SNAPFLAG_SERVERCOUNT) != 0);
	}
}
