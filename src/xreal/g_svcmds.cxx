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
// xreal --------------------------------------------------------------------
#include "g_local.h"




void	Svcmd_Test_f()
{
	gi.SV_CPrintf(NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

writeip
Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter lists are not saved and restored by default, because I beleive it would cause too much confusion.

filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

typedef struct
{
	unsigned	mask;
	unsigned	compare;
} ipfilter_t;

#define	MAX_IPFILTERS	1024

ipfilter_t	ipfilters[MAX_IPFILTERS];
int			numipfilters;

static bool	StringToFilter(const char *s, ipfilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			gi.SV_CPrintf(NULL, PRINT_HIGH, "Bad filter address: %s\n", s);
			return false;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return true;
}

bool 	SV_FilterPacket(const char *from)
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;

	i = 0;
	p = (char*)from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for (i=0 ; i<numipfilters ; i++)
		if ( (in & ipfilters[i].mask) == ipfilters[i].compare)
			return filterban->getInteger();

	return !filterban->getInteger();
}


void 	SVCmd_AddIP_f()
{
	int		i;
	
	if (gi.Cmd_Argc() < 3) {
		gi.SV_CPrintf(NULL, PRINT_HIGH, "Usage:  addip <ip-mask>\n");
		return;
	}

	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numipfilters)
	{
		if (numipfilters == MAX_IPFILTERS)
		{
			gi.SV_CPrintf (NULL, PRINT_HIGH, "IP filter list is full\n");
			return;
		}
		numipfilters++;
	}
	
	if (!StringToFilter (gi.Cmd_Argv(2), &ipfilters[i]))
		ipfilters[i].compare = 0xffffffff;
}

void 	SVCmd_RemoveIP_f()
{
	ipfilter_t	f;
	int			i, j;

	if (gi.Cmd_Argc() < 3) {
		gi.SV_CPrintf(NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n");
		return;
	}

	if (!StringToFilter (gi.Cmd_Argv(2), &f))
		return;

	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].mask == f.mask
		&& ipfilters[i].compare == f.compare)
		{
			for (j=i+1 ; j<numipfilters ; j++)
				ipfilters[j-1] = ipfilters[j];
			numipfilters--;
			gi.SV_CPrintf (NULL, PRINT_HIGH, "Removed.\n");
			return;
		}
	gi.SV_CPrintf (NULL, PRINT_HIGH, "Didn't find %s.\n", gi.Cmd_Argv(2));
}

void 	SVCmd_ListIP_f()
{
	int		i;
	byte	b[4];

	gi.SV_CPrintf (NULL, PRINT_HIGH, "Filter list:\n");
	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		gi.SV_CPrintf (NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
	}
}

void 	SVCmd_WriteIP_f()
{
	FILE	*f;
	char	name[MAX_OSPATH];
	byte	b[4];
	int		i;
	cvar_t	*vfs_game;

	vfs_game = gi.Cvar_Get("vfs_game", "", 0);

	if(X_strequal(vfs_game->getString(), ""))
		sprintf(name, "%s/listip.cfg", GAMEVERSION);
	else
		sprintf(name, "%s/listip.cfg", vfs_game->getString());

	gi.SV_CPrintf(NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen (name, "wb");
	if (!f)
	{
		gi.SV_CPrintf (NULL, PRINT_HIGH, "Couldn't open %s\n", name);
		return;
	}
	
	fprintf(f, "set filterban %d\n", filterban->getInteger());

	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		fprintf (f, "sv addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
	}
	
	fclose (f);
}

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void	G_ServerCommand()
{
	const char	*cmd;

	cmd = gi.Cmd_Argv(1);
	
	if (X_stricmp (cmd, "test") == 0)
		Svcmd_Test_f ();
		
	else if (X_stricmp (cmd, "addip") == 0)
		SVCmd_AddIP_f ();
		
	else if (X_stricmp (cmd, "removeip") == 0)
		SVCmd_RemoveIP_f ();
		
	else if (X_stricmp (cmd, "listip") == 0)
		SVCmd_ListIP_f ();
		
	else if (X_stricmp (cmd, "writeip") == 0)
		SVCmd_WriteIP_f ();
		
	else
		gi.SV_CPrintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

