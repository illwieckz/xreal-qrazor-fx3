/// ============================================================================
/*
Copyright (C) 2003 Christophe Devine <devine@cr0.net>
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
#ifndef SHA1_H
#define SHA1_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "x_shared.h"

#ifdef __cplusplus
extern "C" {
#endif


//#define uint8  unsigned char
#define uint32 unsigned long int

struct sha1_context_t
{
	uint32	total[2];
	uint32	state[5];
	byte	buffer[64];
};

void SHA1_Init(sha1_context_t *ctx);
void SHA1_Update(sha1_context_t *ctx, const byte *input, uint32 length);
void SHA1_Finish(sha1_context_t *ctx, byte digest[20]);

#ifdef __cplusplus
}
#endif



#endif // SHA1_H
