package xreal;

import javax.vecmath.Vector3f;

import xreal.common.EntityType;

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

	// entityState_t->event values
	// entity events are for effects that take place reletive
	// to an existing entities origin.  Very network efficient.

	// two bits at the top of the entityState->event field
	// will be incremented with each change in the event so
	// that an identical event started twice in a row can
	// be distinguished.  And off the value with ~EV_EVENT_BITS
	// to retrieve the actual event number
	public static final int EV_EVENT_BIT1 = 0x00000100;
	public static final int EV_EVENT_BIT2 = 0x00000200;
	public static final int EV_EVENT_BITS = (EV_EVENT_BIT1|EV_EVENT_BIT2);

	public static final int EVENT_VALID_MSEC = 300;
	
	/** entity index */
	private int number;

	/** ordinal of EntityType enum */
	public EntityType eType;
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
		this.eType = EntityType.values()[eType];
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
	
	public EntityState(int clientNum) {
		number = clientNum;
		
		eType = EntityType.GENERAL;
		
		this.pos = new Trajectory();
		this.apos = new Trajectory();
	}

	public int getNumber() {
		return number;
	}
	
	// --------------------------------------------------------------------------------------------
	// entityState_t->eFlags
	
	private void addEntityFlags(int flags) {
		eFlags |= flags;
	}
	
	private void delEntityFlags(int flags) {
		eFlags = eFlags & ~flags;
	}
	
	private boolean hasEntityFlags(int flags) {
		return (eFlags & flags) != 0;
	}

	/** don't draw a foe marker over players with EF_DEAD */
	private static final int EF_DEAD	= (1 << 0);
	
	/** toggled every time the origin abruptly changes */
	private static final int EF_TELEPORT_BIT		= (1 << 1);
	
	/** draw an excellent sprite */
	private static final int EF_AWARD_EXCELLENT		= (1 << 2);
	
	
	private static final int EF_PLAYER_EVENT		= (1 << 3);
	
	/** for missiles */
	private static final int EF_BOUNCE				= (1 << 4);
	
	/** for missiles */
	private static final int EF_BOUNCE_HALF			= (1 << 5);
	
	/** draw a gauntlet sprite */
	private static final int EF_AWARD_GAUNTLET		= (1 << 6);
	
	/** may have an event, but no model (unspawned items) */
	private static final int EF_NODRAW				= (1 << 7);
	
	/** for lightning gun */
	private static final int EF_FIRING				= (1 << 8);
	
	/** will push otherwise */
	private static final int EF_MOVER_STOP			= (1 << 9);
	
	/** draw the capture sprite */
	private static final int EF_AWARD_CAP			= (1 << 10);
	
	/** draw a talk balloon */
	private static final int EF_TALK				= (1 << 11);
	
	/** draw a connection trouble sprite */
	private static final int EF_CONNECTION			= (1 << 12);
	
	/** already cast a vote */
	private static final int EF_VOTED				= (1 << 13);
	
	/** draw an impressive sprite */
	private static final int EF_AWARD_IMPRESSIVE	= (1 << 14);
	
	/** draw a defend sprite */
	private static final int EF_AWARD_DEFEND		= (1 << 15);
	
	/** draw a assist sprite */
	private static final int EF_AWARD_ASSIST		= (1 << 16);
	
	/** denied */
	private static final int EF_AWARD_DENIED		= (1 << 17);
	
	/** draw a telefrag sprite */
	private static final int EF_AWARD_TELEFRAG		= (1 << 18);
	
	/** already cast a team vote */
	private static final int EF_TEAMVOTED			= (1 << 19);
	
	
	private static final int EF_KAMIKAZE			= (1 << 20);
	
	/** used to make players play the prox mine ticking sound */
	private static final int EF_TICKING				= (1 << 21);
	
	/** for lightning gun */
	private static final int EF_FIRING2				= (1 << 22);
	
	
	/** TA: wall walking */
	private static final int EF_WALLCLIMB			= (1 << 23);
	
	/** TA: wall walking ceiling hack */
	private static final int EF_WALLCLIMBCEILING	= (1 << 24);
	
	public boolean isEntityFlag_dead() {
		return hasEntityFlags(EF_DEAD);
	}

	public void setEntityFlag_dead(boolean dead) {
		
		if(dead) {
			addEntityFlags(EF_DEAD);
		} else {
			delEntityFlags(EF_DEAD);
		}
	}

	public boolean isEntityFlag_wallClimbCeiling() {
		return hasEntityFlags(EF_WALLCLIMBCEILING);
	}

	public void setEntityFlag_wallClimbCeiling(boolean b) {
		
		if(b) {
			addEntityFlags(EF_WALLCLIMBCEILING);
		} else {
			delEntityFlags(EF_WALLCLIMBCEILING);
		}
	}
	
	
	
	// --------------------------------------------------------------------------------------------

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
