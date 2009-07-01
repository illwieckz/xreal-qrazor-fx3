package xreal.server.game;

import xreal.Engine;
import xreal.common.Config;
import xreal.common.ConfigStrings;
import xreal.server.Server;

public class Game implements GameListener {
	
	private Game() {
		
	}

	@Override
	public void clientBegin(int clientNum) {
		Engine.print("xreal.game.Game.clientBegin(clientNum = " + clientNum + ")\n");

	}

	@Override
	public void clientCommand(int clientNum) {
		Engine.print("xreal.game.Game.clientCommand(clientNum = " + clientNum + ")\n");
	}

	@Override
	public String clientConnect(int clientNum, boolean firstTime, boolean isBot) {
		Engine.print("xreal.game.Game.clientConnect(clientNum = " + clientNum + ", firstTime = " + firstTime + ", isBot = " + isBot + ")\n");
		
		//return "Game.clientConnect() is not implemented yet.";	// deny message
		return null;
	}

	@Override
	public void clientDisconnect(int clientNum) {
		Engine.print("xreal.game.Game.clientDisconnect(clientNum = " + clientNum + ")\n");
	}

	@Override
	public void clientThink(int clientNum) {
		Engine.print("xreal.game.Game.clientThink(clientNum = " + clientNum + ")\n");
	}

	@Override
	public void clientUserInfoChanged(int clientNum) {
		Engine.print("xreal.game.Game.clientUserInfoChanged(clientNum = " + clientNum + ")\n");
	}

	@Override
	public boolean consoleCommand() {
		Engine.print("xreal.game.Game.consoleCommand()\n");
		return false;
	}

	@Override
	public void initGame(int levelTime, int randomSeed, boolean restart) {
		
		Engine.print("xreal.game.Game.initGame(levelTime = "+ levelTime + ", randomSeed = " + randomSeed + ", restart = " + restart + ")\n");
		
		Engine.print("------- Game Initialization -------\n");
		//Engine.print("gamename: %s\n", GAMEVERSION);
		//Engine.print("gamedate: %s\n", __DATE__);

		// make some data visible to connecting client
		//Server.setConfigstring(ConfigStrings.GAME_VERSION, Config.GAME_VERSION);

		/*
		trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime));

		G_SpawnString("music", "", &s);
		trap_SetConfigstring(CS_MUSIC, s);

		G_SpawnString("message", "", &s);
		trap_SetConfigstring(CS_MESSAGE, s);	// map specific message

		trap_SetConfigstring(CS_MOTD, g_motd.string);	// message of the day
		*/
	}

	@Override
	public void runAIFrame(int time) {
		//Engine.print("xreal.game.Game.runAIFrame(time = " + time + ")\n");

	}

	@Override
	public void runFrame(int time) {
		//Engine.print("xreal.game.Game.runFrame(time = " + time + ")\n");
		
		//CVars.g_gametype.set("99");
		//CVars.g_gametype = null;
		//Engine.print(CVars.g_gametype.toString() + "\n");
	}

	@Override
	public void shutdownGame(boolean restart) {
		Engine.print("xreal.game.Game.shutdownGame(restart = " + restart + ")\n");
	}
}
