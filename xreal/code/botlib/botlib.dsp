# Microsoft Developer Studio Project File - Name="botlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=botlib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "botlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "botlib.mak" CFG="botlib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "botlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "botlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/MissionPack/code/botlib", HBAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "botlib - Win32 Release"

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
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /G6 /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_LIB" /D "BOTLIB" /D "_MBCS" /YX /GF /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_LIB" /D "BOTLIB" /D "_MBCS" /YX /GF /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

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
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "BOTLIB" /D "DEBUG" /D "_MBCS" /FR /YX /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "BOTLIB" /D "DEBUG" /D "_MBCS" /FR /YX /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "botlib - Win32 Release"
# Name "botlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=be_aas_bspq3.c
DEP_CPP_BE_AA=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_cluster.c
DEP_CPP_BE_AAS=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_debug.c
DEP_CPP_BE_AAS_=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_entity.c
DEP_CPP_BE_AAS_E=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_file.c
DEP_CPP_BE_AAS_F=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_main.c
DEP_CPP_BE_AAS_M=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_move.c
DEP_CPP_BE_AAS_MO=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\l_libvar.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_optimize.c
DEP_CPP_BE_AAS_O=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_reach.c
DEP_CPP_BE_AAS_R=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_route.c
DEP_CPP_BE_AAS_RO=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_crc.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_routealt.c
DEP_CPP_BE_AAS_ROU=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_aas_sample.c
DEP_CPP_BE_AAS_S=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_ai_char.c
DEP_CPP_BE_AI=\
	"..\game\be_aas.h"\
	"..\game\be_ai_char.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_ai_chat.c
DEP_CPP_BE_AI_=\
	"..\game\be_aas.h"\
	"..\game\be_ai_chat.h"\
	"..\game\be_ea.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_ai_gen.c
DEP_CPP_BE_AI_G=\
	"..\game\be_aas.h"\
	"..\game\be_ai_gen.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_ai_goal.c
DEP_CPP_BE_AI_GO=\
	"..\game\be_aas.h"\
	"..\game\be_ai_goal.h"\
	"..\game\be_ai_move.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_ai_weight.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_ai_move.c
DEP_CPP_BE_AI_M=\
	"..\game\be_aas.h"\
	"..\game\be_ai_goal.h"\
	"..\game\be_ai_move.h"\
	"..\game\be_ea.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_ai_weap.c
DEP_CPP_BE_AI_W=\
	"..\game\be_aas.h"\
	"..\game\be_ai_weap.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_ai_weight.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_ai_weight.c
DEP_CPP_BE_AI_WE=\
	"..\game\be_aas.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_ai_weight.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_ea.c
DEP_CPP_BE_EA=\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\be_interface.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=be_interface.c
DEP_CPP_BE_IN=\
	"..\game\be_aas.h"\
	"..\game\be_ai_char.h"\
	"..\game\be_ai_chat.h"\
	"..\game\be_ai_gen.h"\
	"..\game\be_ai_goal.h"\
	"..\game\be_ai_move.h"\
	"..\game\be_ai_weap.h"\
	"..\game\be_ea.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\aasfile.h"\
	".\be_aas_bsp.h"\
	".\be_aas_cluster.h"\
	".\be_aas_debug.h"\
	".\be_aas_def.h"\
	".\be_aas_entity.h"\
	".\be_aas_file.h"\
	".\be_aas_funcs.h"\
	".\be_aas_main.h"\
	".\be_aas_move.h"\
	".\be_aas_optimize.h"\
	".\be_aas_reach.h"\
	".\be_aas_route.h"\
	".\be_aas_routealt.h"\
	".\be_aas_sample.h"\
	".\be_ai_weight.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=l_crc.c
DEP_CPP_L_CRC=\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\be_interface.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=l_libvar.c
DEP_CPP_L_LIB=\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\l_libvar.h"\
	".\l_memory.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=l_log.c
DEP_CPP_L_LOG=\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=l_memory.c
DEP_CPP_L_MEM=\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	".\be_interface.h"\
	".\l_log.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=l_precomp.c
DEP_CPP_L_PRE=\
	"..\bspc\l_bsp_ent.h"\
	"..\bspc\l_cmd.h"\
	"..\bspc\l_log.h"\
	"..\bspc\l_math.h"\
	"..\bspc\l_mem.h"\
	"..\bspc\l_poly.h"\
	"..\bspc\l_qfiles.h"\
	"..\bspc\l_threads.h"\
	"..\bspc\l_utils.h"\
	"..\bspc\q2files.h"\
	"..\bspc\qbsp.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\unzip.h"\
	".\be_interface.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_utils.h"\
	
NODEP_CPP_L_PRE=\
	".\qcc.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=l_script.c
DEP_CPP_L_SCR=\
	"..\bspc\l_bsp_ent.h"\
	"..\bspc\l_cmd.h"\
	"..\bspc\l_log.h"\
	"..\bspc\l_math.h"\
	"..\bspc\l_mem.h"\
	"..\bspc\l_poly.h"\
	"..\bspc\l_qfiles.h"\
	"..\bspc\l_threads.h"\
	"..\bspc\l_utils.h"\
	"..\bspc\q2files.h"\
	"..\bspc\qbsp.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\unzip.h"\
	".\be_interface.h"\
	".\l_libvar.h"\
	".\l_log.h"\
	".\l_memory.h"\
	".\l_script.h"\
	
NODEP_CPP_L_SCR=\
	".\qcc.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=l_struct.c
DEP_CPP_L_STR=\
	"..\bspc\l_bsp_ent.h"\
	"..\bspc\l_cmd.h"\
	"..\bspc\l_log.h"\
	"..\bspc\l_math.h"\
	"..\bspc\l_mem.h"\
	"..\bspc\l_poly.h"\
	"..\bspc\l_qfiles.h"\
	"..\bspc\l_threads.h"\
	"..\bspc\l_utils.h"\
	"..\bspc\q2files.h"\
	"..\bspc\qbsp.h"\
	"..\game\bg_lib.h"\
	"..\game\botlib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\unzip.h"\
	".\be_interface.h"\
	".\l_precomp.h"\
	".\l_script.h"\
	".\l_struct.h"\
	".\l_utils.h"\
	

!IF  "$(CFG)" == "botlib - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "botlib - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=aasfile.h
# End Source File
# Begin Source File

SOURCE=be_aas_bsp.h
# End Source File
# Begin Source File

SOURCE=be_aas_cluster.h
# End Source File
# Begin Source File

SOURCE=be_aas_debug.h
# End Source File
# Begin Source File

SOURCE=be_aas_def.h
# End Source File
# Begin Source File

SOURCE=be_aas_entity.h
# End Source File
# Begin Source File

SOURCE=be_aas_file.h
# End Source File
# Begin Source File

SOURCE=be_aas_funcs.h
# End Source File
# Begin Source File

SOURCE=be_aas_main.h
# End Source File
# Begin Source File

SOURCE=be_aas_move.h
# End Source File
# Begin Source File

SOURCE=be_aas_optimize.h
# End Source File
# Begin Source File

SOURCE=be_aas_reach.h
# End Source File
# Begin Source File

SOURCE=be_aas_route.h
# End Source File
# Begin Source File

SOURCE=be_aas_routealt.h
# End Source File
# Begin Source File

SOURCE=be_aas_sample.h
# End Source File
# Begin Source File

SOURCE=be_ai_weight.h
# End Source File
# Begin Source File

SOURCE=be_interface.h
# End Source File
# Begin Source File

SOURCE=..\game\bg_public.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\cm_public.h
# End Source File
# Begin Source File

SOURCE=..\game\g_public.h
# End Source File
# Begin Source File

SOURCE=l_crc.h
# End Source File
# Begin Source File

SOURCE=l_libvar.h
# End Source File
# Begin Source File

SOURCE=l_log.h
# End Source File
# Begin Source File

SOURCE=l_memory.h
# End Source File
# Begin Source File

SOURCE=l_precomp.h
# End Source File
# Begin Source File

SOURCE=l_script.h
# End Source File
# Begin Source File

SOURCE=l_struct.h
# End Source File
# Begin Source File

SOURCE=l_utils.h
# End Source File
# Begin Source File

SOURCE=..\game\q_shared.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qcommon.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qfiles.h
# End Source File
# Begin Source File

SOURCE=..\server\server.h
# End Source File
# Begin Source File

SOURCE=..\game\surfaceflags.h
# End Source File
# End Group
# End Target
# End Project
