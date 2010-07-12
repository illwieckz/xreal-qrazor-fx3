package xreal;


public abstract class CVars {
	
	public static final CVar cl_paused = new CVar("cl_paused", "0", CVar.ROM);
	
	public static final CVar ui_debug = new CVar("ui_debug", "0", 0);
	
	public static final CVar cg_stereoSeparation = new CVar("cg_stereoSeparation", "0.4", CVar.ARCHIVE);
	public static final CVar cg_fov = new CVar("cg_fov", "90", CVar.ARCHIVE);
	public static final CVar cg_viewsize = new CVar("cg_viewsize", "100", CVar.ARCHIVE);
	
	public static final CVar cg_draw2D = new CVar("cg_draw2D", "1", CVar.ARCHIVE);
	public static final CVar cg_drawFPS = new CVar("cg_drawFPS", "0", CVar.ARCHIVE);
	public static final CVar cg_drawPlayerCollision = new CVar("cg_drawPlayerCollision", "1", CVar.CHEAT);
	
	public static final CVar cg_lagometer = new CVar("cg_lagometer", "1", CVar.ARCHIVE);
	public static final CVar cg_nopredict = new CVar("cg_nopredict", "1", 0);
	
	public static final CVar cg_smoothClients = new CVar("cg_smoothClients", "0", CVar.USERINFO | CVar.ARCHIVE);
	
	public static final CVar cg_thirdPersonRange = new CVar("cg_thirdPersonRange", "40", CVar.CHEAT);
	public static final CVar cg_thirdPersonAngle = new CVar("cg_thirdPersonAngle", "0", CVar.CHEAT);
	public static final CVar cg_thirdPerson = new CVar("cg_thirdPerson", "0", 0);
	
	
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
	
	public static final CVar g_threadEntities = new CVar("g_threadEntities", "0", CVar.LATCH);
	

	// change anytime vars
	
	// turn off client side player movement prediction
	public static final CVar g_synchronousClients = new CVar("g_synchronousClients", "0", CVar.SYSTEMINFO);
	
	public static final CVar g_password = new CVar("g_password", "", CVar.USERINFO);
	public static final CVar g_dedicated = new CVar("dedicated", "0", 0);
	
	public static final CVar g_speed = new CVar("g_speed", "400", 0);
	public static final CVar g_gravityX = new CVar("g_gravityX", "0", CVar.SYSTEMINFO);
	public static final CVar g_gravityY = new CVar("g_gravityY", "0", CVar.SYSTEMINFO);
	
	// FIXME: should be 386 = SI gravity 9.81 * 39.27 inches for 1 meter
	public static final CVar g_gravityZ = new CVar("g_gravityZ", "-800", CVar.SYSTEMINFO);
	
	// don't override the cheat state set by the system
	public static final CVar sv_cheats = new CVar("sv_cheats", "", 0);
	public static final CVar sv_killserver = new CVar("sv_killserver", "", 0);
	public static final CVar sv_fps = new CVar("sv_fps", "", 0);
	
	
	// player movement
	public static final CVar pm_debug = new CVar("pm_debug", "0", 0);
	
	public static final CVar pm_useGhostObjectSweepTest = new CVar("pm_useGhostObjectSweepTest", "0", CVar.SYSTEMINFO);
	
	public static final CVar pm_stopSpeed = new CVar("pm_stopSpeed", "100", CVar.SYSTEMINFO);
	public static final CVar pm_duckScale = new CVar("pm_duckScale", "0.25", CVar.SYSTEMINFO);
	public static final CVar pm_swimScale = new CVar("pm_swimScale", "0.5", CVar.SYSTEMINFO);
	public static final CVar pm_wadeScale = new CVar("pm_wadeScale", "0.7", CVar.SYSTEMINFO);

	public static final CVar pm_accelerate = new CVar("pm_acceralerate", "10", CVar.SYSTEMINFO);
	public static final CVar pm_airAccelerate = new CVar("pm_airAcceralerate", "1", CVar.SYSTEMINFO);
	public static final CVar pm_waterAccelerate = new CVar("pm_waterAcceralerate", "4", CVar.SYSTEMINFO);
	public static final CVar pm_flyAccelerate = new CVar("pm_flyAcceralerate", "8", CVar.SYSTEMINFO);

	public static final CVar pm_friction = new CVar("pm_friction", "6", CVar.SYSTEMINFO);
	public static final CVar pm_waterFriction = new CVar("pm_waterFriction", "1", CVar.SYSTEMINFO);
	public static final CVar pm_flightFriction = new CVar("pm_flightFriction", "3", CVar.SYSTEMINFO);
	public static final CVar pm_spectatorFriction = new CVar("pm_spectatorFriction", "5", CVar.SYSTEMINFO);
	
	public static final CVar pm_jumpHeight = new CVar("pm_jumpHeight", "48", CVar.SYSTEMINFO);

}
