# Microsoft Developer Studio Project File - Name="cgame" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=cgame - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cgame.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cgame.mak" CFG="cgame - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cgame - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cgame - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/MissionPack/code/cgame", NPAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cgame - Win32 Debug"

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
# ADD BASE CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /base:"0x30000000" /subsystem:windows /dll /incremental:no /map /debug /machine:IX86 /out:"..\Debug\cgamex86.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /base:"0x30000000" /subsystem:windows /dll /incremental:no /map /debug /machine:IX86 /out:"..\Debug\cgamex86.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

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
# ADD BASE CPP /nologo /G6 /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /GF /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /base:"0x30000000" /subsystem:windows /dll /map /machine:IX86 /out:"..\Release\cgamex86.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /base:"0x30000000" /subsystem:windows /dll /map /machine:IX86 /out:"..\Release\cgamex86.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "cgame - Win32 Debug"
# Name "cgame - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=..\game\bg_lib.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\game\bg_misc.c
DEP_CPP_BG_MI=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\bg_pmove.c
DEP_CPP_BG_PM=\
	"..\game\bg_lib.h"\
	"..\game\bg_local.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\bg_slidemove.c
DEP_CPP_BG_SL=\
	"..\game\bg_lib.h"\
	"..\game\bg_local.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_consolecmds.c
DEP_CPP_CG_CO=\
	"..\..\ui\menudef.h"\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\ui\keycodes.h"\
	"..\ui\ui_shared.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_draw.c
DEP_CPP_CG_DR=\
	"..\..\ui\menudef.h"\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\ui\keycodes.h"\
	"..\ui\ui_shared.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_drawtools.c
DEP_CPP_CG_DRA=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_effects.c
DEP_CPP_CG_EF=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_ents.c
DEP_CPP_CG_EN=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_event.c
DEP_CPP_CG_EV=\
	"..\..\ui\menudef.h"\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_info.c
DEP_CPP_CG_IN=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_localents.c
DEP_CPP_CG_LO=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_main.c
DEP_CPP_CG_MA=\
	"..\..\ui\menudef.h"\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\ui\keycodes.h"\
	"..\ui\ui_shared.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_marks.c
DEP_CPP_CG_MAR=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_newDraw.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=cg_players.c
DEP_CPP_CG_PL=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_playerstate.c
DEP_CPP_CG_PLA=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_predict.c
DEP_CPP_CG_PR=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_scoreboard.c
DEP_CPP_CG_SC=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_servercmds.c
DEP_CPP_CG_SE=\
	"..\..\ui\menudef.h"\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_snapshot.c
DEP_CPP_CG_SN=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_syscalls.c
DEP_CPP_CG_SY=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_view.c
DEP_CPP_CG_VI=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=cg_weapons.c
DEP_CPP_CG_WE=\
	"..\game\bg_lib.h"\
	"..\game\bg_public.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\cg_local.h"\
	".\cg_public.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\q_math.c
DEP_CPP_Q_MAT=\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\q_shared.c
DEP_CPP_Q_SHA=\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ui\ui_shared.c
DEP_CPP_UI_SH=\
	"..\..\ui\menudef.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\ui\keycodes.h"\
	"..\ui\ui_shared.h"\
	".\tr_types.h"\
	

!IF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\game\bg_public.h
# End Source File
# Begin Source File

SOURCE=cg_local.h
# End Source File
# Begin Source File

SOURCE=cg_public.h
# End Source File
# Begin Source File

SOURCE=..\game\q_shared.h
# End Source File
# Begin Source File

SOURCE=..\game\surfaceflags.h
# End Source File
# End Group
# Begin Source File

SOURCE=cgame.def
# End Source File
# End Target
# End Project
