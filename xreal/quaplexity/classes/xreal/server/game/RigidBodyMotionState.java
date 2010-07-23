package xreal.server.game;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;

public class RigidBodyMotionState
{
	public int entityNumber;
	
	public RigidBodyMotionState(int entityNumber)
	{
		this.entityNumber = entityNumber;
	}
	
	public Vector3f origin = new Vector3f();
	public Quat4f	rotation = new Quat4f();
	public Vector3f	linearVelocity = new Vector3f();
	public Vector3f angularVelocity = new Vector3f();
}
