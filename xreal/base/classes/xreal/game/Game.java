package xreal.game;

import xreal.Engine;

public class Game implements GameListener {
	
	private Game() {
		
	}

	@Override
	public void clientBegin(int clientNum) {
		// TODO Auto-generated method stub
		Engine.print("xreal.game.Game.clientBegin()\n");

	}

	@Override
	public void clientCommand(int clientNum) {
		// TODO Auto-generated method stub
		Engine.print("xreal.game.Game.clientCommand()\n");
	}

	@Override
	public String clientConnect(int clientNum, boolean firstTime, boolean isBot) {
		Engine.print("xreal.game.Game.clientConnect()\n");
		
		return "Game.clientConnect() is not implemented yet.";	// deny message
		//return null;
	}

	@Override
	public void clientDisconnect(int clientNum) {
		// TODO Auto-generated method stub
		Engine.print("xreal.game.Game.clientDisconnect()\n");
	}

	@Override
	public void clientThink(int clientNum) {
		// TODO Auto-generated method stub
		Engine.print("xreal.game.Game.clientThink()\n");
	}

	@Override
	public void clientUserInfoChanged(int clientNum) {
		// TODO Auto-generated method stub
		Engine.print("xreal.game.Game.clientUserInfoChanged()\n");
	}

	@Override
	public boolean consoleCommand() {
		// TODO Auto-generated method stub
		Engine.print("xreal.game.Game.consoleCommand()\n");
		return false;
	}

	@Override
	public void initGame(int levelTime, int randomSeed, boolean restart) {
		
		Engine.print("------- Game Initialization -------\n");
		Engine.print("Successfully called xreal.game.Game.initGame() from Java!\n");
		//Engine.print("gamename: %s\n", GAMEVERSION);
		//Engine.print("gamedate: %s\n", __DATE__);

	}

	@Override
	public void runAIFrame(int time) {
		// TODO Auto-generated method stub
		Engine.print("xreal.game.Game.runAIFrame()\n");

	}

	@Override
	public void runFrame(int time) {
		// TODO Auto-generated method stub
		Engine.print("xreal.game.Game.runFrame()\n");

	}

	@Override
	public void shutdownGame(boolean restart) {
		// TODO Auto-generated method stub
		Engine.print("xreal.game.Game.shutdownGame()\n");
	}

}
