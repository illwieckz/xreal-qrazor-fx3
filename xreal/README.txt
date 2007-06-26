XreaL Readme - http://sourceforge.net/projects/xreal
====================================================

This file contains the following sections:

GENERAL NOTES
GETTING THE SOURCE CODE AND MEDIA
COMPILING ON WIN32
COMPILING ON GNU/LINUX
COMPILING ON MAC
LICENSE


GENERAL NOTES
=============

A short summary of the file layout:

XreaL/base/					XreaL media directory ( models, textures, sounds etc. )
XreaL/gtkradiant/			XreaL configured GtkRadiant editor work dir
XreaL/code/			   		XreaL source code ( renderer, game code, OS layer etc. )
XreaL/code/bspc				bot routes compiler source code
XreaL/code/common			framework source code for command line tools like xmap
XreaL/code/xmap				map compiler ( .map -> .bsp )
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
	
7. Add necessary include Directories in VS 2005 under Tools -> Options... -> Project and Solutions -> VC++ Directories:
	example:
	C:\Program Files\Microsoft DirectX SDK (April 2007)\Include
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

8. Add necessary lib Directories in VS 2005 under Tools -> Options... -> Project and Solutions -> VC++ Directories:
	example:
	C:\Program Files\Microsoft DirectX SDK (April 2007)\Lib\x86
	C:\GTK\lib

9. Use the VC8 / Visual C++ 2005 solutions to compile what you need:
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
6. Add the Python installation directory to the system variable %PATH%
7. Download and install Gtk+ 2.10.7 development environment from http://gladewin32.sourceforge.net
8. Set the system variable: PKG_CONFIG_PATH to %GTK_BASEPATH%\lib\pkgconfig
9. Compile XreaL:
	>scons arch=win32-mingw


COMPILING ON GNU/LINUX
======================

The new build system is using SCons. 
Make sure you have the X Direct Graphics Access and X Video Mode extensions headers for your X11.
Just type "scons" in the source toplevel dir xreal/.
You can provide SCons options for compiling. Use "scons -h" to list those options.


COMPILING ON MAC
================

project file for OSX compile is in code/macosx/Quake3.pbproj


LICENSE
=======

See COPYING.txt for all the legal stuff.
