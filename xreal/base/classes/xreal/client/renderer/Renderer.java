package xreal.client.renderer;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;

import xreal.Color;

/**
 * 
 * @author Robert Beckebans
 */
public abstract class Renderer {
	
	/**
	 * Set the current renderer back end color.
	 */
	public synchronized static native void setColor(float red, float green, float blue, float alpha);
	
	public static void setColor(Color color) {
		setColor(color.red, color.green, color.blue, color.alpha);
	}
	
	/**
	 * Draw a single quad on the screen.
	 * @param x Vertical position of the upper left corner.
	 * @param y Horizontal position of the upper left corner.
	 * @param w Width
	 * @param h Height
	 * @param s1
	 * @param t1
	 * @param s2
	 * @param t2
	 * @param hShader The material.
	 */
	public synchronized static native void drawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, int hShader);
	
	
	/**
	 * Loads a TrueType font and returns its glyphs.
	 * 
	 * @param fontName The font name in fonts/ including the .ttf suffix.
	 * 
	 * @param pointSize We also need to adjust the scale based on point size relative to 48 points as the ui scaling is based on a 48 point font.
	 * 					The renderer will do: glyphScale *= 48.0f / pointSize;
	 * @return
	 */
	public synchronized static native Font registerFont(String fontName, int pointSize);
	
	/**
	 * Loads a material for 2D or 3D rendering if it's not already loaded.
	 * 
	 * @param name The material name in materials/*.mtr
	 * 
	 * @return The qhandle_t index if found, _default material if not.
	 */
	public synchronized static native int registerMaterial(String name);
	
	/**
	 * Loads a material for 2D rendering if it's not already loaded.
	 * 
	 * @param name The material name in materials/*.mtr
	 * 
	 * @return The qhandle_t index if found, _default material if not.
	 */
	public synchronized static native int registerMaterialNoMip(String name);
	
	/**
	 * Loads a material for real time lighting if it's not already loaded.
	 * This material defines the light volume and should be only used by RefLight objects.
	 * 
	 * @param name The material name in materials/*.mtr
	 * 
	 * @return The qhandle_t index if found, _default material if not.
	 */
	public synchronized static native int registerMaterialLightAttenuation(String name);
	
	
	/**
	 * Loads a 3D model if it's not already loaded.
	 * 
	 * @param name The model name with the .md3, .md5mesh or .psk suffix
	 * @param forceStatic Only relevant for .md3 models.
	 * 			If true it forces to ignore all keyframes by the md3 model and it will only generate a static VBO model for it.
	 * 
	 * @return Returns rgb axis if not found.
	 */
	public synchronized static native int registerModel(String name, boolean forceStatic);
	
	/**
	 * Loads a skeletal animation if it's not already loaded.
	 * 
	 * @param name The animation name with the .md5mesh or .psa suffix
	 * 
	 * @return Returns empty animation if not found,
	 * which can't be used for any further skeletal animation settings in combination with any skeletal model.
	 */
	public synchronized static native int registerAnimation(String name);
	
	/**
	 * Loads a skin if it's not already loaded.
	 * 
	 * @param name The skin name with the .skin suffix
	 * 
	 * @return Returns default skin if not found, which will just point to the first material entry of the model
	 */
	public synchronized static native int registerSkin(String name);
	
	
	public synchronized static native void loadWorldBsp(String name);
	
	
	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	
	public synchronized static native void clearScene();
	
	private synchronized static native void addRefEntityToScene(int reType, int renderfx, int hModel,
																float posX, float posY, float posZ,
																float quatX, float quatY, float quatZ, float quatW,
																float scaleX, float scaleY, float scaleZ,
																float lightPosX, float lightPosY, float lightPosZ,
																float shadowPlane,
																int frame,
																float oldPosX, float oldPosY, float oldPosZ,
																int oldFrame,
																float lerp,
																int skinNum, int customSkin, int customMaterial,
																float materialRed, float materialGreen, float materialBlue, float materialAlpha,
																float materialTexCoordU, float materialTexCoordV,
																float materialTime,
																float radius, float rotation,
																int noShadowID,
																boolean useSkeleton);
	
	private synchronized static native void setRefEntityBone(int boneIndex, String name, int parentIndex,
															float posX, float posY, float posZ,
															float quatX, float quatY, float quatZ, float quatW);
	
	private synchronized static native void setRefEntitySkeleton(	int type,
															int numBones,
															float minX, float minY, float minZ,
															float maxX, float maxY, float maxZ,
															float scaleX, float scaleY, float scaleZ);
	
	private static void setRefEntitySkeleton(RefSkeleton skel) {
		
		RefBone bones[] = skel.getBones();
		
		setRefEntitySkeleton(skel.getType().ordinal(),
				bones.length,
				skel.mins.x, skel.mins.y, skel.mins.z,
				skel.maxs.x, skel.maxs.y, skel.maxs.z,
				skel.scale.x, skel.scale.y, skel.scale.z);
		
		for(int i = 0; i < bones.length; i++) {
			RefBone b = bones[i];
			
			setRefEntityBone(i,
							b.name, b.parentIndex,
							b.origin.x, b.origin.y, b.origin.z,
							b.rotation.x, b.rotation.y, b.rotation.z, b.rotation.w);
		}
	}
	
	public static void addRefEntityToScene(RefEntity ent) {
		
		boolean useSkeleton = ent.skeleton != null && ent.skeleton.getType() == RefSkeletonType.ABSOLUTE;
		
		if(useSkeleton) {
			setRefEntitySkeleton(ent.skeleton);
		}
		
		addRefEntityToScene(ent.reType.ordinal(),
							ent.renderFX, 
							ent.hModel,
							
							ent.origin.x,
							ent.origin.y,
							ent.origin.z,
							
							ent.quat.x,
							ent.quat.y,
							ent.quat.z,
							ent.quat.w,
							
							ent.scale.x,
							ent.scale.y,
							ent.scale.z,
							
							ent.lightingOrigin.x,
							ent.lightingOrigin.y,
							ent.lightingOrigin.z,
							
							ent.shadowPlane,
							
							ent.frame,
							
							ent.oldOrigin.x,
							ent.oldOrigin.y,
							ent.oldOrigin.z,
							
							ent.oldFrame,
							ent.lerp,
							
							ent.skinNum, ent.customSkin, ent.customMaterial,
							
							ent.materialRGBA.red, ent.materialRGBA.green, ent.materialRGBA.blue, ent.materialRGBA.alpha,
							ent.materialTexCoordU, ent.materialTexCoordV,
							ent.materialTime,
							
							ent.radius, ent.rotation,
							ent.noShadowID,
							
							useSkeleton);
	}
	
	public synchronized static native RefSkeleton buildSkeleton(int hAnim, int startFrame, int endFrame, float frac, boolean clearOrigin);
	
	
	
	
	private synchronized static native void addPolygonToSceneBegin(int hMaterial, int numVertices);
	
	private synchronized static native void addPolygonVertexToScene(float posX, float posY, float posZ,
																	float texCoordU, float texCoordV,
																	float red, float green, float blue, float alpha);
	
	private synchronized static native void addPolygonToSceneEnd();
	
	public static void addPolygonToScene(Polygon poly) throws Exception {
		
		if(poly.hMaterial <= 0) {
			throw new Exception("WARNING: Renderer.addPolygonToScene: null poly material\n");
		}
		
		addPolygonToSceneBegin(poly.hMaterial, poly.vertices.size());
		
		for(Vertex v : poly.vertices) {
			
			addPolygonVertexToScene(v.pos.x, v.pos.y, v.pos.z,
									v.st.x, v.st.y,
									v.color.x, v.color.y, v.color.z, v.color.w);
		}
		
		addPolygonToSceneEnd();
	}
	
	
	private synchronized static native void renderScene(int viewPortX, int viewPortY, int viewPortWidth, int viewPortHeight,
														float fovX, float fovY,
														float posX, float posY, float posZ,
														float quatX, float quatY, float quatZ, float quatW,
														int time,
														int flags);
	
	public static void renderScene(Camera c) {
		renderScene(c.x, c.y, c.width, c.height, c.fovX, c.fovY, c.position.x, c.position.y, c.position.z, c.quat.x, c.quat.y, c.quat.z, c.quat.w, c.time, c.rdflags);
	}
	
	/*
	
	void            trap_R_AddRefEntityToScene(const refEntity_t * ent);
	void            trap_R_AddRefLightToScene(const refLight_t * light);

	// polys are intended for simple wall marks, not really for doing
	// significant construction
	void            trap_R_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t * verts);
	void            trap_R_AddPolysToScene(qhandle_t hShader, int numVerts, const polyVert_t * verts, int numPolys);
	void            trap_R_AddLightToScene(const vec3_t org, float intensity, float r, float g, float b);
	int             trap_R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir);
	void            trap_R_RenderScene(const refdef_t * fd);
	void            trap_R_SetColor(const float *rgba);	// NULL = 1,1,1,1
	void            trap_R_DrawStretchPic(float x, float y, float w, float h,
										  float s1, float t1, float s2, float t2, qhandle_t hShader);
	void            trap_R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs);
	int             trap_R_LerpTag(orientation_t * tag, clipHandle_t mod, int startFrame, int endFrame,
								   float frac, const char *tagName);
	int				trap_R_CheckSkeleton(refSkeleton_t * skel, qhandle_t hModel, qhandle_t hAnim);
	int             trap_R_BuildSkeleton(refSkeleton_t * skel, qhandle_t anim, int startFrame, int endFrame, float frac,
										 qboolean clearOrigin);
	int             trap_R_BlendSkeleton(refSkeleton_t * skel, const refSkeleton_t * blend, float frac);
	int             trap_R_BoneIndex(qhandle_t hModel, const char *boneName);
	int             trap_R_AnimNumFrames(qhandle_t hAnim);
	int             trap_R_AnimFrameRate(qhandle_t hAnim);

	void            trap_R_RemapShader(const char *oldShader, const char *newShader, const char *timeOffset);

	// The glConfig_t will not change during the life of a cgame.
	// If it needs to change, the entire cgame will be restarted, because
	// all the qhandle_t are then invalid.
	void            trap_GetGlconfig(glConfig_t * glconfig);
	*/
}
