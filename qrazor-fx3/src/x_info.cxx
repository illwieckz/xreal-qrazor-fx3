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
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility.hpp>

// shared -------------------------------------------------------------------
#include "x_shared.h"

// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------


void	info_c::setValueForKey(const std::string &key, const std::string &value)
{
	if(strstr(key.c_str(), "\\") || strstr(value.c_str(), "\\"))
	{
		Com_Printf("Can't use keys or values with a \\\n");
		return;
	}

	if(strstr(key.c_str(), ";"))
	{
		Com_Printf("Can't use keys or values with a semicolon\n");
		return;
	}

	if(strstr(key.c_str(), "\"") || strstr(value.c_str(), "\""))
	{
		Com_Printf("Can't use keys or values with a \"\n");
		return;
	}

	if(key.length() > MAX_INFO_KEY-1 || value.length() > MAX_INFO_KEY-1)
	{
		Com_Printf("Keys and values must be < %i characters.\n", MAX_INFO_KEY);
		return;
	}
	
	std::map<std::string, std::string>::iterator ir = _map.find(key);
	
	if(ir != _map.end())
	{
		ir->second = value;
	}
	else
	{
		_map.insert(make_pair(key, value));
	}
}

const char*	info_c::getValueForKey(const std::string &key)
{
	std::map<std::string, std::string>::iterator ir = _map.find(key);
	
	if(ir != _map.end())
		return ir->second.c_str();
	else
		return NULL;
}

void	info_c::removeKey(const std::string &key)
{
	if(strstr(key.c_str(), "\\"))
	{
		Com_Printf("Can't remove keys with a \\\n");
		return;
	}

	std::map<std::string, std::string>::iterator ir = _map.find(key);
	
	if(ir != _map.end())
		_map.erase(ir);
}



void	info_c::fromString(const std::string &s)
{
	std::vector<std::string> v;

	boost::spirit::parse_info<> info = boost::spirit::parse
	(	
		s.c_str(),
		(
		// start grammar
		*(boost::spirit::ch_p('\\') >> boost::spirit::refactor_unary_d[*boost::spirit::anychar_p - boost::spirit::ch_p('\\')][boost::spirit::append(v)])
		// end grammar	
		)
	);
	
	if(!info.full)
	{
		Com_Error(ERR_FATAL, "info_c::fromString: parsing failed");
		return;
	}
	
	if(!v.size())
	{
		Com_Error(ERR_FATAL, "info_c::fromString: empty!");
		return;
	}
	
	/*
	for(uint_t i=0; i<v.size(); i += 2)
	{
		Com_Printf("'%s' '%s'\n", v[i].c_str(), v[i+1].c_str());
	}
	*/
	
	if(v.size() % 2)
	{
		Com_Error(ERR_FATAL, "info_c::fromString: missing value for key '%s'", v[v.size()-1].c_str());
		return;
	}
	
	for(uint_t i=0; i<v.size(); i += 2)
	{
		setValueForKey(v[i], v[i+1]);
	}
}

const char*	info_c::toString() const
{
	std::string s;
	
	for(std::map<std::string, std::string>::const_iterator ir = _map.begin(); ir != _map.end(); ir++)
	{
		s += '\\';
		s += ir->first;
		
		s += '\\';
		s += ir->second;
	}

	return s.c_str();
}

void	info_c::print() const
{
	std::string s;
	
	for(std::map<std::string, std::string>::const_iterator ir = _map.begin(); ir != _map.end(); ir++)
	{
		s += '\''; s += ir->first; s += '\'';
		s += '\t';
		s += s += '\''; ir->second; s += '\'';
	}

	Com_Printf("%s\n", s.c_str());
}




/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
/*
const char*	Info_ValueForKey (char *s, const char *key)
{
	char	pkey[512];
	static	char value[2][512];	// use two buffers so compares
								// work without stomping on each other
	static	int	valueindex;
	char	*o;
	
	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
			return value[valueindex];

		if (!*s)
			return "";
		s++;
	}
}

void 	Info_RemoveKey (char *s, const char *key)
{
	char	*start;
	char	pkey[512];
	char	value[512];
	char	*o;

	if (strstr (key, "\\"))
	{
//		Com_Printf ("Can't use a key with a \\\n");
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			strcpy (start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}




//Some characters are illegal in info strings because they
//can mess up the server's parsing
bool 	Info_Validate (const char *s)
{
	if (strstr (s, "\""))
		return false;
		
	if (strstr (s, ";"))
		return false;
		
	return true;
}

void 	Info_SetValueForKey(char *s, const char *key, const char *value)
{
	char	newi[MAX_INFO_STRING], *v;
	int		c;
	int		maxsize = MAX_INFO_STRING;

	if(strstr (key, "\\") || strstr (value, "\\") )
	{
		Com_Printf ("Can't use keys or values with a \\\n");
		return;
	}

	if(strstr (key, ";") )
	{
		Com_Printf ("Can't use keys or values with a semicolon\n");
		return;
	}

	if(strstr (key, "\"") || strstr (value, "\"") )
	{
		Com_Printf ("Can't use keys or values with a \"\n");
		return;
	}

	if(strlen(key) > MAX_INFO_KEY-1 || strlen(value) > MAX_INFO_KEY-1)
	{
		Com_Printf ("Keys and values must be < 64 characters.\n");
		return;
	}
	
	Info_RemoveKey(s, key);
	
	if(!value || !strlen(value))
		return;

	Com_sprintf(newi, (int)sizeof(newi), "\\%s\\%s", key, value);

	if((int)strlen(newi) + (int)strlen(s) > maxsize)
	{
		Com_Printf ("Info string length exceeded\n");
		return;
	}

	// only copy ascii values
	s += strlen(s);
	v = newi;
	while(*v)
	{
		c = *v++;
		c &= 127;		// strip high bits
		
		if(c >= 32 && c < 127)

			*s++ = c;
	}
	*s = 0;
}

*/

