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
#ifndef S_LOCAL_H
#define S_LOCAL_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// system -------------------------------------------------------------------
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

// qrazor-fx ----------------------------------------------------------------
#include "s_public.h"

// xreal --------------------------------------------------------------------


extern uint_t		s_registration_sequence;

enum s_sound_type_t
{
	SND_TYPE_OMNI,
	SND_TYPE_DIRECTIONAL
};


class s_buffer_c
{
public:
	s_buffer_c(const std::string &name, uint_t id, uint_t size, uint_t format, uint_t bits, uint_t freq, uint_t loop);
	
	~s_buffer_c();
	
	const char*	getName() const			{return _name.c_str();}
	
	uint_t		getRegistrationSequence()	{return _registration_sequence;}
	void		setRegistrationSequence()	{_registration_sequence = s_registration_sequence;}
	
	uint_t		getId() const			{return _id;}


private:
	std::string	_name;
//	std::string	_name_true;
	uint_t		_registration_sequence;
	
	uint_t		_id;
	
	uint_t 		_size;
	uint_t		_format;
	uint_t		_bits;
	uint_t		_freq;
	uint_t		_loop;
};
	


class s_source_c
{
public:
	s_source_c(int entity_num, int entity_channel);
	
	~s_source_c();
	
	bool		isPlaying();
	void		setBuffer(s_buffer_c *buffer);
	void		setPosition(const vec3_c &v);
	void		setVelocity(const vec3_c &v);
	
	void		setGain(float gain);
	
	void		setMinGain(float gain);
	void		setMaxGain(float gain);
	
	void		setRefDistance(float dist);
	void		setMaxDistance(float dist);
	void		setRolloffFactor(float factor);
	
	void		setPitch(float pitch);
	
	void		setLooping(bool looping);
	
	
	uint_t		getId() const			{return _id;}
	
	bool		hasBuffer()			{return _buffer ? true : false;}
	
	bool		isLoopSound() const		{return _looping;}
	
	int		getEntityNum() const		{return _entity_num;}
	void		setEntityNum(int num)		{_entity_num = num;}
	
	int		getEntityChannel() const	{return _entity_channel;}
	void		setEntityChannel(int channel)	{_entity_channel = channel;}
	
	bool		isActivated() const		{return _activated;}
	void		isActivated(bool activated)	{_activated = activated;}
	
private:
	uint_t		_id;

	s_buffer_c*	_buffer;
	
	vec3_c		_position;		// in OpenAL coords and meters instead of units
	vec3_c		_velocity;		// same here
	bool		_looping;		// from entity_state_t::sound
	
	int		_entity_num;		// to allow overriding a specific sound
	int		_entity_channel;
	
	bool		_activated;
};



class s_shader_c
{
	friend void	S_Buffer_sc(char const* begin, char const* end);
	friend void	S_FreeUnusedShaders();

public:
	s_shader_c(const std::string &name);
	
	~s_shader_c();

	const char*	getName() const			{return _name.c_str();}
	
	uint_t		getRegistrationSequence()	{return _registration_sequence;}
	void		setRegistrationSequence()	{_registration_sequence = s_registration_sequence;}
	
	s_sound_type_t	getType() const			{return _type;}
	
	void		setGain(float gain)		{_gain = gain;}
	void		setMinGain(float gain)		{_gain_min = gain;}
	void		setMaxGain(float gain)		{_gain_max = gain;}
	
	// distance model attributes
	void		setRefDistance(float dist)	{_distance_ref = dist;}
	void		setMaxDistance(float dist)	{_distance_max = dist;}
	void		setRolloffFactor(float factor)	{_rolloff_factor = factor;}
	
	void		setPitch(float pitch)		{_pitch = pitch;}
	
	bool		hasBuffers() const		{return _buffers.size();}
	
	void		createDefaultBuffer();		// called if no shader cache was found
	
	// will be called every time a shader is running
	void		createSource(const vec3_c &origin, const vec3_c &velocity, int ent_num, int ent_channel, bool looping);
		
private:
	s_buffer_c*	selectRandomBuffer();

	std::string			_name;
	uint_t				_registration_sequence;
	
	// attributes set by the scripted shader
	s_sound_type_t			_type;
	
	float				_gain;
	float				_gain_min;
	float				_gain_max;
	
	float				_distance_ref;
	float				_distance_max;
	float				_rolloff_factor;
	
	float				_pitch;
	
	std::vector<s_buffer_c*>	_buffers;	// all possible sound buffers that could be played by this shader
};

struct alconfig_t
{
	int         	renderer;
	const char*	renderer_string;
	const char*	vendor_string;
	const char*	version_string;
	const char*	extensions_string;
	
	// save here what extensions are currently available
	bool		loki_attenuation_scale;
	bool		ext_vorbis;
	bool		ext_mp3;
};


extern bool		s_initialized;

extern cvar_t	*s_nosound;
extern cvar_t	*s_sfxvolume;
extern cvar_t	*s_musicvolume;
extern cvar_t	*s_khz;
extern cvar_t	*s_show;
extern cvar_t	*s_testsound;

extern cvar_t	*s_loki_attenuation_scale;
extern cvar_t	*s_ext_vorbis;
extern cvar_t	*s_ext_mp3;



extern vec3_c	s_origin;

extern vec3_c	s_velocity;

extern vec3_c	s_forward;
extern vec3_c	s_right;
extern vec3_c	s_up;


extern std::vector<s_buffer_c*>	s_buffers;
extern std::vector<s_source_c*>	s_sources;
extern std::vector<s_shader_c*>	s_shaders;


//
// s_backend
//
void		S_InitOpenAL();
void		S_ShutdownOpenAL();

s_buffer_c*	S_LoadBuffer(const std::string &name);



//
// s_main
//
void		S_Init();
void		S_Shutdown();

void		S_BeginRegistration();
int		S_RegisterSound(const std::string &name);
void 		S_EndRegistration();

s_buffer_c*	S_FindBuffer(const std::string &name);

int		S_GetNumForBuffer(s_buffer_c *buffer);
s_buffer_c*	S_GetBufferByNum(int num);

void		S_FreeUnusedBuffers();


//
// s_shader
//
void		S_InitShaders();
void		S_ShutdownShaders();

void		S_ShaderList_f();
void		S_ShaderCacheList_f();
void		S_ShaderSearch_f();
void		S_SpiritTest_f();


s_shader_c*	S_FindShader(const std::string &name);

int		S_GetNumForShader(s_shader_c *shader);
s_shader_c*	S_GetShaderByNum(int num);

void		S_FreeUnusedShaders();


#endif // S_LOCAL_H
