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
#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

// qrazor-fx ----------------------------------------------------------------
#include "sys.h"
#include "common.h"
#include "cvar.h"
#include "winquake.h"
#include "resource.h"
#include "vfs.h"
#include "sys.h"

// shared -------------------------------------------------------------------
#include "x_protocol.h"
#include "x_bitmessage.h"

#define MINIMUM_WIN_MEMORY	0x0a00000
#define MAXIMUM_WIN_MEMORY	0x1000000

bool s_win95;

int			starttime;
bool			ActiveApp;
bool	Minimized;

static HANDLE		hinput, houtput;

unsigned	sys_msg_time;


static HANDLE		qwclsemaphore;

#define	MAX_NUM_ARGVS	128
int			argc;
char		*argv[MAX_NUM_ARGVS];


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

std::string Sys_GetError()
{
	TCHAR szBuf[1024];
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();

	FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

	wsprintf(szBuf, "%s", lpMsgBuf);

	std::string retval(szBuf);

	LocalFree(lpMsgBuf);

	return retval;
}

void	Sys_Init()
{
	OSVERSIONINFO	vinfo;

#if 0
	// allocate a named semaphore on the client so the
	// front end can tell if it is alive

	// mutex will fail if semephore already exists
    qwclsemaphore = CreateMutex(
        NULL,         /* Security attributes */
        0,            /* owner       */
        "qwcl"); /* Semaphore name      */
	if (!qwclsemaphore)
		Sys_Error ("QWCL is already running on this system");
	CloseHandle (qwclsemaphore);

    qwclsemaphore = CreateSemaphore(
        NULL,         /* Security attributes */
        0,            /* Initial count       */
        1,            /* Maximum count       */
        "qwcl"); /* Semaphore name      */
#endif

	timeBeginPeriod(1);

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if(!GetVersionEx(&vinfo))
		Sys_Error("Couldn't get OS info");

	if(vinfo.dwMajorVersion < 4)
		Sys_Error("Quake2 requires windows version 4 or greater");
	if(vinfo.dwPlatformId == VER_PLATFORM_WIN32s)
		Sys_Error("Quake2 doesn't run on Win32s");
	else if(vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
		s_win95 = true;

	if((bool)dedicated->getValue())
	{
		if( ! FreeConsole())
		{
			std::string error("Error: \n");
			error += "Couldn't detach from console\n\n";
			error += "Message: ";
			error += Sys_GetError();
			Sys_Error(error.c_str());
		}
		if( ! AllocConsole())
		{
			std::string error("Error: \n");
			error += "Couldn't create dedicated"
				" server console\n\n";
			error += "Message: ";
			error += Sys_GetError();
			Sys_Error(error.c_str());
		}
		hinput = GetStdHandle(STD_INPUT_HANDLE);
		houtput = GetStdHandle(STD_OUTPUT_HANDLE);
	
		// let QHOST hook in
		//InitConProc (argc, argv);
	}
}

void	Sys_Shutdown()
{
	timeEndPeriod( 1 );

	CloseHandle(qwclsemaphore);
	
	if((bool)dedicated && dedicated->getValue())
		FreeConsole();

	// shut down QHOST hooks if necessary
	//DeinitConProc ();

	exit(0);
}



void	Sys_Error(const char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	//CL_Shutdown();
	//Com_Shutdown();

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	//MessageBox(NULL, text, "Error", 0 /* MB_OK */ );
    printf(text);

	if(qwclsemaphore)
		CloseHandle(qwclsemaphore);

	// shut down QHOST hooks if necessary
	//DeinitConProc ();

	exit(1);
}


/*
static void	WinError()
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);

	// Display the string.
	MessageBox(NULL, (const CHAR*)lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION);

	// Free the buffer.
	LocalFree(lpMsgBuf);
}
*/

char*	Sys_ExpandSquiggle(const char *in)
{
	//TODO ?
	return "";
}

int	Sys_Milliseconds()
{
	static int		base;
	static bool	initialized = false;

	if(!initialized)
	{
		// let base retain 16 bits of effectively random data
		base = timeGetTime() & 0xffff0000;
		initialized = true;
	}
	
	return timeGetTime() - base;
}

void	Sys_Mkdir(const std::string &path)
{
	_mkdir(path.c_str());
}




#if ! defined(BSPCOMPILER_ONLY)
static netadr_t	net_local_adr;
static int			ip_socket;

char*	Sys_NetErrorString (void);



void	Sys_NetadrToSockadr(const netadr_t &a, struct sockaddr *s)
{
	memset(s, 0, sizeof(*s));

	switch(a.type)
	{
		case NA_BROADCAST:
			((struct sockaddr_in *)s)->sin_family = AF_INET;
			((struct sockaddr_in *)s)->sin_port = a.port;
			((struct sockaddr_in *)s)->sin_addr.s_addr = INADDR_BROADCAST;
			break;

		case NA_IP:
			((struct sockaddr_in *)s)->sin_family = AF_INET;
			((struct sockaddr_in *)s)->sin_addr.s_addr = *(int *)&a.ip;
			((struct sockaddr_in *)s)->sin_port = a.port;
			break;

		case NA_IP6:
			//TODO
			break;
	}
	//else if (a->type == NA_IPX)
	//{
	//	((struct sockaddr_ipx *)s)->sa_family = AF_IPX;
	//	memcpy(((struct sockaddr_ipx *)s)->sa_netnum, &a->ipx[0], 4);
	//	memcpy(((struct sockaddr_ipx *)s)->sa_nodenum, &a->ipx[4], 6);
	//	((struct sockaddr_ipx *)s)->sa_socket = a->port;
	//}
	//else if (a->type == NA_BROADCAST_IPX)
	//{
	//	((struct sockaddr_ipx *)s)->sa_family = AF_IPX;
	//	memset(((struct sockaddr_ipx *)s)->sa_netnum, 0, 4);
	//	memset(((struct sockaddr_ipx *)s)->sa_nodenum, 0xff, 6);
	//	((struct sockaddr_ipx *)s)->sa_socket = a->port;
	//}
}

static void	Sys_SockadrToNetadr(struct sockaddr *s, netadr_t &a)
{
	//if (s->sa_family == AF_INET)
	{
		a.type = NA_IP;
		*(int *)&a.ip = ((struct sockaddr_in *)s)->sin_addr.s_addr;
		a.port = ((struct sockaddr_in *)s)->sin_port;
		a.type = NA_IP;	//FIXME
	}
	//else if (s->sa_family == AF_IPX)
	//{
	//	a->type = NA_IPX;
	//	memcpy(&a->ipx[0], ((struct sockaddr_ipx *)s)->sa_netnum, 4);
	//	memcpy(&a->ipx[4], ((struct sockaddr_ipx *)s)->sa_nodenum, 6);
	//	a->port = ((struct sockaddr_ipx *)s)->sa_socket;
	//}
}


bool	Sys_CompareAdr(const netadr_t &a, const netadr_t &b)
{
	if(a.type != b.type)
		return false;

	if(a.type == NA_IP)
	{
		if(a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] && a.port == b.port)
			return true;
		return false;
	}

	//if (a.type == NA_IPX)
	//{
	//	if ((memcmp(a.ipx, b.ipx, 10) == 0) && a.port == b.port)
	//		return true;
	//	return false;
	//}
	
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

	//if (a.type == NA_IPX)
	//{
	//	if ((memcmp(a.ipx, b.ipx, 10) == 0))
	//		return true;
	//	return false;
	//}
	
	return false;
}

char*	Sys_AdrToString(const netadr_t &a)
{
	static	char	s[64];

	//if(a.type == NA_IP)
		Com_sprintf(s, sizeof(s), "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], ntohs(a.port));
	//else
	//	Com_sprintf (s, sizeof(s), "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%i", a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9], ntohs(a.port));

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
	{
		if(*colon == ':')
		{
			*colon = 0;
			((struct sockaddr_in *)sadr)->sin_port = htons((short)atoi(colon+1));	
		}
	}
		
	if (copy[0] >= '0' && copy[0] <= '9')
	{
		*(int *)&((struct sockaddr_in *)sadr)->sin_addr = inet_addr(copy);
	}
	else
	{
		if(! (h = gethostbyname(copy)) )
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
	struct sockaddr sadr;
	
	if(!Sys_StringToSockaddr(s, &sadr))
		return false;
	
	Sys_SockadrToNetadr(&sadr, a);

	return true;
}


bool	Sys_IsLocalAddress(const netadr_t &adr)
{
	return Sys_CompareAdr(adr, net_local_adr);
}


int	Sys_GetPacket(void *data, int length, netadr_t &net_from)
{
	int 	ret;
	struct sockaddr from;
	int		fromlen;
	int		socket;
	int		err;

//	for(int protocol = 0 ; protocol < 2 ; protocol++)
	{
		//if(protocol == 0)
			socket = ip_socket;
		//else
		//	net_socket = ipx_sockets[sock];

		if(!socket)
			return -1;

		fromlen = sizeof(from);
		ret = recvfrom(socket, (char*)data, length, 0, (struct sockaddr*)&from, &fromlen);

		Sys_SockadrToNetadr(&from, net_from);

		if(ret == -1)
		{
			err = WSAGetLastError();

			if(err == WSAEWOULDBLOCK)
				return -1;
				
			if(err == WSAEMSGSIZE)
			{
				Com_Printf("Sys_GetPacket:  Oversize packet from '%s'\n", Sys_AdrToString(net_from));
				return -1;
			}

			//if((bool)dedicated->getValue())	// let dedicated servers continue after errors
				//Com_Printf ("NET_GetPacket: %s from %s\n", Sys_NetErrorString(), Sys_AdrToString(*net_from));
			//else
				//Com_Error (ERR_DROP, "NET_GetPacket: %s from %s", Sys_NetErrorString(), Sys_AdrToString(*net_from));
			
			Com_Printf("Sys_GetPacket: %i from '%s'\n", err, Sys_AdrToString(net_from));
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


void	Sys_SendPacket(const bitmessage_c &msg, const netadr_t &to)
{
	int		ret;
	struct sockaddr	addr;
	int		socket = 0;

	switch(to.type)
	{
		//case NA_LOOPBACK:
			//Sys_SendLoopPacket (sock, length, data, to);
			//return;

		case NA_BROADCAST:
		case NA_IP:
			socket = ip_socket;
			if(!socket)
				return;
			break;

		default:
			Com_Error(ERR_FATAL, "Sys_SendPacket: bad address type");
	}
	//else if (to.type == NA_IPX)
	//{
	//	net_socket = ipx_sockets[sock];
	//	if (!net_socket)
	//		return;
	//}
	//else if (to.type == NA_BROADCAST_IPX)
	//{
	//	net_socket = ipx_sockets[sock];
	//	if (!net_socket)
	//		return;
	//}

	Sys_NetadrToSockadr(to, &addr);

	ret = sendto(socket, (const char*)&msg._data[0], msg.getCurSizeInBytes(), 0, &addr, sizeof(addr));
	if(ret == -1)
	{
		int err = WSAGetLastError();

		// wouldblock is silent
		if(err == WSAEWOULDBLOCK)
			return;

		// some PPP links dont allow broadcasts
		//if ((err == WSAEADDRNOTAVAIL) && ((to.type == NA_BROADCAST) || (to.type == NA_BROADCAST_IPX)))
		//	return;

		//if((bool)dedicated->getValue())	// let dedicated servers continue after errors
		//{
		//	Com_Printf ("NET_SendPacket ERROR: %s to %s\n", Sys_NetErrorString(), Sys_AdrToString (to));
		//}
		//else
		{
			if(err == WSAEADDRNOTAVAIL)
			{
				Com_DPrintf("Sys_SendPacket: '%s' : '%s'\n", Sys_NetErrorString(), Sys_AdrToString(to));
			}
			else
			{
				Com_Error(ERR_DROP, "Sys_SendPacket ERROR: '%s' to '%s'\n", Sys_NetErrorString(), Sys_AdrToString(to));
			}
		}
	}
}




/*
====================
Sys_Socket
====================
*/
static int 	Sys_IPSocket(const char *net_interface, int port)
{
	int					newsocket;
	struct sockaddr_in	address;
	bool			_true = true;
	int					i = 1;
	int					err;

	if((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		err = WSAGetLastError();
		if(err != WSAEAFNOSUPPORT)
			Com_Printf("WARNING: Sys_IPSocket: socket: %s", Sys_NetErrorString());
		return 0;
	}

	// make it non-blocking
	if(ioctlsocket (newsocket, FIONBIO, (u_long*)&_true) == -1)
	{
		Com_Printf("WARNING: Sys_IPSocket: ioctl FIONBIO: %s\n", Sys_NetErrorString());
		return 0;
	}

	// make it broadcast capable
	if(setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) == -1)
	{
		Com_Printf("WARNING: Sys_IPSocket: setsockopt SO_BROADCAST: %s\n", Sys_NetErrorString());
		return 0;
	}

	if(!net_interface || !net_interface[0] || !stricmp(net_interface, "localhost"))
		address.sin_addr.s_addr = INADDR_ANY;
	else
		Sys_StringToSockaddr(net_interface, (struct sockaddr *)&address);

	if(port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	address.sin_family = AF_INET;

	if(bind (newsocket, (const sockaddr*)&address, sizeof(address)) == -1)
	{
		Com_Printf("WARNING: Sys_IPSocket: bind: %s\n", Sys_NetErrorString());
		closesocket(newsocket);
		return 0;
	}

	return newsocket;
}


/*
====================
Sys_OpenIP
====================
*/
/*
static void	Sys_OpenIP()
{
	cvar_t	*ip;
	int		port;
	int		dedicated;

	ip = Cvar_Get("ip", "localhost", CVAR_NOSET);

	dedicated = Cvar_VariableInteger("dedicated");

	if (!ip_sockets[NS_SERVER])
	{
		port = Cvar_Get("ip_hostport", "0", CVAR_NOSET)->getInteger;
		if (!port)
		{
			port = Cvar_Get("hostport", "0", CVAR_NOSET)->getInteger;
			if (!port)
			{
				port = Cvar_Get("port", va("%i", PORT_SERVER), CVAR_NOSET)->getInteger;
			}
		}
		ip_sockets[NS_SERVER] = Sys_IPSocket (ip->string, port);
		if (!ip_sockets[NS_SERVER] && dedicated)
			Com_Error (ERR_FATAL, "Couldn't allocate dedicated server IP port");
	}


	// dedicated servers don't need client ports
	if (dedicated)
		return;

	if (!ip_sockets[NS_CLIENT])
	{
		port = Cvar_Get("ip_clientport", "0", CVAR_NOSET)->getInteger;
		if (!port)
		{
			port = Cvar_Get("clientport", va("%i", PORT_CLIENT), CVAR_NOSET)->getInteger;
			if (!port)
				port = PORT_ANY;
		}
		ip_sockets[NS_CLIENT] = Sys_IPSocket (ip->string, port);
		if (!ip_sockets[NS_CLIENT])
			ip_sockets[NS_CLIENT] = Sys_IPSocket (ip->string, PORT_ANY);
	}
}
*/

/*
====================
IPX_Socket
====================
*/
/*
static int	Sys_IPXSocket (int port)
{
	int					newsocket;
	struct sockaddr_ipx	address;
	int					_true = 1;
	int					err;

	if ((newsocket = socket (PF_IPX, SOCK_DGRAM, NSPROTO_IPX)) == -1)
	{
		err = WSAGetLastError();
		if (err != WSAEAFNOSUPPORT)
			Com_Printf ("WARNING: IPX_Socket: socket: %s\n", NET_ErrorString());
		return 0;
	}

	// make it non-blocking
	if (ioctlsocket (newsocket, FIONBIO, &_true) == -1)
	{
		Com_Printf ("WARNING: IPX_Socket: ioctl FIONBIO: %s\n", NET_ErrorString());
		return 0;
	}

	// make it broadcast capable
	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&_true, sizeof(_true)) == -1)
	{
		Com_Printf ("WARNING: IPX_Socket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString());
		return 0;
	}

	address.sa_family = AF_IPX;
	memset (address.sa_netnum, 0, 4);
	memset (address.sa_nodenum, 0, 6);
	if (port == PORT_ANY)
		address.sa_socket = 0;
	else
		address.sa_socket = htons((short)port);

	if( bind (newsocket, (void *)&address, sizeof(address)) == -1)
	{
		Com_Printf ("WARNING: IPX_Socket: bind: %s\n", NET_ErrorString());
		closesocket (newsocket);
		return 0;
	}

	return newsocket;
}
*/

/*
====================
NET_OpenIPX
====================
*/
/*
static void	Sys_OpenIPX (void)
{

	int		port;
	int		dedicated;

	dedicated = Cvar_VariableValue ("dedicated");

	if (!ipx_sockets[NS_SERVER])
	{
		port = Cvar_Get("ipx_hostport", "0", CVAR_NOSET)->value;
		if (!port)
		{
			port = Cvar_Get("hostport", "0", CVAR_NOSET)->value;
			if (!port)
			{
				port = Cvar_Get("port", va("%i", PORT_SERVER), CVAR_NOSET)->value;
			}
		}
		ipx_sockets[NS_SERVER] = NET_IPXSocket (port);
	}

	// dedicated servers don't need client ports
	if (dedicated)
		return;

	if (!ipx_sockets[NS_CLIENT])
	{
		port = Cvar_Get("ipx_clientport", "0", CVAR_NOSET)->value;
		if (!port)
		{
			port = Cvar_Get("clientport", va("%i", PORT_CLIENT), CVAR_NOSET)->value;
			if (!port)
				port = PORT_ANY;
		}
		ipx_sockets[NS_CLIENT] = NET_IPXSocket (port);
		if (!ipx_sockets[NS_CLIENT])
			ipx_sockets[NS_CLIENT] = NET_IPXSocket (PORT_ANY);
	}
}
*/


/*
====================
NET_Config

A single player game will only use the loopback code
====================
*/
static WSADATA	winsockdata;

void	Sys_InitNet(bool server)
{
	Com_Printf("------- Sys_InitNet -------\n");

	WORD wVersionRequested;
	wVersionRequested = MAKEWORD(1, 1);
	int r = WSAStartup(MAKEWORD(1, 1), &winsockdata);

	if(r)
		Com_Error(ERR_FATAL,"Winsock initialization failed.");

	Com_Printf("Winsock Initialized\n");

	cvar_t  *port, *ip;
	
	port    = Cvar_Get("port",      va("%i", PORT_SERVER),  CVAR_INIT);
	ip      = Cvar_Get("ip",        "localhost",            CVAR_INIT);

	if(server)
	{
		int     i = 0;
		
		for(i=0; i<4; i++)
		{
			ip_socket = Sys_IPSocket(ip->getString(), port->getInteger());

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
			ip_socket = Sys_IPSocket(ip->getString(), PORT_ANY);
	}
}


// sleeps msec or until net socket is ready
void 	Sys_SleepNet(int msec)
{
    	struct timeval timeout;
	fd_set	fdset;
	extern cvar_t *dedicated;

	if(!ip_socket || (!dedicated || !dedicated->getValue()))
		return; // we're not a server, just run full speed

	FD_ZERO(&fdset);
	FD_SET((uint_t)ip_socket, &fdset); // network socket
	timeout.tv_sec = msec/1000;
	timeout.tv_usec = (msec%1000)*1000;
	select(ip_socket+1, &fdset, NULL, NULL, &timeout);
}


/*
====================
NET_Shutdown
====================
*/
void	Sys_ShutdownNet()
{
	// shut down any existing sockets
	if(ip_socket)
	{
		close(ip_socket);
		ip_socket = 0;
	}
	
	WSACleanup ();
}


/*
====================
NET_ErrorString
====================
*/
char*	Sys_NetErrorString()
{
	int		code;

	code = WSAGetLastError();
	switch(code)
	{
	case WSAEINTR: return "WSAEINTR";
	case WSAEBADF: return "WSAEBADF";
	case WSAEACCES: return "WSAEACCES";
	case WSAEDISCON: return "WSAEDISCON";
	case WSAEFAULT: return "WSAEFAULT";
	case WSAEINVAL: return "WSAEINVAL";
	case WSAEMFILE: return "WSAEMFILE";
	case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
	case WSAEINPROGRESS: return "WSAEINPROGRESS";
	case WSAEALREADY: return "WSAEALREADY";
	case WSAENOTSOCK: return "WSAENOTSOCK";
	case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
	case WSAEMSGSIZE: return "WSAEMSGSIZE";
	case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
	case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
	case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
	case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
	case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
	case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
	case WSAEADDRINUSE: return "WSAEADDRINUSE";
	case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
	case WSAENETDOWN: return "WSAENETDOWN";
	case WSAENETUNREACH: return "WSAENETUNREACH";
	case WSAENETRESET: return "WSAENETRESET";
	case WSAECONNABORTED: return "WSWSAECONNABORTEDAEINTR";
	case WSAECONNRESET: return "WSAECONNRESET";
	case WSAENOBUFS: return "WSAENOBUFS";
	case WSAEISCONN: return "WSAEISCONN";
	case WSAENOTCONN: return "WSAENOTCONN";
	case WSAESHUTDOWN: return "WSAESHUTDOWN";
	case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
	case WSAETIMEDOUT: return "WSAETIMEDOUT";
	case WSAECONNREFUSED: return "WSAECONNREFUSED";
	case WSAELOOP: return "WSAELOOP";
	case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
	case WSASYSNOTREADY: return "WSASYSNOTREADY";
	case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
	case WSANOTINITIALISED: return "WSANOTINITIALISED";
	case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
	case WSATRY_AGAIN: return "WSATRY_AGAIN";
	case WSANO_RECOVERY: return "WSANO_RECOVERY";
	case WSANO_DATA: return "WSANO_DATA";
	default: return "NO ERROR";
	}
}

#endif // from -- #if ! defined(BSPCOMPILER_ONLY)






char	findbase[MAX_OSPATH];
char	findpath[MAX_OSPATH];
int	findhandle;

static bool	Sys_CompareAttributes(unsigned found, unsigned musthave, unsigned canthave)
{
	if ( ( found & _A_RDONLY ) && ( canthave & SFF_RDONLY ) )
		return false;
	if ( ( found & _A_HIDDEN ) && ( canthave & SFF_HIDDEN ) )
		return false;
	if ( ( found & _A_SYSTEM ) && ( canthave & SFF_SYSTEM ) )
		return false;
	if ( ( found & _A_SUBDIR ) && ( canthave & SFF_SUBDIR ) )
		return false;
	if ( ( found & _A_ARCH ) && ( canthave & SFF_ARCH ) )
		return false;

	if ( ( musthave & SFF_RDONLY ) && !( found & _A_RDONLY ) )
		return false;
	if ( ( musthave & SFF_HIDDEN ) && !( found & _A_HIDDEN ) )
		return false;
	if ( ( musthave & SFF_SYSTEM ) && !( found & _A_SYSTEM ) )
		return false;
	if ( ( musthave & SFF_SUBDIR ) && !( found & _A_SUBDIR ) )
		return false;
	if ( ( musthave & SFF_ARCH ) && !( found & _A_ARCH ) )
		return false;

	return true;
}

std::string	Sys_FindFirst(const std::string &path, unsigned musthave, unsigned canthave)
{
	struct _finddata_t findinfo;

	if (findhandle)
		Sys_Error ("Sys_BeginFind without close");
	findhandle = 0;

	Com_FilePath ((char*)path.c_str(), findbase);
	findhandle = _findfirst (path.c_str(), &findinfo);
	if (findhandle == -1)
		return "";
		
	if (!Sys_CompareAttributes(findinfo.attrib, musthave, canthave))
		return "";
		
	Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
	return findpath;
}

std::string	Sys_FindNext(unsigned musthave, unsigned canthave)
{
	struct _finddata_t findinfo;

	if (findhandle == -1)
		return "";
		
	if (_findnext (findhandle, &findinfo) == -1)
		return "";
		
	if (!Sys_CompareAttributes(findinfo.attrib, musthave, canthave))
		return "";

	Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
	return findpath;
}

void	Sys_FindClose()
{
	if (findhandle != -1)
		_findclose (findhandle);
		
	findhandle = 0;
}






static char	console_text[256];
static int	console_textlen;

/*
================
Sys_ConsoleInput
================
*/
char*	Sys_ConsoleInput()
{
	INPUT_RECORD	recs[1024];
	DWORD		dummy;
	DWORD		ch, numread, numevents;

	if(!dedicated || !dedicated->getValue())
		return NULL;

	for(;;)
	{
		if (!GetNumberOfConsoleInputEvents (hinput, &numevents))
			Sys_Error ("Error getting # of console events");

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput(hinput, recs, 1, &numread))
			Sys_Error ("Error reading console input");

		if (numread != 1)
			Sys_Error ("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
					case '\r':
						WriteFile(houtput, "\r\n", 2, &dummy, NULL);	

						if (console_textlen)
						{
							console_text[console_textlen] = 0;
							console_textlen = 0;
							return console_text;
						}
						break;

					case '\b':
						if (console_textlen)
						{
							console_textlen--;
							WriteFile(houtput, "\b \b", 3, &dummy, NULL);	
						}
						break;

					default:
						if (ch >= ' ')
						{
							if (console_textlen < (int)sizeof(console_text)-2)
							{
								WriteFile(houtput, &ch, 1, &dummy, NULL);	
								console_text[console_textlen] = ch;
								console_textlen++;
							}
						}

						break;

				}
			}
		}
	}

	return NULL;
}


/*
================
Sys_ConsoleOutput

Print text to the dedicated console
================
*/
void Sys_ConsoleOutput (char *string)
{
	DWORD		dummy;
	char	text[256];

    if (!dedicated || !dedicated->getValue())
    {
        printf("%s", string);
        return;
    }

	if (console_textlen)
	{
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen+1] = '\r';
		text[console_textlen+2] = 0;
		WriteFile(houtput, text, console_textlen+2, &dummy, NULL);
	}

	WriteFile(houtput, string, strlen(string), &dummy, NULL);

	if (console_textlen)
		WriteFile(houtput, console_text, console_textlen, &dummy, NULL);
}


void	Sys_PushEvents()
{
	//
	// push SE_KEY events
	//
	MSG	msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if(!GetMessage(&msg, NULL, 0, 0))
			Com_Shutdown();
			
		sys_msg_time = msg.time;
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

#if ! defined(BSPCOMPILER_ONLY)
	//
	// push SE_PACKET events
	//
	netadr_t        adr;
	byte            packet[MAX_PACKETLEN];
	int             length;
	
	byte*           data;
	int             data_size;
	
	while((length = Sys_GetPacket(packet, sizeof(packet), adr)) >= 0)
	{
		data_size = sizeof(adr) + length;
		data = new byte[data_size];

		memcpy(data, &adr, sizeof(adr));
		memcpy((byte*)data + sizeof(adr), packet, length);

		Com_PushEvent(SE_PACKET, -1, -1, -1, data, data_size);
	}
#endif
}


char*	Sys_GetClipboardData()
{
	char *data = NULL;
	char *cliptext;

	if(OpenClipboard(NULL) != 0)
	{
		HANDLE hClipboardData;

		if((hClipboardData = GetClipboardData(CF_TEXT)) != 0)
		{
			if ( ( cliptext = (char*)GlobalLock( hClipboardData ) ) != 0 )
			{
				data = (char*) malloc( GlobalSize( hClipboardData ) + 1 );
				strcpy( data, cliptext );
				GlobalUnlock( hClipboardData );
			}
		}
		CloseClipboard();
	}
	return data;
}

/*
==============================================================================

 WINDOWS CRAP

==============================================================================
*/

/*
=================
Sys_AppActivate
=================
*/
void	Sys_AppActivate()
{
#if !defined(DEDICATED_ONLY) && !defined(BSPCOMPILER_ONLY)
	ShowWindow(cl_hwnd, SW_RESTORE);
	SetForegroundWindow(cl_hwnd);
#endif
}



/*
=================
Sys_GetAPI

Loads the game dll or ui dll or whatever api dll you designed
=================
*/
void*	Sys_GetAPI(const char *api_name, const char *api_main, void *api_parms, void **api_handle)
{
	// ADJ - major rewrite based off linux version
	std::string	name;
	std::string	name_modul;
	
	std::string	path = "";
	void*	(*api) (void *) = NULL;

	name_modul = api_name;
	name_modul += ".dll";

	if(*api_handle)
		Com_Error(ERR_FATAL, "Sys_GetAPI without Sys_UnloadAPI");

	Com_Printf("------- Loading %s -------\n", name_modul.c_str());

	name = VFS_FullPath(name_modul);

	*api_handle = LoadLibrary (name.c_str());
		
	if(*api_handle)
	{
		api = (void*(*)(void*))GetProcAddress((HINSTANCE)*api_handle, api_main);
	}
	else
	{
		//DWORD = GetLastError();
		//Com_Printf("no success: %s\n", dlerror());
	}

	if(!api)
	{
		Sys_UnloadAPI(api_handle);		
		return NULL;
	}

	return api(api_parms);
}


void 	Sys_UnloadAPI(void **api_handle)
{
	if(!FreeLibrary((HINSTANCE)*api_handle))
		Com_Error(ERR_FATAL, "Sys_UnloadAPI: FreeLibrary failed");

	*api_handle = NULL;
}

static void	ParseCommandLine(LPSTR lpCmdLine)
{
	argc = 1;
	argv[0] = "exe";

	while (*lpCmdLine && (argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}
			
		}
	}

}


HINSTANCE	global_hInstance;

int WINAPI	WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG		msg;
	int		time, oldtime, newtime;

	/* previous instances do not exist in Win32 */
	if(hPrevInstance)
		return 0;

	global_hInstance = hInstance;

	ParseCommandLine (lpCmdLine);

	Com_Init(argc, argv);
	
	oldtime = Sys_Milliseconds ();

	/* main window message loop */
	while(true)
	{
		// if at a full screen console, don't update unless needed
		if(Minimized || (dedicated && dedicated->getValue()))
		{
			Sleep(1);
		}

		while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if(!GetMessage(&msg, NULL, 0, 0))
				Com_Shutdown();
				
			sys_msg_time = msg.time;
			
			TranslateMessage(&msg);
   			DispatchMessage(&msg);
		}

		do
		{
			newtime = Sys_Milliseconds();
			time = newtime - oldtime;
		} while(time < 1);
//			Con_Printf("time:%5.2f - %5.2f = %5.2f\n", newtime, oldtime, time);
		
		Com_Frame(time);

		oldtime = newtime;
	}

	// never gets here
    	return TRUE;
}
