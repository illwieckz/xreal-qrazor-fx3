package xreal.client.game;

import javax.vecmath.AxisAngle4f;
import javax.vecmath.Matrix3f;
import javax.vecmath.Matrix4f;
import javax.vecmath.Point3f;
import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;

import xreal.Angle3f;
import xreal.Color;
import xreal.client.EntityState;
import xreal.client.game.util.RenderUtils;
import xreal.client.renderer.Polygon;
import xreal.client.renderer.Renderer;
import xreal.client.renderer.Vertex;

import com.bulletphysics.collision.dispatch.CollisionObject;

public class CEntity_PhysicsCylinder extends CEntity {
	
	public CEntity_PhysicsCylinder(EntityState es) {
		super(es);
	}

	
	@Override
	public void addToRenderer() throws Exception 
	{
		super.addToRenderer();
		
		Vector3f        mins = new Vector3f();
		Vector3f        maxs = new Vector3f();

		{
			int             x, zd, zu;

			// otherwise grab the encoded bounding box
			x = (currentState.solid & 255);
			zd = ((currentState.solid >> 8) & 255);
			zu = ((currentState.solid >> 16) & 255) - 32;

			mins.x = mins.y = -x;
			maxs.x = maxs.y = x;
			mins.z = -zd;
			maxs.z = zu;
		}

		Color color;
		switch (currentState.generic1)
		{
			case CollisionObject.ACTIVE_TAG:
				color = Color.White;
				break;

			case CollisionObject.ISLAND_SLEEPING:
				color = Color.Green;
				break;

			case CollisionObject.WANTS_DEACTIVATION:
				color = Color.Cyan;
				break;

			case CollisionObject.DISABLE_DEACTIVATION:
				color = Color.Red;
				break;

			case CollisionObject.DISABLE_SIMULATION:
				color = Color.Yellow;
				break;

			default:
				color = Color.Magenta;
				break;
		}

		Matrix3f rotation = new Matrix3f();
		rotation.setIdentity();
		rotation.set(lerpQuat);
		//lerpAngles.get(rotation);
		
		//MatrixFromAngles(rotation, cent->lerpAngles[PITCH], cent->lerpAngles[YAW], cent->lerpAngles[ROLL]);
		//MatrixSetupTransformFromRotation(transform, rotation, cent->lerpOrigin);
		
		Matrix4f transform = new Matrix4f(rotation, lerpOrigin, 1);

		// draw body
		RenderUtils.renderCylinderShapeZ(transform, mins, maxs, color, ClientGame.getMedia().debugPlayerAABB_twoSided);
		
		// draw axis angle
		mins.set(0, -0.5f, -0.5f);
		maxs.set(10, 0.5f, 0.5f);
		
		AxisAngle4f axisAngle = new AxisAngle4f();
		axisAngle.set(lerpQuat);
		
		Vector3f axis = new Vector3f(axisAngle.x, axisAngle.y, axisAngle.z);
		
		Angle3f angles = new Angle3f(axis);
		//sangles.set(lerpQuat);
		angles.get(rotation);
		
		//axisAngle.x = 1;
		//axisAngle.y = 0;
		//axisAngle.z = 0;
		
		//axisAngle.x = currentState.apos.trDelta.x;
		//axisAngle.y = currentState.apos.trDelta.y;
		//axisAngle.z = currentState.apos.trDelta.z;
		axisAngle.angle = ClientGame.getTime() * 0.001f;
		
		//axisAngle.angle *= 2;
		//rotation.set(axisAngle);
		
		Quat4f quat = new Quat4f();
		quat.set(axisAngle);
		//rotation.set(quat);
		
		//quat.set(new Quat4f(currentState.apos.trDelta));
		//rotation.set(quat);
		
		//rotation.setIdentity();
		transform = new Matrix4f(rotation, lerpOrigin, 1);
		
		RenderUtils.renderBox(transform, mins, maxs, Color.Red, ClientGame.getMedia().debugPlayerAABB_twoSided);
	}
}
