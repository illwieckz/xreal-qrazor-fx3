# Microsoft Developer Studio Project File - Name="renderer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=renderer - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "renderer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "renderer.mak" CFG="renderer - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "renderer - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "renderer - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/MissionPack/code/renderer", EJBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "renderer - Win32 Release"

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
# ADD BASE CPP /nologo /G6 /W4 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_MBCS" /YX /GF /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_MBCS" /YX /GF /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

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
# ADD BASE CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_MBCS" /FR /YX /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_MBCS" /FR /YX /GZ /c
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

# Name "renderer - Win32 Release"
# Name "renderer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=ref_trin.def
# End Source File
# Begin Source File

SOURCE=tr_animation.c
DEP_CPP_TR_AN=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_AN=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_backend.c
DEP_CPP_TR_BA=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_BA=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_bsp.c
DEP_CPP_TR_BS=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_BS=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_cmds.c
DEP_CPP_TR_CM=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_CM=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_curve.c
DEP_CPP_TR_CU=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_CU=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_flares.c
DEP_CPP_TR_FL=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_FL=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_font.c
DEP_CPP_TR_FO=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_FO=\
	"..\ft2\freetype.h"\
	"..\ft2\fterrors.h"\
	"..\ft2\ftimage.h"\
	"..\ft2\ftoutln.h"\
	"..\ft2\ftsystem.h"\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_image.c
DEP_CPP_TR_IM=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_IM=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_init.c
DEP_CPP_TR_IN=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_IN=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_light.c
DEP_CPP_TR_LI=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_LI=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_main.c
DEP_CPP_TR_MA=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_MA=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_marks.c
DEP_CPP_TR_MAR=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_MAR=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_mesh.c
DEP_CPP_TR_ME=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_ME=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_model.c
DEP_CPP_TR_MO=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_MO=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_noise.c
DEP_CPP_TR_NO=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_NO=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_scene.c
DEP_CPP_TR_SC=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_SC=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_shade.c
DEP_CPP_TR_SH=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_SH=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_shade_calc.c
DEP_CPP_TR_SHA=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_SHA=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_shader.c
DEP_CPP_TR_SHAD=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_SHAD=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_shadows.c
DEP_CPP_TR_SHADO=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_SHADO=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_sky.c
DEP_CPP_TR_SK=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_SK=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_surface.c
DEP_CPP_TR_SU=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_SU=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=tr_world.c
DEP_CPP_TR_WO=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_TR_WO=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\win_gamma.c
DEP_CPP_WIN_G=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	"..\win32\glw_win.h"\
	"..\win32\win_local.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_WIN_G=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\win_glimp.c
DEP_CPP_WIN_GL=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	"..\win32\glw_win.h"\
	"..\win32\resource.h"\
	"..\win32\win_local.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_WIN_GL=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\win_qgl.c
DEP_CPP_WIN_Q=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	"..\win32\glw_win.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_WIN_Q=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\qcommon\cm_public.h
# End Source File
# Begin Source File

SOURCE=..\win32\glw_win.h
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

SOURCE=qgl.h
# End Source File
# Begin Source File

SOURCE=.\qgl_linked.h
# End Source File
# Begin Source File

SOURCE=..\game\surfaceflags.h
# End Source File
# Begin Source File

SOURCE=tr_local.h
# End Source File
# Begin Source File

SOURCE=tr_public.h
# End Source File
# Begin Source File

SOURCE=..\cgame\tr_types.h
# End Source File
# Begin Source File

SOURCE=..\win32\win_local.h
# End Source File
# End Group
# Begin Group "jpeg"

# PROP Default_Filter ""
# Begin Group "Source Files No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\jpeg-6\jcapimin.c"
DEP_CPP_JCAPI=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jccoefct.c"
DEP_CPP_JCCOE=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jccolor.c"
DEP_CPP_JCCOL=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jcdctmgr.c"
DEP_CPP_JCDCT=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jdct.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jchuff.c"
DEP_CPP_JCHUF=\
	"..\jpeg-6\jchuff.h"\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jcinit.c"
DEP_CPP_JCINI=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jcmainct.c"
DEP_CPP_JCMAI=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jcmarker.c"
DEP_CPP_JCMAR=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jcmaster.c"
DEP_CPP_JCMAS=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jcomapi.c"
DEP_CPP_JCOMA=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jcparam.c"
DEP_CPP_JCPAR=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jcphuff.c"
DEP_CPP_JCPHU=\
	"..\jpeg-6\jchuff.h"\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jcprepct.c"
DEP_CPP_JCPRE=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jcsample.c"
DEP_CPP_JCSAM=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jctrans.c"
DEP_CPP_JCTRA=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdapimin.c"
DEP_CPP_JDAPI=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdapistd.c"
DEP_CPP_JDAPIS=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdatadst.c"
DEP_CPP_JDATA=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdatasrc.c"
DEP_CPP_JDATAS=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdcoefct.c"
DEP_CPP_JDCOE=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdcolor.c"
DEP_CPP_JDCOL=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jddctmgr.c"
DEP_CPP_JDDCT=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jdct.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdhuff.c"
DEP_CPP_JDHUF=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jdhuff.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdinput.c"
DEP_CPP_JDINP=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdmainct.c"
DEP_CPP_JDMAI=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdmarker.c"
DEP_CPP_JDMAR=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdmaster.c"
DEP_CPP_JDMAS=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdpostct.c"
DEP_CPP_JDPOS=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdsample.c"
DEP_CPP_JDSAM=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdtrans.c"
DEP_CPP_JDTRA=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jerror.c"
DEP_CPP_JERRO=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	"..\jpeg-6\jversion.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_JERRO=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jfdctflt.c"
DEP_CPP_JFDCT=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jdct.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jidctflt.c"
DEP_CPP_JIDCT=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jdct.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jmemmgr.c"
DEP_CPP_JMEMM=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmemsys.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jmemnobs.c"
DEP_CPP_JMEMN=\
	"..\cgame\tr_types.h"\
	"..\game\bg_lib.h"\
	"..\game\q_shared.h"\
	"..\game\surfaceflags.h"\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmemsys.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	"..\qcommon\cm_public.h"\
	"..\qcommon\qcommon.h"\
	"..\qcommon\qfiles.h"\
	".\qgl.h"\
	".\qgl_linked.h"\
	".\tr_local.h"\
	".\tr_public.h"\
	{$(INCLUDE)}"gl\glext.h"\
	
NODEP_CPP_JMEMN=\
	".\macosx_glimp.h"\
	".\macosx_qgl.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jutils.c"
DEP_CPP_JUTIL=\
	"..\jpeg-6\jconfig.h"\
	"..\jpeg-6\jerror.h"\
	"..\jpeg-6\jinclude.h"\
	"..\jpeg-6\jmorecfg.h"\
	"..\jpeg-6\jpegint.h"\
	"..\jpeg-6\jpeglib.h"\
	

!IF  "$(CFG)" == "renderer - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "renderer - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\jpeg-6\jchuff.h"
# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jconfig.h"
# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdct.h"
# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jdhuff.h"
# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jerror.h"
# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jinclude.h"
# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jmemsys.h"
# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jmorecfg.h"
# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jpegint.h"
# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jpeglib.h"
# End Source File
# Begin Source File

SOURCE="..\jpeg-6\jversion.h"
# End Source File
# End Group
# End Group
# Begin Group "FreeType2"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Include files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\ft2\ahangles.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahglobal.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahglyph.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahhint.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahloader.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahmodule.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahoptim.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahtypes.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\autohint.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\freetype.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftbbox.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftcalc.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftconfig.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftdebug.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftdriver.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\fterrors.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftextend.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftglyph.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftimage.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftlist.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftmemory.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftmm.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftmodule.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftnames.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftobjs.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftoption.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftoutln.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftrender.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftstream.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftsystem.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\fttypes.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\psnames.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\sfnt.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\t1errors.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\t1tables.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\t1types.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\t2errors.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\t2types.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttdriver.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\tterrors.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttgload.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttinterp.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttnameid.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttobjs.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttpload.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\tttables.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\tttags.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\tttypes.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=..\ft2\ahangles.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahglobal.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahglyph.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahhint.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahmodule.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ahoptim.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftcalc.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftdebug.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftextend.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftglyph.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftgrays.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftinit.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftlist.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftmm.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftnames.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftobjs.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftoutln.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftraster.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftrend1.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftsmooth.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftstream.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ftsystem.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\sfdriver.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\sfobjs.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttcmap.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttdriver.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttgload.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttinterp.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttload.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttobjs.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttpload.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttpost.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\ft2\ttsbit.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
