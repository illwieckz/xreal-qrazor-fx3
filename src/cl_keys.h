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
#ifndef CL_KEYS_H
#define CL_KEYS_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "shared/x_keycodes.h"


extern char*		keybindings[256];
extern int		key_repeats[256];

extern int	anykeydown;
extern char 	chat_buffer[];
extern int 	chat_bufferlen;
extern bool	chat_team;

void		Key_Init();
void		Key_Event(int key, bool down, int time);
void 		Key_WriteBindings(VFILE *v);
char*		Key_KeynumToString(int keynum);
char*		Key_GetBinding(int keynum);
void 		Key_SetBinding(int keynum, char *binding);
void 		Key_ClearStates();
int 		Key_GetKey();
keydest_t	Key_GetKeyDest();
void		Key_SetKeyDest(keydest_t dest);


#endif // CL_KEYS_H

