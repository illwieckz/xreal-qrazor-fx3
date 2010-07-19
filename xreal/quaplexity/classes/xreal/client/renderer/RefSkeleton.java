package xreal.client.renderer;

import javax.vecmath.Vector3f;

/**
 * 
 * @author Robert Beckebans
 */
public class RefSkeleton {
	private RefSkeletonType type;

	private RefBone bones[];

	// bounds of all applied animations
	public Vector3f mins = new Vector3f(); 
	public Vector3f maxs = new Vector3f();

	public Vector3f scale = new Vector3f(1, 1, 1);

	public RefSkeletonType getType() {
		return type;
	}

	public RefSkeleton(int type, RefBone[] bones, Vector3f mins, Vector3f maxs) {
		super();
		this.type = RefSkeletonType.values()[type];
		this.bones = bones;
		this.mins = mins;
		this.maxs = maxs;
	}

	public RefBone[] getBones() {
		return bones;
	}

	// TODO blend operations between skeletons
}
