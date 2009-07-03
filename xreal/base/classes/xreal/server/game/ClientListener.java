package xreal.server.game;

/**
 * @author Robert Beckebans
 */
public interface ClientListener {
	
	public void			clientBegin(int clientNum);
	public void			clientUserInfoChanged(int clientNum);
	public void			clientDisconnect(int clientNum);
	public void			clientCommand(int clientNum);
	public void			clientThink(int clientNum);
}
