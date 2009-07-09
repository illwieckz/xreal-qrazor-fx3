package xreal;

import javax.vecmath.Vector3f;

/**
 * 
 * @author Robert Beckebans
 */
public class Trajectory {
	
	public Trajectory() {
		this.trBase = new Vector3f();
		this.trDelta = new Vector3f();
	}
	
	public Trajectory(TrajectoryType trType, int trTime, int trDuration, float trAcceleration, Vector3f trBase, Vector3f trDelta) {
		super();
		
		this.trType = trType;
		this.trTime = trTime;
		this.trDuration = trDuration;
		this.trAcceleration = trAcceleration;
		this.trBase = trBase;
		this.trDelta = trDelta;
	}
	
	public Trajectory(int trType, int trTime, int trDuration, float trAcceleration, float trBaseX, float trBaseY, float trBaseZ, float trDeltaX, float trDeltaY, float trDeltaZ) {
		super();
		
		this.trType = TrajectoryType.values()[trType];
		this.trTime = trTime;
		this.trDuration = trDuration;
		this.trAcceleration = trAcceleration;
		this.trBase = new Vector3f(trBaseX, trBaseY, trBaseZ);
		this.trDelta = new Vector3f(trDeltaX, trDeltaY, trDeltaZ);
	}

	public TrajectoryType  trType = TrajectoryType.STATIONARY;
	public int             trTime;
	
	/**
	 * if non 0, trTime + trDuration = stop time
	 */
	public int             trDuration;
	
	/**
	 * gravity factor, etc
	 */
	public float           trAcceleration;
	
	public Vector3f        trBase;
	
	/**
	 * velocity, etc
	 */
	public Vector3f        trDelta;
}
