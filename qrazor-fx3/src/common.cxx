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
#include <setjmp.h>
#include <zlib.h>

#include <iostream>
#include <queue>
#include <boost/dynamic_bitset.hpp>

// qrazor-fx ----------------------------------------------------------------
#include "common.h"
#include "sys.h"
#include "net_chan.h"
#include "cmd.h"
#include "cvar.h"
#include "vfs.h"
#include "sha1.h"
#include "cm.h"

#include "x_bitmessage.h"


#define	MAX_PRINT_MSG	4096


class sys_event_c
{
public:
	sys_event_c(sys_event_type_e type, int time, int parm1, int parm2, byte *data, uint_t size)
	{
		_type		= type;
		_time		= time;
		_parm1		= parm1;
		_parm2		= parm2;
		_data		= data;
		_data_size	= size;
	}
	
	~sys_event_c()
	{
		if(_data)
			delete _data;
	}

	sys_event_type_e	getType() const		{return _type;}
	int			getTime() const		{return _time;}
	int			getParm1() const	{return _parm1;}
	int			getParm2() const	{return _parm2;}
	byte*			getData() const		{return _data;}
	uint_t			getDataSize() const	{return _data_size;}
	
private:
	sys_event_type_e	_type;
	int			_time;
	int			_parm1;
	int			_parm2;
	byte*			_data;
	uint_t			_data_size;
};


static std::vector<std::string>	com_argv;
static std::string		com_cmdline;

static std::queue<sys_event_c*>	com_event_queue;


jmp_buf abortframe;		// an ERR_DROP occured, exit the entire frame


cvar_t	*com_speeds;
cvar_t	*developer;
cvar_t	*timescale;
cvar_t	*fixedtime;
cvar_t	*logfile_active;	// 1 = buffer log, 2 = flush after each print
cvar_t	*showtrace;
cvar_t	*dedicated;

VFILE*	logfile = NULL;

int		server_state;

// host_speeds times
int		time_before_game;
int		time_after_game;
int		time_before_ref;
int		time_after_ref;

/*
============================================================================

CLIENT / SERVER interactions

============================================================================
*/

static char*		rd_buffer = NULL;
static int		rd_buffer_size = 0;
static netadr_t		rd_adr;

static void		(*rd_flush)(char *buffer, const netadr_t &adr);



void 	Com_BeginRedirect(char *buffer, int buffer_size, void (*flush)(char*, const netadr_t&), const netadr_t &adr)
{
	if(!buffer || !buffer_size || !flush)
		return;
		
	rd_buffer = buffer;
	rd_buffer_size = buffer_size;
	rd_adr = adr;
	
	rd_flush = flush;

	*rd_buffer = 0;
}

void 	Com_EndRedirect()
{
	rd_flush(rd_buffer, rd_adr);

	rd_buffer = NULL;
	rd_buffer_size = 0;
	memset(&rd_adr, 0, sizeof(rd_adr));
	
	rd_flush = NULL;
}



/*
=============
Com_Printf

Both client and server can use this, and it will output
to the apropriate place.
=============
*/
void 	Com_Printf(const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAX_PRINT_MSG];

	va_start(argptr,fmt);
	vsprintf(msg,fmt,argptr);
	va_end(argptr);

	if(rd_buffer)
	{
		if((int)(strlen(msg) + strlen(rd_buffer)) > (rd_buffer_size - 1))
		{
			rd_flush(rd_buffer, rd_adr);
			*rd_buffer = 0;
		}
		strcat(rd_buffer, msg);
		return;
	}

	Con_Print(msg);
		
	// also echo to debugging console
	Sys_ConsoleOutput(msg);

	// logfile
	if(logfile_active && logfile_active->getInteger())
	{
		if(!logfile)
		{
			if(logfile_active->getInteger() > 2)
				VFS_FOpenAppend("console.log", &logfile);
			else
			{
				VFS_FOpenWrite("console.log", &logfile);
			}
		}
		
		if(logfile)
			VFS_FPrintf(logfile, "%s", msg);
			
		if(logfile_active->getInteger() > 1)
			VFS_FFlush(logfile);		// force it to save every time
	}
}


/*
================
Com_DPrintf

A Com_Printf that only shows up if the "developer" cvar is set
================
*/
void 	Com_DPrintf(const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAX_PRINT_MSG];
		
	if(!developer || !developer->getValue())
		return;			// don't confuse non-developers with techie stuff...

	va_start(argptr,fmt);
	vsprintf(msg,fmt,argptr);
	va_end(argptr);
	
	Com_Printf("%s", msg);
}


/*
=============
Com_Error

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void 	Com_Error(err_type_e type, const char *fmt, ...)
{
	va_list		argptr;
	static char	msg[4096];
	static	bool	recursive;

	if(recursive)
		Sys_Error("recursive error after: %s", msg);
	recursive = true;

	va_start(argptr,fmt);
	vsprintf(msg,fmt,argptr);
	va_end(argptr);
	
	switch(type)
	{
		case ERR_FATAL:
			SV_Shutdown(va("Server fatal crashed: %s\n", msg), false);
			CL_Shutdown();
			Map_Shutdown();
			break;
		
		case ERR_DROP:
			Com_Printf("********************\nERROR: %s\n********************\n", msg);
			SV_Shutdown(va("Server crashed: %s\n", msg), false);
			CL_Drop();
			recursive = false;
			longjmp(abortframe, -1);	
			break;
		
		case ERR_DISCONNECT:
			CL_Drop();
			recursive = false;
			longjmp(abortframe, -1);
			break;
			
		case ERR_WARNING:
			Com_Printf(va("WARNING: %s\n", msg));
			return;
	}
	

	if(logfile)
		VFS_FClose(&logfile);

	Sys_Error("%s", msg);
}


void	Com_PushEvent(sys_event_type_e type, int time, int parm1, int parm2, byte *data, uint_t size)
{
	//Com_Printf("Com_PushEvent: %i %3i %i %i %p %i\n", type, time, parm1, parm2, data, size);

	com_event_queue.push(new sys_event_c(type, time, parm1, parm2, data, size));
}

static void	Com_EventLoop()
{
#ifndef BSPCOMPILER_ONLY
	netadr_t	adr;
	bitmessage_c	msg(tobits(MAX_PACKETLEN));	// Tr3B - don't use MAX_MSGLEN, will cause buffer overflow -> exception -> kills server badly
							// don't ask my for the reason
#endif

	while(com_event_queue.size())
	{
		const sys_event_c* event = com_event_queue.front();
		
		switch(event->getType())
		{
			case SE_CONSOLE:
				Cbuf_AddText((const char*)event->getData());
				break;
				
			case SE_KEY:
				Key_Event(event->getParm1(), event->getParm2(), event->getTime());
				break;
				
			case SE_MOUSE:
				CL_MouseEvent(event->getParm1(), event->getParm2());
				break;
				
			case SE_PACKET:
			{
			#ifndef BSPCOMPILER_ONLY
				memcpy(&adr, event->getData(), sizeof(adr));
				
				msg.beginWriting();
				msg.writeBytes(event->getData() + sizeof(adr), event->getDataSize() - sizeof(adr));
				msg.beginReading();
			
				SV_PacketEvent(msg, adr);
				CL_PacketEvent(msg, adr);
			#endif
				break;
			}
		}
		
		delete event;
		
		com_event_queue.pop();
	}
}



int 	Com_ServerState()
{
	return server_state;
}

void 	Com_SetServerState(int state)
{
	server_state = state;
}





/*
================
Com_CheckParm

Returns the position (1 to argc-1) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
int 	Com_CheckParm(const std::string &parm)
{
	for(unsigned int i=0; i<com_argv.size(); i++)
	{
		if(X_strequal(parm.c_str(), com_argv[i].c_str()))
			return i;
	}
	
	return -1;
}

int 	Com_Argc()
{
	return com_argv.size();
}

const char*	Com_Argv(int arg)
{
	if(arg < 0 || arg >= (int)com_argv.size() || com_argv[arg].empty())
		return "";
		
	return com_argv[arg].c_str();
}

void 	Com_ClearArgv(int arg)
{
	if(arg < 0 || arg >= (int)com_argv.size() || com_argv[arg].empty())
		return;
		
	com_argv[arg] = "";
}


void 	Com_InitArgv(int argc, char **argv)
{
	Com_Printf("------- Com_InitArgv -------\n");

	/*
	if(argc > MAX_NUM_ARGVS)
		Com_Error(ERR_FATAL, "Com_InitAgv: argc > MAX_NUM_ARGVS");
		
	com_argc = argc;
	
	for(int i=0; i<argc; i++)
	{
		if(!argv[i] || strlen(argv[i]) >= MAX_TOKEN_CHARS )
			com_argv[i] = "";
		else
			com_argv[i] = argv[i];
	}
	*/
	
	for(int i=0; i<argc; i++)
	{
		if(!argv[i] || strlen(argv[i]) >= MAX_TOKEN_CHARS )
			com_argv.push_back("");
		else
			com_argv.push_back(argv[i]);
	}
	
	Com_Printf("com_argc:  %i\n", com_argv.size());
	
	for(uint i=0; i < com_argv.size(); i++)
	{
		Com_Printf("com_argv: [%i][%s]\n", i, com_argv[i].c_str());
		
		com_cmdline += com_argv[i];
		com_cmdline += ' ';
	}

	Com_Printf("com_cmdline: %s\n", com_cmdline.c_str());
}

void 	Com_AddParm(const std::string &parm)
{
	if(parm.empty())
	{
		Com_Error(ERR_DROP, "Com_AddParm: empty parameter");
		return;
	}

	com_argv.push_back(parm);
}



void 	Com_PrintInfo(const char *s)
{
	char	key[512];
	char	value[512];
	char	*o;
	int		l;

	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;
		if (l < 20)
		{
			memset (o, ' ', 20-l);
			key[20] = 0;
		}
		else
			*o = 0;
		Com_Printf ("%s", key);

		if (!*s)
		{
			Com_Printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;
		Com_Printf ("%s\n", value);
	}
}






static byte chktbl[1024] =
{
0x84, 0x47, 0x51, 0xc1, 0x93, 0x22, 0x21, 0x24, 0x2f, 0x66, 0x60, 0x4d, 0xb0, 0x7c, 0xda,
0x88, 0x54, 0x15, 0x2b, 0xc6, 0x6c, 0x89, 0xc5, 0x9d, 0x48, 0xee, 0xe6, 0x8a, 0xb5, 0xf4,
0xcb, 0xfb, 0xf1, 0x0c, 0x2e, 0xa0, 0xd7, 0xc9, 0x1f, 0xd6, 0x06, 0x9a, 0x09, 0x41, 0x54,
0x67, 0x46, 0xc7, 0x74, 0xe3, 0xc8, 0xb6, 0x5d, 0xa6, 0x36, 0xc4, 0xab, 0x2c, 0x7e, 0x85,
0xa8, 0xa4, 0xa6, 0x4d, 0x96, 0x19, 0x19, 0x9a, 0xcc, 0xd8, 0xac, 0x39, 0x5e, 0x3c, 0xf2,
0xf5, 0x5a, 0x72, 0xe5, 0xa9, 0xd1, 0xb3, 0x23, 0x82, 0x6f, 0x29, 0xcb, 0xd1, 0xcc, 0x71,
0xfb, 0xea, 0x92, 0xeb, 0x1c, 0xca, 0x4c, 0x70, 0xfe, 0x4d, 0xc9, 0x67, 0x43, 0x47, 0x94,
0xb9, 0x47, 0xbc, 0x3f, 0x01, 0xab, 0x7b, 0xa6, 0xe2, 0x76, 0xef, 0x5a, 0x7a, 0x29, 0x0b,
0x51, 0x54, 0x67, 0xd8, 0x1c, 0x14, 0x3e, 0x29, 0xec, 0xe9, 0x2d, 0x48, 0x67, 0xff, 0xed,
0x54, 0x4f, 0x48, 0xc0, 0xaa, 0x61, 0xf7, 0x78, 0x12, 0x03, 0x7a, 0x9e, 0x8b, 0xcf, 0x83,
0x7b, 0xae, 0xca, 0x7b, 0xd9, 0xe9, 0x53, 0x2a, 0xeb, 0xd2, 0xd8, 0xcd, 0xa3, 0x10, 0x25,
0x78, 0x5a, 0xb5, 0x23, 0x06, 0x93, 0xb7, 0x84, 0xd2, 0xbd, 0x96, 0x75, 0xa5, 0x5e, 0xcf,
0x4e, 0xe9, 0x50, 0xa1, 0xe6, 0x9d, 0xb1, 0xe3, 0x85, 0x66, 0x28, 0x4e, 0x43, 0xdc, 0x6e,
0xbb, 0x33, 0x9e, 0xf3, 0x0d, 0x00, 0xc1, 0xcf, 0x67, 0x34, 0x06, 0x7c, 0x71, 0xe3, 0x63,
0xb7, 0xb7, 0xdf, 0x92, 0xc4, 0xc2, 0x25, 0x5c, 0xff, 0xc3, 0x6e, 0xfc, 0xaa, 0x1e, 0x2a,
0x48, 0x11, 0x1c, 0x36, 0x68, 0x78, 0x86, 0x79, 0x30, 0xc3, 0xd6, 0xde, 0xbc, 0x3a, 0x2a,
0x6d, 0x1e, 0x46, 0xdd, 0xe0, 0x80, 0x1e, 0x44, 0x3b, 0x6f, 0xaf, 0x31, 0xda, 0xa2, 0xbd,
0x77, 0x06, 0x56, 0xc0, 0xb7, 0x92, 0x4b, 0x37, 0xc0, 0xfc, 0xc2, 0xd5, 0xfb, 0xa8, 0xda,
0xf5, 0x57, 0xa8, 0x18, 0xc0, 0xdf, 0xe7, 0xaa, 0x2a, 0xe0, 0x7c, 0x6f, 0x77, 0xb1, 0x26,
0xba, 0xf9, 0x2e, 0x1d, 0x16, 0xcb, 0xb8, 0xa2, 0x44, 0xd5, 0x2f, 0x1a, 0x79, 0x74, 0x87,
0x4b, 0x00, 0xc9, 0x4a, 0x3a, 0x65, 0x8f, 0xe6, 0x5d, 0xe5, 0x0a, 0x77, 0xd8, 0x1a, 0x14,
0x41, 0x75, 0xb1, 0xe2, 0x50, 0x2c, 0x93, 0x38, 0x2b, 0x6d, 0xf3, 0xf6, 0xdb, 0x1f, 0xcd,
0xff, 0x14, 0x70, 0xe7, 0x16, 0xe8, 0x3d, 0xf0, 0xe3, 0xbc, 0x5e, 0xb6, 0x3f, 0xcc, 0x81,
0x24, 0x67, 0xf3, 0x97, 0x3b, 0xfe, 0x3a, 0x96, 0x85, 0xdf, 0xe4, 0x6e, 0x3c, 0x85, 0x05,
0x0e, 0xa3, 0x2b, 0x07, 0xc8, 0xbf, 0xe5, 0x13, 0x82, 0x62, 0x08, 0x61, 0x69, 0x4b, 0x47,
0x62, 0x73, 0x44, 0x64, 0x8e, 0xe2, 0x91, 0xa6, 0x9a, 0xb7, 0xe9, 0x04, 0xb6, 0x54, 0x0c,
0xc5, 0xa9, 0x47, 0xa6, 0xc9, 0x08, 0xfe, 0x4e, 0xa6, 0xcc, 0x8a, 0x5b, 0x90, 0x6f, 0x2b,
0x3f, 0xb6, 0x0a, 0x96, 0xc0, 0x78, 0x58, 0x3c, 0x76, 0x6d, 0x94, 0x1a, 0xe4, 0x4e, 0xb8,
0x38, 0xbb, 0xf5, 0xeb, 0x29, 0xd8, 0xb0, 0xf3, 0x15, 0x1e, 0x99, 0x96, 0x3c, 0x5d, 0x63,
0xd5, 0xb1, 0xad, 0x52, 0xb8, 0x55, 0x70, 0x75, 0x3e, 0x1a, 0xd5, 0xda, 0xf6, 0x7a, 0x48,
0x7d, 0x44, 0x41, 0xf9, 0x11, 0xce, 0xd7, 0xca, 0xa5, 0x3d, 0x7a, 0x79, 0x7e, 0x7d, 0x25,
0x1b, 0x77, 0xbc, 0xf7, 0xc7, 0x0f, 0x84, 0x95, 0x10, 0x92, 0x67, 0x15, 0x11, 0x5a, 0x5e,
0x41, 0x66, 0x0f, 0x38, 0x03, 0xb2, 0xf1, 0x5d, 0xf8, 0xab, 0xc0, 0x02, 0x76, 0x84, 0x28,
0xf4, 0x9d, 0x56, 0x46, 0x60, 0x20, 0xdb, 0x68, 0xa7, 0xbb, 0xee, 0xac, 0x15, 0x01, 0x2f,
0x20, 0x09, 0xdb, 0xc0, 0x16, 0xa1, 0x89, 0xf9, 0x94, 0x59, 0x00, 0xc1, 0x76, 0xbf, 0xc1,
0x4d, 0x5d, 0x2d, 0xa9, 0x85, 0x2c, 0xd6, 0xd3, 0x14, 0xcc, 0x02, 0xc3, 0xc2, 0xfa, 0x6b,
0xb7, 0xa6, 0xef, 0xdd, 0x12, 0x26, 0xa4, 0x63, 0xe3, 0x62, 0xbd, 0x56, 0x8a, 0x52, 0x2b,
0xb9, 0xdf, 0x09, 0xbc, 0x0e, 0x97, 0xa9, 0xb0, 0x82, 0x46, 0x08, 0xd5, 0x1a, 0x8e, 0x1b,
0xa7, 0x90, 0x98, 0xb9, 0xbb, 0x3c, 0x17, 0x9a, 0xf2, 0x82, 0xba, 0x64, 0x0a, 0x7f, 0xca,
0x5a, 0x8c, 0x7c, 0xd3, 0x79, 0x09, 0x5b, 0x26, 0xbb, 0xbd, 0x25, 0xdf, 0x3d, 0x6f, 0x9a,
0x8f, 0xee, 0x21, 0x66, 0xb0, 0x8d, 0x84, 0x4c, 0x91, 0x45, 0xd4, 0x77, 0x4f, 0xb3, 0x8c,
0xbc, 0xa8, 0x99, 0xaa, 0x19, 0x53, 0x7c, 0x02, 0x87, 0xbb, 0x0b, 0x7c, 0x1a, 0x2d, 0xdf,
0x48, 0x44, 0x06, 0xd6, 0x7d, 0x0c, 0x2d, 0x35, 0x76, 0xae, 0xc4, 0x5f, 0x71, 0x85, 0x97,
0xc4, 0x3d, 0xef, 0x52, 0xbe, 0x00, 0xe4, 0xcd, 0x49, 0xd1, 0xd1, 0x1c, 0x3c, 0xd0, 0x1c,
0x42, 0xaf, 0xd4, 0xbd, 0x58, 0x34, 0x07, 0x32, 0xee, 0xb9, 0xb5, 0xea, 0xff, 0xd7, 0x8c,
0x0d, 0x2e, 0x2f, 0xaf, 0x87, 0xbb, 0xe6, 0x52, 0x71, 0x22, 0xf5, 0x25, 0x17, 0xa1, 0x82,
0x04, 0xc2, 0x4a, 0xbd, 0x57, 0xc6, 0xab, 0xc8, 0x35, 0x0c, 0x3c, 0xd9, 0xc2, 0x43, 0xdb,
0x27, 0x92, 0xcf, 0xb8, 0x25, 0x60, 0xfa, 0x21, 0x3b, 0x04, 0x52, 0xc8, 0x96, 0xba, 0x74,
0xe3, 0x67, 0x3e, 0x8e, 0x8d, 0x61, 0x90, 0x92, 0x59, 0xb6, 0x1a, 0x1c, 0x5e, 0x21, 0xc1,
0x65, 0xe5, 0xa6, 0x34, 0x05, 0x6f, 0xc5, 0x60, 0xb1, 0x83, 0xc1, 0xd5, 0xd5, 0xed, 0xd9,
0xc7, 0x11, 0x7b, 0x49, 0x7a, 0xf9, 0xf9, 0x84, 0x47, 0x9b, 0xe2, 0xa5, 0x82, 0xe0, 0xc2,
0x88, 0xd0, 0xb2, 0x58, 0x88, 0x7f, 0x45, 0x09, 0x67, 0x74, 0x61, 0xbf, 0xe6, 0x40, 0xe2,
0x9d, 0xc2, 0x47, 0x05, 0x89, 0xed, 0xcb, 0xbb, 0xb7, 0x27, 0xe7, 0xdc, 0x7a, 0xfd, 0xbf,
0xa8, 0xd0, 0xaa, 0x10, 0x39, 0x3c, 0x20, 0xf0, 0xd3, 0x6e, 0xb1, 0x72, 0xf8, 0xe6, 0x0f,
0xef, 0x37, 0xe5, 0x09, 0x33, 0x5a, 0x83, 0x43, 0x80, 0x4f, 0x65, 0x2f, 0x7c, 0x8c, 0x6a,
0xa0, 0x82, 0x0c, 0xd4, 0xd4, 0xfa, 0x81, 0x60, 0x3d, 0xdf, 0x06, 0xf1, 0x5f, 0x08, 0x0d,
0x6d, 0x43, 0xf2, 0xe3, 0x11, 0x7d, 0x80, 0x32, 0xc5, 0xfb, 0xc5, 0xd9, 0x27, 0xec, 0xc6,
0x4e, 0x65, 0x27, 0x76, 0x87, 0xa6, 0xee, 0xee, 0xd7, 0x8b, 0xd1, 0xa0, 0x5c, 0xb0, 0x42,
0x13, 0x0e, 0x95, 0x4a, 0xf2, 0x06, 0xc6, 0x43, 0x33, 0xf4, 0xc7, 0xf8, 0xe7, 0x1f, 0xdd,
0xe4, 0x46, 0x4a, 0x70, 0x39, 0x6c, 0xd0, 0xed, 0xca, 0xbe, 0x60, 0x3b, 0xd1, 0x7b, 0x57,
0x48, 0xe5, 0x3a, 0x79, 0xc1, 0x69, 0x33, 0x53, 0x1b, 0x80, 0xb8, 0x91, 0x7d, 0xb4, 0xf6,
0x17, 0x1a, 0x1d, 0x5a, 0x32, 0xd6, 0xcc, 0x71, 0x29, 0x3f, 0x28, 0xbb, 0xf3, 0x5e, 0x71,
0xb8, 0x43, 0xaf, 0xf8, 0xb9, 0x64, 0xef, 0xc4, 0xa5, 0x6c, 0x08, 0x53, 0xc7, 0x00, 0x10,
0x39, 0x4f, 0xdd, 0xe4, 0xb6, 0x19, 0x27, 0xfb, 0xb8, 0xf5, 0x32, 0x73, 0xe5, 0xcb, 0x32
};

/*
====================
Com_BlockSequenceCRCByte

For proxy protecting
====================
*/
byte	Com_BlockSequenceCRCByte(byte *base, int length, int sequence)
{
	int		n;
	byte	*p;
	int		x;
	byte chkb[60 + 4];
	unsigned short crc;


	if(sequence < 0)
		Com_Error(ERR_FATAL, "sequence < 0, this shouldn't happen\n");

	p = chktbl + (sequence % (sizeof(chktbl) - 4));

	if(length > 60)
		length = 60;
		
	memcpy(chkb, base, length);

	chkb[length] = p[0];
	chkb[length+1] = p[1];
	chkb[length+2] = p[2];
	chkb[length+3] = p[3];

	length += 4;

#if 0
	crc = CRC_Block(chkb, length);
#else
	crc = crc32(0L, chkb, length);
#endif

	for(x=0, n=0; n<length; n++)
		x += chkb[n];

	crc = (crc ^ x) & 0xff;

	return crc;
}





/*
=============
Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
void	Com_Error_f()
{
	Com_Error(ERR_FATAL, "%s", Cmd_Argv(1));
}



static void	Com_BenchMatrix_f()
{
	int start, end;
	
	if(Cmd_Argc() != 2)
	{
		Com_Printf("usage: benchmatrix <iterations number>\n");
		return;
	}
	
	int iterations_num = Cmd_ArgvInt(1);
	if(!iterations_num)
		iterations_num = 1000;
	
	matrix_c m1, m2, m3;
	vec4_c v1(1, 0, 0, 0), v2;
	
	m1.setupXRotation(56);
	m2.setupYRotation(79);
		
	start = Sys_Microseconds();
	for(int i=0; i<iterations_num; i++)
	{
		m3 = m1 * m2;
	}
	end = Sys_Microseconds();
	Com_Printf("matrix_c * matrix_c: %i\n", end - start);
	
	start = Sys_Microseconds();
	for(int i=0; i<iterations_num; i++)
	{
		v2 = m1 * v1;
	}
	end = Sys_Microseconds();
	Com_Printf("matrix_c * vec4_c: %i\n", end - start);
}

void	Com_MathCheck_f()
{
	//vec4_c v0(0, 0, 0, 0);
	//Com_Printf("vec4_c v0 : %s\n", v0.toString());
	
	//vec3_c		angles(0, 320, 0);
	//matrix_c	m;
	//quaternion_c	q;
	
	//m.fromAngles(angles);
	//Com_Printf("%s\n\n", m.toString());
	
	//angles.set(0, 90, 0);
	//Com_Printf("%s\n\n", angles.toString());
	
	//m.fromAngles(angles);
	//Com_Printf("%s\n\n", m.toString());
	
	//q.fromAngles(angles);
	//Com_Printf("%s\n\n", q.toString());
	
	//m.setupYRot(135);
	//Com_Printf("%s\n\n", m.toString());
	
	//m.toAngles(angles);
	
	vec3_c v(-344, 296, 244);
	vec3_c u(-352,-336, 244);
	
	vec_t dot = v.dotProduct(u);
	vec3_c n;
	n.crossProduct(u, v);
	
	Com_Printf("%f\n", dot);
	Com_Printf("%s\n", n.toString());
}



static int	sortfunc(void const *a, void const *b)
{
	int anum;
	int bnum;
	
	anum = *(int*)a;
	bnum = *(int*)b;

	if(anum > bnum)
		return -1;
	
	else if(anum < bnum)
		return 1;
	
	else
		return 0;
}

void	Com_SortCheck_f()
{
	int	a[10];
	
	a[0] = 5;
	a[1] = 7;
	a[2] = 8;
	a[3] = 9;
	a[4] = 0;
	a[5] = 1;
	a[6] = 3;
	a[7] = 4;
	a[8] = 2;
	a[9] = 6;
	
	qsort(a, 10, sizeof(a[0]), sortfunc);
	
	for(int i=0; i<10; i++)
		Com_Printf("%i ", a[i]);
	Com_Printf("\n");
}


/*
 * those are the standard FIPS 180-1 test vectors
 */

static char *sha1_msg[] = 
{
    "abc",
    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    NULL
};

static char *sha1_val[] =
{
    "a9993e364706816aba3e25717850c26c9cd0d89d",
    "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
    "34aa973cd4c4daa4f61eeb2bdbad27316534016f"
};


static void	Com_CryptoCheck_f()
{
	int i, j;
	char output[41];
	sha1_context_t ctx;
	byte sha1sum[20], buffer[1000];


	//if(Cmd_Argc()

	for(i=0; i<3; i++)
        {
		SHA1_Init(&ctx);
		
		if(i < 2)
		{
			SHA1_Update(&ctx, (byte*)sha1_msg[i], strlen(sha1_msg[i]));
		}
		else
		{
                	memset(buffer, 'a', 1000);
			
			for(j=0; j<1000; j++)
				SHA1_Update(&ctx, buffer, 1000);
		}
		
		SHA1_Finish(&ctx, sha1sum);
	
		for(j=0; j<20; j++)
		{
			//Com_sprintf(output + j*2, sizeof(output), "%02x", sha1sum[j]);
			sprintf(output + j*2, "%02x", sha1sum[j]);
		}
		
		output[41] = 0;
		
		Com_Printf("test %d ...\n", i+1);
		
		Com_Printf("orig: '%s'\n", sha1_val[i]);
		Com_Printf("new : '%s'\n", output);
		
		if(!memcmp(output, sha1_val[i], 40))
		{
			Com_Printf("passed\n\n");
		}
		else
		{
			Com_Printf("failed\n\n");
		}
		
		
	}
	
	Com_Printf("SHA1 encryption checks finished\n");
}


static void	Com_D3MapToQ3AMap_f()
{
	//char *token;
	
	char*		buf = NULL;
	std::string	filename;
	int		len;

	if(Cmd_Argc () != 3)
	{
		Com_Printf("d3maptoq3amap <importfilename> <exportfilename>: convert a DoomIII .map to a Q3A .map\n");
		return;
	}
	
	filename = Cmd_Argv(1);
	
	len = VFS_FLoad(filename, (void **)&buf);
	if (!buf)
	{
		Com_Printf("couldn't load %s\n", filename.c_str());
		return;
	}
	Com_Printf("converting %s ...\n", filename.c_str());
		
	Com_Printf("TODO");

	VFS_FFree(buf);
}

static void	Com_MiscCheck_f()
{
	/*
	union
	{
		unsigned long	l;
		byte		b[2];
	} dat1;
	
	union
	{
		unsigned long	l;
		byte		b[2];
	} dat2;
	
	dat1.l = 65336;
	dat2.l = 0;
	
	for(int i=0; i<32; i++)
	{
		//int val = dat1.b[i>>3] >> (i&7);
		//dat2.b[i>>3] |= (val & 1) << i;
		
		//dat2.b[i>>3] |= dat1.b[i>>3];
		
		dat2.b[i>>3] |= (dat1.l>>i) & 1;
		
	}

	for(int i=0; i<32; i++)
	{
		int val = dat1.b[i>>3] >> (i&7);
		Com_Printf("%i", (val & 1) << i ? 1 : 0);
	}
	Com_Printf("\n");
	
	for(int i=0; i<32; i++)
	{
		int val = dat2.b[i>>3] >> (i&7);
		Com_Printf("%i", (val & 1) << i ? 1 : 0);
	}
	Com_Printf("\n");
	*/

	//boost::dynamic_bitset<> bits(32, 5ul);
	//for(int i=0; i<32; i++)
	//	bits[i] = (dat.b[i>>3] >> (i&7)) ? 1 : 0;
			
	//for(boost::dynamic_bitset<>::size_type i=0; i<bits.size(); ++i)
	//	std::cout << bits[i];
	//std::cout << std::endl;
	
	//std::cout << bits << std::endl;
	
	std::vector<int>	ints;
	for(int i=0; i<10; i++)
		ints.push_back(i);
		
	for(std::vector<int>::const_iterator ir = ints.begin(); ir != ints.end(); ++ir)
		Com_Printf("%i", *ir);
	Com_Printf("\n");
	
	for(std::vector<int>::const_iterator ir = ints.end()-1; ir != ints.begin()-1; --ir)
		Com_Printf("%i", *ir);
	Com_Printf("\n");
}

static void	Com_MessageCheck_f()
{
	bitmessage_c	msg(MAX_PACKETLEN*8);
	
	msg.beginWriting();
	msg.writeByte(37);
	msg.writeBits(10, 4);
	msg.writeShort(789);
	msg.writeLong(125000);
	msg.writeFloat(3.142f);
	msg.writeString("This\nis\na\nline\nseparated\nstring.\n");
	msg.writeFloat(0.0000123f);
	msg.writeString("This is a non-separated string.");
//	static byte buf[] = "Hello World. The grass is green. Bob was here.";
//	msg.writeBytesCompressed(buf, sizeof(buf));
//	msg.writeBytesCompressed(buf, sizeof(buf));
//	msg.writeBytesCompressed(buf, sizeof(buf));
	
	msg.beginReading();
	byte b = msg.readByte();
	Com_Printf("%i\n", b);
	
	b = msg.readBits(4);
	Com_Printf("%i\n", b);
	
	short s = msg.readShort();
	Com_Printf("%i\n", s);
	
	int l = msg.readLong();
	Com_Printf("%i\n", l);
	
	float f = msg.readFloat();
	Com_Printf("%f\n", f);
	
	const char *str = msg.readString();
	Com_Printf("%s\n", str);
	
	f = msg.readFloat();
	Com_Printf("%f\n", f);
	
	str = msg.readString();
	Com_Printf("%s\n", str);
	
	//while(msg.getReadCount() < msg.getCurSize())
//	for(int i=0; i<3; i++)
//	{
//		std::vector<byte> v;
//		msg.readBytesCompressed(v);
//		str = (const char*)&v[0];
//		Com_Printf("'%s'\n", str);
//	}
	
	Com_Printf("%i\n", msg.getReadCount() == msg.getCurSize());
}

void 	Com_Init(int argc, char **argv)
{
	Com_Printf("------- Com_Init -------\n");

	if(setjmp(abortframe))
		Com_Error(ERR_FATAL, "Error during initialization");

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	Com_InitArgv(argc, argv);
	
	Swap_Init();
	
	Cmd_Init();
	Cvar_Init();
	
	Key_Init();

	// we need to add the early commands twice, because
	// a basedir or cddir needs to be set before execing
	// config files, but we want other parms to override
	// the settings of the config files
	Cbuf_AddEarlyCommands(false);
	Cbuf_Execute();

	VFS_Init();

	Cbuf_AddText("exec default.cfg\n");
	Cbuf_AddText("exec config.cfg\n");

	Cbuf_AddEarlyCommands(true);
	Cbuf_Execute();

	//
	// init commands and vars
	//
    	Cmd_AddCommand("error", Com_Error_f);
	Cmd_AddCommand("mathcheck", Com_MathCheck_f);
	Cmd_AddCommand("sortcheck", Com_SortCheck_f);
	Cmd_AddCommand("cryptocheck", Com_CryptoCheck_f);
	Cmd_AddCommand("d3maptoq3amap", Com_D3MapToQ3AMap_f);
	Cmd_AddCommand("misccheck", Com_MiscCheck_f);
	Cmd_AddCommand("messagecheck", Com_MessageCheck_f);
	
	Cmd_AddCommand("benchmatrix", Com_BenchMatrix_f);
	

	com_speeds	= Cvar_Get("com_speeds", "0", 0);
	developer	= Cvar_Get("developer", "1", 0);
	timescale	= Cvar_Get("timescale", "1", 0);
	fixedtime	= Cvar_Get("fixedtime", "0", 0);
	logfile_active	= Cvar_Get("logfile", "0", 0);
	showtrace	= Cvar_Get("showtrace", "0", 0);
#ifdef DEDICATED_ONLY
	dedicated	= Cvar_Get("dedicated", "1", CVAR_INIT);
#else
	dedicated	= Cvar_Get("dedicated", "0", CVAR_INIT);
#endif

	char *s = va("%s %s %s", VERSION, BUILDHOST, __DATE__);
	Cvar_Get("version", s, CVAR_SERVERINFO | CVAR_INIT);


	if(dedicated->getValue())
		Cmd_AddCommand("quit", Com_Shutdown);

	Sys_Init();

#ifdef DEDICATED_ONLY
	Sys_InitNet(true);
#else
	Sys_InitNet(false);
#endif
	
#ifdef BSPCOMPILER_ONLY
	Map_Init();
#else
	Netchan_Init();

	SV_Init();
	CL_Init();
#endif

	// add + commands from command line
	if(!Cbuf_AddLateCommands())
	{	
		// if the user didn't give any commands, run default action
		if(!dedicated->getValue())
			Cbuf_AddText("d1\n");
		else
			Cbuf_AddText("dedicated_start\n");
			
		Cbuf_Execute();
	}
	else
	{	// the user asked for something explicit
		// so drop the loading plaque
		//SCR_EndLoadingPlaque ();
	}

	Com_Printf("====== QRazor FX III Initialized ======\n\n");	
}

void	Com_Frame(int msec)
{
	char	*s;
	int	time_before, time_between, time_after;

	time_before = 0;
	time_between = 0;
	time_after = 0;

	if(setjmp(abortframe))
		return;			// an ERR_DROP was thrown

	if(fixedtime->getInteger())
	{
		msec = fixedtime->getInteger();
	}	
	else if(timescale->getValue())
	{
		msec = (int)(msec * timescale->getValue());
		if(msec < 1)
			msec = 1;
	}

	if(showtrace->getValue())
	{
		extern	int cm_traces, cm_brush_traces;
		extern	int cm_pointcontents;

		Com_Printf("%4i traces  %4i points\n", cm_traces, cm_pointcontents);
		
		cm_traces = 0;
		cm_brush_traces = 0;
		cm_pointcontents = 0;
	}
	
	Sys_PushEvents();

	do
	{
		s = Sys_ConsoleInput();
		if(s)
			Cbuf_AddText(va("%s\n",s));
	} while(s);
	
	Com_EventLoop();
	
	Cbuf_Execute();
	
	VFS_CheckForChanges();

	if(com_speeds->getValue())
		time_before = Sys_Milliseconds();

	SV_Frame(msec);

	if(com_speeds->getValue())
		time_between = Sys_Milliseconds();		

	CL_Frame(msec);

	if(com_speeds->getValue())
		time_after = Sys_Milliseconds();
		
	Map_Frame(msec);

	if(com_speeds->getValue())
	{
		int			all, sv, gm, cl, rf;

		all = time_after - time_before;
		sv = time_between - time_before;
		cl = time_after - time_between;
		gm = time_after_game - time_before_game;
		rf = time_after_ref - time_before_ref;
		sv -= gm;
		cl -= rf;
		
		Com_Printf ("all:%3i sv:%3i gm:%3i cl:%3i rf:%3i\n", all, sv, gm, cl, rf);
	}	
}

void	Com_Shutdown()
{
	SV_Shutdown("Server quit\n", false);
	CL_Shutdown();
	Map_Shutdown();

	if(logfile)
		VFS_FClose(&logfile);

	Sys_Shutdown();
}

