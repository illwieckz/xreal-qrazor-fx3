/**
 * 
 */
package xreal.server.game;

import java.util.Hashtable;
import java.util.StringTokenizer;

import xreal.Engine;
import xreal.UserCommand;
import xreal.common.ConfigStrings;
import xreal.server.Server;

/**
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
		
		userInfoChanged(userinfo);
		
		//throw new GameException("Connection refused");
	}

	@Override
	public void clientBegin() {
		Engine.print("xreal.server.game.Player.clientBegin(clientNum = " + getEntityIndex() + ")\n");

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

	@Override
	public void clientUserInfoChanged(String userinfo) {
		Engine.print("xreal.server.game.Player.clientUserInfoChanged(clientNum = " + getEntityIndex() + ")\n");
		
		userInfoChanged(userinfo);
	}

	/**
	 * Called from ClientConnect when the player first connects and
	 * directly by the server system when the player updates a userinfo variable.
	 * 
	 * @param userinfo
	 *            the userinfo string, formatted as:
	 *            "\keyword\value\keyword\value\....\keyword\value"
	 */
	public void userInfoChanged(String userinfo) {
		
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
					//playerVariableChanged(key, oldVal, val);
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
}
