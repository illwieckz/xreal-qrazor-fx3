/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
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
// shared -------------------------------------------------------------------
#include "x_shared.h"


const vec3_t	bytedirs[NUMVERTEXNORMALS] =
{
#include "x_anorms.h"
};

const entity_state_t	null_entity_state;
const player_state_t	null_player_state;


const char*	Com_StripExtension(const std::string &name)
{
	std::string name_short;
	
	/*
	if(name[0] == '/' || name[0] == '\\')
	{
		name_short = name.substr(1, name.length());
		
		if(name[name.length()-4] == '.')
			name_short = name_short.substr(0, name.length()-4-1);
	}
	else
	{
		name_short = name;
		if(name.find('.'))
			name_short = name.substr(0, name.find('.'));
	}

	for(int i=0; i<name_short[i]; i++)
		if(name_short[i] == '\\')
			name_short[i] = '/';
	*/
	
	if(name.find_last_of('.'))
		name_short = name.substr(0, name.find_last_of('.'));
	else
		name_short = name;
		
	return name_short.c_str();
}


/*
============
Com_FilePath

Returns the path up to, but not including the last /
============
*/
void	Com_FilePath(const char *in, char *out)
{
	const char *s;
	
	s = in + strlen(in) - 1;
	
	while (s != in && *s != '/')
		s--;

	strncpy (out,in, s-in);
	out[s-in] = 0;
}



void*	Com_Alloc(int size)
{
	assert(size);
	
	void *buf = malloc(size);
	memset(buf, 0, size);
	return buf;
}

void*	Com_ReAlloc(void *ptr, int size)
{
	assert(ptr);
	assert(size);
	
	return realloc(ptr, size);
}

void	Com_Free(void *ptr)
{
//	assert(ptr);

	if(ptr)
		free(ptr);
}




/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char*	va(char *format, ...)
{
	va_list		argptr;
	static char		string[1024];
	
	va_start (argptr, format);
	vsprintf (string, format,argptr);
	va_end (argptr);

	return string;	
}




/*
==============
Com_Parse

Parse a token out of a string
==============
*/
char*	Com_Parse(char **data_p, bool allow_next_line)
{
	int		c;

	int		len;
	char	*data;
	bool	newlines = false;
	
	static char	token[MAX_TOKEN_CHARS];
	int		lines;

	data = *data_p;
	len = 0;
	
	token[0] = 0;
	lines = 0;
	
	if(!data)
	{
		*data_p = NULL;
		return "";
	}
		
	// skip whitespace
skipwhite:
	while((c = *data) <= ' ')
	{
		if(c == 0)
		{
			*data_p = NULL;
			return "";
		}
		
		if(c == '\n')
			newlines = true;
			
		data++;
	}
	
	// handle new lines specially
	if(newlines && !allow_next_line)
	{
		*data_p = data;
		return token;
	}
	
	
	// skip // comments
	if(c=='/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}
	
	
	// skip /* */ comments
	if(c=='/' && data[1] == '*')
	{
		data++;
checkcomment:
		data++;
		
		while(*data && *data != '*')
			data++;
		
		if(data[0] == '*' && data[1] == '/')
		{
			data++;
			data++;
			goto skipwhite;
		}
		else
			goto checkcomment;
	}


	// handle quoted strings specially
	if(c == '\"')
	{
		data++;
		while(true)
		{
			c = *data++;
			if(c=='\"' || !c)
			{
				token[len] = 0;
				*data_p = data;
				return token;
			}
			
			if(len < MAX_TOKEN_CHARS)
			{
				token[len] = c;
				len++;
			}
		}
	}


	// parse a regular word
	do
	{
		if(len < MAX_TOKEN_CHARS)
		{
			token[len] = c;
			len++;
		}
		data++;
		c = *data;
		
		if(c == '\n')
			lines++;
		
	}while (c>32);

	if(len == MAX_TOKEN_CHARS)
	{
//		Com_Printf("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	}
	token[len] = 0;

	*data_p = data;
	return token;
}


int	Com_ParseInt(char **data_p, bool allow_next_line)
{
	if(!data_p || !*data_p)
		return 0;
		
	//if(!**data_p || **data_p == '}')
	//	return 0;
	
	return atoi(Com_Parse(data_p, allow_next_line));
}

float	Com_ParseFloat(char **data_p, bool allow_next_line)
{
	if(!data_p || !*data_p)
		return 0;
		
	//if(!**data_p || **data_p == '}')
	//	return 0;
	
	return atof(Com_Parse(data_p, allow_next_line));
}


vec3_c	Com_ParseVec3(char **data_p)
{
	vec3_c v;
	
	if(!data_p || !*data_p)
		return v;
		
	Com_Parse(data_p);	// skip '('
		
	for(int i=0; i<3; i++)
	{
		v[i] = Com_ParseFloat(data_p);
	}
	
	Com_Parse(data_p);	// skip ')'
	
	return v;
}

void 	Com_sprintf(char *dest, int size, char *fmt, ...)
{
	int		len;
	va_list		argptr;
	char	bigbuffer[0x10000];

	memset(bigbuffer, 0, sizeof (bigbuffer));
	
	va_start(argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end(argptr);
	
	if(len >= size)
		Com_Printf("Com_sprintf: overflow of %i in %i\n", len, size);
		
	strncpy(dest, bigbuffer, size-1);
}


