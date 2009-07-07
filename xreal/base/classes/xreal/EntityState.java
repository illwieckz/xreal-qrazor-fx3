package xreal;

import javax.vecmath.Vector3f;

/**
 * EntityState_t is the information conveyed from the server in an update
 * message about entities that the client will need to render in some way.
 * 
 * Different eTypes may use the information in different ways The messages are
 * delta compressed, so it doesn't really matter if the structure size is fairly
 * large
 * 
 * @author Robert Beckebans
 */
public class EntityState {

	/**
	 * entity index
	 */
	public int number;
	
	/**
	 * ordinal of EntityType enum
	 */
	public int eType;
	public int eFlags;

	/**
	 * for calculating position
	 */
	public Trajectory pos;
	
	/**
	 * for calculating angles
	 */
	public Trajectory apos;

	public int time;
	public int time2;

	public Vector3f origin;
	public Vector3f origin2;

	public Vector3f angles;
	public Vector3f angles2;

	/**
	 * shotgun sources, etc
	 */
	public int otherEntityNum;
	public int otherEntityNum2;

	/**
	 * -1 = in air
	 */
	public int groundEntityNum;

	/**
	 * r + (g<<8) + (b<<16) + (intensity<<24)
	 */
	public int constantLight;
	
	/**
	 * constantly loop this sound
	 */
	public int loopSound;

	public int modelindex;
	public int modelindex2;
	
	/**
	 * 0 to (MAX_CLIENTS - 1), for players and corpses
	 */
	public int clientNum;
	
	public int frame;

	/**
	 * for client side prediction, trap_linkentity sets this properly
	 */
	public int solid;

	/**
	 * impulse events -- muzzle flashes, footsteps, etc
	 */
	public int event;
	public int eventParm;

	// for players
	
	/**
	 * bit flags
	 */
	public int powerups;
	
	/**
	 * determines weapon and flash model, etc
	 */
	public int weapon;
	
	/**
	 * mask off ANIM_TOGGLEBIT
	 */
	public int legsAnim;
	
	public int torsoAnim; // mask off ANIM_TOGGLEBIT

	public int generic1;
}
