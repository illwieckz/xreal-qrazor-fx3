package xreal;

import javax.vecmath.Vector3f;

/**
 * A trace is returned when a box is swept through the world.
 * 
 * @author Robert Beckebans
 */
public class Trace {
	/** if true, plane is not valid */
	boolean        	allsolid;
	
	/** if true, the initial point was in a solid area */
	boolean        	startsolid;
	
	/** time completed, 1.0 = didn't hit anything */
	float           fraction;
	
	/** final position */
	Vector3f        endpos;
	
	/** surface normal at impact, transformed to world space */
//	cplane_t        plane;
	
	/** surface hit */
	int             surfaceFlags;
	
	/** contents on other side of surface hit */
	int             contents;
	
	/** entity the contacted surface is a part of */
	int             entityNum;
	
	/** fraction of collision tangentially to the trace direction */
	float           lateralFraction;
}
