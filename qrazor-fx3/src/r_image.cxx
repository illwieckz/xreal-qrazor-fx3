/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2002 Marco Corbetta <ivy_mantled@yahoo.com>
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
#include "roq.h"
 
std::vector<r_image_c*>	r_images;
std::vector<r_image_c*>	r_images_lm;

int		r_filter_min = GL_LINEAR_MIPMAP_NEAREST;
int		r_filter_max = GL_LINEAR;


//static char*	r_cubemap_suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};
//static char*	r_cubemap_suf[6] = {"ft", "bk", "up", "dn", "rt", "lf"};

//static char*	r_cubemap_suf[6] = {"rt", "lf", "up", "dn", "bk", "ft"};
static char*	r_cubemap_suf[6] = {"px", "nx", "py", "ny", "pz", "nz"};

static uint_t	r_yuv_to_rgba[1024 * 1024];

static void	R_ResampleTexture(unsigned *in, int inwidth, int inheight, unsigned *out, int outwidth, int outheight);

static r_image_c*	R_LoadImage(const std::string &name, void *data, uint_t width, uint_t height, uint_t flags, r_image_upload_type_e upload_type);

r_image_c::r_image_c(uint_t target, const std::string &name, uint_t width, uint_t height, uint_t flags, roq_info_t *roq, bool global)
{
//	ri.Com_Printf("r_image_c::ctor: '%s'\n", name.c_str());

	_name			= name;
	_target			= target;
	_width			= width;
	_height			= height;
	_flags			= flags;
	
	xglGenTextures(1, &_id);
	
	_registration_sequence	= r_registration_sequence;
	_roq			= roq;
	
	// find free image slot
	if(global)
	{
		if(flags & IMAGE_LIGHTMAP)
		{
			r_images_lm.push_back(this);
		}
		else
		{
			std::vector<r_image_c*>::iterator ir = find(r_images.begin(), r_images.end(), static_cast<r_image_c*>(NULL));
	
			if(ir != r_images.end())
				*ir = this;
			else
				r_images.push_back(this);
		}
	}
}

r_image_c::~r_image_c()
{
	if(_id != 0)
	{
		xglDeleteTextures(1, &_id);
	}
	
	if(_roq)
	{
		ri.Roq_Close(_roq);
	}
}

void	r_image_c::updateTexture()
{
	if(!_roq)
		return;
	
	int newread = 0;
	
	if(_roq->lastframe_time < 0)
	{
		// first frame
		_roq->lastframe_time = ri.Sys_Milliseconds();
		ri.Roq_ReadVideo(_roq);
		newread++;
	
	}
	else if(_roq->frame_num >= (int)_roq->num_frames)
	{
		//loop video
		ri.Roq_ResetStream(_roq);
		
	
	}
	else
	{
		//read frames untill we have catched up with the curent time
		while((ri.Sys_Milliseconds() - _roq->lastframe_time) > (1000/30))
		{
			_roq->lastframe_time = ri.Sys_Milliseconds();
			ri.Roq_ReadVideo(_roq);
			newread++;
		}
	}

	if(newread)
	{
		convertColors();
		
		xglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _roq->width, _roq->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, r_yuv_to_rgba);
	}
}


void	r_image_c::convertColors()
{
	int i, j, y1, y2, u, v;
	byte *pa, *pb, *pc, *c;

	pa = _roq->y[0];
	pb = _roq->u[0];
	pc = _roq->v[0];
	c = (byte*)&r_yuv_to_rgba[0];


#define limit(x) ((((x) > 0xffffff) ? 0xff0000 : (((x) <= 0xffff) ? 0 : (x) & 0xff0000)) >> 16)
	for(j = 0; j < _roq->height; j++)
	{
		for(i = 0; i < _roq->width/2; i++)
		{
			int r, g, b;
			y1 = *(pa++);
			y2 = *(pa++);
			
			u = pb[i] - 128;
			v = pc[i] - 128;
			
			y1 *= 65536;
			y2 *= 65536;
			
			r = 91881 * v;
			g = -22554 * u + -46802 * v;
			b = 116130 * u;

			(*c++) = limit(r + y1);
			(*c++) = limit(g + y1);
			(*c++) = limit(b + y1);
			(*c++) = 255; //alpha
			(*c++) = limit(r + y2);
			(*c++) = limit(g + y2);
			(*c++) = limit(b + y2);
			(*c++) = 255; //alpha
		}
		
		if(j & 0x01)
		{
			pb += _roq->width/2;
			pc += _roq->width/2;
		}
	}
}


#define GRSIZE 64

static void 	R_InitDefaultImage()
{
	ri.Com_Printf("regenerating '_default' ...\n");
	
	int width = 64;
	int height = 64;	
	byte *data = new byte[width*height*4]; //rgb

	// create the black and white chess default texture and 3 pixels
	// size borders (more or less the default one created by d3)
	memset(data, 0, width*height*4);

	byte *data_p = data;
	
	for(int k=0; k<height; k++)
	{		
		if((k & (GRSIZE-1))==(GRSIZE-1))		
		{
			// make big borders
			memset(&data_p[-(width*4)*2], 255, width*4);					
			memset(&data_p[-(width*4)], 255, width*4);					
			memset(data_p, 255, width*4);					
			
			data_p += width*4;
		}
		else
		{
			for(int j=0; j<width; j+= GRSIZE)
			{			
				data_p[0]   = 255;	data_p[1]   = 255;	data_p[2]   = 255;
				data_p[4+0] = 255;	data_p[4+1] = 255;	data_p[4+2] = 255;
				data_p[8+0] = 255;	data_p[8+1] = 255;	data_p[8+2] = 255;
				
				data_p += GRSIZE*4;
			} //j					
		}
	} //k
	
	r_img_default = R_LoadImage("_default", (byte*)data, width, height, IMAGE_NONE, IMAGE_UPLOAD_COLORMAP);
	
	delete [] data;
}

static void	R_InitWhiteImage()
{
	ri.Com_Printf("regenerating '_white' ...\n");

	byte	data[32][32][4];
	
	memset(data, 255, sizeof(data));
	
	r_img_white = R_LoadImage("_white", (byte*)data, 32, 32, IMAGE_NONE, IMAGE_UPLOAD_COLORMAP);
}

static void	R_InitBlackImage()
{
	ri.Com_Printf("regenerating '_black' ...\n");

	byte	data[32][32][4];
	
	memset(data, 255, sizeof(data));
	
	r_img_black = R_LoadImage("_black", (byte*)data, 32, 32, IMAGE_NONE, IMAGE_UPLOAD_COLORMAP);
}


static void	R_InitFlatImage()
{
	ri.Com_Printf("regenerating '_flat' ...\n");

	byte	data[32][32];
	
	memset(data, 128, sizeof(data));
	
	r_img_flat = R_LoadImage("_flat", (byte*)data, 32, 32, IMAGE_NORMALMAP, IMAGE_UPLOAD_BUMPMAP);
}

static void	R_InitQuadraticImage()
{
	ri.Com_Printf("regenerating '_quadratic' ...\n");
	
	//FIXME

	byte	data[32][32][4];
	
	memset(data, 255, sizeof(data));
	
	r_img_quadratic = R_LoadImage("_quadratic", (byte*)data, 32, 32, IMAGE_NONE, IMAGE_UPLOAD_COLORMAP);
}

static void	R_InitWhiteCubeMapImage()
{
	ri.Com_Printf("regenerating '_cubemap_white' ...\n");

	int		format;
	int		format_internal;
	
	r_image_c	*image = NULL;
	
	byte	data[32][32][3];
	
	memset(data, 255, sizeof(data));
	
	//
	// create image
	//
	image = new r_image_c(GL_TEXTURE_CUBE_MAP_ARB, "_cubemap_white", 32, 32, IMAGE_NOMIPMAP | IMAGE_NOPICMIP, NULL);
	
	
	//
	// bind texture
	//
	RB_Bind(image);
	
	
	//
	// setup formats
	//
	if(gl_config.arb_texture_compression)
		format_internal = GL_COMPRESSED_RGB_ARB;
	else
		format_internal = GL_RGB;
		
	format = GL_RGB;
		
	//
	// set texture filter type
	//
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, r_filter_max);
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, r_filter_max);
	
	
	//
	// set texture wrap type
	//
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	
	//
	// create cube sides
	//
	for(int i=0; i<6; i++)
	{
		xglTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB+i, 0, format_internal, 32, 32, 0, format, GL_UNSIGNED_BYTE, data);
	}	
	
	r_img_cubemap_white = image;
}

static void	R_GetCubeVector(int i, int cubesize, int x, int y, float *vector)
{
	float s, t, sc, tc, mag;

	s = ((float)x + 0.5) / (float)cubesize;
	t = ((float)y + 0.5) / (float)cubesize;
	sc = s*2.0 - 1.0;
	tc = t*2.0 - 1.0;

	switch (i)
	{
		case 0:
			vector[0] = 1.0;
			vector[1] = -tc;
			vector[2] = -sc;
			break;
			
		case 1:
			vector[0] = -1.0;
			vector[1] = -tc;
			vector[2] = sc;
			break;
			
		case 2:
			vector[0] = sc;
			vector[1] = 1.0;
			vector[2] = tc;
			break;
			
		case 3:
			vector[0] = sc;
			vector[1] = -1.0;
			vector[2] = -tc;
			break;
			
		case 4:
			vector[0] = sc;
			vector[1] = -tc;
			vector[2] = 1.0;
			break;
			
		case 5:
			vector[0] = -sc;
			vector[1] = -tc;
			vector[2] = -1.0;
			break;
	}

	mag = 1.0/X_sqrt(vector[0]*vector[0] + vector[1]*vector[1] + vector[2]*vector[2]);
	vector[0] *= mag;
	vector[1] *= mag;
	vector[2] *= mag;
}


/*
==================
R_InitNormalizationCubeMapTexture
Tr3B - recoded from Nvidia's md2shader demo
==================
*/
static void	R_InitNormalizationCubeMapImage(int size)
{
	ri.Com_Printf("regenerating '_cubemap_normal' ...\n");

	int		i;
	int 		x, y;
	float		vector[3];
	int		format;
	int		format_internal;
	
	r_image_c	*image = NULL;
	byte		*pic = NULL;
	
		
	extern byte	R_RangeCompress(const float f);

	pic = new byte[size*size*3];
	
	//
	// create image
	//
	image = new r_image_c(GL_TEXTURE_CUBE_MAP_ARB, "_cubemap_normal", size, size, IMAGE_NOMIPMAP | IMAGE_NOPICMIP, NULL);
	
	
	xglEnable(GL_TEXTURE_CUBE_MAP_ARB);
	
	//
	// bind texture
	//
	RB_Bind(image);
	
	
	//
	// setup formats
	//
	if(gl_config.arb_texture_compression)
		format_internal = GL_COMPRESSED_RGB_ARB;
	else
		format_internal = GL_RGB;
		
	format = GL_RGB;
		
	//
	// set texture filter type
	//
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, r_filter_max);
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, r_filter_max);
	
	
	//
	// set texture wrap type
	//
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	
	//
	// create cube sides
	//
	for (i = 0; i < 6; i++)
	{
		for (y = 0; y < size; y++)
		{
			for (x = 0; x < size; x++)
			{
				R_GetCubeVector(i, size, x, y, vector);
				
				pic[3*(y*size+x) + 0] = R_RangeCompress(vector[0]);	// do range compress
				pic[3*(y*size+x) + 1] = R_RangeCompress(vector[1]);
				pic[3*(y*size+x) + 2] = R_RangeCompress(vector[2]);		
			}
		}

		xglTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB+i, 0, format_internal, size, size, 0, format, GL_UNSIGNED_BYTE, pic);
	}

	xglDisable(GL_TEXTURE_CUBE_MAP_ARB);

	delete [] pic;
		
	
	r_img_cubemap_normal = image;
}

static void	R_InitSkyCubeMapImage()
{
	// default sky texture
	r_img_cubemap_sky = r_img_cubemap_normal;
	
	//r_sky_cubemap_texture = R_LoadCubeMap("env/unit1");
}


static void	R_InitNoFalloffImage()
{
	ri.Com_Printf("regenerating '_nofalloff' ...\n");

	byte	data[32][32][4];
	
	memset(data, 255, sizeof(data));
	
	r_img_nofalloff = R_LoadImage("_nofalloff", (byte*)data, 32, 0, IMAGE_CLAMP, IMAGE_UPLOAD_COLORMAP);
}


/*
==================
R_Init3DAttenuationTexture
Tr3B - recoded from Omnio engine
==================
*/
/*
static void	R_Init3DAttenuationImage(int atten_volume_size)
{
	ri.Com_Printf("regenerating '_attenuation_3d' ...\n");

	int		format;
	int		format_internal;

	byte *data = new byte[atten_volume_size*atten_volume_size*atten_volume_size];
	
	//
	// create image
	//
	r_image_c *image = new r_image_c(GL_TEXTURE_3D, "_attenuation_3d", atten_volume_size, atten_volume_size, IMAGE_NONE, NULL);
	
			
	xglEnable(GL_TEXTURE_3D);
	
	
	//
	// bind texture
	//
	RB_Bind(image);
	
		
	//
	// process attenuation volume
	//	
	float centerx, centery, centerz, radiussq;
	int s,t,r;

	centerx = atten_volume_size/2.0f;
	centery = atten_volume_size/2.0f;
	centerz = atten_volume_size/2.0f;
	radiussq = atten_volume_size/2.0f;
	radiussq = radiussq*radiussq;

	for(s=0; s<atten_volume_size; s++) 
	{
		for(t=0; t<atten_volume_size; t++) 
		{
			for(r=0; r<atten_volume_size; r++) 
			{
				float DistSq = X_sqr(s-centerx) + X_sqr(t-centery) + X_sqr(r-centerz);
				
				if(DistSq < (radiussq)) 
				{ 
					byte value;
					float FallOff = (radiussq - DistSq) / (radiussq);
					//float FallOff = ((radiussq*1.4f) / DistSq);
					FallOff*= FallOff;
					FallOff*=256.0f;
					if (FallOff>255)
						FallOff=255;					
					value =(unsigned char)(FallOff);
					data[r * atten_volume_size * atten_volume_size *1+ t * atten_volume_size *1+ s *1+ 0] = value;
					//data[r * atten_volume_size * atten_volume_size *4+ t * atten_volume_size *4+ s *4+ 1] = value;
					//data[r * atten_volume_size * atten_volume_size *4+ t * atten_volume_size *4+ s *4+ 2] = value;
					//data[r * atten_volume_size * atten_volume_size *4+ t * atten_volume_size *4+ s *4+ 3] = value;
				} 
				else 
				{
					data[r * atten_volume_size * atten_volume_size *1+ t * atten_volume_size *1+ s *1+ 0] = 0;
					//data[r * atten_volume_size * atten_volume_size *4+ t * atten_volume_size *4+ s *4+ 1] = 0;
					//data[r * atten_volume_size * atten_volume_size *4+ t * atten_volume_size *4+ s *4+ 2] = 0;
					//data[r * atten_volume_size * atten_volume_size *4+ t * atten_volume_size *4+ s *4+ 3] = 0;
				}

			}
		}
	}
	
	
	//
	// setup formats
	//
	if(gl_config.arb_texture_compression)
		format_internal = GL_COMPRESSED_RGB_ARB;
	else
		format_internal = GL_RGB;
		
	format = GL_RGB;
	

	//glTexImage3DEXT(GL_TEXTURE_3D, 0, 4,atten_volume_size, atten_volume_size, atten_volume_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	xglTexImage3D(GL_TEXTURE_3D, 0, format_internal, atten_volume_size, atten_volume_size, atten_volume_size, 0, format, GL_UNSIGNED_BYTE, data);

	//
	// set texture wrap type
	//
	xglTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	xglTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	xglTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	
	//
	// set texture filter type
	//
	xglTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, r_filter_max);
	xglTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, r_filter_max);	
	

	xglDisable(GL_TEXTURE_3D);
	
	delete [] data;	
	
	r_img_attenuation_3d = image;
}
*/


static void	R_InitLightViewDepthImage()
{
	ri.Com_Printf("regenerating '_lightview_depth' ...\n");

	r_image_c *image = new r_image_c(GL_TEXTURE_2D, "_lightview_depth", vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), IMAGE_NONE, NULL);
	
	image->bind();
	
	xglTexImage2D(GL_TEXTURE_2D, 0, r_depth_format, image->getWidth(), image->getHeight(), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);	RB_CheckForError();
	
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_filter_max);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_filter_max);
	
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
	
	r_img_lightview_depth = image;
}

static void	R_InitLightViewColorImage()
{
	ri.Com_Printf("regenerating '_lightview_color' ...\n");

	r_image_c *image = new r_image_c(GL_TEXTURE_2D, "_lightview_color", vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), IMAGE_NONE, NULL);
	
	image->bind();

	xglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->getWidth(), image->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	r_img_lightview_color = image;
}

static void	R_InitCurrentRenderImage()
{
	ri.Com_Printf("regenerating '_currentrender' ...\n");

	r_image_c *image = new r_image_c(GL_TEXTURE_2D, "_currentrender", vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), IMAGE_NONE, NULL);
	
	image->bind();	RB_CheckForError();
	
	xglTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

	xglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->getWidth(), image->getHeight(), 0, GL_RGB, GL_FLOAT, 0);	RB_CheckForError();
//	xglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->getWidth(), image->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_filter_max);	RB_CheckForError();
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_filter_max);	RB_CheckForError();
	
//	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	RB_CheckForError();
//	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	RB_CheckForError();

	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
//	if(gl_config.ext_texture_filter_anisotropic)
//		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_ext_texture_filter_anisotropic_level->getInteger());
	
	r_img_currentrender = image;
}


void	R_InitImages()
{
	ri.Com_Printf("------- R_InitImages -------\n");
	
	r_registration_sequence = 1;
	
	//
	// support for dynamic maximum texture size
	//
	int	maxtexsize;
	xglGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtexsize);
	ri.Cvar_SetValue("r_maxtexsize", maxtexsize);
	
	//
	// support for anisotropic filtering
	//
	int	maxanisotropy;
	xglGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxanisotropy);
	ri.Cvar_SetValue("r_ext_texture_filter_anisotropic_level", maxanisotropy);
	
	//
	// setup default textures
	//
	R_InitDefaultImage();			RB_CheckForError();
	R_InitWhiteImage();			RB_CheckForError();
	R_InitBlackImage();			RB_CheckForError();
	R_InitFlatImage();			RB_CheckForError();
	R_InitQuadraticImage();			RB_CheckForError();

	R_InitWhiteCubeMapImage();		RB_CheckForError();
	R_InitNormalizationCubeMapImage(128);	RB_CheckForError();
	R_InitSkyCubeMapImage();		RB_CheckForError();
	
	R_InitNoFalloffImage();			RB_CheckForError();
//	R_Init3DAttenuationImage(32);		RB_CheckForError();
	
	R_InitLightViewDepthImage();		RB_CheckForError();
	R_InitLightViewColorImage();		RB_CheckForError();
	
	R_InitCurrentRenderImage();		RB_CheckForError();
}

void	R_ShutdownImages()
{
	X_purge<std::vector<r_image_c*> >(r_images);
	r_images.clear();
	
	X_purge<std::vector<r_image_c*> >(r_images_lm);
	r_images_lm.clear();
}


void	R_ImageList_f()
{
	r_image_c*	image;
	int		texels;
	char*	target;
	

	ri.Com_Printf("------------------\n");
	texels = 0;

	for(std::vector<r_image_c*>::const_iterator ir = r_images.begin(); ir != r_images.end(); ++ir)
	{
		image = *ir;
		
		if(image->getId() <= 0)
			continue;
			
		texels += image->getWidth()*image->getHeight();
		
		switch(image->getTarget())
		{
			case GL_TEXTURE_1D:
				target = "1D";
				break;
		
			case GL_TEXTURE_2D:
				target = "2D";
				break;
			
			case GL_TEXTURE_3D:
				target = "3D";
				break;
			
			case GL_TEXTURE_CUBE_MAP_ARB:
				target = "CUBE";
				break;
			
			default:
				target = "NA";
		}	
		
		ri.Com_Printf(" %4i %4i %4i %s\t '%s'\n", image->getWidth(), image->getHeight(), image->getId(), target, image->getName());
	}
	ri.Com_Printf("Total texel count (not counting mipmaps): %i\n", texels);
}


static void	R_ResampleTexture(unsigned *in, int inwidth, int inheight, unsigned *out, int outwidth, int outheight)
{
	int		i, j;
	unsigned	*inrow, *inrow2;
	unsigned	frac, fracstep;
	unsigned	p1[1024], p2[1024];
	byte		*pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth*0x10000/outwidth;

	frac = fracstep>>2;
	for(i=0 ; i<outwidth ; i++)
	{
		p1[i] = 4*(frac>>16);

		frac += fracstep;
	}
	frac = 3*(fracstep>>2);
	for(i=0 ; i<outwidth ; i++)
	{
		p2[i] = 4*(frac>>16);
		frac += fracstep;
	}

	for(i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(int)((i+0.25)*inheight/outheight);
		inrow2 = in + inwidth*(int)((i+0.75)*inheight/outheight);
		frac = fracstep >> 1;
		
		for(j=0 ; j<outwidth ; j++)
		{
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];

			((byte *)(out+j))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			((byte *)(out+j))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			((byte *)(out+j))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			((byte *)(out+j))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3])>>2;
		}
	}
}


static void	R_Resample8BitTexture(byte *in, int inwidth, int inheight, byte *out, int outwidth, int outheight)
{
	int		i, j;
	unsigned	char *inrow;
	unsigned	frac, fracstep;

	fracstep = inwidth*0x10000/outwidth;
	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(i*inheight/outheight);
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j+=4)
		{
			out[j] = inrow[frac>>16];
			frac += fracstep;
			out[j+1] = inrow[frac>>16];
			frac += fracstep;
			out[j+2] = inrow[frac>>16];
			frac += fracstep;
			out[j+3] = inrow[frac>>16];
			frac += fracstep;
		}
	}
}




/*
================
R_MipMap

Operates in place, quartering the size of the texture
================
*/
static void	R_MipMap(byte *in, int width, int height)
{
	int		i, j;
	byte	*out;

	width <<=2;
	height >>= 1;
	out = in;
	
	for(i=0; i<height; i++, in+=width)
	{
		for(j=0; j<width; j+=8, out+=4, in+=8)
		{
			out[0] = (in[0] + in[4] + in[width+0] + in[width+4])>>2;
			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}




/*
================
R_MipMapNormalmap

Tr3B - recoded from Tenebrae2
================
*/
static void	R_MipMapNormalmap(byte *in, int width, int height)
{
	int		i, j;
	byte	*out;
	//float	inv255	= 1.0f/255.0f;
	float	inv127	= 1.0f/127.0f;
	
	vec3_c n;
	float len;
	//float	g;

	width <<=2;
	height >>= 1;
	out = in;
	
	for(i=0; i<height; i++, in+=width)
	{
		for(j=0; j<width; j+=8, out+=4, in+=8)
		{
			n[0] = 	(inv127*in[0]-1.0)+
				(inv127*in[4]-1.0)+
				(inv127*in[width+0]-1.0)+
				(inv127*in[width+4]-1.0);
		 	
			n[1] = 	(inv127*in[1]-1.0)+
				(inv127*in[5]-1.0)+
				(inv127*in[width+1]-1.0)+
				(inv127*in[width+5]-1.0);
				
			n[2] =	(inv127*in[2]-1.0)+
				(inv127*in[6]-1.0)+
				(inv127*in[width+2]-1.0)+
				(inv127*in[width+6]-1.0);
	
			/*
	    		g = 	(inv255*in[3])+
				(inv255*in[7])+
				(inv255*in[width+3])+
				(inv255*in[width+7]);
			*/

			len = n.length();
			
			if(len)
			{
				n[0] /= len;
				n[1] /= len;
				n[2] /= len;
			}
			else
				n.set(0.0, 0.0, 1.0);
			
			out[0] = (byte)(128 + 127*n[0]);
			out[1] = (byte)(128 + 127*n[1]);
			out[2] = (byte)(128 + 127*n[2]);
			//out[3] = (byte)(g * 255.0/4.0);
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}


/*
================
R_GenNormalMap
Tr3B - recoded from Nvidia's SDK without array2 template code
================
*/
static void	R_GenNormalMap(byte *in, int width, int height, vec3_c &scale, vec3_c *out)
{
	if(scale[0] == 0.0f || scale[1] == 0.0f || scale[2] == 0.0f)
	{
		float a = float(width)/float(height);
		if(a < 1.f)
		{
			scale[0] = 1.0f;
			scale[1] = 1.0f/a;
		}
		else
		{
			scale[0] = a;
			scale[1] = 1.f;
		}
		scale[2] = 1.f;
	}
	
	for(int i=1; i < width-1; i++)
	{
		for(int j=1; j < height-1; j++)
		{
			vec3_c dfdi(2.f, 0.f, float(in[i+1 + j*width]     - in[(i-1) + j*width])/255.f);
			vec3_c dfdj(0.f, 2.f, float(in[i   + (j+1)*width] - in[i     + (j-1)*width])/255.f);
			vec3_c n(false);
			n.crossProduct(dfdi, dfdj);
			n *= scale;
			n.normalize();
			out[i + j*width] = n;
		}
	}
	
	// microsoft non-ansi c++ scoping concession
#if 1
	{
		// cheesy boundary cop-out
		for(int i=0; i < width; i++)
		{
			out[i + 0*width]   = out[i + 1*width];
			out[i + (height-1)*width] = out[i + (height-2)*width];
		}
		for(int j=0; j < height; j++)
		{
			out[0 + j*width]   = out[1 + j*width];
			out[(width-1) + j*width] = out[(width-2) + j*width];
		}
	}
#endif	
}



byte	R_RangeCompress(const float f)
{
	return byte(128 + 127*f);
}

static void	R_GenNormalMapTexture(vec3_c *in, int width, int height, byte *out)
{
#if 0
	GLshort *sip = (GLshort*)out;
    
	for(int i=0; i < height; i++)
	{
        	for(int j=0; j < width; j++)
        	{
        		const vec3_c n = in[i*width + j];
			
			*sip++ = (GLshort)(n[0] * 32767);
			*sip++ = (GLshort)(n[1] * 32767);
		}
	}
#else
	byte *sip = out;
    
	for(int i=0; i < height; i++)
	{
        	for(int j=0; j < width; j++)
        	{
        		const vec3_c n = in[i*width + j];
			
			*sip++ = R_RangeCompress(n[0]);
			*sip++ = R_RangeCompress(n[1]);
			*sip++ = R_RangeCompress(n[2]);
			*sip++ = R_RangeCompress(1.0);
		}
	}
#endif
}


static r_image_c*	R_UploadBumpMap(const std::string &name, byte *data, uint_t width, uint_t height, uint_t flags)
{	
	byte*		scaled;
	vec3_c*		scaled_normalmap_float;
	byte*		scaled_normalmap;
	uint_t		scaled_width;
	uint_t		scaled_height;
	
	vec3_c 		scale(1, 1, 0.2f);
	
	int		format;
	int		format_internal;
	

	for(scaled_width = 1; scaled_width < width ; scaled_width<<=1)
		;
			
	for(scaled_height = 1; scaled_height < height; scaled_height<<=1)
		;
	//
	// let people sample down the world textures for speed
	//
	if(!(flags & IMAGE_NOPICMIP))
	{
		scaled_width >>= r_picmip->getInteger();
		scaled_height >>= r_picmip->getInteger();
	}

	X_clamp(scaled_width, 1, (uint_t)r_maxtexsize->getInteger());
	X_clamp(scaled_height, 1, (uint_t)r_maxtexsize->getInteger());
	
	
	//
	// create image
	//
	r_image_c *image = new r_image_c(GL_TEXTURE_2D, name, scaled_width, scaled_height, flags, NULL);
	
	
	//
	// bind texture
	//
	RB_Bind(image);
	

	if((int)(scaled_width * scaled_height) > r_maxtexsize->getInteger() * r_maxtexsize->getInteger())
		ri.Com_Error(ERR_DROP, "R_UploadBumpMap: too big");

	scaled = new byte[scaled_width * scaled_height];
	scaled_normalmap_float = new vec3_c[scaled_width * scaled_height];
	scaled_normalmap = new byte[scaled_width * scaled_height*4];
	memset(scaled_normalmap, 0, sizeof(*scaled_normalmap));
	
	
	//
	// setup formats
	//
	if(gl_config.arb_texture_compression && !(flags & IMAGE_NOCOMPRESSION))
		format_internal = GL_COMPRESSED_RGBA_ARB;
	else
		format_internal = GL_RGBA;
		
	format = GL_RGBA;
	

	if(scaled_width == width && scaled_height == height)
	{
		if(flags & (IMAGE_NOMIPMAP))
		{
			memcpy(scaled, data, scaled_width*scaled_height);
			R_GenNormalMap((byte*)scaled, scaled_width, scaled_height, scale, scaled_normalmap_float);
			R_GenNormalMapTexture(scaled_normalmap_float, scaled_width, scaled_height, scaled_normalmap);
			xglTexImage2D(GL_TEXTURE_2D, 0, format_internal, scaled_width, scaled_height, 0, format, GL_UNSIGNED_BYTE, scaled_normalmap);
			goto r_upload_bumpmap_done;
		}
		
		
		memcpy (scaled, data, width*height);
	}
	else
	{
		R_Resample8BitTexture(data, width, height, scaled, scaled_width, scaled_height);
	}
	
	
	R_GenNormalMap((byte*)scaled, scaled_width, scaled_height, scale, scaled_normalmap_float);
	R_GenNormalMapTexture(scaled_normalmap_float, scaled_width, scaled_height, scaled_normalmap);
	
	xglTexImage2D(GL_TEXTURE_2D, 0, format_internal, scaled_width, scaled_height, 0, format, GL_UNSIGNED_BYTE, scaled_normalmap);
	
	
	if(!(flags & IMAGE_NOMIPMAP))
	{
		int		miplevel;

		miplevel = 0;
		while(scaled_width > 1 || scaled_height > 1)
		{
			R_MipMapNormalmap(scaled_normalmap, scaled_width, scaled_height);
			
			scaled_width >>= 1;
			scaled_height >>= 1;
			
			if(scaled_width < 1)
				scaled_width = 1;
				
			if(scaled_height < 1)
				scaled_height = 1;
				
			miplevel++;
			
			xglTexImage2D(GL_TEXTURE_2D, miplevel, format_internal, scaled_width, scaled_height, 0, format, GL_UNSIGNED_BYTE, scaled_normalmap);
			
		}
	}
r_upload_bumpmap_done:


	xglTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

	if(!(flags & IMAGE_NOMIPMAP))
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_filter_min);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_filter_max);
	}
	else
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_filter_max);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_filter_max);
	}
	
	if(flags & IMAGE_CLAMP)
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	
	if(gl_config.ext_texture_filter_anisotropic)
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_ext_texture_filter_anisotropic_level->getInteger());

	
	// free memory 
	delete [] scaled;
	delete [] scaled_normalmap_float;
	delete [] scaled_normalmap;
	
	return image;
}

static r_image_c*	R_UploadAlphaMap(const std::string &name, byte *data, uint_t width, uint_t height, uint_t flags)
{	
	byte*		scaled;
	uint_t		scaled_width;
	uint_t		scaled_height;
	
	int		format;
	int		format_internal;
	

	for(scaled_width = 1; scaled_width < width ; scaled_width<<=1)
		;
			
	for(scaled_height = 1; scaled_height < height; scaled_height<<=1)
		;
	//
	// let people sample down the world textures for speed
	//
	if(!(flags & IMAGE_NOPICMIP))
	{
		scaled_width >>= r_picmip->getInteger();
		scaled_height >>= r_picmip->getInteger();
	}

	X_clamp(scaled_width, 1, (uint_t)r_maxtexsize->getInteger());
	X_clamp(scaled_height, 1, (uint_t)r_maxtexsize->getInteger());
	
	
	//
	// create image
	//
	r_image_c *image = new r_image_c(GL_TEXTURE_2D, name, scaled_width, scaled_height, flags, NULL);
	
	
	//
	// bind texture
	//
	RB_Bind(image);
	

	if((int)(scaled_width * scaled_height) > r_maxtexsize->getInteger() * r_maxtexsize->getInteger())
		ri.Com_Error(ERR_DROP, "R_UploadBumpMap: too big");

	scaled = new byte[scaled_width * scaled_height];
	
	
	//
	// setup formats
	//
	if(gl_config.arb_texture_compression && !(flags & IMAGE_NOCOMPRESSION))
		format_internal = GL_COMPRESSED_ALPHA_ARB;
	else
		format_internal = GL_ALPHA;
		
	format = GL_ALPHA;
	

	if(scaled_width == width && scaled_height == height)
	{
		//if(flags & (IMAGE_NOMIPMAP))
		{
			xglTexImage2D(GL_TEXTURE_2D, 0, format_internal, scaled_width, scaled_height, 0, format, GL_UNSIGNED_BYTE, data);
			goto r_upload_alphamap_done;
		}
		
		
		memcpy(scaled, data, width*height);
	}
	else
	{
		R_Resample8BitTexture(data, width, height, scaled, scaled_width, scaled_height);
	}
	
	xglTexImage2D(GL_TEXTURE_2D, 0, format_internal, scaled_width, scaled_height, 0, format, GL_UNSIGNED_BYTE, scaled);
	
	/*
	if(!(flags & IMAGE_NOMIPMAP))
	{
		int		miplevel;

		miplevel = 0;
		while(scaled_width > 1 || scaled_height > 1)
		{
			R_MipMapNormalmap(scaled_normalmap, scaled_width, scaled_height);
			
			scaled_width >>= 1;
			scaled_height >>= 1;
			
			if(scaled_width < 1)
				scaled_width = 1;
				
			if(scaled_height < 1)
				scaled_height = 1;
				
			miplevel++;
			
			xglTexImage2D(GL_TEXTURE_2D, miplevel, format_internal, scaled_width, scaled_height, 0, format, GL_UNSIGNED_BYTE, scaled_normalmap);
			
		}
	}
	*/
r_upload_alphamap_done:


	xglTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

	/*
	if(!(flags & IMAGE_NOMIPMAP))
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_filter_min);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_filter_max);
	}
	else
	*/
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_filter_max);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_filter_max);
	}
	
	if(flags & IMAGE_CLAMP)
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	
	if(gl_config.ext_texture_filter_anisotropic)
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_ext_texture_filter_anisotropic_level->getInteger());

	
	// free memory 
	delete [] scaled;
	
	return image;
}

static r_image_c*	R_UploadColorMap(const std::string &name, uint_t *data, uint_t width, uint_t height, uint_t flags)
{	
	unsigned int*	scaled;			//32 bit pointer
	uint_t		scaled_width;
	uint_t		scaled_height;
	
	int		format;
	int		format_internal;
	

	for(scaled_width = 1; scaled_width < width ; scaled_width<<=1)
		;
			
	for(scaled_height = 1; scaled_height < height; scaled_height<<=1)
		;
	//
	// let people sample down the world textures for speed
	//
	if(!(flags & IMAGE_NOPICMIP))
	{
		scaled_width >>= r_picmip->getInteger();
		scaled_height >>= r_picmip->getInteger();
	}

	X_clamp(scaled_width, 1, (uint_t)r_maxtexsize->getInteger());
	X_clamp(scaled_height, 1, (uint_t)r_maxtexsize->getInteger());
	
	
	//
	// create image
	//
	r_image_c *image = new r_image_c(GL_TEXTURE_2D, name, scaled_width, scaled_height, flags, NULL);
	
	
	//
	// bind texture
	//
	RB_Bind(image);
	
	if((int)(scaled_width * scaled_height) > r_maxtexsize->getInteger() * r_maxtexsize->getInteger())
		ri.Com_Error(ERR_DROP, "R_UploadRGBA: too big");

	scaled = new unsigned[scaled_width * scaled_height * 4];

	//
	// setup formats
	//
	if(gl_config.arb_texture_compression && !(flags & IMAGE_NOCOMPRESSION))
		format_internal = GL_COMPRESSED_RGBA_ARB;
	else
		format_internal = GL_RGBA;
		
	format = GL_RGBA;
	
	
	if(scaled_width == width && scaled_height == height)
	{
		if(flags & (IMAGE_NOMIPMAP))
		{
			xglTexImage2D(GL_TEXTURE_2D, 0, format_internal, scaled_width, scaled_height, 0, format, GL_UNSIGNED_BYTE, data);
			goto r_upload_colormap_done;
		}
		
		memcpy(scaled, data, width*height*4);
	}
	else
	{
		R_ResampleTexture(data, width, height, scaled, scaled_width, scaled_height);
	}
	
	xglTexImage2D(GL_TEXTURE_2D, 0, format_internal, scaled_width, scaled_height, 0, format, GL_UNSIGNED_BYTE, scaled);
	
	if(!(flags & IMAGE_NOMIPMAP))
	{
		int		miplevel;

		miplevel = 0;
		while(scaled_width > 1 || scaled_height > 1)
		{
			
			if(flags & IMAGE_NORMALMAP)
			{
				R_MipMapNormalmap((byte*)scaled, scaled_width, scaled_height);
			}
			else
			{
				R_MipMap((byte*)scaled, scaled_width, scaled_height);
			}
			
			scaled_width >>= 1;
			scaled_height >>= 1;
			
			if(scaled_width < 1)
				scaled_width = 1;
				
			if(scaled_height < 1)
				scaled_height = 1;
				
			miplevel++;
			
			xglTexImage2D(GL_TEXTURE_2D, miplevel, format_internal, scaled_width, scaled_height, 0, format, GL_UNSIGNED_BYTE, scaled);
		}
	}
r_upload_colormap_done:


	xglTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

	//
	// set texture filter type
	//
	if(!(flags & IMAGE_NOMIPMAP))
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_filter_min);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_filter_max);
	}
	else
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_filter_max);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_filter_max);
	}
	
	//
	// set texture wrap type
	//
	if(flags & IMAGE_CLAMP)
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else if(flags & IMAGE_CLAMP_TO_EDGE)
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else if(flags & IMAGE_CLAMP_TO_BORDER)
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		xglTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, vec4_c(0, 0, 0, 1));
	}
	else
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	
	
	
	if(gl_config.ext_texture_filter_anisotropic)
		xglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_ext_texture_filter_anisotropic_level->getInteger());

	//
	// free memory 
	//
	delete [] scaled;
	
	return image;
}

static r_image_c*	R_UploadCubeMap(const std::string &name, uint_t flags)
{
	int		width, height;
	std::string	name_new;
	
	int		format;
	int		format_internal;

	//
	// create image
	//
	r_image_c* image = new r_image_c(GL_TEXTURE_CUBE_MAP_ARB, name, 0, 0, IMAGE_NOMIPMAP | IMAGE_NOPICMIP, NULL);
	
		
	//
	// bind texture
	//
	RB_Bind(image);
	
	
	//
	// setup formats
	//
	if(gl_config.arb_texture_compression && !(flags & IMAGE_NOCOMPRESSION))
		format_internal = GL_COMPRESSED_RGBA_ARB;
	else
		format_internal = GL_RGBA;
		
	format = GL_RGBA;
		
	//
	// set texture filter type
	//
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, r_filter_max);
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, r_filter_max);
	
	
	//
	// set texture wrap type
	//
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	

	//
	// create cube sides
	//
	for(int i=0; i<6; i++)
	{	
		byte* pic = NULL;
	
		name_new = name + '_' + r_cubemap_suf[i] + ".tga";
		IMG_LoadTGA(name_new, &pic, &width, &height);
		if(pic)
		{
			xglTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB +i, 0, format_internal, width, height, 0, format, GL_UNSIGNED_BYTE, pic);
			
			delete [] pic;
		}
	}
	
	return image;
}


static r_image_c*	R_UploadVideoMap(const std::string &name, roq_info_t *roq, uint_t width, uint_t height, uint_t flags)
{
	//
	// create image
	//
	r_image_c *image = new r_image_c(GL_TEXTURE_2D, name, width, height, flags, roq);
	
	
	//
	// bind texture
	//
	RB_Bind(image);
	
	
	//
	// set texture filter type
	//
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_filter_max);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_filter_max);
	
	
	//
	// set texture wrap type
	//
	if(flags & IMAGE_CLAMP)
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else if(flags & IMAGE_CLAMP_TO_EDGE)
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else if(flags & IMAGE_CLAMP_TO_BORDER)
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	else
	{
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	
	return image;
}

static r_image_c*	R_LoadImage(const std::string &name, void *data, uint_t width, uint_t height, uint_t flags, r_image_upload_type_e upload_type)
{
	switch(upload_type)
	{	
		case IMAGE_UPLOAD_COLORMAP:
			return R_UploadColorMap(name, (uint_t*)data, width, height, flags);
			
		case IMAGE_UPLOAD_ALPHAMAP:
			return R_UploadAlphaMap(name, (byte*)data, width, height, flags);
														
		case IMAGE_UPLOAD_BUMPMAP:
			return R_UploadBumpMap(name, (byte*)data, width, height, flags);
			
		case IMAGE_UPLOAD_CUBEMAP:
			return R_UploadCubeMap(name, flags);
		
		case IMAGE_UPLOAD_VIDEOMAP:
			return R_UploadVideoMap(name, (roq_info_t*)data, width, height, flags);
				
		default:
			ri.Com_Error(ERR_DROP, "R_LoadImage: no proper upload type passed %i for image '%s'", upload_type, name.c_str());
			break;
	};
		
	return NULL;
}



r_image_c*	R_FindImage(const std::string &name, uint_t flags, r_image_upload_type_e upload_type)
{
	r_image_c	*image = NULL;
	byte		*pic = NULL;
	roq_info_t	*roq = NULL;
	int		width, height;
	std::string	name_short;
	std::string	name_new;

	if(!name.length())
		return NULL;
		
		
	name_short = X_strlwr(Com_StripExtension(name));
	
	//
	// look for it
	//
	for(std::vector<r_image_c*>::const_iterator ir = r_images.begin(); ir != r_images.end(); ++ir)
	{
		image = *ir;
		
		if(!image)
			continue;
		
		if(X_strcaseequal(name_short.c_str(), image->getName()))
		{
			image->setRegistrationSequence();
			return image;
		}
	}
	
	//
	// load the pic from disk
	//
	image = NULL;
	
	switch(upload_type)
	{
		case IMAGE_UPLOAD_COLORMAP:
		{
			name_new = name_short + ".tga";
			IMG_LoadTGA(name_new, &pic, &width, &height);
			if(pic)
			{
				image = R_LoadImage(name_short, pic, width, height, flags, upload_type);
				goto r_find_image_done;
			}
	
			name_new = name_short + ".png";
			IMG_LoadPNG(name_new, &pic, &width, &height);
			if(pic)
			{
				image = R_LoadImage(name_short, pic, width, height, flags, upload_type);
				goto r_find_image_done;
			}
	
			name_new = name_short + ".jpg";
			IMG_LoadJPG(name_new, &pic, &width, &height);
			if(pic)
			{
				image = R_LoadImage(name_short, pic, width, height, flags, upload_type);
				goto r_find_image_done;
			}
		
			ri.Com_Printf("R_FindImage: couldn't find rgba image '%s'\n", name_short.c_str());
			break;
		}
		
		case IMAGE_UPLOAD_ALPHAMAP:
		{
			name_new = name_short + ".tga";
			IMG_LoadTGAGrey(name_new, &pic, &width, &height);
			if(pic)
			{
				image = R_LoadImage(name_short, pic, width, height, flags, upload_type);
				goto r_find_image_done;
			}
		
			name_new = name_short + ".png";
			IMG_LoadPNGGrey(name_new, &pic, &width, &height);
			if(pic)
			{
				image = R_LoadImage(name_short, pic, width, height, flags, upload_type);
				goto r_find_image_done;
			}
			
			ri.Com_Printf("R_FindImage: couldn't find greyscale image '%s'\n", name_short.c_str());
			break;
		}
		
		case IMAGE_UPLOAD_BUMPMAP:
		{
			name_new = name_short + ".tga";
			IMG_LoadTGAGrey(name_new, &pic, &width, &height);
			if(pic)
			{
				image = R_LoadImage(name_short, pic, width, height, flags, upload_type);
				goto r_find_image_done;
			}
		
			name_new = name_short + ".png";
			IMG_LoadPNGGrey(name_new, &pic, &width, &height);
			if(pic)
			{
				image = R_LoadImage(name_short, pic, width, height, flags, upload_type);
				goto r_find_image_done;
			}
			
			ri.Com_Printf("R_FindImage: couldn't find greyscale image '%s'\n", name_short.c_str());
			break;
		}
		
		case IMAGE_UPLOAD_CUBEMAP:
		{
			for(int i=0; i<6; i++)
			{			
				name_new = name_short + '_' + r_cubemap_suf[i] + ".tga";
				if(ri.VFS_FLoad(name_new, NULL) <= 0)
				{
					ri.Com_Printf("R_FindImage: couldn't find cubemap rgba image '%s'\n", name_new.c_str());
					goto r_find_image_done;
				}
			}
			
			image = R_LoadImage(name_short, NULL, 0, 0, flags, upload_type);
			break;
		};
		
		case IMAGE_UPLOAD_VIDEOMAP:
		{
			name_new = name_short + ".roq";
			roq = ri.Roq_Open(name_new);
			if(roq)
			{
				image = R_LoadImage(name_short, roq, roq->width, roq->height, flags, upload_type);
				goto r_find_image_done;
			}
		
			ri.Com_Printf("R_FindImage: couldn't find video '%s'\n", name_short.c_str());
			break;
		}
	}

r_find_image_done:


	if(pic)
		delete [] pic;

	return image;
}



void	R_LoadLightMapImages(const std::string &mapname)
{
	// destroy old lightmaps
	X_purge<std::vector<r_image_c*> >(r_images_lm);
	r_images_lm.clear();


	ri.Com_DPrintf("loading light maps ...\n");
	
	std::vector<std::string>	lightmaps;
	
	if((lightmaps = ri.VFS_ListFiles("maps/" + mapname, ".tga")).size() != 0)
	{
		for(std::vector<std::string>::const_iterator ir = lightmaps.begin(); ir != lightmaps.end(); ++ir)
		{
			ri.Com_DPrintf("loading lightmap '%s' ...\n", (*ir).c_str());
		
			r_image_c *img = R_FindImage(*ir, IMAGE_CLAMP | IMAGE_NOMIPMAP | IMAGE_LIGHTMAP | IMAGE_NOCOMPRESSION, IMAGE_UPLOAD_COLORMAP);
			
			if(!img)
			{
				ri.Com_Error(ERR_DROP, "R_LoadLightMapImages: couldn't load lightmap '%s' for '%s'", (*ir).c_str(), mapname.c_str());
			}
		}
	}
}



/*
================
R_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/
void	R_FreeUnusedImages()
{
	r_img_default->setRegistrationSequence();
	r_img_white->setRegistrationSequence();
	r_img_black->setRegistrationSequence();
	r_img_flat->setRegistrationSequence();
	r_img_quadratic->setRegistrationSequence();
	
	r_img_cubemap_white->setRegistrationSequence();
	r_img_cubemap_normal->setRegistrationSequence();
	r_img_cubemap_sky->setRegistrationSequence();
	
	r_img_nofalloff->setRegistrationSequence();
//	r_img_attenuation_3d->setRegistrationSequence();
	
	r_img_lightview_depth->setRegistrationSequence();
	r_img_lightview_color->setRegistrationSequence();
	
	r_img_currentrender->setRegistrationSequence();
	
	for(std::vector<r_image_c*>::iterator ir = r_images.begin(); ir != r_images.end(); ir++)
	{
		r_image_c *image = *ir;
		
		if(!image)
			continue;
		
		if(image->getRegistrationSequence() == r_registration_sequence)
			continue;		// used this sequence
		
		delete image;
		
		*ir = NULL;
	}
}


int	R_GetNumForImage(r_image_c *image)
{
	if(!image)
	{
		//ri.Com_Error(ERR_DROP, "R_GetNumForImage: NULL parameter\n");
		return -1;
	}

	for(unsigned int i=0; i<r_images.size(); i++)
	{
		if(image == r_images[i])
			return i;
	}
	
	ri.Com_Error(ERR_DROP, "R_GetNumForImage: bad pointer\n");
	return -1;
}

r_image_c*	R_GetImageByNum(int num)
{
	if(num < 0 || num >= (int)r_images.size())
	{
		ri.Com_Error(ERR_DROP, "R_GetImageByNum: bad number %i\n", num);
		return NULL;
	}

	return r_images[num];
}


r_image_c*	R_GetLightMapImageByNum(int num)
{
	if(num < 0 || num >= (int)r_images_lm.size())
	{
		ri.Com_Error(ERR_DROP, "R_GetLightMapImageByNum: bad number %i\n", num);
		return NULL;
	}

	return r_images_lm[num];
}


