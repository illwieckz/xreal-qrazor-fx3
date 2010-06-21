package xreal.client.game;

import xreal.Engine;


/**
 * Main class of the client game that is loaded by the engine.
 * 
 * @author	Robert Beckebans
 * @since	21.06.2010
 */
public class ClientGame implements ClientGameListener {

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
	public void drawActiveFrame(int serverTime, int stereoView, boolean demoPlayback) {
		
		//Engine.print("xreal.client.game.ClientGame.drawActiveFrame(serverTime = "+ serverTime + ", stereoView = " + stereoView + ", demoPlayback = " + demoPlayback + ")\n");
		
		// TODO Auto-generated method stub

	}

	@Override
	public void eventHandling(int type) {
		
		Engine.println("xreal.client.game.ClientGame.eventHandling(type = " + type + ")");
		
		// TODO Auto-generated method stub
	}

	@Override
	public void initClientGame(int serverMessageNum, int serverCommandSequence, int clientNum) {
		
		Engine.print("xreal.client.game.ClientGame.initClientGame(serverMessageNum = "+ serverMessageNum + ", serverCommandSequence = " + serverCommandSequence + ", clientNum = " + clientNum  + ")\n");
		
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

}
