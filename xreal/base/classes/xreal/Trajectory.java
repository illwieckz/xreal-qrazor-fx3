package xreal;

import javax.vecmath.Vector3f;

public class Trajectory {
	TrajectoryType  trType = TrajectoryType.STATIONARY;
	int             trTime;
	
	/**
	 * if non 0, trTime + trDuration = stop time
	 */
	int             trDuration;
	
	/**
	 * gravity factor, etc
	 */
	float           trAcceleration;
	
	Vector3f        trBase = new Vector3f();
	
	/**
	 * velocity, etc
	 */
	Vector3f        trDelta = new Vector3f();
}
