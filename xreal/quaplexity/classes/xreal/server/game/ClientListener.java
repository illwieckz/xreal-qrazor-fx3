package xreal.server.game;

import xreal.UserCommand;

/**
 * @author Robert Beckebans
 */
public interface ClientListener {
	
	public void			clientBegin();
	public void			clientUserInfoChanged(String s);
	public void			clientDisconnect();
	public void			clientCommand();
	public void			clientThink(UserCommand ucmd);
}
