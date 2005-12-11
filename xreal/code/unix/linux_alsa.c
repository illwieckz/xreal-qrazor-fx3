#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include <alsa/asoundlib.h>

#include "../game/q_shared.h"
#include "../client/snd_local.h"


static int      snd_inited;

static snd_pcm_t *pcm_handle;
static snd_pcm_hw_params_t *hw_params;
static snd_pcm_sw_params_t *sw_params;

cvar_t         *sndbits;
cvar_t         *sndspeed;
cvar_t         *sndchannels;
cvar_t         *snddevice;

/* Some devices may work only with 48000 */
//static int      tryrates[] = { 22050, 11025, 44100, 48000, 8000 };

static int      tryrates[] = { 48000, 44100, 22051, 11025, 8000 };

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
	int             framesize;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16;
	snd_pcm_uframes_t buffersize;
	snd_pcm_uframes_t fragsize;

	if(snd_inited)
	{
		return 1;
	}

	sndbits = Cvar_Get("sndbits", "16", CVAR_ARCHIVE);
	sndspeed = Cvar_Get("sndspeed", "0", CVAR_ARCHIVE);
	sndchannels = Cvar_Get("sndchannels", "2", CVAR_ARCHIVE);
	snddevice = Cvar_Get("snddevice", "default", CVAR_ARCHIVE);

	err = snd_pcm_open(&pcm_handle, snddevice->string, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if(err < 0)
	{
		Com_Printf("ALSA snd error, cannot open device %s (%s)\n", snddevice->string, snd_strerror(err));
		return 0;
	}

	err = snd_pcm_hw_params_malloc(&hw_params);
	if(err < 0)
	{
		Com_Printf("ALSA snd error, cannot allocate hw params (%s)\n", snd_strerror(err));
		return 0;
	}

	err = snd_pcm_sw_params_malloc(&sw_params);
	if(err < 0)
	{
		Com_Printf("ALSA snd error, cannot allocate hw params (%s)\n", snd_strerror(err));
		return 0;
	}

	err = snd_pcm_hw_params_any(pcm_handle, hw_params);
	if(err < 0)
	{
		Com_Printf("ALSA snd error, cannot init hw params (%s)\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_sw_params_free(sw_params);
		return 0;
	}

	err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	if(err < 0)
	{
		Com_Printf("ALSA snd error, cannot set access (%s)\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_sw_params_free(sw_params);
		return 0;
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
			format = SND_PCM_FORMAT_S16;
		}
	}

	if(dma.samplebits == 8)
	{
		err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_U8);
		if(err < 0)
		{
			Com_Printf("ALSA snd error, cannot set sample format (%s)\n", snd_strerror(err));
			snd_pcm_hw_params_free(hw_params);
			snd_pcm_sw_params_free(sw_params);
			return 0;
		}
		else
		{
			format = SND_PCM_FORMAT_U8;
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
		snd_pcm_sw_params_free(sw_params);
		return 0;
	}

	dma.speed = sndspeed->integer;
	if(!dma.speed)
	{
		for(i = 0; i < sizeof(tryrates) / 4; i++)
		{
			int             test = tryrates[i];

			err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &test, 0);
			if(err < 0)
			{
				Com_Printf("ALSA snd error, cannot set sample rate %d (%s)\n", tryrates[i], snd_strerror(err));
			}
			else
			{
				dma.speed = test;
				fragsize = 8 * dma.samplebits * dma.speed / 11025;
				break;
			}
		}
	}

	if(!dma.speed)
	{
		Com_Printf("ALSA snd error couldn't set rate.\n");
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_sw_params_free(sw_params);
		return 0;
	}

	err = snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params, &fragsize, 0);
	if(err < 0)
	{
		Com_Printf("ALSA unable to set period size near %i. %s\n", (int)fragsize, snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_sw_params_free(sw_params);
		return 0;
	}

	err = snd_pcm_hw_params(pcm_handle, hw_params);
	if(err < 0)
	{
		Com_Printf("ALSA snd error couldn't set hw params (%s).\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_sw_params_free(sw_params);
		return 0;
	}

	err = snd_pcm_sw_params_current(pcm_handle, sw_params);
	if(err < 0)
	{
		Com_Printf("ALSA snd error couldn't determine sw params (%s).\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_sw_params_free(sw_params);
		return 0;
	}

	err = snd_pcm_sw_params_set_start_threshold(pcm_handle, sw_params, ~0U);
	if(err < 0)
	{
		Com_Printf("ALSA snd error couldn't set playback threshold (%s).\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_sw_params_free(sw_params);
		return 0;
	}

	err = snd_pcm_sw_params_set_stop_threshold(pcm_handle, sw_params, ~0U);
	if(err < 0)
	{
		Com_Printf("ALSA snd error couldn't set stop threshold (%s).\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_sw_params_free(sw_params);
		return 0;
	}

	err = snd_pcm_sw_params(pcm_handle, sw_params);
	if(err < 0)
	{
		Com_Printf("ALSA snd error couldn't set sw params (%s).\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_sw_params_free(sw_params);
		return 0;
	}

	err = snd_pcm_hw_params_get_buffer_size(hw_params, &buffersize);
	if(err < 0)
	{
		Com_Printf("ALSA snd error couldn't get buffersize (%s).\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_sw_params_free(sw_params);
		return 0;
	}

	framesize = (snd_pcm_format_physical_width(format) * dma.channels) / 8;


	snd_pcm_hw_params_free(hw_params);
	snd_pcm_sw_params_free(sw_params);
	hw_params = NULL;
	sw_params = NULL;

	/*
	   if ( ( err = snd_pcm_prepare( pcm_handle ) ) < 0 ) {
	   Com_Printf("ALSA snd error preparing audio (%s)\n", snd_strerror( err ) );
	   return 0;
	   }
	 */

//  snd_bufferSize = buffersize * framesize;
//  snd_buffer = malloc( snd_bufferSize );
//    memset( snd_buffer, 0, snd_bufferSize );

//  dma.samples = info.fragstotal * info.fragsize / (dma.samplebits/8);
	dma.samples = buffersize * framesize / (dma.samplebits / 8);
//  dma.samples = buffersize * dma.channels;
	dma.submission_chunk = 1;
//  dma.buffer = (char *)snd_buffer;

	SNDDMA_GetDMAPos();			// sets dma.buffer

	snd_inited = 1;
	return 1;
}

int SNDDMA_GetDMAPos(void)
{
	const snd_pcm_channel_area_t *areas;
	snd_pcm_uframes_t offset;
	snd_pcm_uframes_t nframes = dma.samples / dma.channels;

	if(!snd_inited)
	{
		Com_Printf("Sound not inizialized\n");
		return 0;
	}

	snd_pcm_avail_update(pcm_handle);
	snd_pcm_mmap_begin(pcm_handle, &areas, &offset, &nframes);
	offset *= dma.channels;
	nframes *= dma.channels;
//  dma.samplepos = offset;
	dma.buffer = areas->addr;
	return offset;
}

void SNDDMA_Shutdown(void)
{
	if(snd_inited)
	{
		snd_pcm_drop(pcm_handle);
		snd_pcm_close(pcm_handle);
		snd_inited = 0;
	}

//  free( dma.buffer );
//  dma.buffer = NULL;
}

/*
  SNDDMA_Submit
Send sound to device if buffer isn't really the dma buffer
*/
void SNDDMA_Submit(void)
{
	extern int      s_soundtime;	// sample PAIRS
	extern int      s_paintedtime;	// sample PAIRS
	int             state;
	int             count = s_paintedtime - s_soundtime;
	const snd_pcm_channel_area_t *areas;
	snd_pcm_uframes_t nframes;
	snd_pcm_uframes_t offset;

	nframes = count / dma.channels;

	snd_pcm_avail_update(pcm_handle);
	snd_pcm_mmap_begin(pcm_handle, &areas, &offset, &nframes);

	state = snd_pcm_state(pcm_handle);

	switch (state)
	{
		case SND_PCM_STATE_PREPARED:
			snd_pcm_mmap_commit(pcm_handle, offset, nframes);
			snd_pcm_start(pcm_handle);
			break;
		case SND_PCM_STATE_RUNNING:
			snd_pcm_mmap_commit(pcm_handle, offset, nframes);
			break;
		default:
			break;
	}
}


void SNDDMA_BeginPainting(void)
{
}
