package xreal.client.renderer;

/**
 * 
 * @author Robert Beckebans
 */
public abstract class Renderer {
	
	/**
	 * Set the current renderer back end color.
	 */
	public static native void setColor(float red, float green, float blue, float alpha);
	
	public static native void drawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, int hShader);
	
	/**
	 * Loads a material for 2D or 3D rendering if it's not already loaded.
	 * 
	 * @param name The material name in materials/*.mtr
	 * 
	 * @return The qhandle_t index if found, _default material if not.
	 */
	public static native int registerMaterial(String name);
	
	/**
	 * Loads a material for 2D rendering if it's not already loaded.
	 * 
	 * @param name The material name in materials/*.mtr
	 * 
	 * @return The qhandle_t index if found, _default material if not.
	 */
	public static native int registerMaterialNoMip(String name);
	
	/**
	 * Loads a material for real time lighting if it's not already loaded.
	 * This material defines the light volume and should be only used by RefLight objects.
	 * 
	 * @param name The material name in materials/*.mtr
	 * 
	 * @return The qhandle_t index if found, _default material if not.
	 */
	public static native int registerMaterialLightAttenuation(String name);
	
	
	/**
	 * Loads a 3D model if it's not already loaded.
	 * 
	 * @param name The model name with the .md3, .md5mesh or .psk suffix
	 * @param forceStatic Only relevant for .md3 models.
	 * 			If true it forces to ignore all keyframes by the md3 model and it will only generate a static VBO model for it.
	 * 
	 * @return Returns rgb axis if not found.
	 */
	public static native int registerModel(String name, boolean forceStatic);
	
	/**
	 * Loads a skeletal animation if it's not already loaded.
	 * 
	 * @param name The animation name with the .md5mesh or .psa suffix
	 * 
	 * @return Returns empty animation if not found,
	 * which can't be used for any further skeletal animation settings in combination with any skeletal model.
	 */
	public static native int registerAnimation(String name);
	
	/**
	 * Loads a skin if it's not already loaded.
	 * 
	 * @param name The skin name with the .skin suffix
	 * 
	 * @return Returns default skin if not found, which will just point to the first material entry of the model
	 */
	public static native int registerSkin(String name);
	
	/*
	void            trap_R_LoadWorldMap(const char *mapname);

	// all media should be registered during level startup to prevent
	// hitches during gameplay
	qhandle_t       trap_R_RegisterModel(const char *name, qboolean forceStatic);	// returns rgb axis if not found
	qhandle_t       trap_R_RegisterAnimation(const char *name);
	qhandle_t       trap_R_RegisterSkin(const char *name);	// returns all white if not found
	qhandle_t       trap_R_RegisterShader(const char *name);	// returns all white if not found
	qhandle_t       trap_R_RegisterShaderNoMip(const char *name);	// returns all white if not found
	qhandle_t       trap_R_RegisterShaderLightAttenuation(const char *name);

	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	void            trap_R_ClearScene(void);
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
