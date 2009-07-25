package xreal.client.renderer;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;


/**
 * 
 * @author Robert Beckebans
 */
public class RefBone {
	
	public String name;
	public int parentIndex; // parent index (-1 if root)
	public Vector3f origin;
	public Quat4f rotation;
	
	public RefBone(String name, int parentIndex, Vector3f origin, Quat4f rotation) {
		super();
		this.name = name;
		this.parentIndex = parentIndex;
		this.origin = origin;
		this.rotation = rotation;
	}
}
