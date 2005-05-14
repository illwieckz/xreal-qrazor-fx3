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


/// includes ===================================================================
// system -------------------------------------------------------------------
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/uio.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <mntent.h>
#include <dlfcn.h>
#include <dirent.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <exception>

// shared -------------------------------------------------------------------
#include "x_bitmessage.h"
#include "x_protocol.h"

// qrazor-fx ----------------------------------------------------------------
#include "sys.h"
#include "common.h"
#include "cvar.h"
#include "vfs.h"
#include "glob.h"
#include "rw_linux.h"


cvar_t *nostdout;


unsigned	sys_frame_time;


bool stdin_active = true;


void 	Sys_Init()
{
	Com_Printf("------- Sys_Init -------\n");
	
	//do nothing
}

void 	Sys_Shutdown()
{
	fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
	//_exit(0);
	exit(EXIT_SUCCESS);
}



void	Sys_ConsoleOutput(char *string)
{
	if(nostdout && nostdout->getInteger())
		return;

	fputs(string, stdout);
}

void 	Sys_Printf(char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];
	unsigned char		*p;

	va_start(argptr,fmt);
	vsprintf(text,fmt,argptr);
	va_end(argptr);

	if(strlen(text) > sizeof(text))
		Sys_Error("memory overwrite in Sys_Printf");

	if(nostdout && nostdout->getInteger())
        	return;

	for(p = (unsigned char *)text; *p; p++)
	{
		*p &= 0x7f;
		
		if((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf("[%02x]", *p);
		else
			putc(*p, stdout);
	}
}


void 	Sys_Error(const char *error, ...)
{ 
	va_list     argptr;
	char        string[1024];

	// change stdin to non blocking
	fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);

	//CL_Shutdown();
	//Com_Shutdown();
    
	va_start(argptr,error);
	vsprintf(string,error,argptr);
	va_end(argptr);
	fprintf(stderr, "Error: %s\n", string);

	_exit(1);
}

char*	Sys_ConsoleInput()
{
	static char text[256];
	int     len;
	fd_set	fdset;
	struct timeval timeout;

	if(!dedicated || !dedicated->getInteger())
		return NULL;

	if(!stdin_active)
		return NULL;

	FD_ZERO(&fdset);
	FD_SET(0, &fdset); // stdin
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	if(select(1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
		return NULL;

	len = read (0, text, sizeof(text));
	if(len == 0)
	{ // eof!
		stdin_active = false;
		return NULL;
	}

	if(len < 1)
		return NULL;
	text[len-1] = 0;    // rip off the /n and terminate

	return text;
}

extern "C" void	KBD_Update();

void 	Sys_PushEvents()
{
	//
	// push SE_KEY events
	//
#ifndef DEDICATED_ONLY
 #ifndef BSPCOMPILER_ONLY
  #ifndef REF_HARD_LINKED
	if(KBD_Update_fp)
		KBD_Update_fp();	
  #else
	KBD_Update();
  #endif
 #endif
#endif


	//
	// push SE_PACKET events
	//
	netadr_t	adr;
	byte		packet[MAX_PACKETLEN];
	int		length;
	
	byte*		data;
	int		data_size;
	
	while((length = Sys_GetPacket(packet, sizeof(packet), adr)) >= 0)
	{
		data_size = sizeof(adr) + length;
		data = new byte[data_size];
		
		memcpy(data, &adr, sizeof(adr));
		memcpy((byte*)data + sizeof(adr), packet, length);
		
		Com_PushEvent(SE_PACKET, -1, -1, -1, data, data_size);
	}
}




/*
=================
Sys_GetAPI

Loads the game dll or ui dll or whatever api dll you designed
=================
*/
void*	Sys_GetAPI(const char *api_name, const char *api_main, void *api_parms, void **api_handle)
{
	std::string	name;
	std::string	name_modul;
	
	std::string	path = "";
	void*	(*api) (void*) = NULL;

	name_modul = api_name;
	name_modul += ".so";

	if(*api_handle)
		Com_Error(ERR_FATAL, "Sys_GetAPI without Sys_UnloadAPI");

	Com_Printf("------- Loading %s -------\n", name_modul.c_str());

	name = VFS_FullPath(name_modul);
	
	*api_handle = dlopen(name.c_str(), RTLD_NOW);
			
	if(*api_handle)
	{
		Com_Printf("success: '%s'\n", name.c_str());
	}
	else
	{
		Com_Printf("no success: '%s'\n", dlerror());
		return NULL;
	}

	api = (void*(*)(void*))dlsym(*api_handle, api_main);
	
	if(!api)
	{
		Sys_UnloadAPI(api_handle);		
		return NULL;
	}

	return api(api_parms);
}

void 	Sys_UnloadAPI(void **api_handle)
{
	if(*api_handle) 
		dlclose(*api_handle);
		
	*api_handle = NULL;
}



void	Sys_AppActivate()
{
	//DO NOTHING
}




char*	Sys_GetClipboardData()
{
	return NULL;
}

int 	main(int argc, char **argv)
{
	int 	time, oldtime, newtime;

	Com_Init(argc, argv);

	fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);

	nostdout = Cvar_Get("nostdout", "0", 0);
	if(!nostdout->getInteger())
	{
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
	}

	oldtime = Sys_Milliseconds();
	while(true)
	{
		// find time spent rendering last frame
		do
		{
			newtime = Sys_Milliseconds();
			time = newtime - oldtime;
		}while(time < 1);
		
		try
		{
			Com_Frame(time);
		}
		catch(std::exception e)
		{
			Com_Error(ERR_FATAL, "exception thrown: '%s'", e.what()); 
		}
		
		oldtime = newtime;
	}
}




const char*	Sys_GetHomeDir()
{
	const char* home = getenv("HOME");
	
	if(!home)
		return ".";
	else
		return (std::string(home) + "/" + ".qrazor-fx").c_str();
}

int	Sys_Milliseconds()
{
	struct timeval tp;
	static int		secbase;

	gettimeofday(&tp, NULL);
	
	if(!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000;
	}

	return ((tp.tv_sec - secbase)*1000 + tp.tv_usec/1000);
}

void 	Sys_Mkdir(const std::string &path)
{
	mkdir(path.c_str(), 0777);
}


static netadr_t	net_local_adr;

static int			ip_socket;
//static int			ip6_socket = 0;

char*	Sys_NetErrorString();



static void 	Sys_NetadrToSockadr(const netadr_t &a, struct sockaddr_in *s)
{
	memset(s, 0, sizeof(*s));

	switch(a.type)
	{
		case NA_BROADCAST:
			s->sin_family = AF_INET;
			
			s->sin_port = a.port;
			*(int *)&s->sin_addr = -1;
			break;
		
		case NA_IP:
			s->sin_family = AF_INET;
			
			*(int *)&s->sin_addr = *(int *)&a.ip;
			s->sin_port = a.port;
			break;
			
		case NA_IP6:
			//TODO
			break;
	}
}

static void 	Sys_SockadrToNetadr(struct sockaddr_in *s, netadr_t &a)
{
	*(int *)&a.ip = *(int *)&s->sin_addr;
	a.port = s->sin_port;
	a.type = NA_IP;
}


bool	Sys_CompareAdr(const netadr_t &a, const netadr_t &b)
{
	if(a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] && a.port == b.port)
		return true;
		
	return false;
}

/*
===================
NET_CompareBaseAdr

Compares without the port
===================
*/
bool	Sys_CompareBaseAdr(const netadr_t &a, const netadr_t &b)
{
	if(a.type != b.type)
		return false;

	if(a.type == NA_IP)
	{
		if(a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3])
			return true;
			
		return false;
	}

	return false;
}

char*	Sys_AdrToString(const netadr_t &a)
{
	static	char	s[64];
	
	Com_sprintf(s, sizeof(s), "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], ntohs(a.port));

	return s;
}

char*	Sys_BaseAdrToString(const netadr_t &a)
{
	static	char	s[64];
	
	Com_sprintf(s, sizeof(s), "%i.%i.%i.%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3]);

	return s;
}

/*
=============
NET_StringToAdr

localhost
idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
static bool	Sys_StringToSockaddr(const char *s, struct sockaddr *sadr)
{
	struct hostent	*h;
	char	*colon;
	char	copy[128];
	
	memset(sadr, 0, sizeof(*sadr));
	
	((struct sockaddr_in *)sadr)->sin_family = AF_INET;
	
	((struct sockaddr_in *)sadr)->sin_port = 0;

	strcpy (copy, s);
	
	// strip off a trailing :port if present
	for(colon = copy ; *colon ; colon++)
		if (*colon == ':')
		{
			*colon = 0;
			((struct sockaddr_in *)sadr)->sin_port = htons((short)atoi(colon+1));	
		}
	
	if(copy[0] >= '0' && copy[0] <= '9')
	{
		*(int *)&((struct sockaddr_in *)sadr)->sin_addr = inet_addr(copy);
	}
	else
	{
		if (! (h = gethostbyname(copy)) )
			return 0;
		*(int *)&((struct sockaddr_in *)sadr)->sin_addr = *(int *)h->h_addr_list[0];
	}
	
	return true;
}

/*
=============
NET_StringToAdr

localhost
idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
bool	Sys_StringToAdr(const char *s, netadr_t &a)
{
	struct sockaddr_in sadr;
	
	if(!Sys_StringToSockaddr(s, (struct sockaddr *)&sadr))
		return false;
	
	Sys_SockadrToNetadr(&sadr, a);

	return true;
}


bool	Sys_IsLocalAddress(const netadr_t &adr)
{
	return Sys_CompareAdr(adr, net_local_adr);
}





void 	Sys_SendPacket(const bitmessage_c &msg, const netadr_t &to)
{
	int			ret;
	struct sockaddr_in	addr;
	int			socket = 0;

	switch(to.type)
	{
		case NA_BROADCAST:
		case NA_IP:
			socket = ip_socket;
			if(!socket)
				return;
			break;
		
		default:
			Com_Error(ERR_FATAL, "Sys_SendPacket: bad address type");
	}

	Sys_NetadrToSockadr(to, &addr);

	ret = sendto(socket, &msg._data[0], msg.getCurSizeInBytes(), 0, (struct sockaddr *)&addr, sizeof(addr));
	if(ret == -1)
	{
		Com_Printf("Sys_SendPacket ERROR: '%s' to '%s'\n", Sys_NetErrorString(), Sys_AdrToString(to));
	}
}

int	Sys_GetPacket(void *data, int length, netadr_t &net_from)
{
	int 	ret;
	struct sockaddr_in	from;
	int		fromlen;
	int		socket;
//	int		protocol;
	int		err;

	//for(protocol = 0; protocol < 1; protocol++)
	{
		//if(protocol == 0)
			socket = ip_socket;
		//else
		//	socket = ip6_socket;

		if(!socket)
			return -1;

		fromlen = sizeof(from);
		ret = recvfrom(socket, data, length, 0, (struct sockaddr*)&from, (socklen_t*)&fromlen);

		Sys_SockadrToNetadr(&from, net_from);

		if(ret == -1)
		{
			err = errno;

			if(err == EWOULDBLOCK || err == ECONNREFUSED)
				return -1;
				
			Com_Printf("Sys_GetPacket: '%s' from '%s'\n", Sys_NetErrorString(), Sys_AdrToString(net_from));
			return -1;
		}

		if(ret == length)
		{
			Com_Printf("Sys_GetPacket: Oversize packet from '%s'\n", Sys_AdrToString(net_from));
			return -1;
		}

		return ret;
	}

	return -1;
}

static void 	Sys_OpenIP()
{
	
}

/*
====================
NET_Config

A single player game will only use the loopback code
====================
*/
void	Sys_ConfigNet(bool multiplayer)
{
	if(!multiplayer)
	{	
		
	}
	else
	{	// open sockets
		Sys_OpenIP();
	}
}


static int 	Sys_Socket(const char *net_interface, int port)
{
	int newsocket;
	struct sockaddr_in address;
	bool _true = true;
	int	i = 1;

	if((newsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		Com_Printf("ERROR: Sys_OpenSocket: socket: %s", Sys_NetErrorString());
		return 0;
	}

	// make it non-blocking
	if(ioctl(newsocket, FIONBIO, &_true) == -1)
	{
		Com_Printf("ERROR: Sys_OpenSocket: ioctl FIONBIO:%s\n", Sys_NetErrorString());
		return 0;
	}

	// make it broadcast capable
	if(setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) == -1)
	{
		Com_Printf("ERROR: Sys_OpenSocket: setsockopt SO_BROADCAST:%s\n", Sys_NetErrorString());
		return 0;
	}

	if(!net_interface || !net_interface[0] || !X_stricmp(net_interface, "localhost"))
		address.sin_addr.s_addr = INADDR_ANY;
	else
		Sys_StringToSockaddr(net_interface, (struct sockaddr *)&address);

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	address.sin_family = AF_INET;


	if(bind(newsocket, (const sockaddr*)&address, sizeof(address)) == -1)

	{
		Com_Printf("ERROR: Sys_OpenSocket: bind: %s\n", Sys_NetErrorString());
		close(newsocket);
		return 0;
	}

	return newsocket;
}

void 	Sys_InitNet(bool server)
{
	Com_Printf("------- Sys_InitNet -------\n");
	
	cvar_t	*port, *ip;

	port	= Cvar_Get("port",	va("%i", PORT_SERVER),	CVAR_INIT);
	ip	= Cvar_Get("ip",	"localhost",		CVAR_INIT);


	if(server)
	{
		int	i = 0;
		
		for(i=0; i<4; i++)
		{
			ip_socket = Sys_Socket(ip->getString(), port->getInteger());
			
			if(ip_socket)
			{
				Cvar_SetInteger("port", port->getInteger() + 1);
				break;
			}
		}
		
		if(i == 4)
		{
			Com_Printf("Sys_InitNet: couldn't allocate server IP port\n");
		}
	}
	else
	{
		if(!ip_socket)
			ip_socket = Sys_Socket(ip->getString(), PORT_ANY);//PORT_CLIENT);
	}
}

void	Sys_ShutdownNet()
{
	// shut down any existing sockets
	if(ip_socket)
	{
		close(ip_socket);
		ip_socket = 0;
	}
}


char*	Sys_NetErrorString()
{
	int		code;

	code = errno;
	
	return strerror(code);
}

// sleeps msec or until net socket is ready
void 	Sys_SleepNet(int msec)
{
    	struct timeval timeout;
	fd_set	fdset;
	extern cvar_t *dedicated;
	extern bool stdin_active;

	if(!ip_socket || (dedicated && !dedicated->getInteger()))
		return; // we're not a server, just run full speed

	FD_ZERO(&fdset);
	if(stdin_active)
		FD_SET(0, &fdset); // stdin is processed too
		
	FD_SET(ip_socket, &fdset); // network socket
	timeout.tv_sec = msec/1000;
	timeout.tv_usec = (msec%1000)*1000;
	select(ip_socket+1, &fdset, NULL, NULL, &timeout);
}









static char	findbase[MAX_OSPATH];
static char	findpath[MAX_OSPATH];
static char	findpattern[MAX_OSPATH];
static DIR*	fdir;


static bool 	Sys_CompareAttributes(char *path, char *name, unsigned musthave, unsigned canthave)
{
	struct stat st;
	char fn[MAX_OSPATH];

	// . and .. never match
	if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return false;

	return true;

	if(stat(fn, &st) == -1)
		return false; // shouldn't happen

	if((st.st_mode & S_IFDIR) && (canthave & SFF_SUBDIR))
		return false;

	if((musthave & SFF_SUBDIR) && !(st.st_mode & S_IFDIR))
		return false;

	return true;
}

std::string	Sys_FindFirst(const std::string &path, unsigned musthave, unsigned canhave)
{
	struct dirent *d;
	char *p;

	if(fdir)
		Sys_Error("Sys_BeginFind without close");

//	COM_FilePath (path, findbase);
	strcpy(findbase, path.c_str());

	if((p = strrchr(findbase, '/')) != NULL)
	{
		*p = 0;
		strcpy(findpattern, p + 1);
	}
	else
		strcpy(findpattern, "*");

	if(strcmp(findpattern, "*.*") == 0)
		strcpy(findpattern, "*");
	
	if((fdir = opendir(findbase)) == NULL)
		return "";
		
	while((d = readdir(fdir)) != NULL) 
	{
		if(!*findpattern || glob_match(findpattern, d->d_name)) 
		{
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);

			if(Sys_CompareAttributes(findbase, d->d_name, musthave, canhave)) 
			{
				sprintf(findpath, "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return "";
}

std::string	Sys_FindNext(unsigned musthave, unsigned canhave)
{
	struct dirent *d;

	if(fdir == NULL)
		return "";
		
	while((d = readdir(fdir)) != NULL)
	{
		if(!*findpattern || glob_match(findpattern, d->d_name)) 
		{
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);

			if(Sys_CompareAttributes(findbase, d->d_name, musthave, canhave)) 
			{
				sprintf(findpath, "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return "";
}

void 	Sys_FindClose()
{
	if(fdir != NULL)
		closedir(fdir);
		
	fdir = NULL;
}




