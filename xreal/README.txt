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


COMPILING ON WIN32 WITH VISUAL STUDIO
=====================================

VC8 / Visual C++ 2005 project files are provided:
XreaL/code/xreal.sln
XreaL/code/bspc/bspc.sln
XreaL/code/xmap/xmap.sln


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
