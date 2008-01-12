/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2006 Jason Thomas Dolan <jay@jaydolan.com>

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
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
//#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include <alsa/asoundlib.h>

#include "../game/q_shared.h"
#include "../client/snd_local.h"


static int      snd_inited = 0;

static snd_pcm_t *pcm_handle;
static snd_pcm_hw_params_t *hw_params;

/*
 * These are reasonable default values for good latency.  If ALSA
 * playback stutters or plain does not work, try adjusting these.
 * Period must always be a multiple of 2.  Buffer must always be
 * a multiple of period.  See http://alsa-project.org.
 */
static snd_pcm_uframes_t period_size = 1024;
static snd_pcm_uframes_t buffer_size = 4096;

static int      sample_bytes;
static int      buffer_bytes;

cvar_t         *sndbits;
cvar_t         *sndspeed;
cvar_t         *sndchannels;
cvar_t         *snddevice;

/* Some devices may work only with 48000 */
//static int      tryrates[] = { 22050, 11025, 44100, 48000, 8000 };
static int      tryrates[] = { 48000, 44100, 22050, 11025, 8000 };

static qboolean use_custom_memset = qfalse;

/*
===============
Snd_Memset

https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=371

<TTimo> some shitty mess with DMA buffers
<TTimo> the mmap'ing permissions were write only
<TTimo> and glibc optimized for mmx would do memcpy with a prefetch and a read
<TTimo> causing segfaults
<TTimo> some other systems would not let you mmap the DMA with read permissions
<TTimo> so I think I ended up attempting opening with read/write, then try write only
<TTimo> and use my own copy instead of the glibc crap
===============
*/
void Snd_Memset(void *dest, const int val, const size_t count)
{
	int            *pDest;
	int             i, iterate;

	if(!use_custom_memset)
	{
		Com_Memset(dest, val, count);
		return;
	}

	iterate = count / sizeof(int);
	pDest = (int *)dest;
	for(i = 0; i < iterate; i++)
	{
		pDest[i] = val;
	}
}

qboolean SNDDMA_Init(void)
{
	int             i;
	int             err;

	if(snd_inited)
	{
		return qtrue;
	}

	sndbits = Cvar_Get("sndbits", "16", CVAR_ARCHIVE);
	sndspeed = Cvar_Get("sndspeed", "0", CVAR_ARCHIVE);
	sndchannels = Cvar_Get("sndchannels", "2", CVAR_ARCHIVE);
	snddevice = Cvar_Get("snddevice", "default", CVAR_ARCHIVE);

	err = snd_pcm_open(&pcm_handle, snddevice->string, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if(err < 0)
	{
		Com_Printf("ALSA snd error, cannot open device %s (%s)\n", snddevice->string, snd_strerror(err));
		return qfalse;
	}

	err = snd_pcm_hw_params_malloc(&hw_params);
	if(err < 0)
	{
		Com_Printf("ALSA snd error, cannot allocate hw params (%s)\n", snd_strerror(err));
		return qfalse;
	}

	err = snd_pcm_hw_params_any(pcm_handle, hw_params);
	if(err < 0)
	{
		Com_Printf("ALSA snd error, cannot init hw params (%s)\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return qfalse;
	}

	err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(err < 0)
	{
		Com_Printf("ALSA snd error, cannot set access (%s)\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return qfalse;
	}

	dma.samplebits = sndbits->integer;
	if(dma.samplebits == 16 || dma.samplebits != 8)
	{
		err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16);
		if(err < 0)
		{
			Com_Printf("ALSA snd error, %i bit sound not supported, trying 8\n", dma.samplebits);
			dma.samplebits = 8;
		}
		else
		{
			dma.samplebits = 16;
		}
	}

	if(dma.samplebits == 8)
	{
		err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_U8);
		if(err < 0)
		{
			Com_Printf("ALSA snd error, cannot set sample format (%s)\n", snd_strerror(err));
			snd_pcm_hw_params_free(hw_params);
			return qfalse;
		}
	}

	dma.channels = sndchannels->integer;
	if(dma.channels < 1 || dma.channels > 2)
		dma.channels = 2;

	err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, dma.channels);
	if(err < 0)
	{
		Com_Printf("ALSA snd error couldn't set channels %d (%s).\n", dma.channels, snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return qfalse;
	}

	dma.speed = sndspeed->integer;
	if(!dma.speed)
	{
		// try all rates
		int             test;
		int             dir;

		for(i = 0; i < sizeof(tryrates) / 4; i++)
		{
			test = tryrates[i];
			dir = 0;

			err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &test, &dir);
			if(err < 0)
			{
				Com_Printf("ALSA snd error, cannot set sample rate %d (%s)\n", tryrates[i], snd_strerror(err));
			}
			else
			{
				// rate succeeded, but is perhaps slightly different
				if(dir != 0)
					Com_Printf("ALSA snd error, rate %d not supported, using %d\n", dma.speed, test);

				dma.speed = test;
				break;
			}
		}
	}
	else
	{
		// try specific rate
		int             test;
		int             dir;

		test = dma.speed;
		dir = 0;

		dma.speed = 0;			// zero so we're caught on failure

		err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &test, &dir);
		if(err < 0)
		{
			Com_Printf("ALSA snd error, cannot set sample rate %d (%s)\n", tryrates[i], snd_strerror(err));
		}
		else
		{
			// rate succeeded, but is perhaps slightly different
			if(dir != 0)
				Com_Printf("ALSA snd error, rate %d not supported, using %d\n", dma.speed, test);

			dma.speed = test;
		}
	}

	if(!dma.speed)
	{
		Com_Printf("ALSA snd error couldn't set rate.\n");
		snd_pcm_hw_params_free(hw_params);
		return qfalse;
	}

	err = snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params, &period_size, 0);
	if(err < 0)
	{
		Com_Printf("ALSA unable to set period size near (%s)\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return qfalse;
	}

	err = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hw_params, &buffer_size);
	if(err < 0)
	{
		Com_Printf("ALSA unable to set buffer size near (%s)\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return qfalse;
	}

	err = snd_pcm_hw_params(pcm_handle, hw_params);
	if(err < 0)
	{
		Com_Printf("ALSA snd error couldn't set hw params (%s).\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return qfalse;
	}

	Com_Printf("ALSA: period size %d, buffer size %d\n", period_size, buffer_size);

	sample_bytes = dma.samplebits / 8;
	buffer_bytes = buffer_size * dma.channels * sample_bytes;

	// allocate pcm frame buffer
	dma.buffer = Com_Allocate(buffer_bytes);
	Com_Memset(dma.buffer, 0, buffer_bytes);

	dma.samples = buffer_size * dma.channels;
	dma.submission_chunk = period_size * dma.channels;

	dma.samplepos = 0;

	snd_pcm_prepare(pcm_handle);

	snd_inited = 1;
	return qtrue;
}

int SNDDMA_GetDMAPos(void)
{
	if(!snd_inited)
	{
		Com_Printf("Sound not inizialized\n");
		return 0;
	}
	else
	{
		return dma.samplepos;
	}
}

void SNDDMA_Shutdown(void)
{
	if(snd_inited)
	{
		snd_pcm_drop(pcm_handle);
		snd_pcm_close(pcm_handle);

		Com_Dealloc(dma.buffer);
		dma.buffer = NULL;

		snd_inited = 0;
	}
}

/*
  SNDDMA_Submit
Send sound to device if buffer isn't really the dma buffer
*/
void SNDDMA_Submit(void)
{
	int             s, w, frames;
	void           *start;

	if(!dma.buffer)
		return;

	s = dma.samplepos * sample_bytes;
	start = (void *)&dma.buffer[s];

	frames = dma.submission_chunk / dma.channels;

	if((w = snd_pcm_writei(pcm_handle, start, frames)) < 0)
	{
		// write to card
		snd_pcm_prepare(pcm_handle);	// xrun occured
		return;
	}

	dma.samplepos += w * dma.channels;	// mark progress

	if(dma.samplepos >= dma.samples)
		dma.samplepos = 0;		// wrap buffer
}


void SNDDMA_BeginPainting(void)
{
}
