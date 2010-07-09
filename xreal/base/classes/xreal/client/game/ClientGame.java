package xreal.client.game;

import java.util.Set;
import java.util.StringTokenizer;
import java.util.Vector;

import javax.vecmath.Vector3f;

import xreal.CVars;
import xreal.Engine;
import xreal.UserInfo;
import xreal.client.Client;
import xreal.client.EntityState;
import xreal.client.renderer.Camera;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.renderer.StereoFrame;
import xreal.common.Config;
import xreal.common.ConfigStrings;
import xreal.common.EntityType;
import xreal.common.GameType;
import xreal.server.game.GameEntity;


/**
 * Main class of the client game that is loaded by the engine.
 * 
 * @author	Robert Beckebans
 * @since	21.06.2010
 */
public class ClientGame implements ClientGameListener {

	static private int			clientFrame;	// incremented each frame
	
	static private int			clientNum;
	
	static private int			serverCommandSequence;	// reliable command stream counter
	
	/**  this is the time value that the client is rendering at */
	static private int			time;
	
	/** time at last frame, used for missile trails and prediction checking */
	static private int			oldTime;
	
	/** time - oldTime */
	static private int			frameTime;
	
	static private int			levelStartTime;
	
	static private boolean		demoPlayback;

	// information screen text during loading
	//progressInfo_t  progressInfo[NUM_PROGRESS];
	static private int      	loadingProgress = 1;
	
	static private String		mapFileName;
	
	static private Media		media;
	
	static private ClientCamera	camera;
	
	static private HUD			hud;
	
	static private SnapshotManager snapshotManager;
	
	static private PredictionManager predictionManager;
	
	static private Lagometer	lagometer;
	
	static private Vector<CEntity> entities;
	
	
	private ClientGame(int serverMessageNum, int serverCommandSequence, int clientNum) throws Exception {
		
		Engine.print("xreal.client.game.ClientGame.ClientGame(serverMessageNum = "+ serverMessageNum + ", serverCommandSequence = " + serverCommandSequence + ", clientNum = " + clientNum  + ")\n");
		
		Engine.print("------- CGame Initialization -------\n");
		
		// clear everything
		media = new Media();
		camera = new ClientCamera();
		hud = new HUD();
		snapshotManager = new SnapshotManager(serverMessageNum);
		predictionManager = new PredictionManager();
		lagometer = new Lagometer();
		
		entities = new Vector<CEntity>();
		for(int i = 0; i < Engine.MAX_GENTITIES; i++) {
			entities.add(null);
		}
		
		
		// create own player entity ---------------------------------------------------------------
		// ----------------------------------------------------------------------------------------
		
		//cg.progress = 0;
		
		ClientGame.clientNum = clientNum;

		ClientGame.serverCommandSequence = serverCommandSequence;
		
		// make sure we are running the same version of the game as the server does
		String s = Client.getConfigString(ConfigStrings.GAME_VERSION);
		if(!s.equals(Config.GAME_VERSION))
		{
			throw new Exception("Client/Server game mismatch: " + Config.GAME_VERSION + "/" + s);
		}

		s = Client.getConfigString(ConfigStrings.LEVEL_START_TIME);
		ClientGame.levelStartTime = Integer.parseInt(s);
		
		
		parseServerinfo();
		
		registerGraphics();
		
		// TODO

		startMusic();
		
		// we are done loading 
		loadingProgress = 0;
		
		System.gc();
	}
	
	@Override
	public boolean consoleCommand() {
		
		//Engine.print("xreal.client.game.ClientGame.consoleCommand()\n");
		
		String cmd = Engine.getConsoleArgv(0);
		String args = Engine.getConsoleArgs();
		
		Engine.print("xreal.client.game.ClientGame.consoleCommand(command = '" + cmd + "', args='" + args + "')\n");
		
		return false;
	}

	@Override
	public int crosshairPlayer() {
		
		Engine.println("xreal.client.game.ClientGame.crosshairPlayer()");
		
		// TODO Auto-generated method stub
		return -1;
	}

	@Override
	public void drawActiveFrame(int serverTime, int stereoView, boolean demoPlayback) throws Exception {
		
		//Engine.print("xreal.client.game.ClientGame.drawActiveFrame(serverTime = "+ serverTime + ", stereoView = " + stereoView + ", demoPlayback = " + demoPlayback + ")\n");
		
		time = serverTime;
		ClientGame.demoPlayback = demoPlayback;

		// update cvars
		//CG_UpdateCvars();

		// if we are only updating the screen as a loading
		// pacifier, don't even try to read snapshots
		if(loadingProgress > 0)
		{
			//CG_DrawInformation();
			return;
		}

		// any looped sounds will be respecified as entities
		// are added to the render list
		Client.clearLoopingSounds(false);

		// clear all the render lists
		Renderer.clearScene();

		// set up cg.snap and possibly cg.nextSnap
		snapshotManager.processSnapshots();

		// if we haven't received any snapshots yet, all
		// we can draw is the information screen
		if(!snapshotManager.hasValidSnapshot())
		{
			//CG_DrawInformation();
			return;
		}

		// let the client system know what our weapon and zoom settings are
//		trap_SetUserCmdValue(cg.weaponSelect, cg.zoomSensitivity);

		// this counter will be bumped for every valid scene we generate
		clientFrame++;

		// update cg.predictedPlayerState
		predictionManager.predictPlayerState();
//
//		// build cg.refdef
		boolean inwater = camera.calcViewValues(predictionManager.getPredictedPlayerState());
		if(inwater)
		{
			camera.rdflags |= Camera.RDF_UNDERWATER;
		}
//
//		// first person blend blobs, done after AnglesToAxis
//		if(!cg.renderingThirdPerson)
//		{
//			CG_DamageBlendBlob();
//		}
//
//		// build the render lists
		if(!predictionManager.isHyperspace())
		{
			addPacketEntities();	// adter calcViewValues, so predicted player state is correct
//			CG_AddMarks();
//			CG_AddParticles();
//			CG_AddLocalEntities();
		}
//		CG_AddViewWeapon(&cg.predictedPlayerState);
//
//		// add buffered sounds
//		CG_PlayBufferedSounds();
//
//		// play buffered voice chats
//		CG_PlayBufferedVoiceChats();
//
//		// finish up the rest of the refdef
//		if(cg.testModelEntity.hModel)
//		{
//			CG_AddTestModel();
//		}
//
//		// Tr3B - test light to preview Doom3 style light attenuation shaders
//		if(cg.testLight.attenuationShader)
//		{
//			CG_AddTestLight();
//		}
//
//		memcpy(cg.refdef.areamask, cg.snap->areamask, sizeof(cg.refdef.areamask));
//
//		// warning sounds when powerup is wearing off
//		CG_PowerupTimerSounds();
//
//		// update audio positions
//		trap_S_Respatialize(cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater);

		
		StereoFrame stereoFrame; 
		try
		{
			stereoFrame = StereoFrame.values()[stereoView];
		}
		catch(Exception e)
		{
			throw new Exception("drawActiveFrame: Undefined stereoView");
		}
		
		// make sure the lagometerSample and frame timing isn't done twice when in stereo
		if(stereoFrame != StereoFrame.STEREO_RIGHT)
		{
			frameTime = time - oldTime;

			if(frameTime < 0)
			{
				frameTime = 0;
			}
			oldTime = time;
			
			lagometer.addFrameInfo(snapshotManager.getLatestSnapshotTime());
		}

//		if(cg_timescale.value != cg_timescaleFadeEnd.value)
//		{
//			if(cg_timescale.value < cg_timescaleFadeEnd.value)
//			{
//				cg_timescale.value += cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
//				if(cg_timescale.value > cg_timescaleFadeEnd.value)
//					cg_timescale.value = cg_timescaleFadeEnd.value;
//			}
//			else
//			{
//				cg_timescale.value -= cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
//				if(cg_timescale.value < cg_timescaleFadeEnd.value)
//					cg_timescale.value = cg_timescaleFadeEnd.value;
//			}
//
//			if(cg_timescaleFadeSpeed.value)
//			{
//				trap_Cvar_Set("timescale", va("%f", cg_timescale.value));
//			}
//		}
//
		
		
		// actually issue the rendering calls
		drawActive(stereoFrame);
//
//		if(cg_stats.integer)
//		{
//			CG_Printf("cg.clientFrame:%i\n", cg.clientFrame);
//		}

	}

	@Override
	public void eventHandling(int type) {
		
		Engine.println("xreal.client.game.ClientGame.eventHandling(type = " + type + ")");
		
		// TODO Auto-generated method stub
	}

	@Override
	public void keyEvent(int time, int key, boolean down) {
		
		Engine.println("xreal.client.game.ClientGame.keyEvent(time = " + time +", key = " + key + ", down = " + down + ")");
		
		// TODO Auto-generated method stub
	}

	@Override
	public int lastAttacker() {
		
		Engine.println("xreal.client.game.ClientGame.lastAttacker()");
		
		// TODO Auto-generated method stub
		return -1;
	}

	@Override
	public void mouseEvent(int time, int dx, int dy) {
		
		Engine.println("xreal.client.game.ClientGame.mouseEvent(time = " + time +", dx = " + dx + ", dy = " + dy + ")");
		
		// TODO Auto-generated method stub
	}

	@Override
	public void shutdownClientGame() {
		
		Engine.println("xreal.client.game.ClientGame.shutdownClientGame()");
		
		// TODO Auto-generated method stub
	}

	// --------------------------------------------------------------------------------------------
	
	
	/**
	 * This is called explicitly when the gamestate is first received,
	 * and whenever the server updates any serverinfo flagged cvars.
	 */
	private void parseServerinfo()
	{
		UserInfo info = new UserInfo();
		
		info.read(Client.getConfigString(ConfigStrings.SERVERINFO));
	
		String gametype = info.get("g_gametype");
		CVars.g_gametype.set(gametype);
		Engine.println("Game Type: " + GameType.values()[CVars.g_gametype.getInteger()]);
		
		mapFileName = "maps/" + info.get("mapname") + ".bsp";
		Engine.println("ClientGame.parseServerinfo: mapname = '" + mapFileName + "'");
		
		//Com_sprintf(cgs.mapname, sizeof(cgs.mapname), "maps/%s.bsp", mapname);
		
		/*
		cgs.dmflags = atoi(Info_ValueForKey(info, "dmflags"));
		cgs.teamflags = atoi(Info_ValueForKey(info, "teamflags"));
		cgs.fraglimit = atoi(Info_ValueForKey(info, "fraglimit"));
		cgs.capturelimit = atoi(Info_ValueForKey(info, "capturelimit"));
		cgs.timelimit = atoi(Info_ValueForKey(info, "timelimit"));
		cgs.maxclients = atoi(Info_ValueForKey(info, "sv_maxclients"));
		mapname = Info_ValueForKey(info, "mapname");
		
		Q_strncpyz(cgs.redTeam, Info_ValueForKey(info, "g_redTeam"), sizeof(cgs.redTeam));
		trap_Cvar_Set("g_redTeam", cgs.redTeam);
		Q_strncpyz(cgs.blueTeam, Info_ValueForKey(info, "g_blueTeam"), sizeof(cgs.blueTeam));
		trap_Cvar_Set("g_blueTeam", cgs.blueTeam);
		*/
	}
	
	private void registerGraphics()
	{
		
		
		// clear any references to old media
		//memset(&cg.refdef, 0, sizeof(cg.refdef));
		
		Renderer.clearScene();

		//CG_LoadingString(cgs.mapname, qfalse);
		
		Renderer.loadWorldBsp(mapFileName);
	}
	
	private void startMusic()
	{
		String s = Client.getConfigString(ConfigStrings.MUSIC);
		
		if(s != null)
		{
			StringTokenizer st = new StringTokenizer(s, " ");
			
			String parm1 = "", parm2 = "";
			if(st.hasMoreTokens()) {
				parm1 = st.nextToken();
			}
			
			if(st.hasMoreTokens()) {
				parm2 = st.nextToken();
			}
			
			// start the background music
			Client.startBackgroundTrack(parm1, parm2);
		}
	}
	
	/**
	 * Perform all drawing needed to completely fill the screen
	 * 
	 * @param stereoView
	 * @throws Exception 
	 */
	public void drawActive(StereoFrame stereoView) throws Exception
	{
		float           separation;
		Vector3f        baseOrg;

		/*
		// optionally draw the info screen instead
		if(!cg.snap)
		{
			CG_DrawInformation();
			return;
		}

		// optionally draw the tournement scoreboard instead
		if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR && (cg.snap->ps.pm_flags & PMF_SCOREBOARD))
		{
			CG_DrawTourneyScoreboard();
			return;
		}
		*/

		switch (stereoView)
		{
			case STEREO_CENTER:
				separation = 0;
				break;
			case STEREO_LEFT:
				separation = -CVars.cg_stereoSeparation.getValue() / 2;
				break;
			case STEREO_RIGHT:
				separation = CVars.cg_stereoSeparation.getValue() / 2;
				break;
			default:
				separation = 0;
		}


		// clear around the rendered view if sized down
		//CG_TileClear();

		/*
		// offset vieworg appropriately if we're doing stereo separation
		VectorCopy(cg.refdef.vieworg, baseOrg);
		if(separation != 0)
		{
			VectorMA(cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg);
		}
		*/

		// draw 3D view
		Renderer.renderScene(camera);

		/*
		// restore original viewpoint if running stereo
		if(separation != 0)
		{
			VectorCopy(baseOrg, cg.refdef.vieworg);
		}
		*/

		// draw status bar and other floating elements
		hud.render();
		lagometer.render();
	}
	
	
	public static CEntity createClientEntity(EntityState es) {
		
		CEntity cent;
		
		// check for state.eType and create objects inherited from ClientEntity
		EntityType eType = es.eType;
			
		switch (eType)
		{
			default:
			case GENERAL:
				cent = new CEntity_General(es);
				break;
			
			case PLAYER:
				cent = new CEntity_Player(es);
				break;
				
			case PHYSICS_BOX:
				cent = new CEntity_PhysicsBox(es);
				break;
		}
		
		// puts itself in the correct slot
		
		//ClientGame.getEntities().setElementAt(cent, es.getNumber());
		
		return cent;
	}
	
	private void addPacketEntities() throws Exception
	{
		/*
		playerState_t  *ps;

		// the auto-rotating items will all have the same axis
		cg.autoAngles[0] = 0;
		cg.autoAngles[1] = (cg.time & 2047) * 360 / 2048.0;
		cg.autoAngles[2] = 0;

		cg.autoAnglesFast[0] = 0;
		cg.autoAnglesFast[1] = (cg.time & 1023) * 360 / 1024.0f;
		cg.autoAnglesFast[2] = 0;

		AnglesToAxis(cg.autoAngles, cg.autoAxis);
		AnglesToAxis(cg.autoAnglesFast, cg.autoAxisFast);
		*/

		// generate and add the entity from the playerstate
		CEntity_Player self = predictionManager.getPredictedPlayerEntity();
		self.addToRenderer();

		
		// lerp the non-predicted value for lightning gun origins
		/*
		CG_CalcEntityLerpPositions(&cg_entities[cg.snap->ps.clientNum]);
		*/

		// add each entity sent over by the server
		EntityState[] entityStates = snapshotManager.getSnapshot().getEntities(); 
		for(EntityState es : entityStates)
		{
			CEntity cent = entities.get(es.getNumber());
			
			if(cent != null)
			{
				cent.addToRenderer();
			}
		}
	}
	
	// --------------------------------------------------------------------------------------------
	
	
	public static int getClientNum() {
		return clientNum;
	}

	
	public static int getServerCommandSequence() {
		return serverCommandSequence;
	}

	/**
	 * Can be set by the SnapshotManager after a vid_restart.
	 * 
	 * @param time
	 */
	public static void resetTime(int time) {
		ClientGame.time = time;
	}
	
	public static int getTime() {
		return time;
	}
	
	public static Media getMedia() {
		return media;
	}
	
	public static Vector<CEntity> getEntities() {
		return entities;
	}
	
	public static SnapshotManager getSnapshotManager() {
		return snapshotManager;
	}

	public static Lagometer getLagometer() {
		return lagometer;
	}
	
	public static ClientCamera getCamera() {
		return camera;
	}
	
	public static boolean isDemoPlayback() {
		return demoPlayback;
	}
}
