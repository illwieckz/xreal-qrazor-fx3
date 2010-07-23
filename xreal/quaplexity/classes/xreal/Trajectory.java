package xreal;

import javax.vecmath.AxisAngle4f;
import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;
import javax.vecmath.Vector4f;

/**
 * 
 * @author Robert Beckebans
 */
public class Trajectory {
	
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
	
	public Vector4f        trBase;
	
	/**
	 * velocity, etc
	 */
	public Vector4f        trDelta;
	
	
	
	public Trajectory() {
		this.trBase = new Vector4f();
		this.trDelta = new Vector4f();
	}
	
	public Trajectory(TrajectoryType trType, int trTime, int trDuration, float trAcceleration, Vector4f trBase, Vector4f trDelta) {
		super();
		
		this.trType = trType;
		this.trTime = trTime;
		this.trDuration = trDuration;
		this.trAcceleration = trAcceleration;
		this.trBase = trBase;
		this.trDelta = trDelta;
	}
	
	public Trajectory(int trType, int trTime, int trDuration, float trAcceleration, float trBaseX, float trBaseY, float trBaseZ, float trBaseW, float trDeltaX, float trDeltaY, float trDeltaZ, float trDeltaW) {
		super();
		
		this.trType = TrajectoryType.values()[trType];
		this.trTime = trTime;
		this.trDuration = trDuration;
		this.trAcceleration = trAcceleration;
		this.trBase = new Vector4f(trBaseX, trBaseY, trBaseZ, trBaseW);
		this.trDelta = new Vector4f(trDeltaX, trDeltaY, trDeltaZ, trDeltaW);
	}
	
	
	
	
	/**
	 * Evaluate the trajectory's position for a given time.
	 * 
	 * @param atTime
	 
	 * @return The position.
	 */
	public Vector3f evaluatePosition(int atTime)
	{
		float           deltaTime;
		float           phase;
		Vector4f		position = new Vector4f();

		switch (trType)
		{
			case STATIONARY:
			case INTERPOLATE:
				position.set(trBase);
				break;

			case LINEAR:
				deltaTime = (atTime - trTime) * 0.001f;	// milliseconds to seconds
				position.scaleAdd(deltaTime, trDelta, trBase);
				break;

			case SINE:
				deltaTime = (atTime - trTime) / (float)trDuration;
				phase = (float) Math.sin(deltaTime * Math.PI * 2);
				position.scaleAdd(phase, trDelta, trBase);
				break;

			case LINEAR_STOP:
				if(atTime > trTime + trDuration)
				{
					atTime = trTime + trDuration;
				}
				deltaTime = (atTime - trTime) * 0.001f;	// milliseconds to seconds
				if(deltaTime < 0)
				{
					deltaTime = 0;
				}
				position.scaleAdd(deltaTime, trDelta, trBase);
				break;

			case GRAVITY:
				deltaTime = (atTime - trTime) * 0.001f;	// milliseconds to seconds
				position.scaleAdd(deltaTime, trDelta, trBase);
				position.z -= 0.5 * trAcceleration * deltaTime * deltaTime; // trAcceleration is the gravity
				break;

			case BUOYANCY:
				deltaTime = (atTime - trTime) * 0.001f;	// milliseconds to seconds
				position.scaleAdd(deltaTime, trDelta, trBase);
				position.z += 0.5 * trAcceleration * deltaTime * deltaTime;	// trAcceleration is the gravity
				break;

			case ACCELERATION:
				// Tr3B: see http://code3arena.planetquake.gamespy.com/tutorials/tutorial38.shtml

				// simple physics equation, normally written like so:
				// s = u*t + .5*a*t^2

				// milliseconds to seconds
				deltaTime = (atTime - trTime) * 0.001f;

				// the u*t part. adds the velocity of the object
				// multiplied by the time to the last result.
				position.scaleAdd(deltaTime, trDelta, trBase);

				// so far that was the same as TR_LINEAR

				// the .5*a*t^2 part.
				// phase gives the magnitude of the distance we need to move
				phase = (0.5f * trAcceleration) * (deltaTime * deltaTime);

				// make dir equal to the velocity of the object
				Vector3f dir = new Vector3f(trDelta.x, trDelta.y, trDelta.z);
				dir.normalize();
				dir.scale(phase);
				
				// move a distance "phase" in the direction "dir" from our starting point
				position.add(new Vector4f(dir));
				break;
		}
		
		return new Vector3f(position.x, position.y, position.z);
	}
	
	/**
	 * For determining velocity at a given time.
	 */
	public Vector3f evaluateVelocity(int atTime)
	{
		float           deltaTime;
		float           phase;
		Vector4f		velocity = new Vector4f();

		switch (trType)
		{
			case STATIONARY:
			case INTERPOLATE:
				break;

			case LINEAR:
				velocity.set(trDelta);
				break;
				
			case SINE:
				deltaTime = (atTime - trTime) / (float)trDuration;
				phase = (float) Math.cos(deltaTime * Math.PI * 2);	// derivative of sin = cos
				phase *= 0.5;
				velocity.scale(phase, trDelta);
				break;
				
			case LINEAR_STOP:
				if(atTime > trTime + trDuration)
				{
					break;
				}
				velocity.set(trDelta);
				break;
				
			case GRAVITY:
				deltaTime = (atTime - trTime) * 0.001f;	// milliseconds to seconds
				velocity.set(trDelta);
				velocity.z -= trAcceleration * deltaTime;
				break;
				
			case BUOYANCY:
				deltaTime = (atTime - trTime) * 0.001f;	// milliseconds to seconds
				velocity.set(trDelta);
				velocity.z += trAcceleration * deltaTime;
				break;
				
			case ACCELERATION:
				deltaTime = (atTime - trTime) * 0.001f;

				// turn magnitude of acceleration into a vector
				Vector3f dir = new Vector3f(trDelta.x, trDelta.y, trDelta.z);
				dir.normalize();
				dir.scale(trAcceleration);

				// u + t * a = v
				velocity.scaleAdd(deltaTime, new Vector4f(dir), trDelta);
				break;
		}
		
		return new Vector3f(velocity.x, velocity.y, velocity.z);
	}
	
	/**
	 * For determining velocity at a given time.
	 */
	public Quat4f evaluateRotation(int atTime)
	{
		float           deltaTime;
		float           phase;
		Quat4f			rotation = new Quat4f();

		switch (trType)
		{
			default:
			case STATIONARY:
			case INTERPOLATE:
				rotation.set(trBase);
				break;

			case LINEAR:
				deltaTime = (atTime - trTime) * 0.001f;	// milliseconds to seconds
				
				
				AxisAngle4f deltaAxisAngle = new AxisAngle4f();
				
				//deltaAxisAngle.set(new Quat4f(trDelta));
				deltaAxisAngle.set(trDelta);
				
				deltaAxisAngle.angle *= deltaTime;
				
				Quat4f deltaQuat = new Quat4f();
				deltaQuat.set(deltaAxisAngle);
				
				rotation.set(trBase);
				rotation.mul(deltaQuat);
				
				break;
		}
		
		return rotation;
	}
}
