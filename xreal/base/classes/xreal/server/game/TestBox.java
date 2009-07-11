package xreal.server.game;

import javax.vecmath.Vector3f;

import com.bulletphysics.collision.shapes.BoxShape;
import com.bulletphysics.collision.shapes.CollisionShape;
import com.bulletphysics.dynamics.RigidBody;
import com.bulletphysics.dynamics.RigidBodyConstructionInfo;
import com.bulletphysics.linearmath.DefaultMotionState;
import com.bulletphysics.linearmath.Transform;
import com.sun.org.apache.xml.internal.utils.StopParseException;

import xreal.Engine;
import xreal.Trajectory;
import xreal.TrajectoryType;
import xreal.common.EntityType;

public class TestBox extends GameEntity {
	
	TestBox(final Vector3f start, final Vector3f dir)
	{
		//Engine.println("TestBox(start = " + start + ", dir = " + dir);
		
		setEntityState_origin(start);
		setEntityState_eType(EntityType.GENERAL);
		setEntityState_modelindex("models/powerups/ammo/rocketam.md3");
		
		Trajectory pos = new Trajectory();
	
		pos.trType = TrajectoryType.LINEAR;
		pos.trTime = Game.getLevelTime();
		pos.trBase = start;
		pos.trDelta.scale(30, dir);
		
		setEntityState_pos(pos);
		
		link();
		
		//new Thread(this).start();
		
		initPhysics(start);
	}
	
	private void initPhysics(final Vector3f start) {
		
		collisionShape = new BoxShape(new Vector3f(16, 16, 24));
		// colShape = new SphereShape(1f);
		
		Game.getCollisionShapes().add(collisionShape);

		Transform startTransform = new Transform();
		startTransform.setIdentity();

		float mass = 1f;

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
		rigidBody.setUserPointer(this);

		Game.getDynamicsWorld().addRigidBody(rigidBody);
	}
	
	@Override
	public void updateEntityStateByPhysics() {
		//Engine.println("TestBox.updateEntityStateByPhysics()");
		
		if(rigidBody.getMotionState() != null)
		{
			Transform trans = new Transform();
			rigidBody.getMotionState().getWorldTransform(trans);
			
			setEntityState_origin(trans.origin);
			
			Trajectory pos = new Trajectory();
		
			pos.trType = TrajectoryType.STATIONARY;
			pos.trTime = Game.getLevelTime();
			pos.trBase = trans.origin;
			rigidBody.getLinearVelocity(pos.trDelta);
			//pos.trDelta.scale(30, dir);
			
			setEntityState_pos(pos);
			
			link();
		}
	}
}
