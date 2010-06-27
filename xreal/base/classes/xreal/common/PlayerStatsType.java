package xreal.common;

/**
 * player_state->stats[] indexes
 * NOTE: may not have more than 16
 * 
 * @author Robert Beckebans
 */
public enum PlayerStatsType {
	
	HEALTH,
	HOLDABLE_ITEM,
	
	/*#ifdef MISSIONPACK
		STAT_PERSISTANT_POWERUP,
	#endif */
	
	/** 16 bit fields */
	WEAPONS,
	ARMOR,
	
	/** look this direction when dead (FIXME: get rid of?) */
	DEAD_YAW,
	
	
	/** bit mask of clients wishing to exit the intermission (FIXME: configstring?) */
	CLIENTS_READY,
	
	/** health / armor limit, changable by handicap */
	MAX_HEALTH
}
