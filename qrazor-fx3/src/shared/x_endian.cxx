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


static bool	bigendien;

// can't just use function pointers, or dll linkage can
// mess up when qcommon is included in multiple places
static short	(*_BigShort)(short l);
static short	(*_LittleShort)(short l);
static int	(*_BigLong)(int l);
static int	(*_LittleLong)(int l);
static float	(*_BigFloat)(float l);
static float	(*_LittleFloat)(float l);


short	BigShort(short l)	{return _BigShort(l);}
short	LittleShort(short l) 	{return _LittleShort(l);}
int	BigLong(int l) 		{return _BigLong(l);}
int	LittleLong(int l)	{return _LittleLong(l);}
float	BigFloat(float l)	{return _BigFloat(l);}
float	LittleFloat(float l)	{return _LittleFloat(l);}


static short   ShortSwap(short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

static short	ShortNoSwap(short l)
{
	return l;
}

static int    LongSwap(int l)
{
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

static int	LongNoSwap(int l)
{
	return l;
}

static float 	FloatSwap(float f)
{
	union
	{
		float	f;
		byte	b[4];
	} dat1, dat2;
	
	
	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

static float 	FloatNoSwap(float f)
{
	return f;
}

void 	Swap_Init()
{	
	byte	swaptest[2] = {1,0};

	Com_Printf("------- Swap_Init -------\n");
		
	// set the byte swapping variables in a portable manner	
	if(*(short *)swaptest == 1)
	{
		bigendien = false;
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	}
	else
	{
		bigendien = true;
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}

}
