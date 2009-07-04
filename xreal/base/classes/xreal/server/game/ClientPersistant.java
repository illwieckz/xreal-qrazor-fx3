package xreal.server.game;

import xreal.UserCommand;


enum ClientConnected
{
	DISCONNECTED,
	CONNECTING,
	CONNECTED
}

enum SpectatorState
{
	NOT,
	FREE,
	FOLLOW,
	SCOREBOARD
}

enum PlayerTeamState
{
	BEGIN,					// Beginning a team game, spawn at base
	ACTIVE					// Now actively playing
}

/**
 * Client data that stays across multiple respawns, but is cleared
 * on each level change or team change at clientBegin().
 *  
 * @author Robert Beckebans
 */
public class ClientPersistant {
	
	ClientConnected 	connected;
	UserCommand       	cmd;			// we would lose angles if not persistant
	boolean        		localClient;	// true if "ip" info key is "localhost"
	boolean        		initialSpawn;	// the first spawn should be at a cool location
	boolean     	   	predictItemPickup;	// based on cg_predictItems userinfo
	boolean 	       	pmoveFixed;	//
	String            	netname;
	int             	maxHealth;	// for handicapping
	int             	enterTime;	// level.time the client entered the game
	PlayerTeamState 	teamState;	// status in teamplay games
	int             	voteCount;	// to prevent people from constantly calling votes
	int             	teamVoteCount;	// to prevent people from constantly calling votes
	boolean         	teamInfo;	// send team overlay updates?
}

