/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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
#ifndef IMG_PNG_H
#define IMG_PNG_H

/// includes ===================================================================
// system -------------------------------------------------------------------
#include <string>
// qrazor-fx ----------------------------------------------------------------


void 	IMG_LoadPNG(const std::string &filename, byte **pic, int *width, int *height);
void 	IMG_LoadPNGGrey(const std::string &filename, byte **pic, int *width, int *height);

#endif // IMG_PNG_H



