import os, string, sys
import SCons

opts = Options()
opts.Add(BoolOption('warnings', 'Set to 1 to compile with -Wall -Werror', 1))
opts.Add(EnumOption('debug', 'Set to >= 1 to build for debug', '0', allowed_values=('0', '1', '2', '3')))
opts.Add(EnumOption('optimize', 'Set to >= 1 to build with general optimizations', '2', allowed_values=('0', '1', '2')))
opts.Add(EnumOption('simd', 'Choose special CPU register optimizations', 'none', allowed_values=('none', 'sse', '3dnow')))
#opts.Add(EnumOption('cpu', 'Set to 1 to build with special CPU register optimizations', 'i386', allowed_values=('i386', 'athlon-xp', 'pentium4')))

if sys.platform == 'linux2' or sys.platform == 'linux-i386':
	opts.Add(PathOption('PKGDATADIR', 'Installation path', '/usr/games/share/qrazor-fx'))
	
elif sys.platform == 'win32':
	opts.Add(PathOption('PKGDATADIR', 'Installation path', 'C:\Program Files\QRazor-FX'))

env = Environment(options = opts)
Help(opts.GenerateHelpText(env))

print 'compiling for platform ', sys.platform

env.Append(CXXFLAGS = '-pipe')

if env['warnings'] == 1:
	env.Append(CXXFLAGS = '-Wall -Werror')

env.Append(CXXFLAGS = '-DDEBUG=${debug}')
if env['debug'] != '0':
	env.Append(CXXFLAGS = '-ggdb${debug}')

if env['optimize'] != '0':
	env.Append(CXXFLAGS = '-O${optimize}')#-ffast-math')

	#if env['cpu'] != 'i386':
	#	env.Append(CXXFLAGS = '-march=${cpu}')
	
	#if env['cpu'] == 'athlon-xp':
	#	env.Append(CXXFLAGS = '-march=athlon-xp -msse')#-mfpmath=sse')
	
	if env['simd'] == 'sse':
		env.Append(CXXFLAGS = '-DSIMD_SSE')
	
	elif env['simd'] == '3dnow':
		env.Append(CXXFLAGS = '-DSIMD_3DNOW')



	
env.Append(CXXFLAGS = '-DVFS_PKGDATADIR=\\"${PKGDATADIR}\\"')
#env.Append(CXXFLAGS = '-DVFS_PKGLIBDIR="${PKGDATADIR}"')



conf = Configure(env)

if sys.platform == 'linux2' or sys.platform == 'linux-i386':
	if not conf.CheckLib('dl', autoadd=0):
		print 'Did not find libdl.a, exiting!'
		Exit(1)

	if not conf.CheckLib('m', autoadd=0):
		print 'Did not find libm.a or m.lib, exiting!'
		Exit(1)

if not conf.CheckCHeader('zlib.h'):
	print 'zlib.h must be installed!'
	Exit(1)
if not conf.CheckLib('z', autoadd=0):
	print 'Did not find libz.a or z.lib, exiting!'
	Exit(1)
	
if not conf.CheckCHeader('AL/al.h'):
	print 'AL/al.h must be installed!'
	Exit(1)
if not conf.CheckCHeader('AL/alc.h'):
	print 'AL/alc.h must be installed!'
	Exit(1)
if not conf.CheckCHeader('AL/alut.h'):
	print 'AL/alut.h must be installed!'
	Exit(1)
if not conf.CheckLib('openal', autoadd=0):
	print 'Did not find libopenal.a or openal.lib, exiting!'
	Exit(1)
	
#if not conf.CheckCHeader('ode/ode.h'):
#	print 'ode/ode.h must be installed!'
#	Exit(1)
#if not conf.CheckLib('ode', autoadd=0):
#	print 'Did not find libode.a or ode.lib, exiting!'
#	Exit(1)
	
if not conf.CheckCHeader('X11/Xlib.h'):
	print 'X11/Xlib.h must be installed!'
	Exit(1)
if not conf.CheckCHeader('X11/Xutil.h'):
	print 'X11/Xutil.h must be installed!'
	Exit(1)
if not conf.CheckCHeader('X11/Xatom.h'):
	print 'X11/Xatom.h must be installed!'
	Exit(1)
#if not conf.CheckLib('X11', autoadd=0):
#	print 'Did not find libX11.a, exiting!'
#	Exit(1)
#if not conf.CheckLib('Xext', symbol='XShmQueryExtension', autoadd=0):
#	print 'Did not find libXext.a, exiting!'
#	Exit(1)

#if conf.CheckCHeader('X11/extensions/xf86dga.h'):
#	conf.env.Append(CXXFLAGS='-DHAVE_XF86_DGA')
#if not conf.CheckLib('Xxf86dga', symbol='XF86DGAQueryVersion', autoadd=0):
#	print 'Did not find libXxf86dga.a, exiting!'
#	Exit(1)

#if conf.CheckCHeader('X11/extensions/xf86vmode.h'):
#	conf.env.Append(CXXFLAGS='-DHAVE_XF86_VIDMODE')
#if not conf.CheckLib('Xxf86vm', symbol='XF86VidModeSwitchToMode', autoadd=0):
#	print 'Did not find libXxf86vm.a, exiting!'
#	Exit(1)

#if not conf.CheckCHeader('jpeglib.h'):
#	print 'jpeglib.h must be installed!'
#	Exit(1)
if not conf.CheckLib('jpeg', symbol='jpeg_start_decompress', autoadd=0):
	print 'Did not find libjpeg.a or jpeg.lib, exiting!'
	Exit(1)
	
#if not conf.CheckCHeader('png.h'):
#	print 'png.h must be installed!'
#	Exit(1)
#if not conf.CheckLib('png12', symbol='png_create_read_struct', autoadd=0):
#	print 'Did not find libpng.a or png.lib, exiting!'
#	Exit(1)
		
env = conf.Finish()


Export('env')
#SConscript('SConscript_ode')
SConscript('SConscript_qrazor-fx-server', build_dir='build/server', duplicate=0)
SConscript('SConscript_qrazor-fx-client', build_dir='build/client', duplicate=0)
SConscript('SConscript_qrazor-fx-map', build_dir='build/map', duplicate=0)
#SConscript('SConscript_ref_gl', build_dir='build/ref_gl', duplicate=0)
SConscript('SConscript_xreal_cgame', build_dir='build/xreal/cgame', duplicate=0)
SConscript('SConscript_xreal_game', build_dir='build/xreal/game', duplicate=0)
SConscript('SConscript_xreal_ui', build_dir='build/xreal/ui', duplicate=0)
SConscript('SConscript_xreal_data')

