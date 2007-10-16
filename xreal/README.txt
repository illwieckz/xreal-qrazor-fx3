XreaL Readme - http://sourceforge.net/projects/xreal
====================================================

This file contains the following sections:

GENERAL NOTES
GETTING THE SOURCE CODE AND MEDIA
COMPILING ON WIN32
COMPILING ON GNU/LINUX
COMPILING ON MAC
LICENSE
USING HTTP/FTP DOWNLOAD SUPPORT
USING R1ADMIN


GENERAL NOTES
=============

A short summary of the file layout:

XreaL/base/			XreaL media directory ( models, textures, sounds etc. )
XreaL/gtkradiant/		XreaL configured GtkRadiant editor work dir
XreaL/code/			XreaL source code ( renderer, game code, OS layer etc. )
XreaL/code/bspc			bot routes compiler source code
XreaL/code/common		framework source code for command line tools like xmap
XreaL/code/xmap			map compiler ( .map -> .bsp )
XreaL/code/xmass		master server
XreaL/code/gtkradiant		GtkRadiant editor source based off GPL release on 17th February 2006


GETTING THE SOURCE CODE AND MEDIA
=================================

This project's SourceForge.net Subversion repository can be checked out through SVN with the following instruction set: 

svn co https://svn.sourceforge.net/svnroot/xreal/trunk/xreal XreaL


COMPILING ON WIN32 WITH VISUAL STUDIO 2005
==========================================

1. Download and install the Visual C++ 2005 Express Edition.
2. Download and install the Platform SDK.
3. Download and install the DirectX SDK.
4. Download http://oss.sgi.com/projects/ogl-sample/ABI/glext.h
	and copy it over the existing $(PlatformSDK)/include/GL/glext.h.
5. Download and install Gtk+ 2.10.11 development environment from http://gladewin32.sourceforge.net/.
6. Download http://xreal.varcache.org/STLport-5.0.2.7z
	and extract it to XreaL/code/.
7. Download libcURL from http://curl.hoxt.com/download/libcurl-7.15.5-win32-msvc.zip
	and extract it to C:\libcURL
8. Download and install the OpenAL SDK from http://www.openal.org.
	
9. Add necessary include Directories in VS 2005 under Tools -> Options... -> Project and Solutions -> VC++ Directories:
	example:
	C:\Program Files\Microsoft Platform SDK\include
	C:\Program Files\Microsoft Platform SDK\include\mfc
	C:\Program Files\Microsoft DirectX SDK (April 2007)\include
	C:\Program Files\OpenAL 1.1 SDK\include
	C:\GTK\include
	C:\GTK\include\libxml2
	C:\GTK\include\glib-2.0
	C:\GTK\lib\glib-2.0\include
	C:\GTK\include\gtk-2.0
	C:\GTK\lib\gtk-2.0\include
	C:\GTK\include\cairo
	C:\GTK\include\pango-1.0
	C:\GTK\include\atk-1.0
	C:\GTK\include\gtkglext-1.0
	C:\GTK\lib\gtkglext-1.0\include
	C:\libcURL\include

10. Add necessary lib Directories in VS 2005 under Tools -> Options... -> Project and Solutions -> VC++ Directories:
	example:
	C:\Program Files\Microsoft Platform SDK\lib
	C:\Program Files\Microsoft DirectX SDK (April 2007)\lib\x86
	C:\Program Files\OpenAL 1.1 SDK\lib\Win32
	C:\GTK\lib
	C:\libcURL

11. Use the VC8 / Visual C++ 2005 solutions to compile what you need:
	XreaL/code/xreal.sln
	XreaL/code/gtkradiant/GtkRadiant.sln
	XreaL/code/xmap/xmap.sln
	XreaL/code/bspc/bspc.sln


COMPILING ON WIN32 WITH MINGW
=============================

1. Download and install MinGW from http://www.mingw.org/.
2. Download http://www.libsdl.org/extras/win32/common/directx-devel.tar.gz
     and untar it into your MinGW directory (usually C:\MinGW).
3. Download http://oss.sgi.com/projects/ogl-sample/ABI/glext.h
	and copy it over the existing C:\MingW\include\GL\glext.h.
4. Download and install Python from http://www.python.org/.
5. Download and install SCons from http://www.scons.org/.
6. Download and install libcURL from http://curl.haxx.se/.
7. Download and install the OpenAL SDK from http://www.openal.org.
8. Add the Python installation directory to the system variable %PATH%
9. Download and install Gtk+ 2.10.7 development environment from http://gladewin32.sourceforge.net
10. Set the system variable: PKG_CONFIG_PATH to %GTK_BASEPATH%\lib\pkgconfig
11. Compile XreaL:
	>scons arch=win32-mingw


COMPILING ON GNU/LINUX
======================

Make sure you have OpenAL headers and libraries and the X Direct Graphics Access and
X Video Mode extensions headers for your X11 or if using the SDL backend the SDL
headers and libraries.

Compile XreaL for x86 processers:
	>scons arch=linux-i386
Compile XreaL for x86_64 processers:
	>scons arch=linux-x86_64


COMPILING ON MAC OS X
================

Make sure you have libcURL and the SDL framework installed.

Download http://oss.sgi.com/projects/ogl-sample/ABI/glext.h
	and copy it to XreaL/code/renderer

Use the included XCode project to compile XreaL and friends for both PPC & Intel Macs.
The XCode project is located here:
	>code/unix/MacSupport/XreaL.xcodeprj


LICENSE
=======

See COPYING.txt for all the legal stuff.


USING HTTP/FTP DOWNLOAD SUPPORT
===============================

You can enable redirected downloads on your server by using the 'sets'
command to put the sv_dlURL cvar into your SERVERINFO string and
ensure sv_allowDownloads is set to 1.
 
sv_dlURL is the base of the URL that contains your custom .pk3 files
the client will append both fs_game and the filename to the end of
this value.  For example, if you have sv_dlURL set to
"http://xreal.sourceforge.net/", fs_game is "base", and the client is
missing "test.pk3", it will attempt to download from the URL
"http://xreal.sourceforge.net/base/test.pk3"

sv_allowDownload's value is now a bitmask made up of the following
flags:
    1 - ENABLE
    2 - do not use HTTP/FTP downloads
    4 - do not use UDP downloads
    8 - do not ask the client to disconnect when using HTTP/FTP

Server operators who are concerned about potential "leeching" from their
HTTP servers from other XreaL servers can make use of the HTTP_REFERER
that XreaL sets which is "XreaL://{SERVER_IP}:{SERVER_PORT}".  For,
example, Apache's mod_rewrite can restrict access based on HTTP_REFERER. 

Simply setting cl_allowDownload to 1 will enable HTTP/FTP downloads on 
the clients side assuming XreaL was compiled with USE_CURL=1.
Like sv_allowDownload, cl_allowDownload also uses a bitmask value
supporting the following flags:
    1 - ENABLE
    2 - do not use HTTP/FTP downloads
    4 - do not use UDP downloads


USING R1ADMIN
=============

r1admin comes builtin with XreaL. To use it, you must have an admin
account created. This is done by using the addaccount command. You can
also remove accounts by using the removeaccount command. To use those
commands, you must have access to the server console.

Once an account is created you can login yourself in using the login
command. When your logged in to the server, you can then use the r1admin
commands:

	@kick <player/id> - kick a player
	@ban <player/id> - ban a player
	@mute <player/id> - mute a player
	@map <map> - change the map
	@map_restart - restart the current map
	@ip <player/id> - show the players ip
	@listip - list all the players and their ips
	@removeip <ip/mask> - remove an ip from the banlist
	@status - show the current players on the server along with their ip
	@search <[name|ip] - player/ip> - search for the player name or ip

