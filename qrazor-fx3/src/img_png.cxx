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
#include <png.h>
// qrazor-fx ----------------------------------------------------------------
#include "r_public.h"


extern  ref_import_t     ri;


static byte*	png_buffer = NULL;
static int	png_buffer_ofs = 0;

static void	png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	memcpy(data, ((byte*)png_ptr->io_ptr) + png_buffer_ofs, length);
	png_buffer_ofs += length;
}

void 	IMG_LoadPNG(const std::string &name, byte **pic, int *width, int *height)
{
	int	length;
	int	png_width;
	int	png_height;
	int	depth, type;

	png_byte*	png_data = NULL;
	png_byte**	row_pointers = NULL;
	byte*		png_rgba;
	
	//
	// load the file
	//
	length = ri.VFS_FLoad(name, (void **)&png_buffer);
	if (!png_buffer)
	{
		//ri.Com_Printf ("IMG_LoadPNG: Bad png file %s\n", name);
		return;
	}
	
	// check ident
	if(png_sig_cmp(png_buffer, 0, 8))
	{
		ri.Com_Printf("IMG_LoadPNG: png file %s has bad ident\n", name.c_str());
		return;
	}
	
	// setup read struct
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
	{
		ri.Com_Printf("IMG_LoadPNG: png_create_read_struct() failed\n");
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
        	png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		ri.Com_Printf("IMG_LoadPNG: png_create_info_struct() failed\n");
		return;
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if(!end_info)
	{
        	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		ri.Com_Printf("IMG_LoadPNG: second png_create_info_struct() failed\n");
		return;
	}
	
	if(setjmp(png_jmpbuf(png_ptr)))
	{
        	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		ri.Com_Printf("IMG_LoadPNG: png_jmpbuf() failed\n");
		return;
	}
	
	
	png_set_sig_bytes(png_ptr, 0);
	
	png_set_read_fn(png_ptr, png_buffer, png_user_read_data);
	
	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)&png_width, (png_uint_32*)&png_height, &depth, &type, NULL, NULL, NULL);

	if(depth == 16)
		png_set_strip_16(png_ptr);
		
        if(type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_expand(png_ptr);
	}
	
	if(depth < 8)
            png_set_expand(png_ptr);
	
	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);
	
	if(type == PNG_COLOR_TYPE_GRAY || type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);
	
		
	png_read_update_info(png_ptr, info_ptr);
	
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)&png_width, (png_uint_32*)&png_height, &depth, &type, NULL, NULL, NULL);
	

	png_uint_32 row_bytes = png_get_rowbytes(png_ptr, info_ptr);
        //png_uint_32 channels = png_get_channels(png_ptr, info_ptr);
	
	
	png_data = new png_byte[row_bytes * png_height];
	if(png_data == NULL)
        {
		ri.Com_Error(ERR_DROP, "IMG_LoadPNG: Not enough RAM\n");
		return;
        }
	
	// and allocate memory for an array of row-pointers
	row_pointers = (png_bytepp) Com_Alloc((png_height) * sizeof(png_bytep));
	if(row_pointers == NULL)
        {
		ri.Com_Error(ERR_DROP, "IMG_LoadPNG: Not enough RAM\n");
		return;
        }
        
        // set the individual row-pointers to point at the correct offsets
	for(int i=0; i<png_height; i++)
		row_pointers[i] = png_data + i * row_bytes;
	
	// now we can go ahead and just read the whole image
	png_read_image(png_ptr, row_pointers);

	// read the additional chunks in the PNG file (not really needed)
        png_read_end(png_ptr, NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	
	// allocate RGBA memory
	png_rgba = *pic = new byte[png_width * png_height * 4];
	if(!png_rgba)
	{
	
		ri.VFS_FFree(png_buffer);
		ri.Com_Error(ERR_DROP, "IMG_LoadPNG: Not enough RAM\n");
		return;
		
	}
	
	// convert RGB png image to a RGBA image
	byte *buf_p = png_data;
	byte *pixbuf;
	
	for(int i=0; i<png_height; i++) 
	{
		pixbuf = png_rgba + i*png_width*4;
		
		for(int j=0; j<png_width; j++) 
		{
			byte red, green, blue, alpha;
			
			red = *buf_p++;
			green = *buf_p++;
			blue = *buf_p++;
			alpha = 0;
			
			*pixbuf++ = red;
			*pixbuf++ = green;
			*pixbuf++ = blue;
			*pixbuf++ = alpha;
		}
	}
	
	*pic = png_rgba;
	*width = png_width;
	*height = png_height;
	
	ri.VFS_FFree((void*)png_buffer);
	Com_Free(row_pointers);
	delete [] png_data;
	png_buffer = NULL;
	png_buffer_ofs = 0;
}


void 	IMG_LoadPNGGrey(const std::string &name, byte **pic, int *width, int *height)
{
	byte	*buffer;
	int		length;
	
	//
	// load the file
	//
	length = ri.VFS_FLoad (name, (void **)&buffer);
	if (!buffer)
	{
		//ri.Com_Printf ("IMG_LoadPNGGrey: Bad png file %s\n", name);
		return;
	}

	//TODO
	
	ri.VFS_FFree ((void*)buffer);
}


