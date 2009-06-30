package xreal.game;

import xreal.CVar;

public class CVars {
	
	// don't override the cheat state set by the system
	public static CVar g_cheats = new CVar("sv_cheats", "", 0);

	// noset vars
//	{NULL, "gamename", GAMEVERSION, CVar.SERVERINFO | CVar.ROM, 0, qfalse},
//	{NULL, "gamedate", __DATE__, CVar.ROM, 0, qfalse},
	public static CVar g_restarted = new CVar("g_restarted", "0", CVar.ROM);
//	{NULL, "sv_mapname", "", CVar.SERVERINFO | CVar.ROM, 0, qfalse},

	// latched vars
	public static CVar g_gametype = new CVar("g_gametype", "0", CVar.SERVERINFO | CVar.USERINFO | CVar.LATCH);
	public static CVar g_maxclients = new CVar("sv_maxclients", "8", CVar.SERVERINFO | CVar.LATCH | CVar.ARCHIVE);
	public static CVar g_maxGameClients = new CVar("g_maxGameClients", "0", CVar.SERVERINFO | CVar.LATCH | CVar.ARCHIVE);

	// change anytime vars
}
