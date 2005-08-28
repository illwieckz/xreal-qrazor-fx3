# Microsoft Developer Studio Project File - Name="Splines" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Splines - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Splines.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Splines.mak" CFG="Splines - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Splines - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Splines - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/MissionPack/code/Splines", WJNAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Splines - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_MBCS" /YX /GF /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_MBCS" /YX /GF /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Splines - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_MBCS" /YX /GZ /c
# ADD CPP /nologo /MTd /W3 /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_MBCS" /YX /GZ /c
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

# Name "Splines - Win32 Release"
# Name "Splines - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=math_angles.cpp
DEP_CPP_MATH_=\
	".\math_angles.h"\
	".\math_matrix.h"\
	".\math_quaternion.h"\
	".\math_vector.h"\
	".\q_shared.h"\
	

!IF  "$(CFG)" == "Splines - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "Splines - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=math_matrix.cpp
DEP_CPP_MATH_M=\
	".\math_angles.h"\
	".\math_matrix.h"\
	".\math_quaternion.h"\
	".\math_vector.h"\
	".\q_shared.h"\
	

!IF  "$(CFG)" == "Splines - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "Splines - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=math_quaternion.cpp
DEP_CPP_MATH_Q=\
	".\math_matrix.h"\
	".\math_quaternion.h"\
	".\math_vector.h"\
	

!IF  "$(CFG)" == "Splines - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "Splines - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=math_vector.cpp
DEP_CPP_MATH_V=\
	".\math_vector.h"\
	

!IF  "$(CFG)" == "Splines - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "Splines - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=q_parse.cpp
DEP_CPP_Q_PAR=\
	".\math_angles.h"\
	".\math_matrix.h"\
	".\math_quaternion.h"\
	".\math_vector.h"\
	".\q_shared.hpp"\
	

!IF  "$(CFG)" == "Splines - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "Splines - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=q_shared.cpp
DEP_CPP_Q_SHA=\
	".\math_angles.h"\
	".\math_matrix.h"\
	".\math_quaternion.h"\
	".\math_vector.h"\
	".\q_shared.hpp"\
	

!IF  "$(CFG)" == "Splines - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "Splines - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=splines.cpp
DEP_CPP_SPLIN=\
	".\math_angles.h"\
	".\math_matrix.h"\
	".\math_quaternion.h"\
	".\math_vector.h"\
	".\q_shared.hpp"\
	".\splines.h"\
	".\util_list.h"\
	".\util_str.h"\
	
NODEP_CPP_SPLIN=\
	"..\qgl.h"\
	

!IF  "$(CFG)" == "Splines - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "Splines - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=util_str.cpp
DEP_CPP_UTIL_=\
	".\util_str.h"\
	

!IF  "$(CFG)" == "Splines - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "Splines - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=math_angles.h
# End Source File
# Begin Source File

SOURCE=math_matrix.h
# End Source File
# Begin Source File

SOURCE=math_quaternion.h
# End Source File
# Begin Source File

SOURCE=math_vector.h
# End Source File
# Begin Source File

SOURCE=q_shared.h
# End Source File
# Begin Source File

SOURCE=splines.h
# End Source File
# Begin Source File

SOURCE=util_list.h
# End Source File
# Begin Source File

SOURCE=util_str.h
# End Source File
# End Group
# End Target
# End Project
