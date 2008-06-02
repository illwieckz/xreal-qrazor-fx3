/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// snd_mem.c - sound caching and memory managment
#include "snd_local.h"
#include "snd_codec.h"

#define DEF_COMSOUNDMEGS "8"

static sndBuffer *sndbuffers = NULL;
static sndBuffer *freelist = NULL;
static int      sndmem_avail = 0;
static int      sndmem_inuse = 0;

void SND_free(sndBuffer * v)
{
	*(sndBuffer **) v = freelist;
	freelist = (sndBuffer *) v;
	sndmem_avail += sizeof(sndBuffer);
}

sndBuffer      *SND_malloc(void)
{
	sndBuffer      *v;

	while(!freelist)
	{
		S_FreeOldestSound();
	}

	sndmem_avail -= sizeof(sndBuffer);
	sndmem_inuse += sizeof(sndBuffer);

	v = freelist;
	freelist = *(sndBuffer **) freelist;
	v->next = NULL;
	return v;
}

void SND_setup(void)
{
	sndBuffer      *p, *q;

	// a sndBuffer is actually 2K+, so this isn't even REMOTELY close to actual megs
	const cvar_t   *cv = Cvar_Get("com_soundMegs", DEF_COMSOUNDMEGS, CVAR_LATCH | CVAR_ARCHIVE);
	int             scs = cv->integer * 1024;

	sndbuffers = (sndBuffer *) Hunk_Alloc(scs * sizeof(sndBuffer), h_high);
	sndmem_avail = scs * sizeof(sndBuffer);

	p = sndbuffers;
	q = p + scs;
	while(--q > p)
		*(sndBuffer **) q = q - 1;

	*(sndBuffer **) q = NULL;
	freelist = p + scs - 1;

	Com_Printf("Sound memory manager started\n");
}

/*
================
ResampleSfx

resample / decimate to the current source rate
================
*/
static void ResampleSfx(sfx_t * sfx, int inrate, int inwidth, byte * data, qboolean compressed)
{
	int             outcount;
	int             srcsample;
	float           stepscale;
	int             i;
	int             sample, samplefrac, fracstep;
	int             part;
	sndBuffer      *chunk;

	stepscale = (float)inrate / dma.speed;	// this is usually 0.5, 1, or 2

	outcount = sfx->soundLength / stepscale;
	sfx->soundLength = outcount;

	samplefrac = 0;
	fracstep = stepscale * 256;
	chunk = sfx->soundData;

	for(i = 0; i < outcount; i++)
	{
		srcsample = samplefrac >> 8;
		samplefrac += fracstep;
		if(inwidth == 2)
		{
			sample = (((short *)data)[srcsample]);
		}
		else
		{
			sample = (int)((unsigned char)(data[srcsample]) - 128) << 8;
		}
		part = (i & (SND_CHUNK_SIZE - 1));
		if(part == 0)
		{
			sndBuffer      *newchunk;

			newchunk = SND_malloc();
			if(chunk == NULL)
			{
				sfx->soundData = newchunk;
			}
			else
			{
				chunk->next = newchunk;
			}
			chunk = newchunk;
		}

		chunk->sndChunk[part] = sample;
	}
}

/*
================
ResampleSfxRaw

resample / decimate to the current source rate
================
*/
static int ResampleSfxRaw(short *sfx, int inrate, int inwidth, int samples, byte * data)
{
	int             outcount;
	int             srcsample;
	float           stepscale;
	int             i;
	int             sample, samplefrac, fracstep;

	stepscale = (float)inrate / dma.speed;	// this is usually 0.5, 1, or 2

	outcount = samples / stepscale;

	samplefrac = 0;
	fracstep = stepscale * 256;

	for(i = 0; i < outcount; i++)
	{
		srcsample = samplefrac >> 8;
		samplefrac += fracstep;
		if(inwidth == 2)
		{
			sample = LittleShort(((short *)data)[srcsample]);
		}
		else
		{
			sample = (int)((unsigned char)(data[srcsample]) - 128) << 8;
		}
		sfx[i] = sample;
	}
	return outcount;
}

//=============================================================================

/*
==============
S_LoadSound

The filename may be different than sfx->name in the case
of a forced fallback of a player specific sound
==============
*/
qboolean S_LoadSound(sfx_t * sfx)
{
	byte           *data;
	short          *samples;
	snd_info_t      info;

	// player specific sounds are never directly loaded
	if(sfx->soundName[0] == '*')
	{
		return qfalse;
	}

	// load it in
	data = S_CodecLoad(sfx->soundName, &info);
	if(!data)
		return qfalse;

	if(info.width == 1)
	{
		Com_DPrintf(S_COLOR_YELLOW "WARNING: %s is a 8 bit wav file\n", sfx->soundName);
	}

	if(info.rate != 22050)
	{
		Com_DPrintf(S_COLOR_YELLOW "WARNING: %s is not a 22kHz wav file\n", sfx->soundName);
	}

	samples = Hunk_AllocateTempMemory(info.samples * sizeof(short) * 2);

	sfx->lastTimeUsed = Com_Milliseconds();
	sfx->soundLength = info.samples;
	sfx->soundData = NULL;

	ResampleSfx(sfx, info.rate, info.width, data + info.dataofs, qfalse);

	Hunk_FreeTempMemory(samples);
	Z_Free(data);

	return qtrue;
}

void S_DisplayFreeMemory(void)
{
	Com_Printf("%d bytes sound buffer memory in use, %d free \n", sndmem_inuse, sndmem_avail);
}
