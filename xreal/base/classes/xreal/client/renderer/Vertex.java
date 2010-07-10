package xreal.client.renderer;

import javax.vecmath.Color4f;
import javax.vecmath.Point3f;
import javax.vecmath.Vector2f;

import xreal.Color;


/**
 * Equivalent to polyVert_t
 * 
 * @author Robert Beckebans
 */
public class Vertex {
	public Point3f pos = new Point3f();
	public Vector2f st = new Vector2f();
	public Color	color = new Color();
}
