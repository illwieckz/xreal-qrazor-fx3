package xreal.client;

/**
 * channel 0 never willingly overrides
 * other channels will always override a playing sound on that channel
 * 
 * @author Robert Beckebans
 */
public enum SoundChannel {
	AUTO,
	/**
	 * menu sounds, etc
	 */
	LOCAL, 
	WEAPON,
	VOICE,
	ITEM,
	BODY,
	
	/**
	 * chat messages, etc
	 */
	LOCAL_SOUND,
	
	/**
	 * announcer voices, etc
	 */
	ANNOUNCER
}
