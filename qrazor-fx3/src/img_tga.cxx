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





typedef struct
{
	unsigned char 	id_length;
	unsigned char	colormap_type;
	unsigned char	image_type;
	unsigned short	colormap_index;
	unsigned short	colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin;
	unsigned short	y_origin;
	unsigned short	width;
	unsigned short	height;
	unsigned char	pixel_size;
	unsigned char	attributes;
}dtarga_header_t;



void 	IMG_LoadTGA(const std::string &name, byte **pic, int *width, int *height)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	dtarga_header_t		targa_header;
	byte			*targa_rgba;
	byte tmp[2];

	*pic = NULL;

	//ri.Com_DPrintf ("IMG_LoadTGA: '%s'\n", name);

	//
	// load the file
	//
	length = ri.VFS_FLoad (name, (void **)&buffer);
	if(!buffer)
	{
		//ri.Com_DPrintf ("IMG_LoadTGA: Bad tga file %s\n", name);
		return;
	}
	

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_index = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_length = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.y_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.width = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.height = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if(targa_header.image_type != 2 && targa_header.image_type != 10) 
	{
		ri.Com_Printf("IMG_LoadTGA: Only type 2 and 10 targa images supported but not %i\n", targa_header.image_type);
		return;
	}

	if(targa_header.colormap_type != 0 || (targa_header.pixel_size != 32 && targa_header.pixel_size != 24))
	{
		ri.Com_Printf("IMG_LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");
		return;
	}


	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = new byte[numPixels*4];
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if (targa_header.image_type==2) 
	{  
		// Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) 
			{
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) 
				{
					case 24:
							
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
					case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) 
	{
		// Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		red = 0;
		green = 0;
		blue = 0;
		alphabyte = 0;
		
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) 
			{
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) 
				{        // run-length packet
					switch (targa_header.pixel_size) 
					{
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								break;
					}
	
					for(j=0;j<packetSize;j++) 
					{
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) 
						{
							// run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else 
				{       // non run-length packet
					for(j=0;j<packetSize;j++) 
					{
						switch (targa_header.pixel_size)
						{
							case 24:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = 255;
									break;
							case 32:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									break;
						}
						column++;
						if (column==columns) 
						{ 
							// pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	ri.VFS_FFree (buffer);
}


void 	IMG_LoadTGAGrey(const std::string &name, byte **pic, int *width, int *height)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	dtarga_header_t	targa_header;
	byte			*targa_grey;
	byte tmp[2];

	*pic = NULL;


	//
	// load the file
	//
	length = ri.VFS_FLoad (name, (void **)&buffer);
	if (!buffer)
	{
		//ri.Com_DPrintf ("IMG_LoadTGAGrey: Bad tga file %s\n", name.c_str());
		return;
	}
	

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_index = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_length = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.y_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.width = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.height = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if(targa_header.image_type != 1 && targa_header.image_type != 3)
	{
		ri.Com_Printf("IMG_LoadTGAGrey: Only type 1 and 3 targa images supported but not %i\n", targa_header.image_type);
		return;
	}

	//if (targa_header.colormap_type != 0 || (targa_header.pixel_size != 8 && targa_header.pixel_size != 24))
	//	ri.Com_Error (ERR_DROP, "IMG_LoadTGAGrey: Only 32 or 24 bit images supported (no colormaps)\n");


	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_grey = new byte[numPixels];
	*pic = targa_grey;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;	// skip TARGA image comment
	
	if (targa_header.image_type == 1)
		buf_p += 768;				// skip palette
	
	// Uncompressed, Grey images
	for(row=rows-1; row>=0; row--) 
	{
		pixbuf = targa_grey + row*columns;
		for(column=0; column<columns; column++) 
		{
			*pixbuf++ = *buf_p++;
		}
	}
	
	ri.VFS_FFree (buffer);
}


void	IMG_WriteTGA(const std::string &filename, byte *pic, int width, int height, int bits)
{
	int		i, tmp;
	int		fsize;
	byte		*buffer;
	
	/*
	typedef struct
	{
		unsigned char 	id_length;
		unsigned char	colormap_type;
		unsigned char	image_type;
		unsigned short	colormap_index;
		unsigned short	colormap_length;
		unsigned char	colormap_size;
		unsigned short	x_origin;
		unsigned short	y_origin;
		unsigned short	width;
		unsigned short	height;
		unsigned char	pixel_size;
		unsigned char	attributes;
	}dtarga_header_t;
	*/
	
	//ri.Com_DPrintf("IMG_WriteTGA: %s %p %i %i\n", filename, pic, width, height);
	
	fsize = width*height*bits+18;
	
	buffer = new byte[fsize];
	memset(buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width&255;
	buffer[13] = width>>8;
	buffer[14] = height&255;
	buffer[15] = height>>8;
	buffer[16] = bits*8;	// pixel size
	
	memcpy(buffer+18, pic, width*height*bits);

	// swap rgb to bgr
	for(i=18 ; i<fsize ; i+=bits)
	{
		tmp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = tmp;
	}


	ri.VFS_FSave(filename, buffer, fsize);
	
	delete [] buffer;
}


