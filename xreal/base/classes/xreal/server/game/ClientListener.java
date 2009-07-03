package xreal.server.game;

/**
 * @author Robert Beckebans
 */
public interface ClientListener {
	
	public void			clientBegin();
	public void			clientUserInfoChanged(String s);
	public void			clientDisconnect();
	public void			clientCommand();
	public void			clientThink();
}
