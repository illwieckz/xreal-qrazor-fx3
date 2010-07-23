package xreal.server.game;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;
import javax.vecmath.Vector4f;

import xreal.Engine;
import xreal.Trajectory;
import xreal.TrajectoryType;

import com.bulletphysics.collision.broadphase.BroadphaseInterface;
import com.bulletphysics.collision.broadphase.Dispatcher;
import com.bulletphysics.collision.dispatch.CollisionConfiguration;
import com.bulletphysics.collision.dispatch.CollisionObject;
import com.bulletphysics.dynamics.DiscreteDynamicsWorld;
import com.bulletphysics.dynamics.RigidBody;
import com.bulletphysics.dynamics.constraintsolver.ConstraintSolver;
import com.bulletphysics.linearmath.MotionState;
import com.bulletphysics.linearmath.Transform;

public class GameDynamicsWorld extends DiscreteDynamicsWorld implements Runnable
{
	public GameDynamicsWorld(Dispatcher dispatcher, BroadphaseInterface pairCache, ConstraintSolver constraintSolver,
			CollisionConfiguration collisionConfiguration)
	{
		super(dispatcher, pairCache, constraintSolver, collisionConfiguration);
		// TODO Auto-generated constructor stub
	}

	@Override
	public void run()
	{
		// Thread thisThread = Thread.currentThread();

		// while (ownThread == thisThread) {
		// while(!stopThreadRequested) {
		while(true)
		{
			long stepTime = 1000 / 60;
			
			try
			{
				stepSimulation(stepTime * 0.001f, 10);
				
				for (int j = getNumCollisionObjects() - 1; j >= 0; j--) {
					
					CollisionObject obj = getCollisionObjectArray().get(j);
					RigidBody body = RigidBody.upcast(obj);
					
					if (body != null && body.getMotionState() != null) {
						
						MotionState bodyMotionState = body.getMotionState();
						RigidBodyMotionState gameCopyMotionState = (RigidBodyMotionState) body.getUserPointer();
						
						if (gameCopyMotionState != null) {
							
							Transform trans = new Transform();
							bodyMotionState.getWorldTransform(trans);
							
							Quat4f rotation = new Quat4f();
							trans.getRotation(rotation);
							
							Vector3f linearVelocity = new Vector3f();
							body.getLinearVelocity(linearVelocity);
							
							Vector3f angularVelocity = new Vector3f();
							body.getAngularVelocity(angularVelocity);
							
							Game.updateEntityPhysicsState(gameCopyMotionState.entityNumber, trans.origin, rotation, linearVelocity, angularVelocity);		
						}
					}
				}

				Thread.sleep(stepTime);
			}
			catch(InterruptedException e)
			{
				// Engine.println("Interrupted Exception caught");
				Engine.println("DiscreteDynamicsWorldThread.run(): Interrupted Exception caught in thread = " + Thread.currentThread().getId());
				// destroy();
			}

			// Engine.println("TestBox.run() in thread = " + Thread.currentThread().getId());
		}

		//Engine.println("TestBox.run(): stopped in thread = " + Thread.currentThread().getId());
	}
}
