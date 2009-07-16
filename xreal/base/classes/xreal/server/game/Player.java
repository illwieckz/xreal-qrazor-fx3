package xreal.server.game;

import javax.vecmath.Vector3f;

import xreal.Angle3f;
import xreal.ConsoleColorStrings;
import xreal.Engine;
import xreal.PlayerStateAccess;
import xreal.UserCommand;
import xreal.UserInfo;
import xreal.common.ConfigStrings;
import xreal.common.GameType;
import xreal.common.PlayerController;
import xreal.common.PlayerMove;
import xreal.common.PlayerMovementType;
import xreal.common.Team;
import xreal.server.Server;

/**
 * Represents, uses and writes to a native gclient_t
 * 
 * @author Robert Beckebans
 */
public class Player extends GameEntity implements ClientListener, PlayerStateAccess {
	
	UserInfo _userInfo = new UserInfo();
	
	private ClientPersistant	_pers = new ClientPersistant();
	private ClientSession		_sess = new ClientSession();
	private PlayerController	_playerController = new PlayerController();
	private int					_lastCmdTime; 
	
	// --------------------------------------------------------------------------------------------
	
	/**
	 * Send a command to the client which will be interpreted by the client game module
	 * 
	 * @param string
	 */
	public synchronized native static void sendClientCommand(int clientNum, String command);
	
	private static native String	getUserInfo(int clientNum);
	private static native void 		setUserInfo(int clientNum, String s);
	
	// --------------------------------------------------------------------------------------------
	
	
	Player(int clientNum, boolean firstTime, boolean isBot) throws GameException
	{
		super(clientNum);
		
		_sess.sessionTeam = Team.SPECTATOR;
		
		String userinfo = getUserInfo(clientNum);
		if(userinfo.length() == 0)
			userinfo = "\\name\\badinfo";
		
		clientUserInfoChanged(userinfo);
		
		// IP filtering
		// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=500
		// recommanding PB based IP / GUID banning, the builtin system is pretty limited
		// check to see if they are on the banned IP list
		String ip = _userInfo.get("ip");
		/*
		if(G_FilterPacket(ip))
		{
			throw new GameException("You are banned from this server.");
		}
		*/

		// we don't check password for bots and local client
		// NOTE: local client <-> "ip" "localhost"
		//   this means this client is not running in our current process
		if(!isBot && ip != null && !ip.equals("localhost"))
		{
			// check for a password
			String userPassword = _userInfo.get("password");
			String requiredPassword = CVars.g_password.getString();
			
			if(requiredPassword != null && requiredPassword.length() > 0 && !requiredPassword.equals(userPassword))
			{
				throw new GameException("Invalid password");
			}
		}

		_pers.connected = ClientConnectionState.CONNECTING;
		

		// read or initialize the session data
		/*
		if(firstTime || level.newSession)
		{
			G_InitSessionData(client, userinfo);
		}
		G_ReadSessionData(client);
		*/
		
		Game.getPlayers().add(this);
	}

	@Override
	public void clientBegin() {
		Engine.print("xreal.server.game.Player.clientBegin(clientNum = " + getEntityState_number() + ")\n");
		
		_pers.connected = ClientConnectionState.CONNECTED;
		_pers.enterTime = Game.getLevelTime();
		
		//_pers.teamState.state = Team.BEGIN;
		
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		Angle3f viewAngles = getPlayerState_viewAngles();
		setPlayerState_deltaPitch(Angle3f.toShort(viewAngles.x));
		setPlayerState_deltaYaw(Angle3f.toShort(viewAngles.y));
		setPlayerState_deltaRoll(Angle3f.toShort(viewAngles.z));

	}

	@Override
	public void clientCommand() {
		//Engine.print("xreal.server.game.Player.clientCommand(clientNum = " + getEntityIndex() + ")\n");
		
		String cmd = Engine.getConsoleArgv(0);
		
		Engine.print("xreal.server.game.Player.clientCommand(clientNum = " + getEntityState_number() + ", command '" + cmd + "')\n");
		
		if (cmd.equals("say")) {
			Server.broadcastClientCommand("chat \"" + _pers.netname + ": " + ConsoleColorStrings.GREEN + Engine.concatConsoleArgs(1) + "\n\"");

		} else if (cmd.equals("shootbox")) {

			Vector3f forward = new Vector3f();
			getPlayerState_viewAngles().getVectors(forward, null, null);
			
			GameEntity ent = new TestBox(getPlayerState_origin(), forward);
			//ent.start();
			
		} else if (cmd.equals("shootboxes")) {

			
			Vector3f forward = new Vector3f();
			Vector3f right = new Vector3f();
			Vector3f up = new Vector3f();
			
			getPlayerState_viewAngles().getVectors(forward, right, up);
			
			Vector3f origin = getPlayerState_origin();
			Vector3f newOrigin = new Vector3f();
			
			for(int i = -48; i < 48; i += 12)
			{
				newOrigin.scaleAdd(i, right, origin);
				GameEntity ent = new TestBox(newOrigin, forward);
			}
			
			//ent.start();
			
		} else {
			sendClientCommand(getEntityState_number(), "print \"unknown cmd " + cmd + "\n\"");
		}
	}
	
	@Override
	public void clientDisconnect() {
		Engine.print("xreal.server.game.Player.clientDisconnect(clientNum = " + getEntityState_number() + ")\n");
	}

	@Override
	public void clientThink(UserCommand ucmd) {
		//Engine.print("xreal.server.game.Player.clientThink(clientNum = " + getEntityIndex() + ")\n");
		
		//Engine.println(ucmd.toString());
		
		// mark the time we got info, so we can display the
		// phone jack if they don't get any for a while
		_lastCmdTime = Game.getLevelTime();
		
		/*
		//if(!CVars.g_synchronousClients.getBoolean())
		{
			//ClientThink_real(ent);
			
			// shut up client about outdated player states
			setPlayerState_commandTime(ucmd.serverTime);
			
			setPlayerState_pm_type(PlayerMovementType.SPECTATOR);
		}
		*/
		
		// spectators don't do much
		if(_sess.sessionTeam == Team.SPECTATOR)
		{
			if(_sess.spectatorState == SpectatorState.SCOREBOARD)
			{
				return;
			}
			
			spectatorThink(ucmd);
			return;
		}
		
		// TODO more movement
	}
	
	private void spectatorThink(UserCommand ucmd) {
		
		//Engine.println("spectatorThink()");
		
		PlayerMove pm = new PlayerMove(this, ucmd, 0, 0, 0, 0, true, false, 0);
		
		//if(_sess.spectatorState != SpectatorState.FOLLOW)
		{
			setPlayerState_pm_type(PlayerMovementType.NOCLIP);
			setPlayerState_speed(700);	// faster than normal

			// perform a pmove
			_playerController.movePlayer(pm);

			// save results of pmove
			//VectorCopy(client->ps.origin, ent->s.origin);

			//G_TouchTriggers(ent);
			//trap_UnlinkEntity(ent);
		}

		//client->oldbuttons = client->buttons;
		//client->buttons = ucmd->buttons;

		// attack button cycles through spectators
		/*
		if((client->buttons & BUTTON_ATTACK) && !(client->oldbuttons & BUTTON_ATTACK))
		{
			Cmd_FollowCycle_f(ent, 1);
		}
		*/
	}

	/**
	 * Called from Player() when the player first connects and
	 * directly by the server system when the player updates a userinfo variable.
	 * 
	 * The game can override any of the settings and call Player.setUserinfo
	 * if desired.
	 * 
	 * @param userinfo
	 *            the userinfo string, formatted as:
	 *            "\keyword\value\keyword\value\....\keyword\value"
	 */
	@Override
	public void clientUserInfoChanged(String userinfo) {
		Engine.print("xreal.server.game.Player.clientUserInfoChanged(clientNum = " + getEntityState_number() + ")\n");

		if (userinfo == null)
			return;

		// fill and update the user info hash table
		_userInfo.read(userinfo);

		//Engine.println("Player.userinfo = " + _userInfo.toString());

		// check for local client
		String ip = _userInfo.get("ip");
		if (ip.equals("localhost")) {
			_pers.localClient = true;
		}

		// set name
		String oldname = _pers.netname;
		String name = _userInfo.get("name");

		// TODO _pers.netname = ClientCleanName(name);
		_pers.netname = name;

		if (_sess.sessionTeam == Team.SPECTATOR) {
			if (_sess.spectatorState == SpectatorState.SCOREBOARD) {
				_pers.netname = "scoreboard";
			}
		}

		if (_pers.connected == ClientConnectionState.CONNECTED) {
			if (!_pers.netname.equals(oldname)) {
				Server.broadcastClientCommand("print \"" + oldname + ConsoleColorStrings.WHITE + " renamed to " + _pers.netname + "\n\"");
			}
		}
		
		// set model
		String model = _userInfo.get("model");

		// bots set their team a few frames later
		Team team = _sess.sessionTeam;
		
		GameType gt = GameType.values()[CVars.g_gametype.getInteger()];
		if((gt == GameType.TEAM || gt == GameType.CTF || gt == GameType.ONEFLAG || gt == GameType.OBELISK || gt == GameType.HARVESTER) /* && g_entities[clientNum].r.svFlags & SVF_BOT */)
		{
			/*
			s = Info_ValueForKey(userinfo, "team");
			if(!Q_stricmp(s, "red") || !Q_stricmp(s, "r"))
			{
				team = TEAM_RED;
			}
			else if(!Q_stricmp(s, "blue") || !Q_stricmp(s, "b"))
			{
				team = TEAM_BLUE;
			}
			else
			{
				// pick the team with the least number of players
				team = PickTeam(clientNum);
			}
			*/
		}
		
		// team task (0 = none, 1 = offence, 2 = defence)
		String teamTask = _userInfo.get("teamtask");
		
		
		// team Leader (1 = leader, 0 is normal player)
		boolean teamLeader = _sess.teamLeader;
		

		// colors
		String c1 = _userInfo.get("color1");
		String c2 = _userInfo.get("color2");

		/*
		 * Com_sprintf(userinfo, sizeof(userinfo),
		 * "n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\g_redteam\\%s\\g_blueteam\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d"
		 * , client->pers.netname, team, model, "", redTeam, blueTeam, c1, c2,
		 * client->pers.maxHealth, client->sess.wins, client->sess.losses,
		 * teamTask, teamLeader);
		 */
		
		// build new user info CG_NewClientInfo
		
		UserInfo uinfo = new UserInfo();
		uinfo.put("n", _pers.netname);
		uinfo.put("t",  team.toString());
		uinfo.put("model", model);
		uinfo.put("hmodel", "");
		uinfo.put("g_redteam", "");
		uinfo.put("g_redteam", "");
		uinfo.put("c1", c1);
		uinfo.put("c2", c2);
		uinfo.put("hc", _pers.maxHealth);
		uinfo.put("w", _sess.wins);
		uinfo.put("l", _sess.losses);
		uinfo.put("tt", teamTask);
		uinfo.put("tl", teamLeader);
		
		//Engine.println("CS_PLAYERS userinfo = '" + uinfo.toString() + "'");
		
		Server.setConfigString(ConfigStrings.PLAYERS + getEntityState_number(), uinfo.toString());
		
	}
	
	// ------------------- playerState_t:: fields in gclient_t::ps --------------------------------
	
	private synchronized static native int getPlayerState_commandTime(int clientNum);

	private synchronized static native void setPlayerState_commandTime(int clientNum, int commandTime);

	private synchronized static native int getPlayerState_pm_type(int clientNum);

	private synchronized static native void setPlayerState_pm_type(int clientNum, int pm_type);

	private synchronized static native int getPlayerState_pm_flags(int clientNum);

	private synchronized static native void setPlayerState_pm_flags(int clientNum, int pm_flags);
	
	private synchronized static native void addPlayerState_pm_flags(int clientNum, int pm_flags);
	
	private synchronized static native void delPlayerState_pm_flags(int clientNum, int pm_flags);
	
	private synchronized static native boolean hasPlayerState_pm_flags(int clientNum, int pm_flags);

	private synchronized static native int getPlayerState_pm_time(int clientNum);

	private synchronized static native void setPlayerState_pm_time(int clientNum, int pm_time);

	private synchronized static native int getPlayerState_bobCycle(int clientNum);

	private synchronized static native void setPlayerState_bobCycle(int clientNum, int bobCycle);

	private synchronized static native Vector3f getPlayerState_origin(int clientNum);

	private synchronized static native void setPlayerState_origin(int clientNum, float x, float y, float z);

	private synchronized static native Vector3f getPlayerState_velocity(int clientNum);

	private synchronized static native void setPlayerState_velocity(int clientNum, float x, float y, float z);

	private synchronized static native int getPlayerState_weaponTime(int clientNum);

	private synchronized static native void setPlayerState_weaponTime(int clientNum, int weaponTime);

	private synchronized static native int getPlayerState_gravity(int clientNum);

	private synchronized static native void setPlayerState_gravity(int clientNum, int gravity);

	private synchronized static native int getPlayerState_speed(int clientNum);

	private synchronized static native void setPlayerState_speed(int clientNum, int speed);
	
	private synchronized static native Vector3f getPlayerState_deltaAngles(int clientNum);
	
	private synchronized static native void setPlayerState_deltaAngles(int clientNum, int pitch, int yaw, int roll);

	private synchronized static native short getPlayerState_deltaPitch(int clientNum);

	private synchronized static native void setPlayerState_deltaPitch(int clientNum, short deltaPitch);

	private synchronized static native short getPlayerState_deltaYaw(int clientNum);

	private synchronized static native void setPlayerState_deltaYaw(int clientNum, short deltaYaw);

	private synchronized static native short getPlayerState_deltaRoll(int clientNum);

	private synchronized static native void setPlayerState_deltaRoll(int clientNum, short deltaRoll);

	private synchronized static native int getPlayerState_groundEntityNum(int clientNum);

	private synchronized static native void setPlayerState_groundEntityNum(int clientNum, int groundEntityNum);

	private synchronized static native int getPlayerState_legsTimer(int clientNum);

	private synchronized static native void setPlayerState_legsTimer(int clientNum, int legsTimer);

	private synchronized static native int getPlayerState_legsAnim(int clientNum);

	private synchronized static native void setPlayerState_legsAnim(int clientNum, int legsAnim);

	private synchronized static native int getPlayerState_torsoTimer(int clientNum);

	private synchronized static native void setPlayerState_torsoTimer(int clientNum, int torsoTimer);

	private synchronized static native int getPlayerState_torsoAnim(int clientNum);

	private synchronized static native void setPlayerState_torsoAnim(int clientNum, int torsoAnim);

	private synchronized static native int getPlayerState_movementDir(int clientNum);

	private synchronized static native void setPlayerState_movementDir(int clientNum, int movementDir);

	private synchronized static native Vector3f getPlayerState_grapplePoint(int clientNum);

	private synchronized static native void setPlayerState_grapplePoint(int clientNum, Vector3f grapplePoint);

	private synchronized static native int getPlayerState_eFlags(int clientNum);

	private synchronized static native void setPlayerState_eFlags(int clientNum, int flags);

	private synchronized static native int getPlayerState_eventSequence(int clientNum);

	private synchronized static native void setPlayerState_eventSequence(int clientNum, int eventSequence);

	private synchronized static native int getPlayerState_externalEvent(int clientNum);

	private synchronized static native void setPlayerState_externalEvent(int clientNum, int externalEvent);

	private synchronized static native int getPlayerState_externalEventParm(int clientNum);

	private synchronized static native void setPlayerState_externalEventParm(int clientNum, int externalEventParm);

	private synchronized static native int getPlayerState_externalEventTime(int clientNum);

	private synchronized static native void setPlayerState_externalEventTime(int clientNum, int externalEventTime);

//	private synchronized static native int getPlayerState_clientNum(int clientNum);

//	private synchronized static native void setPlayerState_clientNum(int clientNum, int clientNum);

	private synchronized static native int getPlayerState_weapon(int clientNum);

	private synchronized static native void setPlayerState_weapon(int clientNum, int weapon);

	private synchronized static native int getPlayerState_weaponState(int clientNum);

	private synchronized static native void setPlayerState_weaponState(int clientNum, int weaponState);

	private synchronized static native Angle3f getPlayerState_viewAngles(int clientNum);
	
	private synchronized static native void setPlayerState_viewAngles(int clientNum, float pitch, float yaw, float roll);

	private synchronized static native int getPlayerState_viewHeight(int clientNum);

	private synchronized static native void setPlayerState_viewHeight(int clientNum, int viewHeight);

	private synchronized static native int getPlayerState_damageEvent(int clientNum);

	private synchronized static native void setPlayerState_damageEvent(int clientNum, int damageEvent);

	private synchronized static native int getPlayerState_damageYaw(int clientNum);

	private synchronized static native void setPlayerState_damageYaw(int clientNum, int damageYaw);

	private synchronized static native int getPlayerState_damagePitch(int clientNum);

	private synchronized static native void setPlayerState_damagePitch(int clientNum, int damagePitch);

	private synchronized static native int getPlayerState_damageCount(int clientNum);

	private synchronized static native void setPlayerState_damageCount(int clientNum, int damageCount);

	private synchronized static native int getPlayerState_generic1(int clientNum);

	private synchronized static native void setPlayerState_generic1(int clientNum, int generic1);

	private synchronized static native int getPlayerState_loopSound(int clientNum);

	private synchronized static native void setPlayerState_loopSound(int clientNum, int loopSound);

	private synchronized static native int getPlayerState_jumppad_ent(int clientNum);

	private synchronized static native void setPlayerState_jumppad_ent(int clientNum, int jumppad_ent);

	private synchronized static native int getPlayerState_ping(int clientNum);

	private synchronized static native void setPlayerState_ping(int clientNum, int ping);
	
	@Override
	public int getPlayerState_bobCycle() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_clientNum() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_commandTime() {
		return getPlayerState_commandTime(getEntityState_number());
	}
	@Override
	public int getPlayerState_damageCount() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_damageEvent() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_damagePitch() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_damageYaw() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public Vector3f getPlayerState_deltaAngles() {
		return getPlayerState_deltaAngles(getEntityState_number());
	}
	
	@Override
	public short getPlayerState_deltaPitch() {
		return getPlayerState_deltaPitch(getEntityState_number());
	}
	
	@Override
	public short getPlayerState_deltaRoll() {
		return getPlayerState_deltaRoll(getEntityState_number());
	}
	
	@Override
	public short getPlayerState_deltaYaw() {
		return getPlayerState_deltaYaw(getEntityState_number());
	}
	
	@Override
	public int getPlayerState_eFlags() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_eventSequence() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_externalEvent() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_externalEventParm() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_externalEventTime() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_generic1() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public Vector3f getPlayerState_grapplePoint() {
		// TODO Auto-generated method stub
		return null;
	}
	@Override
	public int getPlayerState_gravity() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_groundEntityNum() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_jumppad_ent() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_legsAnim() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_legsTimer() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_loopSound() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_movementDir() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public Vector3f getPlayerState_origin() {
		return getPlayerState_origin(getEntityState_number());
	}
	
	@Override
	public int getPlayerState_ping() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public int getPlayerState_pm_flags() {
		return getPlayerState_pm_flags(getEntityState_number());
	}
	
	@Override
	public int getPlayerState_pm_time() {
		return getPlayerState_pm_time(getEntityState_number());
	}
	
	@Override
	public PlayerMovementType getPlayerState_pm_type() {
		return PlayerMovementType.values()[getPlayerState_pm_type(getEntityState_number())];
	}
	
	@Override
	public int getPlayerState_speed() {
		return getPlayerState_speed(getEntityState_number());
	}
	@Override
	public int getPlayerState_torsoAnim() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_torsoTimer() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public Vector3f getPlayerState_velocity() {
		return getPlayerState_velocity(getEntityState_number());
	}
	
	@Override
	public Angle3f getPlayerState_viewAngles() {
		return getPlayerState_viewAngles(getEntityState_number());
	}
	@Override
	public int getPlayerState_viewHeight() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_weapon() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_weaponState() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_weaponTime() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public void setPlayerState_bobCycle(int bobCycle) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_clientNum(int clientNum) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void setPlayerState_commandTime(int time) {
		setPlayerState_commandTime(getEntityState_number(), time);
	}
	
	@Override
	public void setPlayerState_damageCount(int damageCount) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_damageEvent(int damageEvent) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_damagePitch(int damagePitch) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_damageYaw(int damageYaw) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void setPlayerState_deltaPitch(short deltaPitch) {
		 setPlayerState_deltaPitch(getEntityState_number(), deltaPitch);
	}
	
	@Override
	public void setPlayerState_deltaRoll(short deltaRoll) {
		setPlayerState_deltaRoll(getEntityState_number(), deltaRoll);
	}
	
	@Override
	public void setPlayerState_deltaYaw(short deltaYaw) {
		setPlayerState_deltaYaw(getEntityState_number(), deltaYaw);	
	}
	
	@Override
	public void setPlayerState_eFlags(int flags) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_eventSequence(int eventSequence) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_externalEvent(int externalEvent) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_externalEventParm(int externalEventParm) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_externalEventTime(int externalEventTime) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_generic1(int generic1) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_grapplePoint(Vector3f grapplePoint) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_gravity(int gravity) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_groundEntityNum(int groundEntityNum) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_jumppad_ent(int jumppad_ent) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_legsAnim(int legsAnim) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_legsTimer(int legsTimer) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_loopSound(int loopSound) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_movementDir(int movementDir) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void setPlayerState_origin(Vector3f origin) {
		setPlayerState_origin(getEntityState_number(), origin.x, origin.y, origin.z);	
	}
	
	@Override
	public void setPlayerState_ping(int ping) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void setPlayerState_pm_flags(int flags) {
		setPlayerState_pm_flags(getEntityState_number(), flags);
	}

	@Override
	public void setPlayerState_pm_time(int time) {
		setPlayerState_pm_time(getEntityState_number(), time);
	}

	@Override
	public void setPlayerState_pm_type(PlayerMovementType type) {
		setPlayerState_pm_type(getEntityState_number(), type.ordinal());
	}
	
	@Override
	public void setPlayerState_speed(int speed) {
		setPlayerState_speed(getEntityState_number(), speed);	
	}
	
	@Override
	public void setPlayerState_torsoAnim(int torsoAnim) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_torsoTimer(int torsoTimer) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void setPlayerState_velocity(Vector3f velocity) {
		setPlayerState_velocity(getEntityState_number(), velocity.x, velocity.y, velocity.z);	
	}
	
	@Override
	public void setPlayerState_viewAngles(Angle3f viewAngles) {
		setPlayerState_viewAngles(getEntityState_number(), viewAngles.x, viewAngles.y, viewAngles.z);	
	}
	
	@Override
	public void setPlayerState_viewAngles(float pitch, float yaw, float roll) {
		setPlayerState_viewAngles(getEntityState_number(), pitch, yaw, roll);
	}
	
	@Override
	public void setPlayerState_viewHeight(int viewHeight) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_weapon(int weapon) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_weaponState(int weaponState) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_weaponTime(int weaponTime) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void addPlayerState_pm_flags(int pm_flags) {
		setPlayerState_pm_flags(getPlayerState_pm_flags() | pm_flags);
	}
	
	@Override
	public void delPlayerState_pm_flags(int pm_flags) {
		setPlayerState_pm_flags(getPlayerState_pm_flags() & ~pm_flags);
	}
	
	@Override
	public boolean hasPlayerState_pm_flags(int pm_flags) {
		return (getPlayerState_pm_flags() & pm_flags) > 0;
	}
	
	// --------------------------------------------------------------------------------------------
}
