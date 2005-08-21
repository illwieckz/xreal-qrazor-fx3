/// ============================================================================
/*
Copyright (C) 2001 Dr. Tim Ferguson <http://www.csse.monash.edu.au/~timf/>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// qrazor-fx ----------------------------------------------------------------

#include "roq.h"
#include "vfs.h"

#define DBUG	0

/*
static unsigned int	get_word(VFILE *fp)
{
	unsigned int ret;

	ret  = ((fgetc((FILE*)fp->file)) & 0xff);
	ret |= ((fgetc((FILE*)fp->file)) & 0xff) << 8;
	return(ret);
}

static unsigned long	get_long(VFILE *fp)
{
	unsigned long ret;

	ret  = ((fgetc((FILE*)fp->file)) & 0xff);
	ret |= ((fgetc((FILE*)fp->file)) & 0xff) << 8;
	ret |= ((fgetc((FILE*)fp->file)) & 0xff) << 16;
	ret |= ((fgetc((FILE*)fp->file)) & 0xff) << 24;
	return(ret);
}
*/


/*
static unsigned int	get_word(VFILE *fp)
{
	unsigned int ret;
	
	VFS_FRead(&ret, sizeof(short), fp);
	
	return LittleShort(ret);
}

static unsigned long	get_long(VFILE *fp)
{
	unsigned long ret;
	
	VFS_FRead(&ret, sizeof(int), fp);
	
	return LittleLong(ret);
}
*/

static void	roq_read_info(roq_info_t *ri)
{
	short	tmp[4];
	
	VFS_FRead(tmp, sizeof(tmp), ri->stream);
	
	ri->width = LittleShort(tmp[0]);
	ri->height = LittleShort(tmp[1]);
	
	//ri->width = get_word(ri->stream);
	//ri->height = get_word(ri->stream);
	//get_word(ri->stream);
	//get_word(ri->stream);
}

static int	roq_parse_file(VFILE *fp, roq_info_t *ri)
{
	unsigned short head1, head3, chunk_id, chunk_arg;
	int head2, chunk_size;
	long fpos;
	int max_frame;

	ri->num_audio_bytes = ri->num_frames = max_frame = 0;
	ri->audio_channels = 0;
	ri->frame_offset = NULL;
	ri->buf_size = 0;
	
	//head1 = get_word(fp);
	//head2 = get_long(fp);
	//head3 = get_word(fp);
	
	VFS_FRead(&head1, sizeof(short), fp);
	VFS_FRead(&head2, sizeof(int), fp);
	VFS_FRead(&head3, sizeof(short), fp);
	
	head1 = LittleShort(head1);
	head2 = LittleLong(head2);
	head3 = LittleShort(head3);
	
	if((head1 != 0x1084) && (head2 != (int)0xffffffff) && (head3 != 0x1e))
	{
		Com_Printf("Not an RoQ file.\n");
		return 1;
	}

	ri->roq_start = VFS_FTell(fp);
	
	while(!VFS_FEOF(fp))
	{
#if DBUG > 20
		printf("---------------------------------------------------------------------------\n");
#endif
		fpos = VFS_FTell(fp);
		
		//chunk_id = get_word(fp);
		//chunk_size = get_long(fp);
		//chunk_arg = get_word(fp);
		
		VFS_FRead(&chunk_id, sizeof(short), fp);
		VFS_FRead(&chunk_size, sizeof(int), fp);
		VFS_FRead(&chunk_arg, sizeof(short), fp);
	
		chunk_id = LittleShort(chunk_id);
		chunk_size = LittleLong(chunk_size);
		chunk_arg = LittleShort(chunk_arg);
		
		if(chunk_size > ri->buf_size)
			ri->buf_size = chunk_size;
			
		if(VFS_FEOF(fp))
			break;
#if DBUG > 20
		Com_Printf("%03d  0x%06lx: chunk: 0x%02x size: %ld  cells: 2x2=%d,4x4=%d\n", i,
			fpos, chunk_id, chunk_size, v1>>8,v1&0xff);
#endif

		if(chunk_id == RoQ_INFO)		/* video info */
		{
			roq_read_info(ri);
		}
		else
		{
			if(chunk_id == RoQ_QUAD_VQ)
			{
				ri->num_frames++;
				
				if((int)ri->num_frames > max_frame)
				{
					max_frame += 5000;
					
					if((ri->frame_offset = (long int*)realloc(ri->frame_offset, sizeof(long) * max_frame)) == NULL)
						return 1;
				}
				
				ri->frame_offset[ri->num_frames] = fpos;
			}
			
			if(chunk_id == RoQ_SOUND_MONO || chunk_id == RoQ_SOUND_STEREO)
			{
				if(chunk_id == RoQ_SOUND_MONO)
					ri->audio_channels = 1;
				else
					ri->audio_channels = 2;
					
				ri->num_audio_bytes += chunk_size;
			}
			
			VFS_FSeek(fp, chunk_size, VFS_SEEK_CUR);
		}
	}

	return 0;
}

static void	roq_apply_vector_2x2(roq_info_t *ri, int x, int y, roq_cell_t *cell)
{
	unsigned char *yptr;

	yptr = ri->y[0] + (y * ri->width) + x;
	*yptr++ = cell->y0;
	*yptr++ = cell->y1;
	yptr += (ri->width - 2);
	*yptr++ = cell->y2;
	*yptr++ = cell->y3;
	ri->u[0][(y/2) * (ri->width/2) + x/2] = cell->u;
	ri->v[0][(y/2) * (ri->width/2) + x/2] = cell->v;
}

static void	roq_apply_vector_4x4(roq_info_t *ri, int x, int y, roq_cell_t *cell)
{
	unsigned long row_inc, c_row_inc;
	register unsigned char y0, y1, u, v;
	unsigned char *yptr, *uptr, *vptr;

	yptr = ri->y[0] + (y * ri->width) + x;
	uptr = ri->u[0] + (y/2) * (ri->width/2) + x/2;
	vptr = ri->v[0] + (y/2) * (ri->width/2) + x/2;

	row_inc = ri->width - 4;
	c_row_inc = (ri->width/2) - 2;
	*yptr++ = y0 = cell->y0; *uptr++ = u = cell->u; *vptr++ = v = cell->v;
	*yptr++ = y0;
	*yptr++ = y1 = cell->y1; *uptr++ = u; *vptr++ = v;
	*yptr++ = y1;

	yptr += row_inc;

	*yptr++ = y0;
	*yptr++ = y0;
	*yptr++ = y1;
	*yptr++ = y1;

	yptr += row_inc; uptr += c_row_inc; vptr += c_row_inc;

	*yptr++ = y0 = cell->y2; *uptr++ = u; *vptr++ = v;
	*yptr++ = y0;
	*yptr++ = y1 = cell->y3; *uptr++ = u; *vptr++ = v;
	*yptr++ = y1;

	yptr += row_inc;

	*yptr++ = y0;
	*yptr++ = y0;
	*yptr++ = y1;
	*yptr++ = y1;
}

static void	roq_apply_motion_4x4(roq_info_t *ri, int x, int y, unsigned char mv, char mean_x, char mean_y)
{
	int i, mx, my;
	unsigned char *pa, *pb;

	mx = x + 8 - (mv >> 4) - mean_x;
	my = y + 8 - (mv & 0xf) - mean_y;

	pa = ri->y[0] + (y * ri->width) + x;
	pb = ri->y[1] + (my * ri->width) + mx;
	for(i = 0; i < 4; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa[2] = pb[2];
		pa[3] = pb[3];
		pa += ri->width;
		pb += ri->width;
	}

	pa = ri->u[0] + (y/2) * (ri->width/2) + x/2;
	pb = ri->u[1] + (my/2) * (ri->width/2) + (mx + 1)/2;
	for(i = 0; i < 2; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa += ri->width/2;
		pb += ri->width/2;
	}

	pa = ri->v[0] + (y/2) * (ri->width/2) + x/2;
	pb = ri->v[1] + (my/2) * (ri->width/2) + (mx + 1)/2;
	for(i = 0; i < 2; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa += ri->width/2;
		pb += ri->width/2;
	}
}

static void	roq_apply_motion_8x8(roq_info_t *ri, int x, int y, unsigned char mv, char mean_x, char mean_y)
{
	int mx, my, i;
	unsigned char *pa, *pb;

	mx = x + 8 - (mv >> 4) - mean_x;
	my = y + 8 - (mv & 0xf) - mean_y;

	pa = ri->y[0] + (y * ri->width) + x;
	pb = ri->y[1] + (my * ri->width) + mx;
	for(i = 0; i < 8; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa[2] = pb[2];
		pa[3] = pb[3];
		pa[4] = pb[4];
		pa[5] = pb[5];
		pa[6] = pb[6];
		pa[7] = pb[7];
		pa += ri->width;
		pb += ri->width;
	}

	pa = ri->u[0] + (y/2) * (ri->width/2) + x/2;
	pb = ri->u[1] + (my/2) * (ri->width/2) + (mx + 1)/2;
	for(i = 0; i < 4; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa[2] = pb[2];
		pa[3] = pb[3];
		pa += ri->width/2;
		pb += ri->width/2;
	}

	pa = ri->v[0] + (y/2) * (ri->width/2) + x/2;
	pb = ri->v[1] + (my/2) * (ri->width/2) + (mx + 1)/2;
	for(i = 0; i < 4; i++)
	{
		pa[0] = pb[0];
		pa[1] = pb[1];
		pa[2] = pb[2];
		pa[3] = pb[3];
		pa += ri->width/2;
		pb += ri->width/2;
	}
}


roq_info_t*	Roq_Open(const std::string &filename)
{
	VFILE *stream = NULL;
	roq_info_t *ri;
	int i;

	if(!VFS_FOpenRead(filename, &stream))
	{
		Com_Printf("Roq_Open: error opening '%s'\n", filename.c_str());
		return NULL;
	}

	if((ri = (roq_info_t*)Com_Alloc(sizeof(roq_info_t))) == NULL)
	{
		Com_Printf("Roq_Open: error allocating memory.\n");
		return NULL;
	}

	ri->stream = stream;
	if(roq_parse_file(stream, ri))
		return NULL;

	ri->stream_length = (ri->num_frames * 1000)/30;

	for(i = 0; i < 128; i++)
	{
		ri->snd_sqr_arr[i] = i * i;
		ri->snd_sqr_arr[i + 128] = -(i * i);
	}

	for(i = 0; i < 2; i++)
	{
		if(	(ri->y[i] = (byte*)calloc(ri->width * ri->height, 1)) == NULL ||
			(ri->u[i] = (byte*)calloc((ri->width * ri->height)/4, 1)) == NULL ||
			(ri->v[i] = (byte*)calloc((ri->width * ri->height)/4, 1)) == NULL)
		{
			Com_Printf("Roq_Open: memory allocation error.\n");
			return NULL;
		}
	}

	ri->buf_size *= 2;
	if((ri->buf = (byte*)calloc(ri->buf_size, 1)) == NULL)
	{
		Com_Printf("Roq_Open: memory allocation error.\n");
		return NULL;
	}
	
	ri->audio_buf_size = 0;
	ri->audio = NULL;

	ri->frame_num = 0;
	ri->aud_pos = ri->vid_pos = ri->roq_start;

	return ri;
}


void	Roq_Close(roq_info_t *ri)
{
	if(ri == NULL)
		return;
	
	VFS_FClose(&ri->stream);
	
	for(int i=0; i<2; i++)
	{
		if(ri->y[i] != NULL)
			Com_Free(ri->y[i]);
			
		if(ri->u[i] != NULL)
			Com_Free(ri->u[i]);
			
		if(ri->v[i] != NULL)
			Com_Free(ri->v[i]);
	}
	
	if(ri->buf != NULL)
		Com_Free(ri->buf);
		
	Com_Free(ri);
}


int	Roq_ReadVideo(roq_info_t *ri)
{
	VFILE *fp = ri->stream;
	unsigned short chunk_id = 0, chunk_arg = 0;
	unsigned int chunk_size = 0;
	int k, nv1, nv2, vqflg = 0, vqflg_pos = -1, vqid, bpos, xpos, ypos, xp, yp, x, y;
	unsigned char *tp, *buf;
	int frame_stats[2][4] = {{0},{0}};
	roq_qcell_t *qcell;

	VFS_FSeek(fp, ri->vid_pos, VFS_SEEK_SET);

	while(!VFS_FEOF(fp))
	{
		//chunk_id = get_word(fp);
		//chunk_size = get_long(fp);
		//chunk_arg = get_word(fp);
		
		VFS_FRead(&chunk_id, sizeof(short), fp);
		VFS_FRead(&chunk_size, sizeof(int), fp);
		VFS_FRead(&chunk_arg, sizeof(short), fp);
	
		chunk_id = LittleShort(chunk_id);
		chunk_size = LittleLong(chunk_size);
		chunk_arg = LittleShort(chunk_arg);
		
		if(VFS_FEOF(fp))
			break;
		
		if(chunk_id == RoQ_QUAD_VQ)
			break;
		
		if(chunk_id == RoQ_QUAD_CODEBOOK)
		{
			if((nv1 = chunk_arg >> 8) == 0)
				nv1 = 256;
				
			if((nv2 = chunk_arg & 0xff) == 0 && nv1 * 6 < (int)chunk_size)
				nv2 = 256;
			
			/*	
			for(int i=0; i<nv1; i++)
			{
				ri->cells[i].y0 = fgetc((FILE*)fp->file);
				ri->cells[i].y1 = fgetc((FILE*)fp->file);
				ri->cells[i].y2 = fgetc((FILE*)fp->file);
				ri->cells[i].y3 = fgetc((FILE*)fp->file);
				ri->cells[i].u = fgetc((FILE*)fp->file);
				ri->cells[i].v = fgetc((FILE*)fp->file);
			
				
				//VFS_FRead(&ri->cells[i].y0, 1, fp); 
				//VFS_FRead(&ri->cells[i].y1, 1, fp);
				//VFS_FRead(&ri->cells[i].y2, 1, fp);
				//VFS_FRead(&ri->cells[i].y3, 1, fp);
				//VFS_FRead(&ri->cells[i].u, 1, fp);
				//VFS_FRead(&ri->cells[i].v, 1, fp);
				
				
				//VFS_FRead(&ri->cells[i], sizeof(roq_cell_t), fp);
			}
			*/
			
			VFS_FRead(ri->cells, sizeof(roq_cell_t)*nv1, fp);
			
			//for(int i=0; i<nv2; i++)
			//	for(int j=0; j<4; j++)
			//		VFS_FRead(&ri->qcells[i].idx[j], 1, fp);
					
			VFS_FRead(ri->qcells, sizeof(roq_qcell_t)*nv2, fp);
		}
		else
		{
			VFS_FSeek(fp, chunk_size, VFS_SEEK_CUR);
		}
	}

	if(chunk_id != RoQ_QUAD_VQ)
	{
		ri->vid_pos = VFS_FTell(fp);
		return 0;
	}

	ri->frame_num++;
	if(ri->buf_size < (int)chunk_size)
	{
		ri->buf_size *= 2;
		Com_Free(ri->buf);
		
		if((ri->buf = (byte*)calloc(ri->buf_size, 1)) == NULL)
		{
			Com_Printf("Roq_ReadVideo: memory allocation error.\n");
			return -1;
		}
	}
	//fread(ri->buf, chunk_size, 1, (FILE*)fp->file);
	VFS_FRead(ri->buf, chunk_size, fp);
	buf = ri->buf;

	bpos = xpos = ypos = 0;
	while(bpos < (int)chunk_size)
	{
		for(yp = ypos; yp < ypos + 16; yp += 8)
		{
			for(xp = xpos; xp < xpos + 16; xp += 8)
			{
				if(vqflg_pos < 0)
				{
					vqflg = buf[bpos++]; vqflg |= (buf[bpos++] << 8);
					vqflg_pos = 7;
				}
				vqid = (vqflg >> (vqflg_pos * 2)) & 0x3;
				frame_stats[0][vqid]++;
				vqflg_pos--;

				switch(vqid)
				{
					case RoQ_ID_MOT: break;
					case RoQ_ID_FCC:
						roq_apply_motion_8x8(ri, xp, yp, buf[bpos++], chunk_arg >> 8, chunk_arg & 0xff);
						break;
					case RoQ_ID_SLD:
						qcell = ri->qcells + buf[bpos++];
						roq_apply_vector_4x4(ri, xp, yp, ri->cells + qcell->idx[0]);
						roq_apply_vector_4x4(ri, xp+4, yp, ri->cells + qcell->idx[1]);
						roq_apply_vector_4x4(ri, xp, yp+4, ri->cells + qcell->idx[2]);
						roq_apply_vector_4x4(ri, xp+4, yp+4, ri->cells + qcell->idx[3]);
						break;
					case RoQ_ID_CCC:
						for(k = 0; k < 4; k++)
						{
							x = xp; y = yp;
							if(k & 0x01) x += 4;
							if(k & 0x02) y += 4;

							if(vqflg_pos < 0)
							{
								vqflg = buf[bpos++]; vqflg |= (buf[bpos++] << 8);
								vqflg_pos = 7;
							}
							vqid = (vqflg >> (vqflg_pos * 2)) & 0x3;
							frame_stats[1][vqid]++;
							vqflg_pos--;
							switch(vqid)
							{
								case RoQ_ID_MOT: break;
								case RoQ_ID_FCC:
									roq_apply_motion_4x4(ri, x, y, buf[bpos++], chunk_arg >> 8, chunk_arg & 0xff);
									break;
								case RoQ_ID_SLD:
									qcell = ri->qcells + buf[bpos++];
									roq_apply_vector_2x2(ri, x, y, ri->cells + qcell->idx[0]);
									roq_apply_vector_2x2(ri, x+2, y, ri->cells + qcell->idx[1]);
									roq_apply_vector_2x2(ri, x, y+2, ri->cells + qcell->idx[2]);
									roq_apply_vector_2x2(ri, x+2, y+2, ri->cells + qcell->idx[3]);
									break;
								case RoQ_ID_CCC:
									roq_apply_vector_2x2(ri, x, y, ri->cells + buf[bpos]);
									roq_apply_vector_2x2(ri, x+2, y, ri->cells + buf[bpos+1]);
									roq_apply_vector_2x2(ri, x, y+2, ri->cells + buf[bpos+2]);
									roq_apply_vector_2x2(ri, x+2, y+2, ri->cells + buf[bpos+3]);
									bpos += 4;
									break;
							}
						}
						break;
					default:
						Com_Printf("Roq_ReadFrame: unknown vq code: %d\n", vqid);
				}
			}
		}

		xpos += 16;
		
		if(xpos >= ri->width)
		{
			xpos -= ri->width;
			ypos += 16;
		}
		
		if(ypos >= ri->height)
			break;
	}

#if 0
	frame_stats[0][3] = 0;
	Com_Printf("<%d  0x%04x -> %d,%d>\n", ri->frame_num, chunk_arg, (char)(chunk_arg >> 8), (char)(chunk_arg & 0xff));
	Com_Printf("for 08x08 CCC = %d, FCC = %d, MOT = %d, SLD = %d, PAT = 0\n", frame_stats[0][3], frame_stats[0][1], frame_stats[0][0], frame_stats[0][2]);
	Com_Printf("for 04x04 CCC = %d, FCC = %d, MOT = %d, SLD = %d, PAT = 0\n", frame_stats[1][3], frame_stats[1][1], frame_stats[1][0], frame_stats[1][2]);
#endif

	ri->vid_pos = VFS_FTell(fp);

	if(ri->frame_num == 1)
	{
		memcpy(ri->y[1], ri->y[0], ri->width * ri->height);
		memcpy(ri->u[1], ri->u[0], (ri->width * ri->height)/4);
		memcpy(ri->v[1], ri->v[0], (ri->width * ri->height)/4);
	}
	else
	{
		tp = ri->y[0]; ri->y[0] = ri->y[1]; ri->y[1] = tp;
		tp = ri->u[0]; ri->u[0] = ri->u[1]; ri->u[1] = tp;
		tp = ri->v[0]; ri->v[0] = ri->v[1]; ri->v[1] = tp;
	}

	return 1;
}


int	Roq_ReadAudio(roq_info_t *ri)
{
	VFILE *fp = ri->stream;
	unsigned short chunk_id = 0, chunk_arg = 0;
	unsigned int chunk_size = 0;
	int snd_left, snd_right;

	VFS_FSeek(fp, ri->aud_pos, VFS_SEEK_SET);
	ri->audio_size = 0;

	while(!VFS_FEOF(fp))
	{
		//chunk_id = get_word(fp);
		//chunk_size = get_long(fp);
		//chunk_arg = get_word(fp);
		
		VFS_FRead(&chunk_id, sizeof(short), fp);
		VFS_FRead(&chunk_size, sizeof(int), fp);
		VFS_FRead(&chunk_arg, sizeof(short), fp);
	
		chunk_id = LittleShort(chunk_id);
		chunk_size = LittleLong(chunk_size);
		chunk_arg = LittleShort(chunk_arg);
		
		if(VFS_FEOF(fp) || chunk_id == RoQ_SOUND_MONO || chunk_id == RoQ_SOUND_STEREO)
			break;
			
		VFS_FSeek(fp, chunk_size, VFS_SEEK_CUR);
	}

	if(ri->audio_buf_size < (int)chunk_size * 2)
	{
		if(ri->audio != NULL)
			free(ri->audio);
			
		ri->audio_buf_size = chunk_size * 3;
		
		if((ri->audio = (byte*)calloc(ri->audio_buf_size, 1)) == NULL)
			return -1;
	}

	if(chunk_id == RoQ_SOUND_MONO)
	{
		ri->audio_size = chunk_size * 2;
		
		snd_left = chunk_arg;
	
		for(unsigned int i=0; i < chunk_size; i++)
		{
			int idx;
			
			VFS_FRead(&idx, 1, fp);
			snd_left += ri->snd_sqr_arr[idx];
			//snd_left += ri->snd_sqr_arr[fgetc(fp)];
			
			ri->audio[i * 2] = snd_left & 0xff;
			ri->audio[i * 2 + 1] = (snd_left & 0xff00) >> 8;
		}
		
		ri->aud_pos = VFS_FTell(fp);
		return 1;
	}

	if(chunk_id == RoQ_SOUND_STEREO)
	{
		ri->audio_size = chunk_size * 2;
		
		snd_left = (chunk_arg & 0xFF00);
		snd_right = (chunk_arg & 0xFF) << 8;
		
		for(unsigned int i=0; i < chunk_size; i += 2)
		{
			int idx;
			
			VFS_FRead(&idx, 1, fp);
			snd_left += ri->snd_sqr_arr[idx];
			
			VFS_FRead(&idx, 1, fp);
			snd_right += ri->snd_sqr_arr[idx];
		
			//snd_left += ri->snd_sqr_arr[fgetc(fp)];
			//snd_right += ri->snd_sqr_arr[fgetc(fp)];
			
			ri->audio[i * 2] = snd_left & 0xff;
			ri->audio[i * 2 + 1] = (snd_left & 0xff00) >> 8;
			ri->audio[i * 2 + 2] = snd_right & 0xff;
			ri->audio[i * 2 + 3] = (snd_right & 0xff00) >> 8;
		}
	
		ri->aud_pos = VFS_FTell(fp);
		return 1;
	}

	ri->aud_pos = VFS_FTell(fp);
	return 0;
}


void	Roq_ResetStream(roq_info_t *info)
{
	info->aud_pos = info->vid_pos = info->roq_start;
	info->frame_num = 0;
}


