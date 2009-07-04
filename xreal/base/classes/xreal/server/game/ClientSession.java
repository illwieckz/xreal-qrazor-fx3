package xreal.server.game;

import xreal.common.Team;

/**
 * Client data that stays across multiple levels or tournament restarts
 * this is achieved by writing all the data to cvar strings at game shutdown
 * time and reading them back at connection time.  Anything added here
 * MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData().
 * 
 * @author Robert Beckebans
 */
public class ClientSession {
	
	public Team            sessionTeam;
	public int             spectatorTime;	// for determining next-in-line to play
	public SpectatorState  spectatorState;
	public int             spectatorClient;	// for chasecam and follow mode
	public int             wins, losses;	// tournament stats
	public boolean         teamLeader;	// true when this client is a team leader
}