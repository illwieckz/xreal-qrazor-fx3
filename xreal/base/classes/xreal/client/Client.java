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
	
	/**
	 * Get all current key catchers defined in KeyCatchers
	 */
	public synchronized native static int getKeyCatchers();
	
	/**
	 * Set all current key catchers. Be careful when using this method.
	 */
	public synchronized native static void setKeyCatchers(int catchers);
	
	private synchronized native static String getKeyBinding(int keynum);
	
	/**
	 * Returns the text that was associated to this key with the console command "bind <button> <binding>" 
	 */
	public static String getKeyBinding(KeyCode key) {
		return getKeyBinding(key.getCode());
	}
	
	private synchronized native static void setKeyBinding(int keynum, String binding);
	
	public static void setKeyBinding(KeyCode key, String binding) {
		setKeyBinding(key.getCode(), binding);
	}
	
	private synchronized native static boolean isKeyDown(int keynum);
	
	public static boolean isKeyDown(KeyCode key) {
		return isKeyDown(key.getCode());
	}
	
	/**
	 * Issue all remaining key events and reset each key state.
	 */
	public synchronized native static void clearKeyStates();
}
