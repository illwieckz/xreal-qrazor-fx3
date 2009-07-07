package xreal;

public enum TrajectoryType {
	STATIONARY,
	
	/**
	 * non-parametric, but interpolate between snapshots
	 */
	INTERPOLATE, 
	LINEAR,
	LINEAR_STOP,
	
	/**
	 * value = base + sin( time / duration ) * delta
	 */
	SINE,
	GRAVITY,
	BUOYANCY,
	ACCELERATION
}
