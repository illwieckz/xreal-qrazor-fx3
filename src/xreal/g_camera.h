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
#ifndef G_CAMERA_H
#define G_CAMERA_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// xreal --------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "g_entity.h"



class g_camera_c : public g_entity_c
{
public:
	g_camera_c();
	
	//virtual void	think();
	//virtual void	touch(g_entity_c *other, cplane_c *plane, csurface_c *surf);
	
	//virtual void	activate();
private:
	//TODO
};



#endif // G_CAMERA_H
