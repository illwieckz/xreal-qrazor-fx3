package xreal.game;

import xreal.Engine;

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

	}

	@Override
	public void runAIFrame(int time) {
		//Engine.print("xreal.game.Game.runAIFrame(time = " + time + ")\n");

	}

	@Override
	public void runFrame(int time) {
		//Engine.print("xreal.game.Game.runFrame(time = " + time + ")\n");
		
		//CVars.g_gametype.set("99");
		//Engine.print(CVars.g_gametype.toString() + "\n");
	}

	@Override
	public void shutdownGame(boolean restart) {
		Engine.print("xreal.game.Game.shutdownGame(restart = " + restart + ")\n");
	}
}
