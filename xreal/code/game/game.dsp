# Microsoft Developer Studio Project File - Name="game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=game - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "game.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="game - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "game - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "game - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/MissionPack/code/game", CYAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "game - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /D "DEBUG" /D "GLOBALRANK" /FR /YX /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /D "DEBUG" /D "GLOBALRANK" /FR /YX /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /incremental:no /map /debug /machine:IX86 /out:"..\Debug\qagamex86.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /incremental:no /map /debug /machine:IX86 /out:"..\Debug\qagamex86.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "GLOBALRANK" /YX /GF /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "GLOBALRANK" /YX /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /map /machine:IX86 /out:"..\Release\qagamex86.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /map /machine:IX86 /out:"..\Release\qagamex86.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "game - Win32 Debug"
# Name "game - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=ai_chat.c
DEP_CPP_AI_CH=\
	"..\..\ui\menudef.h"\
	".\ai_chat.h"\
	".\ai_cmd.h"\
	".\ai_dmnet.h"\
	".\ai_dmq3.h"\
	".\ai_main.h"\
	".\be_aas.h"\
	".\be_ai_char.h"\
	".\be_ai_chat.h"\
	".\be_ai_gen.h"\
	".\be_ai_goal.h"\
	".\be_ai_move.h"\
	".\be_ai_weap.h"\
	".\be_ea.h"\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\botlib.h"\
	".\chars.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\inv.h"\
	".\match.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	".\syn.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=ai_cmd.c
DEP_CPP_AI_CM=\
	"..\..\ui\menudef.h"\
	".\ai_chat.h"\
	".\ai_cmd.h"\
	".\ai_dmnet.h"\
	".\ai_dmq3.h"\
	".\ai_main.h"\
	".\ai_team.h"\
	".\be_aas.h"\
	".\be_ai_char.h"\
	".\be_ai_chat.h"\
	".\be_ai_gen.h"\
	".\be_ai_goal.h"\
	".\be_ai_move.h"\
	".\be_ai_weap.h"\
	".\be_ea.h"\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\botlib.h"\
	".\chars.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\inv.h"\
	".\match.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	".\syn.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=ai_dmnet.c
DEP_CPP_AI_DM=\
	"..\..\ui\menudef.h"\
	".\ai_chat.h"\
	".\ai_cmd.h"\
	".\ai_dmnet.h"\
	".\ai_dmq3.h"\
	".\ai_main.h"\
	".\ai_team.h"\
	".\be_aas.h"\
	".\be_ai_char.h"\
	".\be_ai_chat.h"\
	".\be_ai_gen.h"\
	".\be_ai_goal.h"\
	".\be_ai_move.h"\
	".\be_ai_weap.h"\
	".\be_ea.h"\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\botlib.h"\
	".\chars.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\inv.h"\
	".\match.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	".\syn.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=ai_dmq3.c
DEP_CPP_AI_DMQ=\
	"..\..\ui\menudef.h"\
	".\ai_chat.h"\
	".\ai_cmd.h"\
	".\ai_dmnet.h"\
	".\ai_dmq3.h"\
	".\ai_main.h"\
	".\ai_team.h"\
	".\be_aas.h"\
	".\be_ai_char.h"\
	".\be_ai_chat.h"\
	".\be_ai_gen.h"\
	".\be_ai_goal.h"\
	".\be_ai_move.h"\
	".\be_ai_weap.h"\
	".\be_ea.h"\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\botlib.h"\
	".\chars.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\inv.h"\
	".\match.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	".\syn.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=ai_main.c
DEP_CPP_AI_MA=\
	".\ai_chat.h"\
	".\ai_cmd.h"\
	".\ai_dmnet.h"\
	".\ai_dmq3.h"\
	".\ai_main.h"\
	".\ai_vcmd.h"\
	".\be_aas.h"\
	".\be_ai_char.h"\
	".\be_ai_chat.h"\
	".\be_ai_gen.h"\
	".\be_ai_goal.h"\
	".\be_ai_move.h"\
	".\be_ai_weap.h"\
	".\be_ea.h"\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\botlib.h"\
	".\chars.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\inv.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	".\syn.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=ai_team.c
DEP_CPP_AI_TE=\
	"..\..\ui\menudef.h"\
	".\ai_chat.h"\
	".\ai_cmd.h"\
	".\ai_dmnet.h"\
	".\ai_dmq3.h"\
	".\ai_main.h"\
	".\ai_team.h"\
	".\ai_vcmd.h"\
	".\be_aas.h"\
	".\be_ai_char.h"\
	".\be_ai_chat.h"\
	".\be_ai_gen.h"\
	".\be_ai_goal.h"\
	".\be_ai_move.h"\
	".\be_ai_weap.h"\
	".\be_ea.h"\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\botlib.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\match.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=ai_vcmd.c
DEP_CPP_AI_VC=\
	"..\..\ui\menudef.h"\
	".\ai_chat.h"\
	".\ai_cmd.h"\
	".\ai_dmnet.h"\
	".\ai_dmq3.h"\
	".\ai_main.h"\
	".\ai_team.h"\
	".\ai_vcmd.h"\
	".\be_aas.h"\
	".\be_ai_char.h"\
	".\be_ai_chat.h"\
	".\be_ai_gen.h"\
	".\be_ai_goal.h"\
	".\be_ai_move.h"\
	".\be_ai_weap.h"\
	".\be_ea.h"\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\botlib.h"\
	".\chars.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\inv.h"\
	".\match.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	".\syn.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=bg_lib.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=bg_misc.c
DEP_CPP_BG_MI=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=bg_pmove.c
DEP_CPP_BG_PM=\
	".\bg_lib.h"\
	".\bg_local.h"\
	".\bg_public.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=bg_slidemove.c
DEP_CPP_BG_SL=\
	".\bg_lib.h"\
	".\bg_local.h"\
	".\bg_public.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_active.c
DEP_CPP_G_ACT=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_arenas.c
DEP_CPP_G_ARE=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_bot.c
DEP_CPP_G_BOT=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_client.c
DEP_CPP_G_CLI=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_cmds.c
DEP_CPP_G_CMD=\
	"..\..\ui\menudef.h"\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_combat.c
DEP_CPP_G_COM=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_items.c
DEP_CPP_G_ITE=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_main.c
DEP_CPP_G_MAI=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_mem.c
DEP_CPP_G_MEM=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_misc.c
DEP_CPP_G_MIS=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_missile.c
DEP_CPP_G_MISS=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_mover.c
DEP_CPP_G_MOV=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_session.c
DEP_CPP_G_SES=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_spawn.c
DEP_CPP_G_SPA=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_svcmds.c
DEP_CPP_G_SVC=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_syscalls.c
DEP_CPP_G_SYS=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_target.c
DEP_CPP_G_TAR=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_team.c
DEP_CPP_G_TEA=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_trigger.c
DEP_CPP_G_TRI=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_utils.c
DEP_CPP_G_UTI=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=g_weapon.c
DEP_CPP_G_WEA=\
	".\bg_lib.h"\
	".\bg_public.h"\
	".\g_local.h"\
	".\g_public.h"\
	".\g_team.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=game.def
# End Source File
# Begin Source File

SOURCE=q_math.c
DEP_CPP_Q_MAT=\
	".\bg_lib.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=q_shared.c
DEP_CPP_Q_SHA=\
	".\bg_lib.h"\
	".\q_shared.h"\
	".\surfaceflags.h"\
	

!IF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=ai_chat.h
# End Source File
# Begin Source File

SOURCE=ai_cmd.h
# End Source File
# Begin Source File

SOURCE=ai_dmnet.h
# End Source File
# Begin Source File

SOURCE=ai_dmq3.h
# End Source File
# Begin Source File

SOURCE=ai_main.h
# End Source File
# Begin Source File

SOURCE=ai_team.h
# End Source File
# Begin Source File

SOURCE=ai_vcmd.h
# End Source File
# Begin Source File

SOURCE=be_aas.h
# End Source File
# Begin Source File

SOURCE=be_ai_char.h
# End Source File
# Begin Source File

SOURCE=be_ai_chat.h
# End Source File
# Begin Source File

SOURCE=be_ai_gen.h
# End Source File
# Begin Source File

SOURCE=be_ai_goal.h
# End Source File
# Begin Source File

SOURCE=be_ai_move.h
# End Source File
# Begin Source File

SOURCE=be_ai_weap.h
# End Source File
# Begin Source File

SOURCE=be_ea.h
# End Source File
# Begin Source File

SOURCE=bg_local.h
# End Source File
# Begin Source File

SOURCE=bg_public.h
# End Source File
# Begin Source File

SOURCE=botlib.h
# End Source File
# Begin Source File

SOURCE=chars.h
# End Source File
# Begin Source File

SOURCE=g_local.h
# End Source File
# Begin Source File

SOURCE=g_public.h
# End Source File
# Begin Source File

SOURCE=g_team.h
# End Source File
# Begin Source File

SOURCE=inv.h
# End Source File
# Begin Source File

SOURCE=match.h
# End Source File
# Begin Source File

SOURCE=q_shared.h
# End Source File
# Begin Source File

SOURCE=surfaceflags.h
# End Source File
# Begin Source File

SOURCE=syn.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
