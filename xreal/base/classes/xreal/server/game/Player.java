/**
 * 
 */
package xreal.server.game;

import xreal.Engine;

/**
 * @author Robert Beckebans
 */
public class Player extends GameEntity implements ClientListener {
	
	Player(int clientNum)
	{
		super(clientNum);
	}

	@Override
	public void clientBegin(int clientNum) {
		Engine.print("xreal.server.game.Player.clientBegin(clientNum = " + clientNum + ")\n");

	}

	@Override
	public void clientCommand(int clientNum) {
		Engine.print("xreal.server.game.Player.clientCommand(clientNum = " + clientNum + ")\n");
	}
	
	@Override
	public void clientDisconnect(int clientNum) {
		Engine.print("xreal.server.game.Player.clientDisconnect(clientNum = " + clientNum + ")\n");
	}

	@Override
	public void clientThink(int clientNum) {
		Engine.print("xreal.server.game.Player.clientThink(clientNum = " + clientNum + ")\n");
	}

	@Override
	public void clientUserInfoChanged(int clientNum) {
		Engine.print("xreal.server.game.Player.clientUserInfoChanged(clientNum = " + clientNum + ")\n");
	}

}
