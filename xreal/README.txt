Quake III Arena GPL source release - XreaL version - http://sourceforge.net/projects/xreal
==========================================================================================

This file contains the following sections:

GENERAL NOTES
GETTING THE SOURCE CODE
COMPILING ON WIN32
COMPILING ON GNU/LINUX
COMPILING ON MAC
LICENSE


GENERAL NOTES
=============

A short summary of the file layout:

base/					XreaL media directory ( models, textures, sounds etc. )
gtkradiant/				XreaL configured GtkRadiant editor work dir
code/			   		XreaL source code ( renderer, game code, OS layer etc. )
code/bspc				bot routes compiler source code
code/common				framework source code for command line tools like xmap
code/xmap				map compiler ( .map -> .bsp )
code/gtkradiant			GtkRadiant editor source based off GPL release on 17th February 2006

GETTING THE SOURCE CODE
=======================

This project's SourceForge.net Subversion repository can be checked out through SVN with the following instruction set: 

svn co https://svn.sourceforge.net/svnroot/xreal/trunk/xreal xreal


COMPILING ON WIN32 WITH VISUAL STUDIO
=====================================

VC7 / Visual C++ 2003 project files are provided:
code/xreal.sln
code/bspc/bspc.sln
code/xmap/xmap.sln


COMPILING ON WIN32 WITH MINGW
=============================

1. Download and install MinGW from http://www.mingw.org/.
2. Download http://www.libsdl.org/extras/win32/common/directx-devel.tar.gz
     and untar it into your MinGW directory (usually C:\MinGW).
3. Download http://oss.sgi.com/projects/ogl-sample/ABI/glext.h
	and copy it over the existing C:\MingW\include\GL\glext.h.
4. Download and install Python from http://www.python.org/.
5. Download and install SCons from http://www.scons.org/.
6. Download libxml2 from http://www.xmlsoft.org/.
7. Configure libxml2:
	in libxml2-2.6.26\win32
	>cscript configure.js ftp=no http=no iconv=no compiler=mingw prefix=C:\MingW bindir=C:\MingW\bin incdir=C:\MingW\include libdir=C:\MingW\lib sodir=C:\MingW\lib
8. Compile libxml2:
	>mingw32-make -f Makefile.mingw
9. Install libxml2:
	>mingw32-make -f Makefile.mingw install
10. Compile XreaL:
	>scons arch=win32-mingw



COMPILING ON GNU/LINUX
==================

The new build system is using SCons. 
Make sure you have the X Direct Graphics Access and X Video Mode extensions headers for your X11.
Just type "scons" in the source toplevel dir xreal/.
You can provide SCons options for compiling. Use "scons -h" to list those options.


COMPILING ON MAC
================

project file for OSX compile is in code/macosx/Quake3.pbproj


LICENSE
=======

See COPYING.txt for the GNU GENERAL PUBLIC LICENSE

Some source code in this release is not covered by the GPL:

IO on .zip files using portions of zlib
-----------------------------------------------------------------------------
lines	file(s)
4299	code/qcommon/unzip.c
4546	libs/pak/unzip.cpp
Copyright (C) 1998 Gilles Vollant
zlib is Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

MD4 Message-Digest Algorithm
-----------------------------------------------------------------------------
lines	file(s)
299		code/qcommon/md4.c
277		common/md4.c
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All rights reserved.

License to copy and use this software is granted provided that it is identified 
as the <93>RSA Data Security, Inc. MD4 Message-Digest Algorithm<94> in all mater
ial mentioning or referencing this software or this function.
License is also granted to make and use derivative works provided that such work
s are identified as <93>derived from the RSA Data Security, Inc. MD4 Message-Dig
est Algorithm<94> in all material mentioning or referencing the derived work.
RSA Data Security, Inc. makes no representations concerning either the merchanta
bility of this software or the suitability of this software for any particular p
urpose. It is provided <93>as is<94> without express or implied warranty of any 
kind.

checksums are used to validate pak files

standard C library replacement routines
-----------------------------------------------------------------------------
lines	file(s)
1324	code/game/bg_lib.c
Copyright (c) 1992, 1993
The Regents of the University of California. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:
     This product includes software developed by the University of
     California, Berkeley and its contributors.
4. Neither the name of the University nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

ADPCM coder/decoder
-----------------------------------------------------------------------------
lines	file(s)
330		code/client/snd_adpcm.c
Copyright 1992 by Stichting Mathematisch Centrum, Amsterdam, The
Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.

STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

JPEG library
-----------------------------------------------------------------------------
code/jpeg-6
Copyright (C) 1991-1995, Thomas G. Lane

Permission is hereby granted to use, copy, modify, and distribute this
software (or portions thereof) for any purpose, without fee, subject to these
conditions:
(1) If any part of the source code for this software is distributed, then this
README file must be included, with this copyright and no-warranty notice
unaltered; and any additions, deletions, or changes to the original files
must be clearly indicated in accompanying documentation.
(2) If only executable code is distributed, then the accompanying
documentation must state that "this software is based in part on the work of
the Independent JPEG Group".
(3) Permission for use of this software is granted only if the user accepts
full responsibility for any undesirable consequences; the authors accept
NO LIABILITY for damages of any kind.

These conditions apply to any software derived from or based on the IJG code,
not just to the unmodified library.  If you use our work, you ought to
acknowledge us.

NOTE: unfortunately the README that came with our copy of the library has
been lost, so the one from release 6b is included instead. There are a few
'glue type' modifications to the library to make it easier to use from
the engine, but otherwise the dependency can be easily cleaned up to a
better release of the library.

Lua library
-----------------------------------------------------------------------------
code/lua-5.1
Copyright (C) 1994-2006 Lua.org, PUC-Rio.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

PNG library
-----------------------------------------------------------------------------
code/png
Copyright (c) 2004, 2006 Glenn Randers-Pehrson

Permission is hereby granted to use, copy, modify, and distribute this
source code, or portions hereof, for any purpose, without fee, subject
to the following restrictions:

1. The origin of this source code must not be misrepresented.

2. Altered versions must be plainly marked as such and must not
   be misrepresented as being the original source.

3. This Copyright notice may not be removed or altered from any
   source or altered source distribution.

The Contributing Authors and Group 42, Inc. specifically permit, without
fee, and encourage the use of this source code as a component to
supporting the PNG file format in commercial products.  If you use this
source code in a product, acknowledgment is not required but would be
appreciated.

The PNG Reference Library is supplied "AS IS".  The Contributing Authors
and Group 42, Inc. disclaim all warranties, expressed or implied,
including, without limitation, the warranties of merchantability and of
fitness for any purpose.  The Contributing Authors and Group 42, Inc.
assume no liability for direct, indirect, incidental, special, exemplary,
or consequential damages, which may result from the use of the PNG
Reference Library, even if advised of the possibility of such damage.
