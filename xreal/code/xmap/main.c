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
#include "../common/cmdlib.h"
#include "../common/inout.h"

/*
============
main
============
*/

void            Bspinfo(int count, char **fileNames);
int             BspMain(int argc, char **argv);
int             VisMain(int argc, char **argv);
int             AASMain(int argc, char **argv);

int main(int argc, char **argv)
{
	Sys_Printf("xmap v1.0s (c) 1999-2006 Id Software, Inc. and XreaL contributors\n");

	if(argc < 2)
	{
		goto showUsage;
	}

	// check for general program options
	if(!strcmp(argv[1], "-info"))
	{
		Bspinfo(argc - 2, argv + 2);
		return 0;
	}
	if(!strcmp(argv[1], "-map2bsp"))
	{
		BspMain(argc - 1, argv + 1);
		return 0;
	}
	if(!strcmp(argv[1], "-vis"))
	{
		VisMain(argc - 1, argv + 1);
		return 0;
	}
#ifdef AAS
	if(!strcmp(argv[1], "-bsp2aas"))
	{
		AASMain(argc - 1, argv + 1);
		return 0;
	}
#endif
	
showUsage:
	Error(	"usage: xmap [-<switch> [-<switch> ...]] <mapname>\n"
		  	"\n"
			"Switches:\n"
			"   info           = print BSP information\n"
			"   map2bsp        = compile MAP to BSP\n"
			#ifdef AAS
			"   bsp2aas        = convert BSP to AAS\n"
			#endif
			"   vis            = compute visibility\n");

	return 0;
}
