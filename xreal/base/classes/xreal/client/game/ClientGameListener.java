package xreal.client.game;


/**
 * 
 * @author Robert Beckebans
 *
 */
public interface ClientGameListener
{
	public void			initClientGame(int serverMessageNum, int serverCommandSequence, int clientNum) throws Exception;
	
	public void			shutdownClientGame();
	
	/**
	 *  ConsoleCommand will be called when a command has been issued
	 *  that is not recognized as a builtin function.
	 *  The game can issue Engine.argc() / Engine.argv() commands to get the command
	 *  and parameters.
	 *  
	 *  @return Return false if the game doesn't recognize it as a command.
	 */ 
	public boolean		consoleCommand();
	
	/**
	 * Generates and draws a game scene and status information at the given time.
	 * 
	 * @param serverTime
	 * @param stereoView
	 * @param demoPlayback	If demoPlayback is set, local movement prediction will not be enabled
	 * @throws Exception 
	 */
	public void			drawActiveFrame(int serverTime, int stereoView, boolean demoPlayback) throws Exception;
	
	
	public int			crosshairPlayer();

	public int			lastAttacker();

	public void			keyEvent(int time, int key, boolean down);

	public void			mouseEvent(int time, int dx, int dy);

	public void			eventHandling(int type);
}
