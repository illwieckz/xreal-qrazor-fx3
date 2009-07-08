package xreal;

import javax.vecmath.Vector3f;

import xreal.common.PlayerMovementType;

//playerState_t is the information needed by both the client and server
//to predict player motion and actions
//nothing outside of pmove should modify these, or some degree of prediction error
//will occur

//you can't add anything to this without modifying the code in msg.c

//playerState_t is a full supersetPlayerState_ of entityState_t as it is used by players,
//so if a playerState_t is transmitted, the entityState_t can be fully derived
//from it.
/*
 typedef struct playerState_s
 {
 int             commandTime;	// cmd->serverTime of last executed command
 int             pm_type;
 int             bobCycle;	// for view bobbing and footstep generation
 int             pm_flags;	// ducked, jump_held, etc
 int             pm_time;

 vec3_t          origin;
 vec3_t          velocity;
 int             weaponTime;
 int             gravity;
 int             speed;
 int             delta_angles[3];	// add to command angles to getPlayerState_ view direction
 // changed by spawns, rotating objects, and teleporters

 int             groundEntityNum;	// ENTITYNUM_NONE = in air

 int             legsTimer;	// don't change low priority animations until this runs out
 int             legsAnim;	// mask off ANIM_TOGGLEBIT

 int             torsoTimer;	// don't change low priority animations until this runs out
 int             torsoAnim;	// mask off ANIM_TOGGLEBIT

 int             movementDir;	// a number 0 to 7 that represents the relative angle
 // of movement to the view angle (axial and diagonals)
 // when at rest, the value will remain unchanged
 // used to twist the legs during strafing

 vec3_t          grapplePoint;	// location of grapple to pull towards if PMF_GRAPPLE_PULL

 int             eFlags;		// copied to entityState_t->eFlags

 int             eventSequence;	// pmove generated events
 int             events[MAX_PS_EVENTS];
 int             eventParms[MAX_PS_EVENTS];

 int             externalEvent;	// events setPlayerState_ on player from another source
 int             externalEventParm;
 int             externalEventTime;

 int             clientNum;	// ranges from 0 to MAX_CLIENTS-1
 int             weapon;		// copied to entityState_t->weapon
 int             weaponstate;

 vec3_t          viewangles;	// for fixed views
 int             viewheight;

 // damage feedback
 int             damageEvent;	// when it changes, latch the other parms
 int             damageYaw;
 int             damagePitch;
 int             damageCount;

 int             stats[MAX_STATS];
 int             persistant[MAX_PERSISTANT];	// stats that aren't cleared on death
 int             powerups[MAX_POWERUPS];	// level.time that the powerup runs out
 int             ammo[MAX_WEAPONS];

 int             generic1;
 int             loopSound;
 int             jumppad_ent;	// jumppad entity hit this frame

 // not communicated over the net at all
 int             ping;		// server to game info for scoreboard
 int             pmove_framecount;	// FIXME: don't transmit over the network
 int             jumppad_frame;
 int             entityEventSequence;
 } playerState_t;
 */

public interface PlayerStateAccess {
	
	public int getPlayerState_commandTime();

	public void setPlayerState_commandTime(int commandTime);

	public PlayerMovementType getPlayerState_pm_type();

	public void setPlayerState_pm_type(PlayerMovementType pm_type);

	public int getPlayerState_pm_flags();

	public void setPlayerState_pm_flags(int pm_flags);
	
	public void addPlayerState_pm_flags(int pm_flags);
	
	public void delPlayerState_pm_flags(int pm_flags);
	
	public boolean hasPlayerState_pm_flags(int pm_flags);

	public int getPlayerState_pm_time();

	public void setPlayerState_pm_time(int pm_time);

	public int getPlayerState_bobCycle();

	public void setPlayerState_bobCycle(int bobCycle);

	public Vector3f getPlayerState_origin();

	public void setPlayerState_origin(Vector3f origin);

	public Vector3f getPlayerState_velocity();

	public void setPlayerState_velocity(Vector3f velocity);

	public int getPlayerState_weaponTime();

	public void setPlayerState_weaponTime(int weaponTime);

	public int getPlayerState_gravity();

	public void setPlayerState_gravity(int gravity);

	public int getPlayerState_speed();

	public void setPlayerState_speed(int speed);

	public int getPlayerState_deltaPitch();

	public void setPlayerState_deltaPitch(int deltaPitch);

	public int getPlayerState_deltaYaw();

	public void setPlayerState_deltaYaw(int deltaYaw);

	public int getPlayerState_deltaRoll();

	public void setPlayerState_deltaRoll(int deltaRoll);

	public int getPlayerState_groundEntityNum();

	public void setPlayerState_groundEntityNum(int groundEntityNum);

	public int getPlayerState_legsTimer();

	public void setPlayerState_legsTimer(int legsTimer);

	public int getPlayerState_legsAnim();

	public void setPlayerState_legsAnim(int legsAnim);

	public int getPlayerState_torsoTimer();

	public void setPlayerState_torsoTimer(int torsoTimer);

	public int getPlayerState_torsoAnim();

	public void setPlayerState_torsoAnim(int torsoAnim);

	public int getPlayerState_movementDir();

	public void setPlayerState_movementDir(int movementDir);

	public Vector3f getPlayerState_grapplePoint();

	public void setPlayerState_grapplePoint(Vector3f grapplePoint);

	public int getPlayerState_eFlags();

	public void setPlayerState_eFlags(int flags);

	public int getPlayerState_eventSequence();

	public void setPlayerState_eventSequence(int eventSequence);

	public int getPlayerState_externalEvent();

	public void setPlayerState_externalEvent(int externalEvent);

	public int getPlayerState_externalEventParm();

	public void setPlayerState_externalEventParm(int externalEventParm);

	public int getPlayerState_externalEventTime();

	public void setPlayerState_externalEventTime(int externalEventTime);

	public int getPlayerState_clientNum();

	public void setPlayerState_clientNum(int clientNum);

	public int getPlayerState_weapon();

	public void setPlayerState_weapon(int weapon);

	public int getPlayerState_weaponState();

	public void setPlayerState_weaponState(int weaponState);

	public Angle3f getPlayerState_viewAngles();

	public void setPlayerState_viewAngles(Angle3f viewAngles);
	
	public void setPlayerState_viewAngles(float pitch, float yaw, float roll);

	public int getPlayerState_viewHeight();

	public void setPlayerState_viewHeight(int viewHeight);

	public int getPlayerState_damageEvent();

	public void setPlayerState_damageEvent(int damageEvent);
	
	public Vector3f getPlayerState_deltaAngles();

	public int getPlayerState_damageYaw();

	public void setPlayerState_damageYaw(int damageYaw);

	public int getPlayerState_damagePitch();

	public void setPlayerState_damagePitch(int damagePitch);

	public int getPlayerState_damageCount();

	public void setPlayerState_damageCount(int damageCount);

	public int getPlayerState_generic1();

	public void setPlayerState_generic1(int generic1);

	public int getPlayerState_loopSound();

	public void setPlayerState_loopSound(int loopSound);

	public int getPlayerState_jumppad_ent();

	public void setPlayerState_jumppad_ent(int jumppad_ent);

	public int getPlayerState_ping();

	public void setPlayerState_ping(int ping);

	/*
	public int getPlayerState_pmove_framecount();

	public void setPlayerState_pmove_framecount(int pmove_framecount);

	public int getPlayerState_jumppad_frame();

	public void setPlayerState_jumppad_frame(int jumppad_frame);

	public int getPlayerState_entityEventSequence();

	public void setPlayerState_entityEventSequence(int entityEventSequence);
	*/
}
