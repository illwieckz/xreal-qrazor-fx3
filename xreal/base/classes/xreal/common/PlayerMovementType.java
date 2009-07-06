package xreal.common;

public enum PlayerMovementType {
	
	/**
	 * can accelerate and turn
	 */
	NORMAL,
	
	/**
	 * noclip movement
	 */
	NOCLIP,
	
	/**
	 * still run into walls
	 */
	SPECTATOR,
	
	/**
	 * no acceleration or turning, but free falling
	 */
	DEAD,
	
	/**
	 * stuck in place with no control
	 */
	FREEZE,
	
	/**
	 * no movement or status bar
	 */
	INTERMISSION,
	
	/**
	 * no movement or status bar
	 */
	SPINTERMISSION 

}
