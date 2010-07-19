package xreal.client.game.util;

import javax.vecmath.Matrix4f;
import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;

import xreal.Color;
import xreal.client.renderer.Polygon;
import xreal.client.renderer.Renderer;
import xreal.client.renderer.Vertex;


/**
 * 
 * @author Robert Beckebans
 */
public class RenderUtils {

	public static void renderBox(Matrix4f transform, Vector3f mins, Vector3f maxs, Color color, int hMaterial) throws Exception
	{
		Polygon			poly = new Polygon();
		Vertex			verts[] = new Vertex[4];
		int             i;
		float           extx, exty, extz;
		Point3f         corners[] = new Point3f[8];
		
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
		for(i = 0; i < 4; i++)
		{
			verts[i].color.set(color);
		}

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
		
		poly.hMaterial = hMaterial;

		// top
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
	
	public static void renderCylinderShapeZ(Matrix4f transform, Vector3f mins, Vector3f maxs, Color color, int hMaterial) throws Exception
	{
		Polygon			poly = new Polygon();
		Vertex			verts[] = new Vertex[4];
		int             i;
		float           extx, exty, extz;
		Point3f         corners[] = new Point3f[4];
		
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
		for(i = 0; i < 4; i++)
		{
			verts[i].color.set(color);
		}
		
		
		Vector3f dir = new Vector3f(0, 0, 1);
		
		Vector3f p1 = new Vector3f(maxs.x, 0, 0);
		Vector3f p2 = new Vector3f(maxs.x, 0, 0);
		
		poly.hMaterial = hMaterial;
		
		poly.vertices.add(verts[0]);
		poly.vertices.add(verts[1]);
		poly.vertices.add(verts[2]);
		poly.vertices.add(verts[3]);
		
		for(float angle = 0; angle < 360; angle += 30)
		{
			
			//p1.rotatePointAroundVector(dir, angle);
			//p2.rotatePointAroundVector(dir, angle + 30);
			
			p1.x = (float) Math.cos(Math.toRadians(angle)) * maxs.x;
			p1.y = (float) Math.sin(Math.toRadians(angle)) * maxs.x;
			
			p2.x = (float) Math.cos(Math.toRadians(angle + 30)) * maxs.x;
			p2.y = (float) Math.sin(Math.toRadians(angle + 30)) * maxs.x;
			
			corners[0].set(p1);
			corners[0].z = maxs.z;
			
			corners[1].set(p2);
			corners[1].z = maxs.z;
			
			corners[2].set(p2);
			corners[2].z = mins.z;
			
			corners[3].set(p1);
			corners[3].z = mins.z;
			
			for(i = 0; i < 4; i++)
			{
				transform.transform(corners[i]);
			}
			
			// quad
			verts[0].pos = corners[0];
			verts[1].pos = corners[1];
			verts[2].pos = corners[2];
			verts[3].pos = corners[3];
			
			Renderer.addPolygonToScene(poly);
		}
	}
}
