package xreal.server.game;

import xreal.Engine;
import xreal.common.Config;
import xreal.common.ConfigStrings;
import xreal.server.Server;

public class Game implements GameListener {
	
	static private int levelTime;

	private Game() {
		
	}

	@Override
	public String clientConnect(Player client, boolean firstTime, boolean isBot) {
		Engine.print("xreal.server.game.Game.clientConnect(clientNum = " + client.getEntityIndex() + ", firstTime = " + firstTime + ", isBot = " + isBot + ")\n");
		
		//return "Game.clientConnect() is not implemented yet.";	// deny message
		return null;
	}

	@Override
	public boolean consoleCommand() {
		Engine.print("xreal.server.game.Game.consoleCommand()\n");
		return false;
	}

	@Override
	public void initGame(int levelTime, int randomSeed, boolean restart) {
		
		Engine.print("xreal.server.game.Game.initGame(levelTime = "+ levelTime + ", randomSeed = " + randomSeed + ", restart = " + restart + ")\n");
		
		Engine.print("------- Game Initialization -------\n");
		//Engine.println("gamename: "Config.GAME_VERSION);
		//Engine.print("gamedate: %s\n", __DATE__);

		//Engine.sendConsoleCommand(Engine.EXEC_APPEND, "echo cool!");
		
		this.levelTime = levelTime;
		
		// make some data visible to connecting client
		Server.setConfigString(ConfigStrings.GAME_VERSION, Config.GAME_VERSION);
		Server.setConfigString(ConfigStrings.LEVEL_START_TIME, Integer.toString(levelTime));
		Server.setConfigString(ConfigStrings.MOTD, CVars.g_motd.getString());
		
		Engine.println("Game Version: " + Server.getConfigString(ConfigStrings.GAME_VERSION));
		
		
//		for(int i = 0; i < 30; i++)
//		{
//			GameEntity e1 = new GameEntity();
//			
//			/*
//			Engine.println(e1.toString());
//			try {
//				e1.finalize();
//			} catch (Throwable e) {
//				// TODO Auto-generated catch block
//				e.printStackTrace();
//			}
//			*/
//		}
		
		
		Engine.print("-----------------------------------\n");
	}

	@Override
	public void runAIFrame(int time) {
		//Engine.print("xreal.server.game.Game.runAIFrame(time = " + time + ")\n");

	}

	@Override
	public void runFrame(int time) {
		//Engine.print("xreal.server.game.Game.runFrame(time = " + time + ")\n");
		
		this.levelTime = time;
		
		//CVars.g_gametype.set("99");
		//CVars.g_gametype = null;
		//Engine.print(CVars.g_gametype.toString() + "\n");
		
		//System.gc();
		
		//Engine.print("xreal.server.game.Game.runFrame(time2 = " + Engine.getTimeInMilliseconds() + ")\n");
	}

	@Override
	public void shutdownGame(boolean restart) {
		Engine.print("xreal.server.game.Game.shutdownGame(restart = " + restart + ")\n");
	}
	
	static public int getLevelTime() {
		return levelTime;
	}
}
