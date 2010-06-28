package xreal;


public abstract class CVars {
	
	public static final CVar cl_paused = new CVar("cl_paused", "0", CVar.ROM);
	
	public static final CVar cg_stereoSeparation = new CVar("cg_stereoSeparation", "0.4", CVar.ARCHIVE);
	public static final CVar cg_fov = new CVar("cg_fov", "90", CVar.ARCHIVE);
	public static final CVar cg_viewsize = new CVar("cg_viewsize", "100", CVar.ARCHIVE);
	
	public static final CVar cg_draw2D = new CVar("cg_draw2D", "1", CVar.ARCHIVE);
	public static final CVar cg_drawFPS = new CVar("cg_drawFPS", "0", CVar.ARCHIVE);
	
	public static final CVar cg_nopredict = new CVar("cg_nopredict", "0", 0);
	
	
	// noset vars
//	{NULL, "gamename", GAMEVERSION, CVar.SERVERINFO | CVar.ROM, 0, qfalse},
//	{NULL, "gamedate", __DATE__, CVar.ROM, 0, qfalse},
	public static final CVar g_restarted = new CVar("g_restarted", "0", CVar.ROM);
	public static final CVar g_mapname = new CVar("mapname", "", CVar.SERVERINFO | CVar.ROM);

	// latched vars
	public static final CVar g_gametype = new CVar("g_gametype", "0", CVar.SERVERINFO | CVar.USERINFO | CVar.LATCH);
	
	public static final CVar g_maxclients = new CVar("sv_maxclients", "8", CVar.SERVERINFO | CVar.LATCH | CVar.ARCHIVE);
	public static final CVar g_maxGameClients = new CVar("g_maxGameClients", "0", CVar.SERVERINFO | CVar.LATCH | CVar.ARCHIVE);

	public static final CVar g_motd = new CVar("g_motd", "", 0);

	// change anytime vars
	
	// turn off client side player movement prediction
	public static final CVar g_synchronousClients = new CVar("g_synchronousClients", "1", CVar.SYSTEMINFO);
	
	public static final CVar g_password = new CVar("g_password", "", CVar.USERINFO);
	public static final CVar g_dedicated = new CVar("dedicated", "0", 0);
	
	public static final CVar g_speed = new CVar("g_speed", "400", 0);
	public static final CVar g_gravityX = new CVar("g_gravityX", "0", CVar.SYSTEMINFO);
	public static final CVar g_gravityY = new CVar("g_gravityY", "0", CVar.SYSTEMINFO);
	
	// FIXME: should be 313.92 = 9.81 * 32 SI gravity in Quake units
	public static final CVar g_gravityZ = new CVar("g_gravityZ", "-200", CVar.SYSTEMINFO);
	
	// don't override the cheat state set by the system
	public static final CVar sv_cheats = new CVar("sv_cheats", "", 0);
	public static final CVar sv_killserver = new CVar("sv_killserver", "", 0);

}
