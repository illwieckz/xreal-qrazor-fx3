/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================


/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"
#include "r_backend.h"



static bool		rb_arrays_locked;
static double		rb_shader_time;

std::vector<r_table_t>	r_tables;

struct glmode_t
{
	char *name;
	int	minimize, maximize;
};

static glmode_t gl_modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};


void	RB_InitBackend()
{
	ri.Com_Printf("------- RB_InitBackend -------\n");
	
//	xglClearColor(0.0, 0.0, 0.0, 1.0);
	xglClearColor(0.3, 0.3, 0.3, 1.0);
	xglColor4fv(color_white);
	
	//xglEnable(GL_DEPTH_TEST);
	
	xglDisable(GL_TEXTURE_2D);

	//xglDisable(GL_DEPTH_TEST);
	//xglDisable(GL_CULL_FACE);
	//xglDisable(GL_BLEND);
	//xglDisable(GL_STENCIL_TEST);
	
	
	//xglShadeModel(GL_SMOOTH);
	gl_state.polygon_mode = GL_FILL;
        xglPolygonMode(GL_FRONT_AND_BACK, gl_state.polygon_mode);
	xglPolygonOffset(-1, -2);
		
	rb_arrays_locked = false;

	r_framecount = 1;
	r_visframecount = 1;
	r_lightframecount = 1;
	r_shadowframecount = 1;
	r_checkcount = 1;
	
	memset(gl_state.current_tmu_images, 0, sizeof(gl_state.current_tmu_images));
	memset(gl_state.current_tmu_mats, 0, sizeof(gl_state.current_tmu_mats));
	memset(gl_state.current_tmu_entities, 0, sizeof(gl_state.current_tmu_entities));
	memset(gl_state.current_tmu_lights, 0, sizeof(gl_state.current_tmu_lights));
	gl_state.current_tmu			= 0;
	
	gl_state.current_vbo_array_buffer	= 0;
	gl_state.current_vbo_vertexes_ofs	= 0;
	
	gl_state.matrix_quake_to_opengl.setupRotation   (1, 0, 0,-90);    	// put Z going up
	gl_state.matrix_quake_to_opengl.multiplyRotation(0, 0, 1, 90);		// put Z going up
	
//	ri.Com_DPrintf("quake2opengl matrix:\n%s\n", gl_state.matrix_quake_to_opengl.toString());
	
	r_scene_main.cmds_num			= 0;
	r_scene_main.cmds			= std::vector<r_command_t>(r_cmds_max->getInteger());
	r_scene_main.cmds_light_num		= 0;
	r_scene_main.cmds_light			= std::vector<r_command_t>(r_cmds_light_max->getInteger());
	r_scene_main.cmds_translucent_num	= 0;
	r_scene_main.cmds_translucent		= std::vector<r_command_t>(r_cmds_translucent_max->getInteger());
	
	r_scene_portal.cmds_num			= 0;
	r_scene_portal.cmds			= std::vector<r_command_t>(r_cmds_max->getInteger());
	r_scene_portal.cmds_light_num		= 0;
	r_scene_portal.cmds_light		= std::vector<r_command_t>(r_cmds_light_max->getInteger());
	r_scene_portal.cmds_translucent_num	= 0;
	r_scene_portal.cmds_translucent		= std::vector<r_command_t>(r_cmds_translucent_max->getInteger());
	
	RB_InitGPUShaders();
}

void	RB_ShutdownBackend()
{
	ri.Com_Printf("------- RB_ShutdownBackend -------\n");
	
	RB_ShutdownGPUShaders();
}

void	RB_BeginBackendFrame()
{
	// draw buffer
	if(r_drawbuffer->isModified())
	{
		r_drawbuffer->isModified(false);

		if(X_stricmp(r_drawbuffer->getString(), "GL_FRONT") == 0)
			xglDrawBuffer(GL_FRONT);
		else
			xglDrawBuffer(GL_BACK);
		
	}
	
	// texturemode
	if(r_texturemode->isModified())
	{
		RB_TextureMode(r_texturemode->getString());
		
		r_texturemode->isModified(false);
	}
	
	// polygonmode
	if(r_polygonmode->isModified())
	{
		r_polygonmode->isModified(false);
	
		if(X_strcaseequal(r_polygonmode->getString(), "GL_POINT"))
		{
			gl_state.polygon_mode = GL_POINT;
		}
		else if(X_strcaseequal(r_polygonmode->getString(), "GL_LINE"))
		{
			gl_state.polygon_mode = GL_LINE;
		}
		else
		{
			gl_state.polygon_mode = GL_FILL;
		}
			
		xglPolygonMode(GL_FRONT_AND_BACK, gl_state.polygon_mode);
	}

	c_leafs = 0;
	c_entities = 0;
	c_lights = 0;
	c_cmds = 0;
	c_cmds_radiosity = 0;
	c_cmds_light = 0;
	c_cmds_translucent = 0;
	c_cmds_fog = 0;
	c_cmds_postprocess = 0;
	c_triangles = 0;
	c_draws = 0;
	c_expressions = 0;
}

void	RB_EndBackendFrame()
{
	//TODO
}

void	RB_SetShaderTime(double time)
{
	rb_shader_time = time;
}

static void	RB_SetupViewPort(int x, int y, int w, int h)
{
#if 0
	int	x, x2, y2, y, w, h;

	x = floor(r_newrefdef.x * vid.width / vid.width);
	x2 = ceil((r_newrefdef.x + r_newrefdef.width) * vid.width / vid.width);
	y = floor(vid.height - r_newrefdef.y * vid.height / vid.height);
	y2 = ceil(vid.height - (r_newrefdef.y + r_newrefdef.height) * vid.height / vid.height);

	w = x2 - x;

	h = y - y2;

	xglViewport(x, y2, w, h);
#else
	gl_state.vrect_viewport.x		= x;
	gl_state.vrect_viewport.y		= y;	//vid.height - (y + h);
	gl_state.vrect_viewport.width		= w;
	gl_state.vrect_viewport.height	= h;
	
	xglViewport(gl_state.vrect_viewport.x, gl_state.vrect_viewport.y, gl_state.vrect_viewport.width, gl_state.vrect_viewport.height);
#endif
}

static void	RB_SetupOrthoProjectionMatrix()
{
	double l = 0.0;
	double r = vid.width;
		
	double b = vid.height;
	double t = 0;
	
	double f = 99999;
	double n = -f;

	matrix_c& m = gl_state.matrix_projection;
		
	m[0][0] = 2.0/(r-l);		m[0][1] = 0.0;			m[0][2] = 0.0;			m[0][3] =-(r+l)/(r-l);
	m[1][0] = 0.0;			m[1][1] = 2.0/(t-b);		m[1][2] = 0.0;			m[1][3] =-(t+b)/(t-b);
	m[2][0] = 0.0;			m[2][1] = 0.0;			m[2][2] =-(2.0)/(f-n);		m[2][3] =-(f+n)/(f-n);
	m[3][0] = 0.0;			m[3][1] = 0.0;			m[3][2] = 0.0;			m[3][3] = 1.0;

	xglMatrixMode(GL_PROJECTION);
	
	xglLoadTransposeMatrixfARB(&m[0][0]);
	
	xglMatrixMode(GL_MODELVIEW);
}



static void	RB_SetupPerspectiveProjectionMatrix(const r_refdef_t &refdef)
{
	if(r_znear->getValue() < 0.1)
		ri.Cvar_SetValue("r_znear", 4.0);
		
	if(r_zfar->getValue() <= r_znear->getValue())
		ri.Cvar_SetValue("r_zfar", 65536.0);
	
	double n = r_znear->getValue();
	double f = r_zfar->getValue();
	
	if(refdef.flip_x)
		std::swap<double>(n, f);
		
	double r = n * tan(refdef.fov_x * M_PI / 360.0);
	double l = -r;
	
	if(refdef.flip_y)
		std::swap<double>(l, r);
	
	double t;
	if(gl_state.active_pbuffer)
		t = n * tan(CalcFOV(refdef.fov_x, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger()) * M_PI / 360.0);
	else
		t = n * tan(refdef.fov_y * M_PI / 360.0);
	double b = -t;
	
	if(refdef.flip_z)
		std::swap<double>(b, t);
			
	matrix_c& m = gl_state.matrix_projection;
	
	RB_OpenGLFrustum(m, l, r, b, t, n, f);
	m.multiply(gl_state.matrix_quake_to_opengl);
	
	xglMatrixMode(GL_PROJECTION);
	xglLoadIdentity();

	xglLoadTransposeMatrixfARB(&m[0][0]);
	
	xglMatrixMode(GL_MODELVIEW);
}


static void	RB_SetupOrthoViewMatrix()
{
	gl_state.matrix_view.identity();
}


static void	RB_SetupViewMatrix(const vec3_c &origin, const vec3_c &angles)
{
	matrix_c m;//, m2;
	
	m.setupTranslation(origin);
	m.multiplyRotation(angles[PITCH], angles[YAW], angles[ROLL]);

//	m2.fromAngles(angles[PITCH], angles[YAW], angles[ROLL]);

//	quaternion_c q;
//	q.fromMatrix(m2);
	
//	q.fromAngles(angles[PITCH], angles[YAW], angles[ROLL]);
//	q.fromAngles(angles);
	
//	matrix_c m2;
//	m2.fromQuaternion(q);
//	m.multiply(m2);
	
	//q.multiplyRotation(0, 0, 1, angles[YAW]);
	//q.multiplyRotation(0, 1, 0,-angles[PITCH]);
	//q.multiplyRotation(1, 0, 0,-angles[ROLL]);
		
//	m.multiplyRotation(0, 0, 1, angles[YAW]);
//	m.multiplyRotation(0, 1, 0, angles[PITCH]);
//	m.multiplyRotation(1, 0, 0, angles[ROLL]);

//	m.multiplyRotation(q);
	
//	matrix_c m_z; m_z.setupZRotation(angles[YAW]);		m.multiply(m_z);
//	matrix_c m_y; m_y.setupYRotation(angles[PITCH]);	m.multiply(m_y);
//	matrix_c m_x; m_x.setupXRotation(angles[ROLL]);		m.multiply(m_x);

	gl_state.matrix_view = m.affineInverse();
}



static void 	RB_SetupFrustum()
{
	// http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf
	
	matrix_c& m = gl_state.matrix_model_view_projection;
//	matrix_c m = gl_state.matrix_projection * gl_state.matrix_view;

	// left
	r_frustum[FRUSTUM_LEFT]._normal[0]	=  m[3][0] + m[0][0];
	r_frustum[FRUSTUM_LEFT]._normal[1]	=  m[3][1] + m[0][1];
	r_frustum[FRUSTUM_LEFT]._normal[2]	=  m[3][2] + m[0][2];
	r_frustum[FRUSTUM_LEFT]._dist		=-(m[3][3] + m[0][3]);
	r_frustum[FRUSTUM_LEFT].normalize();
//	r_frustum[FRUSTUM_LEFT]._type	= PLANE_ANYZ;
	r_frustum[FRUSTUM_LEFT].setType();
	r_frustum[FRUSTUM_LEFT].setSignBits();
	
	// right
	r_frustum[FRUSTUM_RIGHT]._normal[0]	=  m[3][0] - m[0][0];
	r_frustum[FRUSTUM_RIGHT]._normal[1]	=  m[3][1] - m[0][1];
	r_frustum[FRUSTUM_RIGHT]._normal[2]	=  m[3][2] - m[0][2];
	r_frustum[FRUSTUM_RIGHT]._dist		=-(m[3][3] - m[0][3]);
	r_frustum[FRUSTUM_RIGHT].normalize();
//	r_frustum[FRUSTUM_RIGHT]._type	= PLANE_ANYZ;
	r_frustum[FRUSTUM_RIGHT].setType();
	r_frustum[FRUSTUM_RIGHT].setSignBits();
	
	// bottom
	r_frustum[FRUSTUM_BOTTOM]._normal[0]	=  m[3][0] + m[1][0];
	r_frustum[FRUSTUM_BOTTOM]._normal[1]	=  m[3][1] + m[1][1];
	r_frustum[FRUSTUM_BOTTOM]._normal[2]	=  m[3][2] + m[1][2];
	r_frustum[FRUSTUM_BOTTOM]._dist		=-(m[3][3] + m[1][3]);
	r_frustum[FRUSTUM_BOTTOM].normalize();
//	r_frustum[FRUSTUM_BOTTOM]._type	= PLANE_ANYZ;
	r_frustum[FRUSTUM_BOTTOM].setType();
	r_frustum[FRUSTUM_BOTTOM].setSignBits();
	
	// top
	r_frustum[FRUSTUM_TOP]._normal[0]	=  m[3][0] - m[1][0];
	r_frustum[FRUSTUM_TOP]._normal[1]	=  m[3][1] - m[1][1];
	r_frustum[FRUSTUM_TOP]._normal[2]	=  m[3][2] - m[1][2];
	r_frustum[FRUSTUM_TOP]._dist		=-(m[3][3] - m[1][3]);
	r_frustum[FRUSTUM_TOP].normalize();
//	r_frustum[FRUSTUM_TOP]._type	= PLANE_ANYZ;
	r_frustum[FRUSTUM_TOP].setType();
	r_frustum[FRUSTUM_TOP].setSignBits();
	
#if 1
	// near
	r_frustum[FRUSTUM_NEAR]._normal[0]	=  m[3][0] + m[2][0];
	r_frustum[FRUSTUM_NEAR]._normal[1]	=  m[3][1] + m[2][1];
	r_frustum[FRUSTUM_NEAR]._normal[2]	=  m[3][2] + m[2][2];
	r_frustum[FRUSTUM_NEAR]._dist		=-(m[3][3] + m[2][3]);
	r_frustum[FRUSTUM_NEAR].normalize();
//	r_frustum[FRUSTUM_NEAR]._type	= PLANE_ANYZ;
	r_frustum[FRUSTUM_NEAR].setType();
	r_frustum[FRUSTUM_NEAR].setSignBits();
#endif
	
#if 0
	// far
	r_frustum[FRUSTUM_FAR]._normal[0]	=  m[3][0] - m[2][0];
	r_frustum[FRUSTUM_FAR]._normal[1]	=  m[3][1] - m[2][1];
	r_frustum[FRUSTUM_FAR]._normal[2]	=  m[3][2] - m[2][2];
	r_frustum[FRUSTUM_FAR]._dist		=-(m[3][3] - m[2][3]);
	r_frustum[FRUSTUM_FAR].normalize();
//	r_frustum[FRUSTUM_FAR]._type	= PLANE_ANYZ;
	r_frustum[FRUSTUM_FAR].setType();
	r_frustum[FRUSTUM_FAR].setSignBits();
#endif
}


void	RB_OpenGLFrustum(matrix_c &m, double l, double r, double b, double t, double n, double f)
{
	m[0][0] = (2.0*n)/(r-l);	m[0][1] = 0.0;			m[0][2] = (r+l)/(r-l);		m[0][3] = 0.0;
	m[1][0] = 0.0;			m[1][1] = (2.0*n)/(t-b);	m[1][2] = (t+b)/(t-b);		m[1][3] = 0.0;
	m[2][0] = 0.0;			m[2][1] = 0.0;			m[2][2] =-(f+n)/(f-n);		m[2][3] =-(2.0*f*n)/(f-n);
	m[3][0] = 0.0;			m[3][1] = 0.0;			m[3][2] =-1.0;			m[3][3] = 0.0;
}

void	RB_QuakeFrustum(matrix_c &m, double l, double r, double b, double t, double n, double f)
{
	// QRazorFX specific: Quake coordinate system
	m[0][0] = (f+n)/(f-n);		m[0][1] = 0.0;			m[0][2] = (f+n)/(f-n);		m[0][3] = 0.0;
	m[1][0] = 0.0;			m[1][1] = (2.0*n)/(r-l);	m[1][2] = (r+l)/(r-l);		m[1][3] = 0.0;
	m[2][0] = 0.0;			m[2][1] = 0.0;			m[2][2] = (2.0*n)/(t-b);	m[2][3] =-(2.0*f*n)/(t-b);
	m[3][0] = 0.0;			m[3][1] = 0.0;			m[3][2] =-1.0;			m[3][3] = 0.0;
}

void	RB_SetupModelviewMatrix(const matrix_c &m, bool force)
{
//	if(force || !(m == gl_state.matrix_model))
	{
		xglMatrixMode(GL_MODELVIEW);
		
		gl_state.matrix_model = m;
		
		gl_state.matrix_model_view = gl_state.matrix_view * gl_state.matrix_model;
		
		// load the final modelview matrix 
		xglLoadTransposeMatrixfARB(&gl_state.matrix_model_view[0][0]);
		
		gl_state.matrix_model_view_projection = gl_state.matrix_projection * gl_state.matrix_model_view;
	}
}

void	RB_SetupLightviewMatrix(const matrix_c &m)
{
	/*
	if(!(m == gl_state.matrix_light))
	{
		//xglMatrixMode(GL_MODELVIEW);
		
		gl_state.matrix_light = m;
		
		rb_lightview_matrix = gl_state.matrix_view * gl_state.matrix_light.affineInverse();
		//gl_state.matrix_view.copyTranspose(gl_state.matrix_model_view_gl);
		
		// load the final modelview matrix 
		//xglLoadMatrixf(&gl_state.matrix_model_view_gl[0][0]);
	}
	*/
}

void	RB_SetupGL2D()
{
	gl_state.is2d = true;

	if(gl_state.active_pbuffer)
		RB_SetupViewPort(0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger());
	else
		RB_SetupViewPort(0, 0, vid.width, vid.height);
	
	RB_SetupOrthoProjectionMatrix();
	
	RB_SetupOrthoViewMatrix();
	
	RB_SetupModelviewMatrix(matrix_identity, true);
	

	xglDisable(GL_SCISSOR_TEST);
	
	xglDisable(GL_DEPTH_TEST);
	//xglDepthMask(GL_FALSE);
	
	xglDisable(GL_CULL_FACE);
	
	xglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void	RB_SetupGL3D()
{
	gl_state.is2d = false;
	
	if(gl_state.active_pbuffer)
		RB_SetupViewPort(0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger());
	else
		RB_SetupViewPort(r_newrefdef.x, r_newrefdef.y, r_newrefdef.width, r_newrefdef.height);
	
	RB_SetupPerspectiveProjectionMatrix(r_newrefdef);
	
	RB_SetupViewMatrix(r_origin, r_newrefdef.view_angles);
	
	RB_SetupModelviewMatrix(matrix_identity, true);
	
	RB_SetupFrustum();
	
	if(r_portal_view || r_mirror_view)
	{
		GLdouble clip[4];

		clip[0] = r_clipplane._normal[0];
		clip[1] = r_clipplane._normal[1];
		clip[2] = r_clipplane._normal[2];
		clip[3] =-r_clipplane._dist;

		xglClipPlane(GL_CLIP_PLANE0, clip);
		xglEnable(GL_CLIP_PLANE0);
	}

	xglEnable(GL_SCISSOR_TEST);
	xglScissor(gl_state.vrect_viewport.x, gl_state.vrect_viewport.y, gl_state.vrect_viewport.width, gl_state.vrect_viewport.height);
		
	if(r_newrefdef.flip_y ^ r_newrefdef.flip_z)
		xglFrontFace(GL_CW);
	else
		xglFrontFace(GL_CCW);
	
	xglEnable(GL_DEPTH_TEST);
	xglDepthFunc(GL_LEQUAL);
	xglDepthMask(GL_TRUE);
	
	xglEnable(GL_CULL_FACE);
	xglCullFace(GL_FRONT);
	
	xglPolygonMode(GL_FRONT_AND_BACK, gl_state.polygon_mode);
}


void 	RB_Clear()
{
#if 0
	if(r_clear->getInteger())
		xglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	else
		xglClear(GL_DEPTH_BUFFER_BIT);
		
	if(r_shadows->getInteger() == 1)
	{
		xglClearStencil(128);

		xglClear(GL_STENCIL_BUFFER_BIT);
	}
#else
	xglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#endif
	
	r_depthmin = 0;
	r_depthmax = 1;
	
	xglDepthRange(r_depthmin, r_depthmax);
}

void	RB_RenderLightScale()
{
	if(r_lightscale->getInteger() < 1.0)
		return;

	xglPushAttrib(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	RB_SetupGL2D();

	xglDisable(GL_STENCIL_TEST);
	
	xglDepthMask(GL_FALSE);
	
	xglEnable(GL_BLEND);
	xglBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	
	float scale = 0.5 * r_lightscale->getInteger();
	
	X_clamp(scale, 0.0, 1.0);
	
	vec4_c color(scale, scale, scale, 0.5);
	
	R_DrawFill(0, 0, vid.width, vid.height, color);
	
	xglDisable(GL_BLEND);
		
	xglPopAttrib();
	
	RB_SetupGL3D();
}


void 	RB_SelectTexture(GLenum texture)
{
	uint_t tmu;

	if(!gl_config.arb_multitexture)
		return;

	tmu = texture - GL_TEXTURE0_ARB;
	
	if(tmu == gl_state.current_tmu)
		return;
	else
		gl_state.current_tmu = tmu;
	
	xglActiveTextureARB(texture);
	xglClientActiveTextureARB(texture);
}

void 	RB_TexEnv(GLenum mode)
{
	static int lastmodes[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

	if((int)mode != lastmodes[gl_state.current_tmu])
	{
		xglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
		lastmodes[gl_state.current_tmu] = mode;
	}
}

void 	RB_Bind(r_image_c *image)
{
	if(!image)
	{
		ri.Com_Error(ERR_FATAL, "RB_Bind: NULL");
		assert(image);
	}
	
	image->bind();
}

void 	RB_TextureMode(const std::string &string)
{
	int		i;
	r_image_c	*image;

	if(!string.length())
	{
		ri.Com_Printf ("RB_TextureMode: NULL parameter\n");
		return;
	}

	for(i=0; i< X_asz(gl_modes); i++)

	{
		if(!X_stricmp(gl_modes[i].name, string.c_str()))
			break;
	}

	if(i == X_asz(gl_modes))
	{
		ri.Com_Printf ("RB_TextureMode: bad filter name %s\n", string.c_str());
		return;
	}

	r_filter_min = gl_modes[i].minimize;
	r_filter_max = gl_modes[i].maximize;


	// change all the existing mipmap texture objects
	for(std::vector<r_image_c*>::const_iterator ir = r_images.begin(); ir != r_images.end(); ++ir)
	{
		image = *ir;
		
		if(!image)
			continue;
			
		if(r_arb_texture_rectangle->getInteger())
		{
			if(image->getTarget() == GL_TEXTURE_RECTANGLE_ARB)
				continue;
		}
		
		if(image->hasFlags(IMAGE_NOMIPMAP) || (image->getTarget() == GL_TEXTURE_CUBE_MAP_ARB) || image->hasVideo())
			continue;
			
		RB_Bind(image);
			
		xglTexParameteri(image->getTarget(), GL_TEXTURE_MIN_FILTER, r_filter_min);
		xglTexParameteri(image->getTarget(), GL_TEXTURE_MAG_FILTER, r_filter_max);
	}
}


void	RB_PushMesh(r_mesh_c *mesh, bool create_vbo_array_buffer, bool create_vbo_element_array_buffer)
{
	if(gl_config.arb_vertex_buffer_object)
	{	
		//
		// create element array buffer
		//
		if(!mesh->vbo_element_array_buffer && create_vbo_element_array_buffer)
		{
			index_t* indexes = (index_t*)Com_Alloc(mesh->indexes.size() * sizeof(index_t));
			index_t* index_ptr = indexes;
		
			for(std::vector<index_t>::const_iterator ir = mesh->indexes.begin(); ir != mesh->indexes.end(); ++ir)
			{
				*index_ptr = *ir;
				index_ptr++;
			}
		
			xglGenBuffersARB(1, &mesh->vbo_element_array_buffer);
			xglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->vbo_element_array_buffer);
			xglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->indexes.size() * sizeof(index_t), indexes, GL_STATIC_DRAW_ARB);
			
			Com_Free(indexes);
		}
		
		
		//
		// create array buffer
		//
		if(!mesh->vbo_array_buffer && create_vbo_array_buffer)
		{
			byte*		data = NULL;
			int		data_ofs = 0;
			int		data_size;
		
			data_size = mesh->vertexes.size() * ((sizeof(vec3_c)*4) + sizeof(vec2_c));
			data = (byte*)Com_Alloc(data_size);
			
			mesh->vbo_vertexes_ofs = data_ofs;
			for(std::vector<vec3_c>::const_iterator ir = mesh->vertexes.begin(); ir != mesh->vertexes.end(); ++ir)
			{
				memcpy(data + data_ofs, (float*)*ir, sizeof(vec3_c));
				data_ofs += sizeof(vec3_c);
			}
		
			mesh->vbo_texcoords_ofs = data_ofs;
			for(std::vector<vec2_c>::const_iterator ir = mesh->texcoords.begin(); ir != mesh->texcoords.end(); ++ir)
			{
				memcpy(data + data_ofs, *ir, sizeof(vec2_c));
				data_ofs += sizeof(vec2_c);
			}
		
			mesh->vbo_tangents_ofs = data_ofs;
			for(std::vector<vec3_c>::const_iterator ir = mesh->tangents.begin(); ir != mesh->tangents.end(); ++ir)
			{
				memcpy(data + data_ofs, (float*)*ir, sizeof(vec3_c));
				data_ofs += sizeof(vec3_c);
			}
			
			mesh->vbo_binormals_ofs = data_ofs;
			for(std::vector<vec3_c>::const_iterator ir = mesh->binormals.begin(); ir != mesh->binormals.end(); ++ir)
			{
				memcpy(data + data_ofs, (float*)*ir, sizeof(vec3_c));
				data_ofs += sizeof(vec3_c);
			}
		
			mesh->vbo_normals_ofs = data_ofs;
			for(std::vector<vec3_c>::const_iterator ir = mesh->normals.begin(); ir != mesh->normals.end(); ++ir)
			{
				memcpy(data + data_ofs, (float*)*ir, sizeof(vec3_c));
				data_ofs += sizeof(vec3_c);
			}
			
			if(data_ofs != data_size)
			{
				ri.Com_Error(ERR_FATAL, "RB_PushMesh: data ofs != data size");
				return;
			}
			
			xglGenBuffersARB(1, (GLuint*)&mesh->vbo_array_buffer);
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vbo_array_buffer);
        		xglBufferDataARB(GL_ARRAY_BUFFER_ARB, data_size, data, GL_STATIC_DRAW_ARB);
		
			Com_Free(data);
		}
	}
}


void	RB_FlushMesh(const r_command_t *cmd)
{
	const r_mesh_c* mesh = cmd->getEntityMesh();
	const std::vector<index_t>* light_indexes = cmd->getLightIndexes();

	if(gl_config.arb_vertex_buffer_object && gl_config.ext_draw_range_elements && mesh->vbo_element_array_buffer && !light_indexes)
	{
		xglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->vbo_element_array_buffer);
	
		xglDrawRangeElementsEXT(GL_TRIANGLES, 0, mesh->vertexes.size(), mesh->indexes.size(), GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(mesh->vbo_indexes_ofs));
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	
		if(light_indexes)
		{
			xglDrawElements(GL_TRIANGLES, light_indexes->size(), GL_UNSIGNED_INT, &((*light_indexes)[0]));
			c_triangles += light_indexes->size() / 3;
		}
		else
		{
			xglDrawElements(GL_TRIANGLES, mesh->indexes.size(), GL_UNSIGNED_INT, &(mesh->indexes[0]));
			c_triangles += mesh->indexes.size() / 3;
		}
	}
		
	c_draws++;
}

/*
void	RB_LockArrays(int vertexes_num)
{
	if(rb_arrays_locked)
		return;

	if(gl_config.ext_compiled_vertex_array)
	{
		xglLockArraysEXT(0, vertexes_num);
		rb_arrays_locked = true;
	}
}
*/

/*
void	RB_UnlockArrays()
{
	if(!rb_arrays_locked)
		return;

	if(gl_config.ext_compiled_vertex_array)
	{
		xglUnlockArraysEXT();
		rb_arrays_locked = false;
	}
}
*/

void	RB_EnableShaderStates(const r_shader_c *shader)
{
	// check culling
	if(shader->hasFlags(SHADER_TWOSIDED))
	{
		xglDisable(GL_CULL_FACE);
	}

	// check polygonOffset
	if(shader->hasFlags(SHADER_POLYGONOFFSET))
	{
		xglEnable(GL_POLYGON_OFFSET_FILL);
		xglPolygonOffset(-1, -2);
	}
}

void	RB_DisableShaderStates(const r_shader_c *shader)
{
	// check culling
	if(shader->hasFlags(SHADER_TWOSIDED))
	{
		xglEnable(GL_CULL_FACE);
		xglCullFace(GL_FRONT);
	}

	// check polygonOffset
	if(shader->hasFlags(SHADER_POLYGONOFFSET))
		xglDisable(GL_POLYGON_OFFSET_FILL);
}



void	RB_EnableShaderStageStates(const r_entity_c *ent, const r_shader_stage_c *stage)
{
	// check blending
	if(stage->flags & SHADER_STAGE_BLEND)
	{
		xglEnable(GL_BLEND);
		xglBlendFunc(stage->blend_src, stage->blend_dst);
	}
	
	// check alpha funcs
	if(stage->flags & SHADER_STAGE_ALPHATEST)
	{
		xglEnable(GL_ALPHA_TEST);
		
		float	value = X_bound(0.0f, RB_Evaluate(ent->getShared(), stage->alpha_ref, 0.5), 1.0f);
		
		xglAlphaFunc(GL_GREATER, value);
	}
	
	// check masking
	if(stage->flags & SHADER_STAGE_MASKALPHA)
	{
		xglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	}
	else if(stage->flags & SHADER_STAGE_MASKCOLOR)
	{
		xglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	}
	
	// check depth write
	if(stage->flags & SHADER_STAGE_MASKDEPTH)
	{
		xglDepthMask(GL_FALSE);
	}
}


void	RB_DisableShaderStageStates(const r_entity_c *ent, const r_shader_stage_c *stage)
{
	// check blending
	if(stage->flags & SHADER_STAGE_BLEND)
	{
		xglDisable(GL_BLEND);
	}
	
	// check alpha funcs
	if(stage->flags & SHADER_STAGE_ALPHATEST)
	{
		xglDisable(GL_ALPHA_TEST);
	}
	
	// check masking
	if(stage->flags & (SHADER_STAGE_MASKALPHA | SHADER_STAGE_MASKCOLOR))
	{
		xglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	
	// check depth write
	if(stage->flags & SHADER_STAGE_MASKDEPTH)
	{
		xglDepthFunc(GL_LEQUAL);
		xglDepthMask(GL_TRUE);
	}
}


static float	RB_EvalExpression(const r_entity_t &shared, boost::spirit::tree_match<r_iterator_t, r_factory_t>::const_tree_iterator const & i)
{
	c_expressions++;

	//ri.Com_Printf("RB_EvalExpression: i->value = %s i->children.size() = %i\n", std::string(i->value.begin(), i->value.end()).c_str(), i->children.size());

	if(i->value.id() == SHADER_GENERIC_RULE_REAL)
	{
		if(i->children.size() != 0)
			ri.Com_Error(ERR_DROP, "RB_EvalExpression: rule 'GENERIC_REAL' has %i children = '%s'", i->children.size(), std::string(i->value.begin(), i->value.end()).c_str());
				
		return i->value.value();
	}
	else if(i->value.id() == SHADER_GENERIC_RULE_PARM)
	{
		assert(i->children.size() == 0);
		
		switch((r_shader_parms_e)i->value.value())
		{
			case SHADER_PARM_TIME:
				return rb_shader_time;
			
			case SHADER_PARM_PARM0:
				return shared.shader_parms[0];
				
			case SHADER_PARM_PARM1:
				return shared.shader_parms[1];
				
			case SHADER_PARM_PARM2:
				return shared.shader_parms[2];
			
			case SHADER_PARM_PARM3:
				return shared.shader_parms[3];
				
			case SHADER_PARM_PARM4:
				return shared.shader_parms[4];
				
			case SHADER_PARM_PARM5:
				return shared.shader_parms[5];
				
			case SHADER_PARM_PARM6:
				return shared.shader_parms[6];
				
			case SHADER_PARM_PARM7:
				return shared.shader_parms[7];
				
			case SHADER_PARM_GLOBAL0:
				return 1.0;	//TODO
			
			case SHADER_PARM_GLOBAL1:
				return rb_shader_time - floorf(rb_shader_time);	//TODO
				
			case SHADER_PARM_GLOBAL2:
				return 1.0;	//TODO
				
			case SHADER_PARM_SOUND:
				return 0.5;	//shared.shader_sound;
				
			case SHADER_PARM_FRANDOM:
				return X_frand();
				
			case SHADER_PARM_CRANDOM:
				return X_crand();
				
			default:
				ri.Com_Error(ERR_FATAL, "RB_EvalExpression: unknown parm '%s'\n", std::string(i->value.begin(), i->value.end()).c_str());
				return 0;
		}
	}
	else if(i->value.id() == SHADER_GENERIC_RULE_TABLE_EVAL)
	{
		/*
		const r_table_t& table = r_tables[(uint_t)RB_EvalExpression(shared, i->children.begin())];
		const std::vector<float>& values = table.values;
		
		float idx_value = RB_EvalExpression(shared, i->children.begin()+1);
		
		uint_t idx_int = (uint_t)((float)floorf(values.size() * idx_value)) % values.size();
		
		return values[idx_int];
		*/
		
		const r_table_t& table = r_tables[(uint_t)RB_EvalExpression(shared, i->children.begin())];
		const std::vector<float>& values = table.values;
		
		const int num_values = (int)values.size();
		float index = RB_EvalExpression(shared, i->children.begin()+1) * num_values; // float index into the table´s elements
		float frac = floorf(index);	// integer index
		
		int idx = (int)frac;
		frac = index - frac;	// being inbetween two elements of the table
		int nextidx = idx+1;
		
		if(table.flags & SHADER_TABLE_CLAMP)
		{
			// clamp indices to table-range
			X_clamp(idx, 0, num_values-1);
			
			X_clamp(nextidx, 0, num_values-1);
		}
		else
		{
			// wrap around indices
			idx %= num_values;
			nextidx %= num_values;
		}
		
		if(table.flags & SHADER_TABLE_SNAP)
		{
			return values[idx];
		}
		
		return values[nextidx]*frac + values[idx]*(1.0f-frac);	// linearily interpolate
	}
	else if(i->value.id() == SHADER_GENERIC_RULE_TABLE_INDEX)
	{		
		return i->value.value();
		
	}
	else if(i->value.id() == SHADER_GENERIC_RULE_TABLE_VALUE)
	{	
		return RB_EvalExpression(shared, i->children.begin());
	}
	else if(i->value.id() == SHADER_GENERIC_RULE_FACTOR)
	{
		if(*i->value.begin() == '+')
		{
			assert(i->children.size() == 1);
			return RB_EvalExpression(shared, i->children.begin());
		}
		else if(*i->value.begin() == '-')
		{
			assert(i->children.size() == 1);
			return - RB_EvalExpression(shared, i->children.begin());
		}
		else
		{
			ri.Com_Error(ERR_FATAL, "RB_EvalExpression: unknown factor '%s'\n", *i->value.begin());
		}
	}
	else if(i->value.id() == SHADER_GENERIC_RULE_TERM)
	{
		if(*i->value.begin() == '*')
		{
			assert(i->children.size() == 2);
			return RB_EvalExpression(shared, i->children.begin()) * RB_EvalExpression(shared, i->children.begin()+1);
		}
		else if(*i->value.begin() == '/')
		{
			assert(i->children.size() == 2);
			return RB_EvalExpression(shared, i->children.begin()) / RB_EvalExpression(shared, i->children.begin()+1);
		}
		else if(*i->value.begin() == '%')
		{
			assert(i->children.size() == 2);
			return static_cast<int>(RB_EvalExpression(shared, i->children.begin())) % static_cast<int>(RB_EvalExpression(shared, i->children.begin()+1));
		}
		else if(*i->value.begin() == '<' && *(i->value.begin()+1) == '=')
		{
			assert(i->children.size() == 2);
			return (RB_EvalExpression(shared, i->children.begin()) <= RB_EvalExpression(shared, i->children.begin()+1)) ? 1.0 : 0.0;
		}
		else if(*i->value.begin() == '<')
		{
			assert(i->children.size() == 2);
			return (RB_EvalExpression(shared, i->children.begin()) < RB_EvalExpression(shared, i->children.begin()+1)) ? 1.0 : 0.0;
		}
		else if(*i->value.begin() == '>' && *(i->value.begin()+1) == '=')
		{
			assert(i->children.size() == 2);
			return (RB_EvalExpression(shared, i->children.begin()) >= RB_EvalExpression(shared, i->children.begin()+1)) ? 1.0 : 0.0;
		}
		else if(*i->value.begin() == '>')
		{
			assert(i->children.size() == 2);
			return (RB_EvalExpression(shared, i->children.begin()) > RB_EvalExpression(shared, i->children.begin()+1)) ? 1.0 : 0.0;
		}
		else if(*i->value.begin() == '=' && *(i->value.begin()+1) == '=')
		{
			assert(i->children.size() == 2);
			return (RB_EvalExpression(shared, i->children.begin()) == RB_EvalExpression(shared, i->children.begin()+1)) ? 1.0 : 0.0;
		}
		else if(*i->value.begin() == '!' && *(i->value.begin()+1) == '=')
		{
			assert(i->children.size() == 2);
			return (RB_EvalExpression(shared, i->children.begin()) != RB_EvalExpression(shared, i->children.begin()+1)) ? 1.0 : 0.0;
		}
		else if(*i->value.begin() == '&' && *(i->value.begin()+1) == '&')
		{
			assert(i->children.size() == 2);
			return (RB_EvalExpression(shared, i->children.begin()) && RB_EvalExpression(shared, i->children.begin()+1)) ? 1.0 : 0.0;
		}
		else if(*i->value.begin() == '|' && *(i->value.begin()+1) == '|')
		{
			assert(i->children.size() == 2);
			return (RB_EvalExpression(shared, i->children.begin()) || RB_EvalExpression(shared, i->children.begin()+1)) ? 1.0 : 0.0;
		}
		else
		{
			ri.Com_Error(ERR_FATAL, "RB_EvalExpression: unknown term '%s'\n", *i->value.begin());
		}
	}	
	else if(i->value.id() == SHADER_GENERIC_RULE_EXPRESSION)
	{
		if(*i->value.begin() == '+')
		{
			assert(i->children.size() == 2);
			return RB_EvalExpression(shared, i->children.begin()) + RB_EvalExpression(shared, i->children.begin()+1);
		}
		else if(*i->value.begin() == '-')
		{
			assert(i->children.size() == 2);
			return RB_EvalExpression(shared, i->children.begin()) - RB_EvalExpression(shared, i->children.begin()+1);
		}
		else
		{
			ri.Com_Error(ERR_FATAL, "RB_EvalExpression: unknown expression '%s'\n", *i->value.begin());
		}
	}
	else
	{
		ri.Com_Error(ERR_FATAL, "RB_EvalExpression: unknown id");
	}
	
	return 0;
}

float	RB_Evaluate(const r_entity_t &shared, const boost::spirit::tree_parse_info<r_iterator_t, r_factory_t> &info, float default_value)
{
	if(info.full && r_evalexpressions->getInteger())
		return RB_EvalExpression(shared, info.trees.begin());
	else
		return default_value;
}


static bool	RB_SetupTCModMatrix(const r_entity_t &shared, const r_shader_stage_c *stage, matrix_c &m)
{
	m = matrix_identity;
	
	if(stage->tcmod_cmds.empty())
		return false;
	
	vec_t x;
	vec_t y;
	vec_t z;
	
	for(std::vector<r_tcmod_t>::const_iterator ir = stage->tcmod_cmds.begin(); ir != stage->tcmod_cmds.end(); ++ir)
	{
		const r_tcmod_t& tcmod = *ir;
	
		switch(tcmod.type)
		{
			case SHADER_TCMOD_SCALE:
				x = RB_Evaluate(shared, tcmod.x, 1.0);
				y = RB_Evaluate(shared, tcmod.y, 1.0);
				
				m.multiplyScale(x, y, 1.0);
				break;
			
			case SHADER_TCMOD_SCALE3D:
				x = RB_Evaluate(shared, tcmod.x, 1.0);
				y = RB_Evaluate(shared, tcmod.y, 1.0);
				z = RB_Evaluate(shared, tcmod.y, 1.0);
				
				m.multiplyScale(x, y, z);
				break;
				
			case SHADER_TCMOD_CENTERSCALE:		
				x = RB_Evaluate(shared, tcmod.x, 1.0);
				y = RB_Evaluate(shared, tcmod.y, 1.0);
				
				m.multiplyTranslation( 0.5, 0.5, 0.0);
				m.multiplyScale(x, y, 1.0);
				m.multiplyTranslation(-0.5,-0.5, 0.0);
				break;
				
			case SHADER_TCMOD_SCROLL:					
				x = RB_Evaluate(shared, tcmod.x, 0.0);
				y = RB_Evaluate(shared, tcmod.y, 0.0);
				
				m.multiplyTranslation(x, y, 0.0);
				break;
				
			case SHADER_TCMOD_ROTATE:				
				x = RADTODEG(RB_Evaluate(shared, tcmod.x, 0.0));
				
				m.multiplyTranslation( 0.5, 0.5, 0);
				m.multiplyRotation(0, 0, 1, x);
				m.multiplyTranslation(-0.5,-0.5, 0);
				break;
			
			case SHADER_TCMOD_TRANSLATE:		
				x = RB_Evaluate(shared, tcmod.x, 0.0);
				y = RB_Evaluate(shared, tcmod.y, 0.0);
				
				m.multiplyTranslation(x, y, 0.0);
				break;
			
			case SHADER_TCMOD_SHEAR:
				//TODO
				break;
	
			default:
				ri.Com_Error(ERR_DROP, "RB_SetupTCModMatrix: tcmod type %i not supported\n", tcmod.type);
				break;
		}
	}
	
	return true;
}


void	RB_ModifyTextureMatrix(const r_entity_c *ent, const r_shader_stage_c *stage)
{	
	matrix_c	m;
	
	RB_SetupTCModMatrix(ent->getShared(), stage, m);
	
//	if(gl_state.current_tmu_mats[gl_state.current_tmu] == m)
//		return;

//	gl_state.current_tmu_mats[gl_state.current_tmu] = m;
	
	// upload it
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixfARB(&m[0][0]);
	xglMatrixMode(GL_MODELVIEW);
}

void	RB_ModifyOmniLightTextureMatrix(const r_command_t *cmd, const r_shader_stage_c *stage)
{
	matrix_c	m;
		
	RB_SetupTCModMatrix(cmd->getLight()->getShared(), stage, m);
	m.multiply(cmd->getLightAttenuation());

	/*
	if(	gl_state.current_tmu_entities[gl_state.current_tmu] != cmd->getEntity() ||
		gl_state.current_tmu_lights[gl_state.current_tmu] != cmd->getLight() ||
		gl_state.current_tmu_mats[gl_state.current_tmu] == m)
		return;

	if(!RB_SetupTCModMatrix(ent->getShared(), stage, m))
	{
		if(gl_state.current_tmu_mats[gl_state.current_tmu] == matrix_identity)
			return;
	}
	else
	{
		if(gl_state.current_tmu_mats[gl_state.current_tmu] == m)
			return;
	}
	
	m.multiply(cmd->getLight()->getAttenuation());
	if(&r_world_entity == cmd->getEntity())
		m.multiply(cmd->getLight()->getView());
	else
		m.multiply(cmd->getLight()->getView() * cmd->getEntity()->getTransform());
	*/
		
	// upload it
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixfARB(&m[0][0]);
	xglMatrixMode(GL_MODELVIEW);
}

void	RB_ModifyOmniLightCubeTextureMatrix(const r_command_t *cmd, const r_shader_stage_c *stage)
{
#if 0
	matrix_c	m;
	
	RB_SetupTCModMatrix(cmd->getLight()->getShared(), stage, m);	
	m.multiply(cmd->getLightTransform());

	/*
	if(&r_world_entity == cmd->getEntity())
		m.multiply(cmd->getLight()->getView());
	else
		m.multiply(cmd->getLight()->getView() * cmd->getEntity()->getTransform());
	*/
	
	// upload it
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixfARB(&m[0][0]);
	xglMatrixMode(GL_MODELVIEW);
#endif
}

void	RB_ModifyProjLightTextureMatrix(const r_command_t *cmd, const r_shader_stage_c *stage)
{
	matrix_c	m;
	
	RB_SetupTCModMatrix(cmd->getLight()->getShared(), stage, m);
	m.multiply(cmd->getLightAttenuation());

	/*
	m.multiply(cmd->getLight()->getAttenuation());
	if(&r_world_entity == cmd->getEntity())
		m.multiply(cmd->getLight()->getView());
	else
		m.multiply(cmd->getLight()->getView() * cmd->getEntity()->getTransform());
	*/
	
	// upload it
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixfARB(&m[0][0]);
	xglMatrixMode(GL_MODELVIEW);
}

void	RB_ModifyProjShadowTextureMatrix(const r_command_t *cmd)
{
#if 0
	matrix_c	m;
	m.setupTranslation(0.5, 0.5, 0.5);
	m.multiplyScale(0.5, 0.5, 0.5);
	m.multiply(cmd->getLight()->getProjection());
	m.multiply(cmd->getLight()->getView());
	m.multiply(cmd->getEntity()->getTransform());
#else
	const matrix_c& m = cmd->getLightAttenuation();

	/*
	matrix_c	m;
	m.multiply(cmd->getLight()->getAttenuation());
	if(&r_world_entity == cmd->getEntity())
		m.multiply(cmd->getLight()->getView());
	else
		m.multiply(cmd->getLight()->getView() * cmd->getEntity()->getTransform());
	*/
#endif
	
	// upload it
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixfARB(&m[0][0]);
	xglMatrixMode(GL_MODELVIEW);
}


void	RB_ModifyColor(const r_entity_t &shared, const r_shader_stage_c *stage, vec4_c &color)
{
	switch(stage->rgb_gen)
	{
		
		case SHADER_RGB_GEN_IDENTITY:
//		case SHADER_RGB_GEN_CUSTOM:
//		case SHADER_RGB_GEN_ENTITY:
			color[0] = 1.0;
			color[1] = 1.0;
			color[2] = 1.0;
			break;	
#if 1
		case SHADER_RGB_GEN_CUSTOM:
			color[0] = X_bound(0.0, RB_Evaluate(shared, stage->red, shared.shader_parms[0]), 1.0);
			color[1] = X_bound(0.0, RB_Evaluate(shared, stage->green, shared.shader_parms[1]), 1.0);
			color[2] = X_bound(0.0, RB_Evaluate(shared, stage->blue, shared.shader_parms[2]), 1.0);
			break;
#endif

#if 1
		case SHADER_RGB_GEN_ENTITY:
			color[0] = X_bound(0.0, shared.shader_parms[0], 1.0);
			color[1] = X_bound(0.0, shared.shader_parms[1], 1.0);
			color[2] = X_bound(0.0, shared.shader_parms[2], 1.0);
			break;
#endif		
		default:
			ri.Com_Error(ERR_DROP, "RB_ModifyColor: rgbgen mode %i not supported", stage->rgb_gen);
			break;
	}

	switch(stage->alpha_gen)
	{
		case SHADER_ALPHA_GEN_IDENTITY:
			color[3] = 1.0;
			break;
			
		case SHADER_ALPHA_GEN_CUSTOM:
			color[3] = X_bound(0.0, RB_Evaluate(shared, stage->alpha, 1.0), 1.0);
			break;
		
		case SHADER_ALPHA_GEN_ENTITY:
			color[3] = X_bound(0.0, shared.shader_parms[3], 1.0);
			break;
			
		default:
			ri.Com_Error(ERR_DROP, "RB_ModifyColor: alphagen %i not supported", stage->alpha_gen);
	}
}




void	RB_RenderCommand(const r_command_t *cmd, r_render_type_e type)
{
	const r_shader_c*	entity_shader	= cmd->getEntityShader();
	const r_shader_c*	light_shader	= cmd->getLightShader();

	RB_EnableShaderStates(entity_shader);

	//
	// call the apropiate flush function
	//
	switch(type)
	{
		case RENDER_TYPE_DEFAULT:
		{
			for(std::vector<r_shader_stage_c*>::const_iterator ir = entity_shader->stages.begin(); ir != entity_shader->stages.end(); ++ir)
			{
				const r_shader_stage_c* stage = *ir;
				
				if(!RB_Evaluate(cmd->getEntity()->getShared(), stage->condition, 1))
					continue;
			
				switch(stage->type)
				{
					case SHADER_MATERIAL_STAGE_TYPE_COLORMAP:
					{
						RB_EnableShader_generic();
						RB_RenderCommand_generic(cmd, stage);
						RB_DisableShader_generic();
						break;
					}
				
					case SHADER_MATERIAL_STAGE_TYPE_REFLECTIONMAP:
					{
						RB_EnableShader_reflection_C();
						RB_RenderCommand_reflection_C(cmd, stage);
						RB_DisableShader_reflection_C();
						break;
					}
					
					case SHADER_MATERIAL_STAGE_TYPE_REFRACTIONMAP:
					{
						RB_EnableShader_refraction_C();
						RB_RenderCommand_refraction_C(cmd, stage);
						RB_DisableShader_refraction_C();
						break;
					}
					
					case SHADER_MATERIAL_STAGE_TYPE_DISPERSIONMAP:
					{
						RB_EnableShader_dispersion_C();
						RB_RenderCommand_dispersion_C(cmd, stage);
						RB_DisableShader_dispersion_C();
						break;
					}
					
					case SHADER_MATERIAL_STAGE_TYPE_LIQUIDMAP:
					{
						RB_EnableShader_liquid_C();
						RB_RenderCommand_liquid_C(cmd, stage);
						RB_DisableShader_liquid_C();
						break;
					}
					
					case SHADER_MATERIAL_STAGE_TYPE_SKYBOXMAP:
					{
						RB_EnableShader_skybox();
						RB_RenderCommand_skybox(cmd, stage);
						RB_DisableShader_skybox();
						break;
					}
					
					default:
						break;
				}
			}
			
			break;
		}
		
		case RENDER_TYPE_GENERIC:
		{
			for(std::vector<r_shader_stage_c*>::const_iterator ir = entity_shader->stages.begin(); ir != entity_shader->stages.end(); ++ir)
			{
				const r_shader_stage_c* stage = *ir;
				
				if(stage->type == SHADER_MATERIAL_STAGE_TYPE_COLORMAP)
				{
					if(!RB_Evaluate(cmd->getEntity()->getShared(), stage->condition, 1))
						continue;
					
					RB_RenderCommand_generic(cmd, stage);
				}
			}
			
			break;
		}
		
		case RENDER_TYPE_ZFILL:
		{
			if(entity_shader->stage_diffusemap)
			{
				if(!RB_Evaluate(cmd->getEntity()->getShared(), entity_shader->stage_diffusemap->condition, 1))
					break;
					
				RB_RenderCommand_zfill(cmd, entity_shader->stage_diffusemap);
			}
			else if(entity_shader->hasFlags(SHADER_FORCEOPAQUE))
			{
				for(std::vector<r_shader_stage_c*>::const_iterator ir = entity_shader->stages.begin(); ir != entity_shader->stages.end(); ++ir)
				{
					const r_shader_stage_c* stage = *ir;
					
					if(!RB_Evaluate(cmd->getEntity()->getShared(), stage->condition, 1))
						continue;
			
					RB_RenderCommand_zfill(cmd, stage);
				}
			}
			
			break;
		}
		
		case RENDER_TYPE_SKY:
		{
			for(std::vector<r_shader_stage_c*>::const_iterator ir = entity_shader->stages.begin(); ir != entity_shader->stages.end(); ++ir)
			{
				const r_shader_stage_c* stage = *ir;
				
				if(!RB_Evaluate(cmd->getEntity()->getShared(), stage->condition, 1))
					continue;
					
				switch(stage->type)
				{
					case SHADER_MATERIAL_STAGE_TYPE_SKYBOXMAP:
					{
						RB_EnableShader_skybox();
						RB_RenderCommand_skybox(cmd, stage);
						RB_DisableShader_skybox();
						break;
					}
					
					case SHADER_MATERIAL_STAGE_TYPE_SKYCLOUDMAP:
					{
						RB_EnableShader_skycloud();
						RB_RenderCommand_skycloud(cmd, stage);
						RB_DisableShader_skycloud();
						break;
					}
					
					default:
						break;
				}
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_R:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_lightmap &&
				entity_shader->stage_deluxemap)
			{
				RB_RenderCommand_lighting_R(cmd,	entity_shader->stage_diffusemap,
									entity_shader->stage_lightmap,
									entity_shader->stage_deluxemap);
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_RB:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap &&
				entity_shader->stage_lightmap &&
				entity_shader->stage_deluxemap)
			{
				RB_RenderCommand_lighting_RB(cmd,	entity_shader->stage_diffusemap,
									entity_shader->stage_bumpmap,
									entity_shader->stage_lightmap,
									entity_shader->stage_deluxemap);
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_RBH:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap &&
				entity_shader->stage_lightmap &&
				entity_shader->stage_deluxemap)
			{
				RB_RenderCommand_lighting_RBH(cmd,	entity_shader->stage_diffusemap,
									entity_shader->stage_bumpmap,
									entity_shader->stage_lightmap,
									entity_shader->stage_deluxemap);
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_RBHS:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap &&
				entity_shader->stage_specularmap &&
				entity_shader->stage_lightmap &&
				entity_shader->stage_deluxemap)
			{
				RB_RenderCommand_lighting_RBHS(cmd,	entity_shader->stage_diffusemap,
									entity_shader->stage_bumpmap,
									entity_shader->stage_specularmap,
									entity_shader->stage_lightmap,
									entity_shader->stage_deluxemap);
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_RBS:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap &&
				entity_shader->stage_specularmap &&
				entity_shader->stage_lightmap &&
				entity_shader->stage_deluxemap)
			{
				RB_RenderCommand_lighting_RBS(cmd,	entity_shader->stage_diffusemap,
									entity_shader->stage_bumpmap,
									entity_shader->stage_specularmap,
									entity_shader->stage_lightmap,
									entity_shader->stage_deluxemap);
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_D_vstatic:
		{
			if(entity_shader->stage_diffusemap)
			{
				RB_RenderCommand_lighting_D_vstatic(cmd,	entity_shader->stage_diffusemap);
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_DB_vstatic:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap)
			{
				RB_RenderCommand_lighting_DB_vstatic(cmd,	entity_shader->stage_diffusemap,
										entity_shader->stage_bumpmap);
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_DBH_vstatic:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap)
			{
				RB_RenderCommand_lighting_DBH_vstatic(cmd,	entity_shader->stage_diffusemap,
										entity_shader->stage_bumpmap);
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_DBHS_vstatic:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap &&
				entity_shader->stage_specularmap)
			{
				RB_RenderCommand_lighting_DBHS_vstatic(cmd,	entity_shader->stage_diffusemap,
										entity_shader->stage_bumpmap,
										entity_shader->stage_specularmap);
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_DBS_vstatic:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap &&
				entity_shader->stage_specularmap)
			{
				RB_RenderCommand_lighting_DBS_vstatic(cmd,	entity_shader->stage_diffusemap,
										entity_shader->stage_bumpmap,
										entity_shader->stage_specularmap);
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_D_omni:
		{
			if(entity_shader->stage_diffusemap)
			{
				for(std::vector<r_shader_stage_c*>::const_iterator ir = light_shader->stages.begin(); ir != light_shader->stages.end(); ++ir)
				{
					const r_shader_stage_c* stage = *ir;
					
					if(stage->type_light != SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY)
						continue;
						
					if(!RB_Evaluate(cmd->getLight()->getShared(), stage->condition, 1))
						continue;
					
					RB_RenderCommand_lighting_D_omni(cmd,		entity_shader->stage_diffusemap,
											stage,
											light_shader->stage_attenuationmap_z,
											light_shader->stage_attenuationmap_cube);
				}
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_D_proj:
		{
			if(entity_shader->stage_diffusemap)
			{
				for(std::vector<r_shader_stage_c*>::const_iterator ir = light_shader->stages.begin(); ir != light_shader->stages.end(); ++ir)
				{
					const r_shader_stage_c* stage = *ir;
				
					if(stage->type_light != SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY)
						continue;
						
					if(!RB_Evaluate(cmd->getLight()->getShared(), stage->condition, 1))
						continue;
					
					RB_RenderCommand_lighting_D_proj(cmd,		entity_shader->stage_diffusemap,
											stage,
											light_shader->stage_attenuationmap_z);
				}
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_DB_omni:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap)
			{
				for(std::vector<r_shader_stage_c*>::const_iterator ir = light_shader->stages.begin(); ir != light_shader->stages.end(); ++ir)
				{
					const r_shader_stage_c* stage = *ir;
				
					if(stage->type_light != SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY)
						continue;
						
					if(!RB_Evaluate(cmd->getLight()->getShared(), stage->condition, 1))
						continue;
					
					RB_RenderCommand_lighting_DB_omni(cmd,		entity_shader->stage_diffusemap,
											entity_shader->stage_bumpmap,
											stage,
											light_shader->stage_attenuationmap_z,
											light_shader->stage_attenuationmap_cube);
				}
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_DBH_omni:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap)
			{
				for(std::vector<r_shader_stage_c*>::const_iterator ir = light_shader->stages.begin(); ir != light_shader->stages.end(); ++ir)
				{
					const r_shader_stage_c* stage = *ir;
				
					if(stage->type_light != SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY)
						continue;
						
					if(!RB_Evaluate(cmd->getLight()->getShared(), stage->condition, 1))
						continue;
					
					RB_RenderCommand_lighting_DBH_omni(cmd,		entity_shader->stage_diffusemap,
											entity_shader->stage_bumpmap,
											stage,
											light_shader->stage_attenuationmap_z,
											light_shader->stage_attenuationmap_cube);
				}
			}
			
			break;
		}
		
		case RENDER_TYPE_LIGHTING_DBHS_omni:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap &&
				entity_shader->stage_specularmap)
			{
				for(std::vector<r_shader_stage_c*>::const_iterator ir = light_shader->stages.begin(); ir != light_shader->stages.end(); ++ir)
				{
					const r_shader_stage_c* stage = *ir;
				
					if(stage->type_light != SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY)
						continue;
						
					if(!RB_Evaluate(cmd->getLight()->getShared(), stage->condition, 1))
						continue;
					
					RB_RenderCommand_lighting_DBHS_omni(cmd,	entity_shader->stage_diffusemap,
											entity_shader->stage_bumpmap,
											entity_shader->stage_specularmap,
											stage,
											light_shader->stage_attenuationmap_z,
											light_shader->stage_attenuationmap_cube);
				}
			}

			break;
		}
		
		case RENDER_TYPE_LIGHTING_DBS_omni:
		{
			if(	entity_shader->stage_diffusemap &&
				entity_shader->stage_bumpmap &&
				entity_shader->stage_specularmap)
			{
				for(std::vector<r_shader_stage_c*>::const_iterator ir = light_shader->stages.begin(); ir != light_shader->stages.end(); ++ir)
				{
					const r_shader_stage_c* stage = *ir;
				
					if(stage->type_light != SHADER_LIGHT_STAGE_TYPE_ATTENUATIONMAP_XY)
						continue;
						
					if(!RB_Evaluate(cmd->getLight()->getShared(), stage->condition, 1))
						continue;
					
					RB_RenderCommand_lighting_DBS_omni(cmd,		entity_shader->stage_diffusemap,
											entity_shader->stage_bumpmap,
											entity_shader->stage_specularmap,
											stage,
											light_shader->stage_attenuationmap_z,
											light_shader->stage_attenuationmap_cube);
				}
			}
			
			break;
		}
		
		case RENDER_TYPE_FOG_UNIFORM:	// for now same as zfill
		{
			if(entity_shader->stage_diffusemap)
			{
				if(!RB_Evaluate(cmd->getEntity()->getShared(), entity_shader->stage_diffusemap->condition, 1))
					break;
					
				RB_RenderCommand_fog_uniform(cmd, entity_shader->stage_diffusemap);
			}
			else if(entity_shader->hasFlags(SHADER_FORCEOPAQUE))
			{
				for(std::vector<r_shader_stage_c*>::const_iterator ir = entity_shader->stages.begin(); ir != entity_shader->stages.end(); ++ir)
				{
					const r_shader_stage_c* stage = *ir;
					
					if(!RB_Evaluate(cmd->getEntity()->getShared(), stage->condition, 1))
						continue;
			
					RB_RenderCommand_fog_uniform(cmd, stage);
				}
			}
			
			break;
		}
		
		case RENDER_TYPE_POSTPROCESS:
		{
			for(std::vector<r_shader_stage_c*>::const_iterator ir = entity_shader->stages.begin(); ir != entity_shader->stages.end(); ++ir)
			{
				const r_shader_stage_c* stage = *ir;
				
				if(!RB_Evaluate(cmd->getEntity()->getShared(), stage->condition, 1))
					continue;
			
				switch(stage->type)
				{
					case SHADER_MATERIAL_STAGE_TYPE_HEATHAZEMAP:
					{
						// update _currentRender texture
						//r_img_currentrender->copyFromContext();
						
						// update _currentRenderDepth texture
						//r_img_currentrender_depth->copyFromContext();
					
						RB_EnableShader_heathaze();
						RB_RenderCommand_heathaze(cmd, stage);
						RB_DisableShader_heathaze();
						break;
					}
					
					default:
						break;
				}
			}
			
			break;
		}
		
		default:
		{
			break;
		}
		
	} // switch
	
	RB_DisableShaderStates(entity_shader);
}



int	RB_SortByCommandDistanceFunc(void const *a, void const *b)
{
	r_command_t* cmd_a = (r_command_t*)a;
	r_command_t* cmd_b = (r_command_t*)b;

	vec_t dist_a = cmd_a->getDistance();
	vec_t dist_b = cmd_b->getDistance();
	
	if(dist_a < dist_b)
		return 1;
	
	else if(dist_a > dist_b)
		return -1;
		
	else
		return 0;
}

int	RB_SortByEntityShaderFunc(void const *a, void const *b)
{
	r_command_t* cmd_a = (r_command_t*)a;
	r_command_t* cmd_b = (r_command_t*)b;

	r_shader_c* shader_a = cmd_a->getEntityShader();
	r_shader_c* shader_b = cmd_b->getEntityShader();
	
	if(shader_a > shader_b)
		return -1;
	
	else if(shader_a < shader_b)
		return 1;
	
	else
		return 0;
}

int	RB_SortByEntityMeshFunc(void const *a, void const *b)
{
	r_command_t* cmd_a = (r_command_t*)a;
	r_command_t* cmd_b = (r_command_t*)b;

	r_mesh_c* mesh_a = cmd_a->getEntityMesh();
	r_mesh_c* mesh_b = cmd_b->getEntityMesh();
	
	if(mesh_a > mesh_b)
		return -1;
	
	else if(mesh_a < mesh_b)
		return 1;
	
	else
		return 0;	
}

int	RB_SortByEntityMeshVertexBufferOffsetFunc(void const *a, void const *b)
{
	r_command_t* cmd_a = (r_command_t*)a;
	r_command_t* cmd_b = (r_command_t*)b;
	
	uint_t vbo_a = cmd_a->getEntityMesh()->vbo_array_buffer;
	uint_t vbo_b = cmd_b->getEntityMesh()->vbo_array_buffer;

	uint_t ofs_a = cmd_a->getEntityMesh()->vbo_vertexes_ofs;
	uint_t ofs_b = cmd_b->getEntityMesh()->vbo_vertexes_ofs;
	
	if(vbo_a < vbo_b)	return 1;
	if(vbo_a > vbo_b)	return-1;
	
	if(ofs_a < ofs_b)	return 1;
	if(ofs_a > ofs_b)	return-1;
	
	return 0;
}

int	RB_SortByLightFunc(void const *a, void const *b)
{
	r_command_t* cmd_a = (r_command_t*)a;
	r_command_t* cmd_b = (r_command_t*)b;

	r_light_c* light_a = cmd_a->getLight();
	r_light_c* light_b = cmd_b->getLight();
	
	if(light_a > light_b)
		return -1;
	
	else if(light_a < light_b)
		return 1;
	
	else
		return 0;
}

int	R_TranslucentCommandSortFunc(void const *a, void const *b)
{
	r_command_t* cmd_a = (r_command_t*)a;
	r_command_t* cmd_b = (r_command_t*)b;
	
	float sort_a = RB_Evaluate(cmd_a->getEntity()->getShared(), cmd_a->getEntityShader()->getSort(), 0);
	float sort_b = RB_Evaluate(cmd_b->getEntity()->getShared(), cmd_b->getEntityShader()->getSort(), 0);
	
	if(sort_a > sort_b)
		return -1;
	
	else if(sort_a < sort_b)
		return 1;
	
	else
		return 0;	
}

/*
static int	sortfunc(void const *a, void const *b)
{
	int anum;
	int bnum;
	
	anum = *(int*)a;
	bnum = *(int*)b;

	if(anum > bnum)
		return -1;
	
	else if(anum < bnum)
		return 1;
	
	else
		return 0;
}
*/

void	RB_RenderCommands()
{
	uint_t		i, j;
	r_command_t*	cmd = NULL;
	
	//
	// FIXME
	//
	//r_img_currentenvironment->copyFromContext();
	
	
	
	//
	// sort commands
	//
//	qsort(&r_current_scene->cmds[0], r_current_scene->cmds_num, sizeof(r_command_t), RB_SortByCommandDistanceFunc);
	
	qsort(&r_current_scene->cmds_translucent[0], r_current_scene->cmds_translucent_num, sizeof(r_command_t), R_TranslucentCommandSortFunc);
	
	
	if(r_speeds->getInteger())
		time_setup = ri.Sys_Milliseconds();
	

	//
	// draw solid meshes into zbuffer
	//
//	xglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	xglDepthMask(GL_TRUE);
	
	RB_EnableShader_zfill();
	for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
	{
		if(!cmd->getEntity()->isVisible())
			continue;
				
		cmd->getEntityModel()->draw(cmd, RENDER_TYPE_ZFILL);
	}
	RB_DisableShader_zfill();

//	xglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	xglDepthMask(GL_FALSE);
	
	
	if(r_speeds->getInteger())
		time_zfill = ri.Sys_Milliseconds();
	
	
	//
	// draw sky stages, skybox and cloud layer stages
	//
	/*
	if(r_drawsky->getInteger())
	{
		for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
			cmd->getEntityModel()->draw(cmd, RENDER_TYPE_SKY);
	}
	*/
	
	
	//
	// draw static radiosity
	//
	if(r_lightmap->getInteger() && r_images_lm.size())
	{
		xglEnable(GL_BLEND);
		xglBlendFunc(GL_ONE, GL_ONE);
	
		if(r_bump_mapping->getInteger())
		{
			if(r_parallax->getInteger() && r_specular->getInteger())
			{
				RB_EnableShader_lighting_RBHS();
				
				for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
						
					if(!cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_RBHS);
				}
				
				for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
						
					if(!cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_RBHS);
				}
				
				RB_DisableShader_lighting_RBHS();
				
				RB_EnableShader_lighting_DBHS_vstatic();
				
				for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
					
					if(!cmd->hasLightVertexes())
						continue;
					
					if(cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DBHS_vstatic);
				}
				
				for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
					
					if(!cmd->hasLightVertexes())
						continue;
					
					if(cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DBHS_vstatic);
				}
				
				RB_DisableShader_lighting_DBHS_vstatic();
			}
			else if(r_parallax->getInteger())
			{
				RB_EnableShader_lighting_RBH();
				
				for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
						
					if(!cmd->hasLightMap())
						continue;
							
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_RBH);
				}
				
				for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
						
					if(!cmd->hasLightMap())
						continue;
							
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_RBH);
				}
				
				RB_DisableShader_lighting_RBH();
				
				RB_EnableShader_lighting_DBH_vstatic();
				
				for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
					
					if(!cmd->hasLightVertexes())
						continue;
					
					if(cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DBH_vstatic);
				}
				
				for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
					
					if(!cmd->hasLightVertexes())
						continue;
					
					if(cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DBH_vstatic);
				}
				
				RB_DisableShader_lighting_DBH_vstatic();
			}
			else if(r_specular->getInteger())
			{
				RB_EnableShader_lighting_RBS();
				
				for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
						
					if(!cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_RBS);
				}
				
				for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
						
					if(!cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_RBS);
				}
				
				RB_DisableShader_lighting_RBS();
				
				RB_EnableShader_lighting_DBS_vstatic();
				
				for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
					
					if(!cmd->hasLightVertexes())
						continue;
					
					if(cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DBS_vstatic);
				}
				
				for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
					
					if(!cmd->hasLightVertexes())
						continue;
					
					if(cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DBS_vstatic);
				}
				
				RB_DisableShader_lighting_DBS_vstatic();
			}
			else
			{
				RB_EnableShader_lighting_RB();
				
				for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
						
					if(!cmd->hasLightMap())
						continue;
						
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_RB);
				}
				
				for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
						
					if(!cmd->hasLightMap())
						continue;
						
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_RB);
				}
				
				RB_DisableShader_lighting_RB();
				
				RB_EnableShader_lighting_DB_vstatic();
				
				for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
					
					if(!cmd->hasLightVertexes())
						continue;
					
					if(cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DB_vstatic);
				}
				
				for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
					
					if(!cmd->hasLightVertexes())
						continue;
					
					if(cmd->hasLightMap())
						continue;
				
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DB_vstatic);
				}
				
				RB_DisableShader_lighting_DB_vstatic();
			}
		}
		else
		{
			RB_EnableShader_lighting_R();
			
			for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
			{
				if(!cmd->getEntity()->isVisible())
					continue;
					
				if(!cmd->hasLightMap())
					continue;
				
				cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_R);
			}
			
			for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
			{
				if(!cmd->getEntity()->isVisible())
					continue;
					
				if(!cmd->hasLightMap())
					continue;
				
				cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_R);
			}
			
			RB_DisableShader_lighting_R();
			
			RB_EnableShader_lighting_D_vstatic();
			
			for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
			{
				if(!cmd->getEntity()->isVisible())
					continue;
					
				if(!cmd->hasLightVertexes())
					continue;
					
				if(cmd->hasLightMap())
					continue;
				
				cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_D_vstatic);
			}
			
			for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
			{
				if(!cmd->getEntity()->isVisible())
					continue;
					
				if(!cmd->hasLightVertexes())
					continue;
					
				if(cmd->hasLightMap())
					continue;
				
				cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_D_vstatic);
			}
			
			RB_DisableShader_lighting_D_vstatic();
		}
		
		xglDisable(GL_BLEND);
	}
	
	if(r_speeds->getInteger())
		time_lighting_static = ri.Sys_Milliseconds();
	
	
	//
	// setup lights
	//
#if 0
	RB_SetupModelviewMatrix(matrix_identity, true);	
		
	vec3_c vertexes[8];	// max x,y,z points in space
	
	xglDisable(GL_CULL_FACE);
	
	xglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
//	xglDepthMask(GL_FALSE);
		
	for(std::vector<r_light_c*>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
	{
		r_light_c* light = *ir;
		
		if(!light)
			continue;
			
		if(!light->isVisible())
			continue;
		
		if(!light->getShared().radius_aabb.isInside(r_origin))
		{
			light->updateScissor(gl_state.matrix_model_view_projection, rb_vrect_viewport, light->getShared().radius_aabb);
		}
		else
		{
			light->setScissor(rb_vrect_viewport);
		}
		
		if(gl_config.arb_occlusion_query && r_arb_occlusion_query->getInteger())
		{
			const aabb_c& bbox = light->getShared().radius_aabb;
			
			/*
			if(bbox.isInside(r_origin))
				continue;
			*/
		
			vertexes[0].set(bbox._maxs[0], bbox._mins[1], bbox._mins[2]);
			vertexes[1].set(bbox._maxs[0], bbox._mins[1], bbox._maxs[2]);
			vertexes[2].set(bbox._mins[0], bbox._mins[1], bbox._maxs[2]);
			vertexes[3].set(bbox._mins[0], bbox._mins[1], bbox._mins[2]);
			vertexes[4].set(bbox._maxs[0], bbox._maxs[1], bbox._mins[2]);
			vertexes[5].set(bbox._maxs[0], bbox._maxs[1], bbox._maxs[2]);
			vertexes[6].set(bbox._mins[0], bbox._maxs[1], bbox._maxs[2]);
			vertexes[7].set(bbox._mins[0], bbox._maxs[1], bbox._mins[2]);
			
			for(i=0; i<8; i++)
			{
				plane_side_e side = r_frustum[FRUSTUM_NEAR].onSide(vertexes[i]);
					
				if(side == SIDE_BACK)
					break;
			}
			
			if(i != 8)
				continue;
		
			light->beginOcclusionQuery();
			
			xglBegin(GL_QUADS);
							
			// left side
			xglVertex3fv(vertexes[0]);
			xglVertex3fv(vertexes[1]);
			xglVertex3fv(vertexes[2]);
			xglVertex3fv(vertexes[3]);
			
			// right side
			xglVertex3fv(vertexes[4]);
			xglVertex3fv(vertexes[5]);
			xglVertex3fv(vertexes[6]);
			xglVertex3fv(vertexes[7]);
			
			// front side
			xglVertex3fv(vertexes[0]);
			xglVertex3fv(vertexes[1]);
			xglVertex3fv(vertexes[5]);
			xglVertex3fv(vertexes[4]);
		
			// back side
			xglVertex3fv(vertexes[2]);
			xglVertex3fv(vertexes[3]);
			xglVertex3fv(vertexes[7]);
			xglVertex3fv(vertexes[6]);
			
			// top side
			xglVertex3fv(vertexes[1]); 
			xglVertex3fv(vertexes[2]);
			xglVertex3fv(vertexes[6]); 
			xglVertex3fv(vertexes[5]);
		
			// bottom side
			xglVertex3fv(vertexes[0]); 
			xglVertex3fv(vertexes[3]);
			xglVertex3fv(vertexes[7]); 
			xglVertex3fv(vertexes[4]);
			
			xglEnd();
		
			light->endOcclusionQuery();
		
			if(light->getOcclusionSamplesAvailable() && light->getOcclusionSamplesNum() <= 0)
			{
				// light bounding box is not visible
				light->resetFrameCount();
				c_lights--;
			}
		}
	}
	
	xglEnable(GL_CULL_FACE);
	
	xglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//	xglDepthMask(GL_TRUE);
#endif
	
	//
	// draw dynamic shadowing and lighting
	//
	if(r_lighting->getInteger())
	{
		//
		// update shadow maps
		//
		#if 0
		GLimp_ActivatePbuffer();
		
		xglDisable(GL_SCISSOR_TEST);
		//xglEnable(GL_SCISSOR_TEST);
		//xglScissor(0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger());
		
		//if(r_newrefdef.flip_y ^ r_newrefdef.flip_z)
		//	xglFrontFace(GL_CW);
		//else
		//	xglFrontFace(GL_CCW);
	
		xglEnable(GL_DEPTH_TEST);
		xglDepthFunc(GL_LEQUAL);
		xglDepthMask(GL_TRUE);
	
		xglEnable(GL_CULL_FACE);
		xglCullFace(GL_FRONT);
	
		xglPolygonMode(GL_FRONT_AND_BACK, gl_state.polygon_mode);
		
		xglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		xglDepthMask(GL_TRUE);

		for(i=0; i<r_lights.size(); i++)
		{
			r_light_c* light = r_lights[i];
		
			if(!light)
				continue;
			
			if(!light->isVisible())
				continue;
				
			xglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				
			switch(light->getType())
			{
				#if 0
				case LIGHT_OMNI:
				{
					if(r_lighting_omni->getInteger() <= 0)
						continue;
						
					r_refdef_t refdef;
					refdef.x = 0;
					refdef.y = 0;
					refdef.width = vid_pbuffer_width->getInteger();
					refdef.height = vid_pbuffer_height->getInteger();
					refdef.setFOV(90);
					refdef.view_angles.set(0, 0, 90);
					refdef.flip_x = false;
					refdef.flip_y = false;
					refdef.flip_z = false;
					
					RB_SetupViewPort(0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger());
					
					RB_SetupPerspectiveProjectionMatrix(refdef);
					
					RB_SetupViewMatrix(light->getOrigin(), refdef.view_angles);
					
					xglPolygonOffset(2.5f, 10.0f);
					xglEnable(GL_POLYGON_OFFSET_FILL);
					
					RB_EnableShader_depth_to_rgba();
					for(i=0, cmd = &r_current_scene->cmds_light[0]; i<r_current_scene->cmds_light_num; i++, cmd++)
					{
						if(cmd->getLight() != light)
							continue;
							
						if(!cmd->getEntity()->isVisible())
							continue;
				
						if(cmd->getLightShader()->getLightType() == SHADER_LIGHT_FOG)
							continue;
							
						cmd->getEntityModel()->draw(cmd, RENDER_TYPE_DEPTH_TO_RGBA);
					}
					RB_DisableShader_depth_to_rgba();
		
					xglDisable(GL_POLYGON_OFFSET_FILL);
									
					//RB_SelectTexture(GL_TEXTURE0_ARB);
					light->getShadowMap()->bind(true);
					xglCopyTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, 0, 0, 0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger());
					
					#if 0
					if(r_shadows_export->getInteger())
					{
						byte* buffer = new byte[vid_pbuffer_width->getInteger() * vid_pbuffer_height->getInteger() * 3];
						xglReadPixels(0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), GL_RGB, GL_UNSIGNED_BYTE, buffer);
						IMG_WriteTGA(va("shadowmaps/light_%04d_px.tga", i), buffer,vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), 3);
						delete [] buffer;
					}
					#endif
					break;
				}
				#endif
				
				#if 1
				case LIGHT_PROJ:
				{
					if(r_lighting_proj->getInteger() <= 0)
						continue;
				
					// setup viewport
					xglViewport(0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger());
					
					// setup perspective projection
					matrix_c& m = gl_state.matrix_projection = light->getShadowMapProjection() * gl_state.matrix_quake_to_opengl;
					
					xglMatrixMode(GL_PROJECTION);
					xglLoadTransposeMatrixfARB(&m[0][0]);
					xglMatrixMode(GL_MODELVIEW);
					
					// setup view
					gl_state.matrix_view = light->getView();
					
					RB_SetupModelviewMatrix(matrix_identity, true);
					
					//RB_SetupFrustum();
					
					//xglPolygonOffset(2.5f, 10.0f);
					//xglEnable(GL_POLYGON_OFFSET_FILL);
					
					RB_EnableShader_zfill();
					for(j=0, cmd = &r_current_scene->cmds_light[0]; j<r_current_scene->cmds_light_num; j++, cmd++)
					{
						if(cmd->getLight() != light)
							continue;
							
						if(!cmd->getEntity()->isVisible())
							continue;
							
						if(cmd->getLightShader()->getLightType() == SHADER_LIGHT_FOG)
							continue;
							
						cmd->getEntityModel()->draw(cmd, RENDER_TYPE_ZFILL);
					}
					RB_DisableShader_zfill();
					
					/*
					RB_EnableShader_lighting_D_proj();
					for(j=0, cmd = &r_current_scene->cmds_light[0]; j<r_current_scene->cmds_light_num; j++, cmd++)
					{
						if(cmd->getLight() != light)
							continue;
					
						if(!cmd->getEntity()->isVisible())
							continue;
				
						if(cmd->getLightShader()->getLightType() == SHADER_LIGHT_FOG)
							continue;
							
						cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_D_proj);
					}
					RB_DisableShader_lighting_D_proj();
					*/
					
					//xglDisable(GL_POLYGON_OFFSET_FILL);
					
					#if 1
					if(r_shadows_export->getInteger())
					{
						byte* buffer = new byte[vid_pbuffer_width->getInteger() * vid_pbuffer_height->getInteger() * 3];
						xglReadPixels(0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), GL_RGB, GL_UNSIGNED_BYTE, buffer);
						IMG_WriteTGA(va("shadowmaps/light_%04d.tga", i), buffer,vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), 3);
						delete [] buffer;
					}
					#endif
					
					//RB_SelectTexture(GL_TEXTURE0_ARB);
					light->getShadowMap()->bind(true);
					xglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger());
					break;
				}
				#endif
				
				default:
					break;
			}
		}
	
		xglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		xglDepthMask(GL_FALSE);
		
		GLimp_DeactivatePbuffer();
		
		if(r_shadows_export->getInteger())
		{
			ri.Cvar_SetValue("r_shadows_export", 0);
		}
		
		// reset view
		RB_SetupGL3D();
		#endif

		//
		// omni-directional lighting
		//
		r_shadowframecount++;
		
		xglEnable(GL_BLEND);
		xglBlendFunc(GL_ONE, GL_ONE);
	
		//qsort(&r_current_scene->cmds_light[0], r_current_scene->cmds_light_num, sizeof(r_command_t), RB_SortByEntityMeshVertexBufferOffsetFunc);
		
		//qsort(&r_current_scene->cmds_light[0], r_current_scene->cmds_light_num, sizeof(r_command_t), RB_SortByLightFunc);
		
		if(r_lighting_omni->getInteger())
		{
			if(r_bump_mapping->getInteger())
			{
				if(r_parallax->getInteger() && r_specular->getInteger())
				{
					RB_EnableShader_lighting_DBHS_omni();
					for(i=0, cmd = &r_current_scene->cmds_light[0]; i<r_current_scene->cmds_light_num; i++, cmd++)
					{
						if(!cmd->getEntity()->isVisible())
							continue;
					
						if(!cmd->getLight()->isVisible())
							continue;
					
						if(cmd->getLight()->getType() == LIGHT_PROJ)
							continue;
				
						if(/*cmd->getLightShader()->getLightType() == SHADER_LIGHT_AMBIENT ||*/ cmd->getLightShader()->getLightType() == SHADER_LIGHT_FOG)
							continue;
						
						/*	
						xglScissor
						(		
							cmd->getLight()->getScissorX(),
							cmd->getLight()->getScissorY(),
							cmd->getLight()->getScissorWidth(),
							cmd->getLight()->getScissorHeight()
						);
						*/
							
						cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DBHS_omni);
					}
					RB_DisableShader_lighting_DBHS_omni();
				}
				else if(r_parallax->getInteger())
				{
					RB_EnableShader_lighting_DBH_omni();
					for(i=0, cmd = &r_current_scene->cmds_light[0]; i<r_current_scene->cmds_light_num; i++, cmd++)
					{
						if(!cmd->getEntity()->isVisible())
							continue;
					
						if(!cmd->getLight()->isVisible())
							continue;
					
						if(cmd->getLight()->getType() == LIGHT_PROJ)
							continue;
				
						if(/*cmd->getLightShader()->getLightType() == SHADER_LIGHT_AMBIENT ||*/ cmd->getLightShader()->getLightType() == SHADER_LIGHT_FOG)
							continue;
						
						/*	
						xglScissor
						(		
							cmd->getLight()->getScissorX(),
							cmd->getLight()->getScissorY(),
							cmd->getLight()->getScissorWidth(),
							cmd->getLight()->getScissorHeight()
						);
						*/
						
						cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DBH_omni);
					}
					RB_DisableShader_lighting_DBH_omni();
				}
				else if(r_specular->getInteger())
				{
					RB_EnableShader_lighting_DBS_omni();
					for(i=0, cmd = &r_current_scene->cmds_light[0]; i<r_current_scene->cmds_light_num; i++, cmd++)
					{
						if(!cmd->getEntity()->isVisible())
							continue;
					
						if(!cmd->getLight()->isVisible())
							continue;
					
						if(cmd->getLight()->getType() == LIGHT_PROJ)
							continue;
				
						if(/*cmd->getLightShader()->getLightType() == SHADER_LIGHT_AMBIENT ||*/ cmd->getLightShader()->getLightType() == SHADER_LIGHT_FOG)
							continue;
						
						/*	
						xglScissor
						(		
							cmd->getLight()->getScissorX(),
							cmd->getLight()->getScissorY(),
							cmd->getLight()->getScissorWidth(),
							cmd->getLight()->getScissorHeight()
						);
						*/
						
						cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DBS_omni);
					}
					RB_DisableShader_lighting_DBS_omni();
				}
				else
				{
					RB_EnableShader_lighting_DB_omni();
					for(i=0, cmd = &r_current_scene->cmds_light[0]; i<r_current_scene->cmds_light_num; i++, cmd++)
					{
						if(!cmd->getEntity()->isVisible())
							continue;
					
						if(!cmd->getLight()->isVisible())
							continue;
					
						if(cmd->getLight()->getType() == LIGHT_PROJ)
							continue;
				
						if(/*cmd->getLightShader()->getLightType() == SHADER_LIGHT_AMBIENT ||*/ cmd->getLightShader()->getLightType() == SHADER_LIGHT_FOG)
							continue;
						
						/*	
						xglScissor
						(		
							cmd->getLight()->getScissorX(),
							cmd->getLight()->getScissorY(),
							cmd->getLight()->getScissorWidth(),
							cmd->getLight()->getScissorHeight()
						);
						*/
						
						cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_DB_omni);
					}
					RB_DisableShader_lighting_DB_omni();
				}
			}
			else
			{
				RB_EnableShader_lighting_D_omni();
				for(i=0, cmd = &r_current_scene->cmds_light[0]; i<r_current_scene->cmds_light_num; i++, cmd++)
				{
					if(!cmd->getEntity()->isVisible())
						continue;
				
					if(!cmd->getLight()->isVisible())
						continue;
				
					if(cmd->getLight()->getType() == LIGHT_PROJ)
						continue;
			
					if(/*cmd->getLightShader()->getLightType() == SHADER_LIGHT_AMBIENT ||*/ cmd->getLightShader()->getLightType() == SHADER_LIGHT_FOG)
						continue;
					
					/*
					xglScissor
					(		
						cmd->getLight()->getScissorX(),
						cmd->getLight()->getScissorY(),
						cmd->getLight()->getScissorWidth(),
						cmd->getLight()->getScissorHeight()
					);
					*/
						
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_D_omni);
				}	
				RB_DisableShader_lighting_D_omni();
			}
		} // r_lighting_omni
		
		
		//
		// projective lighting
		//
		if(r_lighting_proj->getInteger())
		{
			for(i=0; i<r_lights.size(); i++)
			{
				r_light_c* light = r_lights[i];
		
				if(light == NULL)
					continue;
			
				if(!light->isVisible())
					continue;
					
				if(light->getType() == LIGHT_OMNI)
					continue;
				
				#if 0
				if(!light->isShadowed())
				{
					// update shadowmap
					//GLimp_ActivatePbuffer();
					r_fb_lightview->bind();
					
					xglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
					xglDisable(GL_SCISSOR_TEST);
					//xglEnable(GL_SCISSOR_TEST);
					//xglScissor(0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger());
					
					//xglDisable(GL_DEPTH_TEST);
					xglEnable(GL_DEPTH_TEST);
					xglDepthFunc(GL_LEQUAL);
					xglDepthMask(GL_TRUE);
	
					xglEnable(GL_CULL_FACE);
					xglCullFace(GL_FRONT);
	
					xglPolygonMode(GL_FRONT_AND_BACK, gl_state.polygon_mode);
		
					//xglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					xglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
					xglDepthMask(GL_TRUE);
					
					// setup viewport
					xglViewport(0, 0, 256, 256);
					
					// setup perspective projection
					matrix_c& m = gl_state.matrix_projection = light->getShadowMapProjection() * gl_state.matrix_quake_to_opengl;
					
					xglMatrixMode(GL_PROJECTION);
					xglLoadTransposeMatrixfARB(&m[0][0]);
					xglMatrixMode(GL_MODELVIEW);
					
					// setup view
					gl_state.matrix_view = light->getShadowMapView();
					
					RB_SetupModelviewMatrix(matrix_identity, true);
					
					// enable polygon offset
					xglPolygonOffset(2.5f, 16.0f);
					xglEnable(GL_POLYGON_OFFSET_FILL);
					
					// render shadow map
					RB_EnableShader_zfill();
					for(j=0, cmd = &r_current_scene->cmds_light[0]; j<r_current_scene->cmds_light_num; j++, cmd++)
					{
						if(cmd->getLight() != light)
							continue;
							
						if(!cmd->getEntity()->isVisible())
							continue;
							
						if(cmd->getLightShader()->getLightType() == SHADER_LIGHT_FOG)
							continue;
							
						cmd->getEntityModel()->draw(cmd, RENDER_TYPE_ZFILL);
					}
					RB_DisableShader_zfill();
					
					xglDisable(GL_POLYGON_OFFSET_FILL);
					
					#if 0
					if(r_shadows_export->getInteger())
					{
						byte* buffer = new byte[vid_pbuffer_width->getInteger() * vid_pbuffer_height->getInteger() * 3];
						xglReadPixels(0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, buffer);
						IMG_WriteTGA(va("shadowmaps/light_%04d.tga", i), buffer,vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), 3);
						delete [] buffer;
					}
					#endif
					
					// copy to texture
					//RB_SelectTexture(GL_TEXTURE0_ARB);
					//r_img_lightview_depth->bind(true);
					//xglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger());
					
					//xglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					//xglDepthMask(GL_FALSE);
		
					GLimp_DeactivatePbuffer();
		
					// reset view
					RB_SetupGL3D();
					xglDepthMask(GL_FALSE);
				}
				#endif
				
				RB_EnableShader_lighting_D_proj();
				for(j=0, cmd = &r_current_scene->cmds_light[0]; j<r_current_scene->cmds_light_num; j++, cmd++)
				{
					if(cmd->getLight() != light)
						continue;
				
					if(!cmd->getEntity()->isVisible())
						continue;

					if(cmd->getLight()->getType() == LIGHT_OMNI)
						continue;
			
					if(cmd->getLightShader()->getLightType() == SHADER_LIGHT_FOG)
						continue;
						
					cmd->getEntityModel()->draw(cmd, RENDER_TYPE_LIGHTING_D_proj);
				}	
				RB_DisableShader_lighting_D_proj();
			}
		}// r_lighting_proj
		
		xglDisable(GL_BLEND);
		
		if(r_shadows_export->getInteger())
		{
			ri.Cvar_SetValue("r_shadows_export", 0);
		}
	}// r_lighting

	
	//
	// reset scissor
	//
	xglScissor(gl_state.vrect_viewport.x, gl_state.vrect_viewport.y, gl_state.vrect_viewport.width, gl_state.vrect_viewport.height);
	
	
	//
	// do overbright hack
	//
	RB_RenderLightScale();
	xglDepthMask(GL_FALSE);	// RB_Setup3D was called so disable writing to z-buffer again
	
	
	if(r_speeds->getInteger())
		time_lighting_dynamic = ri.Sys_Milliseconds();	
	
	//
	// draw extra stages
	//
	if(r_drawextra->getInteger())
	{	
		for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
			cmd->getEntityModel()->draw(cmd, RENDER_TYPE_DEFAULT);
	}
	
	if(r_speeds->getInteger())
		time_extra = ri.Sys_Milliseconds();
	
	
	//
	// draw translucent meshes
	//
	if(r_drawtranslucent->getInteger())
	{
		for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
			cmd->getEntityModel()->draw(cmd, RENDER_TYPE_DEFAULT);
	}
	
	if(r_speeds->getInteger())
		time_translucent = ri.Sys_Milliseconds();
	
	
	//
	// draw _currentRender specific stages, post processing effects like heatHaze, Blur
	//
	if(r_drawpostprocess->getInteger())
	{
		// update _currentRender texture
		r_img_currentrender->copyFromContext();
						
		// update _currentRenderDepth texture
		r_img_currentrender_depth->copyFromContext();
	
		#if 0
		RB_EnableShader_fog_uniform();
		for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
		{
			if(!cmd->getEntity()->isVisible())
				continue;
				
			cmd->getEntityModel()->draw(cmd, RENDER_TYPE_FOG_UNIFORM);
		}
		RB_DisableShader_fog_uniform();
		/*
		#else
		RB_EnableShader_fog();
		for(i=0, cmd = &r_current_scene->cmds_light[0]; i<r_current_scene->cmds_light_num; i++, cmd++)
		{
			if(!cmd->getEntity()->isVisible())
				continue;
					
			if(!cmd->getLight()->isVisible())
				continue;
					
			if(cmd->getLight()->getType() == LIGHT_PROJ)
				continue;
				
			if(cmd->getLightShader()->getLightType() != SHADER_LIGHT_FOG)
				continue;
						
			cmd->getEntityModel()->draw(cmd, RENDER_TYPE_FOG);
			
			c_cmds_fog++;
		}
		RB_DisableShader_fog();
		*/
		#endif
	
		for(i=0, cmd = &r_current_scene->cmds[0]; i<r_current_scene->cmds_num; i++, cmd++)
		{
			if(!cmd->getEntityShader()->hasFlags(SHADER_POSTPROCESS))
				continue;
				
			cmd->getEntityModel()->draw(cmd, RENDER_TYPE_POSTPROCESS);
		}
		
		for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
		{
			if(!cmd->getEntityShader()->hasFlags(SHADER_POSTPROCESS))
				continue;
				
			cmd->getEntityModel()->draw(cmd, RENDER_TYPE_POSTPROCESS);
		}
	}
	
	if(r_speeds->getInteger())
		time_post = ri.Sys_Milliseconds();
	
	//
	// draw areaportal debug surfaces
	//
	/*
	if(r_showareaportals->getInteger())
	{
		xglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
		for(i=0, cmd = &r_current_scene->cmds_translucent[0]; i<r_current_scene->cmds_translucent_num; i++, cmd++)
		{
			if(!cmd->getEntityShader()->hasFlags(SHADER_AREAPORTAL))
				continue;
				
			cmd->getEntityModel()->draw(cmd, RENDER_TYPE_GENERIC);
		}
		
		xglPolygonMode(GL_FRONT_AND_BACK, gl_state.polygon_mode);
	}
	*/
	
	
	//
	// done
	//
	xglDepthMask(GL_TRUE);	// reenable default RB_Setup3D state
	
	
	c_cmds += r_current_scene->cmds_num;
//	c_cmds_radiosity += r_current_scene->cmds_radiosity_num;
	c_cmds_light += r_current_scene->cmds_light_num;
	c_cmds_translucent += r_current_scene->cmds_translucent_num;
//	c_cmds_postprocess += r_current_scene->cmds_postprocess_num;
	
	
	//
	// clear current list
	//	
	r_current_scene->cmds_num = 0;
//	r_current_scene->cmds_radiosity_num = 0;
	r_current_scene->cmds_light_num = 0;
	r_current_scene->cmds_translucent_num = 0;
//	r_current_scene->cmds_postprocess_num = 0;
}


void	RB_AddCommand(	r_entity_c*		entity,
			r_model_c*		entity_model,
			r_mesh_c*		entity_mesh,
			r_shader_c*		entity_shader,
			r_light_c*		light,
			std::vector<index_t>*	light_indexes,
			int			infokey,
			vec_t			distance,
			const matrix_c&		light_attenuation)
{
	if(!entity)
	{
		ri.Com_Printf("R_AddCommand: NULL entity\n");
		return;
	}
	
	if(!entity_model)
	{
		ri.Com_Printf("R_AddCommand: NULL entity_model\n");
		return;
	}
	
	if(!entity_mesh)
	{
		ri.Com_Printf("R_AddCommand: NULL entity_mesh\n");
		return;
	}

	if(!entity_shader)
	{
		ri.Com_Printf("R_AddCommand: NULL entity_shader\n");
		return;
	}

#if DEBUG
	if(entity_mesh->isNotValid())
	{
		//ri.Com_Printf("RB_AddCommand: entity_mesh not valid\n");
		return;
	}
	
	if(light_indexes && light_indexes->empty())
	{
		// Tr3B - surface<->light relation ship doesn't result in any lit polygons
		return;
	}
#endif
	
	r_command_t *cmd = NULL, *cmd2 = NULL, *cmd3 = NULL;
	
	if(light && entity_shader->stage_diffusemap)
	{
		// create light command
		try
		{
			cmd = &r_current_scene->cmds_light.at(r_current_scene->cmds_light_num);
		}
		catch(...)
		{
			r_current_scene->cmds_light.push_back(r_command_t());
			cmd = &r_current_scene->cmds_light.at(r_current_scene->cmds_light_num);
		}
		
		/*
		if(&r_world_entity == entity)
			cmd->_light_transform	= light->getView();
		else
			cmd->_light_transform	= light->getView() * entity->getTransform();
			
		cmd->_light_attenuation	= light->getAttenuation() * cmd->_light_transform;
		*/
		
		if(entity->isStatic() && light->isStatic())
		{
			cmd->_light_attenuation = light_attenuation;
		}
		else
		{
			if(&r_world_entity == entity)
				cmd->_light_attenuation	= light->getAttenuation() * light->getView();
			else
				cmd->_light_attenuation	= light->getAttenuation() * (light->getView() * entity->getTransform());
		}
		
		r_current_scene->cmds_light_num++;
	}
	else
	{
		if(entity_shader->hasFlags(SHADER_TRANSLUCENT))
		{	
			// create translucent command
			try
			{		
				cmd = &r_current_scene->cmds_translucent.at(r_current_scene->cmds_translucent_num);
			}
			catch(...)
			{
				r_current_scene->cmds_translucent.push_back(r_command_t());
				cmd = &r_current_scene->cmds_translucent.at(r_current_scene->cmds_translucent_num);
			}
		
			r_current_scene->cmds_translucent_num++;
		}
		else
		{
			// create default command that copes with zfill, diffuse lighting and extra special stages
			try
			{
				cmd = &r_current_scene->cmds.at(r_current_scene->cmds_num);
			}
			catch(...)
			{
				r_current_scene->cmds.push_back(r_command_t());
				cmd = &r_current_scene->cmds.at(r_current_scene->cmds_num);
			}
			
			r_current_scene->cmds_num++;
		}
			
		if(r_lightmap->getInteger() && entity_shader->stage_lightmap && (infokey >= 0) && !light && r_images_lm.size())
			cmd->_light_map = true;
		else
			cmd->_light_map = false;
			
		if(!entity_mesh->lights.empty())
			cmd->_light_vertexes = true;
		else
			cmd->_light_vertexes = false;
	}
	
	/*
	if(entity_shader->hasFlags(SHADER_DEFORM_FLARE))
	{
		cmd->_transform.setupTranslation(entity->getShared().origin);
		
		vec3_c	angles;
		Vector3_ToAngles(entity->getShared().origin - r_origin, angles);
		
		cmd->_transform.multiplyRotation(angles);		
	}
	*/
	
	cmd->_entity		= entity;
	cmd->_entity_model	= entity_model;
	cmd->_entity_mesh	= entity_mesh;
	cmd->_entity_shader	= entity_shader;
	
	cmd->_light		= light;
	cmd->_light_shader	= light ? R_GetShaderByNum(light->getShared().custom_light) : NULL;
	cmd->_light_indexes	= light_indexes;
	
	cmd->_infokey		= infokey;
	
	cmd->_distance		= distance;
		
	if(cmd2)
		*cmd2 = *cmd;
		
	if(cmd3)
		*cmd3 = *cmd;
}



