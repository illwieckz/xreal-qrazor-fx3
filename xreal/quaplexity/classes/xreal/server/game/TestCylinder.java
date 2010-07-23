package xreal.server.game;

import javax.vecmath.Vector3f;

import xreal.CVars;
import xreal.common.EntityType;

import com.bulletphysics.collision.shapes.BoxShape;
import com.bulletphysics.collision.shapes.CylinderShapeZ;
import com.bulletphysics.dynamics.RigidBody;
import com.bulletphysics.dynamics.RigidBodyConstructionInfo;
import com.bulletphysics.linearmath.DefaultMotionState;
import com.bulletphysics.linearmath.Transform;

public class TestCylinder extends TestBox
{

	TestCylinder(Vector3f start, Vector3f dir)
	{
		super(start, dir);
		// TODO Auto-generated constructor stub
		
		setEntityState_eType(EntityType.PHYSICS_CYLINDER);
	}
	
	protected void initPhysics(final Vector3f start, final Vector3f dir) {
		
		Vector3f maxs = new Vector3f(8, 8, 8);
		Vector3f mins = new Vector3f(-8, -8, -8);
		
		setEntityState_solid(mins, maxs);
		
		collisionShape = new CylinderShapeZ(maxs);
		
		Game.getCollisionShapes().add(collisionShape);

		Transform startTransform = new Transform();
		startTransform.setIdentity();

		float mass = 1000.0f;

		// rigidbody is dynamic if and only if mass is non zero, otherwise static
		boolean isDynamic = (mass != 0f);

		Vector3f localInertia = new Vector3f(0, 0, 0);
		if (isDynamic) {
			collisionShape.calculateLocalInertia(mass, localInertia);
		}

		startTransform.origin.set(start);

		// using motionstate is recommended, it provides interpolation
		// capabilities, and only synchronizes 'active' objects
		DefaultMotionState myMotionState = new DefaultMotionState(startTransform);
		RigidBodyConstructionInfo rbInfo = new RigidBodyConstructionInfo(mass, myMotionState, collisionShape, localInertia);
		rigidBody = new RigidBody(rbInfo);
		
		if(CVars.g_threadPhysics.getBoolean())
		{
			RigidBodyMotionState gameCopyMotionState = Game.entityMotionStates.get(getEntityState_number());
			rigidBody.setUserPointer(gameCopyMotionState);
		}
		else
		{
			rigidBody.setUserPointer(this);
		}
		
		Vector3f vel = new Vector3f(dir);
		vel.scale(150);
		rigidBody.setLinearVelocity(vel);
		
		//enable CCD if the object moves more than 1 meter in one simulation frame
		//rigidBody.setCcdMotionThreshold(10);
		//rigidBody.setCcdSweptSphereRadius(20);

		Game.getDynamicsWorld().addRigidBody(rigidBody);
	}
}
