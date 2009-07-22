package xreal.client;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;

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
	
	
	// keyboard and mouse event handling ----------------------------------------------------------
	
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
	
	
	
	// sound handling -----------------------------------------------------------------------------
	
	/**
	 * Load a sound into the sound engine, if it's not already loaded.
	 * 
	 * @return A handle of type sfxHandle_t (int). Returns buzz if not found.
	 */
	public synchronized native static int registerSound(String fileName);
	
	
	/**
	 * Play a sound at the given location.
	 * 
	 * Normal sounds will have their volume dynamically changed as their entity moves and the listener moves.
	 * 
	 * @param posX
	 * @param posY
	 * @param posZ
	 * @param entityNum
	 * @param entityChannel
	 * @param sfxHandle
	 */
	public synchronized native static void startSound(float posX, float posY, float posZ, int entityNum, int entityChannel, int sfxHandle);
	
	public static void startSound(Vector3f pos, int entityNum, int entityChannel, int sfxHandle) {
		startSound(pos.x, pos.y, pos.z, entityNum, entityChannel, sfxHandle);
	}
	
	/**
	 * Play a sound without distance attenuation. A local sound is always played full volume.
	 * 
	 * @param sfx
	 * @param channelNum
	 */
	public synchronized native static void startLocalSound(int sfxHandle, int channelNum);
	
	public synchronized native static void addLoopingSound(int entityNum, float posX, float posY, float posZ, float velX, float velY, float velZ, int sfxHandle);
	
	public static void addLoopingSound(int entityNum, Vector3f position, Vector3f velocity, int sfxHandle) {
		addLoopingSound(entityNum, position.x, position.y, position.z, velocity.x, velocity.y, velocity.z, sfxHandle);
	}
	
	public synchronized native static void addRealLoopingSound(int entityNum, float posX, float posY, float posZ, float velX, float velY, float velZ, int sfxHandle);
	
	public static void addRealLoopingSound(int entityNum, Vector3f position, Vector3f velocity, int sfxHandle) {
		addRealLoopingSound(entityNum, position.x, position.y, position.z, velocity.x, velocity.y, velocity.z, sfxHandle);
	}
	
	/**
	 * Let the sound system know where an entity currently is.
	 * 
	 * @param entityNum
	 */
	public synchronized native static void updateEntitySoundPosition(int entityNum, float posX, float posY, float posZ);
	
	public static void updateEntitySoundPosition(int entityNum, Vector3f position) {
		updateEntitySoundPosition(entityNum, position.x, position.y, position.z);
	}
	
	public synchronized native static void stopLoopingSound(int entityNum);
	
	public synchronized native static void clearLoopingSounds(boolean killall);

	/**
	 * Recalculate the volumes of sound as they should be heard by the given entityNum and position.
	 * 
	 * @param entityNum
	 */
	public synchronized native static void respatialize(int entityNum, float posX, float posY, float posZ, float quatX, float quatY, float quatZ, float quatW, boolean inWater);
	
	public static void respatialize(int entityNum, Vector3f position, Quat4f orientation, boolean inWater) {
		respatialize(entityNum, position.x, position.y, position.z, orientation.x, orientation.y, orientation.z, orientation.w, inWater);
	}
	
	
	/**
	 * Start a background track.
	 * Empty name stops music.
	 * 
	 * @param intro
	 * @param loop
	 */
	public synchronized native static void startBackgroundTrack(String intro, String loop);
	public synchronized native static void stopBackgroundTrack();
	
	
}
