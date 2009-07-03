package xreal.server.game;

/**
 * 
 * @author Robert Beckebans
 *
 */
public interface GameListener
{
	public void			initGame(int levelTime, int randomSeed, boolean restart);
	
	public void			shutdownGame(boolean restart);
	
	/**
	 * @return NULL if the client is allowed to connect, otherwise return a text string with the reason for denial. 
	 */
	public String		clientConnect(Player client, boolean firstTime, boolean isBot);
	public void			runFrame(int time);
	public void			runAIFrame(int time);
	
	/**
	 *  ConsoleCommand will be called when a command has been issued
	 *  that is not recognized as a builtin function.
	 *  The game can issue Engine.argc() / Engine.argv() commands to get the command
	 *  and parameters.
	 *  
	 *  @return Return false if the game doesn't recognize it as a command.
	 */ 
	public boolean		consoleCommand();
}
