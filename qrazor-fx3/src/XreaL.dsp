# Microsoft Developer Studio Project File - Name="XreaL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=XreaL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "XreaL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "XreaL.mak" CFG="XreaL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "XreaL - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "XreaL - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "XreaL - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "XreaL - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "XreaL___Win32_Debug"
# PROP BASE Intermediate_Dir "XreaL___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "XreaL___Win32_Debug"
# PROP Intermediate_Dir "XreaL___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "XreaL - Win32 Release"
# Name "XreaL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\xreal\cg_cin.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_decal.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_dlight.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_ents.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_fx.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_inv.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_main.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_particle.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_screen.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_tent.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_view.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_chase.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_client.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_cmds.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_combat.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_entity.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_func.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_hud.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_item.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_main.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_misc.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_phys.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_pweapon.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_save.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_spawn.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_svcmds.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_target.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_trigger.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_turret.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_utils.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_view.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_weapon.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\g_world.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_address.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_connect.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_controls.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_credits.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_dloptions.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_dmoptions.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_game.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_keys.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_linux.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_loadgame.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_main.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_menu.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_mplayer.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_pconfig.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_quit.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_server.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_utils.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_win.cxx
# End Source File
# Begin Source File

SOURCE=.\xreal\x_shared.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\xreal\cg_local.h
# End Source File
# Begin Source File

SOURCE=.\xreal\cg_public.h
# End Source File
# Begin Source File

SOURCE=.\xreal\g_local.h
# End Source File
# Begin Source File

SOURCE=.\xreal\g_player.h
# End Source File
# Begin Source File

SOURCE=.\xreal\g_public.h
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_keycodes.h
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_local.h
# End Source File
# Begin Source File

SOURCE=.\xreal\ui_public.h
# End Source File
# Begin Source File

SOURCE=.\xreal\x_shared.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
