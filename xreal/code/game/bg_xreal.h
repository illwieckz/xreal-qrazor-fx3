/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2007 Pat Raynor <raynorpat@sbcglobal.net>

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

// bg_xreal.h -- both games xreal definitions (weapon damage, etc.)

// XreaL Weapons
extern float knockback_z;

// Player Movement Physics
extern float pm_airStopAccelerate;
extern float pm_airControl;
extern float pm_strafeAccelerate;
extern float pm_wishSpeed;

void			PM_Aircontrol(pmove_t *pm, vec3_t wishdir, float wishspeed);