/// ============================================================================
/*
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
// qrazor-fx ----------------------------------------------------------------
#include "s_local.h"

#include "cmd.h"
#include "cvar.h"
#include "cm.h"

// xreal --------------------------------------------------------------------




s_buffer_c::s_buffer_c(const std::string &name, uint_t id, uint_t size, uint_t format, uint_t bits, uint_t freq, uint_t loop)
{
	_name			= Com_StripExtension(name);
	_registration_sequence	= s_registration_sequence;
	
	_id			= id;
	
	_size			= size;
	_format			= format;
	_bits			= bits;
	_freq			= freq;
	_loop			= loop;
	
	
	// find free sound buffer slot
	std::vector<s_buffer_c*>::iterator ir = find(s_buffers.begin(), s_buffers.end(), static_cast<s_buffer_c*>(NULL));
	
	if(ir != s_buffers.end())
		*ir = this;
	else
		s_buffers.push_back(this);
}

s_buffer_c::~s_buffer_c()
{
	alDeleteBuffers(1, &_id);
}


s_source_c::s_source_c(int entity_num, int entity_channel)
{
	alGenSources(1, &_id);	S_CheckForError();

	_entity_num	= entity_num;
	_entity_channel = entity_channel;
	
	_cluster = -1;
	
	_activated = false;

	if(s_show->getValue())
		Com_Printf("s_source_c::ctor: %i %i\n", _entity_num, _entity_channel);
	
	// find free sound source slot
	std::vector<s_source_c*>::iterator ir = find(s_sources.begin(), s_sources.end(), static_cast<s_source_c*>(NULL));
	
	if(ir != s_sources.end())
		*ir = this;
	else
		s_sources.push_back(this);
}

s_source_c::~s_source_c()
{
	if(s_show->getValue())
		Com_Printf("s_source_c::dtor: %i %i\n", _entity_num, _entity_channel);

	stop();

	alDeleteSources(1, &_id);	S_CheckForError();
	
	// clear slot
	/*
	std::vector<s_source_c*>::iterator ir = find(s_sources.begin(), s_sources.end(), static_cast<s_source_c*>(this));
	
	if(ir != s_sources.end())
		*ir = NULL;
	*/
}



void	s_source_c::setBuffer(s_buffer_c *buffer)
{
	_buffer = buffer;

	if(_buffer)
		alSourcei(_id, AL_BUFFER, _buffer->getId());
}

void	s_source_c::setPosition(const vec3_c &v)
{
	int leafnum = CM_PointLeafnum(v);
	_cluster = CM_LeafCluster(leafnum);

	// convert from Quake coords to OpenAL coords
	_position[0] = v[1];
	_position[1] = v[2];
	_position[2] =-v[0];
	
	// convert from Quake units to OpenAL meters
	_position.scale(1.0/32.0);
				
	alSourcefv(_id, AL_POSITION, _position);
	
	S_CheckForError();
}

void	s_source_c::setVelocity(const vec3_c &v)
{
	_velocity[0] = v[1];
	_velocity[1] = v[2];
	_velocity[2] =-v[0];
	
	_velocity.scale(1.0/32.0);
				
	alSourcefv(_id, AL_VELOCITY, _velocity);
	
	S_CheckForError();
}

void	s_source_c::setGain(float gain)
{
	alSourcef(_id, AL_GAIN, gain);
	
	S_CheckForError();
}

void	s_source_c::setMinGain(float gain)
{
	alSourcef(_id, AL_MIN_GAIN, gain);
	
	S_CheckForError();
}

void	s_source_c::setMaxGain(float gain)
{
	alSourcef(_id, AL_MAX_GAIN, gain);
	
	S_CheckForError();
}

void	s_source_c::setRefDistance(float dist)
{
	alSourcef(_id, AL_REFERENCE_DISTANCE, dist);
	
	S_CheckForError();
}

void	s_source_c::setMaxDistance(float dist)
{
	alSourcef(_id, AL_MAX_DISTANCE, dist);
	
	S_CheckForError();
}

void	s_source_c::setRolloffFactor(float factor)
{
	alSourcef(_id, AL_ROLLOFF_FACTOR, factor);
	
	S_CheckForError();
}

void	s_source_c::setPitch(float pitch)
{
	alSourcef(_id, AL_PITCH, pitch);
	
	S_CheckForError();
}

void	s_source_c::setLooping(bool looping)
{
	_looping = looping;
	
	alSourcef(_id, AL_LOOPING, _looping);
	
	S_CheckForError();
}

bool	s_source_c::isPlaying() const
{
//	if(alIsSource(_id) == AL_FALSE)
//		return AL_FALSE;

	ALint state = AL_INITIAL;
	alGetSourcei(_id, AL_SOURCE_STATE, &state);	S_CheckForError();
	
	if(state == AL_PLAYING)
		return true;

	return false;
}

bool	s_source_c::isPaused() const
{
//	if(alIsSource(_id) == AL_FALSE)
//		return AL_FALSE;

	ALint state = AL_INITIAL;
	alGetSourcei(_id, AL_SOURCE_STATE, &state);	S_CheckForError();
	
	if(state == AL_PAUSED)
		return true;
		
	return false;
}

bool	s_source_c::isStopped() const
{
//	if(alIsSource(_id) == AL_FALSE)
//		return AL_FALSE;

	ALint state = AL_INITIAL;
	alGetSourcei(_id, AL_SOURCE_STATE, &state);	S_CheckForError();
	
	if(state == AL_STOPPED)
		return true;
		
	return false;
}

void	s_source_c::play()
{
	alSourcePlay(_id);	S_CheckForError();
			
	_activated = true;
}

void	s_source_c::pause()
{
	alSourcePause(_id);	S_CheckForError();
			
	_activated = true;
}

void	s_source_c::stop()
{
	alSourceStop(_id);	S_CheckForError();
}


uint_t		s_registration_sequence;

bool		s_initialized = false;


cvar_t	*s_nosound;
cvar_t	*s_sfxvolume;
cvar_t	*s_musicvolume;
cvar_t	*s_khz;
cvar_t	*s_show;
cvar_t	*s_testsound;

cvar_t	*s_loki_attenuation_scale;
cvar_t	*s_ext_vorbis;
cvar_t	*s_ext_mp3;

vec3_c	s_origin;

vec3_c	s_velocity;

vec3_c	s_forward;
vec3_c	s_right;
vec3_c	s_up;


std::vector<s_buffer_c*>	s_buffers;
std::vector<s_source_c*>	s_sources;
std::vector<s_shader_c*>	s_shaders;



static void	S_SoundList_f()
{
	for(std::vector<s_buffer_c*>::const_iterator ir = s_buffers.begin(); ir != s_buffers.end(); ir++)
	{
		s_buffer_c *buffer = *ir;
		
		if(!buffer)
			continue;
		
		Com_Printf("'%s'\n", buffer->getName());
	}
	
	//Com_Printf("Total resident: %i\n", );
}

static void	S_Play_f()
{
	int 		i;
	std::string	name;
		
	i = 1;
	while(i<Cmd_Argc())
	{
		if(!strrchr(Cmd_Argv(i), '.'))
		{
			name = Cmd_Argv(i);
			name += ".wav";
		}
		else
			name = Cmd_Argv(i);
		
		S_StartLocalSound(name);
		i++;
	}
}


void	S_Init()
{
	Com_Printf("------- S_Init -------\n");
	
	s_nosound = Cvar_Get("s_nosound", "0", CVAR_NONE);
	
	if(s_nosound->getValue())
	{
		Com_Printf("not initializing.\n");
	}
	else
	{
		s_sfxvolume 			= Cvar_Get("s_sfxvolume", "0.7", CVAR_ARCHIVE);
		s_musicvolume 			= Cvar_Get("s_musicvolume", "0.7", CVAR_ARCHIVE);
		s_khz 				= Cvar_Get("s_khz", "11", CVAR_ARCHIVE);
		s_show				= Cvar_Get("s_show", "0", 0);
		s_testsound			= Cvar_Get("s_testsound", "0", 0);
		
		// openal extensions
		s_loki_attenuation_scale	= Cvar_Get("s_loki_attenuation_scale", "1", CVAR_ARCHIVE);
		s_ext_vorbis 			= Cvar_Get("s_ext_vorbis", "1", CVAR_ARCHIVE);
		s_ext_mp3			= Cvar_Get("s_ext_mp3", "1", CVAR_ARCHIVE);
		
		Cmd_AddCommand("soundlist",	S_SoundList_f);
		
		Cmd_AddCommand("play",		S_Play_f);
		Cmd_AddCommand("stopsounds",	S_StopAllSounds);
		
		
		S_InitOpenAL();
		
		S_InitShaders();
	}

	Com_Printf("------------------------------------\n");
}

static void	S_ShutdownBuffers()
{
	X_purge<std::vector<s_buffer_c*> >(s_buffers);
	
	s_buffers.clear();
}

static void	S_ShutdownSources()
{
	X_purge<std::vector<s_source_c*> >(s_sources);

	s_sources.clear();
}

void	S_StopAllSounds()
{
	if(!s_initialized)
		return;
		
	S_ShutdownSources();
}

void	S_Shutdown()
{
	if(!s_initialized)
		return;

	Com_Printf("------- S_Shutdown -------\n");
	
	S_ShutdownBuffers();
	
	S_ShutdownSources();
	
	S_ShutdownShaders();
	
	S_ShutdownOpenAL();

	s_initialized = 0;

	Cmd_RemoveCommand("soundlist");
	
	Cmd_RemoveCommand("play");
	Cmd_RemoveCommand("stopsound");
}




void	S_BeginRegistration()
{
	s_registration_sequence++;
}

int	S_RegisterSound(const std::string &name)
{
	if(name.empty())
	{
		//Com_Error(ERR_DROP, "S_RegisterSound: empty name\n");
		return -1;
	}
		
	return S_GetNumForShader(S_FindShader(name));
}

void	S_EndRegistration()
{
	S_FreeUnusedShaders();
	S_FreeUnusedBuffers();
}




s_buffer_c*	S_FindBuffer(const std::string &name)
{
	if(name.empty())
		Com_Error(ERR_FATAL, "S_FindBuffer: empty name\n");
		
	std::string name_short = X_strlwr(Com_StripExtension(name));
			
	for(std::vector<s_buffer_c*>::const_iterator ir = s_buffers.begin(); ir != s_buffers.end(); ir++)
	{
		s_buffer_c* buffer = *ir;
		
		if(!buffer)
			continue;
		
		if(X_strcaseequal(name_short.c_str(), buffer->getName()))
		{
			buffer->setRegistrationSequence();
			return buffer;
		}
	}
	
	//
	// load sound from disc
	//
	return S_LoadBuffer(name_short);
}

int	S_GetNumForBuffer(s_buffer_c *buffer)
{
	if(!buffer)
	{
		//Com_Error(ERR_DROP, "S_GetNumForSound: NULL parameter\n");
		return -1;
	}

	for(unsigned int i=0; i<s_buffers.size(); i++)
	{
		if(s_buffers[i] == buffer)
			return i;
	}
	
	Com_Error(ERR_DROP, "S_GetNumForBuffer: bad pointer\n");
	return -1;
}


s_buffer_c*	S_GetBufferByNum(int num)
{
	if(num < 0 || num >= (int)s_buffers.size())
	{
		Com_Error(ERR_DROP, "S_GetBufferByNum: bad number %i\n", num);
		return NULL;
	}

	return s_buffers[num];
}

void	S_FreeUnusedBuffers()
{
	for(std::vector<s_buffer_c*>::iterator ir = s_buffers.begin(); ir != s_buffers.end(); ir++)
	{
		s_buffer_c *buffer = *ir;
		
		if(!buffer)
			continue;
		
		if(buffer->getRegistrationSequence() == s_registration_sequence)
			continue;		// used this sequence
		
		// free it
		delete buffer;
		
		*ir = NULL;
	}
}
