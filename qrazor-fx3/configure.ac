dnl stick the revision info into the resulting configure script
AC_REVISION($Revision$) dnl
AC_PREREQ(2.50)

AC_INIT(Makefile.am)
#dnl This is the only place where the package version appears
AM_INIT_AUTOMAKE(qrazor-fx, 0.3.5)

#AM_INIT_AUTOMAKE
#([foreign dist-bzip2 dist-zip])

#AC_CONFIG_AUX_DIR(.)
#AC_CONFIG_SRCDIR(src/sys_linux.c)
AM_CONFIG_HEADER(src/config.h)

dnl get the build and target hosts
#AC_CANONICAL_SYSTEM



dnl configure --enable-maintainer-mode gets some extra targets
#AM_MAINTAINER_MODE

dnl set some macros
AC_DEFINE_UNQUOTED(BUILDHOST, "${target_cpu}-${target_os}",
	[Set to the canonical name of the target machine])
AC_SUBST(PROGRAM)
AC_SUBST(VERSION)

dnl -------------------
dnl Checks for programs
dnl -------------------

AC_PROG_CC
AC_PROG_CXX

#AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
#AC_PROG_INSTALL

HAVE_MASM=""
AC_SUBST(HAVE_MASM)

AS="$CC"
ASFLAGS="\$(DEFS) \$(CFLAGS) \$(CXXFLAGS) \$(CPPFLAGS) \$(DEFAULT_INCLUDES) \$(INCLUDES)"
CCAS="$AS"
CCASFLAGS="$ASFLAGS"
AC_SUBST(AS)
AC_SUBST(ASFLAGS)
AC_SUBST(CCAS)
AC_SUBST(CCASFLAGS)

dnl --------------------
dnl Checks for libraries
dnl --------------------

dnl Check for OpenAL
OPENAL_LIBS=""
AC_CHECK_LIB(openal, alcOpenDevice,
		OPENAL_LIBS="-lopenal"
		HAVE_OPENAL=yes,
		HAVE_OPENAL=no)
if test "$HAVE_OPENAL" = yes; then
	AC_CHECK_HEADERS(AL/al.h AL/alc.h AL/alut.h, HAVE_OPENAL=yes, HAVE_OPENAL=no)
fi

if test "$HAVE_OPENAL" = no; then
	AC_MSG_ERROR(no OpenAL library development stuff found)
fi
AC_SUBST(HAVE_OPENAL)
AC_SUBST(OPENAL_LIBS)



dnl Check for zlib
Z_LIBS=""
AC_CHECK_LIB(z, deflate,
		Z_LIBS="-lz"
		HAVE_ZLIB=yes,
		HAVE_ZLIB=no)
if test "$HAVE_ZLIB" = yes; then
	AC_CHECK_HEADER(zlib.h, HAVE_ZLIB=yes, HAVE_ZLIB=no)
fi

if test "$HAVE_ZLIB" = no; then
	AC_MSG_ERROR(no ZLIB library development stuff found)
fi
AC_SUBST(HAVE_ZLIB)
AC_SUBST(Z_LIBS)


dnl Check for libjpeg
JPEG_LIBS=""
AC_CHECK_LIB(jpeg, jpeg_start_decompress,
		JPEG_LIBS="-ljpeg"
		HAVE_JPEG=yes,
		HAVE_JPEG=no)
if test "$HAVE_JPEG" = yes; then
	AC_CHECK_HEADER(jpeglib.h, HAVE_JPEG=yes, HAVE_JPEG=no)
fi

if test "$HAVE_JPEG" = no; then
	AC_MSG_ERROR(no JPEG library development stuff found)
fi
AC_SUBST(HAVE_JPEG)
AC_SUBST(JPEG_LIBS)


dnl Check for libpng
PNG_LIBS=""
AC_CHECK_LIB(png12, png_create_read_struct,
		PNG_LIBS="-lpng12"
		HAVE_PNG=yes,
		HAVE_PNG=no)
if test "$HAVE_PNG" = yes; then
	AC_CHECK_HEADER(png.h, HAVE_PNG=yes, HAVE_PNG=no)
fi

if test "$HAVE_PNG" = no; then
	AC_MSG_ERROR(no PNG library development stuff found)
fi
AC_SUBST(HAVE_PNG)
AC_SUBST(PNG_LIBS)


dnl Check for Open Dynamics Engine
ODE_LIBS=""
AC_CHECK_LIB(ode, dMassAdd,
		ODE_LIBS="-lode"
		HAVE_ODE=yes,
		HAVE_ODE=no)
if test "$HAVE_ODE" = yes; then
	AC_CHECK_HEADER(ode/ode.h, HAVE_ODE=yes, HAVE_ODE=no)
fi

if test "$HAVE_ODE" = no; then
	AC_MSG_ERROR(no ODE library development stuff found)
fi
AC_SUBST(HAVE_ODE)
AC_SUBST(ODE_LIBS)

dnl Check for Boost Spirit
#AC_CHECK_HEADERS(	boost/spirit/core.hpp \
#			boost/spirit/utility.hpp \
#			boost/spirit/symbols/symbols.hpp \
#			boost/spirit/tree/ast.hpp \
#			boost/spirit/utility/lists.hpp \
#			boost/spirit/utility/escape_char.hpp, HAVE_BOOST_SPIRIT=yes, HAVE_BOOST_SPIRIT=no)
#AC_CHECK_HEADERS(boost/spirit.hpp, HAVE_BOOST_SPIRIT=yes, HAVE_BOOST_SPIRIT=no)

#if test "$HAVE_BOOST_SPIRIT" = no; then
#	AC_MSG_ERROR(no Boost Spirit library development stuff found)
#fi
#AC_SUBST(HAVE_BOOST_SPIRIT)


dnl Check for X11
AC_PATH_XTRA
if test "x$no_x" = x; then
	HAVE_X=yes
	HAVE_XF86_VIDMODE=yes
	HAVE_XF86_DGA=yes
	XTRA_LIBS="-lX11"
	AC_CHECK_LIB(Xext, XShmQueryExtension,
		XTRA_LIBS="$XTRA_LIBS -lXext",
		HAVE_X=no,
		[ $X_LIBS $XTRA_LIBS $X_EXTRA_LIBS ]
	)
	if test "x$HAVE_X" = xyes; then
		#save_CPPFLAGS="$CPPFLAGS"
		#CPPFLAGS="$CPPFLAGS $X_CFLAGS"
		AC_CHECK_HEADER(X11/extensions/xf86vmode.h,
			dnl Make sure the library works
			AC_CHECK_LIB(Xxf86vm, XF86VidModeSwitchToMode,
				XTRA_LIBS="$XTRA_LIBS -lXxf86vm"
				AC_DEFINE(HAVE_XF86_VIDMODE, 1, [Define this if you have the xf86vmode extension and its Xxf86vm library.]),
				HAVE_XF86_VIDMODE=no,
				[$X_LIBS $XTRA_LIBS $X_EXTRA_LIBS]
			),
			HAVE_XF86_VIDMODE=no,
			[#include <X11/Xlib.h>]
		)
		#CPPFLAGS="$save_CPPFLAGS"
	fi
	if test "x$HAVE_X" = xyes; then
		#save_CPPFLAGS="$CPPFLAGS"
		#CPPFLAGS="$CPPFLAGS $X_CFLAGS"
		AC_CHECK_HEADER(X11/extensions/xf86dga.h,
			AC_CHECK_LIB(Xxf86dga, XF86DGAQueryVersion,
				XTRA_LIBS="$XTRA_LIBS -lXxf86dga"
				AC_DEFINE(HAVE_XF86_DGA, 1, [Define this if you have the xf86dga extension and its Xxf86dga library.]),
				HAVE_XF86_DGA=no,
				[$X_LIBS $XTRA_LIBS $X_EXTRA_LIBS]
			),
			HAVE_XF86_DGA=no,
			[#include <X11/Xlib.h>]
		)
		#CPPFLAGS="$save_CPPFLAGS"
	fi
	if test "x$HAVE_X" != xyes; then
		XTRA_LIBS=""
	fi
fi
AC_SUBST(HAVE_X)
AC_SUBST(HAVE_XF86_VIDMODE)
AC_SUBST(HAVE_XF86_DGA)
AC_SUBST(XTRA_LIBS)





#dnl Check for OpenGL
#AC_ARG_WITH(opengl,
#	[  --with-opengl=DIR       use OpenGL in DIR ],  
#	HAVE_OPENGL=$withval,
#	HAVE_OPENGL=auto)
#if test "x$HAVE_OPENGL" != xno; then
#	if test "x$HAVE_OPENGL" != xauto; then
#		OPENGL_CFLAGS="$OPENGL_CFLAGS -I$withval/include"
#		OPENGL_LIBS="$OPENGL_LIBS -L$withval/lib"
#	fi
#	save_CPPFLAGS="$CPPFLAGS"
#	CPPFLAGS="$CPPFLAGS $OPENGL_CFLAGS"
#	AC_CHECK_HEADER(GL/gl.h, HAVE_OPENGL=yes, HAVE_OPENGL=no)
#	CPPFLAGS="$save_CPPFLAGS"
#	if test "x$HAVE_OPENGL" = xyes; then
#		AC_CHECK_LIB(GL, glBegin,
#			OPENGL_LIBS="$OPENGL_LIBS -lGL"
#			HAVE_OPENGL=yes,
#			dnl add -lX11 and -lXext if the first test failed
#			dnl (for, e.g. FreeBSD)
#			unset ac_cv_lib_GL_glBegin
#			AC_CHECK_LIB(GL, glBegin,
#				OPENGL_LIBS="$OPENGL_LIBS -lX11 -lXext"
#				HAVE_OPENGL=yes,
#				HAVE_OPENGL=no,
#				[$OPENGL_LIBS -lX11 -lXext]
#			),
#			[$OPENGL_LIBS]
#		)
#	fi
#	if test "x$HAVE_OPENGL" = xyes; then
#		AC_CHECK_HEADER(GL/glext.h, HAVE_OPENGL_GLEXT=yes
#			AC_DEFINE(HAVE_OPENGL_GLEXT, 1, [Define this if you have GL/glext.h]),
#		HAVE_OPENGL_GLEXT=no,
#		[#include <GL/gl.h>])
#	fi
#	if test "x$HAVE_OPENGL" != xyes; then
#		OPENGL_CFLAGS=""
#		OPENGL_LIBS=""
#	fi
#fi
#AC_SUBST(HAVE_OPENGL)
#AC_SUBST(OPENGL_LIBS)
#AC_SUBST(OPENGL_CFLAGS)


dnl -----------------------
dnl Checks for header files
dnl -----------------------

#AC_PATH_X
#AC_HEADER_DIRENT
#AC_HEADER_STDC
dnl AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([ sys/soundcard.h ], HAVE_OSS=yes, HAVE_OSS=no)
AC_SUBST(HAVE_OSS)

dnl -------------------------------------------------------------
dnl Checks for typedefs, structures, and compiler characteristics
dnl -------------------------------------------------------------

AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T
#AC_TYPE_VA_LIST

AC_MSG_CHECKING(for underscore prefix in names)
AC_TRY_LINK(
	[asm(".long _bar");
	int bar;],
	[],
	AC_DEFINE(HAVE_SYM_PREFIX_UNDERSCORE, 1, [Define this if C symbols are prefixed with an underscore]) AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no)
)


dnl -----------------------------
dnl Checks for library functions.
dnl -----------------------------

#AC_FUNC_ERROR_AT_LINE
#AC_PROG_GCC_TRADITIONAL
#AC_FUNC_MALLOC
#AC_FUNC_MEMCMP
#AC_FUNC_MMAP
#AC_TYPE_SIGNAL
#AC_FUNC_STAT
#AC_FUNC_VPRINTF
#AC_CHECK_FUNCS([bzero floor getcwd gethostbyname getmntent getpagesize gettimeofday memmove memset mkdir munmap pow putenv select socket sqrt strcasecmp strchr strdup strerror strrchr strstr])
#AC_FUNC_VA_COPY
#AC_FUNC__VA_COPY

dnl Check for dynamic loader -- needed for the GL refreshers
AC_CHECK_FUNCS(dlopen)
DL_LIBS=""
if test "x$ac_cv_func_dlopen" != "xyes"; then
	AC_CHECK_LIB(dl, 
		dlopen,
		AC_DEFINE(HAVE_DLOPEN, 1, [Define if you have the dlopen function.]) DL_LIBS="-ldl"
	)
fi
AC_SUBST(DL_LIBS)


dnl --------------------
dnl Input device support
dnl --------------------

AC_MSG_CHECKING(whether to enable joystick support)
AC_ARG_ENABLE(joystick,
	[  --disable-joystick      disable joystick support ],
	AC_MSG_RESULT(no),
	AC_MSG_RESULT(yes)
	HAVE_JOYSTICK=yes
)
if test "x$HAVE_JOYSTICK" = "xyes";then
	AC_CHECK_HEADER(linux/joystick.h, HAVE_JOYSTICK=yes
		AC_DEFINE(HAVE_JOYSTICK, 1, [Define this if you want joystick support to be built])
	, HAVE_JOYSTICK=no)
fi

dnl -------------------------
dnl vec_t double precision
dnl -------------------------

AC_MSG_CHECKING(wether to enable double math precision)
AC_ARG_ENABLE(doublevec_t,
	[  --enable-doublevec_t    enable doube math precision ],
	AC_MSG_RESULT(yes)
	HAVE_DOUBLEVEC_T=yes,
	AC_MSG_RESULT(no)
)
if test "x$HAVE_DOUBLEVEC_T" = "xyes"; then
	AC_DEFINE(HAVE_DOUBLEVEC_T, 1, [Define this if you want double math precision])
fi


dnl -------------------------
dnl Optimising compiler flags
dnl -------------------------

OPT_CXXFLAGS=""
USE_OPT=""
AC_MSG_CHECKING(whether to allow compiler optimisations)
AC_ARG_ENABLE(opt,
	[  --disable-opt           disable compiler optimisations ],
	AC_MSG_RESULT(no),
	AC_MSG_RESULT(yes)
	USE_OPT="yes"
	OPT_CXXFLAGS="$OPT_CXXFLAGS -ffast-math -funroll-loops"
)

if test "x$USE_OPT" = "xyes"; then
#	case "${host}" in
#		i?86-*-*)
#			AC_MSG_CHECKING(whether -falign-functions works)
#			save_CXXFLAGS="$CXXFLAGS"
#			CXXFLAGS="-falign-functions"
#			AC_TRY_COMPILE(
#				[],
#				[],
#				OPT_CXXFLAGS="$OPT_CXXFLAGS -falign-loops=2 -falign-jumps=2 -falign-functions=2"
#				AC_MSG_RESULT(yes),
#				OPT_CXXFLAGS="$OPT_CXXFLAGS -malign-loops=2 -malign-jumps=2 -malign-functions=2"
#				AC_MSG_RESULT(no),
#			)
#			CXXFLAGS="$save_CXXFLAGS"
#			;;
#		*)
#			;;
#	esac
	
	temp_CXXFLAGS=`echo $CXXFLAGS | sed 's/O2/O2/g'`
	CXXFLAGS="$temp_CXXFLAGS"
else
	# kill off that fucking retarded -O2 that gets bunged in by default
	temp_CXXFLAGS=`echo $CXXFLAGS | sed 's/O2/O0/g'`
	CXXFLAGS="$temp_CXXFLAGS"
        AC_MSG_WARN([[** Compiler optimisations switched off **]])
fi
AC_SUBST(OPT_CXXFLAGS)


dnl -------------------------
dnl Debugging compiler flags
dnl -------------------------

AC_MSG_CHECKING(wether to enable debugging information)
AC_ARG_ENABLE(debug,
	[  --enable-debug          enable debugging ],
	AC_MSG_RESULT(yes)
	HAVE_DEBUG=yes,
	AC_MSG_RESULT(no)
)
if test "x$HAVE_DEBUG" = "xyes"; then
	AC_DEFINE(HAVE_DEBUG, 1, [Define this if you want debugging information])
	temp_CXXFLAGS=`echo $CXXFLAGS | sed 's/-g/-ggdb/g'`
	CXXFLAGS="$temp_CXXFLAGS"
else
	temp_CXXFLAGS=`echo $CXXFLAGS | sed 's/-g//g'`
	CXXFLAGS="$temp_CXXFLAGS"
fi

dnl -----------------
dnl Compiler warnings
dnl -----------------

WARN_CXXFLAGS=""
AC_MSG_CHECKING(whether to abort on compiler warnings)
AC_ARG_ENABLE(warn,
        [  --disable-warn          don't abort on compiler warnings ],
        AC_MSG_RESULT(no),
        AC_MSG_RESULT(yes)
        WARN_CXXFLAGS="$WARN_CXXFLAGS -Wall -Werror"
        # -Wstrict-prototypes -Wmissing-prototypes -Waggregate-return -Wcast-align -Wcast-qual -Wnested-externs -Wshadow -Wbad-function-cast -Wwrite-strings
        # -Wpointer-arith -Wundef -Wmissing-declarations -Winline -Wconversion
)
AC_SUBST(WARN_CXXFLAGS)

dnl ------------------------------
dnl Check for archs that use lib64
dnl ------------------------------
                                                                                
AC_MSG_CHECKING(whether 64 bit library install path applies)
if test "x${libdir}" = "x\${exec_prefix}/lib"; then
        case "${host}" in
                powerpc64|x86_64|mips64|s390x-*)
                        libdir=${libdir}64
                        AC_MSG_RESULT(yes)
                        ;;
                *)
                        AC_MSG_RESULT(no)
                        ;;
        esac
else
        AC_MSG_RESULT(no)
fi

dnl -----------------
dnl CXXFLAGS for release and devel versions
dnl -----------------

PROFILE_CXXFLAGS=""
AC_MSG_CHECKING(whether to enable profiling)
AC_ARG_ENABLE(profile,
	[  --enable-profile        compile with profiling (for development)],
	profile=$enable_profile
)

if test "x$profile" = xyes; then
	PROFILE_CXXFLAGS="-pg"
	LDFLAGS="$LDFLAGS -pg"
fi
AC_SUBST(PROFILE_CXXFLAGS)


dnl -------------------
dnl Checks for asm junk
dnl -------------------

AC_MSG_CHECKING(if assembler can be used)
case "${host}" in
	i?86-*-*)
		AC_MSG_RESULT(yes)
		AC_MSG_CHECKING(wether to enable assembler optimizations)
		AC_ARG_ENABLE(asm,
			[  --enable-asm           enable assembler optimizations ],
			AC_MSG_RESULT(yes)
			AC_DEFINE(USE_ASM, 1, [Define this if you want to use assembler optimised code]),
			AC_MSG_RESULT(no)
		)
		;;
	*)
		AC_MSG_RESULT(no)
		;;
esac


dnl -----------------
dnl Checks for SIMD support
dnl -----------------

SIMD_CXXFLAGS=""

AC_MSG_CHECKING(whether to enable 3DNow! support)
AC_ARG_ENABLE(3dnow,
	[  --enable-3dnow          enable 3DNow! optimisations ],
	AC_MSG_RESULT(yes)
	USE_3DNOW=yes,
	AC_MSG_RESULT(no)
)

if test "x$USE_3DNOW" = "xyes"; then
	AC_DEFINE(USE_3DNOW, 1, [Define this if you want to use 3DNow! optimised code])
	SIMD_CXXFLAGS="$SIMD_CXXFLAGS -m3dnow"
fi

AC_MSG_CHECKING(whether to enable SSE support)
AC_ARG_ENABLE(sse,
	[  --enable-3dnow          enable SSE optimisations ],
	AC_MSG_RESULT(yes)
	USE_SSE=yes,
	AC_MSG_RESULT(no)
)

if test "x$USE_SSE" = "xyes"; then
	AC_DEFINE(USE_SSE, 1, [Define this if you want to use SSE optimised code])
	SIMD_CXXFLAGS="$SIMD_CXXFLAGS -msse -mfpmath=sse"
fi

AC_SUBST(SIMD_CXXFLAGS)


dnl -------------------
dnl Checks for system type
dnl -------------------

case "${host}" in
	i?86-*-mingw32*)
		SYSTYPE=WIN32
		SYSTEM_LIBS="$SYSTEMLIBS -lmingw32 -mwindows -lwsock32 -lwinmm -lz"
		;;
	*)
		SYSTYPE=POSIX
		AC_MSG_RESULT(no)
		;;
esac
AM_CONDITIONAL(SYSTYPE_WIN32, test "$SYSTYPE" = "WIN32")
AC_SUBST(SYSTEM_LIBS)



dnl ----------------------
dnl Fill in path variables
dnl ----------------------

dnl $libdir usually gets set to ${exec_prefix}/lib,
dnl $prefix and $exec_prefix is likely to be "NONE"
dnl Autoconf usually sets pkglibdir correctly in the Makefile, but not in
dnl the configure script :(
test "$prefix" = "NONE" && prefix=/usr/local
test "$exec_prefix" = "NONE" && exec_prefix=$prefix

eval PKGLIBDIR="$libdir/$PACKAGE"
AC_DEFINE_UNQUOTED(PKGLIBDIR, "$PKGLIBDIR", [Define this to the path containing the dynamic modules (\${exec-prefix}/lib/qrazor-fx/)])

eval PKGDATADIR="$datadir/$PACKAGE"
AC_DEFINE_UNQUOTED(PKGDATADIR, "$PKGDATADIR", [Define this to the path containing the game data (\${prefix}/share/qrazor-fx/)])

eval BINDIR="$bindir"

dnl ---------------------------------
dnl Work out what refreshers to build
dnl ---------------------------------

VID_REFS=""

if test "x$HAVE_X" = xyes; then
	VID_REFS="$VID_REFS ref_glsl_glx.la"
	BUILD_GLX="yes"
fi

#if test "x$HAVE_X" = xyes -a "x$HAVE_OPENGL" = xyes; then
#	VID_REFS="$VID_REFS ref_arb_glx.la"
#	BUILD_GLX="yes"
#fi

#if test "x$HAVE_X" = xyes -a "x$HAVE_OPENGL" = xyes; then
#	VID_REFS="$VID_REFS ref_nv30_glx.la"
#	BUILD_GLX="yes"
#fi

#if test "x$HAVE_X" = xyes -a "x$HAVE_OPENGL" = xyes -a "x$HAVE_CG" = xyes; then
#	VID_REFS="$VID_REFS ref_nv30_glx.la"
#	BUILD_GLX="yes"
#fi

AM_CONDITIONAL(BUILD_GLX, test "x$BUILD_GLX" = xyes)
AC_SUBST(VID_REFS)


dnl -----------
dnl Dump it out
dnl -----------

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/shared/Makefile
	src/xreal/Makefile
	data/Makefile
	data/xreal/Makefile
	data/xreal/cg/Makefile
	data/xreal/glsl/Makefile
	data/xreal/materials/Makefile
	data/xreal/sound/Makefile
])
AC_OUTPUT

VID_REFS=`echo $VID_REFS | sed -e 's/ref_//g' -e 's/\.la//g'`
AC_MSG_RESULT([
	platform         : ${host}
	compiler flags   : $CXXFLAGS $WARN_CXXFLAGS $OPT_CXXFLAGS $SIMD_CXXFLAGS $PROFILE_CXXFLAGS
	programs         : $BINDIR
	plugins          : $PKGLIBDIR
	game data        : $PKGDATADIR
	video refreshers : ${VID_REFS- none}
])

