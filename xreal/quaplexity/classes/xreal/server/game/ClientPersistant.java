package xreal.server.game;

import xreal.UserCommand;



/**
 * Client data that stays across multiple respawns, but is cleared
 * on each level change or team change at clientBegin().
 *  
 * @author Robert Beckebans
 */
public class ClientPersistant {
	
	public ClientConnectionState 	connected = ClientConnectionState.DISCONNECTED;
	
	/**
	 * we would lose angles if not persistant
	 */
	public UserCommand      	cmd;
	
	/**
	 * true if "ip" info key is "localhost"
	 */
	public boolean        		localClient;
	
	/**
	 * the first spawn should be at a cool location
	 */
	public boolean        		initialSpawn;
	
	/**
	 * based on cg_predictItems userinfo
	 */
	public boolean     	   		predictItemPickup; 
	public boolean 	       		pmoveFixed;
	public String           	netname;
	
	/**
	 * for handicapping
	 */
	public int             		maxHealth;
	
	/**
	 * level.time the client entered the game
	 */
	public int             		enterTime;
	
	/**
	 * status in teamplay games
	 */
	public PlayerTeamState 		teamState;
	
	/**
	 * to prevent people from constantly calling votes
	 */
	public int             		voteCount;
	
	/**
	 * to prevent people from constantly calling votes
	 */
	public int             		teamVoteCount;
	
	/**
	 * send team overlay updates?
	 */
	public boolean         		teamInfo;
}

