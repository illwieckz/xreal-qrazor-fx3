package xreal;

import java.util.Arrays;

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
	
	// bit field limits
	// Tr3B: NOTE: never go beyond 32 without recoding the delta compression of playerState_t
	// in msg.c
	static final int MAX_STATS = 16;
	static final int MAX_PERSISTANT = 16;
	static final int MAX_POWERUPS = 16;
	static final int MAX_WEAPONS = 16;
	
	/** cmd->serverTime of last executed command */
	public int commandTime;

	public int pm_type;
	public int pm_flags;
	public int pm_time;

	/** for view bobbing and footstep generation */
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

	/** ENTITYNUM_NONE = in air */
	public int groundEntityNum;

	/** don't change low priority animations until this runs out */
	public int legsTimer;

	/** mask off ANIM_TOGGLEBIT */
	public int legsAnim;

	/** don't change low priority animations until this runs out */
	public int torsoTimer;

	/** mask off ANIM_TOGGLEBIT */
	public int torsoAnim;

	/**
	 * a number 0 to 7 that represents the relative angle of movement to the
	 * view angle (axial and diagonals) when at rest, the value will remain
	 * unchanged used to twist the legs during strafing
	 */
	public int movementDir;

	/** location of grapple to pull towards if PMF_GRAPPLE_PULL */
	public Vector3f grapplePoint;

	/** copied to entityState_t->eFlags */
	public int eFlags;

	/** pmove generated events */
	public int eventSequence;
	public int events[]; //MAX_PS_EVENTS];
	public int eventParms[]; //MAX_PS_EVENTS];

	/** events set on player from another source */
	public int externalEvent;
	public int externalEventParm;
	public int externalEventTime;

	/** ranges from 0 to MAX_CLIENTS-1 */
	public int clientNum;
	
	/** copied to entityState_t->weapon */
	public int weapon;
	public int weaponState;

	/** for fixed views */
	public Angle3f viewAngles;
	public int viewHeight;

	/** when it changes, latch the other parms */
	public int damageEvent;
	public int damageYaw;
	public int damagePitch;
	public int damageCount;

	/** MAX_STATS many */
	public int stats[];
	
	/** MAX_PERSISTANT many stats that aren't cleared on death */
	public int persistant[];
	
	/** MAX_POWERUPS many level.time that the powerup runs out */
	public int powerups[];
	
	/** MAX_WEAPONS */
	public int ammo[];

	public int generic1;
	public int loopSound;
	
	/** jumppad entity hit this frame */
	public int jumppad_ent;

	// not communicated over the net at all
	public int ping; // server to game info for scoreboard
	public int pmove_framecount; // FIXME: don't transmit over the network
	public int jumppad_frame;
	public int entityEventSequence;
	
	
	
	public PlayerState(int commandTime, int pmType, int pmFlags, int pmTime,
			int bobCycle, Vector3f origin, Vector3f velocity, int weaponTime,
			int gravity, int speed, int deltaPitch, int deltaYaw,
			int deltaRoll, int groundEntityNum, int legsTimer, int legsAnim,
			int torsoTimer, int torsoAnim, int movementDir,
			Vector3f grapplePoint, int eFlags, int eventSequence, int[] events,
			int[] eventParms, int externalEvent, int externalEventParm,
			int externalEventTime, int clientNum, int weapon, int weaponState,
			Angle3f viewAngles, int viewHeight, int damageEvent, int damageYaw,
			int damagePitch, int damageCount, int[] stats, int[] persistant,
			int[] powerups, int[] ammo, int generic1, int loopSound,
			int jumppadEnt, int ping, int pmoveFramecount, int jumppadFrame,
			int entityEventSequence) {
		super();
		this.commandTime = commandTime;
		pm_type = pmType;
		pm_flags = pmFlags;
		pm_time = pmTime;
		this.bobCycle = bobCycle;
		this.origin = origin;
		this.velocity = velocity;
		this.weaponTime = weaponTime;
		this.gravity = gravity;
		this.speed = speed;
		this.deltaPitch = deltaPitch;
		this.deltaYaw = deltaYaw;
		this.deltaRoll = deltaRoll;
		this.groundEntityNum = groundEntityNum;
		this.legsTimer = legsTimer;
		this.legsAnim = legsAnim;
		this.torsoTimer = torsoTimer;
		this.torsoAnim = torsoAnim;
		this.movementDir = movementDir;
		this.grapplePoint = grapplePoint;
		this.eFlags = eFlags;
		this.eventSequence = eventSequence;
		this.events = events;
		this.eventParms = eventParms;
		this.externalEvent = externalEvent;
		this.externalEventParm = externalEventParm;
		this.externalEventTime = externalEventTime;
		this.clientNum = clientNum;
		this.weapon = weapon;
		this.weaponState = weaponState;
		this.viewAngles = viewAngles;
		this.viewHeight = viewHeight;
		this.damageEvent = damageEvent;
		this.damageYaw = damageYaw;
		this.damagePitch = damagePitch;
		this.damageCount = damageCount;
		this.stats = stats;
		this.persistant = persistant;
		this.powerups = powerups;
		this.ammo = ammo;
		this.generic1 = generic1;
		this.loopSound = loopSound;
		jumppad_ent = jumppadEnt;
		this.ping = ping;
		pmove_framecount = pmoveFramecount;
		jumppad_frame = jumppadFrame;
		this.entityEventSequence = entityEventSequence;
	}



	@Override
	public String toString() {
		return String
				.format(
						"PlayerState [\n\tammo=%s,\n\tbobCycle=%s,\n\tclientNum=%s,\n\tcommandTime=%s,\n\tdamageCount=%s,\n\tdamageEvent=%s,\n\tdamagePitch=%s,\n\tdamageYaw=%s,\n\tdeltaPitch=%s,\n\tdeltaRoll=%s,\n\tdeltaYaw=%s,\n\teFlags=%s,\n\tentityEventSequence=%s,\n\teventParms=%s,\n\teventSequence=%s,\n\tevents=%s,\n\texternalEvent=%s,\n\texternalEventParm=%s,\n\texternalEventTime=%s,\n\tgeneric1=%s,\n\tgrapplePoint=%s,\n\tgravity=%s,\n\tgroundEntityNum=%s,\n\tjumppad_ent=%s,\n\tjumppad_frame=%s,\n\tlegsAnim=%s,\n\tlegsTimer=%s,\n\tloopSound=%s,\n\tmovementDir=%s,\n\torigin=%s,\n\tpersistant=%s,\n\tping=%s,\n\tpm_flags=%s,\n\tpm_time=%s,\n\tpm_type=%s,\n\tpmove_framecount=%s,\n\tpowerups=%s,\n\tspeed=%s,\n\tstats=%s,\n\ttorsoAnim=%s,\n\ttorsoTimer=%s,\n\tvelocity=%s,\n\tviewAngles=%s,\n\tviewHeight=%s,\n\tweapon=%s,\n\tweaponState=%s,\n\tweaponTime=%s\t\n]",
						Arrays.toString(ammo), bobCycle, clientNum,
						commandTime, damageCount, damageEvent, damagePitch,
						damageYaw, deltaPitch, deltaRoll, deltaYaw, eFlags,
						entityEventSequence, Arrays.toString(eventParms),
						eventSequence, Arrays.toString(events), externalEvent,
						externalEventParm, externalEventTime, generic1,
						grapplePoint, gravity, groundEntityNum, jumppad_ent,
						jumppad_frame, legsAnim, legsTimer, loopSound,
						movementDir, origin, Arrays.toString(persistant), ping,
						pm_flags, pm_time, pm_type, pmove_framecount, Arrays
								.toString(powerups), speed, Arrays
								.toString(stats), torsoAnim, torsoTimer,
						velocity, viewAngles, viewHeight, weapon, weaponState,
						weaponTime);
	}



	
	
	
}
