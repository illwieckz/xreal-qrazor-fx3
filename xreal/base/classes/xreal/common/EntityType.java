package xreal.common;

/**
 * entityState_t::eType
 * 
 * @author Robert Beckebans
 */
public enum EntityType {
	GENERAL,
	PLAYER,
	ITEM,
	PROJECTILE,
	PROJECTILE2,
	MOVER,
	BEAM,
	PORTAL,
	SPEAKER,
	PUSH_TRIGGER,
	TELEPORT_TRIGGER,
	INVISIBLE,
	
	/**
	 * grapple hooked on wall
	 */
	GRAPPLE,
	
	TEAM,
	
	/**
	 * AI visualization tool
	 */
	AI_NODE,
	AI_LINK,
	
	EXPLOSIVE,
	FIRE,
	
	/**
	 * Any of the EV_* events can be added free standing by setting eType to ET_EVENTS + eventNum.
	 * This avoids having to set eFlags and eventNum.
	 */
	EVENTS
}
