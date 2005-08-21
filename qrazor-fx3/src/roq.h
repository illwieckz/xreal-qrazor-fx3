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
#ifndef ROQ_H
#define ROQ_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "shared/x_shared.h"



// This is a simple decoder for the Id Software RoQ video format.  In
// this format, audio samples are DPCM coded and the video frames are
// coded using motion blocks and vector quantisation.


#define RoQ_INFO		0x1001
#define RoQ_QUAD_CODEBOOK	0x1002
#define RoQ_QUAD_VQ		0x1011
#define RoQ_SOUND_MONO		0x1020
#define RoQ_SOUND_STEREO	0x1021

#define RoQ_ID_MOT		0x00
#define RoQ_ID_FCC		0x01
#define RoQ_ID_SLD		0x02
#define RoQ_ID_CCC		0x03

struct roq_cell_t
{
	byte			y0, y1, y2, y3, u, v;
};

struct roq_qcell_t
{
	byte			idx[4];
};

struct roq_info_t
{
	VFILE*			stream;
	int			buf_size;
	byte*			buf;
	roq_cell_t		cells[256];
	roq_qcell_t		qcells[256];
	short			snd_sqr_arr[260];
	long			roq_start, aud_pos, vid_pos;
	long*			frame_offset;
	unsigned long		num_frames;
	unsigned long		num_audio_bytes;
	int			width, height, frame_num, audio_channels;
	byte			*y[2], *u[2], *v[2];
	long			stream_length;
	int			audio_buf_size, audio_size;
	byte*			audio;
	int			lastframe_time;
};


roq_info_t*	Roq_Open(const std::string &name);
void		Roq_Close(roq_info_t *info);
int		Roq_ReadVideo(roq_info_t *info);
int		Roq_ReadAudio(roq_info_t *info);
void		Roq_ResetStream(roq_info_t *info);

#endif // ROQ_H

