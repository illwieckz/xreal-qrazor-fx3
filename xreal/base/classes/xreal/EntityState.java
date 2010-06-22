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

	/** entity index */
	public int number;
	
	/** ordinal of EntityType enum */
	public int eType;
	public int eFlags;

	/** for calculating position */
	public Trajectory pos;
	
	/** for calculating angles */
	public Trajectory apos;

	public int time;
	public int time2;

	public Vector3f origin;
	public Vector3f origin2;

	public Angle3f angles;
	public Angle3f angles2;

	/** shotgun sources, etc */
	public int otherEntityNum;
	public int otherEntityNum2;

	/** -1 = in air */
	public int groundEntityNum;

	/** r + (g<<8) + (b<<16) + (intensity<<24) */
	public int constantLight;
	
	/** constantly loop this sound */
	public int loopSound;

	public int modelindex;
	public int modelindex2;
	
	/** 0 to (MAX_CLIENTS - 1), for players and corpses */
	public int clientNum;
	
	public int frame;

	/** for client side prediction, trap_linkentity sets this properly */
	public int solid;

	/** impulse events -- muzzle flashes, footsteps, etc */
	public int event;
	public int eventParm;

	// for players
	
	/** bit flags */
	public int powerups;
	
	/** determines weapon and flash model, etc */
	public int weapon;

	/** mask off ANIM_TOGGLEBIT */
	public int legsAnim;
	
	public int torsoAnim; // mask off ANIM_TOGGLEBIT

	public int generic1;
	
	public EntityState(int number, int eType, int eFlags,
			Trajectory pos, Trajectory apos,
			int time, int time2,
			Vector3f origin, Vector3f origin2,
			Angle3f angles, Angle3f angles2,
			int otherEntityNum, int otherEntityNum2, int groundEntityNum,
			int constantLight, int loopSound, int modelindex, int modelindex2,
			int clientNum, int frame, int solid, int event, int eventParm,
			int powerups, int weapon, int legsAnim, int torsoAnim, int generic1) {
		super();
		this.number = number;
		this.eType = eType;
		this.eFlags = eFlags;
		this.pos = pos;
		this.apos = apos;
		this.time = time;
		this.time2 = time2;
		this.origin = origin;
		this.origin2 = origin2;
		this.angles = angles;
		this.angles2 = angles2;
		this.otherEntityNum = otherEntityNum;
		this.otherEntityNum2 = otherEntityNum2;
		this.groundEntityNum = groundEntityNum;
		this.constantLight = constantLight;
		this.loopSound = loopSound;
		this.modelindex = modelindex;
		this.modelindex2 = modelindex2;
		this.clientNum = clientNum;
		this.frame = frame;
		this.solid = solid;
		this.event = event;
		this.eventParm = eventParm;
		this.powerups = powerups;
		this.weapon = weapon;
		this.legsAnim = legsAnim;
		this.torsoAnim = torsoAnim;
		this.generic1 = generic1;
	}

	@Override
	public String toString() {
		return String
				.format(
						"EntityState [\n\tangles=%s,\n\tangles2=%s,\n\tapos=%s,\n\tclientNum=%s,\n\tconstantLight=%s,\n\teFlags=%s,\n\teType=%s,\n\tevent=%s,\n\teventParm=%s,\n\tframe=%s,\n\tgeneric1=%s,\n\tgroundEntityNum=%s,\n\tlegsAnim=%s,\n\tloopSound=%s,\n\tmodelindex=%s,\n\tmodelindex2=%s,\n\tnumber=%s,\n\torigin=%s,\n\torigin2=%s,\n\totherEntityNum=%s,\n\totherEntityNum2=%s,\n\tpos=%s,\n\tpowerups=%s,\n\tsolid=%s,\n\ttime=%s,\n\ttime2=%s,\n\ttorsoAnim=%s,\n\tweapon=%s\t\n]",
						angles, angles2, apos, clientNum, constantLight,
						eFlags, eType, event, eventParm, frame, generic1,
						groundEntityNum, legsAnim, loopSound, modelindex,
						modelindex2, number, origin, origin2, otherEntityNum,
						otherEntityNum2, pos, powerups, solid, time, time2,
						torsoAnim, weapon);
	}

	
	
}
