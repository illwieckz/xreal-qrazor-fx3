package xreal.common;

public abstract class PlayerMovementFlags {
	
	public static final int DUCKED = (1 << 0);
	
	public static final int JUMP_HELD = (1 << 1);

	/**
	 * go into backwards land
	 */
	public static final int BACKWARDS_JUMP = (1 << 2);

	/**
	 * coast down to backwards run
	 */
	public static final int BACKWARDS_RUN = (1 << 3);

	/**
	 * pm_time is time before rejump
	 */
	public static final int TIME_LAND = (1 << 4);

	/**
	 * pm_time is an air-accelerate only time
	 */
	public static final int TIME_KNOCKBACK = (1 << 5);

	/**
	 * pm_time is waterjump
	 */
	public static final int TIME_WATERJUMP = (1 << 6);

	/**
	 * clear after attack and jump buttons come up
	 */
	public static final int RESPAWNED = (1 << 7);

	public static final int USE_ITEM_HELD = (1 << 8);

	/**
	 * pull towards grapple location
	 */
	public static final int GRAPPLE_PULL = (1 << 9);

	/**
	 * spectate following another player
	 */
	public static final int FOLLOW = (1 << 10);

	/**
	 * spectate as a scoreboard
	 */
	public static final int SCOREBOARD = (1 << 11);

	/**
	 * invulnerability sphere set to full size
	 */
	public static final int INVULEXPAND = (1 << 12);
	public static final int WALLCLIMBING = (1 << 13);
	public static final int WALLCLIMBINGCEILING = (1 << 14);

	public static final int ALL_TIMES = (TIME_WATERJUMP | TIME_LAND | TIME_KNOCKBACK);
}
