package xreal;

import javax.vecmath.Vector3f;

/**
 * playerState_t is the information needed by both the client and server to
 * predict player motion and actions- Nothing outside of the PlayerController
 * should modify these, or some degree of prediction error will occur.
 * 
 * You can't add anything to this without modifying the code in msg.c.
 * 
 * playerState_t is a full superset of entityState_t as it is used by players,
 * so if a playerState_t is transmitted, the entityState_t can be fully derived
 * from it.
 * 
 * @author Robert Beckebans
 */
public class PlayerState {
	/**
	 * cmd->serverTime of last executed command
	 */
	public int commandTime;

	public int pm_type;

	/**
	 * ducked, jump_held, etc
	 */
	public int pm_flags;
	public int pm_time;

	/**
	 * for view bobbing and footstep generation
	 */
	public int bobCycle;

	public Vector3f origin;
	public Vector3f velocity;

	public int weaponTime;
	public int gravity;
	public int speed;

	/**
	 * add to command angles to get view direction changed by spawns, rotating
	 * objects, and teleporters
	 */
	public int deltaPitch;
	public int deltaYaw;
	public int deltaRoll;

	/**
	 * ENTITYNUM_NONE = in air
	 */
	public int groundEntityNum;

	/**
	 * don't change low priority animations until this runs out
	 */
	public int legsTimer;

	/**
	 * mask off ANIM_TOGGLEBIT
	 */
	public int legsAnim;

	/**
	 * don't change low priority animations until this runs out
	 */
	public int torsoTimer;

	/**
	 * mask off ANIM_TOGGLEBIT
	 */
	public int torsoAnim;

	/**
	 * a number 0 to 7 that represents the relative angle of movement to the
	 * view angle (axial and diagonals) when at rest, the value will remain
	 * unchanged used to twist the legs during strafing
	 */
	public int movementDir;

	/**
	 * location of grapple to pull towards if PMF_GRAPPLE_PULL
	 */
	public Vector3f grapplePoint;

	/**
	 * copied to entityState_t->eFlags
	 */
	public int eFlags;

	/**
	 * pmove generated events
	 */
	public int eventSequence;
	
	// int events[MAX_PS_EVENTS];
	// int eventParms[MAX_PS_EVENTS];

	/**
	 * events set on player from another source
	 */
	public int externalEvent;
	public int externalEventParm;
	public int externalEventTime;

	/**
	 * ranges from 0 to MAX_CLIENTS-1
	 */
	public int clientNum;
	
	/**
	 * copied to entityState_t->weapon
	 */
	public int weapon;
	public int weaponState;

	/**
	 * for fixed views
	 */
	public Vector3f viewAngles;
	public int viewHeight;

	/**
	 * when it changes, latch the other parms
	 */
	public int damageEvent;
	public int damageYaw;
	public int damagePitch;
	public int damageCount;

	// int stats[MAX_STATS];
	// int persistant[MAX_PERSISTANT]; // stats that aren't cleared on death
	// int powerups[MAX_POWERUPS]; // level.time that the powerup runs out
	// int ammo[MAX_WEAPONS];

	public int generic1;
	public int loopSound;
	
	/**
	 * jumppad entity hit this frame
	 */
	public int jumppad_ent;

	// not communicated over the net at all
	public int ping; // server to game info for scoreboard
	public int pmove_framecount; // FIXME: don't transmit over the network
	public int jumppad_frame;
	public int entityEventSequence;
}
