/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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


viddef_t	vid;

ref_import_t	ri;




float		r_depthmin, r_depthmax;

glconfig_t	gl_config;
glstate_t 	gl_state;

r_image_c*	r_img_default;
r_image_c*	r_img_white;
r_image_c*	r_img_black;
r_image_c*	r_img_flat;
r_image_c*	r_img_quadratic;
r_image_c*	r_img_cubemap_white;
r_image_c*	r_img_cubemap_normal;
r_image_c*	r_img_cubemap_sky;
r_image_c*	r_img_nofalloff;
r_image_c*	r_img_attenuation_3d;
r_image_c*	r_img_lightview_depth;
r_image_c*	r_img_lightview_color;
r_image_c*	r_img_currentrender;
r_image_c*	r_img_currentrender_depth;

r_shader_c*	r_shader_currentrender;

r_scene_t*	r_current_scene;


r_frustum_c	r_frustum;

uint_t		r_framecount;		// used for dlight push checking
uint_t		r_visframecount;	// bumped when going to a new PVS
uint_t		r_lightframecount;
uint_t		r_checkcount;

int		r_depth_format;


uint_t		c_leafs;
uint_t		c_entities;
uint_t		c_lights;
uint_t		c_cmds;
uint_t		c_cmds_radiosity;
uint_t		c_cmds_light;
uint_t		c_cmds_translucent;
uint_t		c_cmds_postprocess;
uint_t		c_triangles;
uint_t		c_draws;
uint_t		c_expressions;

//
// view origin
//
vec3_c		r_up;
vec3_c		r_forward;
vec3_c		r_right;

vec3_c		r_origin;

bool		r_portal_view = false;	// if true, get vis data at
vec3_c		r_portal_org;		// portalorg instead of vieworg

bool		r_mirrorview = false;	// if true, lock pvs

bool		r_envmap = false;

cplane_c	r_clipplane;



r_entity_c	r_world_entity;
r_bsptree_c*	r_world_tree;

std::vector<index_t> 	r_quad_indexes;

bool		r_zfill;

//
// screen size info
//
r_refdef_t	r_newrefdef;

/*
std::map<int, r_entity_c, std::less<int> >	r_entities;
std::map<int, r_light_c>	r_lights;
*/

std::vector<r_entity_c*>		r_entities;
std::vector<r_light_c*>			r_lights(MAX_ENTITIES, NULL);

int		r_particles_num;
r_particle_t	r_particles[MAX_PARTICLES];

int		r_polys_num;
r_poly_t	r_polys[MAX_POLYS];

std::vector<r_contact_t>		r_contacts;

r_scene_t	r_world_scene;


static uint_t	r_video_export_count = 0;


//
// renderer cvars
//
cvar_t	*r_lefthand;
cvar_t	*r_draw2d;
cvar_t	*r_drawentities;
cvar_t	*r_drawworld;
cvar_t	*r_drawparticles;
cvar_t	*r_drawpolygons;
cvar_t	*r_drawsky;
cvar_t	*r_drawextra;
cvar_t	*r_drawtranslucent;
cvar_t	*r_drawpostprocess;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_lerpmodels;
cvar_t	*r_debug;
cvar_t	*r_log;
cvar_t	*r_shadows;
cvar_t	*r_shadows_alpha;
cvar_t	*r_shadows_nudge;
cvar_t	*r_lighting;
cvar_t	*r_lighting_omni;
cvar_t	*r_lighting_proj;
cvar_t	*r_lightmap;
cvar_t	*r_lightscale;
cvar_t	*r_znear;
cvar_t	*r_zfar;
cvar_t	*r_nobind;
cvar_t	*r_picmip;
cvar_t	*r_skymip;
cvar_t	*r_showtris;
cvar_t	*r_showbbox;
cvar_t	*r_showareas;
cvar_t	*r_showareaportals;
cvar_t	*r_shownormals;
cvar_t	*r_showtangents;
cvar_t	*r_showbinormals;
cvar_t	*r_showinvisible;
cvar_t	*r_showlightbboxes;
cvar_t	*r_showlightscissors;
cvar_t	*r_showlighttransforms;
cvar_t	*r_showentitybboxes;
cvar_t	*r_showentitytransforms;
cvar_t	*r_showcontacts;
cvar_t	*r_clear;
cvar_t	*r_cull;
cvar_t	*r_cullplanes;
cvar_t	*r_cullportals;
cvar_t	*r_polyblend;
cvar_t	*r_flashblend;
cvar_t	*r_playermip;
cvar_t	*r_drawbuffer;
cvar_t	*r_swapinterval;
cvar_t	*r_texturemode;
cvar_t	*r_polygonmode;
cvar_t  *r_lockpvs;
cvar_t	*r_vis;
cvar_t  *r_maxtexsize;
cvar_t	*r_finish;
cvar_t	*r_fastsky;
cvar_t	*r_subdivisions;
cvar_t	*r_flares;
cvar_t	*r_flaresize;
cvar_t	*r_flarefade;
cvar_t	*r_octree;
cvar_t	*r_bump_mapping;
cvar_t	*r_gloss;
cvar_t	*r_parallax;
cvar_t	*r_cmds_max;
cvar_t	*r_cmds_light_max;
cvar_t	*r_cmds_translucent_max;
cvar_t	*r_video_export;
cvar_t	*r_evalexpressions;

cvar_t	*r_arb_multitexture;
cvar_t	*r_arb_texture_compression;
cvar_t	*r_arb_vertex_buffer_object;
cvar_t	*r_arb_occlusion_query;
cvar_t	*r_arb_texture_rectangle;

cvar_t	*r_ext_texture3D;
cvar_t	*r_ext_compiled_vertex_array;
cvar_t	*r_ext_draw_range_elements;
cvar_t	*r_ext_texture_filter_anisotropic;
cvar_t	*r_ext_texture_filter_anisotropic_level;

cvar_t	*r_sgix_fbconfig;
cvar_t	*r_sgix_pbuffer;

cvar_t	*vid_fullscreen;
cvar_t	*vid_gamma;
cvar_t	*vid_mode;
cvar_t  *vid_gldriver;
cvar_t	*vid_colorbits;
cvar_t	*vid_depthbits;
cvar_t	*vid_stencilbits;

cvar_t	*vid_pbuffer_width;
cvar_t	*vid_pbuffer_height;
cvar_t	*vid_pbuffer_colorbits;
cvar_t	*vid_pbuffer_depthbits;
cvar_t	*vid_pbuffer_stencilbits;

cvar_t	*vid_ref;







void 	R_DrawNULL(const vec3_c &origin, const vec3_c &angles)
{
#if 0
	vec3_c	vf, vr, vu, org;
	
	// calc axis
	Angles_ToVectors(angles, vf, vr, vu);
	
	vf *= 15;	vf += origin;
	vr *= 15;	vr += origin;
	vu *= 15;	vu += origin;
	
	org = origin;
	
	// draw axis
	//glDisable(GL_BLEND);
	//glDisable(GL_CULL_FACE);
	//glDepthMask(GL_FALSE);
	
	glBegin(GL_LINES);

	glColor4fv(color_red);
	glVertex3fv(org);
	glVertex3fv(vf);
	
	glColor4fv(color_green);
	glVertex3fv(org);
	glVertex3fv(vr);
	
	glColor4fv(color_blue);
	glVertex3fv(org);
	glVertex3fv(vu);
	
	glEnd();
	
	//glDepthMask(GL_TRUE);
	//glEnable(GL_BLEND);
	//glEnable(GL_CULL_FACE);

	//TODO draw using backend
#endif
}


void 	R_DrawBBox(const aabb_c &bbox, const vec4_c &color)
{
#if 1
	//if(!r_showbbox->getValue())
	//	return;

	//
	// compute bbox vertices
	//
	vec3_c vertexes[8];	// max x,y,z points in space
	
	vertexes[0].set(bbox._maxs[0], bbox._mins[1], bbox._mins[2]);
	vertexes[1].set(bbox._maxs[0], bbox._mins[1], bbox._maxs[2]);
	vertexes[2].set(bbox._mins[0], bbox._mins[1], bbox._maxs[2]);
	vertexes[3].set(bbox._mins[0], bbox._mins[1], bbox._mins[2]);
	
	vertexes[4].set(bbox._maxs[0], bbox._maxs[1], bbox._mins[2]);
	vertexes[5].set(bbox._maxs[0], bbox._maxs[1], bbox._maxs[2]);
	vertexes[6].set(bbox._mins[0], bbox._maxs[1], bbox._maxs[2]);
	vertexes[7].set(bbox._mins[0], bbox._maxs[1], bbox._mins[2]);
	
		
	xglDisable(GL_DEPTH_TEST);
	xglDisable(GL_CULL_FACE);
	xglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glDisable(GL_TEXTURE_2D);
	
	
	//
	// draw axis
	//
	
	// TODO
	
	//
	// draw bounds
	//
	xglColor4fv(color);
	
	xglBegin(GL_QUADS);
	
	// left side	
	//xglVertex3fv(vertexes[0]);
	//xglVertex3fv(vertexes[1]);
	//xglVertex3fv(vertexes[2]);
	//xglVertex3fv(vertexes[3]);
	
	// right side
	//xglVertex3fv(vertexes[4]);
	//xglVertex3fv(vertexes[5]);
	//xglVertex3fv(vertexes[6]);
	//xglVertex3fv(vertexes[7]);
	
	
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
	
	
	//xglEnable(GL_TEXTURE_2D);
	xglPolygonMode(GL_FRONT_AND_BACK, gl_state.polygon_mode);
	xglEnable(GL_CULL_FACE);
	xglEnable(GL_DEPTH_TEST);
#endif
}


/*
=============
R_CalcTangentSpace
Tr3B - recoded from Nvidia's SDK
=============
*/
void	R_CalcTangentSpace(	vec3_c &vtangent, vec3_c &vbinormal, vec3_c &vnormal,
				const vec3_c &v0, const vec3_c &v1, const vec3_c &v2,
				const vec2_c &st0, const vec2_c &st1, const vec2_c &st2,
				const vec3_c &n	)
{
#if 1
	vec3_c cp;
	vec3_c e0(v1[0] - v0[0], st1[0] - st0[0], st1[1] - st0[1]);
	vec3_c e1(v2[0] - v0[0], st2[0] - st0[0], st2[1] - st0[1]);

	cp.crossProduct(e0,e1);
	
	vec3_c tangent;
	vec3_c binormal;
	vec3_c normal;
	
		
	if(X_fabs(cp[0]) > X_eps)
	{
		tangent[0]	= -cp[1] / cp[0];
		binormal[0]	= -cp[2] / cp[0];
	}

	e0[0] = v1[1] - v0[1];
	e1[0] = v2[1] - v0[1];

	cp.crossProduct(e0,e1);
	if(X_fabs(cp[0]) > X_eps)
	{
        	tangent[1]	= -cp[1] / cp[0];        
        	binormal[1]	= -cp[2] / cp[0];
	}

	e0[0] = v1[2] - v0[2];
	e1[0] = v2[2] - v0[2];

	cp.crossProduct(e0,e1);
	if(X_fabs(cp[0]) > X_eps)
	{
		tangent[2]	= -cp[1] / cp[0];        
		binormal[2]	= -cp[2] / cp[0];
	}


	// tangent...
	tangent.normalize();
		
	// binormal...
	binormal.normalize();
	
	// normal...
	// compute the cross product TxB
	normal.crossProduct(tangent, binormal);

	normal.normalize();

	
	// Gram-Schmidt orthogonalization process for B
	// compute the cross product B=NxT to obtain 
	// an orthogonal basis
	binormal.crossProduct(normal, tangent);
	
	if(normal.dotProduct(n) < 0)
	{
		normal.negate();
	}

	vtangent = tangent;
	vbinormal = binormal;
	vnormal = normal;
#else

	// other solution 
	// http://members.rogers.com/deseric/tangentspace.htm
	
	vec3_c planes[3];
	vec3_c e0;
	vec3_c e1;
	
	for(int i=0; i<3; i++)
	{
		e0 = vec3_c(v1[i] - v0[i], st1[0] - st0[0], st1[1] - st0[1]);
		e1 = vec3_c(v2[i] - v0[i], st2[0] - st0[0], st2[1] - st0[1]);
				
		planes[i].crossProduct(e0, e1);
	}


	vec3_c tangent;
	vec3_c binormal;
	vec3_c normal;
	
	//So your tangent space will be defined by this :
	//Normal = Normal of the triangle or Tangent X Binormal (careful with the cross product, 
	// you have to make sure the normal points in the right direction)
	//Tangent = ( dp(Fx(s,t)) / ds,  dp(Fy(s,t)) / ds, dp(Fz(s,t)) / ds )   or     ( -Bx/Ax, -By/Ay, - Bz/Az )
	//Binormal =  ( dp(Fx(s,t)) / dt,  dp(Fy(s,t)) / dt, dp(Fz(s,t)) / dt )  or     ( -Cx/Ax, -Cy/Ay, -Cz/Az )
	
	// tangent...
	tangent[0] = -planes[0][1]/planes[0][0];
	tangent[1] = -planes[1][1]/planes[1][0];
	tangent[2] = -planes[2][1]/planes[2][0];
	
	tangent.normalize();
	
	// binormal...
	binormal[0] = -planes[0][2]/planes[0][0];
	binormal[1] = -planes[1][2]/planes[1][0];
	binormal[2] = -planes[2][2]/planes[2][0];
	
	binormal.normalize();
	
	// compute the cross product TxB
	normal.crossProduct(tangent, binormal);
		
	normal.normalize();

	
	// Gram-Schmidt orthogonalization process for B
	// compute the cross product B=NxT to obtain 
	// an orthogonal basis
	//binormal.crossProduct(normal, tangent);
	
	if(normal.dotProduct(n) < 0)
	{
		normal.negate();
	}

	vtangent = tangent;
	vbinormal = binormal;
	vnormal = normal;
#endif
}


 
static void	R_ScreenShot() 
{
	int		i;
	byte		*buffer;
	std::string	checkname;
	 
	// find a file name to save it to
	for(i=0; i<=999; i++)
	{	
		checkname = "screenshots/qrazor-fxIII" + std::string(va("%03d", i)) + ".jpg";
		
		if(ri.VFS_FLoad(checkname, NULL) <= 0)
			break;	// file doesn't exist
	}
	
	if(i==1000)
	{
		ri.Com_Printf("R_ScreenShot_f: Couldn't create a file\n"); 
		return;
 	}
	
	buffer = new byte[vid.width * vid.height * 3];
	
	xglReadPixels(0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

	//save it smart and small
	IMG_WriteJPG(checkname, buffer, vid.width, vid.height, 95);
	//IMG_WriteTGA(checkname, buffer, vid.width, vid.height, 3);

	delete [] buffer;
	
	ri.Com_Printf("wrote '%s'\n", checkname.c_str());
}

void	R_WritePbuffer()
{
	int		i;
	byte		*buffer;
	std::string	checkname;
	 
	// find a file name to save it to
	for(i=0; i<=999; i++)
	{	
		checkname = "screenshots/pbuffer" + std::string(va("%03d", i)) + ".jpg";
		
		if(ri.VFS_FLoad(checkname, NULL) <= 0)
			break;	// file doesn't exist
	}
	
	if(i==1000)
	{
		ri.Com_Printf("R_WritePbuffer_f: Couldn't create a file\n"); 
		return;
 	}
	
	buffer = new byte[vid_pbuffer_width->getInteger() * vid_pbuffer_height->getInteger() * 3];
	
	xglReadPixels(0, 0, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), GL_RGB, GL_UNSIGNED_BYTE, buffer);

	IMG_WriteJPG(checkname, buffer, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), 95);

	delete [] buffer;
	
//	ri.Com_Printf("wrote '%s'\n", checkname.c_str());
}


static void	R_WriteVideoScreenShot() 
{
	std::string number = va("%04d", r_video_export_count);

	std::string filename = "video_export/frame" + number + ".tga";
	
	byte *buffer = new byte[vid.width * vid.height * 3];
	
	xglReadPixels(0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

	//save it fast
	IMG_WriteTGA(filename, buffer, vid.width, vid.height, 3);

	delete [] buffer;
	
	r_video_export_count++;
}

static void	R_ScreenShot_f()
{
	R_ScreenShot();
}

 
static void 	R_Strings_f()
{
	ri.Com_Printf("GL_VENDOR: %s\n", gl_config.vendor_string);
	ri.Com_Printf("GL_RENDERER: %s\n", gl_config.renderer_string);
	ri.Com_Printf("GL_VERSION: %s\n", gl_config.version_string);
	ri.Com_Printf("GL_EXTENSIONS: %s\n", gl_config.extensions_string);
}



static void 	R_AddEntitiesToBuffer()
{
	if(!r_drawentities->getValue())
		return;

	for(std::vector<r_entity_c*>::iterator ir = r_entities.begin(); ir != r_entities.end(); ++ir)
	{
		r_entity_c* ent = *ir;
		
		if(!ent)
			continue;
		
		if(r_mirrorview)
		{
			if(ent->getShared().flags & RF_WEAPONMODEL) 
				continue;
		}
		
		if(!ent->isVisible())
			continue;
		
		r_model_c *model = R_GetModelByNum(ent->getShared().model);
			
		if(!model)
		{
			//R_DrawNULL(r_current_entity->origin, r_current_entity->angles);
			continue;
		}
		
		model->addModelToList(ent);
	}
}


void 	R_ShadowBlend()
{
	if(r_shadows->getValue() != 1)
		return;
#if 0
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 1, 0, -99999, 99999);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//RB_SetupOrtho (0, 0, vid.width, vid.height, -99999, 99999);
#endif
	
	xglDisable(GL_CULL_FACE);
	xglDisable(GL_DEPTH_TEST);
	
	xglDisable(GL_ALPHA_TEST);
	xglEnable(GL_BLEND);
	xglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	xglEnable(GL_STENCIL_TEST);
	xglStencilFunc(GL_NOTEQUAL, 128, ~0);
	xglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	xglColor4f(0, 0, 0, X_bound(0.0f, r_shadows_alpha->getValue(), 1.0f));

	xglBegin(GL_TRIANGLES);
	xglVertex3f(-5, -5, 0);
	xglVertex3f(10, -5, 0);
	xglVertex3f(-5, 10, 0);

	xglEnd();

	xglDisable(GL_STENCIL_TEST);
	
	//glDisable(GL_BLEND);
	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_ALPHA_TEST);

	xglColor4fv(color_white);
}


void	R_SetupFrame()
{
	r_framecount++;

	r_origin = r_newrefdef.view_origin;

	Angles_ToVectors(r_newrefdef.view_angles, r_forward, r_right, r_up);

	// update scenegraph
	if(r_world_tree)
		r_world_tree->update();
}

void 	R_DrawWorld()
{
	if(!r_world_tree)
		return;

	if(r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
		
	r_world_entity.setupTransform();
		
	RB_SetupModelviewMatrix(r_world_entity.getTransform());
	
	r_world_tree->draw();
}


void	R_DrawLightDebuggingInfo()
{
	RB_SetupModelviewMatrix(matrix_identity, true);
	
	if(r_showlightbboxes->getInteger())
	{
		for(std::vector<r_light_c*>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c* light = *ir;
			
			if(!light)
				continue;
				
			if(!light->isVisible())
				R_DrawBBox(light->getShared().radius_aabb, color_red);
	
			else
				R_DrawBBox(light->getShared().radius_aabb, color_green);
		}
	}
	
	if(r_showlightscissors->getInteger())
	{
		RB_SetupGL2D();
		
		xglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		for(std::vector<r_light_c*>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c* light = *ir;
			
			if(!light)
				continue;
			
			if(!light->isVisible())
				continue;
			
					
			xglBegin(GL_QUADS);
			
			xglColor4fv(color_red);
			xglVertex3f(light->getScissorX(), light->getScissorY(), 0.0);
			
			xglColor4fv(color_green);
			xglVertex3f(light->getScissorX()+light->getScissorWidth(), light->getScissorY(), 0.0);
			
			xglColor4fv(color_blue);
			xglVertex3f(light->getScissorX()+light->getScissorWidth(), light->getScissorY()+light->getScissorHeight(), 0.0);
			
			xglColor4fv(color_white);
			xglVertex3f(light->getScissorX(), light->getScissorY()+light->getScissorHeight(), 0.0);
			
			xglEnd();
			
			/*
			R_DrawFill
			(
				r_newrefdef.width - light->getScissorX(), 
				r_newrefdef.height  - light->getScissorY(),
				light->getScissorWidth(),
				light->getScissorHeight(),
				vec4_c(1.0, 0.0, 0.0, 0.1)
			);
			*/
		}
		
		xglPolygonMode(GL_FRONT_AND_BACK, gl_state.polygon_mode);
			
		xglColor4fv(color_white);
		
		RB_SetupGL3D();
	}
	
	if(r_showlighttransforms->getInteger())
	{
		vec3_c	vf, vr, vu;
	
		for(std::vector<r_light_c*>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c* light = *ir;
			
			if(!light)
				continue;
				
			const r_entity_t& s = light->getShared();
				
			light->getTransform().toVectorsFRU(vf, vr, vu);
				
			vf *= 16 * s.scale[0];	vf += s.origin;
			vr *= 16 * s.scale[1];	vr += s.origin;
			vu *= 16 * s.scale[2];	vu += s.origin;
			
			xglBegin(GL_LINES);
			
			xglColor4fv(color_red);
			xglVertex3fv(s.origin);
			xglVertex3fv(vf);
			
			xglColor4fv(color_green);
			xglVertex3fv(s.origin);
			xglVertex3fv(vr);
			
			xglColor4fv(color_blue);
			xglVertex3fv(s.origin);
			xglVertex3fv(vu);
			
			xglColor4fv(color_yellow);
			xglVertex3fv(s.origin);
			xglVertex3fv(light->getOrigin());
			
			xglEnd();
		}
	}
}

void	R_DrawEntityDebuggingInfo()
{
	if(r_showentitybboxes->getInteger())
	{
		for(std::vector<r_entity_c*>::iterator ir = r_entities.begin(); ir != r_entities.end(); ++ir)
		{
			r_entity_c* ent = *ir;
		
			if(!ent)
				continue;
			
				
			if(!ent->isVisible())
				R_DrawBBox(ent->getAABB(), color_red);
			
			else if(ent->getLeafs().empty())
				R_DrawBBox(ent->getAABB(), color_blue);
			
			else
				R_DrawBBox(ent->getAABB(), color_green);
		}
	}

	if(r_showentitytransforms->getInteger())
	{
		vec3_c	vf, vr, vu;
	
		for(std::vector<r_entity_c*>::iterator ir = r_entities.begin(); ir != r_entities.end(); ++ir)
		{
			r_entity_c* ent = *ir;
			
			if(!ent)
				continue;
				
			const r_entity_t& s = ent->getShared();
			
				ent->getTransform().toVectorsFRU(vf, vr, vu);
			
			vf *= 16 * s.scale[0];	vf += s.origin;
			vr *= 16 * s.scale[1];	vr += s.origin;
			vu *= 16 * s.scale[2];	vu += s.origin;
			
			xglBegin(GL_LINES);
			
			xglColor4fv(color_red);
			xglVertex3fv(s.origin);
			xglVertex3fv(vf);
				
			xglColor4fv(color_green);
			xglVertex3fv(s.origin);
			xglVertex3fv(vr);
			
			xglColor4fv(color_blue);
			xglVertex3fv(s.origin);
			xglVertex3fv(vu);
			
			xglEnd();
		}
	}
}

void	R_DrawContactDebuggingInfo()
{
	if(r_showcontacts->getInteger())
	{
		xglBegin(GL_LINES);
	
		for(std::vector<r_contact_t>::const_iterator ir = r_contacts.begin(); ir != r_contacts.end(); ++ir)
		{
			const r_contact_t& contact = *ir;
			
			const vec3_c& a = contact.origin;
			const vec3_c  b = contact.origin + contact.normal;
			const vec3_c  c = contact.origin + (contact.normal * contact.depth);
			
			if(contact.depth <= REAL(0.0))
			{
				xglColor4fv(color_red);
				xglVertex3fv(a);
				xglVertex3fv(b);
			}
			else if(contact.depth < REAL(1.0))
			{
				xglColor4fv(color_yellow);
				xglVertex3fv(a);
				xglVertex3fv(b);
			}
			else if(contact.depth == REAL(1.0))
			{
				xglColor4fv(color_blue);
				xglVertex3fv(a);
				xglVertex3fv(b);
			}
			else
			{
				xglColor4fv(color_green);
				xglVertex3fv(b);
				xglVertex3fv(c);
			}
		}
		
		xglEnd();
	}
}

void	R_DrawAreaPortals()
{
	if(!r_world_tree)
		return;

	if(r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
		
	r_world_tree->drawAreaPortals();
}

void	R_DrawPbufferTest(int x, int y, int w, int h)
{
	R_DrawStretchPic(x, y, w, h, 0, 0, 1, -1, color_white, r_shader_currentrender);
}

static void	R_BeginFrame()
{
	// change modes if necessary
	if(vid_mode->isModified() || vid_fullscreen->isModified())
	{	
		// FIXME: only restart if CDS is required
		//cvar_t	*ref;
		//ref = 
		ri.Cvar_Get("vid_ref", "gl", 0);
		//ref->modified = true;
	}


	// update hardware gamma
	if(vid_gamma->isModified())
	{
		vid_gamma->isModified(false);

		if(gl_state.hwgamma)
			GLimp_Gamma();
	}
	
	// update OpenGL debugging mode
	if(r_debug->isModified())
	{
		r_debug->isModified(false);
		
		XGL_EnableDebugging((bool)r_debug->getInteger());
	}

	GLimp_BeginFrame();	RB_CheckForError();
	
	// assign shader time
	RB_SetShaderTime(ri.Sys_Milliseconds() * 0.001f);
	
	// go into 2D mode
	RB_SetupGL2D();		RB_CheckForError();
		
	// clear screen if desired
	RB_Clear();		RB_CheckForError();
}

/*
================
R_RenderView

r_newrefdef must be set before the first call
================
*/
static void 	R_RenderFrame(const r_refdef_t &fd)
{
	int	time_start = 0;
	int	time_setup = 0;
	int	time_create = 0;
	int	time_commands = 0;
	int	time_end = 0;

	r_newrefdef = fd;

	if(!r_world_tree && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		ri.Com_Error(ERR_DROP, "R_RenderFrame: NULL worldmodel");
		
	if(r_speeds->getInteger())
		time_start = ri.Sys_Milliseconds();
		
	r_current_scene = &r_world_scene;
	
	RB_CheckForError();
	
#if 0
	GLimp_ActivatePbuffer();
		
	xglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
//	RB_Clear();
#endif
			
	RB_BeginBackendFrame();		RB_CheckForError();

	R_SetupFrame();			RB_CheckForError();
	
	RB_SetupGL3D();			RB_CheckForError();
	
	if(r_speeds->getInteger())
		time_setup = ri.Sys_Milliseconds();
	
	R_DrawWorld();			RB_CheckForError();
	
//	R_DrawSky();			RB_CheckForError();
	
	R_AddEntitiesToBuffer();
	
	if(r_speeds->getInteger())
		time_create = ri.Sys_Milliseconds();
	
	RB_RenderCommands();
	
	if(r_speeds->getInteger())
		time_commands = ri.Sys_Milliseconds();
		
	R_DrawLightDebuggingInfo();	RB_CheckForError();
	
	R_DrawEntityDebuggingInfo();	RB_CheckForError();
	
	R_DrawContactDebuggingInfo();	RB_CheckForError();
	
	R_DrawAreaPortals();		RB_CheckForError();
		
//	R_AddPolysToBuffer();
		
//	R_DrawParticles();
	
#if 0
	// update _currentRender image
	r_img_currentrender->copyFromContext();
#endif

//	R_WritePbuffer();

#if 0
	GLimp_DeactivatePbuffer();
	
	xglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
//	RB_Clear();
#endif
	
	RB_SetupGL2D();			RB_CheckForError();
	
//	R_DrawPbufferTest(r_newrefdef.x, r_newrefdef.y, r_newrefdef.width, r_newrefdef.height);
	
//	R_ShadowBlend();
		
	RB_EndBackendFrame();
	
	RB_CheckForError();
	
	if(r_speeds->getInteger())
		time_end = ri.Sys_Milliseconds();
	
	if(r_speeds->getInteger())
	{
		ri.Com_Printf("%4i leafs %4i entities %4i lights %4i tris %4i draws %4i exp\n"
			,c_leafs
			,c_entities
			,c_lights
			//,c_cmds
			//,c_cmds_radiosity
			//,c_cmds_light
			//,c_cmds_translucent
			,c_triangles
			,c_draws
			,c_expressions
			//,r_contacts.size()
			);
			
		//ri.Com_Printf("%4i cmds %4i light cmds %4i translucent cmds\n", c_cmds, c_cmds_light, c_cmds_translucent);
		
		/*	
		int	all, setup, create, commands;

		all = time_end - time_start;
		setup = time_setup - time_start;
		create = time_create - time_setup;
		commands = time_commands - time_create;
		*/
		//ri.Com_Printf("%4i all %4i setup %4i create %4i commands\n", all, setup, create, commands);
		//ri.Com_Printf("%4i ents %4i lights %4i particles %4i polies\n", r_entities.size(), r_lights.size(), r_particles_num, r_polys_num);
	}
}

static void	R_EndFrame()
{
	RB_CheckForError();

	if(r_video_export->getInteger())
		R_WriteVideoScreenShot();
		
	GLimp_EndFrame();	
	
	RB_CheckForError();
}

static void	R_EnvMap_f()
{
	r_refdef_t& refdef = r_newrefdef;
	
	r_envmap = true;
	
	int width = 256;
	int height = 256;
	
	byte* buffer = new byte[width * height * 3];

	refdef.x = 0;
	refdef.y = 0;
	refdef.width = width;
	refdef.height = height;
	
	refdef.setFOV(90);

	refdef.view_angles.set(0, 0, 90);
	refdef.flip_x = false;
	refdef.flip_y = false;
	refdef.flip_z = true;
	R_BeginFrame();
	R_RenderFrame(refdef);
	R_EndFrame();
	xglReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	IMG_WriteTGA("env/dynamic_px.tga", buffer, width, height, 3);
	
	refdef.view_angles.set(0, 180, 90);
	refdef.flip_x = false;
	refdef.flip_y = true;
	refdef.flip_z = false;
	R_BeginFrame();
	R_RenderFrame(refdef);
	R_EndFrame();
	xglReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	IMG_WriteTGA("env/dynamic_nx.tga", buffer, width, height, 3);
	
	refdef.view_angles.set(0, 90, 0);
	refdef.flip_x = false;
	refdef.flip_y = false;
	refdef.flip_z = true;
	R_BeginFrame();
	R_RenderFrame(refdef);
	R_EndFrame();
	xglReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	IMG_WriteTGA("env/dynamic_py.tga", buffer, width, height, 3);
	
	refdef.view_angles.set(0,-90, 0);
	refdef.flip_x = false;
	refdef.flip_y = true;
	refdef.flip_z = false;
	R_BeginFrame();
	R_RenderFrame(refdef);
	R_EndFrame();
	xglReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	IMG_WriteTGA("env/dynamic_ny.tga", buffer, width, height, 3);
	
	refdef.view_angles.set(-90, 90, 0);
	refdef.flip_x = false;
	refdef.flip_y = false;
	refdef.flip_z = true;
	R_BeginFrame();
	R_RenderFrame(refdef);
	R_EndFrame();
	xglReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	IMG_WriteTGA("env/dynamic_pz.tga", buffer, width, height, 3);
	
	refdef.view_angles.set(90, 90, 0);
	refdef.flip_x = false;
	refdef.flip_y = true;
	refdef.flip_z = false;
	R_BeginFrame();
	R_RenderFrame(refdef);
	R_EndFrame();
	xglReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	IMG_WriteTGA("env/dynamic_nz.tga", buffer, width, height, 3);
	
	delete [] buffer;
	
	r_envmap = false;
}

static void	R_BenchCalcTangentSpaces_f()
{
	int start, end;
	r_mesh_c m;
	
	if(ri.Cmd_Argc() != 2)
	{
		ri.Com_Printf("usage: benchcalctangentspaces <vertices number>\n");
		return;
	}
	
	int vertexes_num = atoi(ri.Cmd_Argv(1));
	if(!vertexes_num)
		vertexes_num = 10000;
	
	m.fillVertexes(vertexes_num);
	
	m.indexes = std::vector<index_t>(vertexes_num*6);
	for(uint_t i=0; i<m.indexes.size(); i++)
	{
		m.indexes[i] = i % (vertexes_num-1);
	}
	
	start = ri.Sys_Milliseconds();
	m.calcTangentSpaces();
	end = ri.Sys_Milliseconds();
	ri.Com_Printf("calcTangentSpaces: %i\n", end - start);
	
	start = ri.Sys_Milliseconds();
	m.calcTangentSpaces2();
	end = ri.Sys_Milliseconds();
	ri.Com_Printf("calcTangentSpaces2: %i\n", end - start);
	
//	start = ri.Sys_Milliseconds();
//	m.calcTangentSpaces3();
//	end = ri.Sys_Milliseconds();
//	ri.Com_Printf("calcTangentSpaces3: %i\n", end - start);
}


static void 	R_Register()
{
	r_lefthand 		= ri.Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE );
	
	r_fullbright 		= ri.Cvar_Get("r_fullbright", "0", CVAR_NONE);
	r_draw2d 		= ri.Cvar_Get("r_draw2d", "1", CVAR_NONE);
	r_drawentities 		= ri.Cvar_Get("r_drawentities", "1", CVAR_ARCHIVE);
	r_drawworld 		= ri.Cvar_Get("r_drawworld", "1", CVAR_ARCHIVE);
	r_drawparticles		= ri.Cvar_Get("r_drawparticles", "1", CVAR_ARCHIVE);
	r_drawpolygons		= ri.Cvar_Get("r_drawpolygons", "1", CVAR_ARCHIVE);
	r_drawsky		= ri.Cvar_Get("r_drawsky", "0", CVAR_ARCHIVE);
	r_drawextra		= ri.Cvar_Get("r_drawextra", "1", CVAR_ARCHIVE);
	r_drawtranslucent	= ri.Cvar_Get("r_drawtranslucent", "1", CVAR_ARCHIVE);
	r_drawpostprocess	= ri.Cvar_Get("r_drawpostprocess", "1", CVAR_ARCHIVE);
	r_lerpmodels 		= ri.Cvar_Get("r_lerpmodels", "1", CVAR_NONE);
	r_speeds 		= ri.Cvar_Get("r_speeds", "0", CVAR_NONE);
	r_debug			= ri.Cvar_Get("r_debug", "1", CVAR_ARCHIVE);
	r_log	 		= ri.Cvar_Get("r_log", "0", CVAR_NONE);
	r_shadows 		= ri.Cvar_Get("r_shadows", "0", CVAR_ARCHIVE );
	r_shadows_alpha		= ri.Cvar_Get("r_shadows_alpha", "0.5", CVAR_ARCHIVE);
	r_shadows_nudge		= ri.Cvar_Get("r_shadows_nudge", "1", CVAR_ARCHIVE);
	r_lighting 		= ri.Cvar_Get("r_lighting", "1", CVAR_ARCHIVE);
	r_lighting_omni		= ri.Cvar_Get("r_lighting_omni", "1", CVAR_ARCHIVE);
	r_lighting_proj 	= ri.Cvar_Get("r_lighting_proj", "1", CVAR_ARCHIVE);
	r_lightmap		= ri.Cvar_Get("r_lightmap", "1", CVAR_ARCHIVE);
	r_lightscale		= ri.Cvar_Get("r_lightscale", "4", CVAR_ARCHIVE);
	r_znear			= ri.Cvar_Get("r_znear", "4.0", CVAR_ARCHIVE);
	r_zfar			= ri.Cvar_Get("r_zfar", "65536.0", CVAR_ARCHIVE);
	r_nobind 		= ri.Cvar_Get("r_nobind", "0", CVAR_NONE);
	r_picmip 		= ri.Cvar_Get("r_picmip", "0", CVAR_ARCHIVE);
	r_skymip 		= ri.Cvar_Get("r_skymip", "0", CVAR_ARCHIVE);

	r_showtris 		= ri.Cvar_Get("r_showtris", "0", CVAR_NONE);
	r_showbbox 		= ri.Cvar_Get("r_showbbox", "0", CVAR_NONE);
	r_showareas		= ri.Cvar_Get("r_showareas", "0", CVAR_NONE);
	r_showareaportals	= ri.Cvar_Get("r_showareaportals", "0", CVAR_NONE);
	r_shownormals 		= ri.Cvar_Get("r_shownormals", "0", CVAR_NONE);
	r_showtangents		= ri.Cvar_Get("r_showtangents", "0", CVAR_NONE);
	r_showbinormals		= ri.Cvar_Get("r_showbinormals", "0", CVAR_NONE);
	r_showinvisible		= ri.Cvar_Get("r_showinvisible", "0", CVAR_NONE);
	r_showlightbboxes	= ri.Cvar_Get("r_showlightbboxes", "0", CVAR_NONE);
	r_showlightscissors	= ri.Cvar_Get("r_showlightscissors", "0", CVAR_NONE);
	r_showlighttransforms	= ri.Cvar_Get("r_showlighttransforms", "0", CVAR_NONE);
	r_showentitybboxes	= ri.Cvar_Get("r_showentitybboxes", "0", CVAR_NONE);
	r_showentitytransforms	= ri.Cvar_Get("r_showentitytransforms", "0", CVAR_NONE);
	r_showcontacts		= ri.Cvar_Get("r_showcontacts", "0", CVAR_NONE);

	r_clear 		= ri.Cvar_Get("r_clear", "1", CVAR_NONE);
	r_cull 			= ri.Cvar_Get("r_cull", "1", CVAR_NONE);
	r_cullplanes		= ri.Cvar_Get("r_cullplanes", "1", CVAR_NONE);
	r_cullportals		= ri.Cvar_Get("r_cullportals", "1", CVAR_NONE);
	r_polyblend 		= ri.Cvar_Get("r_polyblend", "1", CVAR_ARCHIVE);
	r_flashblend 		= ri.Cvar_Get("r_flashblend", "0", CVAR_ARCHIVE);
	r_playermip 		= ri.Cvar_Get("r_playermip", "0", 0);
	r_texturemode 		= ri.Cvar_Get("r_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_NONE);
	r_polygonmode		= ri.Cvar_Get("r_polygonmode", "GL_FILL", CVAR_NONE);
	r_lockpvs 		= ri.Cvar_Get("r_lockpvs", "0", CVAR_NONE);
	r_vis 			= ri.Cvar_Get("r_vis", "1", CVAR_NONE);
	r_maxtexsize		= ri.Cvar_Get("r_maxtexsize", "1024", CVAR_ARCHIVE);
	r_finish		= ri.Cvar_Get("r_finish", "1", CVAR_ARCHIVE);
	r_subdivisions		= ri.Cvar_Get("r_subdivisions", "0", CVAR_ARCHIVE);
	r_flares		= ri.Cvar_Get("r_flares", "0", CVAR_ARCHIVE);
	r_flaresize		= ri.Cvar_Get("r_flaresize", "15", CVAR_NONE);
	r_flarefade		= ri.Cvar_Get("r_flarefade", "7", CVAR_NONE);
	r_octree		= ri.Cvar_Get("r_octree", "0", CVAR_ARCHIVE);
	r_bump_mapping		= ri.Cvar_Get("r_bump_mapping", "1", CVAR_ARCHIVE);
	r_gloss			= ri.Cvar_Get("r_gloss", "1", CVAR_ARCHIVE);
	r_parallax		= ri.Cvar_Get("r_parallax", "1", CVAR_ARCHIVE);
	r_cmds_max		= ri.Cvar_Get("r_cmds_max", "8192", CVAR_ARCHIVE);
	r_cmds_light_max	= ri.Cvar_Get("r_cmds_light_max", "8192", CVAR_ARCHIVE);
	r_cmds_translucent_max	= ri.Cvar_Get("r_cmds_translucent_max", "8192", CVAR_ARCHIVE);
	r_video_export		= ri.Cvar_Get("r_video_export", "0", CVAR_NONE);
	r_evalexpressions	= ri.Cvar_Get("r_evalexpressions", "1", CVAR_NONE);
	
	r_drawbuffer 		= ri.Cvar_Get("r_drawbuffer", "GL_BACK", CVAR_NONE);

	// opengl extensions
	r_arb_multitexture 	= ri.Cvar_Get("r_arb_multitexture", "1", CVAR_ARCHIVE);
	r_arb_texture_compression = ri.Cvar_Get("r_arb_texture_compression", "0", CVAR_ARCHIVE);
	r_arb_vertex_buffer_object = ri.Cvar_Get("r_arb_vertex_buffer_object", "1", CVAR_ARCHIVE);
	r_arb_occlusion_query	= ri.Cvar_Get("r_arb_occlusion_query", "1", CVAR_ARCHIVE);
	r_arb_texture_rectangle	= ri.Cvar_Get("r_arb_texture_rectangle", "1", CVAR_ARCHIVE);
	
	r_ext_texture3D		= ri.Cvar_Get("r_ext_texture3D", "1", CVAR_ARCHIVE);
	r_ext_compiled_vertex_array = ri.Cvar_Get("r_ext_compiled_vertex_array", "0", CVAR_ARCHIVE);
	r_ext_draw_range_elements = ri.Cvar_Get("r_ext_draw_range_elements", "1", CVAR_ARCHIVE);
	r_ext_texture_filter_anisotropic = ri.Cvar_Get("r_ext_texture_filter_anisotropic", "0", CVAR_ARCHIVE);
	r_ext_texture_filter_anisotropic_level = ri.Cvar_Get("r_ext_texture_filter_anisotropic_level", "1", CVAR_ARCHIVE);

	r_sgix_fbconfig		= ri.Cvar_Get("r_sgix_fbconfig", "1", CVAR_ARCHIVE);
	r_sgix_pbuffer		= ri.Cvar_Get("r_sgix_pbuffer", "1", CVAR_ARCHIVE);

	vid_fullscreen 		= ri.Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma 		= ri.Cvar_Get("vid_gamma", "1.0", CVAR_ARCHIVE);
	vid_mode 		= ri.Cvar_Get("vid_mode", "0", CVAR_ARCHIVE);
#ifdef _WIN32
	vid_gldriver 		= ri.Cvar_Get("vid_gldriver", "opengl32", CVAR_ARCHIVE);
#elif __linux__
	vid_gldriver 		= ri.Cvar_Get("vid_gldriver", "libGL.so.1", CVAR_ARCHIVE);
#else
	vid_gldriver 		= ri.Cvar_Get("vid_gldriver", "libGL.so", CVAR_ARCHIVE);
#endif
	vid_colorbits		= ri.Cvar_Get("vid_colorbits", "32", CVAR_ARCHIVE);
	vid_depthbits		= ri.Cvar_Get("vid_depthbits", "24", CVAR_ARCHIVE);
	vid_stencilbits		= ri.Cvar_Get("vid_stencilbits", "8", CVAR_ARCHIVE);
	
	vid_pbuffer_width	= ri.Cvar_Get("vid_pbuffer_width", "512", CVAR_ARCHIVE);
	vid_pbuffer_height	= ri.Cvar_Get("vid_pbuffer_height", "512", CVAR_ARCHIVE);
	vid_pbuffer_colorbits	= ri.Cvar_Get("vid_pbuffer_colorbits", "32", CVAR_ARCHIVE);
	vid_pbuffer_depthbits	= ri.Cvar_Get("vid_pbuffer_depthbits", "24", CVAR_ARCHIVE);
	vid_pbuffer_stencilbits	= ri.Cvar_Get("vid_pbuffer_stencilbits", "8", CVAR_ARCHIVE);
	
	vid_ref 		= ri.Cvar_Get("vid_ref", "glx", CVAR_ARCHIVE);

	ri.Cmd_AddCommand("imagelist", 		R_ImageList_f);	
	ri.Cmd_AddCommand("screenshot",		R_ScreenShot_f);
	ri.Cmd_AddCommand("modellist", 		R_Modellist_f);
	ri.Cmd_AddCommand("gl_strings",		R_Strings_f);
	ri.Cmd_AddCommand("shaderlist",		R_ShaderList_f);
	ri.Cmd_AddCommand("shadercachelist",	R_ShaderCacheList_f);
	ri.Cmd_AddCommand("shadersearch",	R_ShaderSearch_f);
	ri.Cmd_AddCommand("spirittest",		R_SpiritTest_f);
	ri.Cmd_AddCommand("skinlist",		R_SkinList_f);
	ri.Cmd_AddCommand("envmap",		R_EnvMap_f);
	
	ri.Cmd_AddCommand("benchcalctangentspaces",	R_BenchCalcTangentSpaces_f);
}

static bool 	R_SetMode()
{
	r_serr_e err;
	bool fullscreen;

	fullscreen = (bool)vid_fullscreen->getInteger();

	vid_fullscreen->isModified(false);
	vid_mode->isModified(false);

	if((err = (r_serr_e)GLimp_SetMode((int*)&vid.width, (int*)&vid.height, vid_mode->getInteger(), fullscreen)) == RSERR_OK)
	{
		gl_state.prev_mode = vid_mode->getInteger();
	}
	else
	{
		if(err == RSERR_INVALID_FULLSCREEN)
		{
			ri.Cvar_SetValue("vid_fullscreen", 0);
			vid_fullscreen->isModified(false);

			ri.Com_Printf("R_SetMode: fullscreen unavailable in this mode\n");
			
			if((err = (r_serr_e)GLimp_SetMode((int*)&vid.width, (int*)&vid.height, vid_mode->getInteger(), false)) == RSERR_OK)
				return true;
		}
		else if(err == RSERR_INVALID_MODE)
		{
			ri.Cvar_SetValue("r_mode", gl_state.prev_mode);
			vid_mode->isModified(false);
			
			ri.Com_Printf("R_SetMode: invalid mode\n" );
		}

		// try setting it back to something safe
		if((err = (r_serr_e)GLimp_SetMode((int*)&vid.width, (int*)&vid.height, gl_state.prev_mode, false)) != RSERR_OK)
		{
			ri.Com_Printf("R_SetMode: could not revert to safe mode\n");
			return false;
		}
	}
	return true;
}

void	R_InitTree(const std::string &name)
{
#if 1
	r_world_tree = new r_bsptree_c("maps/" + name);
#else
	r_world_tree = new r_proctree_c("maps/" + name + ".proc");
#endif

	r_entity_t s;
	s.model = 0;
	r_world_entity = r_entity_c(s);
}

void	R_ShutdownTree()
{
	if(r_world_tree)
		delete r_world_tree;
		
	r_world_tree = NULL;
}


static void	R_InitQuadIndexes()
{
	r_quad_indexes.push_back(0);
	r_quad_indexes.push_back(1);
	r_quad_indexes.push_back(2);
	r_quad_indexes.push_back(0);
	r_quad_indexes.push_back(2);
	r_quad_indexes.push_back(3);
}


bool 	R_Init(void *hinstance, void *hWnd)
{	
	ri.Com_Printf("------- R_Init -------\n");

	ri.Com_Printf("ref_gl version: "REF_VERSION"\n");

	R_Register();
	
	// initialize our QGL dynamic bindings
	if(!XGL_Init(vid_gldriver->getString()))
	{
		XGL_Shutdown();
        	ri.Com_Printf("R_Init: could not load \"%s\"\n", vid_gldriver->getString());
		return false;
	}
	
	if(r_debug->isModified())
	{
		r_debug->isModified(false);
		XGL_EnableDebugging(r_debug->getInteger() >= 0);
	}
	
	// initialize OS-specific parts of OpenGL
	if(!GLimp_Init(hinstance, hWnd))
	{
		return false;
	}

	// set our "safe" modes
	gl_state.prev_mode = 3;

	// create the window and set up the context
	if(!R_SetMode())
	{
		XGL_Shutdown();
		ri.Com_Printf("R_Init: could not R_SetMode()\n");
		return false;
	}

	// get our various GL strings
	gl_config.vendor_string = (const char*)xglGetString (GL_VENDOR);
	ri.Com_Printf("GL_VENDOR: %s\n", gl_config.vendor_string );
	
	gl_config.renderer_string = (const char*)xglGetString (GL_RENDERER);
	ri.Com_Printf("GL_RENDERER: %s\n", gl_config.renderer_string );
	
	gl_config.version_string = (const char*)xglGetString (GL_VERSION);
	ri.Com_Printf("GL_VERSION: %s\n", gl_config.version_string );
	
	gl_config.extensions_string = (const char*)xglGetString (GL_EXTENSIONS);
	ri.Com_Printf("GL_EXTENSIONS: %s\n", gl_config.extensions_string );

	// grab optional OpenGL extensions
	XGL_InitExtensions();
	
	// setup shadow mapping support
	GLimp_InitPbuffer(false, true);		RB_CheckForError();
	GLimp_ActivatePbuffer();		RB_CheckForError();
//	xglClearColor(0.6, 0.1, 0.91, 0.0);
	xglClearColor(0.3, 0.3, 0.3, 1.0);	RB_CheckForError();
	xglEnable(GL_DEPTH_TEST);		RB_CheckForError();
	{
		//TODO recode
		
        	GLint depth_bits;
        	xglGetIntegerv(GL_DEPTH_BITS, & depth_bits);
        
        	if(depth_bits == 16)
			r_depth_format = GL_DEPTH_COMPONENT16_ARB;
        	else
			r_depth_format = GL_DEPTH_COMPONENT24_ARB;
	}
	GLimp_DeactivatePbuffer();		RB_CheckForError();

	// setup image system
	R_InitImages();				RB_CheckForError();
		
        // setup Q3A style shader system
	R_InitShaders();			RB_CheckForError();
	
	// setup skin system
	R_InitSkins();				RB_CheckForError();

	// setup MD5 animation system
	R_InitAnimations();			RB_CheckForError();
	
	// setup quad indices for 2d drawing
	R_InitQuadIndexes();

	// setup default images
	R_InitDraw();				RB_CheckForError();
	
	R_InitEntities();
	
	R_InitLights();
	
	R_InitParticles();			RB_CheckForError();
	R_InitPolys();				RB_CheckForError();
	
	// setup sky
	//R_InitSkyDome();
	
	// setup scenegraph
	//R_InitTree();
	
	// setup OpenGL renderer backend
	RB_InitBackend();			RB_CheckForError();
	
	RB_CheckForError();
	
	ri.Com_Printf("------- R_Init completed-------\n");

	return true;
}

void 	R_Shutdown()
{		
	ri.Com_Printf("------- R_Shutdown -------\n");
	
	ri.Cmd_RemoveCommand("imagelist");
	ri.Cmd_RemoveCommand("screenshot");
	ri.Cmd_RemoveCommand("modellist");
	ri.Cmd_RemoveCommand("gl_strings");
	ri.Cmd_RemoveCommand("shaderlist");
	ri.Cmd_RemoveCommand("shadercachelist");
	ri.Cmd_RemoveCommand("shadersearch");
	ri.Cmd_RemoveCommand("spirittest");
	ri.Cmd_RemoveCommand("skinlist");
	ri.Cmd_RemoveCommand("envmap");

	R_ShutdownTree();
	
	R_ShutdownLights();
	
	R_ShutdownEntities();
	
	R_ShutdownModels();
	
	R_ShutdownAnimations();

	RB_ShutdownBackend();
	
	R_ShutdownSkins();
	
	R_ShutdownShaders();

	R_ShutdownImages();

	GLimp_Shutdown();
	
	XGL_Shutdown();
}




#if 0
static void 	R_DrawBeam( r_entity_t *e )
{
#define NUM_BEAM_SEGS 6

	int	i;
	float r, g, b;

	vec3_t perpvec;
	vec3_t direction, normalized_direction;
	vec3_t	start_points[NUM_BEAM_SEGS], end_points[NUM_BEAM_SEGS];
	vec3_t oldorigin, origin;

	oldorigin[0] = e->oldorigin[0];
	oldorigin[1] = e->oldorigin[1];
	oldorigin[2] = e->oldorigin[2];

	origin[0] = e->origin[0];
	origin[1] = e->origin[1];
	origin[2] = e->origin[2];


	normalized_direction[0] = direction[0] = oldorigin[0] - origin[0];
	normalized_direction[1] = direction[1] = oldorigin[1] - origin[1];
	normalized_direction[2] = direction[2] = oldorigin[2] - origin[2];

	if ( Vector3_Normalize( normalized_direction ) == 0 )
		return;

	Vector3_Perpendicular( perpvec, normalized_direction );
	Vector3_Scale( perpvec, e->frame / 2, perpvec );


	for ( i = 0; i < 6; i++ )
	{
		RotatePointAroundVector( start_points[i], normalized_direction, perpvec, (360.0/NUM_BEAM_SEGS)*i );
		Vector3_Add( start_points[i], origin, start_points[i] );
		Vector3_Add( start_points[i], direction, end_points[i] );
	}

	
	r = ( r_8to24table[e->skin_num & 0xFF] ) & 0xFF;
	g = ( r_8to24table[e->skin_num & 0xFF] >> 8 ) & 0xFF;
	b = ( r_8to24table[e->skin_num & 0xFF] >> 16 ) & 0xFF;

	r *= 1/255.0F;
	g *= 1/255.0F;
	b *= 1/255.0F;
	
	Vector3_Set (e->color, r, g, b);


	//glColor4fv( r, g, b, e->color[3] );
	glColor4fv (e->color);

	glBegin( GL_TRIANGLE_STRIP );
	for ( i = 0; i < NUM_BEAM_SEGS; i++ )
	{
		glVertex3fv( start_points[i] );
		glVertex3fv( end_points[i] );
		glVertex3fv( start_points[(i+1)%NUM_BEAM_SEGS] );
		glVertex3fv( end_points[(i+1)%NUM_BEAM_SEGS] );
	}
	glEnd();
}
#endif



bool	R_SetupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name)
{
	if(name.empty())
		return false;
		
	tag.origin.clear();
	tag.quat.identity();
	
	r_model_c *mod = R_GetModelByNum(ent.model);
	if(!mod)
		return false;
	else
		return mod->setupTag(tag, ent, name);
}


bool	R_SetupAnimation(int model, int animation)
{
	r_skel_animation_c *anim = R_GetAnimationByNum(animation);
	if(!anim)
	{
		ri.Com_Printf("R_SetupAnimation: couldn't get animation by number");
		return false;
	}

	r_model_c *mod = R_GetModelByNum(model);
	if(!mod)
		return false;
	else
		return mod->setupAnimation(anim);
}



static void	R_ClearScene()
{
//	r_entities.clear();
//	r_lights.clear();
	r_particles_num = 0;
	r_polys_num = 0;
//	r_contacts.clear();
}

static void	R_AddEntity(int index, const r_entity_t &shared)
{
//	ri.Com_DPrintf("R_AddEntity(%i)\n", index);
	
	r_entity_c* ent = R_GetEntityByNum(index);
	
	if(ent != NULL)
		delete ent;

	ent = new r_entity_c(shared);
	
	r_entities[index] = ent;
}

static void	R_UpdateEntity(int index, const r_entity_t &shared)
{
//	ri.Com_DPrintf("R_UpdateEntity(%i)\n", index);

	r_entity_c* ent = R_GetEntityByNum(index);
	
	if(ent != NULL)
		delete ent;
	
	ent = new r_entity_c(shared);
	
	r_entities[index] = ent;
}

static void	R_RemoveEntity(int index)
{
//	ri.Com_DPrintf("R_RemoveEntity(%i)\n", index);

	r_entity_c* ent = R_GetEntityByNum(index);
	
	if(ent)
		delete ent;
	
	r_entities[index] = NULL;
}


static void	R_AddLight(int index, const r_entity_t &shared, r_light_type_t type)
{
//	ri.Com_DPrintf("R_AddLight(%i)\n", index);
	
	r_light_c* light = R_GetLightByNum(index);
	
	if(light)
		delete light;
		
	light = new r_light_c(shared, type);
	
	r_lights[index] = light;
}

static void	R_UpdateLight(int index, const r_entity_t &shared, r_light_type_t type)
{
//	ri.Com_DPrintf("R_UpdateLight(%i)\n", index);

	r_light_c* light = R_GetLightByNum(index);
	
	if(light)
		light->update(shared, type);
	else
		light = new r_light_c(shared, type);
	
	r_lights[index] = light;
	
}

static void	R_RemoveLight(int index)
{
//	ri.Com_DPrintf("R_RemoveLight(%i)\n", index);

	r_light_c* light = R_GetLightByNum(index);
	
	if(light)
		delete light;
	
	r_lights[index] = NULL;
}

static void	R_AddParticle(const r_particle_t &part)
{
	if(r_particles_num >= MAX_PARTICLES)
		return;
		
	r_particles[r_particles_num++] = part;
}

static void	R_AddPoly(const r_poly_t &poly)
{
	if(r_polys_num >= MAX_POLYS)
		return;
		
	r_polys[r_polys_num++] = poly;
}

static void	R_AddContact(const r_contact_t &contact)
{
	if(r_contacts.size() < 1024)
		r_contacts.push_back(contact);
	else
		r_contacts.clear();
}



#ifdef __cplusplus
extern "C" {
#endif


ref_export_t 	GetRefAPI(ref_import_t rimp)
{
	ref_export_t	re;

	ri = rimp;

	re.api_version 			= REF_API_VERSION;

	re.Init       			= R_Init;

	re.Shutdown			= R_Shutdown;

	re.R_BeginRegistration 		= R_BeginRegistration;

	re.R_RegisterModel		= R_RegisterModelExp;
	re.R_RegisterAnimation		= R_RegisterAnimationExp;
	re.R_RegisterSkin		= R_RegisterSkinExp;
	re.R_RegisterShader		= R_RegisterShaderExp;
	re.R_RegisterPic		= R_RegisterPicExp;
	re.R_RegisterParticle		= R_RegisterParticleExp;
	re.R_RegisterLight		= R_RegisterLightExp;
	re.R_EndRegistration 		= R_EndRegistration;

	re.R_BeginFrame			= R_BeginFrame;
	re.R_RenderFrame		= R_RenderFrame;
	re.R_EndFrame			= R_EndFrame;

	re.R_DrawPic			= R_DrawPicExp;
	re.R_DrawStretchPic 		= R_DrawStretchPicExp;
	re.R_DrawFill			= R_DrawFill;
	
	re.R_ClearScene			= R_ClearScene;
	
	re.R_AddEntity			= R_AddEntity;
	re.R_UpdateEntity		= R_UpdateEntity;
	re.R_RemoveEntity		= R_RemoveEntity;
	
	re.R_AddLight			= R_AddLight;
	re.R_UpdateLight		= R_UpdateLight;
	re.R_RemoveLight		= R_RemoveLight;
	
	re.R_AddParticle		= R_AddParticle;
	re.R_AddPoly			= R_AddPoly;
	re.R_AddContact			= R_AddContact;
	
	re.R_SetupTag			= R_SetupTag;
	re.R_SetupAnimation		= R_SetupAnimation;
		
	re.AppActivate 		   	= GLimp_AppActivate;
	
	Swap_Init();

	return re;
}

#ifdef __cplusplus
}
#endif


#ifndef REF_HARD_LINKED
// this is only here so the functions in x_shared.c and x_shwin.c can link
void 	Com_Error(err_type_e type, const char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	ri.Com_Error(type, "%s", text);
}

void	Com_Printf(const char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	ri.Com_Printf("%s", text);
}

#endif






