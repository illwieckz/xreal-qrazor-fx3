/**
 * 
 */
package xreal.server.game;

import java.util.Hashtable;
import java.util.StringTokenizer;

import xreal.ConsoleColorStrings;
import xreal.Engine;
import xreal.UserCommand;
import xreal.common.ConfigStrings;
import xreal.common.Team;
import xreal.server.Server;

/**
 * Represents, uses and writes to a native gclient_t
 * 
 * @author Robert Beckebans
 */
public class Player extends GameEntity implements ClientListener {
	
	protected Hashtable<String, String> _userInfo = new Hashtable<String, String>();
	
	private ClientPersistant	_pers = new ClientPersistant();
	private ClientSession		_sess = new ClientSession();
	private int					_lastCmdTime; 
	
	private static native String	getUserInfo0(int clientNum);
	private static native void 		setUserInfo0(int clientNum, String s);
	
	Player(int clientNum, boolean firstTime, boolean isBot) throws GameException
	{
		super(clientNum);
		
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
		if(G_FilterPacket(value))
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
		Engine.print("xreal.server.game.Player.clientThink(clientNum = " + getEntityIndex() + ")\n");
		
		Engine.println(ucmd.toString());
		
		// mark the time we got info, so we can display the
		// phone jack if they don't get any for a while
		_lastCmdTime = Game.getLevelTime();
		
		if(CVars.g_synchronousClients.getBoolean())
		{
			//ClientThink_real(ent);
		}
	}

	/**
	 * Called from ClientConnect when the player first connects and
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
		
		if(userinfo == null)
			return;

		StringTokenizer st = new StringTokenizer(userinfo, "\\");
		while (st.hasMoreTokens())
		{
			String key = st.nextToken();
			if (st.hasMoreTokens()) 
			{
				String val = st.nextToken();
				String oldVal = (String) _userInfo.get(key);
				
				if ((oldVal == null) || (!val.equals(oldVal))) 
				{
					_userInfo.put(key, val);
					
					userInfoVariableChanged(key, oldVal, val);
				}
			}
		}
		
		Engine.println("Player.userinfo = " + _userInfo.toString());

		/*
		Com_sprintf(userinfo, sizeof(userinfo),
				"n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\g_redteam\\%s\\g_blueteam\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d",
				client->pers.netname, team, model, "", redTeam, blueTeam, c1, c2, client->pers.maxHealth,
				client->sess.wins, client->sess.losses, teamTask, teamLeader);
				*/
		
		Server.setConfigString(ConfigStrings.PLAYERS + getEntityIndex(), userinfo);
	}
	
	private void userInfoVariableChanged(String key, String oldValue, String value) {
		
		// check for local client
		if (key.equals("ip")) {
			if (value.equals("localhost"))
				_pers.localClient = true;
		}

		// set name
		if (key.equals("name"))
		{
			String oldname = _pers.netname;
			
			// TODO _pers.netname = ClientCleanName(value);
			_pers.netname = value;
		

		if(_sess.sessionTeam == Team.SPECTATOR)
		{
			if(_sess.spectatorState == SpectatorState.SCOREBOARD)
			{
				_pers.netname = "scoreboard";
			}
		}

		if(_pers.connected == ClientConnectionState.CONNECTED)
		{
			if(!_pers.netname.equals(oldname))
			{
				//trap_SendServerCommand(-1, va("print \"%s" + ConsoleColorStrings.BLUE + " renamed to %s\n\"", oldname, _pers.netname));
				Server.broadcastServerCommand("print \"" + oldname + ConsoleColorStrings.BLUE + " renamed to " + _pers.netname + "\n\"");
			}
		}
		}
	}
}
