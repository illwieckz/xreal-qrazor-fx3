package xreal.client.game;

import java.util.StringTokenizer;

import javax.vecmath.Vector3f;

import xreal.CVars;
import xreal.Engine;
import xreal.UserInfo;
import xreal.client.Client;
import xreal.client.renderer.Camera;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.renderer.StereoFrame;
import xreal.common.Config;
import xreal.common.ConfigStrings;
import xreal.common.GameType;


/**
 * Main class of the client game that is loaded by the engine.
 * 
 * @author	Robert Beckebans
 * @since	21.06.2010
 */
public class ClientGame implements ClientGameListener {

	static private int		clientFrame;	// incremented each frame
	
	static private int		clientNum;
	
	static private int		serverCommandSequence;	// reliable command stream counter
	static private int		processedSnapshotNum;	// the number of snapshots cgame has requested
	
	static private int		time;
	static private int		levelStartTime;
	
	static private boolean	demoPlayback;
	
	// information screen text during loading
	//progressInfo_t  progressInfo[NUM_PROGRESS];
	static private int      loadingProgress = 1;
	
	static private String	mapFileName;
	
	static public final Media	media = new Media();
	
	static private final Camera	camera = new Camera();
	
	static private final HUD	hud = new HUD();
	
	
	private ClientGame() {
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
		
		int             inwater;

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
//		CG_ProcessSnapshots();
//
//		// if we haven't received any snapshots yet, all
//		// we can draw is the information screen
//		if(!cg.snap || (cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE))
//		{
//			CG_DrawInformation();
//			return;
//		}
//
//		// let the client system know what our weapon and zoom settings are
//		trap_SetUserCmdValue(cg.weaponSelect, cg.zoomSensitivity);

		// this counter will be bumped for every valid scene we generate
		clientFrame++;

//		// update cg.predictedPlayerState
//		CG_PredictPlayerState();
//
//		// decide on third person view
//		cg.renderingThirdPerson = cg_thirdPerson.integer || (cg.snap->ps.stats[STAT_HEALTH] <= 0);
//
//		// build cg.refdef
//		inwater = CG_CalcViewValues();
//		if(inwater)
//		{
//			cg.refdef.rdflags |= RDF_UNDERWATER;
//		}
//
//		// first person blend blobs, done after AnglesToAxis
//		if(!cg.renderingThirdPerson)
//		{
//			CG_DamageBlendBlob();
//		}
//
//		// build the render lists
//		if(!cg.hyperspace)
//		{
//			CG_AddPacketEntities();	// adter calcViewValues, so predicted player state is correct
//			CG_AddMarks();
//			CG_AddParticles();
//			CG_AddLocalEntities();
//		}
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
//		cg.refdef.time = cg.time;
//		memcpy(cg.refdef.areamask, cg.snap->areamask, sizeof(cg.refdef.areamask));
//
//		// warning sounds when powerup is wearing off
//		CG_PowerupTimerSounds();
//
//		// update audio positions
//		trap_S_Respatialize(cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater);
//
//		// make sure the lagometerSample and frame timing isn't done twice when in stereo
//		if(stereoView != STEREO_RIGHT)
//		{
//			cg.frametime = cg.time - cg.oldTime;
//
//			if(cg.frametime < 0)
//			{
//				cg.frametime = 0;
//			}
//			cg.oldTime = cg.time;
//			CG_AddLagometerFrameInfo();
//		}
//
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
		StereoFrame stereoFrame; 
		try
		{
			stereoFrame = StereoFrame.values()[stereoView];
		}
		catch(Exception e)
		{
			throw new Exception("CG_DrawActive: Undefined stereoView");
		}
		
//		// actually issue the rendering calls
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
	public void initClientGame(int serverMessageNum, int serverCommandSequence, int clientNum) throws Exception {
		
		Engine.print("xreal.client.game.ClientGame.initClientGame(serverMessageNum = "+ serverMessageNum + ", serverCommandSequence = " + serverCommandSequence + ", clientNum = " + clientNum  + ")\n");
		
		
		Engine.print("------- CGame Initialization -------\n");
		
		//cg.progress = 0;
		
		ClientGame.clientNum = clientNum;		

		ClientGame.processedSnapshotNum = serverMessageNum;
		ClientGame.serverCommandSequence = serverCommandSequence;
		
		// check version
		String s = Client.getConfigString(ConfigStrings.GAME_VERSION);
		if(!s.equals(Config.GAME_VERSION))
		{
			//Engine.error("Client/Server game mismatch: " + Config.GAME_VERSION + "/" + s);
			
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
	
		//cgs.gametype = atoi(Info_ValueForKey(info, "g_gametype"));
		//trap_Cvar_Set("g_gametype", va("%i", cgs.gametype));
		
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
		// register fonts
		media.fontVera = Renderer.registerFont("fonts/Vera.ttf", 48);
		media.fontVeraSe = Renderer.registerFont("fonts/VeraSe.ttf", 48);
		media.fontVeraBold = Renderer.registerFont("fonts/VeraBd.ttf", 48);
		media.fontVeraSerifBold = Renderer.registerFont("fonts/VeraSeBd.ttf", 48);
		
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
		/*
		CG_TileClear();

		// offset vieworg appropriately if we're doing stereo separation
		VectorCopy(cg.refdef.vieworg, baseOrg);
		if(separation != 0)
		{
			VectorMA(cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg);
		}

		// draw 3D view
		trap_R_RenderScene(&cg.refdef);

		// restore original viewpoint if running stereo
		if(separation != 0)
		{
			VectorCopy(baseOrg, cg.refdef.vieworg);
		}
		*/

		// draw status bar and other floating elements
		hud.draw();
	}
	
	// --------------------------------------------------------------------------------------------
	
	
	public static int getClientNum() {
		return clientNum;
	}

	
	public static int getServerCommandSequence() {
		return serverCommandSequence;
	}

	public static int getProcessedSnapshotNum() {
		return processedSnapshotNum;
	}
}
