package xreal.common;

import xreal.Engine;

/**
 * Config strings are a general means of communicating variable length strings
 * from the server to all connected clients.
 * 
 * @author Robert Beckebans
 */
public class ConfigStrings {

	// these are the only configstrings that the system reserves, all the
	// other ones are strictly for servergame to clientgame communication

	/**
	 * an info string with all the serverinfo cvars
	 */
	@SuppressWarnings("unused")
	public static final int SERVERINFO = 0;

	/**
	 * an info string for server system to client system configuration
	 * (timescale, etc)
	 */
	@SuppressWarnings("unused")
	private static final int SYSTEMINFO = 1;

	public static final int MUSIC = 2;

	/**
	 * From the map worldspawn's message field.
	 */
	public static final int MESSAGE = 3;

	/**
	 * g_motd string for server message of the day
	 */
	public static final int MOTD = 4;

	/**
	 * server time when the match will be restarted
	 */
	public static final int WARMUP = 5;

	public static final int SCORES1 = 6;
	public static final int SCORES2 = 7;
	public static final int VOTE_TIME = 8;
	public static final int VOTE_STRING = 9;
	public static final int VOTE_YES = 10;
	public static final int VOTE_NO = 11;

	public static final int TEAMVOTE_TIME = 12;
	public static final int TEAMVOTE_STRING = 14;
	public static final int TEAMVOTE_YES = 16;
	public static final int TEAMVOTE_NO = 18;

	public static final int GAME_VERSION = 20;
	
	/**
	 * so the timer only shows the current level
	 */
	public static final int LEVEL_START_TIME = 21;

	/**
	 * when 1, fraglimit/timelimit has been hit and intermission will start in a
	 * second or two
	 */
	public static final int INTERMISSION = 22;

	/**
	 * string indicating flag status in CTF
	 */
	public static final int FLAGSTATUS = 23;
	public static final int SHADERSTATE = 24;
	public static final int BOTINFO = 25;

	/**
	 * string of 0's and 1's that tell which items are present
	 */
	public static final int ITEMS = 27;

	public static final int MODELS = 32;
	public static final int SOUNDS = (MODELS + Engine.MAX_MODELS);
	public static final int PLAYERS = (SOUNDS + Engine.MAX_SOUNDS);
	public static final int LOCATIONS = (PLAYERS + Engine.MAX_CLIENTS);
	public static final int EFFECTS = (LOCATIONS + Engine.MAX_LOCATIONS);

	public static final int MAX = (EFFECTS + Engine.MAX_EFFECTS);

}
