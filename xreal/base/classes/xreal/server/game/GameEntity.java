package xreal.server.game;

import javax.vecmath.Vector3f;

import xreal.Angle3f;
import xreal.Engine;
import xreal.EntityStateAccess;
import xreal.Trajectory;
import xreal.TrajectoryType;
import xreal.common.EntityType;
import xreal.common.WeaponType;

/**
 * Represents, uses and writes to a native gentity_t
 * 
 * @author Robert Beckebans
 */
public class GameEntity implements EntityStateAccess {

	/**
	 * Set by the server.
	 */
	private static int numEntities;
	
	/**
	 * Set by the server.
	 */
	private static GameEntity[] entities;
	
	/**
	 * Similar to Q3A's G_Spawn()
	 * 
	 * Either finds a free entity, or allocates a new one.
	 * 
	 * The slots from 0 to MAX_CLIENTS-1 are always reserved for clients, and will
	 * never be used by anything else.
	 * 
	 * Another reserved index is Engine.ENTITYNUM_WORLD which is always the last but one entity in g_entities[].
	 * 
	 * Try to avoid reusing an entity that was recently freed, because it
	 * can cause the client to think the entity morphed into something else
	 * instead of being removed and recreated, which can cause interpolated
	 * angles and bad trails.
	 * 
	 * @param reservedIndex Should be -1, if this Entity is not meant for a player or the world.
	 * 
	 * @return The index of the native gentity_t in the C server::g_entities[] array.
	 */
	private synchronized static native int allocateEntity0(int reservedIndex);
	
	/**
	 * Marks the entity as free.
	 * 
	 * @param pointer The index the native gentity_t
	 */
	private synchronized static native boolean freeEntity0(int index);
	
	
	/**
	 * Link the entity into the first world sector node that the ent's box crosses.
	 * This is required to calculate the visibility between entities using the BSP's PVS.
	 * The snapshot of entities build for this entity will only contain entities that are visible to this one.
	 * 
	 * @param pointer The index the native gentity_t
	 */
	private synchronized static native void linkEntity0(int index);
	
	/**
	 * Unlink the entity.
	 * 
	 * @param pointer The index the native gentity_t
	 */
	private synchronized static native void unlinkEntity0(int index);
	
	
	// ------------------- entityState_t:: fields in gentity_t::s ---------------------------------
	
	private synchronized static native int getEntityState_eType(int index);
	
	private synchronized static native void setEntityState_eType(int index, int type);
	
	private synchronized static native int getEntityState_eFlags(int index);
	
	private synchronized static native void setEntityState_eFlags(int index, int flags);
	
	private synchronized static native Trajectory getEntityState_pos(int index);
	
	private synchronized static native void setEntityState_pos(int index, int trType, int trTime, int trDuration, float trAcceleration, float trBaseX, float trBaseY, float trBaseZ, float trDeltaX, float trDeltaY, float trDeltaZ);
	
	private synchronized static native Trajectory getEntityState_apos(int index);
	
	private synchronized static native void setEntityState_apos(int index, int trType, int trTime, int trDuration, float trAcceleration, float trBaseX, float trBaseY, float trBaseZ, float trDeltaX, float trDeltaY, float trDeltaZ);
	
	private synchronized static native int getEntityState_time(int index);

	private synchronized static native void setEntityState_time(int index, int time);

	private synchronized static native int getEntityState_time2(int index);

	private synchronized static native void setEntityState_time2(int index, int time2);
	
	private synchronized static native Vector3f getEntityState_origin(int index);
	
	private synchronized static native void setEntityState_origin(int index, float x, float y, float z);
	
	private synchronized static native Vector3f getEntityState_origin2(int index);
	
	private synchronized static native void setEntityState_origin2(int index, float x, float y, float z);
	
	private synchronized static native Angle3f getEntityState_angles(int index);
	
	private synchronized static native void setEntityState_angles(int index, float pitch, float yaw, float roll);
	
	private synchronized static native Angle3f getEntityState_angles2(int index);
	
	private synchronized static native void setEntityState_angles2(int index, float pitch, float yaw, float roll);
	
	private synchronized static native int getEntityState_otherEntityNum(int index);

	private synchronized static native void setEntityState_otherEntityNum(int index, int otherEntityNum);

	private synchronized static native int getEntityState_otherEntityNum2(int index);

	private synchronized static native void setEntityState_otherEntityNum2(int index, int otherEntityNum2);

	private synchronized static native int getEntityState_groundEntityNum(int index);

	private synchronized static native void setEntityState_groundEntityNum(int index, int groundEntityNum);

	private synchronized static native int getEntityState_constantLight(int index);

	private synchronized static native void setEntityState_constantLight(int index, int constantLight);

	private synchronized static native int getEntityState_loopSound(int index);

	private synchronized static native void setEntityState_loopSound(int index, int loopSound);

	private synchronized static native int getEntityState_modelindex(int index);

	private synchronized static native void setEntityState_modelindex(int index, int modelindex);

	private synchronized static native int getEntityState_modelindex2(int index);

	private synchronized static native void setEntityState_modelindex2(int index, int modelindex2);

	private synchronized static native int getEntityState_clientNum(int index);

	private synchronized static native void setEntityState_clientNum(int index, int clientNum);

	private synchronized static native int getEntityState_frame(int index);

	private synchronized static native void setEntityState_frame(int index, int frame);

	private synchronized static native int getEntityState_solid(int index);

	private synchronized static native void setEntityState_solid(int index, int solid);

	private synchronized static native int getEntityState_event(int index);

	private synchronized static native void setEntityState_event(int index, int event);

	private synchronized static native int getEntityState_eventParm(int index);

	private synchronized static native void setEntityState_eventParm(int index, int eventParm);

	private synchronized static native int getEntityState_powerups(int index);

	private synchronized static native void setEntityState_powerups(int index, int powerups);

	private synchronized static native int getEntityState_weapon(int index);

	private synchronized static native void setEntityState_weapon(int index, int weapon);

	private synchronized static native int getEntityState_legsAnim(int index);

	private synchronized static native void setEntityState_legsAnim(int index, int legsAnim);

	private synchronized static native int getEntityState_torsoAnim(int index);

	private synchronized static native void setEntityState_torsoAnim(int index, int torsoAnim);

	private synchronized static native int getEntityState_generic1(int index);

	private synchronized static native void setEntityState_generic1(int index, int generic1);
	
	// --------------------------------------------------------------------------------------------
	
	
	
	
	/**
	 * Index of the native gentity_t in the C server::g_entities[] array.
	 * 
	 * This is the same as entityState_t::number and should never change because it is important for the network code.
	 */
	private int entityIndex;
	
	GameEntity() {
		entityIndex = allocateEntity0(-1);

		Engine.println("GameEntity() allocated native entity using index: " + entityIndex);
	}

	GameEntity(int reservedIndex) {
		entityIndex = allocateEntity0(reservedIndex);

		Engine.println("GameEntity() allocated native entity using index: " + entityIndex);
	}

	/**
	 * Called by the garbage collector.
	 * 
	 * Mark the entity as free in the server::g_entities[] slot.
	 */
	protected void finalize() throws Throwable {
		Engine.println("GameEntity.finalize()");

		try {
			freeEntity();
		} catch (Exception e) {
			Engine.println("GameEntity.finalize() failed");
			throw e;
		} finally {
			unlink();
			super.finalize();
		}
	}

	protected void freeEntity() throws GameException {
		if (!freeEntity0(entityIndex)) {
			throw new GameException("Could not free entity with native entity index: " + entityIndex);
		}

		// avoid further access to the gentity_t object because it may be used
		// by another GameEntity object
		// this is a save dummy gentity_t not considered by the network
		entityIndex = Engine.ENTITYNUM_NONE;
	}
	
	protected void link() {
		linkEntity0(entityIndex);
	}
	
	protected void unlink() {
		unlinkEntity0(entityIndex);
	}
	
	@Override
	public int getEntityState_number() {
		return entityIndex;
	}
	
	@Override
	public int getEntityState_eType() {
		return getEntityState_eType(entityIndex);
	}
	
	public EntityType getEntityType() {
		return EntityType.values()[getEntityState_eType()];
	}
	
	@Override
	public void setEntityState_eType(int type) {
		setEntityState_eType(entityIndex, type);	
	}
	
	public void setEntityState_eType(EntityType type) {
		setEntityState_eType(entityIndex, type.ordinal());	
	}
	
	@Override
	public int getEntityState_eFlags() {
		return getEntityState_eFlags(entityIndex);
	}
	
	@Override
	public void setEntityState_eFlags(int flags)
	{
		setEntityState_eFlags(entityIndex, flags);
	}
	
	@Override
	public Trajectory getEntityState_pos() {
		return getEntityState_pos(entityIndex);
	}
	
	@Override
	public void setEntityState_pos(Trajectory t) {
		setEntityState_pos(entityIndex, t.trType.ordinal(), t.trTime, t.trDuration, t.trAcceleration, t.trBase.x, t.trBase.y, t.trBase.z, t.trDelta.x, t.trDelta.y, t.trDelta.z);
	}
	
	@Override
	public Trajectory getEntityState_apos() {
		return getEntityState_apos(entityIndex);
	}
	
	@Override
	public void setEntityState_apos(Trajectory t) {
		setEntityState_apos(entityIndex, t.trType.ordinal(), t.trTime, t.trDuration, t.trAcceleration, t.trBase.x, t.trBase.y, t.trBase.z, t.trDelta.x, t.trDelta.y, t.trDelta.z);
	}
	
	@Override
	public int getEntityState_time() {
		return getEntityState_time(entityIndex);
	}

	@Override
	public void setEntityState_time(int time) {
		setEntityState_time(entityIndex, time);
	}

	@Override
	public int getEntityState_time2() {
		return getEntityState_time2(entityIndex);
	}

	@Override
	public void setEntityState_time2(int time2) {
		setEntityState_time2(entityIndex, time2);
	}

	@Override
	public Vector3f getEntityState_origin() {
		return getEntityState_origin(entityIndex);
	}

	@Override
	public void setEntityState_origin(Vector3f origin) {
		setEntityState_origin(entityIndex, origin.x, origin.y, origin.z);
	}

	@Override
	public void setEntityState_origin(float x, float y, float z) {
		setEntityState_origin(entityIndex, x, y, z);
	}
	
	@Override
	public Vector3f getEntityState_origin2() {
		return getEntityState_origin2(entityIndex);
	}

	@Override
	public void setEntityState_origin2(Vector3f origin) {
		setEntityState_origin2(entityIndex, origin.x, origin.y, origin.z);
	}

	@Override
	public void setEntityState_origin2(float x, float y, float z) {
		setEntityState_origin2(entityIndex, x, y, z);
	}
	
	@Override
	public Angle3f getEntityState_angles() {
		return getEntityState_angles(entityIndex);
	}
	
	@Override
	public void setEntityState_angles(Angle3f angles) {
		setEntityState_angles(entityIndex, angles.x, angles.y, angles.z);
	}
	
	@Override
	public void setEntityState_angles(float pitch, float yaw, float roll) {
		setEntityState_angles(entityIndex, pitch, yaw, roll);
	}
	
	@Override
	public Angle3f getEntityState_angles2() {
		return getEntityState_angles2(entityIndex);
	}
	
	@Override
	public void setEntityState_angles2(Angle3f angles) {
		setEntityState_angles2(entityIndex, angles.x, angles.y, angles.z);
	}
	
	@Override
	public void setEntityState_angles2(float pitch, float yaw, float roll) {
		setEntityState_angles2(entityIndex, pitch, yaw, roll);
	}

	@Override
	public int getEntityState_otherEntityNum() {
		return getEntityState_otherEntityNum(entityIndex);
	}

	@Override
	public void setEntityState_otherEntityNum(int otherEntityNum) {
		setEntityState_otherEntityNum(entityIndex, otherEntityNum);
	}
	
	@Override
	public int getEntityState_otherEntityNum2() {
		return getEntityState_otherEntityNum2(entityIndex);
	}

	@Override
	public void setEntityState_otherEntityNum2(int otherEntityNum2) {
		setEntityState_otherEntityNum2(entityIndex, otherEntityNum2);
	}

	@Override
	public int getEntityState_groundEntityNum() {
		return getEntityState_groundEntityNum(entityIndex);
	}

	@Override
	public void setEntityState_groundEntityNum(int groundEntityNum) {
		setEntityState_groundEntityNum(entityIndex, groundEntityNum);
	}

	@Override
	public int getEntityState_constantLight() {
		return getEntityState_constantLight(entityIndex);
	}

	@Override
	public void setEntityState_constantLight(int constantLight) {
		setEntityState_constantLight(entityIndex, constantLight);
	}

	@Override
	public int getEntityState_loopSound() {
		return getEntityState_loopSound(entityIndex);
	}

	@Override
	public void setEntityState_loopSound(int loopSound) {
		setEntityState_loopSound(entityIndex, loopSound);
	}

	@Override
	public int getEntityState_modelindex() {
		return getEntityState_modelindex(entityIndex);
	}

	@Override
	public void setEntityState_modelindex(int modelindex) {
		setEntityState_modelindex(entityIndex, modelindex);
	}

	@Override
	public int getEntityState_modelindex2() {
		return getEntityState_modelindex2(entityIndex);
	}

	@Override
	public void setEntityState_modelindex2(int modelindex2) {
		setEntityState_modelindex2(entityIndex, modelindex2);
	}

	@Override
	public int getEntityState_clientNum() {
		return getEntityState_clientNum(entityIndex);
	}

	@Override
	public void setEntityState_clientNum(int clientNum) {
		setEntityState_clientNum(entityIndex, clientNum);
	}

	@Override
	public int getEntityState_frame() {
		return getEntityState_frame(entityIndex);
	}

	@Override
	public void setEntityState_frame(int frame){
		setEntityState_frame(entityIndex, frame);
	}

	@Override
	public int getEntityState_solid() {
		return getEntityState_solid(entityIndex);
	}

	@Override
	public void setEntityState_solid(int solid) {
		setEntityState_solid(entityIndex, solid);
	}

	@Override
	public int getEntityState_event() {
		return getEntityState_event(entityIndex);
	}

	@Override
	public void setEntityState_event(int event) {
		setEntityState_event(entityIndex, event);
	}

	@Override
	public int getEntityState_eventParm() {
		return getEntityState_eventParm(entityIndex);
	}

	@Override
	public void setEntityState_eventParm(int eventParm) {
		setEntityState_eventParm(entityIndex, eventParm);
	}

	@Override
	public int getEntityState_powerups() {
		return getEntityState_powerups(entityIndex);
	}

	@Override
	public void setEntityState_powerups(int powerups) {
		setEntityState_powerups(entityIndex, powerups);
	}

	public WeaponType getEntityState_weapon() {
		return WeaponType.values()[getEntityState_weapon(entityIndex)];
	}

	public void setEntityState_weapon(WeaponType weapon) {
		setEntityState_weapon(entityIndex, weapon.ordinal());
	}

	@Override
	public int getEntityState_legsAnim() {
		return getEntityState_legsAnim(entityIndex);
	}

	@Override
	public void setEntityState_legsAnim(int legsAnim) {
		setEntityState_legsAnim(entityIndex, legsAnim);
	}
	
	@Override
	public int getEntityState_torsoAnim() {
		return getEntityState_torsoAnim(entityIndex);
	}

	@Override
	public void setEntityState_torsoAnim(int torsoAnim) {
		setEntityState_torsoAnim(entityIndex, torsoAnim);
	}

	@Override
	public int getEntityState_generic1() {
		return getEntityState_generic1(entityIndex);
	}

	@Override
	public void setEntityState_generic1(int generic1) {
		setEntityState_generic1(entityIndex, generic1);
	}
}
