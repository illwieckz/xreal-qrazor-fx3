
SUBDIRS = shared . xreal

std_cflags = -pipe @WARN_CXXFLAGS@ @OPT_CXXFLAGS@ @SIMD_CXXFLAGS@ @PROFILE_CXXFLAGS@
module_ldflags = -module -avoid-version -no-undefined -rpath $(pkglibdir)

bin_PROGRAMS = qrazor-fx-server qrazor-fx-client qrazor-fx-map
pkglib_LTLIBRARIES = @VID_REFS@
EXTRA_LTLIBRARIES = ref_arb_glx.la ref_glsl_glx.la ref_nv30_glx.la

# Common source files
REF_GL_COMMON = r_alias.cxx \
		r_ase.cxx \
		r_backend.cxx \
		r_bsp.cxx \
		r_command.cxx \
		r_draw.cxx \
		r_entity.cxx \
		r_frustum.cxx \
		r_gl.cxx \
		r_image.cxx \
		r_light.cxx \
		r_lwo.cxx \
		r_main.cxx \
		r_md2.cxx \
		r_md3.cxx \
		r_md5.cxx \
		r_mdc.cxx \
		r_mds.cxx \
		r_mesh.cxx \
		r_model.cxx \
		r_obj.cxx \
		r_particle.cxx \
		r_poly.cxx \
		r_proc.cxx \
		r_shader.cxx \
		r_shadow.cxx \
		r_skel.cxx \
		r_skel_anim.cxx \
		r_skin.cxx \
		r_sky.cxx \
		r_static.cxx \
		r_surf.cxx \
		\
		img_tga.cxx \
		img_jpg.cxx \
		img_png.cxx

if SYSTYPE_WIN32
QRAZOR_FX_SERVER_SYS = sys_win.cxx
else
QRAZOR_FX_SERVER_SYS = sys_linux.cxx glob.cxx
endif

qrazor_fx_server_SOURCES = cl_null.cxx \
			\
			cm_ase.cxx \
			cm_bsp.cxx \
			cm_main.cxx \
			cm_md3.cxx \
			cm_md5anim.cxx \
			cm_md5mesh.cxx \
			cmd.cxx \
			common.cxx \
			cvar.cxx \
			md4.cxx \
			net_chan.cxx \
			sha1.cxx \
			vfs.cxx \
			unzip.cxx \
			\
			sv_ccmds.cxx \
			sv_client.cxx \
			sv_ents.cxx \
			sv_game.cxx \
			sv_init.cxx \
			sv_main.cxx \
			sv_send.cxx \
			sv_user.cxx \
			sv_world.cxx \
			\
			map_null.cxx \
			\
			$(QRAZOR_FX_SERVER_SYS)

EXTRA_qrazor_fx_server_SOURCES = sys_win.cxx sys_linux.cxx glob.cxx

qrazor_fx_server_CXXFLAGS = $(std_cflags) -DDEDICATED_ONLY
qrazor_fx_server_LDFLAGS =
qrazor_fx_server_LDADD = shared/libshared.a @DL_LIBS@ @SYSTEM_LIBS@ @Z_LIBS@ @ODE_LIBS@ -lm





if SYSTYPE_WIN32
QRAZOR_FX_CLIENT_SYS = sys_win.cxx in_win.cxx vid_dll.cxx
else
QRAZOR_FX_CLIENT_SYS = sys_linux.cxx glob.cxx vid_so.cxx
endif

qrazor_fx_client_SOURCES = cl_cgame.cxx \
			cl_console.cxx \
			cl_input.cxx \
			cl_keys.cxx \
			cl_main.cxx \
			cl_netchan.cxx \
			cl_parse.cxx \
			cl_screen.cxx \
			cl_ui.cxx \
			\
			s_backend.cxx \
			s_main.cxx \
			s_shader.cxx \
			\
			cm_ase.cxx \
			cm_bsp.cxx \
			cm_main.cxx \
			cm_md3.cxx \
			cm_md5anim.cxx \
			cm_md5mesh.cxx \
			cmd.cxx \
			common.cxx \
			cvar.cxx \
			md4.cxx \
			net_chan.cxx \
			roq.cxx \
			sha1.cxx \
			vfs.cxx \
			unzip.cxx \
			\
			sv_null.cxx \
			\
			map_null.cxx \
			\
			$(QRAZOR_FX_CLIENT_SYS)

EXTRA_qrazor_fx_client_SOURCES = sys_win.cxx vid_dll.cxx in_win.cxx sys_linux.cxx vid_so.cxx glob.cxx

qrazor_fx_client_CXXFLAGS = $(std_cflags) @X_CFLAGS@
qrazor_fx_client_LDFLAGS = 
qrazor_fx_client_LDADD = shared/libshared.a @DL_LIBS@ @SYSTEM_LIBS@ @Z_LIBS@ @OPENAL_LIBS@ @ODE_LIBS@ -lm



if SYSTYPE_WIN32
QRAZOR_FX_MAP_SYS = sys_win.cxx
else
QRAZOR_FX_MAP_SYS = sys_linux.cxx glob.cxx
endif

qrazor_fx_map_SOURCES = cl_null.cxx \
			\
			cmd.cxx \
			common.cxx \
			cvar.cxx \
			sha1.cxx \
			vfs.cxx \
			unzip.cxx \
			\
			sv_null.cxx \
			\
			map_bspfile.cxx \
			map_entity.cxx \
			map_main.cxx \
			map_map.cxx \
			map_winding.cxx \
			\
			$(QRAZOR_FX_MAP_SYS)

EXTRA_qrazor_fx_map_SOURCES =

qrazor_fx_map_CXXFLAGS = $(std_cflags) -DBSPCOMPILER_ONLY
qrazor_fx_map_LDFLAGS =
qrazor_fx_map_LDADD = shared/libshared.a @DL_LIBS@ @SYSTEM_LIBS@ @Z_LIBS@ -lm



if BUILD_GLX
ref_arb_glx_la_SOURCES = $(REF_GL_COMMON) glimp_glx.cxx r_backend_arb.cxx
ref_arb_glx_la_CXXFLAGS = $(std_cflags) -fPIC @X_CFLAGS@
ref_arb_glx_la_LIBADD = shared/libshared.a @X_PRE_LIBS@ @X_LIBS@ @XTRA_LIBS@ @X_EXTRA_LIBS@ @JPEG_LIBS@ @PNG_LIBS@
ref_arb_glx_la_LDFLAGS = $(module_ldflags)

ref_glsl_glx_la_SOURCES = $(REF_GL_COMMON) glimp_glx.cxx r_backend_glsl.cxx
ref_glsl_glx_la_CXXFLAGS = $(std_cflags) -fPIC @X_CFLAGS@
ref_glsl_glx_la_LIBADD = shared/libshared.a @X_PRE_LIBS@ @X_LIBS@ @XTRA_LIBS@ @X_EXTRA_LIBS@ @JPEG_LIBS@ @PNG_LIBS@
ref_glsl_glx_la_LDFLAGS = $(module_ldflags)

ref_nv30_glx_la_SOURCES = $(REF_GL_COMMON) glimp_glx.cxx r_backend_nv30.cxx
ref_nv30_glx_la_CXXFLAGS = $(std_cflags) -fPIC @X_CFLAGS@
ref_nv30_glx_la_LIBADD = shared/libshared.a @X_PRE_LIBS@ @X_LIBS@ @XTRA_LIBS@ @X_EXTRA_LIBS@ @JPEG_LIBS@ @PNG_LIBS@
ref_nv30_glx_la_LDFLAGS = $(module_ldflags)
endif

#EXTRA_ref_arb_glx_la_SOURCES = glimp_glx.cxx


noinst_HEADERS = cl_console.h \
		cl_keys.h \
		cl_local.h \
		cl_screen.h \
		cm.h \
		cm_md3.h \
		cmd.h \
		common.h \
		cvar.h \
		files.h \
		glimp_glx.h \
		glimp_wgl.h \
		glob.h \
		img_jpg.h \
		img_png.h \
		img_tga.h \
		input.h \
		map_local.h \
		net_chan.h \
		resource.h \
		r_backend.h \
		r_gl.h \
		r_local.h \
		r_public.h \
		roq.h \
		rw_linux.h \
		sha1.h \
		s_local.h \
		s_public.h \
		sv_local.h \
		sys.h \
		unzip.h \
		vfs.h \
		vid.h \
		winquake.h
		
EXTRA_DIST = 	QRazorFX.dsp \
		QRazorFX.dsw \
		\
		refGL.def \
		refGL.dsp \
		\
		XreaL.dsp

