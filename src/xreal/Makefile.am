# $Id$

# Process this file with automake to create Makefile.in

pkglibdir=$(libdir)/@PACKAGE@/xreal
pkglib_LTLIBRARIES = cgame.la game.la ui.la


std_cflags = -fPIC -pipe @WARN_CXXFLAGS@ @OPT_CXXFLAGS@ @SIMD_CXXFLAGS@ @PROFILE_CXXFLAGS@
module_ldflags = -module -avoid-version -no-undefined -rpath $(pkglibdir)


cgame_la_SOURCES = cg_decal.cxx \
		cg_entity.cxx \
		cg_event.cxx \
		cg_explosion.cxx \
		cg_hud.cxx \
		cg_inv.cxx \
		cg_light.cxx \
		cg_main.cxx \
		cg_muzzleflash.cxx \
		cg_particle.cxx \
		cg_player.cxx \
		cg_pred.cxx \
		cg_screen.cxx \
		cg_tent.cxx \
		cg_view.cxx \
		cg_weapon.cxx
		
cgame_la_CXXFLAGS = $(std_cflags)
cgame_la_LIBADD = ../shared/libshared.a
cgame_la_LDFLAGS = $(module_ldflags)




game_la_SOURCES = g_combat.cxx \
		g_debris.cxx \
		g_entity.cxx \
		g_field.cxx\
		g_func.cxx \
		g_hud.cxx \
		g_item.cxx \
		g_light.cxx \
		g_main.cxx \
		g_misc.cxx \
		g_phys.cxx \
		g_player.cxx \
		g_projectile.cxx \
		g_pweapon.cxx \
		g_spawn.cxx \
		g_svcmds.cxx \
		g_target.cxx \
		g_trigger.cxx \
		g_utils.cxx \
		g_weapon.cxx \
		g_world.cxx

game_la_CXXFLAGS = $(std_cflags)
game_la_LIBADD = ../shared/libshared.a @ODE_LIBS@
game_la_LDFLAGS = $(module_ldflags)



				
ui_la_SOURCES = ui_address.cxx \
		ui_audio.cxx \
		ui_connect.cxx \
		ui_credits.cxx \
		ui_dloptions.cxx \
		ui_dmoptions.cxx \
		ui_keys.cxx \
		ui_loadgame.cxx \
		ui_main.cxx \
		ui_menu.cxx \
		ui_mplayer.cxx \
		ui_options.cxx \
		ui_pconfig.cxx \
		ui_quit.cxx \
		ui_server.cxx \
		ui_splayer.cxx \
		ui_utils.cxx \
		ui_video.cxx

ui_la_CXXFLAGS = $(std_cflags)
ui_la_LIBADD = ../shared/libshared.a
ui_la_LDFLAGS = $(module_ldflags)

		
noinst_HEADERS = cg_local.h \
		cg_public.h \
		\
		g_camera.h \
		g_debris.h \
		g_entity.h \
		g_field.h \
		g_func.h \
		g_item.h \
		g_light.h \
		g_local.h \
		g_player.h \
		g_projectile.h \
		g_public.h \
		g_target.h \
		g_trigger.h \
		g_world.h \
		\
		ui_local.h \
		ui_public.h


EXTRA_DIST = 	CGame.def \
		Game.def \
		UI.def \
		\
		XreaL_CGame.dsp \
		XreaL_Game.dsp \
		XreaL_UI.dsp


