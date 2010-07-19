package xreal.client.game;

import javax.vecmath.Matrix3f;
import javax.vecmath.Matrix4f;
import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;

import com.bulletphysics.collision.dispatch.CollisionObject;

import xreal.Engine;
import xreal.client.EntityState;
import xreal.client.renderer.Polygon;
import xreal.client.renderer.Renderer;
import xreal.client.renderer.Vertex;

public class CEntity_PhysicsBox extends CEntity {
	
	public CEntity_PhysicsBox(EntityState es) {
		super(es);
		
		//Engine.println("CEntity_PhysicsBox(entity number = " + es.getNumber() + ")");
	}

	
	@Override
	public void addToRenderer() throws Exception 
	{
		super.addToRenderer();
		
		Polygon			poly = new Polygon();
		Vertex			verts[] = new Vertex[4];
		int             i;
		Vector3f        mins = new Vector3f();
		Vector3f        maxs = new Vector3f();
		float           extx, exty, extz;
		Point3f         corners[] = new Point3f[8];

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

		// get the extents (size)
		extx = maxs.x - mins.x;
		exty = maxs.y - mins.y;
		extz = maxs.z - mins.z;
		
		for(i = 0; i < verts.length; i++)
		{
			verts[i] = new Vertex();
		}
		
		for(i = 0; i < corners.length; i++)
		{
			corners[i] = new Point3f();
		}

		// set the polygon's texture coordinates
		verts[0].st.x = 0;
		verts[0].st.y = 0;
		verts[1].st.x = 0;
		verts[1].st.y = 1;
		verts[2].st.x = 1;
		verts[2].st.y = 1;
		verts[3].st.x = 1;
		verts[3].st.y = 0;

		// set the polygon's vertex colors

		switch (currentState.generic1)
		{
			case CollisionObject.ACTIVE_TAG:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.red = 1;
					verts[i].color.green = 1;
					verts[i].color.blue = 1;
					verts[i].color.alpha = 1;
				}
				break;

			case CollisionObject.ISLAND_SLEEPING:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.red = 0;
					verts[i].color.green = 1;
					verts[i].color.blue = 0;
					verts[i].color.alpha = 1;
				}
				break;

			case CollisionObject.WANTS_DEACTIVATION:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.red = 0;
					verts[i].color.green = 1;
					verts[i].color.blue = 1;
					verts[i].color.alpha = 1;
				}
				break;

			case CollisionObject.DISABLE_DEACTIVATION:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.red = 1;
					verts[i].color.green = 0;
					verts[i].color.blue = 0;
					verts[i].color.alpha = 1;
				}
				break;

			case CollisionObject.DISABLE_SIMULATION:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.red = 1;
					verts[i].color.green = 1;
					verts[i].color.blue = 0;
					verts[i].color.alpha = 1;
				}
				break;

			default:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.red = 1;
					verts[i].color.green = 0;
					verts[i].color.blue = 0;
					verts[i].color.alpha = 1;
				}
				break;
		}

		Matrix3f rotation = new Matrix3f();
		rotation.setIdentity();
		rotation.set(lerpQuat);
		//lerpAngles.get(rotation);
		
		//MatrixFromAngles(rotation, cent->lerpAngles[PITCH], cent->lerpAngles[YAW], cent->lerpAngles[ROLL]);
		//MatrixSetupTransformFromRotation(transform, rotation, cent->lerpOrigin);
		
		Matrix4f transform = new Matrix4f(rotation, lerpOrigin, 1);

		//VectorAdd(cent->lerpOrigin, maxs, corners[3]);
		corners[3] = new Point3f(maxs);

		corners[2] = new Point3f(corners[3]);
		corners[2].x -= extx;

		corners[1] = new Point3f(corners[2]);
		corners[1].y -= exty;

		corners[0] = new Point3f(corners[1]);
		corners[0].x += extx;

		for(i = 0; i < 4; i++)
		{
			corners[i + 4] = new Point3f(corners[i]);
			corners[i + 4].z -= extz;
		}

		for(i = 0; i < corners.length; i++)
		{
			transform.transform(corners[i]);
		}
		
		

		// top
		poly.hMaterial = ClientGame.getMedia().debugPlayerAABB;
		
		poly.vertices.add(verts[0]);
		poly.vertices.add(verts[1]);
		poly.vertices.add(verts[2]);
		poly.vertices.add(verts[3]);
		
		verts[0].pos = corners[0];
		verts[1].pos = corners[1];
		verts[2].pos = corners[2];
		verts[3].pos = corners[3];
		
		Renderer.addPolygonToScene(poly);

		// bottom
		verts[0].pos = corners[7];
		verts[1].pos = corners[6];
		verts[2].pos = corners[5];
		verts[3].pos = corners[4];
		
		Renderer.addPolygonToScene(poly);

		// top side
		poly.hMaterial = ClientGame.getMedia().debugPlayerAABB_twoSided;
		
		verts[0].pos = corners[3];
		verts[1].pos = corners[2];
		verts[2].pos = corners[6];
		verts[3].pos = corners[7];
		
		Renderer.addPolygonToScene(poly);

		// left side
		verts[0].pos = corners[2];
		verts[1].pos = corners[1];
		verts[2].pos = corners[5];
		verts[3].pos = corners[6];
		
		Renderer.addPolygonToScene(poly);

		// right side
		verts[0].pos = corners[0];
		verts[1].pos = corners[3];
		verts[2].pos = corners[7];
		verts[3].pos = corners[4];
		
		Renderer.addPolygonToScene(poly);

		// bottom side
		verts[0].pos = corners[1];
		verts[1].pos = corners[0];
		verts[2].pos = corners[4];
		verts[3].pos = corners[5];
		
		Renderer.addPolygonToScene(poly);
	}
}
