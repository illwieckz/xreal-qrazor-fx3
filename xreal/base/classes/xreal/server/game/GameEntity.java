package xreal.server.game;

import xreal.Engine;

/**
 * Represents, uses and writes to a native gentity_t
 * 
 * @author Robert Beckebans
 */
public class GameEntity {

	/**
	 * Set by the server.
	 */
	private static int numEntities;
	
	/**
	 * Set by the server.
	 */
	private static GameEntity[] entities;
	
	/*
	enum EntityStateField
	{
		number,
		
		eType,
		eFlags,
		
		pos_trType,
		pos_trTime,
		pos_trDuration,
		pos_trAcceleration,
		pos_trBase,
		pos_trDelta,
		
		apos_trType,
		apos_trTime,
		apos_trDuration,
		apos_trAcceleration,
		apos_trBase,
		apos_trDelta,
		
		time,
		time2,
		
		origin,
		origin2,
		
		angles,
		angles2,
		
		otherEntityNum,
		otherEntityNum2,
		
		groundEntityNum,

		constantLight,
		loopSound,
		
		modelindex,
		modelindex2,
		clientNum,
		frame,

		solid,

		event,
		eventParm,

		powerups,
		weapon,
		legsAnim,
		torsoAnim,

		generic1
	}
	*/
	
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
	private static native int allocateEntity0(int reservedIndex);
	
	/**
	 * Marks the entity as free.
	 * 
	 * @param pointer The index the native gentity_t
	 */
	private static native boolean freeEntity0(int index);
	
	public static native int getEntityIndex(int index);
	
	/**
	 * Index of the native gentity_t in the C server::g_entities[] array.
	 * 
	 * This is the same as entityState_t::number and should never change because it is important for the network code.
	 */
	private int entityIndex;
	
	GameEntity()
	{
		entityIndex = allocateEntity0(-1);
		
		Engine.println("GameEntity() allocated native entity using index: " + entityIndex);
	}
	
	GameEntity(int reservedIndex)
	{
		entityIndex = allocateEntity0(reservedIndex);
		
		Engine.println("GameEntity() allocated native entity using index: " + entityIndex);
	}
	
	/**
	 * Called by the garbage collector.
	 * 
	 * Mark the entity as free in the server::g_entities[] slot.
	 */
	protected void finalize() throws Throwable
	{
		Engine.println("GameEntity.finalize()");
		
		try
		{
			freeEntity();
		}
		catch(Exception e)
		{
			Engine.println("GameEntity.finalize() failed");
			throw e;
		}
		finally
		{
			super.finalize();
		}
	}
	
	protected void freeEntity() throws GameException
	{
		if(!freeEntity0(entityIndex))
		{
			throw new GameException("Could not free entity");
		}
		
		// avoid further access to the gentity_t object because it may be used by another GameEntity object
		// this is a save dummy gentity_t not considered by the network
		entityIndex = Engine.ENTITYNUM_NONE;
	}
	
	public int getEntityIndex()
	{
		return entityIndex;
	}
}
