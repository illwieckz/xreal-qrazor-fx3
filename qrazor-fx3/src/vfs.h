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
#ifndef VFS_H
#define VFS_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "shared/x_shared.h"




void	VFS_Init();
void	VFS_Shutdown();
void	VFS_Restart();
void	VFS_SetGamedir(const std::string &dir);
std::string	VFS_Gamedir();
void	VFS_ExecAutoexec();
void	VFS_CheckForChanges();

std::vector<std::string>	VFS_ListFiles(const std::string &dir, const std::string &extension);

int	VFS_FOpenByMode(const std::string &filename, VFILE *stream, vfs_mode_e mode);
int	VFS_FOpenRead(const std::string &filename, VFILE **stream);
int	VFS_FOpenWrite(const std::string &filename, VFILE **stream);
int	VFS_FOpenAppend(const std::string &filename, VFILE **stream);
void	VFS_FClose(VFILE **stream);


int	VFS_FLoad(const std::string &filename, void **buffer);
void	VFS_FSave(const std::string &filename, void *buffer, int len);
void	VFS_FFree(void *buffer);


const char*	VFS_FullPath(const std::string &filename);

int	VFS_FRead(void *buffer, int len, VFILE *stream);
int	VFS_FWrite(const void *buffer, int len, VFILE *stream);
int	VFS_FPrintf(VFILE *stream, const char *fmt, ...);
int	VFS_FSeek(VFILE *stream, long offset, vfs_seek_e type);
int 	VFS_FTell(VFILE *stream);
int	VFS_FEOF(VFILE *stream);
int 	VFS_FSize(VFILE *stream);
void	VFS_FFlush(VFILE *stream);


void	VFS_Mkdir(const std::string &path);


#endif	// VFS_H

