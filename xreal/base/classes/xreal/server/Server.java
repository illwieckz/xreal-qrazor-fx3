package xreal.server;

/**
 * Represents Server functionality. Never use this code for the client game or the client ui
 * 
 * @author Robert Beckebans
 */
public class Server
{
	/**
	 * Get the server entry for a config string specified by ConfigStrings.*
	 * 
	 * Same as SV_GetConfigstring.
	 */
	public synchronized native static String getConfigString(int index);

	
	/**
	 * Set the server entry for a config string specified by ConfigStrings.*
	 * 
	 * Same as SV_SetConfigstring.
	 */
	public synchronized native static void setConfigString(int index, String string);

	/*
	public synchronized native static String getUserinfo(int num);

	public synchronized native static String setUserinfo(int num, String buffer);

	public synchronized native static String getServerinfo();
	*/
}