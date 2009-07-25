package xreal.client.renderer;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;

public class Camera {

	/**
	 * used for player configuration screen
	 */
	public static final int RDF_NOWORLDMODEL = 1;

	/**
	 * force renderer to use faster lighting only path
	 */
	public static final int RDF_NOSHADOWS = 2;

	/**
	 * teleportation effect
	 */
	public static final int RDF_HYPERSPACE = 4;

	/**
	 * don't use cubemaps
	 */
	public static final int RDF_NOCUBEMAP = 8;

	public static final int RDF_NOBLOOM = 16;

	/**
	 * enable automatic underwater caustics and fog
	 */
	public static final int RDF_UNDERWATER = 32;

	public int x, y, width, height;
	public float fovX, fovY;
	public Vector3f position;
	public Quat4f quat;

	/**
	 * time in milliseconds for shader effects and other time dependent
	 * rendering issues
	 */
	public int time;

	/**
	 * RDF_NOWORLDMODEL, etc
	 */
	public int rdflags;

	// 1 bits will prevent the associated area from rendering at all
	// byte areamask[MAX_MAP_AREA_BYTES];

	// text messages for deform text shaders
	// char text[MAX_RENDER_STRINGS][MAX_RENDER_STRING_LENGTH];
}
