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
#include "r_public.h"


extern  ref_import_t     ri;

#if 1

#ifdef __cplusplus
extern "C" {
#endif

#include <jpeglib.h>

static void	jpeg_null (struct jpeg_decompress_struct *cinfo)
{
	//DO NOTHING
}	
	
static byte	jpeg_fill_input_buffer (struct jpeg_decompress_struct *cinfo)
{
	ri.Com_Printf ("jpeg_fill_input_buffer: premature end of JPEG data\n");
	return 1;
}
	
static void	jpeg_skip_input_data (struct jpeg_decompress_struct *cinfo, long num_bytes)
{
	cinfo->src->next_input_byte += (size_t) num_bytes;
	cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
	
	if(cinfo->src->bytes_in_buffer < 0)
		ri.Com_Printf ("jpeg_skip_input_data: premature end of JPEG data\n");
}
	
static void	jpeg_mem_src (struct jpeg_decompress_struct *cinfo, byte *mem, int len)
{
	cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
	cinfo->src->init_source 	= (void (*) (struct jpeg_decompress_struct *)) jpeg_null;
	cinfo->src->fill_input_buffer 	= (boolean (*) (struct jpeg_decompress_struct *)) jpeg_fill_input_buffer;
	cinfo->src->skip_input_data 	= (void (*) (struct jpeg_decompress_struct *, long int)) jpeg_skip_input_data;
	cinfo->src->resync_to_restart	= jpeg_resync_to_restart;
	cinfo->src->term_source		= (void (*) (struct jpeg_decompress_struct *)) jpeg_null;
	cinfo->src->bytes_in_buffer	= len;
	cinfo->src->next_input_byte	= mem;
}


static int	jpeg_compressed_size;
static int	jpeg_size;


static boolean	jpeg_empty_output_buffer (struct jpeg_compress_struct *cinfo)
{
	return TRUE;
}

static void	jpeg_term_destination (struct jpeg_compress_struct *cinfo)
{
	jpeg_compressed_size = jpeg_size - cinfo->dest->free_in_buffer;
}


static void	jpeg_mem_dest (struct jpeg_compress_struct *cinfo, byte *mem, int len)
{
	cinfo->dest = (struct jpeg_destination_mgr*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_destination_mgr));
	cinfo->dest->init_destination	= (void (*) (struct jpeg_compress_struct *)) jpeg_null;
	cinfo->dest->empty_output_buffer = (boolean (*) (struct jpeg_compress_struct *)) jpeg_empty_output_buffer;
	cinfo->dest->term_destination 	= (void (*) (struct jpeg_compress_struct *)) jpeg_term_destination;
	cinfo->dest->free_in_buffer 	= jpeg_size = len;	//HACK HACK HACK
	cinfo->dest->next_output_byte	= mem;
}


#ifdef __cplusplus
}
#endif


void 	IMG_LoadJPG(const std::string &name, byte **pic, int *width, int *height, bool fill_alpha = true)
{
	byte	*buffer;
	int		length;
	int 		i;
	int		b;

	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr		jerr;

	byte*			jpeg_rgba;
	byte*			out;
	byte*			dummy;
		

	//
	// load the file
	//
	length = ri.VFS_FLoad (name, (void **)&buffer);
	if(!buffer)
	{
		//ri.Com_DPrintf ("IMG_LoadJPG: Bad JPEG file %s\n", name);
		return;
	}
	
	
	// setup decompression
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	
	// set source
	jpeg_mem_src(&cinfo, buffer, length);
	
	// process header
	jpeg_read_header(&cinfo, true);
	
	// start decompression
	jpeg_start_decompress(&cinfo);
	
	// check color components
	if(cinfo.output_components != 3 /*&& cinfo.output_components != 1*/)
	{

		jpeg_destroy_decompress(&cinfo);
		ri.VFS_FFree(buffer);
		ri.Com_Error (ERR_DROP, "IMG_LoadJPG: Invalid JPEG color components\n");
		return;
	}
	
	// pass sizes
	*width = cinfo.output_width;
	*height = cinfo.output_height;
	
	// allocate rgba memory
	jpeg_rgba = *pic = new byte[cinfo.output_width * cinfo.output_height * 4];
	
	if(!jpeg_rgba)
	{
		jpeg_destroy_decompress(&cinfo);
		ri.VFS_FFree(buffer);
		ri.Com_Error (ERR_DROP, "IMG_LoadJPG: Not enough RAM\n");
		return;
		
	}
	
	if(cinfo.output_components == 1)
	{
		dummy = out = new byte[cinfo.output_width * cinfo.output_height];
		
		// read scanlines
		while(cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, &out, 1);
	
			for(i=0; i<(int)cinfo.output_width; i++, jpeg_rgba += 4)
			{
				b = *out++;
				
				jpeg_rgba[0] = b;
				jpeg_rgba[1] = b;
				jpeg_rgba[2] = b;
				jpeg_rgba[3] = fill_alpha ? 255 : 0;
			}
		}
	}
	else
	{
		dummy = out = new byte[cinfo.output_width * cinfo.output_height * 3];
		
		// read scanlines
		while(cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, &out, 1);
	
			for(i=0; i<(int)cinfo.output_width; i++, out += 3, jpeg_rgba += 4)
			{				
				jpeg_rgba[0] = out[0];
				jpeg_rgba[1] = out[1];
				jpeg_rgba[2] = out[2];
				jpeg_rgba[3] = fill_alpha ? 255 : 0;
			}
		}
	}
	
	
	
	
	
	// finish decompression
	jpeg_finish_decompress(&cinfo);
	
	// destroy jpeg object
	jpeg_destroy_decompress(&cinfo);
		
	// free unneeded memory
	ri.VFS_FFree(buffer);
	delete [] dummy;
}


void	IMG_WriteJPG(const std::string &filename, byte *pic, int width, int height, int quality)
{
	byte	*buffer;
	
	struct jpeg_compress_struct	cinfo;
	struct jpeg_error_mgr		jerr;

	int			jpeg_row_stride;
	JSAMPROW 	row;
	int		offset;

	int	length = 0;

		
	//ri.Com_DPrintf ("IMG_WriteJPG: %s %p %i %i\n", filename, pic, width, height);


	// setup compression
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	
	// set dest
	buffer = (byte*) new byte[width * height * 3];
	jpeg_mem_dest(&cinfo, buffer, length);
	
	// set compression parameters
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.in_color_space = JCS_RGB;
	cinfo.input_components = 3;
	
	jpeg_set_defaults (&cinfo);
	
	jpeg_set_quality (&cinfo, quality, TRUE);
	
	// start compression
	jpeg_start_compress (&cinfo, true);
	
	
	// feed scanline data
	jpeg_row_stride = cinfo.image_width * 3;
	offset = jpeg_row_stride * cinfo.image_height - jpeg_row_stride;
	
	while (cinfo.next_scanline < cinfo.image_height)
	{
		row = &pic[offset - cinfo.next_scanline * jpeg_row_stride];
		jpeg_write_scanlines (&cinfo, &row, 1);
	}

	// finish compression
	jpeg_finish_compress (&cinfo);
	
	jpeg_destroy_compress (&cinfo);

	//ri.Com_DPrintf ("IMG_WriteJPG: %i bytes compressed to %i bytes\n", (width*height*3), jpeg_compressed_size);

	ri.VFS_FSave (filename, buffer, jpeg_compressed_size);
	delete [] buffer;
}





#endif

