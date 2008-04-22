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
//
// q_shared.c -- stateless support routines that are included in each code dll
#include "q_shared.h"

/*
============================================================================

GROWLISTS

============================================================================
*/

// malloc / free all in one place for debugging
//extern          "C" void *Com_Allocate(int bytes);
//extern          "C" void Com_Dealloc(void *ptr);

void Com_InitGrowList(growList_t * list, int maxElements)
{
	list->maxElements = maxElements;
	list->currentElements = 0;
	list->elements = (void **)Com_Allocate(list->maxElements * sizeof(void *));
}

void Com_DestroyGrowList(growList_t * list)
{
	Com_Dealloc(list->elements);
	memset(list, 0, sizeof(*list));
}

int Com_AddToGrowList(growList_t * list, void *data)
{
	void          **old;

	if(list->currentElements != list->maxElements)
	{
		list->elements[list->currentElements] = data;
		return list->currentElements++;
	}

	// grow, reallocate and move
	old = list->elements;

	if(list->maxElements < 0)
	{
		Com_Error(ERR_FATAL, "Com_AddToGrowList: maxElements = %i", list->maxElements);
	}

	if(list->maxElements == 0)
	{
		// initialize the list to hold 100 elements
		Com_InitGrowList(list, 100);
		return Com_AddToGrowList(list, data);
	}

	list->maxElements *= 2;

//  Com_DPrintf("Resizing growlist to %i maxElements\n", list->maxElements);

	list->elements = (void **)Com_Allocate(list->maxElements * sizeof(void *));

	if(!list->elements)
	{
		Com_Error(ERR_DROP, "Growlist alloc failed");
	}

	memcpy(list->elements, old, list->currentElements * sizeof(void *));

	Com_Dealloc(old);

	return Com_AddToGrowList(list, data);
}

void           *Com_GrowListElement(const growList_t * list, int index)
{
	if(index < 0 || index >= list->currentElements)
	{
		Com_Error(ERR_DROP, "Com_GrowListElement: %i out of range of %i", index, list->currentElements);
	}
	return list->elements[index];
}

int Com_IndexForGrowListElement(const growList_t * list, const void *element)
{
	int             i;

	for(i = 0; i < list->currentElements; i++)
	{
		if(list->elements[i] == element)
		{
			return i;
		}
	}
	return -1;
}

//============================================================================

float Com_Clamp(float min, float max, float value)
{
	if(value < min)
	{
		return min;
	}
	if(value > max)
	{
		return max;
	}
	return value;
}

/*
============
Com_SkipPath
============
*/
char           *Com_SkipPath(char *pathname)
{
	char           *last;

	last = pathname;
	while(*pathname)
	{
		if(*pathname == '/')
			last = pathname + 1;
		pathname++;
	}
	return last;
}

/*
============
Com_GetExtension
============
*/
const char     *Com_GetExtension(const char *name)
{
	int             length, i;

	length = strlen(name) - 1;
	i = length;

	while(name[i] != '.')
	{
		i--;
		if(name[i] == '/' || i == 0)
			return "";			// no extension
	}

	return &name[i + 1];
}


/*
============
Com_StripExtension
============
*/
void Com_StripExtension(const char *src, char *dest, int destsize)
{
	int             length;

	Q_strncpyz(dest, src, destsize);

	length = strlen(dest) - 1;

	while(length > 0 && dest[length] != '.')
	{
		length--;

		if(dest[length] == '/')
			return;				// no extension
	}

	if(length)
	{
		dest[length] = 0;
	}
}


/*
==================
Com_DefaultExtension
==================
*/
void Com_DefaultExtension(char *path, int maxSize, const char *extension)
{
	char            oldPath[MAX_QPATH];
	char           *src;


	// if path doesn't have a .EXT, append extension
	// (extension should include the .)
	src = path + strlen(path) - 1;

	while(*src != '/' && src != path)
	{
		if(*src == '.')
		{
			return;				// it has an extension
		}
		src--;
	}

	Q_strncpyz(oldPath, path, sizeof(oldPath));
	Com_sprintf(path, maxSize, "%s%s", oldPath, extension);
}

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/
/*
// can't just use function pointers, or dll linkage can
// mess up when qcommon is included in multiple places
static short	(*_BigShort) (short l);
static short	(*_LittleShort) (short l);
static int		(*_BigLong) (int l);
static int		(*_LittleLong) (int l);
static qint64	(*_BigLong64) (qint64 l);
static qint64	(*_LittleLong64) (qint64 l);
static float	(*_BigFloat) (const float *l);
static float	(*_LittleFloat) (const float *l);

short	BigShort(short l){return _BigShort(l);}
short	LittleShort(short l) {return _LittleShort(l);}
int		BigLong (int l) {return _BigLong(l);}
int		LittleLong (int l) {return _LittleLong(l);}
qint64 	BigLong64 (qint64 l) {return _BigLong64(l);}
qint64 	LittleLong64 (qint64 l) {return _LittleLong64(l);}
float	BigFloat (const float *l) {return _BigFloat(l);}
float	LittleFloat (const float *l) {return _LittleFloat(l);}
*/

short ShortSwap(short l)
{
	byte            b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

short ShortNoSwap(short l)
{
	return l;
}

int LongSwap(int l)
{
	byte            b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

int LongNoSwap(int l)
{
	return l;
}

qint64 Long64Swap(qint64 ll)
{
	qint64          result;

	result.b0 = ll.b7;
	result.b1 = ll.b6;
	result.b2 = ll.b5;
	result.b3 = ll.b4;
	result.b4 = ll.b3;
	result.b5 = ll.b2;
	result.b6 = ll.b1;
	result.b7 = ll.b0;

	return result;
}

qint64 Long64NoSwap(qint64 ll)
{
	return ll;
}

typedef union
{
	float           f;
	unsigned int    i;
} _FloatByteUnion;

float FloatSwap(const float *f)
{
	_FloatByteUnion out;

	out.f = *f;
	out.i = LongSwap(out.i);

	return out.f;
}

float FloatNoSwap(const float *f)
{
	return *f;
}

/*
============================================================================

PARSING

============================================================================
*/

// multiple character punctuation tokens
const char     *punctuation[] = {
	"+=", "-=", "*=", "/=", "&=", "|=", "++", "--",
	"&&", "||", "<=", ">=", "==", "!=",
	NULL
};

static char     com_token[MAX_TOKEN_CHARS];
static char     com_parsename[MAX_TOKEN_CHARS];
static int      com_lines;

void Com_BeginParseSession(const char *name)
{
	com_lines = 0;
	Q_strncpyz(com_parsename, name, sizeof(com_parsename));
}

int Com_GetCurrentParseLine(void)
{
	return com_lines;
}

char           *Com_Parse(char **data_p)
{
	return Com_ParseExt(data_p, qtrue);
}

void Com_ParseError(char *format, ...)
{
	va_list         argptr;
	static char     string[4096];

	va_start(argptr, format);
	vsprintf(string, format, argptr);
	va_end(argptr);

	Com_Printf(S_COLOR_RED "ERROR: '%s', line %d: '%s'\n", com_parsename, com_lines, string);
}

void Com_ParseWarning(char *format, ...)
{
	va_list         argptr;
	static char     string[4096];

	va_start(argptr, format);
	vsprintf(string, format, argptr);
	va_end(argptr);

	Com_Printf(S_COLOR_YELLOW "WARNING: '%s', line %d: '%s'\n", com_parsename, com_lines, string);
}


/*
==============
Com_Parse

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/
static char    *SkipWhitespace(char *data, qboolean * hasNewLines)
{
	int             c;

	while((c = *data) <= ' ')
	{
		if(!c)
		{
			return NULL;
		}
		if(c == '\n')
		{
			com_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

int Com_Compress(char *data_p)
{
	char           *in, *out;
	int             c;
	qboolean        newline = qfalse, whitespace = qfalse;

	in = out = data_p;
	if(in)
	{
		while((c = *in) != 0)
		{
			// skip double slash comments
			if(c == '/' && in[1] == '/')
			{
				while(*in && *in != '\n')
				{
					in++;
				}
				// skip /* */ comments
			}
			else if(c == '/' && in[1] == '*')
			{
				while(*in && (*in != '*' || in[1] != '/'))
					in++;
				if(*in)
					in += 2;
				// record when we hit a newline
			}
			else if(c == '\n' || c == '\r')
			{
				newline = qtrue;
				in++;
				// record when we hit whitespace
			}
			else if(c == ' ' || c == '\t')
			{
				whitespace = qtrue;
				in++;
				// an actual token
			}
			else
			{
				// if we have a pending newline, emit it (and it counts as whitespace)
				if(newline)
				{
					*out++ = '\n';
					newline = qfalse;
					whitespace = qfalse;
				}
				if(whitespace)
				{
					*out++ = ' ';
					whitespace = qfalse;
				}

				// copy quoted strings unmolested
				if(c == '"')
				{
					*out++ = c;
					in++;
					while(1)
					{
						c = *in;
						if(c && c != '"')
						{
							*out++ = c;
							in++;
						}
						else
						{
							break;
						}
					}
					if(c == '"')
					{
						*out++ = c;
						in++;
					}
				}
				else
				{
					*out = c;
					out++;
					in++;
				}
			}
		}
	}
	*out = 0;
	return out - data_p;
}

// *INDENT-OFF*
char           *Com_ParseExt(char **data_p, qboolean allowLineBreaks)
{
	int             c = 0, len;
	qboolean        hasNewLines = qfalse;
	char           *data;
	const char    **punc;

	if(!data_p)
	{
		Com_Error(ERR_FATAL, "Com_ParseExt: NULL data_p");
	}

	data = *data_p;
	len = 0;
	com_token[0] = 0;

	// make sure incoming data is valid
	if(!data)
	{
		*data_p = NULL;
		return com_token;
	}

	// skip whitespace
	while(1)
	{
		data = SkipWhitespace(data, &hasNewLines);
		if(!data)
		{
			*data_p = NULL;
			return com_token;
		}
		if(hasNewLines && !allowLineBreaks)
		{
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if(c == '/' && data[1] == '/')
		{
			data += 2;
			while(*data && *data != '\n')
			{
				data++;
			}
		}
		// skip /* */ comments
		else if(c == '/' && data[1] == '*')
		{
			data += 2;
			while(*data && (*data != '*' || data[1] != '/'))
			{
				data++;
			}
			if(*data)
			{
				data += 2;
			}
		}
		else
		{
			// a real token to parse
			break;
		}
	}

	// handle quoted strings
	if(c == '\"')
	{
		data++;
		while(1)
		{
			c = *data++;

			if((c == '\\') && (*data == '\"'))
			{
				// allow quoted strings to use \" to indicate the " character
				data++;
			}
			else if(c == '\"' || !c)
			{
				com_token[len] = 0;
				*data_p = (char *)data;
				return com_token;
			}
			else if(*data == '\n')
			{
				com_lines++;
			}

			if(len < MAX_TOKEN_CHARS - 1)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// check for a number
	// is this parsing of negative numbers going to cause expression problems
	if(	(c >= '0' && c <= '9') ||
		(c == '-' && data[1] >= '0' && data[1] <= '9') ||
		(c == '.' && data[1] >= '0' && data[1] <= '9') ||
		(c == '-' && data[1] == '.' && data[2] >= '0' && data[2] <= '9'))
	{
		do
		{
			if(len < MAX_TOKEN_CHARS - 1)
			{
				com_token[len] = c;
				len++;
			}
			data++;

			c = *data;
		} while((c >= '0' && c <= '9') || c == '.');

		// parse the exponent
		if(c == 'e' || c == 'E')
		{
			if(len < MAX_TOKEN_CHARS - 1)
			{
				com_token[len] = c;
				len++;
			}
			data++;
			c = *data;

			if(c == '-' || c == '+')
			{
				if(len < MAX_TOKEN_CHARS - 1)
				{
					com_token[len] = c;
					len++;
				}
				data++;
				c = *data;
			}

			do
			{
				if(len < MAX_TOKEN_CHARS - 1)
				{
					com_token[len] = c;
					len++;
				}
				data++;

				c = *data;
			} while(c >= '0' && c <= '9');
		}

		if(len == MAX_TOKEN_CHARS)
		{
			len = 0;
		}
		com_token[len] = 0;

		*data_p = (char *)data;
		return com_token;
	}

	// check for a regular word
	// we still allow forward and back slashes in name tokens for pathnames
	// and also colons for drive letters
	if(	(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c == '_') ||
		(c == '/') ||
		(c == '\\') ||
		(c == '$') || (c == '*')) // Tr3B - for bad shader strings
	{
		do
		{
			if(len < MAX_TOKEN_CHARS - 1)
			{
				com_token[len] = c;
				len++;
			}
			data++;

			c = *data;
		}
		while
			((c >= 'a' && c <= 'z') ||
			 (c >= 'A' && c <= 'Z') ||
			 (c == '_') ||
			 (c == '-') ||
			 (c >= '0' && c <= '9') ||
			 (c == '/') ||
			 (c == '\\') ||
			 (c == ':') ||
			 (c == '.') ||
			 (c == '$') ||
			 (c == '*') ||
			 (c == '@'));

		if(len == MAX_TOKEN_CHARS)
		{
			len = 0;
		}
		com_token[len] = 0;

		*data_p = (char *)data;
		return com_token;
	}

	// check for multi-character punctuation token
	for(punc = punctuation; *punc; punc++)
	{
		int             l;
		int             j;

		l = strlen(*punc);
		for(j = 0; j < l; j++)
		{
			if(data[j] != (*punc)[j])
			{
				break;
			}
		}
		if(j == l)
		{
			// a valid multi-character punctuation
			memcpy(com_token, *punc, l);
			com_token[l] = 0;
			data += l;
			*data_p = (char *)data;
			return com_token;
		}
	}

	// single character punctuation
	com_token[0] = *data;
	com_token[1] = 0;
	data++;
	*data_p = (char *)data;

	return com_token;
}
// *INDENT-ON*


/*
==================
Com_MatchToken
==================
*/
void Com_MatchToken(char **buf_p, char *match)
{
	char           *token;

	token = Com_Parse(buf_p);
	if(strcmp(token, match))
	{
		Com_Error(ERR_DROP, "MatchToken: '%s' != '%s'", token, match);
	}
}


/*
=================
Com_SkipBracedSection

The next token should be an open brace.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
void Com_SkipBracedSection(char **program)
{
	char           *token;
	int             depth;

	depth = 0;
	do
	{
		token = Com_ParseExt(program, qtrue);
		if(token[1] == 0)
		{
			if(token[0] == '{')
			{
				depth++;
			}
			else if(token[0] == '}')
			{
				depth--;
			}
		}
	} while(depth && *program);
}

/*
=================
Com_SkipRestOfLine
=================
*/
void Com_SkipRestOfLine(char **data)
{
	char           *p;
	int             c;

	p = *data;
	while((c = *p++) != 0)
	{
		if(c == '\n')
		{
			com_lines++;
			break;
		}
	}

	*data = p;
}


void Com_Parse1DMatrix(char **buf_p, int x, float *m)
{
	char           *token;
	int             i;

	Com_MatchToken(buf_p, "(");

	for(i = 0; i < x; i++)
	{
		token = Com_Parse(buf_p);
		m[i] = atof(token);
	}

	Com_MatchToken(buf_p, ")");
}

void Com_Parse2DMatrix(char **buf_p, int y, int x, float *m)
{
	int             i;

	Com_MatchToken(buf_p, "(");

	for(i = 0; i < y; i++)
	{
		Com_Parse1DMatrix(buf_p, x, m + i * x);
	}

	Com_MatchToken(buf_p, ")");
}

void Com_Parse3DMatrix(char **buf_p, int z, int y, int x, float *m)
{
	int             i;

	Com_MatchToken(buf_p, "(");

	for(i = 0; i < z; i++)
	{
		Com_Parse2DMatrix(buf_p, y, x, m + i * x * y);
	}

	Com_MatchToken(buf_p, ")");
}

/*
=============
ColorIndex
 
Returns the index in g_color_table[] corresponding to the colour code.
=============
*/
int ColorIndex(char ccode)
{
	if(ccode >= '0' && ccode <= '9')
		return (ccode - '0');
	else if(ccode >= 'a' && ccode <= 'z')
		return (2 * (ccode - 'a') + 10);
	else if(ccode >= 'A' && ccode <= 'Z')
		return (2 * (ccode - 'A') + 11);
	else
		return 7;
}

/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

int Q_isprint(int c)
{
	if(c >= 0x20 && c <= 0x7E)
		return (1);
	return (0);
}

int Q_islower(int c)
{
	if(c >= 'a' && c <= 'z')
		return (1);
	return (0);
}

int Q_isupper(int c)
{
	if(c >= 'A' && c <= 'Z')
		return (1);
	return (0);
}

int Q_isalpha(int c)
{
	if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return (1);
	return (0);
}

char           *Q_strrchr(const char *string, int c)
{
	char            cc = c;
	char           *s;
	char           *sp = (char *)0;

	s = (char *)string;

	while(*s)
	{
		if(*s == cc)
			sp = s;
		s++;
	}
	if(cc == 0)
		sp = s;

	return sp;
}

/*
=============
Q_strncpyz
 
Safe strncpy that ensures a trailing zero
=============
*/
void Q_strncpyz(char *dest, const char *src, int destsize)
{
	if(!dest)
	{
		Com_Error(ERR_FATAL, "Q_strncpyz: NULL dest");
	}
	if(!src)
	{
		Com_Error(ERR_FATAL, "Q_strncpyz: NULL src");
	}
	if(destsize < 1)
	{
		Com_Error(ERR_FATAL, "Q_strncpyz: destsize < 1");
	}

	strncpy(dest, src, destsize - 1);
	dest[destsize - 1] = 0;
}

int Q_stricmpn(const char *s1, const char *s2, int n)
{
	int             c1, c2;

	if(s1 == NULL)
	{
		if(s2 == NULL)
			return 0;
		else
			return -1;
	}
	else if(s2 == NULL)
		return 1;



	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if(!n--)
		{
			return 0;			// strings are equal until end point
		}

		if(c1 != c2)
		{
			if(c1 >= 'a' && c1 <= 'z')
			{
				c1 -= ('a' - 'A');
			}
			if(c2 >= 'a' && c2 <= 'z')
			{
				c2 -= ('a' - 'A');
			}
			if(c1 != c2)
			{
				return c1 < c2 ? -1 : 1;
			}
		}
	} while(c1);

	return 0;					// strings are equal
}

int Q_strncmp(const char *s1, const char *s2, int n)
{
	int             c1, c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if(!n--)
		{
			return 0;			// strings are equal until end point
		}

		if(c1 != c2)
		{
			return c1 < c2 ? -1 : 1;
		}
	} while(c1);

	return 0;					// strings are equal
}

int Q_stricmp(const char *s1, const char *s2)
{
	return (s1 && s2) ? Q_stricmpn(s1, s2, 99999) : -1;
}


char           *Q_strlwr(char *s1)
{
	char           *s;

	s = s1;
	while(*s)
	{
		*s = tolower(*s);
		s++;
	}
	return s1;
}

char           *Q_strupr(char *s1)
{
	char           *s;

	s = s1;
	while(*s)
	{
		*s = toupper(*s);
		s++;
	}
	return s1;
}


// never goes past bounds or leaves without a terminating 0
void Q_strcat(char *dest, int destsize, const char *src)
{
	int             l1;

	l1 = strlen(dest);
	if(l1 >= destsize)
	{
		Com_Error(ERR_FATAL, "Q_strcat: already overflowed");
	}
	Q_strncpyz(dest + l1, src, destsize - l1);
}


/*
=============
Q_stristr
 
Find the first occurrence of find in s.
=============
*/
char           *Q_stristr(const char *s, const char *find)
{
	char            c, sc;
	size_t          len;

	if((c = *find++) != 0)
	{
		if(c >= 'a' && c <= 'z')
		{
			c -= ('a' - 'A');
		}
		len = strlen(find);
		do
		{
			do
			{
				if((sc = *s++) == 0)
				{
					return NULL;
				}

				if(sc >= 'a' && sc <= 'z')
				{
					sc -= ('a' - 'A');
				}
			} while(sc != c);
		} while(Q_stricmpn(s, find, len) != 0);
		s--;
	}
	return (char *)s;
}


/*
=============
Q_strreplace
 
replaces content of find by replace in dest
=============
*/
qboolean Q_strreplace(char *dest, int destsize, const char *find, const char *replace)
{
	int             lstart, lfind, lreplace, lend;
	char           *s;
	char            backup[32000];	// big, but small enough to fit in PPC stack

	lend = strlen(dest);
	if(lend >= destsize)
	{
		Com_Error(ERR_FATAL, "Q_strreplace: already overflowed");
	}

	s = strstr(dest, find);
	if(!s)
	{
		return qfalse;
	}
	else
	{
		Q_strncpyz(backup, dest, lend + 1);
		lstart = s - dest;
		lfind = strlen(find);
		lreplace = strlen(replace);

		strncpy(s, replace, destsize - lstart - 1);
		strncpy(s + lreplace, backup + lstart + lfind, destsize - lstart - lreplace - 1);

		return qtrue;
	}
}


int Q_PrintStrlen(const char *string)
{
	int             len;
	const char     *p;

	if(!string)
	{
		return 0;
	}

	len = 0;
	p = string;
	while(*p)
	{
		if(Q_IsColorString(p))
		{
			p += 2;
			continue;
		}
		p++;
		len++;
	}

	return len;
}


char           *Q_CleanStr(char *string)
{
	char           *d;
	char           *s;
	int             c;

	s = string;
	d = string;
	while((c = *s) != 0)
	{
		if(Q_IsColorString(s))
		{
			s++;
		}
		else if(c >= 0x20 && c <= 0x7E)
		{
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}


int Q_CountChar(const char *string, char tocount)
{
	int				count;

	for(count = 0; *string; string++)
	{
		if(*string == tocount)
			count++;
	}

	return count;
}


void QDECL Com_sprintf(char *dest, int size, const char *fmt, ...)
{
	int             len;
	va_list         argptr;
	char            bigbuffer[32000];	// big, but small enough to fit in PPC stack

	va_start(argptr, fmt);
	len = vsprintf(bigbuffer, fmt, argptr);
	va_end(argptr);
	if(len >= sizeof(bigbuffer))
	{
		Com_Error(ERR_FATAL, "Com_sprintf: overflowed bigbuffer");
	}
	if(len >= size)
	{
		Com_Printf("Com_sprintf: overflow of %i in %i\n", len, size);
#if defined(_MSC_VER) && defined(_DEBUG)
		__asm
		{
			int             3;
		}
#endif
	}
	Q_strncpyz(dest, bigbuffer, size);
}


/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char           *QDECL va(char *format, ...)
{
	va_list         argptr;
	static char     string[2][32000];	// in case va is called by nested functions
	static int      index = 0;
	char           *buf;

	buf = string[index & 1];
	index++;

	va_start(argptr, format);
	vsprintf(buf, format, argptr);
	va_end(argptr);

	return buf;
}

/*
============
Com_TruncateLongString

Assumes buffer is atleast TRUNCATE_LENGTH big
============
*/
void Com_TruncateLongString(char *buffer, const char *s)
{
	int             length = strlen(s);

	if(length <= TRUNCATE_LENGTH)
		Q_strncpyz(buffer, s, TRUNCATE_LENGTH);
	else
	{
		Q_strncpyz(buffer, s, (TRUNCATE_LENGTH / 2) - 3);
		Q_strcat(buffer, TRUNCATE_LENGTH, " ... ");
		Q_strcat(buffer, TRUNCATE_LENGTH, s + length - (TRUNCATE_LENGTH / 2) + 3);
	}
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
char           *Info_ValueForKey(const char *s, const char *key)
{
	char            pkey[BIG_INFO_KEY];
	static char     value[2][BIG_INFO_VALUE];	// use two buffers so compares

	// work without stomping on each other
	static int      valueindex = 0;
	char           *o;

	if(!s || !key)
	{
		return "";
	}

	if(strlen(s) >= BIG_INFO_STRING)
	{
		Com_Error(ERR_DROP, "Info_ValueForKey: oversize infostring");
	}

	valueindex ^= 1;
	if(*s == '\\')
		s++;
	while(1)
	{
		o = pkey;
		while(*s != '\\')
		{
			if(!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while(*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if(!Q_stricmp(key, pkey))
			return value[valueindex];

		if(!*s)
			break;
		s++;
	}

	return "";
}


/*
===================
Info_NextPair

Used to itterate through all the key/value pairs in an info string
===================
*/
void Info_NextPair(const char **head, char *key, char *value)
{
	char           *o;
	const char     *s;

	s = *head;

	if(*s == '\\')
	{
		s++;
	}
	key[0] = 0;
	value[0] = 0;

	o = key;
	while(*s != '\\')
	{
		if(!*s)
		{
			*o = 0;
			*head = s;
			return;
		}
		*o++ = *s++;
	}
	*o = 0;
	s++;

	o = value;
	while(*s != '\\' && *s)
	{
		*o++ = *s++;
	}
	*o = 0;

	*head = s;
}


/*
===================
Info_RemoveKey
===================
*/
void Info_RemoveKey(char *s, const char *key)
{
	char           *start;
	char            pkey[MAX_INFO_KEY];
	char            value[MAX_INFO_VALUE];
	char           *o;

	if(strlen(s) >= MAX_INFO_STRING)
	{
		Com_Error(ERR_DROP, "Info_RemoveKey: oversize infostring");
	}

	if(strchr(key, '\\'))
	{
		return;
	}

	while(1)
	{
		start = s;
		if(*s == '\\')
			s++;
		o = pkey;
		while(*s != '\\')
		{
			if(!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while(*s != '\\' && *s)
		{
			if(!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if(!strcmp(key, pkey))
		{
			strcpy(start, s);	// remove this part
			return;
		}

		if(!*s)
			return;
	}

}

/*
===================
Info_RemoveKey_Big
===================
*/
void Info_RemoveKey_Big(char *s, const char *key)
{
	char           *start;
	char            pkey[BIG_INFO_KEY];
	char            value[BIG_INFO_VALUE];
	char           *o;

	if(strlen(s) >= BIG_INFO_STRING)
	{
		Com_Error(ERR_DROP, "Info_RemoveKey_Big: oversize infostring");
	}

	if(strchr(key, '\\'))
	{
		return;
	}

	while(1)
	{
		start = s;
		if(*s == '\\')
			s++;
		o = pkey;
		while(*s != '\\')
		{
			if(!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while(*s != '\\' && *s)
		{
			if(!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if(!strcmp(key, pkey))
		{
			strcpy(start, s);	// remove this part
			return;
		}

		if(!*s)
			return;
	}

}




/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate(const char *s)
{
	if(strchr(s, '\"'))
	{
		return qfalse;
	}
	if(strchr(s, ';'))
	{
		return qfalse;
	}
	return qtrue;
}

/*
==================
Info_SetValueForKey

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey(char *s, const char *key, const char *value)
{
	char            newi[MAX_INFO_STRING];
	const char     *blacklist = "\\;\"";

	if(strlen(s) >= MAX_INFO_STRING)
	{
		Com_Error(ERR_DROP, "Info_SetValueForKey: oversize infostring");
	}

	for(; *blacklist; ++blacklist)
	{
		if(strchr(key, *blacklist) || strchr(value, *blacklist))
		{
			Com_Printf(S_COLOR_YELLOW "Can't use keys or values with a '%c': %s = %s\n", *blacklist, key, value);
			return;
		}
	}

	Info_RemoveKey(s, key);
	if(!value || !strlen(value))
		return;

	Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

	if(strlen(newi) + strlen(s) >= MAX_INFO_STRING)
	{
		Com_Printf("Info string length exceeded\n");
		return;
	}

	strcat(newi, s);
	strcpy(s, newi);
}

/*
==================
Info_SetValueForKey_Big

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey_Big(char *s, const char *key, const char *value)
{
	char            newi[BIG_INFO_STRING];
	const char     *blacklist = "\\;\"";

	if(strlen(s) >= BIG_INFO_STRING)
	{
		Com_Error(ERR_DROP, "Info_SetValueForKey: oversize infostring");
	}

	for(; *blacklist; ++blacklist)
	{
		if(strchr(key, *blacklist) || strchr(value, *blacklist))
		{
			Com_Printf(S_COLOR_YELLOW "Can't use keys or values with a '%c': %s = %s\n", *blacklist, key, value);
			return;
		}
	}

	Info_RemoveKey_Big(s, key);
	if(!value || !strlen(value))
		return;

	Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

	if(strlen(newi) + strlen(s) >= BIG_INFO_STRING)
	{
		Com_Printf("BIG Info string length exceeded\n");
		return;
	}

	strcat(s, newi);
}




//====================================================================

/*
==================
Com_CharIsOneOfCharset
==================
*/
static qboolean Com_CharIsOneOfCharset(char c, char *set)
{
	int             i;

	for(i = 0; i < strlen(set); i++)
	{
		if(set[i] == c)
			return qtrue;
	}

	return qfalse;
}

/*
==================
Com_SkipCharset
==================
*/
char           *Com_SkipCharset(char *s, char *sep)
{
	char           *p = s;

	while(p)
	{
		if(Com_CharIsOneOfCharset(*p, sep))
			p++;
		else
			break;
	}

	return p;
}

/*
==================
Com_SkipTokens
==================
*/
char           *Com_SkipTokens(char *s, int numTokens, char *sep)
{
	int             sepCount = 0;
	char           *p = s;

	while(sepCount < numTokens)
	{
		if(Com_CharIsOneOfCharset(*p++, sep))
		{
			sepCount++;
			while(Com_CharIsOneOfCharset(*p, sep))
				p++;
		}
		else if(*p == '\0')
			break;
	}

	if(sepCount == numTokens)
		return p;
	else
		return s;
}

qboolean Com_CheckColorCodes(const char *s)
{
	while(s[0])
	{
		if(Q_IsColorString(s))
		{
			if(!s[1])
				return qfalse;
		}
		s++;
	}

	return qtrue;
}

//====================================================================
