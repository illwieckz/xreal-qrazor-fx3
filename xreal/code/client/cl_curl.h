/*
===========================================================================
Copyright (C) 2006 Tony J. White (tjw@tjw.org)

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

#ifndef __QCURL_H__
#define __QCURL_H__

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"

#include <curl/curl.h>

qboolean		CL_cURL_Init(void);
void			CL_cURL_Shutdown(void);
void			CL_cURL_BeginDownload(const char *localName, const char *remoteURL);
void			CL_cURL_PerformDownload(void);
void			CL_cURL_Cleanup(void);

#endif	// __QCURL_H__

