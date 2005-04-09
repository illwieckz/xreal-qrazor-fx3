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
// shared -------------------------------------------------------------------
#include "x_shared.h"
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------




bool	X_strcaseequal(const char *a, const char *b)
{

	if(!a)
	{
		Com_Error(ERR_FATAL, "X_strcaseequal: NULL a");
		return false;
	}
	
	if(!b)
	{
		Com_Error(ERR_FATAL, "X_strcaseequal: NULL b");
		return false;
	}
	
	return (strcasecmp (a, b) == 0);
}

int 	X_strncasecmp(const char *s1, const char *s2, int n)
{
	int		c1, c2;
	
	do
	{

		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;		// strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;		// strings not equal
		}
	} while (c1);
	
	return 0;		// strings are equal
}

char*	X_strlwr(char *s)
{
	while (*s) {
		*s = tolower(*s);
		s++;
	}

	return s;
}

std::string	X_strlwr(const std::string &s)
{
	std::string out = "";
	
	for(unsigned int i=0; i<s.length(); i++)
		out += tolower(s[i]);
		
	return out;
}

char*	X_strupr(char *s)
{
	while (*s) {
		*s = toupper(*s);
		s++;
	}

	return s;
}

void	X_strncpyz(char *dest, const char *src, int size)
{
	strncpy (dest, src, size-1);
	dest[size-1] = 0;
}
