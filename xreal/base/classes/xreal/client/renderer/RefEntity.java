package xreal.client.renderer;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;

import xreal.Color;

/**
 * Represents a refEntity_t
 * 
 * @author Robert Beckebans
 */
public class RefEntity {

	public RefEntityType reType;
	public int renderFX;

	public int hModel; // opaque type outside refresh

	// most recent data
	public Vector3f origin; // also used as MODEL_BEAM's "from"
	public Quat4f quat; // rotation
	public Vector3f scale;

	public Vector3f lightingOrigin; // so multi-part models can be lit
									// identically (RF_LIGHTING_ORIGIN)
	public float shadowPlane; // projection shadows go here, stencils go
								// slightly lower

	public int frame; // also used as MODEL_BEAM's diameter

	// previous data for frame interpolation
	public Vector3f oldOrigin; // also used as MODEL_BEAM's "to"
	public int oldFrame;
	public float lerp; // 0.0 = old, 1.0 = current

	// texturing

	/**
	 * inline skin index
	 */
	public int skinNum;

	/**
	 * NULL for default skin
	 */
	public int customSkin;

	/**
	 * use one image for the entire thing
	 */
	public int customMaterial;

	// misc

	/**
	 * colors used by rgbgen entity shaders
	 */
	public Color materialRGBA;
	public float materialTexCoordU; // texture coordinates used by tcMod entity
									// modifiers
	public float materialTexCoordV;

	/**
	 * subtracted from refdef time to control effect start times
	 */
	public float materialTime;

	// extra sprite information
	public float radius;
	public float rotation;

	// extra light interaction information
	public int noShadowID;

	// extra animation information
	public RefSkeleton skeleton;
}
