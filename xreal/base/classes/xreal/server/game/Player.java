package xreal.server.game;

import xreal.ConsoleColorStrings;
import xreal.Engine;
import xreal.UserCommand;
import xreal.UserInfo;
import xreal.common.ConfigStrings;
import xreal.common.GameType;
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
	private int					_lastCmdTime; 
	
	private static native String	getUserInfo0(int clientNum);
	private static native void 		setUserInfo0(int clientNum, String s);
	
	
	
	
	// --------------------------------------------------------------------------------------------
	
	
	Player(int clientNum, boolean firstTime, boolean isBot) throws GameException
	{
		super(clientNum);
		
		_sess.sessionTeam = Team.SPECTATOR;
		
		String userinfo = getUserInfo0(clientNum);
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
		
		//throw new GameException("Connection refused");
	}

	@Override
	public void clientBegin() {
		Engine.print("xreal.server.game.Player.clientBegin(clientNum = " + getEntityIndex() + ")\n");
		
		_pers.connected = ClientConnectionState.CONNECTED;
		_pers.enterTime = Game.getLevelTime();
		
		//_pers.teamState.state = Team.BEGIN;

	}

	@Override
	public void clientCommand() {
		Engine.print("xreal.server.game.Player.clientCommand(clientNum = " + getEntityIndex() + ")\n");
	}
	
	@Override
	public void clientDisconnect() {
		Engine.print("xreal.server.game.Player.clientDisconnect(clientNum = " + getEntityIndex() + ")\n");
	}

	@Override
	public void clientThink(UserCommand ucmd) {
		//Engine.print("xreal.server.game.Player.clientThink(clientNum = " + getEntityIndex() + ")\n");
		
		//Engine.println(ucmd.toString());
		
		// mark the time we got info, so we can display the
		// phone jack if they don't get any for a while
		_lastCmdTime = Game.getLevelTime();
		
		if(CVars.g_synchronousClients.getBoolean())
		{
			//ClientThink_real(ent);
			
			// shut up client about outdated player states
			setPlayerState_commandTime(ucmd.serverTime);
			
			setPlayerState_pm_type(PlayerMovementType.SPECTATOR);
		}
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
		Engine.print("xreal.server.game.Player.clientUserInfoChanged(clientNum = " + getEntityIndex() + ")\n");

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
				Server.broadcastServerCommand("print \"" + oldname + ConsoleColorStrings.WHITE + " renamed to " + _pers.netname + "\n\"");
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
		
		Server.setConfigString(ConfigStrings.PLAYERS + getEntityIndex(), uinfo.toString());
		
	}
	
	// ------------------- playerState_t:: fields in gclient_t::ps --------------------------------
	
	private static native void setPlayerState_commandTime(int clientNum, int time);

	private static native void setPlayerState_pm_type(int clientNum, int type);

	private static native void setPlayerState_pm_flags(int clientNum, int flags);

	private static native void setPlayerState_pm_time(int clientNum, int time);

	public void setPlayerState_commandTime(int time) {
		setPlayerState_commandTime(getEntityIndex(), time);
	}

	@Override
	public void setPlayerState_pm_flags(int flags) {
		setPlayerState_pm_flags(getEntityIndex(), flags);

	}

	@Override
	public void setPlayerState_pm_time(int time) {
		setPlayerState_pm_time(getEntityIndex(), time);
	}

	@Override
	public void setPlayerState_pm_type(PlayerMovementType type) {
		setPlayerState_pm_type(getEntityIndex(), type.ordinal());
	}
	
	// --------------------------------------------------------------------------------------------
}
