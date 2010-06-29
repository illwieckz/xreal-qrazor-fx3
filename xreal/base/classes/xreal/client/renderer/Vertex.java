package xreal.client.renderer;

import javax.vecmath.Color4f;
import javax.vecmath.Point3f;
import javax.vecmath.Vector2f;


/**
 * Equivalent to polyVert_t
 * 
 * @author Robert Beckebans
 */
public class Vertex {
	public Point3f pos = new Point3f();
	public Vector2f st = new Vector2f();
	public Color4f	color = new Color4f();
}
