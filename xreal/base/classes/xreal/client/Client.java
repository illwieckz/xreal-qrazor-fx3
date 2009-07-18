package xreal.client;

/**
 * Represents Client functionality. Never use this code for the server game.
 * 
 * @author Robert Beckebans
 */
public class Client {
	
	/**
	 * Get the server entry for a config string specified by ConfigStrings.*
	 * 
	 * @return If the index is < 0 or > MAX_CONFIGSTRINGS it will return null. 
	 */
	public synchronized native static String getConfigString(int index);

}
